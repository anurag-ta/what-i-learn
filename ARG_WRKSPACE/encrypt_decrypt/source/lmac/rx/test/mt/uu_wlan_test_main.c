#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_rx_test_main.h"

#define int_to_boolean(x) (x)?1:0
#define MAX_FAILED_CASES 100

/** To maintain the success_rate of testing
 * uu_wlan_test_success_cnt_g is the no.of testchains passed succesfully
 * uu_wlan_test_failure_cnt_g is count of failed testchains
 */
static uu_int32 uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g;
    
/* failed_case array is to list out the testchains names which are failed */
static uu_char failed_case[MAX_FAILED_CASES][100];

/* Function to read test_input from the file and handover to rx_handler 
 * file_name is a file contains test_input to process either RX_START, DATA or END indications
 * returns UU_FAILURE/UU_SUCCESS based on process of indications
 */
static uu_int32 tester_fun_testinput(uu_char *file_name);

/* Function to read test-input file_name from testchain and handover to testinput fun. to process
 * file_name is a file contains testchain(set of test-input file names)
 * returns UU_SUCCESS/UU_FAILURE based on test_input fun result
 */
static uu_int32 tester_fun_testchain(uu_char *file_name);

uu_void uu_wlan_umac_cbk(uu_int8 IND);
uu_lmac_ops_t lmac_ops_g =
{
    .umacCBK = uu_wlan_umac_cbk
};

uu_lmac_ops_t *lmac_ops_gp;


/** Function to set local bssid, ssid and mac addr
 * For every testchain this fun will be called
 */
static void uu_wlan_set_sta_mac_addr(void);


typedef struct module_test_case
{
    /* Name of the function under test (FUT), in string form */    
    uu_char  *fun_to_be_tested;
     
    /* Test case input file in string form, for user understanding */
    uu_char  *test_input_file;
    
    /* Function pointer, to the test driver.
     * Input is test-case-id, so that the FUT will be given suitable inputs for this test case */
    uu_void   (*test_driver)(uu_char*);
} module_test_case_t;


/* Simulation of other module functionalities starts here */
uu_uint32 uu_wlan_lmac_qos_null_tx_handling(uu_uchar *ra , uu_uint16 duration)
{
    return UU_SUCCESS;
}

uu_uint32 uu_wlan_lmac_pspoll_tx_handling(uu_uchar *ra , uu_uint16 aid)
{
    return UU_SUCCESS;
}

uu_bool uu_wlan_is_tx_ready_for_any_AC_Q(uu_void)
{
    return false;
}

uu_int32 uu_wlan_cap_dtim_timer_start(uu_void)
{
    return UU_SUCCESS;
}

uu_void uu_wlan_cap_set_recv_pkt_status(uu_wlan_rx_handler_return_value_t ret)
{
}

uu_void uu_wlan_cap_stop_beacon_tx (uu_void)
{
}

uu_int32 uu_wlan_put_msg_in_CAP_Q(uu_wlan_cap_event_type_t ev_type, uu_uchar *data_p, uu_int32 len)
{
    return UU_SUCCESS;
}
    
uu_int32 uu_wlan_cp_process_rx_frame(uu_wlan_rx_frame_info_t *frame_info, uu_bool is_rxend)
{
    return UU_SUCCESS;
}
/* End of simulations */


uu_int32 uu_wlan_test_get_expected_output(uu_char *input_file)
{
    FILE *fp;
    uu_char buf[100], string[50];
    uu_int32 value;

    fp=fopen(input_file ,"r");
    
    if(fp == NULL)
    {
        return UU_FAILURE;
    }
    else
    {
        while (fgets(buf, sizeof(buf), fp))
        {
            printf("buf read is: %s", buf);
            sscanf(buf, "%s %d", string, &value);
            if(strcmp(string, "rx_end_status:") == 0)
            {
                sscanf(buf, "%s %d", string, &rx_end_status);
            }
            else if(strcmp(string, "tsf_r:") == 0)
            {
                sscanf(buf, "%s %Lx", string, &expected_tsf_g);
                printf("expected tsf_r is: %Lx\n", expected_tsf_g);
            }
            else if(strcmp(string, "ps_enabled:") == 0)
            {
                sscanf(buf, "%s %d", string, &value);
                ps_enable_g = int_to_boolean(value);
            }
            else if(strcmp(string, "dtim_rcvd:") == 0)
            {
                sscanf(buf, "%s %d", string, &value);
                dtim_bcn_g = int_to_boolean(value);
            }
            else if(strcmp(string, "eosp_g:") == 0)
            {
                sscanf(buf, "%s %d", string, &value);
                eosp_g = int_to_boolean(value);
            }
            else if(strcmp(string, "more_data:") == 0)
            {
                sscanf(buf, "%s %d", string, &value);
                more_data_g = int_to_boolean(value);
            }
           else
           {
           }
        }
        fclose(fp);
    }
    return UU_SUCCESS;
} /* uu_wlan_test_get_expected_output */


uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame)
{
    FILE *fp;
    uu_uchar xdigit = 0;
    uu_int32 num = 0;
    uu_int32 len = 0;
    uu_int32 ch, frame_end = 0;

    printf("file to read frame is: %s\n", file_name);

    fp=fopen(file_name ,"r");
    if(fp == NULL)
    {
        uu_wlan_test_failure_cnt_g++;
        return UU_FAILURE;
    }

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == FRAME_END_DELIMITER)
        {
            if (frame_end)
            {
                break;
            }
            frame_end = 1;
            continue;
        }
        if (ch == '\n')
        {
            continue;
        }
        frame_end = 0;

        if ((ch == ' ' ) || (ch == FRAME_HEX_DELIMITER))
        {
            frame[len++] = xdigit;
            xdigit=0;
        }
        else
        {
            if ((ch >= 0x30) && (ch < 0x3A))
                num = ch - 0x30;
            else if ((ch == 0x41) || (ch == 0x61))
                num = 0x0A;
            else if ((ch == 0x42) || (ch == 0x62))
                num = 0x0B;
            else if ((ch == 0x43) || (ch == 0x63))
                num = 0x0C;
            else if ((ch == 0x44) || (ch == 0x64))
                num = 0x0D;
            else if ((ch == 0x45) || (ch == 0x65))
                num = 0x0E;
            else if ((ch == 0x46) || (ch == 0x66))
                num = 0x0F;
            else
                num = 0x00;

            xdigit = (xdigit<<4) | (num&0x0f);
        }
    }
    fclose(fp);
    printf("len of the frame is: %d\n",len);
    return len;
} /* uu_wlan_test_get_frame_from_file */


static uu_void uu_wlan_tester_fun(char *file_name)
{
    FILE *fp;
    uu_char buf[FREAD_BUFF_SIZE + 1];

    fp = fopen(file_name,"r");
    if(fp == NULL)
    {
        strcpy(failed_case[uu_wlan_test_failure_cnt_g], file_name);
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: file(main_script) for testing does not exist: %s\n", file_name);
        return;
    }
    else
    {
        /* Here we read line by line from file(testchain file name) */
        while(fgets(buf, FREAD_BUFF_SIZE, fp))
        {
            if(buf[0] == '#')/* If line from file starting with # then its treated as comment */
            {
                //printf("comment:%s \n",buf);
            }
            else if(strncmp(buf, "@TEST", strlen("@TEST")) == 0)/* Test the given file */
            {
                uu_char  str[FREAD_BUFF_SIZE+1];
                uu_char  filename[FREAD_BUFF_SIZE+1];

                sscanf(buf, "%s %s ", str, filename);
                printf("testchain for testing is %s \n", filename);
                if(strncmp(filename, "rx_testfiles/testchain_ibss", 27) == 0)
                {
                    uu_dot11_op_mode_r = 3;
                    uu_wlan_ps_mode_r = 0;
                    uu_wlan_uapsd_mode_r = 0;
                }
                else if(strncmp(filename, "rx_testfiles/testchain_ps", 25) == 0)
                {
                    uu_wlan_ps_mode_r = 1;
                    uu_wlan_uapsd_mode_r = 0;
                    uu_dot11_op_mode_r = 0;
                    if(strncmp(filename, "rx_testfiles/testchain_ps_uapsd", 31) == 0)
                    {
                        uu_wlan_uapsd_mode_r = 1;
                    }
                }
                else /*if(strncmp(filename, "rx_testfiles/testchain", 22) == 0)*/
                {
                    uu_wlan_ps_mode_r = 0;
                    uu_wlan_uapsd_mode_r = 0;
                    uu_dot11_op_mode_r = 0;
                }

                if(tester_fun_testchain(filename) == UU_SUCCESS)
                {
                    uu_wlan_test_success_cnt_g++;
                }
                else
                {
                    strcpy(failed_case[uu_wlan_test_failure_cnt_g], filename);
                    uu_wlan_test_failure_cnt_g++;
                }
            }
        } // while
        fclose(fp);
    }
}/* uu_wlan_tester_fun */


static uu_int32 tester_fun_testchain(uu_char *file_name)
{
    FILE *fp;
    uu_int32 ret = UU_SUCCESS;
    uu_char buf[FREAD_BUFF_SIZE + 1];

    fp = fopen(file_name,"r");
    if(fp == NULL)
    {
        strcpy(failed_case[uu_wlan_test_failure_cnt_g], file_name);
        printf("ERROR: file(test_chain) for testing does not exist: %s\n", file_name);
        ret = UU_FAILURE;
    }
    else
    {
        /* Here we read line by line from file(test input file name) */
        while(fgets(buf , FREAD_BUFF_SIZE, fp))
        {
            if(buf[0] == '#')/* If line from file starting with # then its treated as comment */
            {
                //printf("comment:%s \n",buf);
            }
            else if(strncmp(buf, "@TEST", strlen("@TEST")) == 0)/* Test the given file */
            {
                uu_char  str[FREAD_BUFF_SIZE+1];
                uu_char  filename[FREAD_BUFF_SIZE+1];
                sscanf(buf,"%s %s ", str, filename);
                ret = tester_fun_testinput(filename);
                printf("ret val of test-input is: %d\n", ret);
                if(ret == UU_FAILURE)
                {
                    break;
                }
            }
        }
        fclose(fp);
    }     
    return ret;
}/* tester_fun_testchain */


static uu_int32 tester_fun_testinput(uu_char *file_name)
{
    uu_int32 rq_type;
    uu_int32 case_id;
    uu_int32 ret = UU_SUCCESS;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    uu_char input_id[30];
    FILE *fp;

    printf("testinput file for testing is: %s\n", file_name);

    fp = fopen(file_name,"r");
    if(fp == NULL)
    {
        strcpy(failed_case[uu_wlan_test_failure_cnt_g], file_name);
        printf("ERROR: file(test_input) for testing does not exist: %s\n", file_name);
        ret = UU_FAILURE;
    }
    else
    {
        /* Here we read line by line from file(individual testcase id and parameters) */
        while(fgets(buf , FREAD_BUFF_SIZE, fp))
        {
            if(buf[0] == '#')/* If line from file starting with # then its treated as comment */
            {
                //printf("comment:%s \n",buf);
            }
            else
            {
                sscanf(buf,"%s %d event_type: %d ", input_id, &case_id, &rq_type);
                printf("test_case id %d, and rq_type %d\n", case_id, rq_type);
                ret = uu_wlan_rx_test_handler(rq_type, buf);
            }
        }
        fclose(fp);
    }
    return ret;
}/* tester_fun_testinput */


module_test_case_t  uu_wlan_test_cases_g[] =
{
    {"UURMI LMAC RX Tester", INPUT_SCRIPT_FILE, uu_wlan_tester_fun},
    {NULL, NULL, NULL}
};


int main(int argc, char *argv[])
{
    int i, k;

    uu_wlan_set_sta_mac_addr();
    uu_lmac_reg_ops(&lmac_ops_g);
    for (i = 0; uu_wlan_test_cases_g[i].fun_to_be_tested != NULL; i++)
    {
        /* This is for each individual test case id in particular function(coressponding test_input_file) */
        if (argc == 3)
        {
            if(((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested))==0) &&
                ((strcmp(argv[2], uu_wlan_test_cases_g[i].test_input_file))==0))
            {
                printf("Testing function: %s, case: %s\n",
                    uu_wlan_test_cases_g[i].fun_to_be_tested, uu_wlan_test_cases_g[i].test_input_file);
                uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);
            }
        }

        /* With this we can run all testcase ids in particular funtion(test_input_file))
          argumet 2 is function under test.
          argument 1 is exe file
         */
        else if( argc == 2)
        {
            if((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested))==0)
            {
                printf("Testing function: %s, case: %s\n", 
                    uu_wlan_test_cases_g[i].fun_to_be_tested, uu_wlan_test_cases_g[i].test_input_file);
                uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);
            }
        }

        /* This is case run all function under test files which are maintained in queue. */
        else
        {
            printf("ELSE Testing function: %s, case: %s\n",
                uu_wlan_test_cases_g[i].fun_to_be_tested, uu_wlan_test_cases_g[i].test_input_file);
            uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);
        }

        printf("Success cases:  %d \n Failed cases:  %d\n Total:  %d\n",
            uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g,
            uu_wlan_test_success_cnt_g+uu_wlan_test_failure_cnt_g);

        /* Displaying list of all failed testchains names at the end of execution */
        if(uu_wlan_test_failure_cnt_g)
        {
            printf("Failed cases are:\n");
        }
        for(k=0; k<uu_wlan_test_failure_cnt_g; k++)
        {
            printf("%s\n", failed_case[k]);
        }
    }

    return 1;
} /* main */


static void uu_wlan_set_sta_mac_addr(void)
{
    /** Setting sta address */
    uu_dot11_sta_mac_addr_r[0] = 0x22;
    uu_dot11_sta_mac_addr_r[1] = 0x22;
    uu_dot11_sta_mac_addr_r[2] = 0x22;
    uu_dot11_sta_mac_addr_r[3] = 0x22;
    uu_dot11_sta_mac_addr_r[4] = 0x22;
    uu_dot11_sta_mac_addr_r[5] = 0x22;
    uu_dot11_sta_bssid_r[0] = 0x22;
    uu_dot11_sta_bssid_r[1] = 0x22;
    uu_dot11_sta_bssid_r[2] = 0x22;
    uu_dot11_sta_bssid_r[3] = 0x22;
    uu_dot11_sta_bssid_r[4] = 0x22;
    uu_dot11_sta_bssid_r[5] = 0x22;

    UU_REG_LMAC_FILTER_FLAG = UU_WLAN_FILTR_NO_FILTER;
} /* uu_wlan_set_sta_mac_addr */


int uu_lmac_reg_ops(uu_lmac_ops_t *ops)
{
    if (ops != NULL)
        lmac_ops_gp = ops;
    return 0;
}


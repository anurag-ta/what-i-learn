#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>

#include "uu_errno.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_cap_init.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_cp_if.h"

#include "uu_wlan_cap_test_main.h" 

#define MAX_FAILED_TEST_CASES  100

/* uu_wlan_test_success_cnt_g maintains the count of testchains passed successfully
 * uu_wlan_test_success_cnt_g maintains the count of failed testchains
 */
static uu_int32 uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g;

/* Structure to maintain failed input_id and testchain_name contains failed input */
struct failed_test_list
{
    uu_char failed_case[100];    /* Test chain file name */
    uu_char failed_input_id[50]; /* Test input id, inside the test chain file */
} failed_test_list_g[MAX_FAILED_TEST_CASES];


uu_wlan_tx_frame_info_t tx_ctl_frame_info_g;
uu_wlan_tx_frame_info_t *frame_info;
uu_uchar  uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t)+256];
uu_wlan_tx_frame_info_t *tx_ctl_frame_info_gp = (uu_wlan_tx_frame_info_t *)uu_wlan_tx_ctl_frame_info_g;

uu_int32 rx_handler_ret_g = 0xff;


/** Function to set defualt reg. values for every testchain
 * It sets mac_addr, txop_limits, cw_values, timers etc.
*/
static uu_void uu_wlan_set_default_reg_values(uu_void);

/* Function to read test-input from testchain file and handover to cap_test_handler 
 * If any test-input is failed then it will break that testchain and continue 
 * returns UU_FAILURE/UU_SUCCESS based on process of indications
 */
static uu_int32 tester_fun_testinput(uu_char *file_name);

/* phy_stub functionality simulation in following four functions */
void uu_wlan_phy_txstart_req(unsigned char *data, int len)
{
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_START_CONFIRM, NULL, 0);
}

void uu_wlan_phy_data_req(unsigned char data, uu_uchar *confirm_flag)
{
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_DATA_CONFIRM, NULL, 0);
}

void uu_wlan_phy_txend_req(void)
{
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_END_CONFIRM, NULL, 0);
}

void uu_wlan_phy_ccareset_req(void)
{
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_CCA_RESET_CONFIRM, NULL, 0);
}
/* End for phy_stub functionality simulation */


/* TX functionality simulation in following functions */
uu_void uu_wlan_tx_set_retry_bit(uu_uint8 ac)
{
}

uu_void uu_wlan_tx_update_with_fb_rate(uu_uint8 ac)
{
}

uu_void uu_wlan_tx_update_status_for_umac(uu_uint8 ac, uu_uint16 seqno, uu_uint8 retries, uu_bool status)
{
}

uu_int32 uu_wlan_handle_txop_tx_start_req(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw)
{
    frame_info = uu_wlan_get_msg_ref_from_AC_Q(ac);
    if(frame_info == UU_NULL)
    {
        /* reset tx_ready flag */
        return UU_FAILURE;
    }
    uu_wlan_cap_retain_tx_frame_info(frame_info);
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_START_CONFIRM, NULL, 0);
    return UU_SUCCESS;
}

uu_int32 uu_wlan_tx_send_more_data(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw)
{
    frame_info = uu_wlan_get_msg_ref_from_AC_Q(ac);
    if(frame_info == UU_NULL)
    {
        /* reset tx_ready flag */
        return UU_FAILURE;
    }
    if(((UU_WLAN_TXOP_LIMIT_R(ac) == 0) || (! UU_WLAN_IEEE80211_QOS_MODE_R))
            && (!IEEE80211_IS_FC1_MOREFRAG_SET(frame_info->mpdu)))
    {
        /* If the previous frame is not control frame, it is error */
        if ((uu_wlan_cap_ctxt_g.tx_frame_info.mpdu[0] & IEEE80211_FC0_TYPE_MASK) != IEEE80211_FC0_TYPE_CTL)
        {
            //    UU_WLAN_LOG_DEBUG(("LMAC: tx_send_more_data - Last frame was not control frame\n"));
            return UU_FAILURE;
        }
    }
    uu_wlan_cap_retain_tx_frame_info(frame_info);
    return UU_SUCCESS;
}

uu_void uu_wlan_tx_phy_data_req(uu_char ac)
{
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_DATA_CONFIRM, NULL, 0);
}
/* End for TX functionality simulation */


/* Rx functions simulation starts here */
uu_void uu_wlan_rx_update_status_for_umac(uu_uint32 offset, uu_uint16 len)
{
}

uu_int32 uu_wlan_rx_handle_phy_rxstart(uu_wlan_rx_vector_t  *rx_vec)
{
    return UU_SUCCESS;
}

uu_int32 uu_wlan_rx_handle_phy_dataind(uu_uchar  data)
{
    return UU_SUCCESS;
}

uu_int32 uu_wlan_rx_handle_phy_rxend(uu_uchar  rxend_stat)
{
     if(rx_handler_ret_g)
     {
         uu_wlan_cap_rx_status_value_g = rx_handler_ret_g;
     }
     else
     {
         uu_wlan_cap_rx_status_value_g = -1;
     }
     return UU_SUCCESS;

} /* uu_wlan_rx_handle_phy_rxend */
/* End of rx functions simulation */


/* Power-save functions simulation starts here */
uu_void uu_wlan_ps_idle_exit(uu_void)
{
}

uu_bool uu_wlan_is_sta_in_sleep(uu_void)
{
    return 0;
}

uu_void uu_wlan_ps_handle_idle_entry(uu_void)
{
}
/* End of power-save functionality simulation */


typedef struct module_test_case
{
    /* Name of the function under test (FUT), in string form */
    uu_char  *fun_to_be_tested;

    /* Test case input file in string form, for user understanding */
    uu_char  *test_input_file;

    /* Function pointer, to the test driver.
     * Input is test-case-id, so that the FUT will be given suitable inputs for this test case */
    uu_void   (*test_driver)(char*);
} module_test_case_t;


uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame)
{
    FILE *fp;
    uu_uchar xdigit=0;
    uu_int32 num=0;
    uu_int32 len=0;
    uu_int32 ch, frame_end=0;
    
    fp=fopen(file_name , "r");
   
    if(fp == NULL)
    {
        //uu_wlan_test_failure_cnt_g++;
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

        if (ch == FRAME_HEX_DELIMITER)
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
    printf("len of frame = %d\n", len);
    return len;
}


/* Reads the testchains names and handover to testinput function
 * file_name is the main script contains list of testchains
 * Based on testchain process it will increment either success/failure count
 */
static uu_void uu_wlan_tester_fun(uu_char *file_name)
{
    FILE *fp;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    uu_int32 ret;

    fp = fopen(file_name, "r");

    if(fp == NULL)
    {
        strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g].failed_case, file_name);
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
                uu_wlan_set_default_reg_values();
                if(strncmp(filename, "cap_testcases/testchain_ibss", 28 )==0)
                {
                    UU_WLAN_IEEE80211_OP_MODE_R = UU_WLAN_MODE_ADHOC;
                }
                else
                {
                    UU_WLAN_IEEE80211_OP_MODE_R = 0;
                }

                ret = tester_fun_testinput(filename);
                if(ret == UU_SUCCESS)
                {
                    uu_wlan_test_success_cnt_g++;
                }
                else
                {
                    strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g].failed_case, filename);
                    uu_wlan_test_failure_cnt_g++;
                }
            }
        } // while
        fclose(fp);
    }
}/* uu_wlan_tester_fun */


static uu_int32 tester_fun_testinput(uu_char *file_name)
{
    uu_int32 rq_type;
    uu_int32 ret = UU_SUCCESS;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    uu_char input_id[30];
    FILE *fp;

    printf("testinput file for testing is: %s\n", file_name);

    fp = fopen(file_name, "r");

    if(fp == NULL)
    {
        strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g].failed_case, file_name);
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
                sscanf(buf,"%s ev_type: %d ", input_id, &rq_type);
                ret = uu_wlan_cap_test_handler(rq_type, buf);
                if(ret != UU_SUCCESS)
                {
                    strcpy(failed_test_list_g[uu_wlan_test_failure_cnt_g].failed_input_id, input_id);
                    break;
                }
            }
        }
        fclose(fp);
    }
    return ret;
}/* tester_fun_testinput */


module_test_case_t  uu_wlan_test_cases_g[] =
{
    {"UURMI LMAC CAP Tester", INPUT_SCRIPT_FILE, uu_wlan_tester_fun}, 
    {NULL, NULL, NULL}
};


int main(int argc, char *argv[])
{
    int i, k;
    uu_wlan_cap_init();

    uu_wlan_set_default_reg_values();
    for (i = 0; uu_wlan_test_cases_g[i].fun_to_be_tested != NULL; i++)
    {
        /* This is for each individual test case id in perticular function (corresponding test_input_file) */
        if (argc == 3)
        {
            if ((strcmp(argv[1], uu_wlan_test_cases_g[i].fun_to_be_tested) == 0) &&
                (strcmp(argv[2], uu_wlan_test_cases_g[i].test_input_file) == 0))
            {
                printf("Testing function: %s, case: %s\n",
                    uu_wlan_test_cases_g[i].fun_to_be_tested, uu_wlan_test_cases_g[i].test_input_file); 
                uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);

            }
        }

        /* With this we can run all testcase ids in perticular funtion(test_input_file))
          argumet 2 is function under test.
          argument 1 is exe file
         */
        else if( argc == 2)
        {  
            if((strcmp(argv[1], uu_wlan_test_cases_g[i].fun_to_be_tested))==0)
            {
                printf("Testing function: %s, case: %s\n",
                    uu_wlan_test_cases_g[i].fun_to_be_tested, uu_wlan_test_cases_g[i].test_input_file); 
                uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);
            }
        }
        /* This is case run all function under test files which are maintained in queue. */
        else
        {
            printf("Testing function: %s, case: %s\n",
                uu_wlan_test_cases_g[i].fun_to_be_tested, uu_wlan_test_cases_g[i].test_input_file);
            uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file);
        }
        printf(" Success cases:  %d \n Failed cases:  %d\n Total no of cases:  %d \n",
            uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g,
            uu_wlan_test_success_cnt_g+uu_wlan_test_failure_cnt_g);

        /* Displaying list of all failed testcases names at the end of execution */
        if(uu_wlan_test_failure_cnt_g)
        {
            printf("failed cases are:\n");
        }
        for(k=0; k<uu_wlan_test_failure_cnt_g; k++)
        {
            printf("%s %s\n", failed_test_list_g[k].failed_case, failed_test_list_g[k].failed_input_id);
        }
    }
    return 1;
} /* main */


static uu_void uu_wlan_set_default_reg_values(uu_void)
{
    /** Setting sta address */
    uu_dot11_sta_mac_addr_r[0] = 0x22;
    uu_dot11_sta_mac_addr_r[1] = 0x22;
    uu_dot11_sta_mac_addr_r[2] = 0x22;
    uu_dot11_sta_mac_addr_r[3] = 0x22;
    uu_dot11_sta_mac_addr_r[4] = 0x22;
    uu_dot11_sta_mac_addr_r[5] = 0x22;

    /** EIFS Timeout value */
    dot11_eifs_timer_value = 10;

    /** Slot timer default value is : 9 micro sec */
    dot11_slot_timer_value =  1;

    /** SIFS timer default value is : 16 micro sec */
    dot11_sifs_timer_value = 16;

    /** DIFS Timeout value */
    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.7 
     * DIFS = aSIFSTime + 2 * aSlotTime 
     * PIFS = aSIFSTime + aSlotTime 
     */
    dot11_difs_value =  2 * dot11_slot_timer_value; //dot11_sifs_timer_value + 2 * dot11_slot_timer_value;

    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.2.8 ACK Procedure 
     * ACK timeout interval value is aSIFSTime + aSlotTime + aPHY-RX-START-Delay 
     */
    /** For OFDM PHY aPHY-RX-START-Delay is 25 micro sec for 20MHz channel spacing */
    dot11_aPhyRxStartDelay = 25;

    /** ACK Timeout value */
    dot11_ack_timer_value = dot11_slot_timer_value + dot11_aPhyRxStartDelay; // Add dot11_sifs_timer_value 

    /** CTS Timeout value */
    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.2.6 CTS Procedure 
     * CTS timeout interval value is aSIFSTime + aSlotTime + aPHY-RX-START-Delay 
     */
    dot11_cts_timer_value = dot11_slot_timer_value + dot11_aPhyRxStartDelay; // Add dot11_sifs_timer_value

    /** Short retry and long retry counts */
    /** Tx retry limit defaults from standard Refer IEEE802.11-2007 - Page 874 - 
     * dot11ShortRetryLimit Integer:= 7
     * dot11LongRetryLimit Integer:= 4, 
     */
    dot11_short_retry_count = 4;
    dot11_long_retry_count = 4;

    /** CWmin value is : 15 
     * CWmax value is : 1023 
     * For OFDM PHY :
     * Following table indicates the Default EDCA parameter values in case of dot11OCBActivated is false 
     * -------------------------------------------------------------------------------------
     * -    AC   -    CWMin            -        CWMax       -     AIFSN    - TXOP Limit    -
     * -------------------------------------------------------------------------------------
     * - AC_BK   -    aCWMin           -      aCWMax        -       7      -     0         -
     * - AC_BE   -    aCWMin           -      aCWMax        -       3      -     0         -
     * - AC_VI   -   (aCWMin+1)/2-1    -      aCWMin        -       2      -  3.008 ms     -
     * - AC_VO   -   (aCWMin+1)/4-1    -    (aCWMin+1)/2-1  -       2      -  1.504 ms     -
     * -------------------------------------------------------------------------------------
     * 
     */
    dot11_cwmin_value = 15;
    dot11_cwmax_value = 1023;

    /** For AC_BK */
    dot11_cwmin_value_ac[0] = 15;
    dot11_cwmax_value_ac[0] = 1023;
    dot11_txop_limit_value[0] = 0;
    dot11_aifs_value[0] = 7;

    /** For AC_BE */
    dot11_cwmin_value_ac[1] = 15;
    dot11_cwmax_value_ac[1] = 1023;
    dot11_txop_limit_value[1] = 0;
    dot11_aifs_value[1] = 3;

    /** For AC_VI */
    dot11_cwmin_value_ac[2] = 7;
    dot11_cwmax_value_ac[2] = 15;
    dot11_txop_limit_value[2] = 3; // Rounded off to 3
    dot11_aifs_value[2] = 2;

    /** For AC_VO */
    dot11_cwmin_value_ac[3] = 3;
    dot11_cwmax_value_ac[3] = 7;
    dot11_txop_limit_value[3] = 1; // Rounded off to 1
    dot11_aifs_value[3] = 2;

    /** Default beacon interval is 100ms (100000usec) */
    dot11_BeaconInterval = 800000; /* TODO: Temp fix - Multiplied by 8 */

    /** Default value for RTS threshold value */ 
    dot11_rts_threshold_value = 1500;

    /** As per 802.11 REV-mb D12 section 9.3.8, signal extension is present for HT-MF or HT-GF or ERP-OFDM, DSSS-OFDM, and NON_HT_DUPOFDM
     * For RIFS case, NO_SIG_EXTN is true.
     * Table 20-25, aSignalExtension 0 μs when operating in the 5 GHz band and 6 μs when operating in the 2.4 GHz band
     * Pg 1753, 3rd 4th paragraph.
     * Since it is defined in Annex 20. It probably means that HT STA shall support signal extension in non-HT format too. It may not be supported by legacy STAs.
     * This is not present in VHT format.
     */
    dot11_signal_extension = 0;

    /** Default value for QoS register is 1 (enabled) */
    uu_dot11_qos_mode_r = 1;

    /** Default operational model is Station (Managed) */
    uu_dot11_op_mode_r = UU_WLAN_MODE_MANAGED;

} /* uu_wlan_set_default_reg_values */


/* Dummy functions, as CAP handler thread is not included in the test-cde build */
uu_int32  uu_wlan_cap_handler_start(uu_void)
{
    return UU_SUCCESS;
}
uu_void  uu_wlan_cap_handler_stop(uu_void) {}

/* Dummy lock, as LMAC init is not included */
uu_wlan_spin_lock_type_t uu_wlan_cap_Q_lock_g = 1;

/* CP dummy functions as cp is not included in test-build */
uu_void uu_wlan_cp_set_waiting_for_cts() {}
uu_void uu_wlan_cp_set_waiting_for_ack() {}
uu_void uu_wlan_cp_reset_waiting_for_cts() {}
uu_void uu_wlan_cp_reset_waiting_for_ack() {}

/* EOF */


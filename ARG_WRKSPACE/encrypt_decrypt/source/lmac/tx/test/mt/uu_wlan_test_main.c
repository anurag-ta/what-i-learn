#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>

#include "crc32.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_tx_test_main.h"


/** Define CAP context global variable here */
uu_wlan_cap_context_t  uu_wlan_cap_ctxt_g;
uu_uchar  uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t)+256];
uu_wlan_tx_frame_info_t *tx_ctl_frame_info_gp = (uu_wlan_tx_frame_info_t *)uu_wlan_tx_ctl_frame_info_g;
uu_wlan_tx_frame_info_t tx_ref_frame_g;
uu_wlan_spin_lock_type_t uu_wlan_cap_Q_lock_g = 1;

uu_int32 uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g;

/* failed_case array is to list out the testcases which are failed */
static uu_char failed_case[3][100];

uu_void uu_wlan_umac_cbk(uu_int8 Ind);

uu_lmac_ops_t lmac_ops_g =
{
    .umacCBK = uu_wlan_umac_cbk
};

uu_lmac_ops_t *lmac_ops_gp;

/** Function to set default reg. values for every testcase
 * It sets mac_addr, txop_limits, rts_threshold etc.
 */
static uu_void uu_wlan_set_default_reg_values(uu_void);

/* Function to read test-input from testchain file and handover to tx_test_handler 
 * If any test-input is failed then it will break that testchain and continue with next testchain
 * returns UU_FAILURE/UU_SUCCESS based on process of indications
 */
static uu_int32 tester_fun_testinput(uu_char *file_name);


/* Simuation of other module functions starts here */
uu_void uu_wlan_cp_update_the_tx_sb(uu_uchar *ra, uu_uint8 tid, uu_uint16 scf)
{
}

uu_void uu_wlan_phy_data_req(uu_uchar data, uu_uchar *confirm_flag)
{
    static uu_int32 mpdu_byte_count = 0;
    *confirm_flag = 1;

    printf("data rcvd is:%x and mpdu_byte_count is: %d\n", data, mpdu_byte_count);
    frame_phy[mpdu_byte_count] = data;
    mpdu_byte_count++;

    printf("confirm flag and data rcvd is: %x %x\n", *confirm_flag, data);


    if(frame_phy[0] == (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_RTS))
    {
        if(mpdu_byte_count == UU_RTS_FRAME_LEN)
        {
            mpdu_byte_count = 0;
            confirm_flag = 0;
        }
    }
    else if(frame_phy[0] == (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_PSPOLL))
    {
        if(mpdu_byte_count == UU_PSPOLL_FRAME_LEN)
        {
            mpdu_byte_count = 0;
            confirm_flag = 0;
        }
    }
    else if(mpdu_byte_count == tx_ref_frame_g.frameInfo.framelen)
    {
        mpdu_byte_count = 0;
        confirm_flag = 0;
    }

    printf("length passed to phy_data_req is: %d\n", tx_ref_frame_g.frameInfo.framelen);
    //for(j = 0; j < tx_ref_frame_g.frameInfo.framelen; j++)
    //  printf("%x ", frame_phy[j]);
    printf("beacon timestamp starting byte is: %d\n ", frame_phy[(tx_ref_frame_g.frameInfo.framelen)+1]);
} /* uu_wlan_phy_data_req */


uu_uint16 uu_calc_impl_BA_duration(uu_wlan_tx_vector_t *txvec, uu_int32 expected_BA_length)
{
    return 10;
}

uu_uint16 uu_calc_bcast_duration(uu_wlan_tx_vector_t *txvec, uu_uint16 next_frag_len)
{
    return 0;
}

uu_uint16 uu_calc_singlep_frame_duration(uu_wlan_tx_vector_t *txvec, uu_bool ack, uu_int32 next_frame_length)
{
    return 100;
}

uu_uint16 uu_calc_frame_duration (uu_wlan_tx_vector_t *txvec)
{
	return 100;
}

uu_uint16 uu_calc_multip_frame_duration( uu_uint8 ac, uu_uint32 tx_nav)
{
	return 200;
}

uu_uint16 uu_calc_multip_first_frame_duration( uu_uint8 ac, uu_uint32 txop_limit)
{
	return 50;
}

uu_void uu_wlan_cap_retain_tx_frame_info(uu_wlan_tx_frame_info_t *frame_info)
{
    UU_WLAN_COPY_BYTES(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu, frame_info->mpdu,
            UU_WLAN_CAP_MIN_FRAME_HEADER_LEN(frame_info->frameInfo.framelen));
    uu_wlan_cap_ctxt_g.tx_frame_info.txvec_is_aggregated = frame_info->txvec.is_aggregated;
}

uu_void uu_wlan_phy_txstart_req(uu_uchar *data, uu_int32 len)
{
    memset(&tx_ref_frame_g, 0, sizeof(uu_wlan_tx_frame_info_t));
    memcpy((uu_uchar*)&tx_ref_frame_g.txvec, data, len);
    printf("uu_wlan_phy_txstart_req - tx_ref_frame_g.txvec l_length,"
        "ht length and apep_length: %d %d %d \n",
        tx_ref_frame_g.txvec.L_length, tx_ref_frame_g.txvec.ht_length,
        tx_ref_frame_g.txvec.tx_vector_user_params[0].apep_length);

    if(tx_ref_frame_g.txvec.is_aggregated)
    {
        if(tx_ref_frame_g.txvec.format==UU_WLAN_FRAME_FORMAT_NON_HT)
        {
            tx_ref_frame_g.frameInfo.framelen = tx_ref_frame_g.txvec.L_length;
        }
	    else if(tx_ref_frame_g.txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
	    {
	        tx_ref_frame_g.frameInfo.framelen = tx_ref_frame_g.txvec.tx_vector_user_params[0].apep_length;
	        printf("length for phy_data req is: %d\n", tx_ref_frame_g.frameInfo.framelen);
	    }
        else
        {
            tx_ref_frame_g.frameInfo.framelen = tx_ref_frame_g.txvec.ht_length;
        } 
    }   
    else
    {
        tx_ref_frame_g.frameInfo.framelen = tx_ref_frame_g.txvec.L_length;
    }
    printf("uu_wlan_phy_txstart_req tx_ref_frame_g.txvec l_length & ht length %d %d \n", 
        tx_ref_frame_g.txvec.L_length, tx_ref_frame_g.txvec.ht_length);
} /* uu_wlan_phy_txstart_req */


uu_int32 uu_wlan_cp_generate_rtscts(uu_uint8 format, uu_uint8 rtscts_rate, uu_uint8 ch_bndwdth, uu_uint8 modulation, uu_uchar *ra)
{
    uu_uchar mpdu[1000];
    uu_int32 i;
   
    uu_wlan_test_get_frame_from_file(rts_cts_fname, mpdu);
    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t ));
    memset(&uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t)], 0, 20);
    tx_ctl_frame_info_g.txvec.format = format;
    tx_ctl_frame_info_g.txvec.L_datarate = rtscts_rate;

    if(UU_WLAN_SELF_CTS_R)
    {
        for(i=0; i<UU_CTS_FRAME_LEN; i++)
        {
            uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t ) + i] = mpdu[i];
        }

        uu_wlan_fill_crc(0, &uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t )], UU_CTS_FRAME_LEN-4);
        tx_ctl_frame_info_g.frameInfo.framelen=UU_CTS_FRAME_LEN;
    }
    else
    {
        for(i=0; i<UU_RTS_FRAME_LEN; i++)
        {
            uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t ) + i] = mpdu[i];
        }
        uu_wlan_fill_crc(0, &uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t )],  UU_RTS_FRAME_LEN -4);
        tx_ctl_frame_info_g.frameInfo.framelen=UU_RTS_FRAME_LEN;
    }
    printf(" RTS frame is ---- :: \n");
    for(int j=0; j<UU_RTS_FRAME_LEN; j++)
    {
        printf("%x|", uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t )+j]);
    }
    return UU_SUCCESS;
} /* uu_wlan_cp_generate_rtscts */


uu_uint16 uu_calc_rtscts_duration (uu_wlan_tx_vector_t *txvec, uu_uint8 rts_cts_rate, uu_bool cts_self, uu_int32 expected_resp_length)
{
	return 100;
}

uu_int32 uu_wlan_sb_update_tx(uu_wlan_ba_sb_t *sb_p, uu_uint16 scf)
{
	return UU_SUCCESS;
}
/* End of simulations */


typedef struct module_test_case
{
    /* Name of the function under test (FUT), in string form */
    uu_char  *fun_to_be_tested;

    /* Test case input file in string form, for user understanding */
    uu_char  *test_input_file;

    /* Function pointer, to the test driver. Input is test-case-id,
    so that the FUT will be given suitable inputs for this test case */
    uu_void  (*test_driver)(uu_char*);
} module_test_case_t;


uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame)
{
    FILE *fp;
    uu_uchar xdigit=0;
    uu_char file_name1[56]={};
    uu_int32 num=0;
    uu_int32 len=0;
    uu_int32 ch,frame_end=0;
    int l;

    l = strlen(file_name);
    if(file_name[l-1]=='\n')
    {
    	strncpy(file_name1, file_name, strlen(file_name)-1);
        fp=fopen(file_name1 , "r");
    }
    else
    {
        printf("%s and", file_name);
        fp=fopen(file_name , "r");
    }
   
    if(fp == NULL)
    {
        //uu_wlan_test_failure_cnt_g++;
        printf("ERROR: file \"%s\" to get frame doesn't exist\n", file_name);
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
    } /* while */
    fclose(fp);
    printf("length of frame=%d\n", len);
    return len;
} /* uu_wlan_test_get_frame_from_file */


/* Reads the testchains names and handover to testinput function
 * @param[in] file_name is the main script contains list of testchains
 * Based on testchain process it will increment either success/failure count
 */
static uu_void uu_wlan_tester_fun(uu_char *file_name)
{
    FILE *fp;
    uu_int32 ret = UU_SUCCESS;
    uu_char buf[FREAD_BUFF_SIZE + 1];

    fp = fopen(file_name, "r");
    printf("-------------Starting of test case-----------\n");
    if(fp == NULL)
    {
        strcpy(failed_case[uu_wlan_test_failure_cnt_g], file_name);
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: main script opening failed\n");
    }
    else
    {
        /* Reading line by line from file(individual testcase id and parameters) */
        while(fgets(buf, FREAD_BUFF_SIZE, fp))
        {
            if(buf[0] == '#')/* If line from file starting with # then its treated as comment */
            {
                //printf("comment:%s \n",buf);
            }
            else //if(strncmp(buf, "@TEST ", strlen("@TEST ")) == 0)/* Test the given file */
            {
                uu_char  str[FREAD_BUFF_SIZE+1];
                uu_char  filename[FREAD_BUFF_SIZE+1];

                sscanf(buf, "%s %s ", str, filename);
                printf("\nTesting another file %s \n", filename);
                if(strncmp(filename, "tx_testfiles/tx_ps", 18) == 0)
                {
                    uu_wlan_ps_mode_r = 1;
                }
                else
                {
                    uu_wlan_ps_mode_r = 0;
                }
                ret = tester_fun_testinput(filename);
                if(ret == UU_SUCCESS)
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


static uu_int32 tester_fun_testinput(uu_char *file_name)
{
    uu_int32 ret = UU_SUCCESS;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    FILE *fp;

    printf("testinput file for testing is: %s\n", file_name);

    fp = fopen(file_name, "r");

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
                ret = uu_wlan_tx_test_handler(buf);
                if(ret != UU_SUCCESS)
                {
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
    {"UURMI LMAC TX Tester", INPUT_SCRIPT_FILE, uu_wlan_tester_fun},
    {NULL, NULL, NULL}
};


int main(int argc, char *argv[])
{
    uu_wlan_set_default_reg_values();

    uu_lmac_reg_ops(&lmac_ops_g);

    for (int i = 0; uu_wlan_test_cases_g[i].fun_to_be_tested != NULL; i++)
    {
        /* This is for each individual test case id in perticular function(coressponding test_input_file) */
        if (argc == 3)
        {
            if((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested) == 0) &&
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
            printf("ELSE Testing function: %s, case: %s \n",
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
        for(int k=0; k<uu_wlan_test_failure_cnt_g; k++)
        {
            printf("%s\n", failed_case[k]);
        }
    }
    return 1;
} /* main */


static void uu_wlan_set_default_reg_values(void)
{
    /** Setting sta address */
    uu_dot11_sta_mac_addr_r[0] = 0x22;
    uu_dot11_sta_mac_addr_r[1] = 0x22;
    uu_dot11_sta_mac_addr_r[2] = 0x22;
    uu_dot11_sta_mac_addr_r[3] = 0x22;
    uu_dot11_sta_mac_addr_r[4] = 0x22;
    uu_dot11_sta_mac_addr_r[5] = 0x22;
    /** SIFS timer default value is : 16 micro sec */
    dot11_sifs_timer_value = 16;
    /** Short retry and long retry counts */
    /** Tx retry limit defaults from standard Refer IEEE802.11-2007 - Page 874 - 
     * dot11ShortRetryLimit Integer:= 7
     * dot11LongRetryLimit Integer:= 4,
     */
    dot11_short_retry_count = 7;
    dot11_long_retry_count = 4;
    dot11_txop_limit_value[0] = 0;
    dot11_txop_limit_value[1] = 0;
    dot11_txop_limit_value[2] = 3; // Rounded off to 3
    dot11_txop_limit_value[3] = 1; // Rounded off to 1
    dot11_BeaconInterval = 800000; /* TODO: Temp fix - Multiplied by 8 */
    dot11_rts_threshold_value = 1170; //1500;
    uu_dot11_qos_mode_r = 1;

} /* uu_wlan_set_default_reg_values */


int uu_lmac_reg_ops(uu_lmac_ops_t *ops)
{
    if (ops != NULL)
        lmac_ops_gp = ops;
    return 0;
}


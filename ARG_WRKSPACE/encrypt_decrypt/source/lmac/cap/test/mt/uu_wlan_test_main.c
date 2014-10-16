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
#include "uu_wlan_phy_if.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_cap_init.h"
#include "uu_wlan_cap_test_main.h" 

uu_int32 uu_wlan_test_success_cnt_g, uu_wlan_test_failure_cnt_g,failure;
uu_wlan_tx_frame_info_t tx_ctl_frame_info_g;
uu_wlan_tx_frame_info_t *frame_info;
uu_uchar  uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t)+256];
uu_wlan_tx_frame_info_t *tx_ctl_frame_info_gp = (uu_wlan_tx_frame_info_t *)uu_wlan_tx_ctl_frame_info_g;

uu_void uu_wlan_tx_update_status_for_umac(uu_uint8 ac, uu_uint16 seqno, uu_uint8 retries, uu_bool status)
{
}

uu_int32 uu_wlan_handle_txop_tx_start_req(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw)
{
    uu_wlan_cap_event_t                 ev;
    frame_info = uu_wlan_get_msg_ref_from_AC_Q(ac);
    if(frame_info == UU_NULL)
    {
        /* reset tx_ready flag */
        return UU_FAILURE;
    }
    uu_wlan_cap_retain_tx_frame_info(frame_info);
    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)UU_WLAN_CAP_EV_TX_START_CONFIRM, (uu_uchar *)&ev);
    return UU_SUCCESS;
}
uu_int32 uu_wlan_tx_send_more_data(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw)
{
    uu_wlan_cap_event_t                 ev;
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
    //uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)UU_WLAN_CAP_EV_TX_START_CONFIRM, (uu_uchar *)&ev);
    return UU_SUCCESS;
}
uu_void uu_wlan_tx_phy_data_req(uu_char ac)
{
    uu_wlan_cap_event_t                 ev;
    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)UU_WLAN_CAP_EV_TX_DATA_CONFIRM, (uu_uchar *)&ev);
}

typedef struct module_test_case
{
    char  *fun_to_be_tested;  /* Name of the function under test (FUT), in string form */

    char  *test_input_file;      /* Test case input file in string form, for user understanding */
    int   (*test_driver)(char*); /* Function pointer, to the test driver. Input is test-case-id, so that the FUT will be given suitable inputs for this test case */
} module_test_case_t;



uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame)
{
    FILE *fp;
    uu_uchar xdigit=0;
    uu_int32 num=0;
    uu_int32 len=0;
    uu_int32 ch,frame_end=0;
    printf("%s and",file_name);
    fp=fopen(file_name ,"r");
    if(fp == NULL)
    {
        //uu_wlan_test_failure_cnt_g++;
        printf("file can't be opened\n");
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
            printf("Xdigits %x \n", frame[len-1]);
        }
        else
        {

            //sscanf(ch, "%x", &num);
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
    printf("\nlen=%d\n",len);
    return len;
}


int uu_wlan_tester_fun(char *file_name)
{
    FILE *fp;
    uu_int32 rq_type;
    uu_char case_id[FREAD_BUFF_SIZE+1];
    uu_int32 ret = UU_SUCCESS;
    uu_int32 ii, jj;
    uu_char buf[FREAD_BUFF_SIZE + 1];
    uu_char *c;
    char string_type[25][50];
    uu_char  filename[FREAD_BUFF_SIZE+1];

    fp = fopen(file_name,"r");
    printf("file is %s, fp:%p\n",file_name, fp);
    c = (char *)buf;
//    failure=0;
    if(fp == NULL)
    {
        //uu_wlan_test_failure_cnt_g++;
        printf("its at fp\n");
        return UU_FAILURE;

    }

    for(; c !=NULL ;)
    {
        c= fgets(buf , FREAD_BUFF_SIZE, fp);/*here we read line by line from file(individual testcase id and parameters) */
        if(c == NULL)//till the end of file we are reading
        {
            break;
        } 

        if(buf[0] == '#')/*if line from file starting with # then its treated as comment*/
        {
            //printf("comment:%s \n",buf);
        }
        else if(strncmp(buf, "@TEST", strlen("@TEST")) == 0)/* Test the given file */
        {
            uu_char  str[FREAD_BUFF_SIZE+1];

            sscanf(buf,"%s %s ", str, filename);
            printf("\nTesting another file %s \n", filename);
            ret = uu_wlan_tester_fun(filename); 
        }
        else
        {
            sscanf(buf,"%s %s %d ",case_id, string_type[0],&rq_type);
            printf("test_case id %s, and rq_type %d\n", case_id, rq_type);
            ret = uu_wlan_cap_test_handler(rq_type, buf); 
            for(jj = 0; jj < 4; jj++)
            {
                    printf("tx_ready for %d is %d\n",jj,uu_wlan_cap_ctxt_g.ac_cntxt[jj].tx_ready);
            }      
        }  

    }
    if(ret==-1)
    {
        printf("testcase failed\n");
        if(strncmp(file_name,"CAP_testchains/testchain",23)==0||strncmp(file_name,"CAP_testchains/rtestchain",24)==0)
        {
           // uu_wlan_test_failure_cnt_g++;
            failure++;
        }
        else 
        {
            printf("file name is --------------%s\n",file_name);
            while(strncmp(file_name,"CAP_testchains/testchain",23)==0||strncmp(file_name,"CAP_testchains/rtestchain",24)!=0)
            {
                printf("file name is -----------------%s\n",file_name);
                failure++;
                c= fgets(buf , FREAD_BUFF_SIZE, fp);/*here we read line by line from file(individual testcase id and parameters) */
                if(c == NULL)//till the end of file we are reading
                {
                    break;
                }

                if(buf[0] == '#')/*if line from file starting with # then its treated as comment*/
                {
                    //printf("comment:%s \n",buf);
                }
                else if(strncmp(buf, "@TEST", strlen("@TEST")) == 0)/* Test the given file */
                {
                    uu_char  str[FREAD_BUFF_SIZE+1];

                    sscanf(buf,"%s %s ", str, filename);
                    printf("\nTesting another file %s \n", filename);

                /*    uu_char  str[FREAD_BUFF_SIZE+1];
                    sscanf(buf,"%s %s ", str, file_name);
                */    ret = uu_wlan_tester_fun(filename); 
                    printf("file name is %s",file_name);
                }
                else
                {
          //          uu_wlan_test_failure_cnt_g++;
                }
            }
        //        uu_wlan_test_failure_cnt_g++;
    }
}
    else
    {
        printf("failure counts--------%d %d\n",failure,uu_wlan_test_failure_cnt_g); 
        if((strncmp(file_name,"CAP_testchains/testchain",23)==0||strncmp(file_name,"CAP_testchains/rtestchain",24)==0 )&& failure==uu_wlan_test_failure_cnt_g)
        {
#if 1
            for(jj = 0; jj < 4; jj++)
            {
                    printf("tx_ready for %d is %d\n",jj,uu_wlan_cap_ctxt_g.ac_cntxt[jj].tx_ready);
                    int len=uu_wlan_tx_get_available_buffer(jj);
                    if(len!=0)
                    {
                        printf("case is success-------------------\n");
                    }
            }
#endif 
        //    failure++;
            printf("testcase passed\n");
            uu_wlan_test_success_cnt_g++;        
        }  
        else if((strncmp(file_name,"CAP_testchains/testchain",23)==0||strncmp(file_name,"CAP_testchains/rtestchain",24)==0 ))
        {
#if 1
            for(jj = 0; jj < 4; jj++)
            {
                    printf("tx_ready for %d is %d\n",jj,uu_wlan_cap_ctxt_g.ac_cntxt[jj].tx_ready);
                    int len=uu_wlan_tx_get_available_buffer(jj);
                    if(len!=0)
                    {
                        printf("case is success-------------------\n");
                    }
            }
#endif 
            uu_wlan_test_failure_cnt_g++;
        }
    }
    printf("Closing fp:%p\n", fp);
    fclose(fp);
}

module_test_case_t  uu_wlan_test_cases_g[] =
{
    {"UURMI LMAC CP Tester", INPUT_SCRIPT_FILE, uu_wlan_tester_fun},
    {NULL, NULL, NULL}
};


int main(int argc, char *argv[])
{
    int i,ret;
    uu_wlan_cap_init();

    uu_wlan_set_default_reg_values();
    for (i = 0; uu_wlan_test_cases_g[i].fun_to_be_tested != NULL; i++)
    {
        /*this is for each individual test case id in perticular function(coressponding test_input_file)*/
        if (argc == 3)
        {
            if(((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested))==0)&&((strcmp(argv[2], uu_wlan_test_cases_g[i].test_input_file))==0))
            {
                printf("Testing function: %s, case: %s, result: %d\n",
                        uu_wlan_test_cases_g[i].fun_to_be_tested,
                        uu_wlan_test_cases_g[i].test_input_file,
                        ret=uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file));

            }
        }
        /*with this we can run all testcase ids in perticular funtion(test_input_file))
          argumet 2 is function under test.
          argument 1 is exe file
         */

        else if( argc == 2)
        {  
            if((strcmp(argv[1],uu_wlan_test_cases_g[i].fun_to_be_tested))==0)
            {
                printf("Testing function: %s, case: %s, result: %d\n",
                        uu_wlan_test_cases_g[i].fun_to_be_tested,
                        uu_wlan_test_cases_g[i].test_input_file,
                        ret=uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file));

            }
        }
        /*this is case run all function under test files which are maintained in queue.*/
        else
        {
            printf("Testing function: %s, case: %s, return_value %d \n",
                    uu_wlan_test_cases_g[i].fun_to_be_tested,
                    uu_wlan_test_cases_g[i].test_input_file,
                    ret=uu_wlan_test_cases_g[i].test_driver(uu_wlan_test_cases_g[i].test_input_file)
                  );
        }
        printf(" Success cases:  %d \n Failed cases:  %d\n Total no of cases:  %d \n", uu_wlan_test_success_cnt_g,uu_wlan_test_failure_cnt_g, uu_wlan_test_success_cnt_g+uu_wlan_test_failure_cnt_g);
    }
    return 1;
} /* main */


void uu_wlan_set_default_reg_values(void)
{
    /** Setting sta address */
    uu_dot11_sta_mac_addr_r[0] = 0x22;
    uu_dot11_sta_mac_addr_r[1] = 0x22;
    uu_dot11_sta_mac_addr_r[2] = 0x22;
    uu_dot11_sta_mac_addr_r[3] = 0x22;
    uu_dot11_sta_mac_addr_r[4] = 0x22;
    uu_dot11_sta_mac_addr_r[5] = 0x22;
    /** In current implementation we are supporting only OFDM. 
     * The following rates are as for OFDM PHY - 20MHz channel spacing as per specification.
     * Refer Table 18-17-OFDM PHY Characteristics . */

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


    /** EIFS Timeout value */

    /** ACK Timeout value */
    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.2.8 ACK Procedure 
     * ACK timeout interval value is aSIFSTime + aSlotTime + aPHY-RX-START-Delay 
     */
    /** For OFDM PHY aPHY-RX-START-Delay is 25 micro sec for 20MHz channel spacing */
    dot11_aPhyRxStartDelay = 25;

    dot11_ack_timer_value = dot11_slot_timer_value + dot11_aPhyRxStartDelay; // Add dot11_sifs_timer_value 


    /** CTS Timeout value */
    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.2.6 CTS Procedure 
     * CTS timeout interval value is aSIFSTime + aSlotTime + aPHY-RX-START-Delay 
     */
    /** For OFDM PHY aPHY-RX-START-Delay is 25 micro sec for 20MHz channel spacing */

    dot11_cts_timer_value = dot11_slot_timer_value + dot11_aPhyRxStartDelay; // Add dot11_sifs_timer_value

    /** preamble length is 16 micro sec */


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

    return;
} /* uu_wlan_set_default_reg_values */


/* Dummy functions, as CAP handler thread is not included in the test-cde build */
uu_int32  uu_wlan_cap_handler_start(uu_void) {}
uu_void  uu_wlan_cap_handler_stop(uu_void) {}
/* Dummy lock, as LMAC init is not included */
uu_wlan_spin_lock_type_t uu_wlan_cap_Q_lock_g = 1;
uu_void uu_wlan_cp_set_waiting_for_cts() {}
uu_void uu_wlan_cp_set_waiting_for_ack() {}
uu_void uu_wlan_cp_reset_waiting_for_cts() {}
uu_void uu_wlan_cp_reset_waiting_for_ack() {}
/* EOF */


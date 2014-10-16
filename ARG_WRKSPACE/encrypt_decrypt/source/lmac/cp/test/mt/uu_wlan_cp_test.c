
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

#include "uu_datatypes.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_duration.h"
#include "uu_wlan_lmac_sta_info.h"
#include "uu_wlan_cp_rx_proc.h"
#include "uu_wlan_cp_ba_session.h"
#include "uu_wlan_cp_ba_sb.h"
#include "uu_wlan_tx_handler.h"

#include "uu_wlan_test_main.h" 

/* Declared here as these are used in ac_q file */
/* TODO: Need to avoid this, in both CP & Test code, as it is not compatible with RTL design */
uu_uint8  uu_wlan_LongRetryCount[UU_WLAN_AC_MAX_AC];
uu_uint8  uu_wlan_ShortRetryCount[UU_WLAN_AC_MAX_AC];
uu_wlan_ampdu_status_t uu_wlan_tx_ampdu_status_g[UU_WLAN_AC_MAX_AC];

/* Simulation of other modules functionalities starts here */
uu_void uu_wlan_tx_update_status_for_umac(uu_uint8 ac, uu_uint16 seqno, uu_uint8 retries, uu_bool status)
{
}

uu_int32 uu_wlan_cap_init(uu_void)
{
    return UU_SUCCESS;
}

uu_int32 uu_wlan_beacon_timer_start(uu_void)
{
    return UU_SUCCESS;
}

uu_int32 uu_wlan_beacon_timer_stop(uu_void)
{
    return UU_SUCCESS;
}

uu_void uu_wlan_cap_shutdown(uu_void)
{
}

uu_int32 uu_wlan_put_msg_in_CAP_Q(uu_wlan_cap_event_type_t ev_type, uu_uchar *data_p, uu_int32 len)
{
    return UU_SUCCESS;
}

uu_int32 uu_wlan_tx_get_expt_response_frame_length (uu_wlan_tx_frame_info_t  *frame_info)
{
    return 0;
}

uu_int32 uu_wlan_tx_is_ampdu_status_clear(uu_uint8 ac)
{
    return UU_SUCCESS;
}

uu_void uu_wlan_tx_update_mpdu_status_for_reagg(uu_uint16 seq_num, uu_uchar ac, uu_bool status)
{
}
/* End of Simulations */


/** Function to test whether rcvd frame is processed properly or not.
 * Upon rcving a frame, some response frames will be generated such as ack, cts, bar etc.
 * Those generated frame frame_type and RA are verified here.
 * This is called after processing of rcvd frame
 * @param[in] frame_type Indicates the type of response frame generated - ctl, ack, bar etc.
 * @param[in] ta which is txmitter_address of rcvd frame
 */
static uu_int32 _comp_gen_frame_with_rx_frame(uu_uchar fc0, uu_uchar *ta)
{
    uu_int32 ra_valid;
    printf("Generated control response frame is \n");
    for(int i = 0; i < tx_ctl_frame_info_g.frameInfo.framelen; i++)
    {
        printf("%x ", tx_ctl_frame_info_g.mpdu[i]);
    }
    ra_valid = memcmp(&ta, &tx_ctl_frame_info_g.mpdu[UU_RA_OFFSET], 6);
    printf("\n");
    if((tx_ctl_frame_info_g.mpdu[0] == fc0) && ra_valid)
    {
        uu_wlan_test_success_cnt_g++;
        return UU_SUCCESS;
    }
    else
    {
        printf("ERROR: Processing of frame failed ra valid is %d , rx frme is %x\n", ra_valid, fc0);
        uu_wlan_test_failure_cnt_g++;
        return UU_FAILURE;
    }   
} /* _comp_gen_frame_with_rx_frame */


/** Function to test CTS generation
 * This checks the generation of CTS, on receiving RTS.
 * Verification is done on - RA, TA, format and rate in generated cts frame.
 * @param[in] - buf Contains inputs and expected outputs:
 *   i/ps: ac, format, frame and self_cts flag etc.
 *   o/ps: format, rate etc. (ra and ta also verified)
 * return UU_SUCCESS if cts generated properly or UU_FAILURE if fails
 */
static uu_int32 __test_handler_for_cp_tx_path(uu_char *buf)
{
    uu_wlan_tx_frame_info_t tx_frame_info;

    uu_int32 format;
    uu_int32 rate;
    uu_char frame_input_file[50];
    uu_uchar array[6][50];
    uu_int32 post_condition[4];
    uu_int32 return_value;
    uu_int32 ret;
    uu_int32 ra_valid;
    uu_int32 ta_valid;
    uu_uint8 ac;
    uu_int32 self_cts_flag;
    uu_int32 test_category;
    uu_uint32 available_buf;
    uu_uchar whole_buffer[3][300];
    uu_uchar *addrs[3];
    uu_uint32 len[4];

   sscanf(buf,"%s %d %s %d %d %d %d %s %s %s %d %d %s %s %d\n", 
           array[0], &test_category, 
           array[1], &ac, &self_cts_flag, &format, &rate, frame_input_file, 
           array[2], 
           array[3], &post_condition[0], &post_condition[1], 
           array[4], 
           array[5], &return_value);


    memset(&tx_frame_info, 0, sizeof(uu_wlan_tx_frame_info_t));

    tx_frame_info.txvec.format = (uu_uint8)format;
    tx_frame_info.rtscts_rate = (uu_uint8)rate;
    tx_frame_info.ac = (uu_uint8)ac;
    ret = uu_wlan_test_get_frame_from_file(frame_input_file, (uu_uchar *)tx_frame_info.mpdu);
    if(ret <= 0)
    {
        return UU_FAILURE;
    }
    else
    {
        tx_frame_info.frameInfo.framelen = ret;
    }

    UU_WLAN_SELF_CTS_R = (uu_bool)self_cts_flag;
    available_buf = uu_wlan_tx_get_available_buffer((uu_int8)tx_frame_info.ac);
    memcpy(whole_buffer[0], &tx_frame_info, sizeof(uu_wlan_tx_frame_info_t));
    printf("len is %d----------------",tx_frame_info.frameInfo.framelen);
    memcpy((whole_buffer[0] + sizeof(uu_wlan_tx_frame_info_t)), tx_frame_info.mpdu, tx_frame_info.frameInfo.framelen);
    addrs[0] = (uu_uchar *)whole_buffer[0];
    len[0] = sizeof(uu_wlan_tx_frame_info_t) + tx_frame_info.frameInfo.framelen;
    len[1] = 0;
    addrs[1] = NULL;
    printf("after filling addr and lens\n");
    uu_wlan_tx_write_buffer(tx_frame_info.ac, addrs, len);

    printf("tx_frame is \n");
    for(int k = 0; k < tx_frame_info.frameInfo.framelen; k++)
    {
        printf("%x ", tx_frame_info.mpdu[k]); 
    }
    printf("\n mac_addr %x %x \n", UU_WLAN_IEEE80211_STA_MAC_ADDR_R[0], UU_WLAN_IEEE80211_STA_MAC_ADDR_R[5]);

    uu_wlan_lmac_get_sta((const uu_uchar *)UU_WLAN_IEEE80211_STA_MAC_ADDR_R);

    /* Handover to CP */
    ret = uu_wlan_cp_generate_rtscts(tx_frame_info.txvec.format, tx_frame_info.rtscts_rate, tx_frame_info.txvec.ch_bndwdth, tx_frame_info.txvec.modulation, &tx_frame_info.mpdu[UU_RA_OFFSET]);

    printf("tx_ctl_frame is \n");
    for(int k = 0; k < tx_ctl_frame_info_g.frameInfo.framelen; k++)
    {
        printf("%x ", tx_ctl_frame_info_g.mpdu[k]); 
    }
    printf("\n");

    /* Verification of test case here */
    ra_valid = (memcmp(&tx_ctl_frame_info_g.mpdu[UU_RA_OFFSET], &tx_frame_info.mpdu[UU_RA_OFFSET], 6) == 0);
    if(tx_ctl_frame_info_g.mpdu[0] != (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_CTS))
    {
        ta_valid = (memcmp(&tx_ctl_frame_info_g.mpdu[UU_TA_OFFSET], UU_WLAN_IEEE80211_STA_MAC_ADDR_R, 6) == 0);
        printf("ta_valid in rq 0 is %d and ra_valid %d\n", ta_valid, ra_valid);
    }
    else
    {
        ta_valid = 1;
    }
    /* non Ht format */
    if(tx_ctl_frame_info_g.txvec.format == 0)
    {
        rate = tx_ctl_frame_info_g.txvec.L_datarate; 
    }
    else
    {
        rate = tx_ctl_frame_info_g.txvec.mcs; 
    }
    printf("tx_ctl_frame_info format %d rate %d and txvec mac %d and txvec datarate %d\n", 
        tx_ctl_frame_info_g.txvec.format, rate, tx_ctl_frame_info_g.txvec.mcs, 
        tx_ctl_frame_info_g.txvec.L_datarate);

    if((tx_ctl_frame_info_g.txvec.format == (uu_uint8)post_condition[0]) && 
        ((rate == (uu_uint8)post_condition[1])) && (ra_valid) && (ta_valid))
    {
        uu_wlan_test_success_cnt_g++;
        printf("testcase id is %s SUCCESS\n", array[0]);
        ret = UU_SUCCESS;
    }
    else
    {
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: ctl_frame generation failed for testcase id is %s\n", array[0]);
        ret = UU_FAILURE;
    }

    uu_wlan_clear_mpdu((uu_uint8)ac, UU_TRUE);
    printf("return value from rx_proc is %d\n", ret);
    return ret;
} /* __test_handler_for_cp_tx_path */


/** Function for processing of rcvd frame 
 * After processing of rcvd frame, fun. will give some return values like send_resp, ack_rcvd etc.
 * This test function matches the expected & actual values for - return, frame_type and RA.
 * @param[in] buf Contains set of inputs - is_aggragated, format, rate, mcs, modulation, frame etc.
 * reurn UU_SUCCESS on successful processing or UU_FAILURE on failed case, 32 bit in length
 */
static uu_int32 __test_handler_for_cp_rx_proc(uu_char *buf)
{
    uu_uchar frame[1000];
    uu_wlan_rx_frame_info_t *rx_frame_info = (uu_wlan_rx_frame_info_t *)frame;
    uu_int32 format;
    uu_int32 rate;
    uu_char frame_input_file[70];
    uu_uchar array[6][70];
    uu_int32 return_value;
    uu_int32 ret;
    uu_int32 is_aggregated;
    uu_int32 mcs;
    uu_int32 modulation;
    uu_int32 ack_flag;
    uu_int32 cts_flag;
    uu_int32 rx_end_ind;
    uu_uchar FC;
    uu_int32 test_category;

    sscanf(buf,"%s %d %s %d %d %d %d %d %d %d %s %d %s %s %s %s %x\n", 
            array[0], &test_category, 
            array[1], &format, &rate, &is_aggregated, &modulation, &mcs, &ack_flag, &cts_flag, frame_input_file, &rx_end_ind,
            array[2], 
            array[3],  
            array[4], 
            array[5], &return_value);
    printf("---------------frame_input_file is: %s\n-------------", frame_input_file);
    printf("size of frame info is %d\n", sizeof(rx_frame_info));
    memset(rx_frame_info, 0, 1000);

    rx_frame_info->rxvec.format = (uu_uint8)format;
    rx_frame_info->rxvec.L_datarate = (uu_uint8)rate;
    rx_frame_info->rxvec.is_aggregated = (uu_uint8)is_aggregated;
    rx_frame_info->rxvec.modulation = (uu_uint8)modulation;
    rx_frame_info->rxvec.mcs = (uu_uint8)mcs;

    ret = uu_wlan_test_get_frame_from_file(frame_input_file, (uu_uchar *)rx_frame_info->mpdu);
    if(ret <= 0)
    {
        return UU_FAILURE;
    }
    else
    {
        rx_frame_info->frameInfo.framelen = ret;
    }

    uu_wlan_cp_waiting_for_ack_g = ack_flag;
    uu_wlan_cp_waiting_for_cts_g = cts_flag;

    printf("Rx frame of length %d is\n", rx_frame_info->frameInfo.framelen);
    for(int i = 0; i < rx_frame_info->frameInfo.framelen ; i++)
    {
        printf("%x ", rx_frame_info->mpdu[i]);
    }
    printf("\n");
    /* Handover to CP */
    ret = uu_wlan_cp_process_rx_frame(rx_frame_info, (uu_bool)rx_end_ind);


    printf("Processing the Rx frame cp ret value %x, and expected ret value %x\n", ret, return_value);

    if((((rx_frame_info->mpdu[0] & IEEE80211_FC0_TYPE_MASK)== IEEE80211_FC0_TYPE_MGT) 
        || ((rx_frame_info->mpdu[0] & IEEE80211_FC0_TYPE_MASK)== IEEE80211_FC0_TYPE_RESERVED))
        && (rx_frame_info->mpdu[0] != (IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK | IEEE80211_FC0_TYPE_MGT)))
    {
        FC = rx_frame_info->mpdu[0] & IEEE80211_FC0_TYPE_MASK; 
    }
    else
    {
        FC = rx_frame_info->mpdu[0];
    }

    /* Verification of testcase here */
    if(ret == return_value)
    {
        switch (FC)
        {
            case (IEEE80211_FC0_SUBTYPE_RTS | IEEE80211_FC0_TYPE_CTL): /* RTS */
                {
                    printf("input is rts frame\n");
                    return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_CTS), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                }
            case (IEEE80211_FC0_SUBTYPE_CTS | IEEE80211_FC0_TYPE_CTL): /* CTS */
                {   
                    uu_wlan_test_success_cnt_g++;
                    return UU_SUCCESS;
                }
            case (IEEE80211_FC0_SUBTYPE_ACK | IEEE80211_FC0_TYPE_CTL): /* ACK */
                {   
                    uu_wlan_test_success_cnt_g++;
                    return UU_SUCCESS;
                }
            case (IEEE80211_FC0_SUBTYPE_PSPOLL | IEEE80211_FC0_TYPE_CTL): /* PS-POLL */
                {
                    return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_ACK), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                }

            /* We are at Rx side and we are going to generate BA frame */
            case (IEEE80211_FC0_SUBTYPE_BAR | IEEE80211_FC0_TYPE_CTL): /* BAR */
                {
                    /* BA Session is invalid */
                    if(ret == UU_BA_SESSION_INVALID)
                    {
                        uu_wlan_test_success_cnt_g++;
                        printf("Received BAR frame case is ba invalid\n");
                        return UU_SUCCESS;
                    }
                    else if(ret == UU_WLAN_RX_HANDLER_SEND_RESP)
                    {
                        if(UU_WLAN_GET_BAR_ACKPOLICY_VALUE(rx_frame_info->mpdu) ==  UU_WLAN_ACKPOLICY_NORMAL)
                        {
                            return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_ACK), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                        }
                        else
                        {
                            return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_BA), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                        }
                    }
                    else
                    {
                        printf("ERROR: Generation of BA is failed at rx side\n");
                        uu_wlan_test_failure_cnt_g++;
                        return UU_FAILURE;
                    }
                }
            case (IEEE80211_FC0_SUBTYPE_BA | IEEE80211_FC0_TYPE_CTL): /* BA */
                {
                    if(ret == UU_WLAN_RX_HANDLER_FRAME_INVALID)
                    {
                        uu_wlan_test_success_cnt_g++;
                        return UU_SUCCESS;
                    }
                    /* BA Session is invalid */
                    else if(ret == UU_BA_SESSION_INVALID)
                    {
                        uu_wlan_test_success_cnt_g++;
                        printf("Received BA frame case is ba invalid\n");
                        return UU_SUCCESS;
                    }
                    /* Received BA frame is valid */
                    else if(ret == UU_WLAN_RX_HANDLER_BA_RCVD)
                    {
                        uu_wlan_test_success_cnt_g++;
                        printf("Received BA frame case ba ack policy is 1\n");
                        return UU_SUCCESS;
                    }
                    /* Received BA frame and response frame to that */
                    else if(ret == UU_WLAN_RX_HANDLER_SEND_RESP)
                    {
                        return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_ACK), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                    }
                    else
                    {
                        printf("ERROR: BA frame response is failed\n");
                        uu_wlan_test_failure_cnt_g++;
                        return UU_FAILURE;
                    }
                }
            case (IEEE80211_FC0_SUBTYPE_QOS | IEEE80211_FC0_TYPE_DATA): /* QoS Data */
            case (IEEE80211_FC0_SUBTYPE_QOS_NULL | IEEE80211_FC0_TYPE_DATA): /* QoS Data null (no-data) */
                {
                    if(UU_WLAN_GET_QOS_ACKPOLICY_VALUE(rx_frame_info->mpdu) == UU_WLAN_ACKPOLICY_NORMAL)
                    {
                        /* If the packet is a part of A-MPDU */
                        if (is_aggregated)
                        {
                            if (rx_end_ind)
                            {
                                return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_BA), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                            }
                            else /* (!info->is_ampdu_end) */
                            {
                                if(ret == UU_BA_SESSION_INVALID)
                                {
                                    uu_wlan_test_success_cnt_g++;
                                    printf("Received implicit QOS frame with ba invalid rx end indication is 0\n");
                                    return UU_SUCCESS;
                                }
                                else if((ret == UU_SUCCESS) || (ret == UU_FAILURE))
                                {
                                    uu_wlan_test_success_cnt_g++;
                                    printf("Received implicit QOS frame with rx end indication is 0\n");
                                    /* check the updated bitmap */
                                    return UU_SUCCESS;
                                }
                                else
                                {
                                    uu_wlan_test_failure_cnt_g++;
                                    return UU_FAILURE;
                                }
                            }
                        }
                        else /* Non-aggregated frame (immediate ACK) */
                        {
                            return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_ACK), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                        }
                    }
                    else if (UU_WLAN_GET_QOS_ACKPOLICY_VALUE(rx_frame_info->mpdu) == UU_WLAN_ACKPOLICY_NO_ACK)
                    {
                        /*
                         ** It is possible to send A-MPDU, without BA session, if all frames have NO-ACK policy.
                         ** Nothing to do in CP, for frame with ACK policy of NO-ACK.
                         */
                        uu_wlan_test_success_cnt_g++;
                        return UU_SUCCESS;
                    }
                    else if (UU_WLAN_GET_QOS_ACKPOLICY_VALUE(rx_frame_info->mpdu) == UU_WLAN_ACKPOLICY_PSMP_ACK)
                    {
                        /* TODO: Support for Power-Save */
                        uu_wlan_test_success_cnt_g++;
                        return UU_SUCCESS;
                    }
                    else /* if(info->qos_ack_policy == UU_WLAN_ACKPOLICY_BA) */
                    {
                        if(ret == UU_BA_SESSION_INVALID)
                        {
                            uu_wlan_test_success_cnt_g++;
                            printf("Received QOS under ba frame case is ba invalid\n");
                            return UU_SUCCESS;
                        }
                        else if((ret == UU_SUCCESS) || (ret == UU_FAILURE))
                        {
                            uu_wlan_test_success_cnt_g++;
                            printf("Received QOS under ba \n");
                            return UU_SUCCESS;
                            /* check the updated bitmap */
                        }
                        else
                        {
                            uu_wlan_test_failure_cnt_g++;
                            return UU_FAILURE;
                        }
                    }
                }
            case IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_DATA: /* Non-QoS Data */
            case IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_DATA_NULL: /* Non-QoS data null (no-data) */
                {
                    return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_ACK), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                }
            case IEEE80211_FC0_TYPE_MGT:
                {
                    return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_ACK), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                }
            case IEEE80211_FC0_TYPE_RESERVED:
                {
                    return _comp_gen_frame_with_rx_frame((IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_BA), &rx_frame_info->mpdu[UU_TA_OFFSET]);
                }
            case (IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK | IEEE80211_FC0_TYPE_MGT):
                {
                    uu_wlan_test_success_cnt_g++;
                    return UU_SUCCESS;
                }
            default:
                return UU_FAILURE;
        }/*switch*/
    }/* if */
    else
    {
        printf("ERROR: Processing of rcvd frame at cp is failed\n");
        uu_wlan_test_failure_cnt_g++;
        return UU_FAILURE;
    }
    return ret;
} /* __test_handler_for_cp_rx_proc */


/** Function for testing of BA establishment
 * param[in] buf Contains set of inputs: sta-addr, directtion, tid, ba_type etc
 * return UU_SUCCESS if ba succcessfully established or UU_FAILURE if failed to establish
 */
static uu_int32 __test_handler_for_cp_addba_session(uu_char *buf)
{
    uu_wlan_asso_sta_info_t *sta;
    uu_wlan_ba_ses_context_t *ba_table;
    uu_int32 test_category;
    uu_uchar array[6][50];
    uu_int32 return_value;
    uu_int32 ret = UU_FAILURE;
    uu_int32 addr[6];
    uu_int32 buff_size;
    uu_int32 tid;
    uu_int32 ba_type;
    uu_int32 seq_num;
    uu_int32 dir;
    uu_uchar addrr[6];

    sscanf(buf,"%s %d %s %d %d %d %d %d %x %x %x %x %x %x %s %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &buff_size, &tid, &ba_type, &seq_num, &dir, &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5], 
            array[2], 
            array[3], 
            array[4], 
            array[5], &return_value);

    addrr[0] = (uu_uchar)addr[0];
    addrr[1] = (uu_uchar)addr[1];
    addrr[2] = (uu_uchar)addr[2];
    addrr[3] = (uu_uchar)addr[3];
    addrr[4] = (uu_uchar)addr[4];
    addrr[5] = (uu_uchar)addr[5];
    sta = uu_wlan_lmac_get_sta((const uu_uchar *)addrr);
    if(sta==NULL)
    {
        uu_wlan_lmac_config_t cmd_info;
        cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_ADD;
        //cmd_info.cmd_data.asso_add.aid = aid;
        memcpy(cmd_info.cmd_data.asso_add.addr, addrr, 6);
        uu_wlan_lmac_config(&cmd_info);
    }

   // uu_wlan_lmac_get_sta((const uu_uchar *)addrr);

    /* BA session adding & initialization */
    uu_wlan_lmac_add_ba((uu_uint8)tid, (uu_uchar*)addrr, (uu_uint16)buff_size, (uu_uint16)seq_num, (uu_uint8)ba_type, (uu_bool)dir);

    sta = uu_wlan_lmac_get_sta((const uu_uchar *)addrr);
    ba_table=uu_wlan_get_ba_contxt_of_sta((uu_uchar*)addrr, (uu_uint8)tid, (uu_bool)dir);
    /* verification start here */ 
    //if(sta->ba_cnxt[tid]!= NULL)
    if(ba_table!= NULL)
    {
        uu_wlan_test_success_cnt_g++;
        ret = UU_SUCCESS;
    }
    else
    {
        uu_wlan_test_failure_cnt_g++;
        ret = UU_FAILURE;
        printf("ERROR: BA session not established\n");
    }
    return ret;
} /* __test_handler_for_cp_addba_session */


/** Function for testing of BA deletion 
 * param[in] buf Contains set of inputs: sta-addr, directtion, tid, ba_type etc
 * return UU_SUCCESS if ba succcessfully deleted or UU_FAILURE if failed to delete
 */
static uu_int32 __test_handler_for_cp_delba_session(uu_char *buf)
{
    uu_wlan_asso_sta_info_t *sta;
    uu_wlan_ba_ses_context_t *ba_table;

    uu_uchar array[6][50];
    uu_int32 return_value;
    uu_int32 addr[6];
    uu_int32 tid;
    uu_int32 dir;
    uu_uchar addrr[6];
    uu_int32 test_category;
    
    sscanf(buf,"%s %d %s %d %d %x %x %x %x %x %x %s %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &tid, &dir, &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5], 
            array[2], 
            array[3], 
            array[4], 
            array[5], &return_value);

    /*parameters are reading here from file*/
    addrr[0] = (uu_uchar)addr[0];
    addrr[1] = (uu_uchar)addr[1];
    addrr[2] = (uu_uchar)addr[2];
    addrr[3] = (uu_uchar)addr[3];
    addrr[4] = (uu_uchar)addr[4];
    addrr[5] = (uu_uchar)addr[5];

    sta = uu_wlan_lmac_get_sta((const uu_uchar *)addrr);

    /* Freeing the BA session */
    uu_wlan_lmac_del_ba((uu_uint8)tid, (uu_uchar*)addrr, (uu_bool)dir);
    //uu_wlan_lmac_free_ba_sessn(sta->ba_cnxt[(uu_uint8)tid]);

    //sta = uu_wlan_lmac_get_sta((const uu_uchar *)addrr);
    ba_table=uu_wlan_get_ba_contxt_of_sta((uu_uchar*)addrr, (uu_uint8)tid, (uu_bool)dir);
    /* verification start here */ 
    if(sta->ba_cnxt[dir][tid] == UU_WLAN_INVALID_BA_SESSION)
    {

        uu_wlan_test_success_cnt_g++;
        printf("BlockAck session Deleted\n");
        return UU_SUCCESS;
    }
    else
    {
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: BlockAck session Del Failed\n");
        return UU_FAILURE;
    }
} /* __test_handler_for_cp_delba_session */


/** Function for testing score-board updation 
 * It compares with expected_return and actual_return from 'sb_update' function of CP.
 * @param[in] buf Contains set of inputs: sta_addr, scf, tid etc.
 * return UU_SUCCESS on successfull updation of sb or UU_FAILURE if fails
 */
static uu_int32 __test_handler_for_cp_BA_SB_at_Tx(uu_char *buf)
{
    uu_wlan_ba_ses_context_t *ba;
    uu_uchar array[6][50];
    uu_int32 return_value;
    uu_int32 ret = UU_FAILURE;
    uu_int32 addr[6];
    uu_int32 tid;
    uu_uchar addrr[6];
    uu_int32 scf;
    uu_int32 test_category;
    
    sscanf(buf,"%s %d %s %x %x %x %x %x %x %x %d %s %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5], &scf, &tid, 
            array[2], 
            array[3], 
            array[4], 
            array[5], &return_value);

    /*parameters are reading here from file*/
    addrr[0] = (uu_uchar)addr[0];
    addrr[1] = (uu_uchar)addr[1];
    addrr[2] = (uu_uchar)addr[2];
    addrr[3] = (uu_uchar)addr[3];
    addrr[4] = (uu_uchar)addr[4];
    addrr[5] = (uu_uchar)addr[5];

    ba = uu_wlan_get_ba_contxt_of_sta(addrr, (uu_uint8)tid, 1);
    if(ba != NULL)
    {
        ret = uu_wlan_sb_update_tx(&ba->ses.sb, (uu_uint16)scf);
        printf("IN request 4 ret %x return_value %x\n", ret, return_value);
        if (ret == return_value)
        {
            uu_wlan_test_success_cnt_g++;
            printf("Tx side BA score board is updated\n");
            return UU_SUCCESS;
        } 
        else
        {
            uu_wlan_test_failure_cnt_g++;
            printf("ERROR: Tx side BA score board is not updated\n");
            return UU_FAILURE;
        }
    }
    else
    {  
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: BA session is not established for rq %d\n", test_category);
        return UU_FAILURE;   
    }
} /* __test_handler_for_cp_BA_SB_at_Tx */


/** Function for testing generation of BAR frame
 * This function checks the length generated bar frame against the expected length.
 * @param[in] buf Contains set of inputs: sta_addr, scf, tid, bartype etc.
 * return UU_SUCCESS if expected and actual return lengths are same or UU_FAILURE if not matches
 */
static uu_int32 __test_handler_for_gen_BAR_frame(uu_char *buf)
{
    uu_uchar array[6][50];
    uu_int32 return_value;
    uu_int32 ret = UU_FAILURE;
    uu_int32 addr[6];
    uu_int32 tid;
    uu_uchar addrr[6];
    uu_int32 scf;
    uu_int32 test_category;
    uu_int32 duration;
    uu_int32 bartype;
    
    sscanf(buf,"%s %d %s %x %x %x %x %x %x %x %d %x %x %s %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5], &scf, &tid, &duration, &bartype, 
            array[2], 
            array[3], 
            array[4], 
            array[5], &return_value);

    /*parameters are reading here from file*/
    addrr[0] = (uu_uchar)addr[0];
    addrr[1] = (uu_uchar)addr[1];
    addrr[2] = (uu_uchar)addr[2];
    addrr[3] = (uu_uchar)addr[3];
    addrr[4] = (uu_uchar)addr[4];
    addrr[5] = (uu_uchar)addr[5];

    ret = uu_wlan_cp_generate_bar((uu_uint8)bartype, (uu_uint8)tid, (uu_uint16)scf, (uu_uint16)duration, addrr);

    printf("IN request 4 ret %x return_value %x\n", ret, return_value);
    for (int i = 0; i < ret; i++)
    {
        printf(" %x", tx_ctl_frame_info_g.mpdu[i]);
    }
    printf("\n");
    if (ret == return_value)
    {
        uu_wlan_test_success_cnt_g++;
        printf("Generated BAR FRAME is success\n");
        return UU_SUCCESS;
    }
    else
    {  
        uu_wlan_test_failure_cnt_g++;
        printf("ERROR: Generation of BAR is not success for rq %d\n", test_category);
        return UU_FAILURE;   
    }     
} /* __test_handler_for_gen_BAR_frame */


uu_int32 uu_wlan_cp_test_handler(uu_int32 test_category, uu_char *buf)
{
    printf("test_category passed here is: %d\n", test_category);
    /* parameters are reading here from file */
    switch(test_category)
    {
        /* rts-cts genration for tx-side*/
        case TX_PATH:
            {
                return __test_handler_for_cp_tx_path(buf);
            }
            /* processing the rx frame */
        case RX_PATH:
            {
                return __test_handler_for_cp_rx_proc(buf);
            }
            /* add BA session */
        case ADD_BA:
            {
                return __test_handler_for_cp_addba_session(buf);
            }
            /* DELETE BA session */
        case DEL_BA:
            {
                return __test_handler_for_cp_delba_session(buf);
            }
            /* For Updating the score board at Tx side */
        case SB_UPDATE:
            {
                return __test_handler_for_cp_BA_SB_at_Tx(buf);
            }
            /* For BA-response generation */
        case BAR_GEN:
            {
                return __test_handler_for_gen_BAR_frame(buf);
            }
        default:
            {
                printf("ERROR: invalid rq_type for test\n");
                uu_wlan_test_failure_cnt_g++;
                return UU_FAILURE;
            }
    }
} /* uu_wlan_cp_test_handler */


/* EOF */


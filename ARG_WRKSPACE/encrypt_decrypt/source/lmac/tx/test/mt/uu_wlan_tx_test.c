#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>
#endif

#include "uu_errno.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_tx_handler.h"
#include "uu_wlan_tx_test_main.h"

/* Function to write frame to buffer for wrap-around case */
static uu_int32 __fill_frame_for_buff_mgt(uu_int32 test_category_id, char *frame_file, char *frameinfo_file);

/* Functions for verification of outputs based on test_category_ids */
static uu_int32 __verify_output(uu_int32 test_category_id, int seq_count_g, uu_int32 ac);
static uu_int32 __verify_output_for_ps(uu_int32 test_category_id, uu_wlan_tx_frame_info_t *frame_info);

/* Functions to process test_cases based on test_category_id */
static uu_void handle_test_single_frame(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_int32 retry, uu_int32 test_category_id);
static uu_void handle_test_two_frame(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_int32 test_category_id);
static uu_void handle_aggr(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_int32 seq_no[], uu_int32 test_category_id);
static uu_void handle_re_aggr(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_char seq_file[], uu_int32 test_category_id);

/* Maintains seqno.s of input frames */
static uu_int32 ac_seqno[20] = {0};

/* Maintains no.of frames transmitted successfully for every testcase */
static int seq_count_g;

uu_uchar frame_phy[65535];
uu_char rts_cts_fname[100];

/** This fun will get the total count of frames that were txmitted successfully */
uu_void uu_wlan_umac_cbk(uu_int8 ind)
{
    uu_uint32 status;

    printf("UMAC cbk called %x %x %x %x \n", ac_seqno[0], ac_seqno[1], ac_seqno[2], ac_seqno[3]);
    if (ind == UU_LMAC_IND_TX_STATUS)
    {
        for(int i=0; i< UU_WLAN_AC_MAX_AC; i++)
        {
            printf("status flag for ac:%d is %x\n", i, uu_wlan_tx_status_flags_g[i]);
            for(int j=0; j<UU_WLAN_MAX_TX_STATUS_TO_UMAC; j++)
            {
                status = (uu_wlan_tx_status_flags_g[i] & (1 << j));
                if(status)
                {
                    printf("j and seq_count_g value is %d %d\n", j, seq_count_g);
                    printf("seq no is %x and Rcvd seq is %x, status is %x and retries %x \n",
                        ac_seqno[seq_count_g], uu_wlan_tx_status_info_g[i][j].seqno,
                        uu_wlan_tx_status_info_g[i][j].status,
                        uu_wlan_tx_status_info_g[i][j].retry_count);
                    if( ac_seqno[seq_count_g] == uu_wlan_tx_status_info_g[i][j].seqno)
                    {
                        printf("MATCH \n");
                        uu_wlan_tx_status_flags_g[i] &= ~(1 << j);
                        seq_count_g++;
                    }
                    else
                    {
                        printf("ERROR: seq no not matched actual:%d expected:%d\n", uu_wlan_tx_status_info_g[i][j].seqno, ac_seqno[seq_count_g]);
                    }
                }
            }
        }
    }
}/* uu_wlan_umac_cbk */


/* Function to fill tx_frame_info from input file */
uu_void uu_wlan_fill_tx_frame_info(uu_char *frame_input_file, uu_wlan_tx_frame_info_t *frame_info)
{
    uu_char string[100];
    uu_char line[100];
    uu_int32 value;
    FILE* fp = NULL;
    uu_char frame_input_file1[70] = {};
    uu_int32 l = strlen(frame_input_file);

    if(frame_input_file[l-1] == '\n')
    {
        strncpy(frame_input_file1, frame_input_file, strlen(frame_input_file)-1);
        fp = fopen(frame_input_file1, "r");
    }
    else
    {
        fp = fopen(frame_input_file,"r");
    }
    if(fp == NULL)
    {
        printf("ERROR: file \"%s\" for filling frame_info doesn't exist\n", frame_input_file);
        return;
    }

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s = %d", string, &value);

        if(strcmp(string,"format") == 0)
        {
            frame_info->txvec.format = value;
        }
        else if(strcmp(string,"ch_bndwdth") == 0)
        {
            frame_info->txvec.ch_bndwdth = value;
        }
        else if(strcmp(string, "n_tx") == 0)
        {
            frame_info->txvec.n_tx=value;
        }
        else if(strcmp(string,"txpwr_level") == 0)
        {
            frame_info->txvec.txpwr_level = value;
        }
        else if(strcmp(string,"ch_bndwdth") == 0)
        {
            frame_info->txvec.ch_bndwdth = value;
        }
        else if(strcmp(string, "is_long_preamble") == 0)
        {
            frame_info->txvec.is_long_preamble = value;
        }
        else if(strcmp(string,"ht_length") == 0)
        {
            frame_info->txvec.ht_length = value;
        } 
        else if(strcmp(string,"L_datarate") == 0)
        {
            frame_info->txvec.L_datarate = value;
        }
        else if(strcmp(string, "L_length") == 0)
        {
            frame_info->txvec.L_length = value;
        }
        else if(strcmp(string,"modulation") == 0)
        {
            frame_info->txvec.modulation = value;
        }
        else if(strcmp(string,"service") == 0)
        {
            frame_info->txvec.service = value;
        }
        else if(strcmp(string, "is_dyn_bw") == 0)
        {
            frame_info->txvec.is_dyn_bw = value;
        }
        else if(strcmp(string, "indicated_chan_bw") == 0)
        {
            frame_info->txvec.indicated_chan_bw = value;
        }
        else if(strcmp(string, "partial_aid") == 0)
        {
            frame_info->txvec.partial_aid = value;
        }
        else if(strcmp(string, "is_smoothing") == 0)
        {
            frame_info->txvec.is_smoothing = value;
        }
        else if(strcmp(string, "is_sounding") == 0)
        {
            frame_info->txvec.is_sounding = value;
        }
        else if(strcmp(string, "is_aggregated") == 0)
        {
            frame_info->txvec.is_aggregated = value;
        }
        else if(strcmp(string, "is_short_GI") == 0)
        {
            frame_info->txvec.is_short_GI = value;
        }
        else if(strcmp(string,"antenna_set") == 0)
        {
            frame_info->txvec.antenna_set = value;
        }
        else if(strcmp(string, "stbc") == 0)
        {
            frame_info->txvec.stbc = value;
        }
        else if(strcmp(string, "num_ext_ss") == 0)
        {
            frame_info->txvec.num_ext_ss = value;
        }
        else if(strcmp(string,"is_no_sig_extn") == 0)
        {
            frame_info->txvec.is_no_sig_extn = value;
        }
        else if(strcmp(string,"ch_offset") == 0)
        {
            frame_info->txvec.ch_offset = value;
        }
        else if(strcmp(string, "mcs") == 0)
        {
            frame_info->txvec.mcs = value;
        }
        else if(strcmp(string, "is_fec_ldpc_coding") == 0)
        {
            frame_info->txvec.is_fec_ldpc_coding = value;
        }
        else if(strcmp(string, "is_beamformed") == 0)
        {
            frame_info->txvec.is_beamformed = value;
        }
        else if(strcmp(string, "group_id") == 0)
        {
            frame_info->txvec.group_id = value;
        }
        else if(strcmp(string, "is_tx_op_ps_allowed") == 0)
        {
            frame_info->txvec.is_tx_op_ps_allowed = value;
        }
        else if(strcmp(string,"is_time_of_departure_req")==0)
        {
            frame_info->txvec.is_time_of_departure_req = value;
        }
        else if(strcmp(string,"num_users") == 0)
        {
            frame_info->txvec.num_users = value;
        }
        else if(strcmp(string,"framelen") == 0)
        {
            frame_info->frameInfo.framelen = value;
        }
        else if(strcmp(string,"is_ampdu") == 0)
        {
            frame_info->frameInfo.is_ampdu = value;
        }
        else if(strcmp(string,"retry") == 0)
        {
            frame_info->frameInfo.retry = value;
        }
        else if(strcmp(string,"keytype") == 0)
        {
            frame_info->frameInfo.keytype = value;
        }
        else if(strcmp(string,"keyix") == 0)
        {
            frame_info->frameInfo.keyix = value;
        }
        else if(strcmp(string,"ac") == 0)
        {
            frame_info->ac = (uu_uint8)value;
        }
        else if(strcmp(string,"stbc_fb") == 0)
        {
            frame_info->stbc_fb = value;
        }
        else if(strcmp(string,"n_tx_fb") == 0)
        {
            frame_info->n_tx_fb = value;
        }
        else if(strcmp(string,"mcs_fb") == 0)
        {
            frame_info->mcs_fb = value;
        }
        else if(strcmp(string,"fallback_rate") == 0)
        {
            frame_info->fallback_rate = value;
        }
        else if(strcmp(string,"rtscts_rate") == 0)
        {
            frame_info->rtscts_rate = value;
        }
        else if(strcmp(string,"rtscts_rate_fb") == 0)
        {
            frame_info->rtscts_rate_fb = value;
        }
        else if(strcmp(string,"aggr_count") == 0)
        {
            frame_info->aggr_count = value;
        }
        else if(strcmp(string,"user_position") == 0)
        {
            frame_info->txvec.tx_vector_user_params[0].user_position = value;
        }
        else if(strcmp(string,"num_sts") == 0)
        {
            frame_info->txvec.tx_vector_user_params[0].num_sts = value;
        }
        else if(strcmp(string,"is_fec_ldpc_coding") == 0)
        {
            frame_info->txvec.tx_vector_user_params[0].is_fec_ldpc_coding = value;
        }
        else if(strcmp(string,"apep_length") == 0)
        {
            frame_info->txvec.tx_vector_user_params[0].apep_length = value;
        }
        else if(strcmp(string,"vht_mcs") == 0)
        {
            frame_info->txvec.tx_vector_user_params[0].vht_mcs = value;
        }
        else if(strcmp(string, "beacon") == 0)
        {
            frame_info->frameInfo.beacon = value;
        }
        else if(strcmp(string, "tsf_r") == 0)
        {
            sscanf(line, "%s = %Lx", string, &uu_wlan_tsf_r);
            printf("tsf after reading is: %s %Lx\n", string, uu_wlan_tsf_r);
        }
        else
        {
        }
    }
    printf("after filling frame info\n");
    fclose(fp);

}/* _fill_tx_frame_info */


uu_wlan_tx_frame_info_t frame_info;
uu_wlan_tx_frame_info_t *frame_lp;
uu_uint32 len1 = 0;
uu_uchar *addrs[20] = {NULL};
uu_uint32 available_buf, total_frame_length;
uu_uchar whole_buffer[20][300];
uu_uint32 len[20] = {0};


/** Simulation  of umac_buffer filling functionality
 * Writing frame to buffer
 * @param[in] test_category_id (1-6)
 * @param[in] frame_input_file contains frame which has to be written to buffer 
 * @param[in] frame_info_file contains frame_info corresponding to the frame
 * @param[in] i, the ith frame is going to write to buffer, till now i-1 frames are written to buffer
 * returns UU_SUCCESS on successfully writing to buffer, or UU_FAILURE on failed case
*/
static uu_int32 __umac_buffer_simulator(uu_int32 test_category_id, uu_char *frame_input_file, uu_char *frame_info_file, uu_int32 i)
{
    uu_int32 sub_len = 0;
    uu_uchar mpdu[20][200];

    printf("frame input file for umac_buff_simulator %d %s", i, frame_input_file);
    if(i == 0)
    {
        memset(&frame_info, 0, sizeof(uu_wlan_tx_frame_info_t));
        frame_info.frameInfo.framelen = 0;
    }
    uu_wlan_fill_tx_frame_info(frame_info_file, &frame_info);

    sub_len = uu_wlan_test_get_frame_from_file(frame_input_file, mpdu[i]);
    available_buf = uu_wlan_tx_get_available_buffer((uu_int8)frame_info.ac);
    printf("available buffer is %d\n", available_buf);
    if(available_buf != 0)
    {
        total_frame_length += sub_len;
        if(test_category_id == TEST_SINGLE_FRAME || test_category_id == TEST_TWO_FRAME || 
            test_category_id == TEST_SINGLE_FRAME_THRESHOLD || test_category_id == TEST_WRAP_AROUND_MPDU)
        {
            frame_info.frameInfo.framelen = sub_len;
            ac_seqno[i] = (*((uu_uint16 *)(&mpdu[i][22])))>>4;//__get_seqno(mpdu[0]);
            printf("seqno is %x----------\n", ac_seqno[i]);
            memcpy(whole_buffer[i], &frame_info, sizeof(uu_wlan_tx_frame_info_t));
            memcpy((whole_buffer[i] + sizeof(uu_wlan_tx_frame_info_t)), mpdu[i], frame_info.frameInfo.framelen);

            addrs[0] = (uu_uchar *)whole_buffer[i];
            len[0] = sizeof(uu_wlan_tx_frame_info_t) + frame_info.frameInfo.framelen;
            printf("length of frame is %d and ac is %d \n", len[0], frame_info.ac);
            printf("after filling addr and lens\n");
            uu_wlan_tx_write_buffer(frame_info.ac, addrs, len);
        }
        else// if(test_category_id == TEST_THREE_FRAME_AGGR || test_category_id == TEST_RE_AGGR)
        {
            frame_info.ampdu_info.mpdu_info[i].mpdu_len = sub_len;
            frame_info.ampdu_info.mpdu_info[i].pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(sub_len);
            if(i == 0)
            {
                memcpy(whole_buffer[i], &frame_info, sizeof(uu_wlan_tx_frame_info_t));
                memcpy((whole_buffer[i] + sizeof(uu_wlan_tx_frame_info_t)), mpdu[i], sub_len);
                len[i] = sizeof(uu_wlan_tx_frame_info_t) + sub_len + UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(sub_len);
                addrs[i] = (uu_uchar *)whole_buffer[i];
                frame_lp = (uu_wlan_tx_frame_info_t *)addrs[0];
            }
            else
            {
                len[i] = sub_len;
                addrs[i] = (uu_uchar *)mpdu[i];
                frame_lp->ampdu_info.mpdu_info[i].mpdu_len = len[i];
                frame_lp->ampdu_info.mpdu_info[i].pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(sub_len);
                len[i] = len[i]+ UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(sub_len);
            }

            //frame_info.frameInfo.framelen += sub_len;
            ac_seqno[i] = (*((uu_uint16 *)(&mpdu[i][22])))>>4;//__get_seqno(mpdu[0]);
            len1 += len[i];
            /* Testing is done max. of 3 frames */
            if(i == 2 && available_buf >= len1)
            {
                printf("before writing buffer ac is %d \n",frame_info.ac);
                uu_wlan_tx_write_buffer(frame_info.ac, addrs, len);
                return UU_SUCCESS;
            }
        }
    }
    printf("eof umac_buffer_simulator-------------\n");
    return UU_SUCCESS;
}/* __umac_buffer_simulator */


/* Clearing of buffers which is done in CAP module */
static uu_void __clear_mpdu_or_ampdu(uu_uint8 ac)
{
    static int n_case = 0;
    uu_wlan_tx_frame_info_t *frame_info;
    frame_info = uu_wlan_tx_get_frame_info(ac);
    while (frame_info)
    {
        if (frame_info->txvec.is_aggregated)
        {
            printf("clearing ampdu frame ========\n");
            uu_wlan_clear_ampdu((uu_uint8) ac);
        }
        else
        {
            if(n_case == 0)
            {
                uu_wlan_clear_mpdu((uu_uint8) ac,UU_FALSE);
                n_case ++;
            }
            else
                uu_wlan_clear_mpdu((uu_uint8) ac,UU_TRUE);
        }
        frame_info = uu_wlan_tx_get_frame_info(ac);
    }
} /* __clear_mpdu_or_ampdu */


uu_int32 uu_wlan_tx_test_handler(uu_char *buf)
{
    uu_int32 ret = UU_SUCCESS;
    uu_int32 ret_val;
    uu_int32 nav, duration, bw = 0;
    uu_int32 qos, retry;
    uu_int32 ac = 0;
    uu_int32 id, test_category_id = 0;
    char frameinfo_file_array[5][100], seq_file[65];
    char frame_input_file_array[5][100];
    uu_uchar mpdu[1000];
    uu_wlan_tx_frame_info_t *frame_info;
    seq_count_g=0;
    int seqno[3];

    sscanf(buf, "testcaseid: %d testcategory: %d", &id, &test_category_id);

    switch(test_category_id)
    {
        case TEST_SINGLE_FRAME:
        case TEST_PS_POLL:
        case TEST_QOS_NULL_DATA:
            {
                sscanf(buf, "testcaseid: %d testcategory: %d input: retry: %d "
                    "frame_info: %s frame: %s nav: %d duration: %d qos: %d output: %d",
                    &id, &test_category_id, &retry, frameinfo_file_array[0], 
                    frame_input_file_array[0], &nav, &duration, &qos, &ret_val);
                dot11_rts_threshold_value = 1170; //1500;
                __umac_buffer_simulator(test_category_id, frame_input_file_array[0], frameinfo_file_array[0], 0);
                uu_wlan_test_get_frame_from_file(frame_input_file_array[0], mpdu);
                if(test_category_id == TEST_QOS_NULL_DATA)
                {
                    uu_wlan_lmac_qos_null_tx_handling((mpdu + UU_TA_OFFSET) , 0 );
                    frame_info = ps_trigger_frame_info_g;
                }
                else if(test_category_id == TEST_PS_POLL)
                {
                    uu_wlan_lmac_pspoll_tx_handling((mpdu + UU_TA_OFFSET) , 1 );
                    frame_info = ps_poll_frame_info_g;
                }
                else
                {
                    frame_info = UU_NULL;
                }
            }
            break;

        case TEST_TWO_FRAME:
            {
                sscanf(buf, "testcaseid: %d testcategory: %d input: frame_info: %s frame: %s " 
                    "frame_info: %s frame: %s rts_cts_frame: %s "
                    "nav: %d duration: %d qos: %d output: %d",
                    &id, &test_category_id, frameinfo_file_array[0], frame_input_file_array[0],
                    frameinfo_file_array[1], frame_input_file_array[1], rts_cts_fname,
                    &nav, &duration, &qos, &ret_val);

                dot11_rts_threshold_value = 60; //1500;

                __umac_buffer_simulator(test_category_id, frame_input_file_array[0], frameinfo_file_array[0], 0);
                __umac_buffer_simulator(test_category_id, frame_input_file_array[1], frameinfo_file_array[1], 1);
            }
            break;

        case TEST_THREE_FRAME_AGGR:
            {
                sscanf(buf, "testcaseid: %d testcategory: %d input: seqnos: %x %x %x frame_info: %s "
                    "frame: %s frame: %s frame: %s "
                    "nav: %d duration: %d qos: %d output: %d",
                    &id, &test_category_id, &seqno[0], &seqno[1], &seqno[2], frameinfo_file_array[0],
                    frame_input_file_array[0], frame_input_file_array[1], frame_input_file_array[2],
                    &nav, &duration, &qos, &ret_val);

                dot11_rts_threshold_value = 1170; //1500;

                __umac_buffer_simulator(test_category_id, frame_input_file_array[0], frameinfo_file_array[0], 0);
                __umac_buffer_simulator(test_category_id, frame_input_file_array[1], frameinfo_file_array[0], 1);
                __umac_buffer_simulator(test_category_id, frame_input_file_array[2], frameinfo_file_array[0], 2);
            }
            break;

        case TEST_SINGLE_FRAME_THRESHOLD:
            {
                sscanf(buf, "testcaseid: %d testcategory: %d input: frame_info: %s "
                    "frame: %s rts_cts_frame: %s nav: %d duration: %d qos: %d output: %d",
                    &id, &test_category_id, frameinfo_file_array[0], 
                    frame_input_file_array[0], rts_cts_fname, &nav, &duration, &qos, &ret_val);

                dot11_rts_threshold_value = 10; //1500;

                __umac_buffer_simulator(test_category_id, frame_input_file_array[0], frameinfo_file_array[0], 0);
            }
            break;

        case TEST_RE_AGGR:
            {
                sscanf(buf, "testcaseid: %d testcategory: %d input: seqnos: %s frame_info: %s "
                "frame: %s frame: %s frame: %s "
                "nav: %d duration: %d qos: %d output: %d",
                &id, &test_category_id, seq_file, frameinfo_file_array[0], 
                frame_input_file_array[0], frame_input_file_array[1], frame_input_file_array[2],
                &nav, &duration, &qos, &ret_val);

                dot11_rts_threshold_value = 1170; //1500;

                __umac_buffer_simulator(test_category_id, frame_input_file_array[0], frameinfo_file_array[0], 0);
                __umac_buffer_simulator(test_category_id, frame_input_file_array[1], frameinfo_file_array[0], 1);
                __umac_buffer_simulator(test_category_id, frame_input_file_array[2], frameinfo_file_array[0], 2);
            }
            break;

        case TEST_WRAP_AROUND_MPDU:
            {
                sscanf(buf, "testcaseid: %d testcategory: %d input: frame_info: %s "
                    "frame: %s frame_info: %s frame: %s "
                    "nav: %d duration: %d qos: %d output: %d",
                    &id, &test_category_id, frameinfo_file_array[0], 
                    frame_input_file_array[0], frameinfo_file_array[1], frame_input_file_array[1],
                    &nav, &duration, &qos, &ret_val);

                __fill_frame_for_buff_mgt(test_category_id, frame_input_file_array[0], frameinfo_file_array[0]);
            }
            break;

        default:
            {
                printf("invalid test category\n");
                return UU_FAILURE;
            }
    }

    uu_dot11_qos_mode_r = qos;
    if(uu_wlan_handle_txop_tx_start_req((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
    {
        if(test_category_id == TEST_SINGLE_FRAME)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);
            handle_test_single_frame((uu_uint8)ac, (uu_int32)nav, (uu_wlan_ch_bndwdth_type_t)bw, (uu_int32)retry, (uu_int32)test_category_id);
        }

        else if(test_category_id == TEST_TWO_FRAME || test_category_id == TEST_SINGLE_FRAME_THRESHOLD)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);
            handle_test_two_frame((uu_uint8)ac, (uu_int32)nav, (uu_wlan_ch_bndwdth_type_t)bw, (uu_int32)test_category_id);
        }
        else if(test_category_id == TEST_THREE_FRAME_AGGR)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);
            handle_aggr((uu_uint8)ac, (uu_int32)nav, (uu_wlan_ch_bndwdth_type_t)bw, seqno, (uu_int32)test_category_id);
        }
        /* Verification of power-save cases */
        else if(test_category_id == TEST_PS_POLL || test_category_id == TEST_QOS_NULL_DATA)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);

            /* Clearing frame_phy after receiving at phy */
            for(int i = 0; i<=65535; i++)
                frame_phy[i] = 0;
            return __verify_output_for_ps(test_category_id, frame_info);
        }
        else if(test_category_id == TEST_RE_AGGR)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);
            handle_re_aggr((uu_uint8)ac, (uu_int32)nav, (uu_wlan_ch_bndwdth_type_t)bw, seq_file, (uu_int32)test_category_id);
        }
        else if(test_category_id == TEST_WRAP_AROUND_MPDU)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);
            uu_wlan_clear_mpdu(ac, UU_TRUE);

            /*giving trigger to two more frames*/
            for(int j = 0; j <= 1; j++)
            {
                uu_wlan_tx_phy_data_req((uu_uint8)ac);
                printf("txop_tx_start req returns success\n");
                uu_wlan_clear_mpdu(ac, UU_TRUE);
            }

            /* Filling frames in buffer to test wrap-around case */
            __fill_frame_for_buff_mgt(test_category_id, frame_input_file_array[1], frameinfo_file_array[1]);
        }
        else
        {
            printf("Invalid test case category\n");
            ret =  UU_FAILURE;
        }
    }

    __clear_mpdu_or_ampdu(ac);

    ret = __verify_output(test_category_id, seq_count_g, ac);
    /* clearing frame_phy after receiving at phy */
    for(int i = 0; i<=65535; i++)
        frame_phy[i] = 0;
    return ret;
} /* uu_wlan_tx_test_handler */


/** Verification of testcases output
 * Verification is done on seq_nos, timestamps etc
 * @param[in] test_category_id, 1-6 i.e except for ps_cases
 * @param[in] seq_count_g no.of frames successfully txmitted
 * @param[in] ac
 * returns UU_SUCCESS if expected and actual tsf same and also seq_count_gs, or UU_FAILURE in other cases 
 */
/* TODO: Add verification of frame_type and length */
static uu_int32 __verify_output(uu_int32 test_category_id, int seq_count_g, uu_int32 ac)
{
    uu_uint64 calculated_timestamp = 0;

    printf("seq_count_g for verification is %d\n", seq_count_g);

    /* Getting of rcvd time stamp for verification */
    if(frame_info.frameInfo.beacon)
    {
        for (int i = IEEE80211_IS_BEACON_TIMESTAMP; i < IEEE80211_IS_BEACON_TIMESTAMP + 8; i++)
        {
            printf("mpdu for getting time stamp is: %x\n", frame_phy[i]);
            calculated_timestamp = (calculated_timestamp << 8) | frame_phy[i];
            printf("time stamp calculated after every beacon mpdu is: %Lx\n", calculated_timestamp);
        }
    }

    if((test_category_id == TEST_SINGLE_FRAME) || (test_category_id == TEST_SINGLE_FRAME_THRESHOLD))
    {
        if((seq_count_g==1) && (calculated_timestamp == uu_wlan_tsf_r))
        {
            return UU_SUCCESS;
        }
        else
        {
            printf("ERROR: timestamps are: actual:%Lx expected:%Lx\n", calculated_timestamp, uu_wlan_tsf_r);
            return UU_FAILURE;
        }
    }
    else if(test_category_id == TEST_TWO_FRAME)
    {
        printf("seq_count_g for category 2 is %d\n", seq_count_g);
        if(seq_count_g==2)
        {
            return UU_SUCCESS;
        }
        else
        {
            return UU_FAILURE;
        }
    }
    else if(test_category_id == TEST_WRAP_AROUND_MPDU)
    {
        if(total_frame_length > 15360)
        {
            return UU_SUCCESS;
        }
        else
        {
            return UU_FAILURE;
        }
    }

    else
    {
        if(seq_count_g == TEST_THREE_FRAME_AGGR)
        {
            return UU_SUCCESS;
        }
        else
        {
            return UU_FAILURE;
        }
    }

}/* verification of output */


/** This function is for output verification of ps_cases
 * It verifies length of generated frame, For qos_null it is 30 and for ps_poll 20
 * @param[in] test_category_id(7 0r 8 i.e PS_POLL or QOS_NULL)
 * @param[in] frame_info i.e generated frame either ps_poll or qos_null frame
 * returns UU_SUCCESS if length matches or UU_FAILURE if not matched
 */
static uu_int32 __verify_output_for_ps(uu_int32 test_category_id, uu_wlan_tx_frame_info_t *frame_info)
{
    printf("test_category_id for ver is: %d\n", test_category_id);
    if(test_category_id == TEST_PS_POLL)
    {
        if(frame_info->frameInfo.framelen == 20)
            return UU_SUCCESS;
    }
    else
    {
        if(frame_info->frameInfo.framelen == 30)
            return UU_SUCCESS;
    }
    return UU_FAILURE;
}/* Verification of output for ps cases */


/** Function to write frames to buffer (internally umac buff_simulation function will be called)
 * In this function multiple frames are written to buffer till buffer filled
 * @param[in] test_category_id i.e 6(for testing wrap-around case)
 * @param[in] frame_file contains list of files which internally contains frames which has to be written to buffer
 * @param[in] farmeinfo_file contains list of files which internally contains frame_infos corresponding to frames
 * return UU_FAILURE if any of the files not existed, or UU_SUCCESS in other cases
 */
static uu_int32 __fill_frame_for_buff_mgt(uu_int32 test_category_id, char *frame_file, char *frameinfo_file)
{
    FILE *fp, *fp1;
    int l, n;
    char line[100][200], line2[100][200];
    int i = 0;
    uu_char *frame_input_file_buf[20] = {NULL}, *frame_info_file[20] = {NULL};

    fp = fopen(frame_file, "r");

    if(fp == NULL)
    {
        printf("\n fopen() Error!!!\n");
        return UU_FAILURE;
    }
    else
    {
        while (fgets(line[i], sizeof(line[i]), fp))
        {

            printf("value of i is: %d\n", i);
            //sscanf(line[i], "%s", frame_input_file_buf1[j]);
            frame_input_file_buf[i] = line[i];
            printf("length of frame input file is %d\n", strlen(frame_input_file_buf[i]));
            printf("frame input file for category 6 1 %s \n", frame_input_file_buf[i]);
            i++;
        }
    }
    fclose(fp);
    fp1 = fopen(frameinfo_file, "r");
    l = 0;

    if(fp1 == NULL)
    {
        printf("\n fopen() Error!!!\n");
        return UU_FAILURE;
    }
    else
    {
        while (fgets(line2[l], sizeof(line2[l]), fp1))
        {
            frame_info_file[l] = line2[l];
            l++;
            printf("frame info file for category 6  %s \n", frame_info_file[i]);
        }
        printf("starting frame_input_file is %s\n", frame_input_file_buf[0]);
    }
    fclose(fp1);
    for(n = 0; n < i; n++)
    {
        printf("filling frame %d %s---------\n", n, frame_input_file_buf[n]);
        __umac_buffer_simulator(test_category_id, frame_input_file_buf[n], frame_info_file[n], n);
    }
    return UU_SUCCESS;
} /* __fill_frame_for_buff_mgt */


/** This function is for txmisssion of single frame 
 * after one time txmission over it will be called
 * if retry bit is given in testcase again retxm will be done
 * @param[in] ac
 * @param[in] nav
 * @param[in] bandwidth 
 * @param[in] retry either 1 or 0 
 * @param[in] testcategory_id is 1 
 */
static uu_void handle_test_single_frame(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_int32 retry, uu_int32 test_category_id)
{
    uu_wlan_tx_frame_info_t *frame_info;
    if(retry)
    {
        frame_info=uu_wlan_tx_get_frame_info(ac);
        frame_info->frameInfo.retry=1;
        if(uu_wlan_handle_txop_tx_start_req((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);
        }
    }
}

/** This function is for txmisssion of second frame 
 * after one frame txmitted it will be called
 * @param[in] ac
 * @param[in] nav
 * @param[in] bandwidth 
 * @param[in] testcategory_id is 2 
 */
static uu_void handle_test_two_frame(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_int32 test_category_id)
{
    if(uu_wlan_tx_send_more_data((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
    {
        printf("tx send more data test_category_id is: %d\n", test_category_id);
        uu_wlan_tx_phy_data_req((uu_uint8)ac);
    }
}

/** This function is for aggregation of frames 
 * After one time txmission of aggregated frame is over it will be called
 * If any frame/frames of aggregated packet failed in txmission then again reaggr will be performed on failed frames 
 * @param[in] ac
 * @param[in] nav
 * @param[in] bandwidth 
 * @param[in] seq_no[0-2] either -1 or seq_nos of frames, if -1 then no need to reaggregate 
 * @param[in] testcategory_id is 3 
 */
static uu_void handle_aggr(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_int32 seq_no[], uu_int32 test_category_id)
{
    uu_wlan_tx_frame_info_t *frame_info;
    if(uu_wlan_tx_send_more_data((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
    {
        printf("tx send more data test_category_id is: %d\n", test_category_id);
        uu_wlan_tx_phy_data_req((uu_uint8)ac);
    }
    for(int i=0;i<=2;i++)
    {
        if(seq_no[i]!=-1)
        {
            uu_wlan_tx_update_mpdu_status_for_reagg((uu_uint16)seq_no[i], ac, UU_TRUE);
        }
    }
    if(uu_wlan_tx_ampdu_status_g[ac].bitmap!=0)
    {
        frame_info=uu_wlan_tx_get_frame_info(ac);
        frame_info->frameInfo.retry=1;
        if(uu_wlan_handle_txop_tx_start_req((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
        {
            uu_wlan_tx_phy_data_req((uu_uint8)ac);
            if(uu_wlan_tx_send_more_data((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
            {
                printf("tx send more data and test category is is: %d\n", test_category_id);
                uu_wlan_tx_phy_data_req((uu_uint8)ac);
            }
        }
    }
} //handle_aggr


/** This function is for multiple times txmission of aggregated frame 
 * after one time txmission of aggregated packet is over it will be called (this function is for multtiple times reaggr)
 * if any frame/frames failed in txmission then again reaggr will be performed on failed frames
 * @param[in] ac
 * @param[in] nav
 * @param[in] bandwidth 
 * @param[in] seq_file contains lines of data either -1 or seq_nos  
 * @param[in] testcategory_id is 5 
 */
static uu_void handle_re_aggr(uu_uint8 ac, uu_int32 nav, uu_wlan_ch_bndwdth_type_t bw, uu_char seq_file[], uu_int32 test_category_id)
{
    FILE *fp;
    char line1[100];
    uu_int32 seqno[3];
    uu_wlan_tx_frame_info_t *frame_info;

    fp = fopen(seq_file,"r");
    if(fp == NULL)
    {
        //uu_wlan_test_failure_cnt_g++;
        printf("ERROR: seqfile to do reaggr doen't exist: %s\n", seq_file);
    }
    else
    {
        uu_wlan_tx_phy_data_req((uu_uint8)ac);
        while (fgets(line1, sizeof(line1), fp))
        {
            sscanf(line1,"%x %x %x", &seqno[0], &seqno[1], &seqno[2]);
            for(int i=0; i<=2; i++)
            {
                if(seqno[i]!=-1)
                {
                    uu_wlan_tx_update_mpdu_status_for_reagg((uu_uint16)seqno[i],ac,UU_TRUE);
                }
            }
            if(uu_wlan_tx_ampdu_status_g[ac].bitmap!=0)
            {
                frame_info=uu_wlan_tx_get_frame_info(ac);
                frame_info->frameInfo.retry=1;
                if(uu_wlan_handle_txop_tx_start_req((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
                {
                    uu_wlan_tx_phy_data_req((uu_uint8)ac);
                    if(uu_wlan_tx_send_more_data((uu_uint8)ac,(uu_int32)nav,(uu_wlan_ch_bndwdth_type_t)bw)==UU_SUCCESS)
                    {
                        printf("tx send more data and test category is: %d\n", test_category_id);
                        uu_wlan_tx_phy_data_req((uu_uint8)ac);
                    }
                }
            }
        }
    }
    fclose(fp);
} //handle_re_aggr


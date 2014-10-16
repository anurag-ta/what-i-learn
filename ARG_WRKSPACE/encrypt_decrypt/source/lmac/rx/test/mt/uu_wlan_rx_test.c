#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>
#endif

#include "uu_errno.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_rx_test_main.h"

/** rx_vec contains the frame_info of rcvd frame
 * At RX_START_IND it is filled and same will be carried for RX_DATA_IND
*/
uu_wlan_rx_vector_t rx_vec;

/* rx_status_ret_g is either 0 or 1. Used to test rx_status */
static uu_bool rx_status_ret_g;

uu_int32 rx_end_status;
uu_uint64 expected_tsf_g;
uu_bool ps_enable_g;
uu_bool dtim_bcn_g;
uu_bool eosp_g;
uu_bool more_data_g;

/* Actual flags returned after execution of testcase */
uu_bool uu_wlan_lmac_ps_enabled_g;
uu_bool uu_wlan_lmac_ps_more_data_g;
uu_bool uu_wlan_lmac_eosp_g;
uu_bool uu_wlan_lmac_dtim_bcn_received_g;

/* lengths of individual mpdus */
static uu_int32 expected_len_in_rx_status_g[32]={0};


/** Function for calculation of lengths of individuals of ampdu and also mpdu
 * @param[in] mpdu contains frame that was rcvd may be aggregated/non-aggregated
*/
static uu_void uu_wlan_get_lengths_mpdus(uu_uchar *mpdu);


uu_void uu_wlan_umac_cbk(uu_int8 dummy)
{
    uu_int32 seqno, status, j=0;
    uu_uchar *mpdu, *addr;

    /* Maintains index of rcvd frame*/
    static int rx_frame_status_index_g = 0;

    uu_wlan_rx_frame_info_t *frame_info = uu_wlan_rx_get_frame_info();
    
    for(j=0; j<UU_WLAN_MAX_RX_STATUS_TO_UMAC; j++)
    {
        status = (uu_wlan_rx_status_flags_g & (1 << rx_frame_status_index_g));
        if(status)
        {
            if(rx_frame_status_index_g > UU_WLAN_MAX_RX_STATUS_TO_UMAC_MASK)
            {
                rx_frame_status_index_g = 0;
            }
            printf("Flag %d is set and reading address and length %d %d\n", 
                j, uu_wlan_rx_status_info_g[j].len, uu_wlan_rx_status_info_g[j].offset);
            addr = uu_wlan_rx_buff_base_addr_reg_g + uu_wlan_rx_status_info_g[j].offset;

#ifdef UU_WLAN_TSF
            expected_len_in_rx_status_g[j] += sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t) + 
                sizeof(frame_info->timestamp_msb) + sizeof(frame_info->timestamp_lsb) + 
                sizeof(frame_info->becn_timestamp_msb_tsf) + sizeof(frame_info->becn_timestamp_lsb_tsf);
            mpdu = addr + sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t) + 
                sizeof(frame_info->timestamp_msb) + sizeof(frame_info->timestamp_lsb) + 
                sizeof(frame_info->becn_timestamp_msb_tsf) + sizeof(frame_info->becn_timestamp_lsb_tsf);
#else            
            expected_len_in_rx_status_g[j] += sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t);
            mpdu = addr + sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t);
#endif
            seqno = (*((uu_uint16 *)(&mpdu[22])))>>4;

            printf("lengths passed for verification are: len:%d, j:%d, rx_status_len:%d, rx_frame_status_index_g:%d\n",
                expected_len_in_rx_status_g[j], j, uu_wlan_rx_status_info_g[rx_frame_status_index_g].len, rx_frame_status_index_g);

            if(expected_len_in_rx_status_g[j] == uu_wlan_rx_status_info_g[rx_frame_status_index_g].len)
            {
                /* TODO: It is better to test addr also but to test addr it uses macrocs which are defined in uu_wlan_ac_q.c*/
                rx_status_ret_g = 1;
            }
            else
            {
                rx_status_ret_g = 0;
                break;
            }
            rx_frame_status_index_g++;
            uu_wlan_rx_status_flags_g = 1;
            printf("seqno rcvd and j value: %d %d", seqno, j);
        }
    } /* for */

    memset (expected_len_in_rx_status_g, 0, sizeof(expected_len_in_rx_status_g));
} /* uu_wlan_umac_cbk */


/** Function to get rcvd frame info
 * this reads the frame info from input_file and storing in rx_vec
 * @param[in] frame_input_file which contains info of frame in the format of string = value
 * returns UU_FAILURE if file not existed, UU_SUCCESS in other cases
 */
static uu_int32 uu_wlan_rx_get_rx_vec_info(char *frame_input_file)
{
    uu_char string[100];
    uu_char line[100];
    uu_int32 value;
    FILE* fp;

    fp = fopen(frame_input_file,"r");

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file \"%s\" for filling rx frame_info", frame_input_file);
        return UU_FAILURE; 
    }

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s = %d", string, &value); 

        if(strcmp(string,"format") == 0)
        {
            rx_vec.format = value;
        }   
        else if(strcmp(string, "modulation") == 0)
        {
            rx_vec.modulation = value;
        }
        else if(strcmp(string, "is_long_preamble") == 0)
        {
            rx_vec.is_long_preamble = value;
        }
        else if(strcmp(string, "is_L_sigvalid") == 0)
        {
            rx_vec.is_L_sigvalid = value;
        }
        else if(strcmp(string, "L_datarate") == 0)
        {
            rx_vec.L_datarate = value;
        }
        else if(strcmp(string, "L_length") == 0)
        {
            rx_vec.L_length = value;
        }
        else if(strcmp(string,"ch_bndwdth")==0)
        {
            rx_vec.ch_bndwdth = value;
        }
        else if(strcmp(string, "is_dyn_bw") == 0)
        {
            rx_vec.is_dyn_bw = value;
        }
        else if(strcmp(string, "indicated_chan_bw") == 0)
        {
            rx_vec.indicated_chan_bw = value;
        }
        else if(strcmp(string, "rssi") == 0)
        {
            rx_vec.rssi = value;
        }
        else if(strcmp(string, "rcpi") == 0)
        {
            rx_vec.rcpi = value;
        }
        else if(strcmp(string, "partial_aid") == 0)
        {
            rx_vec.partial_aid = value;
        }
        else if(strcmp(string, "is_smoothing") == 0)
        {
            rx_vec.is_smoothing = value;
        }
        else if(strcmp(string, "is_sounding") == 0)
        {
            rx_vec.is_sounding = value;
        }
        else if(strcmp(string, "is_aggregated") == 0)
        {
            rx_vec.is_aggregated = value;
        }
        else if(strcmp(string, "is_short_GI") == 0)
        {
            rx_vec.is_short_GI = value;
        }
        else if(strcmp(string, "stbc") == 0)
        {
            rx_vec.stbc = value;
        }
        else if(strcmp(string, "num_ext_ss") == 0)
        {
            rx_vec.num_ext_ss = value;
        }
        else if(strcmp(string, "mcs") == 0)
        {
            rx_vec.mcs = value;
        }
        else if(strcmp(string, "is_fec_ldpc_coding") == 0)
        {
            rx_vec.is_fec_ldpc_coding = value;
        }
        else if(strcmp(string, "rx_start_of_frame_offset") == 0)
        {
            rx_vec.rx_start_of_frame_offset = value;
        }
        else if(strcmp(string, "rec_mcs") == 0)
        {
            rx_vec.rec_mcs = value;
        }
        else if(strcmp(string, "psdu_length") == 0)
        {
            rx_vec.psdu_length = value;
        }
        else if(strcmp(string, "num_sts") == 0)
        {
            rx_vec.num_sts = value;
        }
        else if(strcmp(string, "is_beamformed") == 0)
        {
            rx_vec.is_beamformed = value;
        }
        else if(strcmp(string, "group_id") == 0)
        {
            rx_vec.group_id = value;
        }
        else if(strcmp(string, "is_tx_op_ps_allowed") == 0)
        {
            rx_vec.is_tx_op_ps_allowed = value;
        }
        else if(strcmp(string, "snr") == 0)
        {
            rx_vec.snr = value;
        }

        /*sta timestamp reading from input file */
        else if(strcmp(string, "tsf_r") == 0)
        {
            sscanf(line, "%s = %Lx", string, &uu_wlan_tsf_r); 
        }
        else
        {
            printf("wrong index %s \n", string);
        }
    } /* while */
    fclose(fp);
    return UU_SUCCESS;
} /* uu_wlan_rx_get_rx_vec_info */


uu_int32 uu_wlan_rx_test_handler(uu_int32 rq_type, uu_char *buf)
{
    uu_int32 i, ret_val, event_type, id, rets, filter, ret;
    uu_uchar mpdu[1000];
    uu_char frame_input_file[60],string[5][60];

    /* If UMAC is not interested in this frame,
       then by default test will consider rx_status is filled properly */
    rx_status_ret_g = 1;

    switch(rq_type)
    {
        case UU_WLAN_CAP_EV_RX_START_IND:
            {
                /* To clear timestamps of previous executed testcase */
                uu_wlan_tsf_r = 0;
                sscanf(buf, "%s %d %s %d %s %s %s %d %s %d",
                    string[0], &id, string[1], &event_type, string[2],
                    frame_input_file, string[3], &filter, string[4], &ret_val);

                ret = uu_wlan_rx_get_rx_vec_info(frame_input_file);
                if(ret == UU_FAILURE)
                {
                    return UU_FAILURE;
                }

                UU_REG_LMAC_FILTER_FLAG = filter;
                ret = uu_wlan_rx_handle_phy_rxstart(&rx_vec);
                if(ret == ret_val)
                {
                    return UU_SUCCESS;
                }
                else
                {
                    printf("ERROR: rx_start_ind failed");
                    return UU_FAILURE;
                }
            }
        case UU_WLAN_CAP_EV_DATA_INDICATION:
            {
                sscanf(buf, "%s %d %s %d %s %s %s %d ",
                    string[0], &id, string[1], &event_type, string[2], frame_input_file, string[3], &ret_val);
                expected_len_in_rx_status_g[0] = uu_wlan_test_get_frame_from_file(frame_input_file, mpdu);
                uu_wlan_get_lengths_mpdus(mpdu);

                if(rx_vec.is_aggregated)
                {
                    for(i=0;i<rx_vec.psdu_length;i++)
                    {
                        ret = uu_wlan_rx_handle_phy_dataind(mpdu[i]);
                        if(ret != ret_val)
                        {
                            printf("ERROR: rx_data_ind failed while filling %d byte of mpdu and byte is: %x", i, mpdu[i]);
                            return UU_FAILURE;
                        }
                    }
                }
                else
                {
                    for(i=0; i<rx_vec.L_length; i++)
                    {
                        ret = uu_wlan_rx_handle_phy_dataind(mpdu[i]);
                        if(ret != ret_val)
                        {
                            printf("ERROR: rx_data_ind failed while filling %d byte of mpdu and byte is: %x", i, mpdu[i]);
                            return UU_FAILURE;
                        }
                    }
                }

                break;
            }
        case UU_WLAN_CAP_EV_RX_END_IND:
            {
                sscanf(buf, "%s %d %s %d %s %s %s %d", 
                    string[0], &id, string[1], &event_type, string[2], frame_input_file, string[3], &ret_val);
                rets = uu_wlan_test_get_expected_output(frame_input_file);
                if(rets != UU_SUCCESS)
                {
                    printf("ERROR: file \"%s\" is not existed to get expected output values\n", frame_input_file);
                    return UU_FAILURE;
                }
                else
                {
                    uu_wlan_rx_handle_phy_rxend(rx_end_status);
                }

                /* TODO: Uncomment this check after fix of return_value for groupcast frames. */
                //if(ret == ret_val)

                if((rx_status_ret_g) &&
                   (((uu_wlan_lmac_ps_enabled_g == ps_enable_g) &&
                    (uu_wlan_lmac_dtim_bcn_received_g == dtim_bcn_g) && 
                    (uu_wlan_lmac_ps_more_data_g == more_data_g) &&
                    (uu_wlan_lmac_eosp_g == eosp_g)) ||
                   (expected_tsf_g == uu_wlan_tsf_r)))
                {
                    ret = UU_SUCCESS;
                }
                else
                {
                    printf("ERROR: expected and actual flags are not matched at rx_end_ind\n"
                        "expected ps_enable: %d dtim_bcn: %d more_data: %d eosp_g: %d tsf_g: %Lx\n"
                        "actual ps_enable: %d dtim_bcn: %d more_data: %d eosp_g: %d tsf_g: %Lx \n rx_status_ret_g: %d\n",
                        ps_enable_g, dtim_bcn_g, more_data_g, eosp_g, expected_tsf_g,
                        uu_wlan_lmac_ps_enabled_g, uu_wlan_lmac_dtim_bcn_received_g,
                        uu_wlan_lmac_ps_more_data_g, uu_wlan_lmac_eosp_g, expected_tsf_g, rx_status_ret_g);
                    ret = UU_FAILURE;
                }
                /* Clearing dtim bcn_rcvd at rx_end_ind as it is done in cap module */
                uu_wlan_lmac_dtim_bcn_received_g = 0;
                break;
            }
        default:
            {
                printf("ERROR: invalid test_category %d\n", rq_type);
                ret = UU_FAILURE;
                break;
            }
    } /* switch */
    return ret;
} /* uu_wlan_rx_test_handler */


static uu_void uu_wlan_get_lengths_mpdus(uu_uchar *input_mpdu)
{
    uu_wlan_rx_frame_info_t *frame_info = uu_wlan_rx_get_frame_info();
    uu_int32 x =0, y = 0;
    uu_int32 pad_len[32] = {0}, mpdu_cnt = 0;

    if(frame_info->rxvec.is_aggregated)
    {
        do
        {
            if(frame_info->rxvec.format == UU_WLAN_FRAME_FORMAT_VHT)
            {
                expected_len_in_rx_status_g[mpdu_cnt] = (input_mpdu[x]|(input_mpdu[x+1]<<8))>>2;
            }
            else
            {
                expected_len_in_rx_status_g[mpdu_cnt] = (input_mpdu[x]|(input_mpdu[x+1]<<8))>>4;
            }
            printf("length and mpdu_cnt: %d %d\n", expected_len_in_rx_status_g[mpdu_cnt], mpdu_cnt);
            pad_len[mpdu_cnt] = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(expected_len_in_rx_status_g[mpdu_cnt]);
            y = expected_len_in_rx_status_g[mpdu_cnt]+pad_len[mpdu_cnt] + UU_WLAN_MPDU_DELIMITER_LEN + x;

            /* 2- delimiters */
            x = y + (2*UU_WLAN_MPDU_DELIMITER_LEN);
            mpdu_cnt++;

        } while((input_mpdu[y] == 0) && (input_mpdu[y+1] == 0) && (input_mpdu[y+2] == 0) && (input_mpdu[y+3] == 0x4e));
    }
} /* uu_wlan_get_lengths_mpdus */


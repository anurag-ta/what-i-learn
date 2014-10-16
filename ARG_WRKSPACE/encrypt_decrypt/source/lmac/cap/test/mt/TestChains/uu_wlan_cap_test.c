#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>
#endif

#include "uu_errno.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_txop.h"
#include "uu_wlan_tx_handler.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_cap_test_main.h"

/* Considered as an event for writing frame to buffer */
#define TEST_WRITE_BUF 1024

/* These are declared as these are used in ac_q file*/
uu_uint8  uu_wlan_LongRetryCount[UU_WLAN_MAX_QID];
uu_uint8  uu_wlan_ShortRetryCount[UU_WLAN_MAX_QID];
uu_wlan_ampdu_status_t uu_wlan_tx_ampdu_status_g[UU_WLAN_MAX_QID];

/* These are for testing of ps cases */
uu_bool uu_wlan_lmac_pspoll_frame_ready_g= UU_FALSE;
uu_bool uu_wlan_lmac_trigger_frame_ready_g = UU_FALSE;

uu_wlan_tx_frame_info_t  *ps_poll_frame_info_g;
uu_wlan_tx_frame_info_t *ps_trigger_frame_info_g;

/* rx_handler_ret_g is to maintain the rx_handler return value which is given as input in testcase */
extern uu_int32 rx_handler_ret_g;

/** test_input structure
 * contains all the values required for testing events on CAP state machine
 */
typedef struct test_input
{
    /** ac - ranges in[0-4] on which event takes place
    * qos_mode - indicates lmac is in which mode
    * status - either 0/1 indicates the channel status as busy/idle
    * channel_list - ranges [0-4] indicates primary and secondary channel status 
    * phy_rx_end_ret value indicates which type err is returned from phy_end
    * event_type is considered for testing of action on state
    */
    uu_int32 ac, qos_mode, status, channel_list, phy_rx_end_ret, event_type;

    /* input_id maintains id of test_input of a testchain */
    uu_char input_id[30];

    /* pre_event and post_event timer_tick counts are the no.of timer_tick events
       that has to run before actual event and after actual evnt on state */
    uu_int32 pre_event_timer_tick_count, post_event_timer_tick_count;

    /* frame_avail_flag is added for future purpose, For now it is not used */
    uu_int32 frame_avail_flag;

    /* input_frames_num is the no.of frames in buffer, 
     * considered only when event is TX_READY/TEST_WRITE_BUF */ 
    uu_int32 input_frames_num;

    /* frames_file, frame_info_file are the frame and frame_info going to written to buffer */
    uu_char frames_file[300], frame_info_file[50];

    /* frame_input_file contains frame which is rcvd at phy end 
    * vec_file is frame_info corresponds to frame rcvd at phy end
    */
    uu_char frame_input_file[300], vec_file[50];

    /** precondition[0], precondition[1] indicates main state and supp state before event takes place
    * postcondition[0], postcondition[1] indicates main state and supp state after event takes place
    */
    uu_int32 precondition[5], postcondition[5];

}test_input_t;


/* Function to fill tx_vec from frame_info file */
static uu_wlan_tx_frame_info_t _fill_tx_vec_frame_info(uu_char *frame_input_file);

/* Function to fill tx_vec with default values */
static uu_void _fill_tx_vec_default_values(uu_wlan_tx_frame_info_t *frame_info);

/** Functions to handle events on particular state of cap state machine
 * these will process events on cap state by using inputs given in testcase
 * @param[in] test_input of a testchain contains set of input parameters and expected outputs(main and supp.states)
 * returns UU_SUCCESS on success, or UU_FAILURE on failure, 32 bits in length
 */
static uu_int32 __give_phy_rx_data_ind(test_input_t *input);
static uu_int32 __give_phy_rx_start_ind(test_input_t *input);
static uu_int32 __give_timer_tick(test_input_t *input);
static uu_int32 __give_tx_ready(test_input_t *input);
static uu_int32 __give_phy_energy_level(test_input_t *input);
static uu_int32 __give_phy_rx_error(test_input_t *input);
static uu_int32 __give_phy_rx_end_ind(test_input_t *input);
static uu_int32 __give_invalid_event(test_input_t *input);

/** Function to write frame to buffer internally calls tx_write_buffer functionality
 * @param[in] test_input of a testchain contains set of input parameters and expected outputs(main and supp.states)
*/
static uu_void write_frame_q(test_input_t *input);


/** Writing frame to buffer
 * @param[in] frame_input_file is the frame which has to be written to buffer
 * @param[in] frameinfo_file is the frem_info corresponding to frame 
 */ 
static uu_void __umac_buffer_simulator(uu_char *frame_input_file, uu_char *frameinfo_file, uu_int32 ac)
{
    uu_uint32 available_buf;
    uu_uchar mpdu[3][200];
    uu_uchar whole_buffer[3][300];
    uu_uchar *addrs[3];
    uu_uint32 len[4];
    uu_wlan_tx_frame_info_t frame_info;

    frame_info = _fill_tx_vec_frame_info(frameinfo_file);

    frame_info.ac = ac;   
    printf("aggr count of frame info is: %d\n", frame_info.aggr_count);
    available_buf = uu_wlan_tx_get_available_buffer((uu_int8)frame_info.ac);
    printf("available buffer %d\n", available_buf);
    printf("frame input file is %s", frame_input_file);
    frame_info.frameInfo.framelen = uu_wlan_test_get_frame_from_file(frame_input_file, mpdu[0]);
    memcpy(whole_buffer[0], &frame_info, sizeof(uu_wlan_tx_frame_info_t));
    printf("len is %d----------------", frame_info.frameInfo.framelen);
    memcpy((whole_buffer[0] + sizeof(uu_wlan_tx_frame_info_t)), mpdu[0], frame_info.frameInfo.framelen);
    addrs[0] = (uu_uchar *)whole_buffer[0];
    len[0] = sizeof(uu_wlan_tx_frame_info_t) + frame_info.frameInfo.framelen;
    len[1] = 0;
    addrs[1] = NULL;
    if(available_buf!= 0)
    {
        printf("after filling addr and lens\n");
        uu_wlan_tx_write_buffer(frame_info.ac, addrs, len);
    }
} /* __umac_buffer_simulator */


/* Function to calculate timer_ticks */
static int _cal_timer_ticks(uu_int32 pre_event_timer_tick_count, uu_int32 *precondition, uu_int32 ac)
{
    if(precondition[0] == UU_WLAN_CAP_STATE_CONTEND)
    {
        if(pre_event_timer_tick_count == 65535)
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val;  
        }
        else
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val-2; 
        }
    }
    else if(precondition[0] == UU_WLAN_CAP_STATE_EIFS)
    {
        if(pre_event_timer_tick_count == 65535)
        {
            printf("eifs val is %d\n", uu_wlan_cap_ctxt_g.eifs_timer_val);
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.eifs_timer_val; 
        }
        else
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val-2; 
        }
    }
    else if(precondition[0] == UU_WLAN_CAP_STATE_RECEIVING)
    {
        printf("pre_event_time tick is %d nav_val %d\n", pre_event_timer_tick_count, uu_wlan_cap_ctxt_g.nav_val);
        if(pre_event_timer_tick_count == 65535)
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.nav_val; 
        }
        else
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val-2; 
        }
    }
    else if((precondition[0] == UU_WLAN_CAP_STATE_TXOP) && (precondition[1] == UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX))
    {
        if (IEEE80211_IS_FC0_RTS(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu))
        {
            if(pre_event_timer_tick_count == 65535)
            {
                pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.cts_timer_val; 
            }
            else
            {
                /* For giving the response before timer expires */
                pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.cts_timer_val-2; 
            }
        }
        else
        {
            if(pre_event_timer_tick_count == 65535)
            {
                pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val; 
            }
            else
            {
                pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val-2; 
            }
        }

    }
    else if((precondition[0] == UU_WLAN_CAP_STATE_TXOP) &&
            (precondition[1] == UU_WLAN_CAP_TXOP_STATE_WAIT_TXNAV_END))
    {
        if(pre_event_timer_tick_count == 65535)
        {
            if(uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_nav == 0)
            {
                pre_event_timer_tick_count = 1;
            }
            else
            {
                pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_nav;
            }
        }
        else
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val-2; 
        }
    }
    else if((precondition[0] == UU_WLAN_CAP_STATE_TXOP) &&
            (precondition[1] == UU_WLAN_CAP_TXOP_STATE_SENDING_DATA))
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_START_CONFIRM, NULL, 0);
        pre_event_timer_tick_count = 0;
    }
    else
    {
        pre_event_timer_tick_count = 0;
    }
    printf("Timer ticks %d \n", pre_event_timer_tick_count);
    return pre_event_timer_tick_count;
}/* _cal_timer_ticks */


static uu_void __fill_preconditions_in_cap_context(uu_int32 *precondition)
{
    printf("filling preconditions here\n");
    uu_wlan_cap_ctxt_g.sm_cntxt.cur_state = (uu_wlan_sm_state_info_t *)
        uu_wlan_sm_get_state_info(&uu_wlan_cap_ctxt_g.sm_cntxt, precondition[0]);
    uu_wlan_cap_ctxt_g.slot_timer_countdown = (uu_uint8)UU_SLOT_TIMER_VALUE_R;
    printf("uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state %d\n", uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
    if(uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state == UU_WLAN_CAP_STATE_TXOP)
    {
        uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state = (uu_wlan_sm_state_info_t *)
            uu_wlan_sm_get_state_info(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, precondition[1]);
    }    
}/* __fill_preconditions_in_cap_context */


/* Function to check whether pre-condition matches (main_state and sup_state) */
static uu_int32 __check_precondition(uu_int32 *precondition, uu_int32 ac)
{
    if(precondition[0] == uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state)
    {
        if(precondition[0] == UU_WLAN_CAP_STATE_TXOP)
        {
            if(precondition[1] == uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state)
            {
                return UU_SUCCESS;
            }
            else
            {
                printf("ERROR: precondition failed. Expected main_state: %d supp_state: %d, "
                    "Actual main_state: %d supp_state: %d\n",
                    precondition[0], precondition[1],
                    uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state,
                    uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state);
                return UU_FAILURE;
            }
        }
        else
        {
            return UU_SUCCESS;
        }
    }
    else
    {
        printf("ERROR: precondition failed. Expected main_state: %d, Actual main_state: %d\n",
            precondition[0], uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
        return UU_FAILURE;
    }
}/* __check_precondition */


/* Function to validate output of every test-input in testchain */
static uu_int32 __validate_postconditions(uu_int32 *postcondition, uu_int32 ac)
{
    uu_int32 ret, i;
    uu_wlan_tx_frame_info_t *frame_info;

    printf("verifying post conditions --\n");
    printf("postcondition[0]:%d postcondition[1]:%d\n", postcondition[0], postcondition[1]);

    printf("ac:%d aifs:%d backoff:%d tx_prog%d tx_nav:%d cw_val:%d ack_timer:%d cts_timer:%d "
        "eifs_timer:%d nav_val:%d nav_reset_trimer:%d cca_status%d txop_owner_ac:%d main_state:%d ",
        uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_ready, uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val,
        uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val, uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_prog,
        uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_nav, uu_wlan_cap_ctxt_g.ac_cntxt[ac].cw_val,
        uu_wlan_cap_ctxt_g.ack_timer_val, uu_wlan_cap_ctxt_g.cts_timer_val,
        uu_wlan_cap_ctxt_g.eifs_timer_val, uu_wlan_cap_ctxt_g.nav_val,
        uu_wlan_cap_ctxt_g.nav_reset_timer, uu_wlan_cap_ctxt_g.cca_status,
        uu_wlan_cap_ctxt_g.txop_owner_ac, uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
    if(uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state == UU_WLAN_CAP_STATE_TXOP)
    {
        printf("supp_state:%d\n", uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state);
    }
    else
    {
        printf("supp_state:0\n");
    }

    /* validating post conditions */
    if((uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state == postcondition[0]))
    {
        if(postcondition[0] == UU_WLAN_CAP_STATE_TXOP)
        {
            printf(" supp states in validate post condition are: actual:%d expected:%d\n", 
                uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state, postcondition[1]);
            if(uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state == postcondition[1])
            {
                ret = UU_SUCCESS;
            }
            else
            {
                /* post condition of supplimentary s/m not satisfied */
                printf("ERROR: post condition failed, expected: supp_state: %d, actual: supp_state: %d",
                    uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state, postcondition[0]); 
                ret = UU_FAILURE;
            }    
        }
        else
        {
            ret = UU_SUCCESS;
        }    
    }
    else
    {
        printf("ERROR: post condition failed. Expected main_state: %d, Actual: %d",
            postcondition[0], uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state); 
        ret = UU_FAILURE;
    }

    /* Verifing whether ac_q buffers are free or not at idle state */
    if (ret == UU_SUCCESS &&
       ((postcondition[0] == UU_WLAN_CAP_STATE_IDLE) ||
        (uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state == UU_WLAN_CAP_STATE_IDLE)))
    {
        for(i=0; i<=UU_WLAN_AC_MAX_AC; i++)
        {
            frame_info = uu_wlan_get_msg_ref_from_AC_Q(i);
            if(frame_info != UU_NULL) 
            {
                printf("ERROR: frame info is not cleared for ac: %d\n", i);
                return UU_FAILURE;
            }
        }
    }
    return ret;
}/* __validate_postconditions */


/* cap test handler start here */
uu_int32 uu_wlan_cap_test_handler(uu_int32 event_type, uu_char *buf)
{    
    test_input_t  input;

    /* Reading of test_input into structure */
    sscanf(buf, "%s ev_type: %d pre_timer_tick_count: %d "
        "post_timer_tick_count: %d is_frame_available: %d "
        "no.of_frames: %d frame_input_file: %s frame_info_file: %s "
        "precondition: main_state: %d supp_state: %d "
        "postcondition: main_state: %d supp_state: %d "
        "ac: %d qos: %d status: %d "
        "ch_list: %d vec_file: %s frame_file: %s "
        "rx_end_ret: %x phy_end_ret: %d \n",
        input.input_id, &(input.event_type), &(input.pre_event_timer_tick_count),
        &(input.post_event_timer_tick_count), &(input.frame_avail_flag),
        &(input.input_frames_num), input.frames_file, input.frame_info_file,
        &(input.precondition[0]), &(input.precondition[1]),
        &(input.postcondition[0]), &(input.postcondition[1]),
        &(input.ac), &(input.qos_mode), &(input.status),
        &(input.channel_list), input.vec_file, input.frame_input_file,
        &rx_handler_ret_g, &(input.phy_rx_end_ret));

    /* event_type handling here */
    switch(event_type)
    {
        case TEST_WRITE_BUF:
            {
                write_frame_q(&input);
            }
            break;
        case UU_WLAN_CAP_EV_TIMER_TICK:
            {
                return __give_timer_tick(&input);
            }
        /* At the time of tx_ready event frame is going to written to buffer and tx_ready 
           event will be performed on that */
        case UU_WLAN_CAP_EV_TX_READY:
            {
                write_frame_q(&input);
                return __give_tx_ready(&input);
            }
        case UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL:
            {
                return __give_phy_energy_level(&input);
            }
        case UU_WLAN_CAP_EV_PHY_RX_ERR:
            {
                return __give_phy_rx_error(&input);
            }
        case UU_WLAN_CAP_EV_RX_END_IND:
            {
                return __give_phy_rx_end_ind(&input);
            }
        case UU_WLAN_CAP_EV_RX_START_IND:
            {
                return __give_phy_rx_start_ind(&input);
            }
        case UU_WLAN_CAP_EV_DATA_INDICATION:
            {
                return __give_phy_rx_data_ind(&input);
            }

        /** TXOP_TX_START and CCA_RESET_CONFIRM events are internally posted by CAP module
        * MAX and MAC_RESET events won't perform any operation 
        * TX_START, DATA and END confirm events are when tx processing is done properly i.e done in tx side.
        */
        case UU_WLAN_CAP_EV_TXOP_TX_START:
        case UU_WLAN_CAP_EV_CCA_RESET_CONFIRM:
        case UU_WLAN_CAP_EV_MAX:
        case UU_WLAN_CAP_EV_MAC_RESET:
        case UU_WLAN_CAP_EV_TX_START_CONFIRM:
        case UU_WLAN_CAP_EV_TX_DATA_CONFIRM:
        case UU_WLAN_CAP_EV_TX_END_CONFIRM:
            break;

        /* When unknown event is posted in cap_q */
        default:
            return __give_invalid_event(&input);
    }/* switch */
    return UU_SUCCESS;
} /* uu_wlan_cap_test_handler */


static uu_void write_frame_q(test_input_t *input)
{
    FILE *fp ;
    uu_char input_file_string[130], input_file_string1[300] = "";

    if(input->input_frames_num > 0)
    {
        fp = fopen(input->frames_file, "r");
        if(fp == NULL)
        {
            printf("ERROR: file doesn't exist %s", input->frames_file);
        }
        else
        {
            for(int i = 0; i < input->input_frames_num; i++)
            {
                /* Here we read line by line from file(individual testcase id and parameters) */
                fgets(input_file_string, 200, fp);
                strncpy(input_file_string1, input_file_string, strlen(input_file_string)-1);
                __umac_buffer_simulator(input_file_string1, input->frame_info_file, input->ac);
            }
        }
        fclose(fp);
    }
}/* write_frame_q */


static uu_wlan_tx_frame_info_t _fill_tx_vec_frame_info(uu_char *frame_input_file)
{
    uu_char string[100];
    uu_char line[100];
    uu_int32 value;
    FILE* fp = NULL;
    uu_wlan_tx_frame_info_t frame_info;

    fp = fopen(frame_input_file,"r");
    if(fp == NULL)
    {
        printf("ERROR: file \"%s\" to fill frame info doesn't exist\n", frame_input_file);
        return;
    }

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s = %d", string, &value);

        if(strcmp(string,"format") == 0)
        {
            frame_info.txvec.format = value;
        }
        else if(strcmp(string,"n_tx") == 0)
        {
            frame_info.txvec.n_tx = value;
        }
        else if(strcmp(string,"txpwr_level") == 0)
        {
            frame_info.txvec.txpwr_level = value;
        }
        else if(strcmp(string,"reserved1") == 0)
        {
            frame_info.txvec.reserved1 = value;
        }
        else if(strcmp(string,"L_length") == 0)
        {
            frame_info.txvec.L_length = value;
        }
        else if(strcmp(string,"service") == 0)
        {
            frame_info.txvec.service = value;
        }
        else if(strcmp(string,"antenna_set") == 0)
        {
            frame_info.txvec.antenna_set = value;
        }
        else if(strcmp(string,"num_ext_ss") == 0)
        {
            frame_info.txvec.num_ext_ss = value;
        }
        else if(strcmp(string,"no_sig_extn") == 0)
        {
            frame_info.txvec.is_no_sig_extn = value;
        }
        else if(strcmp(string,"mcs") == 0)
        {
            frame_info.txvec.mcs = value;
        }
        else if(strcmp(string,"txvec.is_long_preamble") == 0)
        {
            frame_info.txvec.is_long_preamble = value;
        }
        else if(strcmp(string,"ht_length") == 0)
        {
            frame_info.txvec.ht_length = value;
        }
        else if(strcmp(string,"is_smoothing") == 0)
        {
            frame_info.txvec.is_smoothing = value;
        }
        else if(strcmp(string,"reserved") == 0)
        {
            frame_info.txvec.reserved = value;
        }
        else if(strcmp(string,"is_aggregated") == 0)
        {
            frame_info.txvec.is_aggregated = value;
        }
        else if(strcmp(string,"stbc") == 0)
        {
            frame_info.txvec.stbc = value;
        }
        else if(strcmp(string,"is_fec_ldpc_coding") == 0)
        {
            frame_info.txvec.is_fec_ldpc_coding = value;
        }
        else if(strcmp(string,"is_short_GI") == 0)
        {
            frame_info.txvec.is_short_GI = value;
        }
        else if(strcmp(string,"partial_aid") == 0)
        {
            frame_info.txvec.partial_aid = value;
        }
        else if(strcmp(string,"is_beamformed") == 0)
        {
            frame_info.txvec.is_beamformed = value;
        }
        else if(strcmp(string,"num_users") == 0)
        {
            frame_info.txvec.num_users = value;
        }
        else if(strcmp(string,"is_dyn_bw") == 0)
        {
            frame_info.txvec.is_dyn_bw = value;
        }
        else if(strcmp(string,"group_id") == 0)
        {
            frame_info.txvec.group_id = value;
        }
        else if(strcmp(string,"is_tx_op_ps_allowed") == 0)
        {
            frame_info.txvec.is_tx_op_ps_allowed = value;
        }
        else if(strcmp(string,"is_time_of_departure_req") == 0)
        {
            frame_info.txvec.is_time_of_departure_req = value;
        }
        else if(strcmp(string,"rts_threshold") == 0)
        {
            dot11_rts_threshold_value = value;
        }
        else if(strcmp(string,"short_retry") == 0)
        {
            dot11_short_retry_count = value;
        }
        else if(strcmp(string,"long_retry") == 0)
        {
            dot11_long_retry_count = value;
        }
        else if(strcmp(string,"slot_timer") == 0)
        {
            dot11_slot_timer_value = value;
        }
        else if(strcmp(string,"cw_min_val_0") == 0)
        {
            dot11_cwmin_value_ac[0] = value;
        }
        else if(strcmp(string,"cw_min_val_1") == 0)
        {
            dot11_cwmin_value_ac[1] = value;
        }
        else if(strcmp(string,"cw_min_val_2") == 0)
        {
            dot11_cwmin_value_ac[2] = value;
            printf("cw min value of ac2%d\n",dot11_cwmin_value_ac[2]);
        }
        else if(strcmp(string,"cw_min_val_3") == 0)
        {
            dot11_cwmin_value_ac[3] = value;
        }
        else if(strcmp(string, "txop_limit_0") == 0)
        {
            dot11_txop_limit_value[0] = value;
        }
        else if(strcmp(string, "txop_limit_1") == 0)
        {
            dot11_txop_limit_value[1] = value;
        }
        else if(strcmp(string, "txop_limit_2") == 0)
        {
            dot11_txop_limit_value[2] = value;
        }
        else if(strcmp(string, "txop_limit_3") == 0)
        {
            dot11_txop_limit_value[3] = value;
        }
        else if(strcmp(string, "eifs_timer") == 0)
        {
            dot11_eifs_timer_value = value;
        }
        else if(strcmp(string, "aggr_count") == 0)
        {
            frame_info.aggr_count = value;        
            printf("aggr count in frame_info is: %d\n", frame_info.aggr_count);
        }
        else
        {
            printf("\n");
        }
    }
    fclose(fp);
    return frame_info;
}/* _fill_tx_vec_frame_info */


static uu_void _fill_tx_vec_default_values(uu_wlan_tx_frame_info_t *frame_info)
{
    /** Enable format = 2 for ht case */
    //frame_info->txvec.format = 2;
    frame_info->txvec.format = 0;
    frame_info->txvec.n_tx = 3;
    frame_info->txvec.txpwr_level = 7;
    frame_info->txvec.reserved1 = 1;
    frame_info->txvec.L_length = 12;
    frame_info->txvec.service = 16;
    frame_info->txvec.antenna_set = 8;
    frame_info->txvec.num_ext_ss = 2;
    frame_info->txvec.is_no_sig_extn = 1;
    frame_info->txvec.mcs = 7;
    frame_info->txvec.is_long_preamble = 1;
    frame_info->txvec.ht_length = 16;
    frame_info->txvec.is_smoothing = 1;
    frame_info->txvec.reserved = 1;
    frame_info->txvec.is_aggregated = 1;
    frame_info->txvec.stbc = 2;
    frame_info->txvec.is_fec_ldpc_coding = 1;
    frame_info->txvec.is_short_GI = 1;
    frame_info->txvec.partial_aid = 9;
    frame_info->txvec.is_beamformed = 1;
    frame_info->txvec.num_users = 2;
    frame_info->txvec.is_dyn_bw = 1;
    frame_info->txvec.group_id = 6;
    frame_info->txvec.is_tx_op_ps_allowed = 0;
    frame_info->txvec.is_time_of_departure_req = 1;
    frame_info->txvec.padding_byte0 = 0;
}/* _fill_tx_vec_default_values */


static uu_int32 __give_invalid_event(test_input_t *input)
{
    __fill_preconditions_in_cap_context(input->precondition);
    
    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)input->event_type, NULL, 0);

    return __validate_postconditions(input->postcondition, input->ac);
} /* __give_invalid_event */


/* Function to handle timer_tick event */
static uu_int32 __give_timer_tick(test_input_t *input)
{
    /* backoff_cnt will get the backoff_val in cap context structure 
     * i and j are loop var.s used in every function */
    uu_int32 i, j, backoff_cnt;
    uu_wlan_cap_event_t    ev;

    /* Whether inputs of a testcase are valid or not by checking with previous tetscase postcondition */ 
    if(__check_precondition(input->precondition, input->ac) != UU_SUCCESS)
    {
        return UU_FAILURE;
    }

    __fill_preconditions_in_cap_context(input->precondition);

    if(input->precondition[0] == UU_WLAN_CAP_STATE_IDLE)
    {
        /* Setting Qos mode */
        uu_dot11_qos_mode_r = input->qos_mode;

        /* SM start with idle state */
        uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, input->precondition[0], input->event_type);
    }

    /** Handling of timer ticks */
    if(input->pre_event_timer_tick_count >= 65534)
    {
        input->pre_event_timer_tick_count =
            _cal_timer_ticks(input->pre_event_timer_tick_count, input->precondition, input->ac);
    }
    /** End */

    /** Posting pre event timer ticks into cap_q */
    for(i = 0; i < input->pre_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    /** Posting post event timer ticks into cap_q */
    for(i = 0; i < input->post_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
    {
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
        printf("State m/c current state %d \n", uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
        if ((input->precondition[0] == UU_WLAN_CAP_STATE_CONTEND) &&
            (uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].aifs_val == 0))
        {
            printf("Inside aifs0 check \n");
            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].backoff_val;
            for (j = 0; j< backoff_cnt; j++)
            {
                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK;
                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
            }
        }
    }

    return __validate_postconditions(input->postcondition, input->ac);
}/* __give_timer_tick */


/* Function to handle tx_ready event */
static uu_int32 __give_tx_ready(test_input_t *input)
{
    /* backoff_cnt will get the backoff_val in cap context structure 
     * i and j are loop var.s used in every function */
    uu_int32 i, j, backoff_cnt;
    /* frame_input_file is to maintain frame rcvd at rx_side which is given as input */
    uu_wlan_cap_event_t    ev;

    printf("post conditions given for tx_ready event is: %d %d\n", input->postcondition[0], input->postcondition[1]);
    
    /* Whether inputs of a testcase are valid or not by checking with previous tetscase postcondition */ 
    if(__check_precondition(input->precondition, input->ac) != UU_SUCCESS)
    {
        return UU_FAILURE;
    }

    __fill_preconditions_in_cap_context(input->precondition);

    if(input->precondition[0] == UU_WLAN_CAP_STATE_IDLE)
    {
        /* Setting Qos mode */
        uu_dot11_qos_mode_r = input->qos_mode;

        /* SM start with idle state */
        uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, input->precondition[0], input->event_type);
    }

    /** Handling of timer ticks */
    if(input->pre_event_timer_tick_count >= 65534)
    {
        input->pre_event_timer_tick_count =
            _cal_timer_ticks(input->pre_event_timer_tick_count, input->precondition, input->ac);
    }

    /** Posting pre event timer ticks */
    for(i = 0; i < input->pre_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }
    /** Posting event */
    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)input->event_type, (unsigned char *)&(input->ac), 1);

    /** Posting post event timer ticks */
    for(i = 0; i < input->post_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    for (;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
    {
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
        if ((input->precondition[0] == UU_WLAN_CAP_STATE_CONTEND) &&
            (uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].aifs_val == 0))
        {
            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].backoff_val;
            for (j = 0; j< backoff_cnt; j++)
            {
                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
            }
        }
    }

    return __validate_postconditions(input->postcondition, input->ac);
}/* __give_tx_ready */


/* Function to handle phy_energy_level event */
static uu_int32 __give_phy_energy_level(test_input_t *input)
{
    /* backoff_cnt will get the backoff_val in cap context structure 
     * i and j are loop var.s used in every function */
    uu_int32 i, j, backoff_cnt;
    uu_wlan_cap_event_t    ev;
    unsigned char cca_ind[2];
    
    /* Whether inputs of a testcase are valid or not by checking with previous tetscase postcondition */
    if(__check_precondition(input->precondition, input->ac) != UU_SUCCESS)
    {
        return UU_FAILURE;
    }

    __fill_preconditions_in_cap_context(input->precondition);

    if(input->precondition[0] == UU_WLAN_CAP_STATE_IDLE)
    {
        /* Setting Qos mode */
        uu_dot11_qos_mode_r = input->qos_mode;

        /* SM start with idle state */
        uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, input->precondition[0], input->event_type);
    }

    /** Handling of timer ticks */
    if(input->pre_event_timer_tick_count >= 65534)
    {
        input->pre_event_timer_tick_count =
            _cal_timer_ticks(input->pre_event_timer_tick_count, input->precondition, input->ac);
    }
    /** End */

    for(i = 0; i < input->pre_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    /** Reading cca status from input */
    cca_ind[0] = (unsigned char)input->status;

    /** Reading channel list from the input  */
    cca_ind[1] = (unsigned char)input->channel_list;

    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)(input->event_type), (unsigned char *)cca_ind, 2);

    for(i = 0; i < input->post_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
    {
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
        if ((input->precondition[0] == UU_WLAN_CAP_STATE_CONTEND) &&
            (uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].aifs_val == 0))
        {
            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].backoff_val;
            for (j = 0; j< backoff_cnt; j++)
            {
                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
            }
        }
    }

    return __validate_postconditions(input->postcondition, input->ac);
}/* __give_phy_energy_level */


/* Function to handle phy_rx_err event */           
static uu_int32 __give_phy_rx_error(test_input_t *input)
{
    /* backoff_cnt will get the backoff_val in cap context structure 
     * i and j are loop var.s used in every function */
    uu_int32 i, j, backoff_cnt;
    uu_wlan_cap_event_t    ev;

    /* Whether inputs of a testcase are valid or not by checking with previous tetscase postcondition */ 
    if(__check_precondition(input->precondition, input->ac) != UU_SUCCESS)
    {
        return UU_FAILURE;
    }

    __fill_preconditions_in_cap_context(input->precondition);

    /** Handling of timer ticks */
    if(input->pre_event_timer_tick_count >= 65534)
    {
        input->pre_event_timer_tick_count =
            _cal_timer_ticks(input->pre_event_timer_tick_count, input->precondition, input->ac);
    }
    /** End */

    for(i = 0; i < input->pre_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)input->event_type, (unsigned char *)&(input->status), 1);

    for(i = 0; i < input->post_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }
    for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
    {
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
        if ((input->precondition[0] == UU_WLAN_CAP_STATE_CONTEND) &&
            (uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].aifs_val == 0))
        {
            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].backoff_val;
            for (j = 0; j< backoff_cnt; j++)
            {
                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
            }
        }
    }

    return __validate_postconditions(input->postcondition, input->ac);        
}/* __give_phy_rx_err */


/* Function to handle phy_rx_end event */
static uu_int32 __give_phy_rx_end_ind(test_input_t *input)
{
    /* backoff_cnt will get the backoff_val in cap context structure 
     * i and j are loop var.s used in every function */
    uu_int32 i, j, backoff_cnt;
    uu_wlan_cap_event_t    ev;

    /* Whether inputs of a testcase are valid or not by checking with previous tetscase postcondition */ 
    if(__check_precondition(input->precondition, input->ac) != UU_SUCCESS)
    {
        return UU_FAILURE;
    }

    __fill_preconditions_in_cap_context(input->precondition);

    /** Handling of timer ticks */
    if(input->pre_event_timer_tick_count >= 65534)
    {
        input->pre_event_timer_tick_count =
            _cal_timer_ticks(input->pre_event_timer_tick_count, input->precondition, input->ac);
    }
    /** End */

    for(i = 0; i < input->pre_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)input->event_type, (unsigned char *)&(input->status), 1);
    for(i = 0; i < input->post_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    uu_wlan_rx_handle_phy_rxend(rx_handler_ret_g);
    for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
    {
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
        if ((input->precondition[0] == UU_WLAN_CAP_STATE_CONTEND) &&
            (uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].aifs_val == 0))
        {
            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].backoff_val;
            for (j = 0; j< backoff_cnt; j++)
            {
                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
            }
        }
    }

    return __validate_postconditions(input->postcondition, input->ac);        
} /* __give_phy_rx_err */


/* Function to handle phy_rx_start_ind event */
static uu_int32 __give_phy_rx_start_ind(test_input_t *input)
{
    /* backoff_cnt will get the backoff_val in cap context structure 
     * i and j are loop var.s used in every function */
    uu_int32 i, j, backoff_cnt;
    uu_wlan_cap_event_t    ev;
    static unsigned char mpdu[1000];
    static int len = 0;
    uu_wlan_rx_vector_t    rx_vector;

    /* Whether inputs of a testcase are valid or not by checking with previous tetscase postcondition */ 
    if(__check_precondition(input->precondition, input->ac) != UU_SUCCESS)
    {
        return UU_FAILURE;
    }
    
    __fill_preconditions_in_cap_context(input->precondition);

    /** Handling of timer ticks */
    if(input->pre_event_timer_tick_count >= 65534)
    {
        input->pre_event_timer_tick_count =
            _cal_timer_ticks(input->pre_event_timer_tick_count, input->precondition, input->ac);
    }
    /** End */

    for(i = 0; i < input->pre_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    len = uu_wlan_test_get_frame_from_file(input->vec_file, mpdu);
    memset(&rx_vector, 0, sizeof(uu_wlan_rx_vector_t));
    rx_vector.L_length = (unsigned short)len;
    rx_vector.L_datarate = (unsigned short)10;//temporarly.
    rx_vector.is_aggregated = 0;
    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)input->event_type,
        (unsigned char *)&rx_vector, sizeof(uu_wlan_rx_vector_t));

    for(i = 0; i < input->post_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    for (;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
    {
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
        if ((input->precondition[0] == UU_WLAN_CAP_STATE_CONTEND) &&
            (uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].aifs_val == 0))
        {
            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].backoff_val;
            for (j = 0; j< backoff_cnt; j++)
            {
                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
            }
        }
    }

    return __validate_postconditions(input->postcondition, input->ac);
}/* __give_phy_rx_start_ind */


/* Function to handle phy_rx_data_ind event */
static uu_int32 __give_phy_rx_data_ind(test_input_t *input)
{
    /* backoff_cnt will get the backoff_val in cap context structure 
     * i and j are loop var.s used in every function */
    uu_int32 i, j, backoff_cnt;
    uu_wlan_cap_event_t    ev;
    static uu_uchar mpdu[1000];
    static uu_int32 len = 0;

    /* form a mpdu and post event to CAP Q */

    /* Whether inputs of a testcase are valid or not by checking with previous tetscase postcondition */ 
    if(__check_precondition(input->precondition, input->ac) != UU_SUCCESS)
    {
        return UU_FAILURE;
    }

    __fill_preconditions_in_cap_context(input->precondition);

    /** Handling of timer ticks */
    if(input->pre_event_timer_tick_count >= 65534)
    {
        input->pre_event_timer_tick_count =
            _cal_timer_ticks(input->pre_event_timer_tick_count, input->precondition, input->ac);
    }
    /** End */

    for(i = 0; i < input->pre_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }
    len = uu_wlan_test_get_frame_from_file(input->frame_input_file, mpdu);
    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)input->event_type, (unsigned char*)mpdu, len);
    len = 0;

    for(i = 0; i < input->post_event_timer_tick_count; i++)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    }

    for (;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
    {
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
        if ((input->precondition[0] == UU_WLAN_CAP_STATE_CONTEND) &&
            (uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].aifs_val == 0))
        {
            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[input->ac].backoff_val;
            for (j = 0; j< backoff_cnt; j++)
            {
                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
            }
        }
    }

    return __validate_postconditions(input->postcondition, input->ac);
}/* __give_phy_rx_data_ind */


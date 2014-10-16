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
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_txop.h"
#include "uu_wlan_tx_handler.h"
#include "uu_wlan_cap_test_main.h"


static void _fill_tx_vec_frame_info(char *frame_input_file);
static void _fill_tx_vec_default_values(uu_wlan_tx_frame_info_t *frame_info);
uu_uint8  uu_wlan_LongRetryCount[UU_WLAN_AC_MAX_AC];
uu_uint8  uu_wlan_ShortRetryCount[UU_WLAN_AC_MAX_AC];
uu_wlan_ampdu_status_t uu_wlan_tx_ampdu_status_g[UU_WLAN_AC_MAX_AC];
int precondition[20];
static uu_wlan_tx_frame_info_t frame_info;
static uu_int32 rets=0xff;
int post_timer_tick_count = 0;
int postcondition[20];
int ac;

static uu_int32 __umac_buffer_simulator(uu_char *frame_input_file, uu_char *frameinfo_file)
{
    uu_uint32 available_buf;
    char mpdu[3][200];
    int sub_len = 0;
    uu_uchar whole_buffer[3][300];
    uu_wlan_tx_frame_info_t *frame_lp;
    uu_uchar *addrs[3];
    uu_uint32 len[4];
    _fill_tx_vec_frame_info(frameinfo_file);
    frame_info.ac=ac;   
    available_buf = uu_wlan_tx_get_available_buffer((uu_int8)frame_info.ac);
    printf("available buffer %d\n", available_buf);
    printf("frame input file is %s",frame_input_file);
    frame_info.frameInfo.framelen = uu_wlan_test_get_frame_from_file(frame_input_file, mpdu[0]);
    memcpy(whole_buffer[0], &frame_info, sizeof(uu_wlan_tx_frame_info_t));
    printf("len is %d----------------",frame_info.frameInfo.framelen);
    memcpy((whole_buffer[0] + sizeof(uu_wlan_tx_frame_info_t)), mpdu[0], frame_info.frameInfo.framelen);
    addrs[0] = (uu_uchar *)whole_buffer[0];
    len[0] = sizeof(uu_wlan_tx_frame_info_t) + frame_info.frameInfo.framelen;
    len[1] = 0;
    addrs[1] = NULL;
    if(available_buf!=0)
    {
        printf("after filling addr and lens\n");
        uu_wlan_tx_write_buffer(frame_info.ac, addrs, len);
    }
}

/*_cal_timer_ticks to calculate no.of timer_tick events based on values of timers*/
static int _cal_timer_ticks(int pre_event_timer_tick_count, int *precondition, int qos_mode, int ac)
{
    //int pre_event_timer_tick_count;

    if(precondition[2] == UU_WLAN_CAP_STATE_CONTEND)
    {
        if(pre_event_timer_tick_count == 65535)
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val; //uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val; 
        }
        else
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val-2; 
        }
    }
    else if(precondition[2] == UU_WLAN_CAP_STATE_EIFS)
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
    else if(precondition[2] == UU_WLAN_CAP_STATE_RECEIVING)
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
    else if((precondition[2] == UU_WLAN_CAP_STATE_TXOP) && (precondition[3] == UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX))
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
    else if((precondition[2] == UU_WLAN_CAP_STATE_TXOP) && (precondition[3] == UU_WLAN_CAP_TXOP_STATE_WAIT_TXNAV_END))
    {
        if(pre_event_timer_tick_count == 65535)
        {
            if(uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_nav == 0)
            {
                pre_event_timer_tick_count=1;//uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav;
            }
            else
            {
                pre_event_timer_tick_count=uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_nav;
            }
        }
        else
        {
            pre_event_timer_tick_count = uu_wlan_cap_ctxt_g.ack_timer_val-2; 
        }
    }
    else if((precondition[2] == UU_WLAN_CAP_STATE_TXOP) && (precondition[3] == UU_WLAN_CAP_TXOP_STATE_SENDING_DATA))
    {
        uu_wlan_cap_event_t	ev;
        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)UU_WLAN_CAP_EV_TX_START_CONFIRM, (uu_uchar *)&ev);
        pre_event_timer_tick_count = 0;
    }
    else
    {
        pre_event_timer_tick_count = 0;
    }
    printf("Timer ticks %d \n", pre_event_timer_tick_count);
    return pre_event_timer_tick_count;
}/*EOF _cal_timer_ticks*/


static void __fill_preconditions_in_cap_context(int *precondition, int ac)
{
    printf("filling preconditions here\n");
    //uu_wlan_cap_ctxt_g.nav_val = (unsigned short)precondition[0];
    //uu_wlan_cap_ctxt_g.nav_reset_timer = (int)precondition[1];
    uu_wlan_cap_ctxt_g.sm_cntxt.cur_state = (uu_wlan_sm_state_info_t *)uu_wlan_sm_get_state_info(&uu_wlan_cap_ctxt_g.sm_cntxt, precondition[2]);
    //    UU_SLOT_TIMER_VALUE_R = 1;
    uu_wlan_cap_ctxt_g.slot_timer_countdown = (uu_uint8)UU_SLOT_TIMER_VALUE_R;
    printf("uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state %d\n", uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
    if(uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state == UU_WLAN_CAP_STATE_TXOP)
    {
        uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state = (uu_wlan_sm_state_info_t *)uu_wlan_sm_get_state_info(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, precondition[3]);
    }	
}/* __fill_preconditions_in_cap_context */

static int __validate_postconditions(int *postcondition, int ac)
{
    int ii;
    printf("verifying post conditions here\n");
    for(ii = 0; ii < 4; ii++)
    {
        printf("postcondition[%d] %d\n", ii, postcondition[ii]);
    }
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_ready,uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val,uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val,uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_prog,uu_wlan_cap_ctxt_g.ac_cntxt[ac].tx_nav,uu_wlan_cap_ctxt_g.ac_cntxt[ac].SSRC,uu_wlan_cap_ctxt_g.ac_cntxt[ac].SLRC,uu_wlan_cap_ctxt_g.ac_cntxt[ac].cw_val,uu_wlan_cap_ctxt_g.ack_timer_val,uu_wlan_cap_ctxt_g.cts_timer_val,uu_wlan_cap_ctxt_g.eifs_timer_val,uu_wlan_cap_ctxt_g.nav_val, uu_wlan_cap_ctxt_g.nav_reset_timer,uu_wlan_cap_ctxt_g.cca_status,uu_wlan_cap_ctxt_g.txop_owner_ac,uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
    if(uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state==UU_WLAN_CAP_STATE_TXOP)
    {	
        printf("%d",uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state);
    }
    else
    {
        printf("0");
    }
    /* validating post conditions */
    if(postcondition[2]==UU_WLAN_CAP_STATE_TXOP)
    {
	    /*Temporary fix:Calling other module functions in cap function calls*/
    }
    else
    {
        if((uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state == postcondition[2]))
        {
            if(postcondition[2] == UU_WLAN_CAP_STATE_TXOP)
            {	
                if(uu_wlan_cap_ctxt_g.txop_sm_cntxt.cur_state->state == postcondition[3])
                {
                     return 0;
                }
                /* post condition of supplimentary s/m not satisfied */
                return -1;
            }
            return 0;
        }
        else
        {
             printf("post conditions failed\n"); 
             return -1;
        }
    }
}

/* cap test handler start here */
uu_int32 uu_wlan_cap_test_handler(uu_int32 event_type, uu_char *buf)
{	
    /*    int precondition[20];
    int postcondition[20];
    */
    char string_type[25][50];
    unsigned char cca_ind[2];
    int return_value;
    int status;
    int channel_list;
    int qos_mode;
    int pre_event_timer_tick_count;
    int post_event_timer_tick_count;
    int tx_ready_ac;
    int input_frames_num;
    char frames_file[300];
    int ii,j;
    static unsigned char mpdu[1000];
    static int len = 0;
    unsigned char frame_input_file[200];
    int ret;
    uu_wlan_tx_frame_info_t	tx_frame_info;
    uu_wlan_cap_event_t	ev;
    uu_wlan_rx_vector_t	rx_vector;
    char input_file_string[130],input_file_string1[300]="",frame_info_file[50];
    FILE *fp ;
    int backoff_cnt=0;

    memset(&frame_info, 0, sizeof(uu_wlan_tx_frame_info_t));

    /* event_type handling here */
    sscanf(buf,"%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s", string_type[0], string_type[1],&event_type, string_type[2],&ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file,string_type[8],frame_info_file);
    frame_info.ac=ac;
    if(input_frames_num > 0)
    {
        fp=fopen(frames_file,"r");
        if(fp==NULL)
        {
            printf("file doesn't exist");
            //return UU_FAILURE;
        }
                    for(ii = 0; ii < input_frames_num; ii++)
                   {
                        fgets(input_file_string, 200, fp);/*here we read line by line from file(individual testcase id and parameters) */
                        strncpy(input_file_string1,input_file_string,strlen(input_file_string)-1);
                        //_fill_ac_queue((unsigned char)ac, input_file_string1);	
                        __umac_buffer_simulator(input_file_string1,frame_info_file);
                    }
                    fclose(fp);
                }
                //_fill_tx_vec_frame_info(frame_info_file);

                switch(event_type)
                {
                    case UU_WLAN_CAP_EV_TIMER_TICK:
                        {
                            /* fill the frame information and put in AC Q */
                            /* post event to CAP Q*/

                            printf("cw min value of ac2 and 3 is%d %d\n",dot11_cwmin_value_ac[2],dot11_cwmin_value_ac[3]);

                            sscanf(buf, "%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %d %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n ", string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file, string_type[8],frame_info_file,string_type[9],string_type[10], &status, string_type[11],&tx_ready_ac, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);

                            printf("JHNASI after reading %d \n", postcondition[2]);
                            if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                            {
                                ret = __validate_postconditions(precondition, ac);
                                if(ret==-1)
                                {
                                    printf("input failed");
                                    return UU_FAILURE;
                                }
                            }
                            __fill_preconditions_in_cap_context(precondition, ac);

                                if(precondition[2] == UU_WLAN_CAP_STATE_IDLE)
                                {
                                    /*Setting default reg values */
                                    //    uu_wlan_set_default_reg_values();
                                    /*Setting Qos mode */
                                    if(qos_mode == 1)
                                    {
                                        uu_dot11_qos_mode_r = 1;
                                    }
                                    else
                                    {
                                        uu_dot11_qos_mode_r = 0;
                                    }
                                    /*SM start with idle state */

                                    uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, precondition[2], event_type);
                                }

                            /** Handling of timer ticks */
                            if(pre_event_timer_tick_count >= 65534)
                            {
                                pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                            }
                            /** End */
                            /** Posting pre event timer ticks */
                            for(ii = 0; ii < pre_event_timer_tick_count; ii++)
                            {
                                uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                            }
                            /** Posting event */
                            //uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char *)&ac, 1);
                            /** Posting post event timer ticks */
                            for(ii = 0; ii < post_event_timer_tick_count; ii++)
                            {
                                uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                            }
                                for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                                {
                                    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                    printf("State m/c current state %d \n", uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
                                    if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                                    {
                                        printf("Inside aifs0 check \n");
                                        backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                        for (j=0; j< backoff_cnt; j++)
                                        {
                                            ev.ev = UU_WLAN_CAP_EV_TIMER_TICK;
                                            uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                        }
                                    }
                                }
                            ret = __validate_postconditions(postcondition, ac);
			       if(ret==-1)
                               {
                                   printf("testcase failed");
                                   return UU_FAILURE;
                                }
                        }
                        break;
                    case UU_WLAN_CAP_EV_TX_READY:
                        {
                            /* fill the frame information and put in AC Q */
                            /* post event to CAP Q*/

                            printf("Back off value is %d %d \n", uu_wlan_cap_ctxt_g.ac_cntxt[2].backoff_val, uu_wlan_cap_ctxt_g.ac_cntxt[3].backoff_val);
                            sscanf(buf, "%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %d %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n",string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file,string_type[8],frame_info_file, string_type[9],string_type[10], &status, string_type[11],&tx_ready_ac, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);
                            printf("JHNASI after reading %d \n", postcondition[2]);
                            if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                            {
                                ret = __validate_postconditions(precondition, ac);
                                if(ret==-1)
                                {
                                    printf("input failed");
                                    return UU_FAILURE;
                                }
                            }
                            __fill_preconditions_in_cap_context(precondition, ac);

                                if(precondition[2] == UU_WLAN_CAP_STATE_IDLE)
                                {
                                    /*Setting default reg values */
                                    //    uu_wlan_set_default_reg_values();
                                    /*Setting Qos mode */
                                    uu_dot11_qos_mode_r = qos_mode;
                                    /*SM start with idle state */
                                    printf("event type is ------------------- %d %d \n", event_type, precondition[2]); 
                                    uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, precondition[2], event_type);
                                }

                            /** Handling of timer ticks */
                            if(pre_event_timer_tick_count >= 65534)
                            {
                                pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                                printf("Jhansi -- Pre event timer ticks %d \n", pre_event_timer_tick_count);
                            }

                            printf("cw min value of ac2 and 3 is%d %d\n",dot11_cwmin_value_ac[2],dot11_cwmin_value_ac[3]);
                            /** End */
                            /** Posting pre event timer ticks */
                            for(ii = 0; ii < pre_event_timer_tick_count; ii++)
                            {
                                uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                            }
                            /** Posting event */
                            uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char *)&ac, 1);
                            /** Posting post event timer ticks */
                            for(ii = 0; ii < post_event_timer_tick_count; ii++)
                            {
                                uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                            }
                                for (;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                                {
                                    printf("Back off value is %d cw value is %d for ac %d\n", uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val, uu_wlan_cap_ctxt_g.ac_cntxt[ac].cw_val,ac);
                                    printf(" Eve tupe %d \n", ev.ev);
                                    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                    printf("main state is %d",uu_wlan_cap_ctxt_g.sm_cntxt.cur_state->state);
                                    if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                                    {
                                        backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                        for (j=0; j< backoff_cnt; j++)
                                        {
                                            ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                                            uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                        }
                                    }
                                }

                            ret = __validate_postconditions(postcondition, ac);
                            if(ret==-1)
                            {
                                printf("testcase failed");
                                return UU_FAILURE;
                            }
                        }      
                        break;
                    case UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL:
                        {
                            /* fill the status in ev */
                            /* post event to CAP Q */
                            sscanf(buf,"%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %d %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n ", string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file, string_type[8], frame_info_file,string_type[9],string_type[10],&status, string_type[11],&channel_list, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);
                            __fill_preconditions_in_cap_context(precondition, ac);
                                if(precondition[2] == UU_WLAN_CAP_STATE_IDLE)
                                {
                                    //        uu_wlan_set_default_reg_values();
                                    /*Setting Qos mode */
                                    if(qos_mode == 1)
                                    {
                                        uu_dot11_qos_mode_r = 1;
                                    }
                                    else
                                    {
                                        uu_dot11_qos_mode_r = 0;
                                    }
                                    /*SM start with idle state */
                                    uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, precondition[2], event_type);
                                }
                            /** Handling of timer ticks */
                            if(pre_event_timer_tick_count >= 65534)
                            {
                                pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                            }
                            /** End */
                            if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                            {
                                ret = __validate_postconditions(precondition, ac);
                                if(ret==-1)
                                {
                                    printf("input failed");
                                    return UU_FAILURE;
                                }
                            }
                            for(ii=0; ii < pre_event_timer_tick_count; ii++)
                            {
                                uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                            }
                            /** Reading cca status from input */
                            cca_ind[0] = (unsigned char)status;
                            /** Reading channel list from the input  */
                            cca_ind[1] = (unsigned char)channel_list;

                            uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char *)cca_ind, 2);

                            for(ii = 0; ii < post_event_timer_tick_count; ii++)
                            {
                                uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                            }
                                for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                                {
                                    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                    if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                                    {
                                        backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                        for (j=0; j< backoff_cnt; j++)
                                        {
                                            ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                                            uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                        }
                                    }
                                }
                            ret = __validate_postconditions(postcondition, ac);
                            if(ret==-1)     
                            {
                                printf("testcase failed");
                                return UU_FAILURE;
                            }
                        }            
                        break;
                    case UU_WLAN_CAP_EV_PHY_RX_ERR:
                        {
                            sscanf(buf, "%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %x %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n ", string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file, string_type[8],frame_info_file,string_type[9], string_type[10],&status, string_type[11],&tx_ready_ac, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);
                            if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                            {
                                ret = __validate_postconditions(precondition, ac);
                                if(ret==-1)
                                {
                                    printf("input failed");
                                    return UU_FAILURE;

                                }
                            }

                            //uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, precondition[15], event_type);
                            __fill_preconditions_in_cap_context(precondition, ac);
                            /** Handling of timer ticks */
                            if(pre_event_timer_tick_count >= 65534)
                            {
                                pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                            }
                            /** End */
                                for(ii=0; ii < pre_event_timer_tick_count; ii++)
                                {
                                    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                                }

                                uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char *)&status, 1);

                                for(ii = 0; ii < post_event_timer_tick_count; ii++)
                                {
                                    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                                }
                                    for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                                    {
                                        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                        if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                                        {
                                            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                            for (j=0; j< backoff_cnt; j++)
                                            {
                                                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                                                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                            }
                                        }
                                    }

                        ret = __validate_postconditions(postcondition, ac);		
                        if(ret==-1)     
                        {
                            printf("testcase failed");
                            return UU_FAILURE;
                        }
                }

                break;
                case UU_WLAN_CAP_EV_RX_END_IND:

                {

                    sscanf(buf, "%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %x %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n ", string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file, string_type[8], frame_info_file,string_type[9],string_type[10],&status, string_type[11],&rets, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);
                    if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                    {
                        ret = __validate_postconditions(precondition, ac);
                        if(ret==-1)
                        {
                            printf("input failed");
                            return UU_FAILURE;
                        }
                    }
                    __fill_preconditions_in_cap_context(precondition, ac);
                    /** Handling of timer ticks */
                    if(pre_event_timer_tick_count >= 65534)
                    {
                        pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                    }
                    /** End */
                        for(ii=0; ii < pre_event_timer_tick_count; ii++)
                        {
                            uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                        }

                        {
                            uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char *)&status, 1);
                        }
                        for(ii = 0; ii < post_event_timer_tick_count; ii++)
                        {
                            uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                        }
                            for(;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                            {
                                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                                {
                                    backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                    for (j=0; j< backoff_cnt; j++)
                                    {
                                        ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                                        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                    }
                                }
                            }
                ret = __validate_postconditions(postcondition, ac);		
                if(ret==-1)     
                {
                    printf("testcase failed");
                    return UU_FAILURE;
                }
            }
            break;
            case UU_WLAN_CAP_EV_RX_START_IND:
            {
                /* fill the rx_vec information */
                /* post event to CAP Q */
                sscanf(buf, "%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %s %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n", string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file, string_type[8],frame_info_file,string_type[9], string_type[10],&status, string_type[11],frame_input_file, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);
                if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                {
                    ret = __validate_postconditions(precondition, ac);
                    if(ret==-1)
                    {
                        printf("input failed");
                        return UU_FAILURE;
                    }
                }
                __fill_preconditions_in_cap_context(precondition, ac);
                /** Handling of timer ticks */
                if(pre_event_timer_tick_count >= 65534)
                {
                    pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                }
                /** End */
                    for(ii = 0; ii < pre_event_timer_tick_count; ii++)
                    {
                        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                    }
                    {
                        len = uu_wlan_test_get_frame_from_file(frame_input_file, mpdu);
                        memset(&rx_vector, 0, sizeof(uu_wlan_rx_vector_t));
                        rx_vector.L_length = (unsigned short)len;
                        rx_vector.L_datarate = (unsigned short)10;//temporarly.
                        rx_vector.is_aggregated = 0;
                        uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char *)&rx_vector, sizeof(uu_wlan_rx_vector_t));
                    }
                    for(ii = 0; ii < post_event_timer_tick_count; ii++)
                    {
                        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                    }
                        for (;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                        {
                            uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                            if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                            {
                                backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                for (j=0; j< backoff_cnt; j++)
                                {
                                    ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                                    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                }
                            }
                        }
                ret = __validate_postconditions(postcondition, ac);
                if(ret==-1)     
                {
                    printf("testcase failed");
                    return UU_FAILURE;
                }
            break;
    case UU_WLAN_CAP_EV_DATA_INDICATION:
            //  case UU_WLAN_CAP_EV_RX_DATA_IND:
            {

                /* form a mpdu and post event to CAP Q */
                sscanf(buf, "%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %s %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n ", string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file, string_type[8],frame_info_file,string_type[9], string_type[10],&status, string_type[11],frame_input_file, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);
                if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                {
                    ret = __validate_postconditions(precondition, ac);
                    if(ret==-1)
                    {
                        printf("input failed");
                        return UU_FAILURE;
                    }
                }
                __fill_preconditions_in_cap_context(precondition, ac);
                /** Handling of timer ticks */
                if(pre_event_timer_tick_count >= 65534)
                {
                    pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                }
                /** End */
                // if(precondition[15]==4||(precondition[15]==3||precondition[16]==2))
                {
                    for(ii = 0; ii < pre_event_timer_tick_count; ii++)
                    {
                        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                    }
                    printf("event type %d\n",(uu_wlan_cap_event_type_t)event_type);
                    {
                        len = uu_wlan_test_get_frame_from_file(frame_input_file, mpdu);
                        uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char*)mpdu, len);
                        len=0;
                    }
                    for(ii = 0; ii < post_event_timer_tick_count; ii++)
                    {
                        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                    }
                        for (;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                        {
                            uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                            if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                            {
                                backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                for (j=0; j< backoff_cnt; j++)
                                {
                                    ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                                    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                }
                            }
                        }
                }	
                ret = __validate_postconditions(postcondition, ac);
                if(ret==-1)     
                {
                    printf("testcase failed");
                    return UU_FAILURE;
                }
            }
            /* else
               {
               goto rx_data_event;
               goto out;
               }*/
            break;

    case UU_WLAN_CAP_EV_TXOP_TX_START:
            {
                    ret = __validate_postconditions(postcondition, ac);
		    if(ret==-1)     
                    {
                        printf("testcase failed");
                        return UU_FAILURE;
                    }
            }
            break;
    default:
            {
                sscanf(buf, "%s %s %d %s %d %s %d %s %d %s %d %s %d %s %s %s %s %s %s %d %s %s %s %s %d %s %d %s %d %s %d %s %s %d %s %d %s %d %s %d \n", string_type[0], string_type[1],&event_type,string_type[2], &ac, string_type[3],&qos_mode, string_type[4],&pre_event_timer_tick_count, string_type[5],&post_event_timer_tick_count, string_type[6],&input_frames_num, string_type[7],frames_file, string_type[8],frame_info_file,string_type[9], string_type[10],&status, string_type[11],frame_input_file, string_type[12], string_type[13],&precondition[0], string_type[14],&precondition[1], string_type[15],&precondition[2], string_type[16],&precondition[3],string_type[17], string_type[18],&postcondition[0], string_type[19],&postcondition[1], string_type[20],&postcondition[2], string_type[21],&postcondition[3]);
                uu_wlan_sm_state_info_t *current_state, *new_state;
                //    uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, (unsigned char*)mpdu, len);
                if(strcmp(string_type[0],"cap_testcase_id1")!=0)
                {
                    ret = __validate_postconditions(precondition, ac);
                    if(ret==-1)
                    {
                        printf("input failed");
                        return UU_FAILURE;
                    }
                }
                    int i;
                    __fill_preconditions_in_cap_context(precondition, ac);
                    /** Handling of timer ticks */
                    if(pre_event_timer_tick_count >= 65534)
                    {
                        pre_event_timer_tick_count = _cal_timer_ticks(pre_event_timer_tick_count, precondition, qos_mode, ac);
                    }
                    /** End */
                        for(ii = 0; ii < pre_event_timer_tick_count; ii++)
                        {
                            uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                        }
                        uu_wlan_put_msg_in_CAP_Q((uu_wlan_cap_event_type_t)event_type, NULL, 0);

                            //     if((precondition[15]==3)&&(event_type!=1))
                                for(ii = 0; ii < post_event_timer_tick_count; ii++)
                                {
                                    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
                                }
                                    for (;uu_wlan_get_msg_from_CAP_Q(&ev) >= 0;)
                                    {
                                        uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                        if ((precondition[2] == UU_WLAN_CAP_STATE_CONTEND) && (uu_wlan_cap_ctxt_g.ac_cntxt[ac].aifs_val == 0))
                                        {
                                            backoff_cnt = uu_wlan_cap_ctxt_g.ac_cntxt[ac].backoff_val;
                                            for (j=0; j< backoff_cnt; j++)
                                            {
                                                ev.ev = UU_WLAN_CAP_EV_TIMER_TICK; 
                                                uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
                                            }
                                        }
                                    }
                    ret = __validate_postconditions(postcondition, ac);
                    if(ret==-1)     
                    {
                        printf("testcase failed");
                        return UU_FAILURE;
                    }
            }
            break;
	}/* switch */
    }   
for(;uu_wlan_get_msg_from_CAP_Q(&ev)==0;)
{
	printf("cap q is not null\n");
}
//event_type=0;
return ret;
}/* uu_wlan_cap_test_handler */

/* phy_stub functions simulating here */

void uu_wlan_phy_txstart_req(unsigned char *data, int len)
{   uu_wlan_cap_event_t			ev;
    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32) UU_WLAN_CAP_EV_TX_START_CONFIRM, (uu_uchar *)&ev);
}

void uu_wlan_phy_data_req(uu_uchar data, uu_uchar *confirm_flag)
{
    uu_wlan_cap_event_t			ev;

    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32) UU_WLAN_CAP_EV_TX_DATA_CONFIRM, (uu_uchar *)&ev);
    //uu_wlan_rx_handle_phy_dataind(data);
}

void uu_wlan_phy_txend_req(void)
{
    uu_wlan_cap_event_t			ev;
    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)UU_WLAN_CAP_EV_TX_END_CONFIRM, (uu_uchar *)&ev);
}

void uu_wlan_phy_ccareset_req(void)
{
    uu_wlan_cap_event_t ev;
    uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32) UU_WLAN_CAP_EV_CCA_RESET_CONFIRM, (uu_uchar *)&ev);
}

static void _fill_tx_vec_frame_info(char *frame_input_file)
{
    /** Enable format = 2 for ht case */
    //frame_info->txvec.format = 2;
    char string[100];
    char line[100];
    int value;
    FILE* fp = NULL;

    uu_wlan_tx_frame_info_t *frame_info=(uu_wlan_tx_frame_info_t *)malloc(sizeof(uu_wlan_tx_frame_info_t));

    memset(frame_info, 0, sizeof(uu_wlan_tx_frame_info_t *));
    fp = fopen(frame_input_file,"r");

    if(fp == NULL)
    {
        printf("\n fopen() Error!!!\n");
        return ;
    }

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s = %d", string, &value);

        if(strcmp(string,"format") == 0)
        {
            frame_info->txvec.format = value;
        }
        else if(strcmp(string,"n_tx")==0)
        {
            frame_info->txvec.n_tx=value;
        }
        else if(strcmp(string,"txpwr_level")==0)
        {
            frame_info->txvec.txpwr_level=value;
        }
        else if(strcmp(string,"reserved1")==0)
        {
            frame_info->txvec.reserved1=value;
        }
        else if(strcmp(string,"L_length")==0)
        {
            frame_info->txvec.L_length=value;
        }
        else if(strcmp(string,"service")==0)
        {
            frame_info->txvec.service=value;
        }
        else if(strcmp(string,"antenna_set")==0)
        {
            frame_info->txvec.antenna_set=value;
        }
        else if(strcmp(string,"num_ext_ss")==0)
        {
            frame_info->txvec.num_ext_ss=value;
        }
        else if(strcmp(string,"no_sig_extn")==0)
        {
            frame_info->txvec.is_no_sig_extn=value;
        }
        else if(strcmp(string,"mcs")==0)
        {
            frame_info->txvec.mcs=value;
        }
        else if(strcmp(string,"txvec.is_long_preamble")==0)
        {
            frame_info->txvec.is_long_preamble=value;
        }
        else if(strcmp(string,"ht_length")==0)
        {
            frame_info->txvec.ht_length=value;
        }
        else if(strcmp(string,"is_smoothing")==0)
        {
            frame_info->txvec.is_smoothing=value;
        }
        else if(strcmp(string,"reserved")==0)
        {
            frame_info->txvec.reserved=value;
        }
        else if(strcmp(string,"is_aggregated")==0)
        {
            frame_info->txvec.is_aggregated=value;
        }
        else if(strcmp(string,"stbc")==0)
        {
            frame_info->txvec.stbc=value;
        }
        else if(strcmp(string,"is_fec_ldpc_coding")==0)
        {
            frame_info->txvec.is_fec_ldpc_coding=value;
        }
        else if(strcmp(string,"is_short_GI")==0)
        {
            frame_info->txvec.is_short_GI=value;
        }
        else if(strcmp(string,"partial_aid")==0)
        {
            frame_info->txvec.partial_aid=value;
        }
        else if(strcmp(string,"is_beamformed")==0)
        {
            frame_info->txvec.is_beamformed=value;
        }
        else if(strcmp(string,"num_users")==0)
        {
            frame_info->txvec.num_users=value;
        }
        else if(strcmp(string,"is_dyn_bw")==0)
        {
            frame_info->txvec.is_dyn_bw=value;
        }
        else if(strcmp(string,"group_id")==0)
        {
            frame_info->txvec.group_id=value;
        }
        else if(strcmp(string,"is_tx_op_ps_allowed")==0)
        {
            frame_info->txvec.is_tx_op_ps_allowed=value;
        }
        else if(strcmp(string,"is_time_of_departure_req")==0)
        {
            frame_info->txvec.is_time_of_departure_req=value;
        }
        else if(strcmp(string,"rts_threshold")==0)
        {
            dot11_rts_threshold_value=value;
        }
        else if(strcmp(string,"short_retry")==0)
        {
          dot11_short_retry_count=value;
        }
        else if(strcmp(string,"long_retry")==0)
        {
          dot11_long_retry_count=value;
        }
        else if(strcmp(string,"slot_timer")==0)
        {
            dot11_slot_timer_value=value;
        }
        else if(strcmp(string,"cw_min_val_0")==0)
        {
            dot11_cwmin_value_ac[0]=value;
        }
        else if(strcmp(string,"cw_min_val_1")==0)
        {
            dot11_cwmin_value_ac[1]=value;
        }
        else if(strcmp(string,"cw_min_val_2")==0)
        {
            dot11_cwmin_value_ac[2]=value;
            printf("cw min value of ac2%d\n",dot11_cwmin_value_ac[2]);
        }
        else if(strcmp(string,"cw_min_val_3")==0)
        {
            dot11_cwmin_value_ac[3]=value;
        }
        else
        {
            printf("");
        }
    }
    fcloseall();
}

static void _fill_tx_vec_default_values(uu_wlan_tx_frame_info_t *frame_info)
{
    /** Enable format = 2 for ht case */
    //frame_info->txvec.format = 2;
    frame_info->txvec.format = 0;
    frame_info->txvec.n_tx=3;
    frame_info->txvec.txpwr_level=7;
    frame_info->txvec.reserved1=1;
    frame_info->txvec.L_length=12;
    frame_info->txvec.service=16;
    frame_info->txvec.antenna_set=8;
    frame_info->txvec.num_ext_ss=2;
    frame_info->txvec.is_no_sig_extn=1;
    frame_info->txvec.mcs=7;
    frame_info->txvec.is_long_preamble=1;
    frame_info->txvec.ht_length=16;
    frame_info->txvec.is_smoothing=1;
    frame_info->txvec.reserved=1;
    frame_info->txvec.is_aggregated=1;
    frame_info->txvec.stbc=2;
    frame_info->txvec.is_fec_ldpc_coding=1;
    frame_info->txvec.is_short_GI=1;
    frame_info->txvec.partial_aid=9;
    frame_info->txvec.is_beamformed=1;
    frame_info->txvec.num_users=2;
    frame_info->txvec.is_dyn_bw=1;
    frame_info->txvec.group_id=6;
    frame_info->txvec.is_tx_op_ps_allowed;
    frame_info->txvec.is_time_of_departure_req=1;
    frame_info->txvec.padding_byte0=0;
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
    if(rets==UU_WLAN_RX_HANDLER_FRAME_ERROR)
    {
        return UU_WLAN_RX_HANDLER_FRAME_ERROR;
    }
    else if(rets==UU_WLAN_RX_HANDLER_NAV_UPDATE)
    {
        return UU_WLAN_RX_HANDLER_NAV_UPDATE;
    }
    else if(rets==UU_WLAN_RX_HANDLER_SEND_RESP)
    {
        return UU_WLAN_RX_HANDLER_SEND_RESP;
    }
    else if(rets==UU_WLAN_RX_HANDLER_ACK_RCVD)
    {
        return UU_WLAN_RX_HANDLER_ACK_RCVD;
    }
    else if(rets==UU_WLAN_RX_HANDLER_CTS_RCVD)
    {
        return UU_WLAN_RX_HANDLER_CTS_RCVD;
    }
    else
        return -1;

} /* uu_wlan_rx_handle_phy_rxend */


/* EOF */

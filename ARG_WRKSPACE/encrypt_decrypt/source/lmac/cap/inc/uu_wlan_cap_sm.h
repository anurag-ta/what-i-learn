/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_cap_sm.h                                        **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                           **
** All rights reserved.                                                       **
** http://www.uurmi.com                                                       **
**                                                                            **
** All information contained herein is property of Uurmi Systems              **
** unless otherwise explicitly mentioned.                                     **
**                                                                            **
** The intellectual and technical concepts in this file are proprietary       **
** to Uurmi Systems and may be covered by granted or in process national      **
** and international patents and are protect by trade secrets and             **
** copyright law.                                                             **
**                                                                            **
** Redistribution and use in source and binary forms of the content in        **
** this file, with or without modification are not permitted unless           **
** permission is explicitly granted by Uurmi Systems.                         **
**                                                                            **
*******************************************************************************/

#ifndef __UU_WLAN_CAP_SM_H__
#define __UU_WLAN_CAP_SM_H__

#include "uu_datatypes.h"

/** Maximum States supported by State Machine */
#define UU_WLAN_SM_MAX_STATES  12

UU_BEGIN_DECLARATIONS


/* Return the State (IF state change required return new state, otherwise returen current state) */
typedef uu_int32 (*action_t)(uu_uchar *sm_cntx, uu_int32 event ,uu_uchar *ev_data);
typedef uu_int32 (*entry_action_t)(uu_uchar *sm_cntx, uu_int32 event);
typedef uu_int32 (*exit_action_t)(uu_uchar *sm_cntx, uu_int32 event);


/** Individual state information structure.
 * It contains entry, exit and action funtions of state.
 * It also contains state info pointer.
 */

typedef struct uu_wlan_sm_state_info
{
    /** State enumerated value 
     * its intialized to state enumerated value while adding state information.
     */
    uu_int32  state;
    /** State info pointer.
     * its intialized to null while adding states information in uu_wlan_cap_sm.c.
     */
    uu_uchar* info;
    /** Action function pointer of state.
     * Its intialized to state action funtion while adding state information in uu_wlan_cap_sm.c.
     */
        action_t  action;
    /** Entry function pointer of state.
     * Its intialized to state entry funtion while adding state information in uu_wlan_cap_sm.c.
     */
    entry_action_t entry_action;
    /**  Exit function pointer of state.
     * Its intialized to state exit funtion while adding state information in uu_wlan_cap_sm.c.
     */
    exit_action_t  exit_action;
} uu_wlan_sm_state_info_t;


/** State machine context structure.
 * It maintains cur_state, prev_state and all states information.
 */
typedef struct uu_wlan_sm_context
{
    /** Current state informtion of state machine.
     * It contains entry, exit and action function pointers of current state.
     */
    uu_wlan_sm_state_info_t *cur_state;
    /** Maintains all states information as an array.
     * All states information added before starting the state machine. 
     */
    uu_wlan_sm_state_info_t states[UU_WLAN_SM_MAX_STATES];
    /** Total no.of added states count.
     * while adding state information, it increments by 1. 
     */
    uu_int32                added_states_cnt;
    /** Optionally holds any user data associated with this state-machine.
     */
    uu_uchar         *data;
} uu_wlan_sm_context_t;



extern uu_wlan_sm_state_info_t* uu_wlan_sm_get_state_info(uu_wlan_sm_context_t *cntx, uu_int32 id);
extern uu_wlan_sm_state_info_t* uu_wlan_sm_add_state(uu_wlan_sm_context_t *cntx, 
                    uu_uint32 id, entry_action_t entry_func, 
                    action_t action, exit_action_t exit_func, 
                    uu_uchar* state_info);
extern uu_void uu_wlan_sm_del_state(uu_wlan_sm_context_t *cntx, uu_int32 id);
extern uu_int32 uu_wlan_sm_start(uu_wlan_sm_context_t *cntx, uu_int32 state_id, uu_int32 event);
extern uu_int32 uu_wlan_sm_run_for_event(uu_wlan_sm_context_t *cntx, uu_uint32 event, uu_uchar *ev_data);
extern uu_int32 uu_wlan_supp_sm_run_for_event(uu_wlan_sm_context_t *cntx, uu_int32 event, uu_uchar *ev_data);


UU_END_DECLARATIONS

#endif /* __UU_WLAN_CAP_SM_H__ */


/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_sm.c                                       **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                          **
 ** All rights reserved.                                                      **
 ** http://www.uurmi.com                                                      **
 **                                                                           **
 ** All information contained herein is property of Uurmi Systems             **
 ** unless otherwise explicitly mentioned.                                    **
 **                                                                           **
 ** The intellectual and technical concepts in this file are proprietary      **
 ** to Uurmi Systems and may be covered by granted or in process national     **
 ** and international patents and are protect by trade secrets and            **
 ** copyright law.                                                            **
 **                                                                           **
 ** Redistribution and use in source and binary forms of the content in       **
 ** this file, with or without modification are not permitted unless          **
 ** permission is explicitly granted by Uurmi Systems.                        **
 **                                                                           **
 ******************************************************************************/

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_fwk_log.h" 
#include "uu_wlan_cap_sm.h"


/*
** Global variables
*/
static uu_wlan_sm_state_info_t uu_wlan_sm_state_info_g[UU_WLAN_SM_MAX_STATES];


uu_wlan_sm_state_info_t*
uu_wlan_sm_get_state_info
	(
		uu_wlan_sm_context_t *cntx,
		uu_int32 state
	)
{
	uu_int16 i;
        for(i=0; i< UU_WLAN_SM_MAX_STATES; i++)
	{
		if(cntx->states[i].state == state)
		{	
			return &cntx->states[i];
		}
	}
	return UU_NULL;
}


uu_wlan_sm_state_info_t* 
uu_wlan_sm_add_state
	(
		uu_wlan_sm_context_t *cntx,
		uu_uint32 state, 
		entry_action_t entry_func, 
		action_t action, 
		exit_action_t exit_func, 
		uu_uchar* state_info
	)
{
	if (entry_func == UU_NULL || action == UU_NULL ||  exit_func == UU_NULL)
	{
		return UU_NULL;
	}
	if (cntx->added_states_cnt >= UU_WLAN_SM_MAX_STATES)
	{
		return UU_NULL;
	}
    
	cntx->states[cntx->added_states_cnt].state = state; 
	cntx->states[cntx->added_states_cnt].action = action;
	cntx->states[cntx->added_states_cnt].entry_action = entry_func;
	cntx->states[cntx->added_states_cnt].exit_action = exit_func;
	cntx->states[cntx->added_states_cnt].info = state_info;

        return &cntx->states[cntx->added_states_cnt++];
}


uu_void uu_wlan_sm_del_state
	(
		uu_wlan_sm_context_t *cntx,
		uu_int32 state
	)
{
	uu_uint16 i;
	for (i=0; i<UU_WLAN_SM_MAX_STATES; i++)
	{
		if(uu_wlan_sm_state_info_g[i].state == state)
		{
			cntx->states[i].state = -1;	
			cntx->states[i].action = UU_NULL;
			cntx->states[i].entry_action = UU_NULL;
			cntx->states[i].exit_action = UU_NULL;
			cntx->states[i].info = UU_NULL;
		}
	}
}


uu_int32 uu_wlan_sm_start(uu_wlan_sm_context_t *cntx, uu_int32 state, uu_int32 event)
{
	uu_wlan_sm_state_info_t *st =(uu_wlan_sm_state_info_t *) uu_wlan_sm_get_state_info( cntx, state);
	if ((!cntx) || (!st))
	{
		UU_WLAN_LOG_ERROR(( KERN_CRIT "Error in entering into state: %d; Null context\n", state));
		return UU_FAILURE;
	}

	cntx->cur_state = st;
	if (st->entry_action((uu_uchar*)cntx, event) != UU_SUCCESS)
	{
		UU_WLAN_LOG_ERROR(( KERN_CRIT "Error in entering into state: %d\n", state));
		return UU_FAILURE;
	}
	return UU_SUCCESS;
}


uu_int32 uu_wlan_sm_run_for_event(uu_wlan_sm_context_t *cntx, uu_uint32 event, uu_uchar *ev_data)
{
	uu_int32 ret = UU_SUCCESS;
	uu_int32 ret_state;
	uu_wlan_sm_state_info_t *current_state, *new_state;

	if ((!cntx) || (cntx->cur_state == UU_NULL))
	{
		UU_WLAN_LOG_ERROR((KERN_CRIT "Error- NULL state in handling of event: %d\n", event));
		return UU_FAILURE;
	}

	current_state = cntx->cur_state;
	//UU_WLAN_LOG_DEBUG(("LMAC:main s/m current state %d\n", current_state->state));
	ret_state = current_state->action((uu_uchar*)cntx, event, ev_data);  /* use previous state here */
	new_state = (uu_wlan_sm_state_info_t *)uu_wlan_sm_get_state_info(cntx, ret_state);
	if (new_state != current_state)
	{
		UU_WLAN_LOG_DEBUG(("LMAC:changing to new state %d\n", new_state->state));
		ret = current_state->exit_action((uu_uchar*)cntx, event);
		if (ret != UU_SUCCESS)
		{
			UU_WLAN_LOG_ERROR((KERN_CRIT "Error exiting state: %d (event: %d, new state: %d)\n", current_state->state, event, ret_state));
			return UU_FAILURE;
		}

		ret = new_state->entry_action((uu_uchar*)cntx, event);
		if (ret != UU_SUCCESS)
		{
			UU_WLAN_LOG_ERROR((KERN_CRIT "Error entering state: %d (event: %d, from state: %d)\n", ret_state, event, current_state->state));
			return UU_FAILURE;
		}

		cntx->cur_state = new_state;
	}

	return ret;
} /* uu_wlan_sm_run_for_event */


/* EOF */ 


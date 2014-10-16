/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_lmac_init.c                                    **
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

/*
 *  LMAC Initialization
 */

#ifdef __KERNEL__
#include <linux/kthread.h>
#include <linux/delay.h>
#endif

#include "uu_datatypes.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_fwk_lock.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_beacon.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_lmac_dev.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_lmac_sta_info.h"

#ifdef PHY_STUB
#include "uu_phy_stub_nl.h"
#endif

/*
** Global variables
*/

uu_wlan_lmac_device_t   uu_wlan_lmac_device_g;

uu_wlan_spin_lock_type_t uu_wlan_cap_Q_lock_g;

uu_wlan_spin_lock_type_t uu_wlan_ac_q_size_lock_g;
static  uu_int8 uu_wlan_started_g = 0;

/* TODO: Get rid of this, after checking the init path */
static  uu_wlan_op_mode_t cur_mode_g;

uu_lmac_ops_t *lmac_ops_gp;

int uu_lmac_reg_ops(uu_lmac_ops_t *ops)
{
    if (ops != NULL)
        lmac_ops_gp = ops;
    return 0;
}
EXPORT_SYMBOL(uu_lmac_reg_ops);

int uu_lmac_dereg_ops(void)
{
    lmac_ops_gp = NULL;
    return 0;    
}
EXPORT_SYMBOL(uu_lmac_dereg_ops);


#ifdef PHY_STUB

static uu_int32 uu_wlan_phy_stub_cbk(uu_int32 ind, uu_uchar *data, uu_int32 len)
{
    uu_int32 i;
    uu_wlan_rx_vector_t rx_vec;

    uu_wlan_cap_event_type_t ev_type = (uu_wlan_cap_event_type_t)ind;

    UU_WLAN_LOG_DEBUG((" LMAC: uu_wlan_phy_stub_cbk and len is %d\n", len));

    switch(ev_type)
    {
        case UU_WLAN_CAP_EV_RX_START_IND:
            {
                if ((!data) || (len <=0) || (len != sizeof(uu_wlan_rx_vector_t)))
                {
                    return UU_FAILURE;
                }
                UU_WLAN_COPY_BYTES(&rx_vec, data, len);
                uu_wlan_rx_handle_phy_rxstart(&rx_vec);
            }
            break;
        case UU_WLAN_CAP_EV_RX_END_IND:
            {
                if ((!data) || (len <=0) || (len != 1))
                {
                    return UU_FAILURE;
                }
                uu_wlan_rx_handle_phy_rxend(*data);
                return UU_SUCCESS;
            }
            break;
        case UU_WLAN_CAP_EV_DATA_INDICATION:
            {
                if ((!data) || (len <=0) || (len > UU_WLAN_MAX_AMPDU_SZ))
                {
                    return UU_FAILURE;
                }
                for (i = 0; i < len; i++)
                {
                    uu_wlan_rx_handle_phy_dataind(data[i]);
                }
            }
            break;
        default: /* No data expected for other events */
            {
            }
            break;
    } /* switch */
    return uu_wlan_put_msg_in_CAP_Q(ind, data, len);
}

phy_stub_proto_ops_t phy_ops_g =
{
    .PhyIndCBK = uu_wlan_phy_stub_cbk
};
#endif // PHY_STUB


uu_int32 uu_wlan_lmac_init(uu_void)
{
    UU_WLAN_SPIN_LOCK_INIT(&uu_wlan_ba_ses_context_lock_g);
    UU_WLAN_SPIN_LOCK_INIT(&uu_wlan_cap_Q_lock_g);

    cur_mode_g = UU_WLAN_IEEE80211_OP_MODE_R;
#ifdef UU_WLAN_TSF /*TODO: Yet to implement in RTL */
    uu_wlan_tsf_r = 0;
#endif
#ifdef PHY_STUB
    uu_phy_stub_nl_reg_ops(&phy_ops_g);
#endif
    return UU_SUCCESS;
}
EXPORT_SYMBOL(uu_wlan_lmac_init);


uu_int32 uu_wlan_lmac_shutdown(uu_void)
{
    return UU_SUCCESS;
}
EXPORT_SYMBOL(uu_wlan_lmac_shutdown);



uu_int32 uu_wlan_lmac_start(uu_void)
{
    /* TODO: Why is this set in both init & start? */
    cur_mode_g = UU_WLAN_IEEE80211_OP_MODE_R;
    if (uu_wlan_cap_init() != UU_SUCCESS)
        return UU_FAILURE;
    if (UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_MASTER ||  (cur_mode_g == UU_WLAN_MODE_ADHOC) || (cur_mode_g == UU_WLAN_MODE_MESH))
    {
        if (uu_wlan_beacon_timer_start() != UU_SUCCESS)
        {
            return UU_FAILURE;
        }
    }

    uu_wlan_started_g = 1;
    return UU_SUCCESS;
}
EXPORT_SYMBOL(uu_wlan_lmac_start);


uu_int32 uu_wlan_lmac_stop(uu_void)
{
    uu_wlan_started_g = 0;
    if ((cur_mode_g == UU_WLAN_MODE_MASTER) || (cur_mode_g == UU_WLAN_MODE_ADHOC) || (cur_mode_g == UU_WLAN_MODE_MESH))
    {
        if (uu_wlan_beacon_timer_stop() != UU_SUCCESS)
            return UU_FAILURE;
    }
    uu_wlan_cap_shutdown();
    return UU_SUCCESS;
}
EXPORT_SYMBOL(uu_wlan_lmac_stop);



uu_int32 uu_wlan_lmac_mode_switch(uu_void)
{
    /* TODO: Restart the CAP handler also */
    /* Stop based on old mode */
    if ((cur_mode_g == UU_WLAN_MODE_MASTER) || (cur_mode_g == UU_WLAN_MODE_ADHOC) || (cur_mode_g == UU_WLAN_MODE_MESH))
    {
        if (uu_wlan_beacon_timer_stop() != UU_SUCCESS)
            return UU_FAILURE;
    }

    /* Start based on new mode */
    cur_mode_g = UU_WLAN_IEEE80211_OP_MODE_R;
    if ((cur_mode_g == UU_WLAN_MODE_MASTER) || (cur_mode_g == UU_WLAN_MODE_ADHOC) || (cur_mode_g == UU_WLAN_MODE_MESH))
    {
        if (uu_wlan_beacon_timer_start() != UU_SUCCESS)
            return UU_FAILURE;
    }

    uu_wlan_started_g = 1;
    return UU_SUCCESS;
}
EXPORT_SYMBOL(uu_wlan_lmac_mode_switch);


#if 0 /* No longer using config-thread */
#define MODULE_NAME  "UU_CONFIG_THRD"


static uu_void uu_wlan_conf_process_req(uu_void)
{
    switch(UU_REG_LMAC_CONF_REQUEST)
    {
        case UU_REG_CONF_BEACON_INFO_UPDATED:
        {
            UU_WLAN_LOG_INFO(("LMAC: Processing Beacon info change request\n Not handled now"));
        }
        break;

#if 0 /* Now doing it as part of station management */
        case UU_REG_CONF_BA_SES_ADD:
        {
            uu_wlan_cp_rq_t rq;

            rq.rq_type = UU_WLAN_CP_RQ_ADD_BA_SESSION;
            rq.rq_info.ba.sta_addr[0] = UU_WLAN_GET_BYTE1_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[1] = UU_WLAN_GET_BYTE2_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[2] = UU_WLAN_GET_BYTE3_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[3] = UU_WLAN_GET_BYTE4_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[4] = UU_WLAN_GET_BYTE1_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V4);
            rq.rq_info.ba.sta_addr[5] = UU_WLAN_GET_BYTE2_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V4);
            rq.rq_info.ba.buff_size = UU_REG_LMAC_CONF_RQ_32BIT_V2;
            rq.rq_info.ba.tid = UU_REG_LMAC_CONF_RQ_8BIT_V1;
            //rq.rq_info.ba.start_seq = UU_REG_LMAC_CONF_RQ_8BIT_V2;
            rq.rq_info.ba.start_seq = UU_WLAN_EXTRACT_BITS(UU_REG_LMAC_CONF_RQ_32BIT_V4, 17, 16);
            rq.rq_info.ba.ba_ses_type = UU_REG_LMAC_CONF_RQ_32BIT_V1;
            //uu_wlan_put_msg_in_CP_Q(&rq);
            uu_wlan_cp_handler(&rq);
            UU_WLAN_LOG_DEBUG((KERN_INFO MODULE_NAME": Processing BA Session add request. "
                "STA %x:%x:%x:%x:%x:%x  TID:%x\n", rq.rq_info.ba.sta_addr[0],
                rq.rq_info.ba.sta_addr[1], rq.rq_info.ba.sta_addr[2],
                rq.rq_info.ba.sta_addr[3], rq.rq_info.ba.sta_addr[4],
                rq.rq_info.ba.sta_addr[5], rq.rq_info.ba.tid));
        }
        break;

        case UU_REG_CONF_BA_SES_DEL:
        {
            uu_wlan_cp_rq_t rq;

            rq.rq_type = UU_WLAN_CP_RQ_DEL_BA_SESSION;
            rq.rq_info.ba.sta_addr[0] = UU_WLAN_GET_BYTE1_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[1] = UU_WLAN_GET_BYTE2_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[2] = UU_WLAN_GET_BYTE3_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[3] = UU_WLAN_GET_BYTE4_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V3);
            rq.rq_info.ba.sta_addr[4] = UU_WLAN_GET_BYTE1_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V4);
            rq.rq_info.ba.sta_addr[5] = UU_WLAN_GET_BYTE2_FROM_WORD(UU_REG_LMAC_CONF_RQ_32BIT_V4);
            rq.rq_info.ba.buff_size = UU_REG_LMAC_CONF_RQ_32BIT_V2;
            rq.rq_info.ba.tid = UU_REG_LMAC_CONF_RQ_8BIT_V1;
            rq.rq_info.ba.start_seq = UU_REG_LMAC_CONF_RQ_8BIT_V2;
            rq.rq_info.ba.ba_ses_type = UU_REG_LMAC_CONF_RQ_32BIT_V1;
            UU_WLAN_LOG_DEBUG((KERN_INFO MODULE_NAME": Processing BA Session delete request. "
                "STA %x:%x:%x:%x:%x:%x  TID:%x\n", rq.rq_info.ba.sta_addr[0],
                rq.rq_info.ba.sta_addr[1], rq.rq_info.ba.sta_addr[2],
                rq.rq_info.ba.sta_addr[3], rq.rq_info.ba.sta_addr[4],
                rq.rq_info.ba.sta_addr[5], rq.rq_info.ba.tid));
            //uu_wlan_put_msg_in_CP_Q(&rq);
            uu_wlan_cp_handler(&rq);
        }
        break;

        case UU_REG_CONF_CH_CHANGE:
        {
            UU_WLAN_LOG_INFO((KERN_INFO MODULE_NAME": Processing Channel switch request\n Not handled now"));
        }
        break;
#endif

        case UU_REG_CONF_NO_CHANGE:
        {
            UU_WLAN_LOG_ERROR((KERN_INFO MODULE_NAME": Invalid Configuration change request\n"));
        }
        break;
    }
    return;
} /* uu_wlan_conf_process_req */




#ifdef LMAC_UMAC_TX_THREAD
static uu_wlan_tx_thrd_info_t *tx_thrd_gp;
#endif

static void uu_wlan_config_handler_thread(void)
{
    config_thrd_gp->running = 1;

    /* main loop */
    while(!config_thrd_gp->stop)
    {
        if (UU_REG_LMAC_CONF_UPDATE_PENDING)
        {
            UU_REG_LMAC_CONF_BUSY = 1;
            uu_wlan_conf_process_req();
            UU_REG_LMAC_CONF_BUSY = 0;
            UU_REG_LMAC_CONF_UPDATE_PENDING = 0;
        }
        msleep(10);
    }

    //config_thrd_gp->thread = NULL;
    config_thrd_gp->running = 0;
}


static uu_int32  uu_wlan_config_handler_start(uu_void)
{
    config_thrd_gp = (uu_wlan_config_thrd_info_t *)uu_wlan_alloc_memory(sizeof(uu_wlan_config_thrd_info_t));
    memset(config_thrd_gp, 0, sizeof(uu_wlan_config_thrd_info_t));

    /* start kernel thread */
#ifdef CONFIG_THREAD
    config_thrd_gp->thread = kthread_run((void *)uu_wlan_config_handler_thread, NULL, MODULE_NAME);
#endif
    if (IS_ERR(config_thrd_gp->thread))
    {
        UU_WLAN_LOG_ERROR((KERN_INFO MODULE_NAME": unable to start kernel thread\n"));
        uu_wlan_free_memory(config_thrd_gp);
        config_thrd_gp = NULL;
        return -ENOMEM;
    }

    return 0;
}

static uu_void  uu_wlan_config_handler_stop(uu_void)
{
    if (config_thrd_gp == NULL)
        return;

    if (config_thrd_gp->thread!= NULL)
    {
        config_thrd_gp->stop = 1;
        while (config_thrd_gp->running == 1)
        {
            msleep(100);
        }
    }

    uu_wlan_free_memory(config_thrd_gp);
    config_thrd_gp = NULL;
}
#endif /* #if 0 -- No longer using config-thread */


#if 0 /* No longer using intermediate Tx Q & Thread */
static void uu_wlan_tx_handler_thread(void)
{
    tx_thrd_gp->running = 1;

    /* main loop */
    while(!tx_thrd_gp->stop)
    {
        if (uu_wlan_tx_handler(NULL) < 0)
        {
            msleep(20);
        }
        else
        {
            //msleep(1);
        }
    }

    //config_thrd_gp->thread = NULL;
    tx_thrd_gp->running = 0;
}



static uu_int32  uu_wlan_tx_handler_start(uu_void)
{
    tx_thrd_gp = uu_wlan_alloc_memory(sizeof(uu_wlan_tx_thrd_info_t));
    memset(tx_thrd_gp, 0, sizeof(uu_wlan_tx_thrd_info_t));

    /* start kernel thread */
    tx_thrd_gp->thread = kthread_run((void *)uu_wlan_tx_handler_thread, NULL, MODULE_NAME);
    if (IS_ERR(tx_thrd_gp->thread))
    {
        UU_WLAN_LOG_ERROR((KERN_INFO MODULE_NAME": unable to start kernel thread\n"));
        uu_wlan_free_memory(tx_thrd_gp);
        tx_thrd_gp = NULL;
        return -ENOMEM;
    }

    return 0;
}

static uu_void  uu_wlan_tx_handler_stop(uu_void)
{
    if (tx_thrd_gp == NULL)
        return;

    if (tx_thrd_gp->thread!= NULL)
    {
        tx_thrd_gp->stop = 1;
        while (tx_thrd_gp->running == 1)
        {
            msleep(100);
        }
    }

    uu_wlan_free_memory(tx_thrd_gp);
    tx_thrd_gp = NULL;

    UU_WLAN_LOG_DEBUG((KERN_INFO MODULE_NAME": Config thread stopped successfully\n"));
}
#endif /* No longer using intermediate Tx Q & Thread */


#define MODULE_NAME  "UU_CONFIG"

/* TODO: Move this functionality to UMAC */
uu_int32 uu_wlan_lmac_alloc_sta(uu_uint8 *buf)
{
    uu_int32    ret = UU_FAILURE;
    uu_wlan_lmac_config_t    *cfg = (uu_wlan_lmac_config_t*)buf;
    uu_wlan_asso_sta_info_t  *sta = NULL;
    uu_uint8* addr_p = cfg->cmd_data.asso_add.addr;
    uu_uint8   i, j;

    /* Multicast addres should not be allocated entry in station info table */
    if (isMulticastAddr(addr_p) || isNullAddr(addr_p))
    {
        ret = UU_FAILURE;
        goto out;
    }

    /* Check whether any free station entry is available */
    for (i = 0; i<UU_WLAN_MAX_ASSOCIATIONS; i++)
    {
        if (!(uu_wlan_sta_info_is_used_g >> i) & 0x01)
        {
            sta = &(uu_sta_info_table_g[i]);

            /* Filling the station information */
            sta->aid = cfg->cmd_data.asso_add.aid;
            sta->ampdu_exponent = cfg->cmd_data.asso_add.ampdu_exponent;
            sta->ch_bndwdth = cfg->cmd_data.asso_add.ch_bndwdth;
            sta->basic_mcs = cfg->cmd_data.asso_add.basic_mcs;
            copyAddr (sta->bssid, cfg->cmd_data.asso_add.bssid);
            sta->ampdu_min_spacing = cfg->cmd_data.asso_add.ampdu_min_spacing;

            /* set Default values */
            for (j = 0; j < UU_WLAN_MAX_TID ; j++)
            {
                uu_wlan_ba_info_is_used_g &= ~(BIT(sta->ba_cnxt[0][j]));
                uu_wlan_ba_info_is_used_g &= ~(BIT(sta->ba_cnxt[1][j]));

                sta->ba_cnxt[0][j] = UU_WLAN_INVALID_BA_SESSION;
                sta->ba_cnxt[1][j] = UU_WLAN_INVALID_BA_SESSION;
            }

            /* Update the MAC address */
            UU_COPY_MAC_ADDR(uu_sta_addr_index_g[i].addr, addr_p);

            /* Update the used bit */
            uu_wlan_sta_info_is_used_g |= 1 << i;
            ret = UU_SUCCESS;
            break;
        }
    }

out:
    return ret;
} /* uu_wlan_lmac_alloc_sta */


uu_void uu_wlan_lmac_free_sta(const uu_uint8 *addr_p)
{
    uu_uint8  i, j;
    uu_wlan_asso_sta_info_t  *sta = UU_NULL;

    for (i = 0; i<UU_WLAN_MAX_ASSOCIATIONS; i++)
    {
        if (uu_wlan_sta_info_is_used_g & BIT(i) && isaddrsame(uu_sta_addr_index_g[i].addr, addr_p))
        {
            /* Found a valid station entry for this station address */
            /* Clear the used bit */
            uu_wlan_sta_info_is_used_g &= ~(BIT(i));

            sta = &(uu_sta_info_table_g[i]);

            /* Block Ack */
            for (j = 0; j < UU_WLAN_MAX_TID ; j++)
            {
                uu_wlan_ba_info_is_used_g &= ~(BIT(sta->ba_cnxt[0][j]));
                uu_wlan_ba_info_is_used_g &= ~(BIT(sta->ba_cnxt[1][j]));

                sta->ba_cnxt[0][j] = UU_WLAN_INVALID_BA_SESSION;
                sta->ba_cnxt[1][j] = UU_WLAN_INVALID_BA_SESSION;
            }
            break;
        }
    }

    UU_WLAN_LOG_DEBUG(("FREE STA\n"));
    return;
} /* uu_wlan_lmac_free_sta */


/* TODO: Move this functionality to UMAC */
static uu_uint16 uu_wlan_lmac_get_new_ba_session(uu_void)
{
    uu_uint8              i;

    /* Find a slot for the new BA session */
    for(i = 0; i<UU_WLAN_MAX_BA_SESSIONS; i++)
    {
        if(!((uu_wlan_ba_info_is_used_g>>i) & 0x01))
        {
            /* empty slot found */
            return i;
        }
    }

    return UU_WLAN_INVALID_BA_SESSION;
} /* uu_wlan_lmac_get_new_ba_session */


/* TODO: Move this functionality to UMAC */
uu_int32 uu_wlan_lmac_sb_init(uu_wlan_ba_sb_t *sb_p, uu_uint16 buffsz, uu_uint16 ssn)
{
   assert(buffsz <= BA_BASIC_BITMAP_BUF_SIZE);
   memset(sb_p, 0, sizeof(sb_p->bitmap));
   sb_p->winstart = (ssn & MPDU_SEQN_MASK);
   sb_p->winsize = buffsz ;
   /* wrap around is considered only when WinStart exceeds 4095 */
   sb_p->winend =  (sb_p->winstart + sb_p->winsize - 1);
   // sb_p->winend =  (sb_p->winstart + sb_p->winsize - 1) & MPDU_SEQN_MASK; /* To handle wraparound */

   sb_p->buf_winstart = 0;
   return 0;
}


/* TODO: Move this functionality to UMAC */
uu_int32 uu_wlan_lmac_add_ba(uu_uint8 tid, uu_uchar* addr, uu_uint16 bufsz, uu_uint16 ssn, uu_uint8 type, uu_bool dir)
{
    uu_wlan_asso_sta_info_t*  sta;
    uu_uint16 ba_ses;
    uu_int32 ret = UU_SUCCESS;

    sta = uu_wlan_lmac_get_sta(addr);
    if (sta == NULL)
    {
        UU_WLAN_LOG_INFO(("STA: Station information not found\n"));
        ret = UU_FAILURE;
        goto out;
    }

    if (sta->ba_cnxt[dir][tid] != UU_WLAN_INVALID_BA_SESSION)
    {
        UU_WLAN_LOG_INFO(("STA: BA session already existing\n"));
        ret = UU_FAILURE;
        goto out;
    }

    ba_ses = uu_wlan_lmac_get_new_ba_session();
    if (ba_ses == UU_WLAN_INVALID_BA_SESSION)
    {
        /* NOTE: Must not happen. UMAC responsibility */
        UU_WLAN_LOG_INFO(("STA: BA slot are not available\n"));
        ret = UU_FAILURE;
        goto out;
    }

    uu_ba_table_g[ba_ses].ses.type = type;

    sta->ba_cnxt[dir][tid] = ba_ses;

    uu_wlan_ba_info_is_used_g |= BIT(ba_ses);

    uu_wlan_lmac_sb_init(&uu_ba_table_g[ba_ses].ses.sb, bufsz, ssn);

out:
    return ret;
} /* uu_wlan_lmac_add_ba */


/* TODO: Move this functionality to UMAC */
uu_void uu_wlan_lmac_del_ba(uu_uint8 tid, uu_uchar* addr, uu_bool dir)
{
    uu_wlan_asso_sta_info_t  *sta;

    /* Clear the information in the station entry and mark BA context as free */
    sta = uu_wlan_lmac_get_sta(addr);
    if ((sta != NULL) && (sta->ba_cnxt[dir][tid] != UU_WLAN_INVALID_BA_SESSION))
    {
        uu_wlan_ba_info_is_used_g &= ~(BIT(sta->ba_cnxt[dir][tid]));
        sta->ba_cnxt[dir][tid] = UU_WLAN_INVALID_BA_SESSION;
    }

    return;
} /* uu_wlan_lmac_del_ba */


/* TODO: Move this functionality to UMAC */
uu_int32 uu_wlan_lmac_config(uu_void* buf)
{
    uu_int8   ret = UU_SUCCESS;
    uu_wlan_lmac_config_t    *cfg = (uu_wlan_lmac_config_t*)buf;

    switch (cfg->cmd_type)
    {
        case UU_WLAN_LMAC_CONFIG_ASSO_ADD:
        {
            if (uu_wlan_lmac_alloc_sta(buf) == UU_FAILURE)
            {
                UU_WLAN_LOG_ERROR((KERN_INFO MODULE_NAME": Station Slots are not available\n"));
                ret = UU_FAILURE;
                goto out;
			}
            break;
        }
        case UU_WLAN_LMAC_CONFIG_ASSO_DEL:
        {
            uu_wlan_lmac_free_sta(cfg->cmd_data.asso_del.addr);
            break;
        }
        case UU_WLAN_LMAC_CONFIG_BA_SESS_ADD:
        {
            printk("LMAC-ADDBA: STA Addr: %x:%x:%x:%x:%x:%x, TID:%d, Buffer Size:%d, SSN=%d, type=%d, dir:%d\n",
                        cfg->cmd_data.addba.addr[0], cfg->cmd_data.addba.addr[1], cfg->cmd_data.addba.addr[2], cfg->cmd_data.addba.addr[3],
                        cfg->cmd_data.addba.addr[4], cfg->cmd_data.addba.addr[5], cfg->cmd_data.addba.tid, cfg->cmd_data.addba.bufsz,
                        cfg->cmd_data.addba.ssn, cfg->cmd_data.addba.type, cfg->cmd_data.addba.is_dir_tx);
            ret =  uu_wlan_lmac_add_ba(cfg->cmd_data.addba.tid, cfg->cmd_data.addba.addr, cfg->cmd_data.addba.bufsz, cfg->cmd_data.addba.ssn, cfg->cmd_data.addba.type,
                                     cfg->cmd_data.addba.is_dir_tx);
            break;
        }

        case UU_WLAN_LMAC_CONFIG_BA_SESS_DEL:
        {
            printk("LMAC-DELBA:  STA Addr: %x:%x:%x:%x:%x:%x, TID:%d \n",
                cfg->cmd_data.delba.addr[0], cfg->cmd_data.delba.addr[1], cfg->cmd_data.delba.addr[2], cfg->cmd_data.delba.addr[3],
                cfg->cmd_data.delba.addr[4], cfg->cmd_data.delba.addr[5], cfg->cmd_data.delba.tid);
            uu_wlan_lmac_del_ba(cfg->cmd_data.addba.tid, cfg->cmd_data.addba.addr, cfg->cmd_data.addba.is_dir_tx);
            break;
        }

        default:
            UU_WLAN_LOG_ERROR((KERN_INFO MODULE_NAME": Invalid command\n"));
    }
out:
    return ret;
} /* uu_wlan_lmac_config */
EXPORT_SYMBOL(uu_wlan_lmac_config);

MODULE_AUTHOR("UURMI Systems");
MODULE_DESCRIPTION("Support for uurmi 802.11ac wireless LAN cards.");
MODULE_SUPPORTED_DEVICE("uurmi 802.11ac WLAN cards");
MODULE_LICENSE("Dual BSD/GPL");


static uu_int32 __init uu_lmac_init(uu_void)
{
    return 0;
}
module_init(uu_lmac_init);


/** Module exit function. */
static uu_void __exit uu_lmac_exit(uu_void)
{
    UU_WLAN_LOG_DEBUG((" Driver unloaded\n"));
} /* uu_wlan_exit */
module_exit(uu_lmac_exit);

/* EOF */


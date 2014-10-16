/** HEADERS */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "uu_datatypes.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_fwk_lock.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_lmac_sta_info.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_frame.h"
#include "uu_mac_drv.h"
#include "uu_wlan_main.h"
#include "uu_wlan_mac_stub.h"
#include "uu_wlan_msgq.h"

static uu_uint16  seq_no;
static uu_uchar uu_wlan_multdata;

/* ASCII to HEX conversion
 *The data received from netlink socket will be of ASCII format and hence it is converted to HEX */
static void ascii_to_hex(uu_uchar *data)
{
    uu_uchar num = *data;
    if((num >= 0x30 ) && (num <= 0x39))
    {
        num = num - 0x30;
        *data = num;
    }
    else if((num >= 0x61) && (num <= 0x66))
    {
        if(num == 0x61)
        {
            num = 0x0a;
        }
        else if(num == 0x62)
        {
            num = 0x0b;
        }
        else if(num == 0x63)
        {
            num = 0x0c;
        }
        else if(num == 0x64)
        {
            num = 0x0d;
        }
        else if(num == 0x65)
        {
            num = 0x0e;
        }
        else if(num == 0x66)
        {
            num = 0x0f;
        }
        else
        {
            num = 0;
        }

        *data = num;
    }
    else
    {
    }
} /* ascii_to_hex */

/* Contains all the registers passed from UMAC to LMAC. 
   These registers are taken from umac/src/uu_wlan_reg_init.c */
typedef struct default_registers
{
    uu_uchar dot11_slot_timer_value;
    uu_uchar dot11_sifs_timer_value;
    uu_uchar dot11_difs_value;
    uu_uchar dot11_aPhyRxStartDelay;
    uu_uchar dot11_ack_timer_value;
    uu_uchar dot11_cts_timer_value;
    uu_uchar dot11_short_retry_count;
    uu_uchar dot11_long_retry_count;
    uu_uchar dot11_cwmin_value;
    uu_uchar dot11_cwmax_value;
    uu_uchar dot11_cwmin_value_ac[5];
    uu_uchar dot11_cwmax_value_ac[5];
    uu_uint32 dot11_txop_limit_value[5];
    uu_uchar dot11_aifs_value[5];
    uu_uchar uu_dot11_op_mode_r;
    uu_uchar uu_dot11_qos_mode_r;
    uu_uchar uu_dot11_reserved;
    uu_uint32 dot11_BeaconInterval; 
    uu_uchar dot11_signal_extension;
    uu_uchar uu_dot11_self_cts_r;
    uu_uchar dot11_ack_failure_count_g;
    uu_uchar dot11_rts_failure_count_g;
    uu_uchar dot11_rts_success_count_g;
    uu_uchar dot11_fcs_error_count_g;
    uu_uchar uu_wlan_rx_frames_g;
    uu_uchar uu_wlan_rx_multicast_cnt_g;
    uu_uchar uu_wlan_rx_broadcast_cnt_g;
    uu_uchar uu_wlan_rx_frame_for_us_g;
    uu_uchar uu_wlan_rx_ampdu_frames_g;
    uu_uchar uu_wlan_rx_ampdu_subframes_g;
    uu_uchar uu_wlan_rx_phy_err_pkts_g; 
    uu_uchar uu_dot11_reserved_one[3];
    uu_uint16 dot11_rts_threshold_value;
    uu_uint16 BSSBasicRateSet;
    uu_uchar uu_dot11_sta_mac_addr_r[6];
    uu_uchar uu_dot11_sta_bssid_r[6];
#if 0 /** TSF relate dregisters */
    uu_uint64 uu_wlan_tsf_r;
    uu_uint8 tsf_unlock_r;
#endif
    uu_uint8 uu_wlan_last_beacon_tx_r;
    uu_uint8 uu_wlan_phy_tx_delay_r;
#if 1 /** Power save registers */
    uu_uchar uu_wlan_ps_mode_r;
    uu_uchar uu_wlan_uapsd_mode_r;
#endif
    uu_uchar uu_wlan_multdata_r;
}default_registers_t; /* default_registers */


/** Setting default register values. These values are obtained from user space application.
  The default register values are taken from umac/src/uu_wlan_reg_init.c */
int set_default_registers(unsigned char *data)
{
    default_registers_t *reg = NULL;
    reg=(default_registers_t *)data;
    dot11_slot_timer_value=reg->dot11_slot_timer_value;
    dot11_sifs_timer_value=reg->dot11_sifs_timer_value;
    dot11_difs_value=reg->dot11_difs_value;
    dot11_aPhyRxStartDelay=reg->dot11_aPhyRxStartDelay;
    dot11_ack_timer_value=reg->dot11_ack_timer_value;
    dot11_cts_timer_value=reg->dot11_cts_timer_value;
    dot11_short_retry_count=reg->dot11_short_retry_count;
    dot11_long_retry_count=reg->dot11_long_retry_count;
    dot11_cwmin_value=reg->dot11_cwmin_value;
    dot11_cwmax_value=reg->dot11_cwmax_value;
    /* For AC_BK */
    dot11_cwmin_value_ac[0]=reg->dot11_cwmin_value_ac[0];
    dot11_cwmax_value_ac[0]=reg->dot11_cwmax_value_ac[0];
    dot11_txop_limit_value[0]=reg->dot11_txop_limit_value[0];
    dot11_aifs_value[0]=reg->dot11_aifs_value[0];
    /* For AC_BE */
    dot11_cwmin_value_ac[1]=reg->dot11_cwmin_value_ac[1];
    dot11_cwmax_value_ac[1]=reg->dot11_cwmax_value_ac[1];
    dot11_txop_limit_value[1]=reg->dot11_txop_limit_value[1];
    dot11_aifs_value[1]=reg->dot11_aifs_value[1];
    /* For AC_VI */
    dot11_cwmin_value_ac[2]=reg->dot11_cwmin_value_ac[2];
    dot11_cwmax_value_ac[2]=reg->dot11_cwmax_value_ac[2];
    dot11_txop_limit_value[2]=reg->dot11_txop_limit_value[2];
    dot11_aifs_value[2]=reg->dot11_aifs_value[2];
    /* For AC_VO */
    dot11_cwmin_value_ac[3]=reg->dot11_cwmin_value_ac[3];
    dot11_cwmax_value_ac[3]=reg->dot11_cwmax_value_ac[3];
    dot11_txop_limit_value[3]=reg->dot11_txop_limit_value[3];
    dot11_aifs_value[3]=reg->dot11_aifs_value[3];
    /* For Beacon Q*/
    dot11_cwmin_value_ac[4]=reg->dot11_cwmin_value_ac[4];
    dot11_cwmax_value_ac[4]=reg->dot11_cwmax_value_ac[4];
    dot11_txop_limit_value[4]=reg->dot11_txop_limit_value[4];
    dot11_aifs_value[4]=reg->dot11_aifs_value[4];
    uu_dot11_op_mode_r=reg->uu_dot11_op_mode_r;
    uu_dot11_qos_mode_r=reg->uu_dot11_qos_mode_r;
    /** Default beacon interval is 100ms (100000usec) */
    dot11_BeaconInterval=reg->dot11_BeaconInterval;
    dot11_signal_extension=reg->dot11_signal_extension;
    uu_dot11_self_cts_r=reg->uu_dot11_self_cts_r;
    /** Statistics initialization */
    dot11_ack_failure_count_g=reg->dot11_ack_failure_count_g;
    dot11_rts_failure_count_g=reg->dot11_rts_failure_count_g;
    dot11_rts_success_count_g=reg->dot11_rts_success_count_g;
    dot11_fcs_error_count_g=reg->dot11_fcs_error_count_g;
    uu_wlan_rx_frames_g=reg->uu_wlan_rx_frames_g;
    uu_wlan_rx_multicast_cnt_g=reg->uu_wlan_rx_multicast_cnt_g;
    uu_wlan_rx_broadcast_cnt_g=reg->uu_wlan_rx_broadcast_cnt_g;
    uu_wlan_rx_frame_for_us_g=reg->uu_wlan_rx_frame_for_us_g;
    uu_wlan_rx_ampdu_frames_g=reg->uu_wlan_rx_ampdu_frames_g;
    uu_wlan_rx_ampdu_subframes_g=reg->uu_wlan_rx_ampdu_subframes_g;
    uu_wlan_rx_phy_err_pkts_g=reg->uu_wlan_rx_phy_err_pkts_g;
    dot11_rts_threshold_value=reg->dot11_rts_threshold_value;
    /** BSSBasicRateSet */
    BSSBasicRateSet=reg->BSSBasicRateSet;
    /** STA MAC address */
    uu_dot11_sta_mac_addr_r[0]=reg->uu_dot11_sta_mac_addr_r[0];
    uu_dot11_sta_mac_addr_r[1]=reg->uu_dot11_sta_mac_addr_r[1];
    uu_dot11_sta_mac_addr_r[2]=reg->uu_dot11_sta_mac_addr_r[2];
    uu_dot11_sta_mac_addr_r[3]=reg->uu_dot11_sta_mac_addr_r[3];
    uu_dot11_sta_mac_addr_r[4]=reg->uu_dot11_sta_mac_addr_r[4];
    uu_dot11_sta_mac_addr_r[5]=reg->uu_dot11_sta_mac_addr_r[5];
    /** STA BSSID address */
    uu_dot11_sta_bssid_r[0]=reg->uu_dot11_sta_bssid_r[0];
    uu_dot11_sta_bssid_r[1]=reg->uu_dot11_sta_bssid_r[1];
    uu_dot11_sta_bssid_r[2]=reg->uu_dot11_sta_bssid_r[2];
    uu_dot11_sta_bssid_r[3]=reg->uu_dot11_sta_bssid_r[3];
    uu_dot11_sta_bssid_r[4]=reg->uu_dot11_sta_bssid_r[4];
    uu_dot11_sta_bssid_r[5]=reg->uu_dot11_sta_bssid_r[5];
    
#if 0 /** TSF related register */
//uu_wlan_tsf_r=reg->uu_wlan_tsf_r;
#endif 
    uu_wlan_last_beacon_tx_r=reg->uu_wlan_last_beacon_tx_r;
    uu_wlan_phy_tx_delay_r=reg->uu_wlan_phy_tx_delay_r;
    
#if 1 /** Power save related registers */
    uu_wlan_ps_mode_r =reg->uu_wlan_ps_mode_r;
    uu_wlan_uapsd_mode_r = reg->uu_wlan_uapsd_mode_r;
#endif
    uu_wlan_multdata= reg->uu_wlan_multdata_r;
    return 0;
} /* set_default_registers */


/** uu_umac_stub */
static int uu_umac_stub(unsigned char *data, int len2)
{
    uu_uint32 i,ii=0;
    uu_uint32 jj;
    uu_uint32 mpdu_length;
    uu_uint32 fi_len,fi_len2;
    uu_uchar ac;
    uu_uint32 available_buf;
    uu_uchar  *addrs[6];
    uu_uchar  *seqno[6];
    uu_uint32 len[6];
    uu_uchar data1[200];
    uu_uchar data2[200];
    uu_uchar data3[200];
    uu_uchar data4[200],tmp_data1[500],tmp_data2[500];
    uu_uchar length[3];
    uu_uchar data_fi[500];
    uu_uchar fi[sizeof(uu_wlan_tx_frame_info_t)];
    uu_uint32 framelen_filen,aggr_len;  
    uu_uint32 length_firstframe,length_secondframe,length_thirdframe,length_fourthframe;
    uu_uint8 tot_len1, tot_len2,tot_len3,tot_len4,count;

    uu_wlan_tx_frame_info_t *frame_fi =  NULL;
    uu_wlan_tx_frame_info_t *frame_lp =  NULL;

    if(data == NULL || len2 <= 0)
    {
        return -1;
    }
    len2 = (len2 -ii)/2;

    for(jj = 0; jj < len2; jj++)
    {
        ascii_to_hex(&data[ii]);
        ascii_to_hex(&data[ii+1]);
        data[jj] = (((data[ii] & 0x0f) << 4) | (data[ii + 1] & 0x0f));
        ii++;
        ii++;
        printk("%x ",data[jj]);
    }
/* This condition will check for internal collision or multiple frame transmission*/
    if(uu_wlan_multdata==1 || uu_wlan_multdata==2) 
    { 
/* Process will go into this condition when checking for internal collision testing*/
        if(uu_wlan_multdata==1)
        {
            tot_len1=data[0];
            for (jj=1; jj<=tot_len1;jj++)
            {
                tmp_data1[jj-1] = data[jj];
            }
            tot_len2=data[jj];
            for (jj=0; jj<tot_len2;jj++)
            {
                tmp_data2[jj] = data[(jj+2)+tot_len2];
                printk("%x",tmp_data2[jj]);
            }

            frame_fi = (uu_wlan_tx_frame_info_t *)tmp_data1;
            ac = frame_fi->ac;
            mpdu_length=frame_fi->frameInfo.framelen;
            fi_len=tot_len1-mpdu_length;//len2-mpdu_length; /* frame info length */
            len[0]=tot_len1;
            addrs[0] = tmp_data1;
            len[1]=0;
            addrs[1] = NULL;
            if(!frame_fi->aggr_count)
            {
                memcpy(seqno,&frame_fi->mpdu[22],2);
                seq_no=seqno[0];
                seq_no=(seq_no>>4);
            }
            available_buf = uu_wlan_tx_get_available_buffer(ac);
            if(available_buf >= len2)
            {
                uu_wlan_tx_write_buffer(ac, addrs, len);
            }

            frame_fi = (uu_wlan_tx_frame_info_t *)tmp_data2;
            ac = frame_fi->ac;
            //    	mpdu_length=frame_fi->frameInfo.framelen;
            //	fi_len=tot_len2-mpdu_length;//len2-mpdu_length; /* frame info length */
            addrs[0]= tmp_data2;
            len[0] = tot_len2;
            len[1]=0; 
            addrs[1] = NULL;
            if(!frame_fi->aggr_count)
            {
                memcpy(seqno,&frame_fi->mpdu[22],2);
                seq_no=seqno[0];
                seq_no=(seq_no>>4);
            }
            available_buf = uu_wlan_tx_get_available_buffer(ac);
            if(available_buf >= len2)
            {
                uu_wlan_tx_write_buffer(ac, addrs, len);
            } 
            return 0;
        }
        else /* Process will go into this when checking for multiple frame transmission*/
        {
            count=data[0];
            tot_len1=data[1];
            for (jj=0; jj<=tot_len1;jj++)
            {
                tmp_data1[jj] = data[jj+2];
            }
            frame_fi = (uu_wlan_tx_frame_info_t *)tmp_data1;
            frame_fi->txvec.is_aggregated = 0; //for multidata is aggregated will always be zero
            ac = frame_fi->ac;
            mpdu_length=frame_fi->frameInfo.framelen;
            fi_len=tot_len1-mpdu_length;//len2-mpdu_length; /* frame info length */
            len[0]=tot_len1;
            addrs[0] = tmp_data1;
            len[1]=0;
            addrs[1] = NULL;
            if(!frame_fi->aggr_count)
            {
                memcpy(seqno,&frame_fi->mpdu[22],2);
                seq_no=seqno[0];
                seq_no=(seq_no>>4);
            }
            available_buf = uu_wlan_tx_get_available_buffer(ac);
            for(i=0;i<count;i++)
            {
                if(available_buf >= len2)
                {	
                    uu_wlan_tx_write_buffer(ac, addrs, len);
                }
            }
            return 0;
        }
    }//End of if statement for multidata

    else /* Process will come when transmitting single frame or aggregation frame*/
    {
        frame_fi = (uu_wlan_tx_frame_info_t *)data;
        ac = frame_fi->ac;
        mpdu_length=frame_fi->frameInfo.framelen;
        fi_len=len2-mpdu_length; /* frame info length */

        memcpy(frame_fi->mpdu,(data+sizeof(uu_wlan_tx_frame_info_t)),mpdu_length);

        if(!frame_fi->aggr_count)
        {
            memcpy(seqno,&frame_fi->mpdu[22],2);
            seq_no=seqno[0];
            seq_no=(seq_no>>4);
        }

        available_buf = uu_wlan_tx_get_available_buffer(ac);

        /** For Aggregation. This test driver supports aggregation for max of 4 MPDUs */
        if(frame_fi->aggr_count)
        {

            length_firstframe=frame_fi->mpdu[0];
            memcpy(data1,(frame_fi->mpdu+1),length_firstframe); // Obtaining 1st MPDU

            memcpy(length,(frame_fi->mpdu+length_firstframe+1),1);
            length_secondframe=length[0];        
            memcpy(data2,(frame_fi->mpdu+length_firstframe+2),length_secondframe); // Obtaining 2nd MPDU

            memcpy(length+1,(frame_fi->mpdu+length_firstframe+length_secondframe+2),1);
            length_thirdframe=length[1];
            memcpy(data3,(frame_fi->mpdu+length_firstframe+length_secondframe+3),length_thirdframe); // Obtaining 3rd MPDU

            memcpy(length+2,(frame_fi->mpdu+length_firstframe+length_secondframe+length_thirdframe+3),1);
            length_fourthframe=length[2];
            memcpy(data4,(frame_fi->mpdu+length_firstframe+length_secondframe+length_thirdframe+4),length_fourthframe); // Obtaing 4th MPDU

            memcpy(fi,data,sizeof(uu_wlan_tx_frame_info_t));
            for(i=0;i<sizeof(uu_wlan_tx_frame_info_t);++i)
                data_fi[i]=fi[i];
            for(i=0;i<length_firstframe;++i)
                data_fi[sizeof(uu_wlan_tx_frame_info_t)+i]=data1[i];

            framelen_filen=sizeof(uu_wlan_tx_frame_info_t)+length_firstframe;
            aggr_len=framelen_filen-sizeof(uu_wlan_tx_frame_info_t);
            frame_lp = (uu_wlan_tx_frame_info_t *)data_fi;

            addrs[0]=data_fi;
            addrs[1]=data2;
            addrs[2]=data3;
            addrs[3]=data4;
            addrs[4] =NULL;

            len[0] = framelen_filen;
            len[1] = length_secondframe;
            len[2] = length_thirdframe;
            len[3] = length_fourthframe;
            len[4] = 0;

            /** Filling AMPDU information */
            frame_lp->ampdu_info.mpdu_info[0].mpdu_len = length_firstframe;
            frame_lp->ampdu_info.mpdu_info[0].pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_firstframe);
            frame_lp->frameInfo.framelen = length_firstframe + UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_firstframe);

            frame_lp->ampdu_info.mpdu_info[1].mpdu_len = length_secondframe;
            frame_lp->ampdu_info.mpdu_info[1].pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_secondframe);
            frame_lp->frameInfo.framelen +=  length_secondframe + UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_secondframe);

            frame_lp->ampdu_info.mpdu_info[2].mpdu_len = length_thirdframe;
            frame_lp->ampdu_info.mpdu_info[2].pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_thirdframe);
            frame_lp->frameInfo.framelen +=  length_thirdframe + UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_thirdframe);

            frame_lp->ampdu_info.mpdu_info[3].mpdu_len = length_fourthframe;
            frame_lp->ampdu_info.mpdu_info[3].pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_fourthframe);
            frame_lp->frameInfo.framelen +=  length_fourthframe + UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(length_fourthframe);

            memcpy(seqno,&frame_fi->mpdu[23],2);
            seq_no=seqno[0];
            seq_no=(seq_no>>4);

            if(available_buf >= (len[0] + len[1] + len[2] + len[3] + len[4]))
            {
                uu_wlan_tx_write_buffer(ac, addrs, len);
            }
            return 0;
        }

        /** For non-aggregated frame */
        len[0] = len2; /* frameinfo + frame length */
        addrs[0]=data; /* frameinfo + frame */
        addrs[1] =NULL;
        len[1] = 0;

        /* Filling Length and staring address of frame for TX write buffer*/
        if(available_buf >= len2)
        {
            uu_wlan_tx_write_buffer(ac, addrs, len);
        }

        return 0;  
    }/* End of else statement*/

}/* uu_umac_stub */

/*Config registers*/
int uu_umac_config_registers(uu_uchar *data,int len)
{
    static uu_uint32 temp;
    uu_uint32 i=0,ii=0,jj,error;

    if(data == NULL || len <= 0)
    {
        return -1;
    }
    len = (len -ii)/2;

    for(jj = 0; jj < len; jj++)
    {
        ascii_to_hex(&data[ii]);
        ascii_to_hex(&data[ii+1]);
        data[jj] = (((data[ii] & 0x0f) << 4) | (data[ii + 1] & 0x0f));
        ii++;
        ii++;
        printk("%x ",data[jj]);
    }

    set_default_registers(data);

    if(temp == 0) /* To Avoid initializing LMAC for every test case */
    {
        temp++;
        uu_wlan_lmac_init();

        if(uu_wlan_lmac_start()!=UU_SUCCESS)
        {
            UU_WLAN_LOG_ERROR(("UU_WLAN: uu_wlan_lmac_init failed. E:%x \n\n", error));
            error = -ENODEV;
            return error;
        }
    }
    return 0; 
} /*Config registers*/

/** uu_umac_stub_phy_indication */
int uu_umac_stub_indication(uu_uchar *data, int len)
{
#ifdef UU_UMAC_STUB
    uu_umac_stub(data, len);
#endif
    return 0;
}/* uu_umac_stub_indication */

/** Rx Status filling after getting frame from lmac. */
static uu_void uu_wlan_fill_rx_status(uu_wlan_rx_frame_info_t *recv_fi, uu_wlan_rx_status_t *rs)
{
    
/*--Commented in UMAC sorce.*/
#if 0
    rs->datalen = UU_REG_RX_DATALEN;
    rs->status = UU_REG_RX_STATUS;
    rs->phyerr = UU_REG_RX_PHYERR;
    rs->rssi = recv_fi->rxvec.rssi;
    rs->keyix = recv_fi->frameInfo.keyix;
    rs->rate = UU_REG_RX_RATE;
    rs->antenna = UU_REG_RX_ANTENNA;
    rs->more = UU_REG_RX_MORE;
    rs->rssi_ctl0 = UU_REG_RX_RSSI_CT10;
    rs->rssi_ctl1 = UU_REG_RX_RSSI_CT11;
    rs->rssi_ctl2 = UU_REG_RX_RSSI_CT12;
    rs->rssi_ext0 = UU_REG_RX_RSSI_EXT0;
    rs->rssi_ext1 = UU_REG_RX_RX_RSSI_EXT1;
    rs->rssi_ext2 = UU_REG_RX_RX_RSSI_EXT2;
    rs->isaggr = UU_REG_RX_ISAGGR;
    rs->moreaggr = UU_REG_RX_MOREAGGR;
    rs->num_delims = UU_REG_RX_DELIMS;
    rs->flags = UU_REG_RX_FLAGS; //TODO fill the status registers from lmac 
#endif
} /* uu_wlan_fill_rx_status */


/** uu_wlan_umac_cbk */
uu_void uu_wlan_umac_cbk(uu_int8 ind)
{
    uu_uint8 ii,jj;
    uu_uint32 status;
    if(ind == UU_LMAC_IND_TX_STATUS)
    {
#ifdef UU_WLAN_BQID
        for(ii=0; ii< UU_WLAN_MAX_QID; ii++)
#else   
            for(ii=0; ii< UU_WLAN_AC_MAX_AC; ii++)
#endif
            {
                for(jj=0; jj<UU_WLAN_MAX_TX_STATUS_TO_UMAC; jj++)
                {
                    status = (uu_wlan_tx_status_flags_g[ii] & (1 << jj));
                    if(status)
                    {
                        printk("Sent frame seq no is :::%x Rcvd frame seq no is :::%x Retry count is :::%d and status is :::%d\n",seq_no,uu_wlan_tx_status_info_g[ii][jj].seqno,uu_wlan_tx_status_info_g[ii][jj].retry_count,uu_wlan_tx_status_info_g[ii][jj].status);
                        if(seq_no == uu_wlan_tx_status_info_g[ii][jj].seqno)
                        {
                            printk("Sent frame and received frame is matched\n\n");
                        }
                        uu_wlan_tx_status_flags_g[ii] &= ~(1 << jj);
                    }
                }
            }
        return 0;
    }
    else if(ind == UU_LMAC_IND_RX_FRAME)
    {
        uu_wlan_rx_frame_info_t rx_fi;
        uu_wlan_rx_status_t rs;

        uu_uchar* mpdu;

        for(jj=0; jj<UU_WLAN_MAX_RX_STATUS_TO_UMAC; jj++)
        {
            status = (uu_wlan_rx_status_flags_g & (1 << jj));
            printk("status is :%d\n",status);
            if(status)
            {
                printk("Flag %d is set and reading address and length %d \n", jj, uu_wlan_rx_status_info_g[jj].len);
                UU_WLAN_LOG_DEBUG(("Flag %d is set and reading address and length %d \n", jj, uu_wlan_rx_status_info_g[jj].len));

                memcpy(&rx_fi, uu_wlan_rx_status_info_g[jj].offset+uu_wlan_rx_buff_base_addr_reg_g, sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t));
                mpdu = (uu_wlan_rx_status_info_g[jj].offset+uu_wlan_rx_buff_base_addr_reg_g) + sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t)+16; /* 16 bytes represents number of time stamp_msb, time_stamp_lsb, beacon_time_stamp_lsb and beacon_time_stamp_msb */

    //            uu_wlan_fill_rx_status(&rx_fi,&rs);

                printk("MPDU in umac_cbk is %x %x %x %x and len is %x\n",mpdu[0],mpdu[1],mpdu[2],mpdu[3],rx_fi.frameInfo.framelen);

                uu_umac_stub_nl_send_frame(mpdu, rx_fi.frameInfo.framelen);
                uu_wlan_rx_status_flags_g &= ~(1 << jj);
            }

        }
    }
#if 0    
    else if (ind == UU_LMAC_IND_GEN_BEACON)
    {
        printk("beacon trigger came to test driver\n");
    } 
#endif
} /* uu_wlan_umac_cbk */

/* EOF */

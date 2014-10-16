/** HEADERS */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "uu_wlan_fwk_log.h"

#include "uu_wlan_phy_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_lmac_if.h";

#include "uu_wlan_reg.h"

#include "uu_phy_stub_nl.h"
#include "uu_wlan_phy_stub.h"

#ifdef UU_WLAN_TPC
#include "uu_wlan_rate.h"
#include "uu_wlan_phy_tpc.h"
#endif

typedef struct phy_stub_tx_vec
{
    uu_wlan_frame_format_t      format : 2;
    uu_uint8                    L_datarate : 4;
    uu_uint8                    reserved:2;
    uu_uint8                    mcs : 7;
    uu_uint8                    is_aggregated : 1;
    uu_uint32                   length;
#ifdef UU_WLAN_TPC
    /* Stbc for HT or VHT */
    uu_uint8                     stbc : 2;
    /* Modulation used for non-HT format */
    uu_wlan_non_ht_modulation_t modulation : 3;
    /* For channel bandwidth used */
    uu_wlan_ch_bndwdth_type_t    ch_bndwdth :3;
    /* Power level: either Txpwr_level or Rssi */
    uu_uint8                     pwr_level : 8;
    /* Indicated channel bandwidth for non-HT */
    uu_wlan_ch_bndwdth_type_t indicated_chan_bw : 3;
    /* Short GI for HT/VHT */
    uu_uint8    is_short_GI : 1;
    /* FEC Coding: BCC or LDPC */
    uu_uint8    is_fec_ldpc_coding : 1;
    /* number of space time streams */
    uu_uint8 num_sts : 3;
#endif
} uu_phy_stub_tx_vec_t;

static uu_phy_stub_tx_vec_t    ptxvec;

static unsigned char mpdu[15320 + sizeof(uu_phy_stub_tx_vec_t)];

#ifdef UU_WLAN_TPC
/**
 * Get rate_index as per the L_datarate.
 * TODO: Here all rates CCK and OFDM are considered to be registered.
 * So, it needs to be modified as per the rate registered.
 */
static uu_uint8 get_rate_index_of_L_datarate(uu_uint8 L_datarate)
{
    uu_uint8 rate_idx;
    if (L_datarate == UU_HW_RATE_1M)
    {
        rate_idx = 0;
    }
    else if (L_datarate == UU_HW_RATE_2M)
    {
        rate_idx = 1;
    }
    else if (L_datarate == UU_HW_RATE_5M5)
    {
        rate_idx = 2;
    }
    else if (L_datarate == UU_HW_RATE_11M)
    {
        rate_idx = 3;
    }
    else if (L_datarate == UU_HW_RATE_6M)
    {
        rate_idx = 4;
    }
    else if (L_datarate == UU_HW_RATE_9M)
    {
        rate_idx = 5;
    }
    else if (L_datarate == UU_HW_RATE_12M)
    {
        rate_idx = 6;
    }
    else if (L_datarate == UU_HW_RATE_18M)
    {
        rate_idx = 7;
    }
    else if (L_datarate == UU_HW_RATE_24M)
    {
        rate_idx = 8;
    }
    else if (L_datarate == UU_HW_RATE_36M)
    {
        rate_idx = 9;
    }
    else if (L_datarate == UU_HW_RATE_48M)
    {
        rate_idx = 10;
    }
    else if (L_datarate == UU_HW_RATE_54M)
    {
        rate_idx = 11;
    }
    else
    {
        rate_idx = 4;
    }

    printk("rate_idx at phy_stub: %u\n", rate_idx);
    return rate_idx;
} /* get_rate_index_of_L_datarate */

/** Receiver Minimum Sensitivity depends on MCS index, Bandwidth,
 *  Nsts (STBC), GI, FEC Coding (BCC, LDPC), PHY_TYPE.
 * So, table needs to be maintained as per the above
 * variables to return corresponding sensitivity.
 *
 * Refer to P802.11 ac D3.0 Table 22-25, Pg. 273, it returns the value,
 * But for future we need to extend the table with the above dependencies
 */
uu_int8 uu_get_rec_min_sensitivity (uu_phy_stub_tx_vec_t* rx_fi)
{
    uu_int8 sensitivity;

    /** For VHT */
    uu_uint8 nsts;

    if (rx_fi->format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        nsts = rx_fi->num_sts;

        printk("Nsts for getting rec_min_sensitivity is: %d\n", nsts);

        if (rx_fi->ch_bndwdth == CBW40)
        {
            if (rx_fi->is_short_GI)
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_40_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_40_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_40_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_40_bcc_lgi;
                }
            }
        }
        else if (rx_fi->ch_bndwdth == CBW80)
        {
            if (rx_fi->is_short_GI)
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_80_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_80_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_80_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_80_bcc_lgi;
                }
            }
        }
        else if (rx_fi->ch_bndwdth >= CBW160)
        {
            if (rx_fi->is_short_GI)
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_160_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_160_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_160_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_160_bcc_lgi;
                }
            }
        }
        else
        {
            if (rx_fi->is_short_GI)
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_20_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_20_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_20_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs].mcs_20_bcc_lgi;
                }
            }
        }
        printk("Sensitivity for HT at bandwidth: %u, and mcs: %u, and sgi: %u, ldpc: %u, nsts: %u, is: %d\n", rx_fi->ch_bndwdth,
                            rx_fi->mcs, rx_fi->is_short_GI, rx_fi->is_fec_ldpc_coding, nsts, sensitivity);
    }
    /** For HT */
    else if (rx_fi->format == UU_WLAN_FRAME_FORMAT_HT_MF || rx_fi->format == UU_WLAN_FRAME_FORMAT_HT_GF)
    {
        nsts = ((rx_fi->mcs)/UU_MAX_MCS_PER_STREAM_HT) + rx_fi->stbc;

        if (rx_fi->ch_bndwdth == CBW40)
        {
            if (rx_fi->is_short_GI)
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_bcc_lgi;
                }
            }
        }
        else
        {
            if (rx_fi->is_short_GI)
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->mcs & 0x07].mcs_20_bcc_lgi;
                }
            }
        }
        printk("Sensitivity for HT at bandwidth: %u, and mcs: %u, and sgi: %u, ldpc: %u, nsts: %u, is: %d\n", rx_fi->ch_bndwdth,
                            rx_fi->mcs, rx_fi->is_short_GI, rx_fi->is_fec_ldpc_coding, nsts, sensitivity);
    }
    else    /** For legacy */
    {
        uu_uint8 rate_index;
        rate_index = get_rate_index_of_L_datarate(rx_fi->L_datarate);
        sensitivity = uu_wlan_recv_min_sensitivity_table_legacy[rate_index];
        printk("Sensitivity for non-HT at datarate: %u, rate_index: %u, is: %d\n", rx_fi->L_datarate,
                            rate_index, sensitivity);
    }
    printk("sensitivity at phy_stub: %d\n", sensitivity);
    return sensitivity;
} /* uu_get_rec_min_sensitivity */

static int uu_phy_drop_pkt (uu_phy_stub_tx_vec_t* prxvec)
{
    uu_int8 min_sens;
    uu_int8 ppdu_rssi;

    min_sens = uu_get_rec_min_sensitivity (prxvec);
    ppdu_rssi = uu_wlan_rssi_level_to_rssi_power[prxvec->pwr_level];

    printk("MIN_SENS: %d, PPDU_RSSI: %d\n", min_sens, ppdu_rssi);

    /* If RSSI level of the PPDU is less than the receiver minimum sensitivity,
     * then drop the frame */
    if (ppdu_rssi < min_sens)
    {
        printk("PPDU RSSI is less than RECEIVER MINIMUM SENSITIVITY, so DROPPED\n");
        return 1;
    }

    /* If RSSI level of the PPDU exceeds the receiver minimum sensitivity than 20,
     * then do successful transmission of the frame */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
    if (ppdu_rssi >= min_sens + 20)
    {
        printk("PPDU RSSI is very good\n");
        return 0;
#if 0
    }
    else
    {
        /* Random dropping of the frames */
        uu_uint8 temp_timer;
        get_random_bytes(&temp_timer, sizeof(temp_timer));
        if (!(temp_timer % 50))
        {
            printk("PPDU DROPPING RANDOMLY\n");
            return 1;
        }
#endif
    }
#else
     if (ppdu_rssi >= min_sens + 20)
        printk("PPDU RSSI is very good\n");
#endif
    return 0;
} /* uu_phy_drop_pkt */
#endif /* UU_WLAN_TPC */


static void ascii_to_hex(unsigned char *data)
{
    unsigned char num = *data;
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
}

/* uu_phy_stub_indication */
int uu_phy_stub_indication(unsigned char *data, int len)
{
    int i=0, ind, ii;
    int phy_ind;
    int jj;

    UU_WLAN_LOG_DEBUG(("Received data len %d and data is :::::: \t",len));

    for(i=0;i<len;i++)
        UU_WLAN_LOG_DEBUG(("%x",data[i]));
    UU_WLAN_LOG_DEBUG(("\n"));

    phy_ind = data[0] - 0x30;
    UU_WLAN_LOG_DEBUG(("cmd type first byte %d\n",phy_ind));

    ii = 1;		
    while(data[ii] != '!')
    {
        phy_ind = (phy_ind * 10 ) + (data[ii] -0x30);
        ii++;
    }
    UU_WLAN_LOG_DEBUG(("cmd type is %d\n",phy_ind));
    ii = ii + 1; // To remove delimiter added

    if(len >= 5)
    {
        len = (len -ii)/2;
        UU_WLAN_LOG_DEBUG(("Received data is in if 1 condition len is %d\t", len));

        for(jj = 0; jj < len; jj++)
        {
            ascii_to_hex(&data[ii]);
            ascii_to_hex(&data[ii+1]);
            data[jj] = (((data[ii] & 0x0f) << 4) | (data[ii + 1] & 0x0f));
            ii++;
            ii++;
            UU_WLAN_LOG_DEBUG(("%x ",data[jj]));
        }	
        UU_WLAN_LOG_DEBUG(("\n"));
        ii = 0;
        len = jj;
    }

    else
    {
        if(len == 4)
        {
            data[3] = data[3] - 0x30;
        }
        if(len == 3)
        {
            data[2] = data[2] - 0x30;
        }
    }

    switch(phy_ind)
    {
        case UU_PHY_INVALID_EVENT:
            UU_WLAN_LOG_DEBUG(("INVALID EVENT \n"));
            break;
        case UU_PHY_DATA_CONF:
            UU_WLAN_LOG_DEBUG(("UU_PHY_DATA_CONF \n"));
            ind = UU_WLAN_CAP_EV_TX_DATA_CONFIRM;
            return 0; //ignored because confirmation is given byte by byte from PHY stub
            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind,NULL,0);
            }
            break;
        case UU_PHY_DATA_IND:
            UU_WLAN_LOG_DEBUG(("UU_PHY_DATA_IND \n"));
            ind = UU_WLAN_CAP_EV_DATA_INDICATION;

            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind, data+ii, len-ii);
            }
            break;
        case UU_PHY_TXSTART_CONF:
            UU_WLAN_LOG_DEBUG(("UU_PHY_TXSTART_CONF \n"));
            ind = UU_WLAN_CAP_EV_TX_START_CONFIRM;

            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind,NULL,0);
            }
            break;
        case UU_PHY_TXEND_CONF:
            UU_WLAN_LOG_DEBUG(("UU_PHY_TXEND_CONF \n"));		
            ind = UU_WLAN_CAP_EV_TX_END_CONFIRM;

            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind,NULL,0);
                msleep(50); // Turnaround time between TX and RX.
            }
            break;
        case UU_PHY_RXSTART_IND:
            UU_WLAN_LOG_DEBUG(("UU_PHY_RXSTART_IND \n"));
            ind = UU_WLAN_CAP_EV_RX_START_IND;

            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind, data+ii, len-ii);
            }
            break;
        case UU_PHY_RXEND_IND:
            UU_WLAN_LOG_DEBUG(("UU_PHY_RXEND_IND \n"));
            ind = UU_WLAN_CAP_EV_RX_END_IND;

            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind, data+ii, len-ii);
				/* phycca indication, giving as idle */
    			ind = UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL;
				unsigned char energy_level_busy[2];
    			energy_level_busy[0] = 0;
    			energy_level_busy[1] = 0;
    			if(phy_ops_gp->PhyIndCBK(ind, energy_level_busy, 1) != 0)
    			{
        			return -1;
    			}
                msleep(50); /* Turnaround time between TX and RX */
            }
            break;
        case UU_PHY_CCARESET_CONF:
            UU_WLAN_LOG_DEBUG(("UU_PHY_CCARESET_CONF \n"));
            ind = UU_WLAN_CAP_EV_CCA_RESET_CONFIRM;
            return 0; /* Ignored because confirm is given directly from PHY-STUB */
            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind,NULL,0);
            }
            break;
        case UU_PHY_CCA_IND:
            UU_WLAN_LOG_DEBUG(("UU_PHY_CCA_IND \n"));
            ind = UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL;
            uu_uchar array[2];
            array[0] = (data[0]>>4);
            array[1] = (data[0]&0x0f);
            len=2;

            if (phy_ops_gp != NULL)
            {
                phy_ops_gp->PhyIndCBK(ind, array, len);
            }
            break;

        case UU_PHY_DATA_REQ:
            UU_WLAN_LOG_ERROR(("NOT EXPECTED----UU_PHY_DATA_REQ from mac to phy request \n"));

        case UU_PHY_TXSTART_REQ:
            UU_WLAN_LOG_ERROR(("NOT EXPECTED----UU_PHY_TXSTART_REQ - mac to phy request \n"));

        case UU_PHY_TXEND_REQ:
            UU_WLAN_LOG_ERROR(("NOT EXPECTED----UU_PHY_TXEND_REQ - mac to phy request \n"));

        case UU_PHY_CCARESET_REQ:
            UU_WLAN_LOG_ERROR(("NOT EXPECTED----UU_PHY_CCARESET_REQ - mac to phy request \n"));

        default:
            UU_WLAN_LOG_ERROR(("NOT EXPECTED----Wrong value \n"));
            break;
    }
    return 0;
}


int uu_wlan_phy_form_data2nl(int ind, unsigned char *data2nl, unsigned char *mac_data, int mac_data_len)
{
    int jj=0,data2nl_len;
    /** Add phy indication and delimitor before actual data */
    data2nl[0] = ind + 0x30;
    data2nl_len = mac_data_len + 1;
    data2nl[1] = '!';
    data2nl_len = data2nl_len + 1;
    if(mac_data_len != 0)
    {
        jj = 2;
        memcpy(data2nl+jj, mac_data, mac_data_len);
    }
    return data2nl_len;
}


#ifdef UU_WLAN_END_TO_END_TESTING
static uu_int32 build_ppdu_pkt(uu_uchar *mpdu,uu_uchar *data_p, uu_int32 data_len)
{
    /* Updating the required tx vector fields */
    memcpy(&mpdu[0], &ptxvec, sizeof(uu_phy_stub_tx_vec_t));
    //Network Layer Payload
    memcpy((mpdu+(sizeof(uu_phy_stub_tx_vec_t))), data_p, data_len);

    return (sizeof(uu_phy_stub_tx_vec_t) + data_len);
} /* build_ppdu_pkt */
#endif

static unsigned char data_temp[15320];
static unsigned char res_data[15320+3];
void uu_wlan_phy_data_req(unsigned char byte, unsigned char *confirm_flag)
{
    static uu_uint32 data_len = 0;
    data_temp[data_len++] = byte; 
    *confirm_flag = 1;
	int i;

    if (data_len != ptxvec.length)
    {
        return;
    }
    *confirm_flag = 0;
#ifndef UU_WLAN_END_TO_END_TESTING
    int res_len;
    UU_WLAN_LOG_DEBUG(("LMAC: uu_wlan_phy_data_req..data %x %x %x\n", data_temp[0], data_temp[1], data_temp[4]));

    {
        for (i = 0; i < ptxvec.length; i++)
        {
            printk("%x ", data_temp[i]);
        }
        printk("\n ");
    }

    res_len = uu_wlan_phy_form_data2nl(UU_PHY_DATA_REQ, res_data, data_temp, data_len);
    // Send to NL interface
    uu_phy_stub_nl_send_frame(res_data,res_len);
    data_len =0;
    return;
#else

    data_len = build_ppdu_pkt(mpdu, data_temp, data_len);
    uu_phy_stub_nl_send_frame(mpdu, data_len);
    data_len = 0;
    return;
#endif /* UU_WLAN_END_TO_END_TESTING */
} /* uu_wlan_phy_data_req */

EXPORT_SYMBOL(uu_wlan_phy_data_req);

void uu_wlan_phy_txstart_req(unsigned char *data, int len)
{
    uu_wlan_tx_vector_t* txvec = (uu_wlan_tx_vector_t*)data;
    memset(&ptxvec, 0, sizeof(uu_phy_stub_tx_vec_t));
    ptxvec.format = txvec->format;
    ptxvec.L_datarate = txvec->L_datarate;

#ifdef UU_WLAN_TPC
    if (txvec->format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        ptxvec.mcs = txvec->tx_vector_user_params[0].vht_mcs;
    }
    else
    {
        ptxvec.mcs = txvec->mcs;
    }
#else
    ptxvec.mcs = txvec->mcs;
#endif
    ptxvec.is_aggregated = txvec->is_aggregated;

    if (ptxvec.format == UU_WLAN_FRAME_FORMAT_NON_HT)
    {
        ptxvec.length = txvec->L_length;
    }
    else if (ptxvec.format == UU_WLAN_FRAME_FORMAT_HT_GF || ptxvec.format == UU_WLAN_FRAME_FORMAT_HT_MF)
    {
        ptxvec.length = txvec->ht_length;
    }
    else
    {
        ptxvec.length = txvec->tx_vector_user_params[0].apep_length;
    }
#ifdef UU_WLAN_TPC
    /* Stbc for HT or VHT */
    ptxvec.stbc = txvec->stbc;
    /* Modulation used for non-HT format */
    ptxvec.modulation = txvec->modulation;
    /* For channel bandwidth used */
    ptxvec.ch_bndwdth = txvec->ch_bndwdth;

    /* Power level: either Txpwr_level or Rssi */
    /* TODO: Create a look up table, and fill the
       power in dBm used for transmission */
    ptxvec.pwr_level = uu_wlan_transmit_power_dBm[txvec->txpwr_level];
    //ptxvec.pwr_level = 85;

    /* Indicated channel bandwidth for non-HT */
    ptxvec.indicated_chan_bw = txvec->indicated_chan_bw;
    /* Short GI for HT/VHT */
    ptxvec.is_short_GI = txvec->is_short_GI;
    /* FEC Coding: BCC or LDPC */
    /* number of space time streams */
    if (txvec->format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        ptxvec.is_fec_ldpc_coding = txvec->tx_vector_user_params[0].is_fec_ldpc_coding;
        ptxvec.num_sts = txvec->tx_vector_user_params[0].num_sts;
    }
    else
    {
        ptxvec.is_fec_ldpc_coding = txvec->is_fec_ldpc_coding;
        ptxvec.num_sts = txvec->n_tx;
    }
    printk("NITESH TXSTUB: format: %u, mcs: %x, L_datarate: %u, is_aggregated = %u\n", ptxvec.format, ptxvec.mcs, ptxvec.L_datarate, ptxvec.is_aggregated);
    printk("stbc: %u, modulation: %u, length: %d, bandwidth: %u, txpwr: %u\n", ptxvec.stbc, ptxvec.modulation, ptxvec.length, ptxvec.ch_bndwdth, ptxvec.pwr_level);
    printk("shortGI: %u, fec_coding: %u, num_sts: %u\n", ptxvec.is_short_GI, ptxvec.is_fec_ldpc_coding, ptxvec.num_sts);
#endif /* UU_WLAN_TPC */

#ifndef UU_WLAN_END_TO_END_TESTING
    unsigned char res_data[len+3];
    int res_len;
    res_len = uu_wlan_phy_form_data2nl(UU_PHY_TXSTART_REQ, res_data, data, len);
    // Send to NL interface
    uu_phy_stub_nl_send_frame(res_data,res_len);
#else
    unsigned char ind = UU_WLAN_CAP_EV_TX_START_CONFIRM;


    phy_ops_gp->PhyIndCBK(ind,NULL,0);
#endif
} /* uu_wlan_phy_txstart_req */
EXPORT_SYMBOL(uu_wlan_phy_txstart_req);


void uu_wlan_phy_txend_req()
{
#ifndef UU_WLAN_END_TO_END_TESTING
    unsigned char res_data[3];
    int res_len;
    res_len = uu_wlan_phy_form_data2nl(UU_PHY_TXEND_REQ, res_data, NULL, 0);
    // Send to NL interface
    uu_phy_stub_nl_send_frame(res_data,res_len);
#else
    unsigned char ind = UU_WLAN_CAP_EV_TX_END_CONFIRM;
    phy_ops_gp->PhyIndCBK(ind,NULL,0);

#endif
}
EXPORT_SYMBOL(uu_wlan_phy_txend_req);

void uu_wlan_phy_ccareset_req()
{
#ifndef UU_WLAN_END_TO_END_TESTING
    unsigned char res_data[3];
    int res_len;
    res_len = uu_wlan_phy_form_data2nl(UU_PHY_CCARESET_REQ, res_data, NULL, 0);
    // Send to NL interface
    uu_phy_stub_nl_send_frame(res_data,res_len);
    unsigned char ind = UU_WLAN_CAP_EV_CCA_RESET_CONFIRM;
    phy_ops_gp->PhyIndCBK(ind,NULL,0);
#else
    unsigned char ind = UU_WLAN_CAP_EV_CCA_RESET_CONFIRM;
    phy_ops_gp->PhyIndCBK(ind,NULL,0);

#endif
}
EXPORT_SYMBOL(uu_wlan_phy_ccareset_req);


int uu_phy_stub_proc_nl_rx_frame(unsigned char *data, int len)
{
    int ind;
    UU_WLAN_LOG_DEBUG(("DATA PATH \n"));	

    ind = UU_WLAN_CAP_EV_DATA_INDICATION;
    if (phy_ops_gp != NULL)
    {
        phy_ops_gp->PhyIndCBK(ind, data, len);
    }
    return 0;
}


#ifdef UU_WLAN_END_TO_END_TESTING
int uu_phy_stub_start_rx_proc(unsigned char *data, int len)
{
    int ind;
#ifdef UU_WLAN_TPC
    int drop;
#endif
    unsigned char energy_level_busy[2];
    unsigned char rx_end_indication = 0;//success case;
    uu_wlan_rx_vector_t rx_vector;
    uu_phy_stub_tx_vec_t* prxvec = (uu_phy_stub_tx_vec_t*)data;

    if(data == NULL || len <= 0)
    {
        return -1;
    }
    memset(&rx_vector, 0, sizeof(uu_wlan_rx_vector_t));
    rx_vector.format =  prxvec->format;
    rx_vector.mcs = prxvec->mcs;
    rx_vector.L_datarate = prxvec->L_datarate;
    rx_vector.is_aggregated = prxvec->is_aggregated;

#ifdef UU_WLAN_TPC
    /* Stbc for HT or VHT */
    rx_vector.stbc = prxvec->stbc;
    /* Modulation used for non-HT format */
    rx_vector.modulation = prxvec->modulation;
    /* For channel bandwidth used */
    rx_vector.ch_bndwdth = prxvec->ch_bndwdth;

    /* Power level: Rssi */
    /* TODO: Need to calculate RSSI as per the power level and distance */
    rx_vector.rssi = prxvec->pwr_level;

    /* Indicated channel bandwidth for non-HT */
    rx_vector.indicated_chan_bw = prxvec->indicated_chan_bw;
    /* Short GI for HT/VHT */
    rx_vector.is_short_GI = prxvec->is_short_GI;
    /* FEC Coding: BCC or LDPC */
    rx_vector.is_fec_ldpc_coding = prxvec->is_fec_ldpc_coding;
    /* number of space time streams */
    rx_vector.num_sts = prxvec->num_sts;
    printk("NITESH RXSTUB: format: %u, mcs: %x, L_datarate: %u, is_aggregated = %u\n", rx_vector.format, rx_vector.mcs, rx_vector.L_datarate, rx_vector.is_aggregated);
    printk("stbc: %u, modulation: %u, bandwidth: %u, rssi: %u\n", rx_vector.stbc, rx_vector.modulation, rx_vector.ch_bndwdth, rx_vector.rssi);
    printk("shortGI: %u, fec_coding: %u, num_sts: %u\n", rx_vector.is_short_GI, rx_vector.is_fec_ldpc_coding, rx_vector.num_sts);

    /** Here we check that if the rssi is below receiver minimum sensitivity, then we will drop the frames. */

    drop = uu_phy_drop_pkt (prxvec);
    if (drop)
    {
        return -1;
    }
#endif /* UU_WLAN_TPC */

	/* TODO: In terms of 10 ns - Just for IBSS testing. */
    rx_vector.rx_start_of_frame_offset = 2013;

    /* phycca indication */
    ind = UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL;
    energy_level_busy[0] = 1;
    energy_level_busy[1] = 1;
    if (phy_ops_gp == NULL)
    {
        return -1;
    }

    if(phy_ops_gp->PhyIndCBK(ind, energy_level_busy, 2) != 0)
    {
        return -1;
    }

    /* rx start indication */
    ind = UU_WLAN_CAP_EV_RX_START_IND;
    rx_vector.L_length = (len - (sizeof(uu_phy_stub_tx_vec_t)));
    rx_vector.psdu_length = (len - (sizeof(uu_phy_stub_tx_vec_t)));

#if 0 /* Print the aggreated frame */
    int j;
    if (rx_vector.is_aggregated)
    {
        printk("Agg frame at Rx stub\n");
        for (j = 0; j < rx_vector.psdu_length; j++)
        {
            printk("%x ", data[j + 4]);
        }
        printk("\n");
    }
#endif
#if 0 /* Corrupt the frame, to trigger retransmission */
    if(len > 500)
    {
        printk("Corruping***************************************\n");
        //data[len-350] = 7;
        data[len-351] = 7;
        data[len-150] = 7;
        //data[len-10] = 7;
    }
#endif

    if(phy_ops_gp->PhyIndCBK(ind, (unsigned char *)&rx_vector, sizeof(uu_wlan_rx_vector_t)) != 0)
    {
        return -1;
    }

    /* rx data indication */
    ind = UU_WLAN_CAP_EV_DATA_INDICATION;
    if(phy_ops_gp->PhyIndCBK(ind, (data+(sizeof(uu_phy_stub_tx_vec_t))), (len - (sizeof(uu_phy_stub_tx_vec_t)))) != 0)
    {
        //phy_ops_gp->PhyIndCBK(ind, &rx_end_error, 1);
        return -1;
    }

    /* rx end indication */
    ind = UU_WLAN_CAP_EV_RX_END_IND;
    if(phy_ops_gp->PhyIndCBK(ind, &rx_end_indication, 1))
    {
        //phy_ops_gp->PhyIndCBK(ind, &rx_end_error, 1);
        return -1;
    }

    /* phycca indication, giving as idle */
    ind = UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL;
    energy_level_busy[0] = 0;
    energy_level_busy[1] = 0;
    if(phy_ops_gp->PhyIndCBK(ind, energy_level_busy, 1) != 0)
    {
        return -1;
    }

    return 0;
} /* uu_phy_stub_start_rx_proc */
#endif

/* EOF */



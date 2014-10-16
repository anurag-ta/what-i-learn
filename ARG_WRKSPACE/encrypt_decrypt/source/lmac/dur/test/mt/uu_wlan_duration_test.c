
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_errno.h"

#include "uu_wlan_phy_if.h"
#include "uu_wlan_duration.h"
#include "uu_wlan_rate.h"
#include "uu_wlan_reg.h"

#include "uu_wlan_test_main.h"
#if 1
uu_wlan_tx_frame_info_t tx_ctl_frame_info_g;
static uu_int32 uu_wlan_cp_fill_resp_rate_dur(uu_wlan_cp_rx_frame_info_t* info, uu_uchar ctrl_resp_type, uu_uint16 ctrl_resp_len)
{
    uu_int32  duration;

    /* for control request frames received, we wont change anything, just will update the duration */
    /* This is because we will get the control request frames only in basic rate or mcs */
    if ((info->fc0 & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL) 
    {
        /* 
         * TODO : For later. This will be used in VHT case
         * This is for Dynamic RTS-CTS procedure in VHT case
         * This case if secondary channel is busy we can send cts in lower channel 
        if (IEEE80211_IS_FC0_RTS (rx_frame_info->mpdu[0]))
         */

        /* For all other control request frames the control response frames will be same */
    }
    else /* for data and management frame, we will send in non-HT or HT-GF format for control response frames */
    {
#if 0
        if (info->format == UU_WLAN_FRAME_FORMAT_VHT)
        {
            tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
            /* Make tx_vector format as OFDM, as we are sending in non-HT reference */
            /* Since for VHT MCS with any Nss have same OFDM Rate */
            tx_ctl_frame_info_g.txvec.L_datarate = uu_vht_mcs_table_g[(info->mcs >> 3) & 0x0f].ofdm_rate;
            printf("l_datarate in vht %d\n", tx_ctl_frame_info_g.txvec.L_datarate);
            tx_ctl_frame_info_g.txvec.L_datarate = uu_assign_basic_rate(info->modulation, UU_WLAN_FRAME_FORMAT_NON_HT, tx_ctl_frame_info_g.txvec.L_datarate);//added by ramesh.
            printf("l_datarate in vht after assign_basic rate %d\n", tx_ctl_frame_info_g.txvec.L_datarate);
        }
        else if (info->format == UU_WLAN_FRAME_FORMAT_HT_MF) /* input RXVECTOR format */
        {
            tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
            tx_ctl_frame_info_g.txvec.L_datarate = uu_ht_mcs_table_g[info->mcs & UU_HT_MCS_MASK].ofdm_rate;
            printf("l_datarate in ht-mf %d\n", tx_ctl_frame_info_g.txvec.L_datarate);
            tx_ctl_frame_info_g.txvec.L_datarate = uu_assign_basic_rate(info->modulation, UU_WLAN_FRAME_FORMAT_NON_HT, tx_ctl_frame_info_g.txvec.L_datarate);//added by ramesh.
            printf("l_datarate in ht-mf after assign_basic rate %d\n", tx_ctl_frame_info_g.txvec.L_datarate);
            /* Make tx_vector format as OFDM, as we are sending in non-HT reference */
        }
        else if (info->format == UU_WLAN_FRAME_FORMAT_HT_GF)
        {
            tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_HT_GF;
            /* Make tx_vector as HT_MF as per section 9.7.6.5.5(3) control response frame can't be transmitted with format = HT_GF, 
                and we are sending the control response frame in HT-PPDU, so we need to select HT_MF PPDU format */
            tx_ctl_frame_info_g.txvec.mcs = (info->mcs) % 8;
        }
        else /* Non-HT */
        {
            tx_ctl_frame_info_g.txvec.format = info->format;
            tx_ctl_frame_info_g.txvec.L_datarate = uu_assign_basic_rate(info->modulation, info->format, info->L_datarate);
            printf("cal resp duration duration %x l_datarate %d \n", info->duration, tx_ctl_frame_info_g.txvec.L_datarate);
        }
#endif
        if (info->format == UU_WLAN_FRAME_FORMAT_HT_GF)
        {
            tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_HT_GF;
            /* Make tx_vector as HT_MF as per section 9.7.6.5.5(3) control response frame can't be transmitted with format = HT_GF, 
                and we are sending the control response frame in HT-PPDU, so we need to select HT_MF PPDU format */
            tx_ctl_frame_info_g.txvec.mcs = (info->mcs) % 8;
        }
        else /* Non-HT */
        {
            tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
            tx_ctl_frame_info_g.txvec.L_datarate = uu_assign_basic_rate(info->modulation, info->format, info->format ? info->mcs:info->L_datarate);
            printf("cal resp duration duration %x l_datarate %d \n", info->duration, tx_ctl_frame_info_g.txvec.L_datarate);
        }
    }

    /* for HT case, we define length in ht_length */
    if (tx_ctl_frame_info_g.txvec.format == UU_WLAN_FRAME_FORMAT_HT_GF) 
    {
        tx_ctl_frame_info_g.txvec.ht_length = ctrl_resp_len;
    }

    duration = info->duration - uu_calc_frame_duration(&tx_ctl_frame_info_g.txvec) - UU_WLAN_SIFS_TIMER_VALUE_R;

#if 1 //just for testing. remove it later. // TODO: Remove
    /* WARNING: Temp fix */
    if (duration < 0)
    {
        duration = 100;
    }
#endif
    return duration;
} /* uu_wlan_get_duration */
#endif


static uu_void __fill_txvec(uu_wlan_tx_vector_t *txvec, uu_uint8 format, uu_uint8 mcs, uu_uint8 modulation, uu_uint8 l_datarate, 
                                uu_uint16 l_length, uu_uint16 ht_length, uu_uint8 stbc, uu_uint8 ch_bw, 
                                uu_uint8 is_short_gi, uu_uint8 apep_length, uu_uint8 num_ext_ss, uu_uint8 is_long_preamble) 
{
    txvec->format = (uu_uint8)format;
    txvec->mcs = (uu_uint8)mcs;
    txvec->modulation = (uu_uint8)modulation; 
    txvec->L_datarate = (uu_uint8)l_datarate; 
    txvec->L_length = (uu_uint16)l_length; 
    txvec->ht_length = (uu_uint16)ht_length; 
    txvec->stbc = (uu_uint8)stbc; 
    txvec->ch_bndwdth = (uu_uint8)ch_bw; 
    txvec->is_short_GI = (uu_uint8)is_short_gi;
    txvec->tx_vector_user_params[0].apep_length = (uu_uint8)apep_length; 
    txvec->num_ext_ss = (uu_uint8)num_ext_ss; 
    txvec->is_long_preamble = (uu_uint8)is_long_preamble;
} /* __fill_txvec */

static uu_int32 __test_handler_for_single_protection_frame(uu_char *buf)
{
    uu_int32 ret = UU_FAILURE;
    uu_int32 test_category;
    uu_int32 format;
    uu_int32 mcs;
    uu_int32 modulation;
    uu_int32 l_datarate;
    uu_int32 l_length;
    uu_int32 ht_length;
    uu_int32 stbc;
    uu_int32 ch_bw;
    uu_int32 is_short_gi;
    uu_int32 apep_length;
    uu_int32 num_ext_ss;
    uu_int32 is_long_preamble;
    uu_char array[6][40];
    uu_int32 resp_frame_len;
    uu_int32 next_frame_length;
    uu_wlan_tx_vector_t txvec;
    uu_int32 return_value;
    uu_int32 bssbasic_set;

    sscanf(buf,"%s %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %x %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &format, &mcs, &modulation, &l_datarate, &l_length, &ht_length, &stbc, &ch_bw, &is_short_gi, 
                        &apep_length, &num_ext_ss, &is_long_preamble, &resp_frame_len, &next_frame_length, 
            array[2], &bssbasic_set, 
            array[3], 
            array[4], 
            array[5], &return_value);
    
    /* filling txvec here */
    __fill_txvec(&txvec, (uu_uint8)format, (uu_uint8)mcs, (uu_uint8)modulation, (uu_uint8)l_datarate, 
                                (uu_uint16)l_length, (uu_uint16)ht_length, (uu_uint8)stbc, (uu_uint8)ch_bw, 
                                (uu_uint8)is_short_gi, (uu_uint8)apep_length, (uu_uint8)num_ext_ss, (uu_uint8)is_long_preamble);
            
    /*setting BSS service set value */
    BSSBasicRateSet = (uu_uint16)bssbasic_set; 
    /* single protection function call in duration module */
    ret = uu_calc_singlep_frame_duration(&txvec, resp_frame_len, next_frame_length);
    printf("expected duration value %d, calculated duration value %d\n", return_value, ret); 
    
    /* Verification is start here */
    if (ret == return_value)
    {
        printf("single protection frame duration calculation is success\n");
        uu_wlan_test_success_cnt_g++;
        ret = UU_SUCCESS;
    }
    else
    {
        printf("single protection frame duration calculation is failure\n");
        uu_wlan_test_failure_cnt_g++;
        ret = UU_FAILURE;
    }
    return ret;
} /* __test_handler_for_single_protection_frame */

static uu_int32 __test_handler_for_single_pro_rtscts(uu_char *buf)
{
    uu_int32 ret = UU_FAILURE;
    uu_int32 test_category;
    uu_int32 format;
    uu_int32 mcs;
    uu_int32 modulation;
    uu_int32 l_datarate;
    uu_int32 l_length;
    uu_int32 ht_length;
    uu_int32 stbc;
    uu_int32 ch_bw;
    uu_int32 is_short_gi;
    uu_int32 apep_length;
    uu_int32 num_ext_ss;
    uu_int32 is_long_preamble;
    uu_char array[6][40];
    uu_int32 self_cts_flag;
    uu_int32 exp_frame_length;
    uu_int32 rtscts_rate;
    uu_wlan_tx_vector_t txvec;
    uu_int32 return_value;
    uu_int32 bssbasic_set;

    sscanf(buf,"%s %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %x %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &format, &mcs, &modulation, &l_datarate, &l_length, &ht_length, &stbc, &ch_bw, &is_short_gi, 
                        &apep_length, &num_ext_ss, &is_long_preamble, &self_cts_flag, &exp_frame_length, &rtscts_rate, 
            array[2], &bssbasic_set, 
            array[3], 
            array[4], 
            array[5], &return_value);
    
    /* filling txvec here */
    __fill_txvec(&txvec, (uu_uint8)format, (uu_uint8)mcs, (uu_uint8)modulation, (uu_uint8)l_datarate, 
                                (uu_uint16)l_length, (uu_uint16)ht_length, (uu_uint8)stbc, (uu_uint8)ch_bw, 
                                (uu_uint8)is_short_gi, (uu_uint8)apep_length, (uu_uint8)num_ext_ss, (uu_uint8)is_long_preamble); 
    
    /*setting BSS service set value */
    BSSBasicRateSet = (uu_uint16)bssbasic_set; 
             
    /* single protection function call in duration module */
    ret = uu_calc_rtscts_duration (&txvec, (uu_uint8)rtscts_rate, (bool)self_cts_flag, exp_frame_length);
   
    printf("expected duration value %d, calculated duration value %d\n", return_value, ret); 
    /* Verification is start here */
    if (ret == return_value)
    {
        printf("single protection under rts/cts considiration frame duration calculation is success\n");
        uu_wlan_test_success_cnt_g++;
        ret = UU_SUCCESS;
    }
    else
    {
        printf("single protection under rts/cts considiration frame duration calculation is failure\n");
        uu_wlan_test_failure_cnt_g++;
        ret = UU_FAILURE;
    }
    return ret;
} /* __test_handler_for_single_pro_rtscts */

static uu_int32 __test_handler_for_implicit_ba_frame(uu_char *buf)
{
    uu_int32 ret = UU_FAILURE;
    uu_int32 test_category;
    uu_int32 format;
    uu_int32 mcs;
    uu_int32 modulation;
    uu_int32 l_datarate;
    uu_int32 l_length;
    uu_int32 ht_length;
    uu_int32 stbc;
    uu_int32 ch_bw;
    uu_int32 is_short_gi;
    uu_int32 apep_length;
    uu_int32 num_ext_ss;
    uu_int32 is_long_preamble;
    uu_char array[6][40];
    uu_int32 exp_BA_length;
    uu_wlan_tx_vector_t txvec;
    uu_int32 return_value;
    uu_int32 bssbasic_set;
    printf("its in req 2\n");
    sscanf(buf,"%s %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %s %x %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &format, &mcs, &modulation, &l_datarate, &l_length, &ht_length, &stbc, &ch_bw, &is_short_gi, 
                        &apep_length, &num_ext_ss, &is_long_preamble, &exp_BA_length, 
            array[2], &bssbasic_set, 
            array[3], 
            array[4], 
            array[5], &return_value);
    
    printf("its in req 2 after sscanf\n");
    /* filling txvec here */
    __fill_txvec(&txvec, (uu_uint8)format, (uu_uint8)mcs, (uu_uint8)modulation, (uu_uint8)l_datarate, 
                                (uu_uint16)l_length, (uu_uint16)ht_length, (uu_uint8)stbc, (uu_uint8)ch_bw, 
                                (uu_uint8)is_short_gi, (uu_uint8)apep_length, (uu_uint8)num_ext_ss, (uu_uint8)is_long_preamble); 
             
    /*setting BSS service set value */
    BSSBasicRateSet = (uu_uint16)bssbasic_set; 
    /* implicit BA duration function call */
    ret = uu_calc_impl_BA_duration(&txvec, exp_BA_length);
    
    printf("expected duration value %d, calculated duration value %d\n", return_value, ret); 
    /* Verification is start here */
    if (ret == return_value)
    {
        printf("implicit ba frame duration calculation is success\n");
        uu_wlan_test_success_cnt_g++;
        ret = UU_SUCCESS;
    }
    else
    {
        printf("implicit ba frame duration calculation is failure\n");
        uu_wlan_test_failure_cnt_g++;
        ret = UU_FAILURE;
    }
    return ret;
} /* __test_handler_for_implicit_ba_frame */

static uu_int32 __test_handler_for_cal_broadcast_frame_dur(uu_char *buf)
{
    uu_int32 ret = UU_FAILURE;
    uu_int32 test_category;
    uu_int32 format;
    uu_int32 mcs;
    uu_int32 modulation;
    uu_int32 l_datarate;
    uu_int32 l_length;
    uu_int32 ht_length;
    uu_int32 stbc;
    uu_int32 ch_bw;
    uu_int32 is_short_gi;
    uu_int32 apep_length;
    uu_int32 num_ext_ss;
    uu_int32 is_long_preamble;
    uu_char array[6][40];
    uu_wlan_tx_vector_t txvec;
    uu_int32 return_value;
    uu_int32 next_frag_len;

    sscanf(buf,"%s %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %s %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &format, &mcs, &modulation, &l_datarate, &l_length, &ht_length, &stbc, &ch_bw, &is_short_gi, 
                        &apep_length, &num_ext_ss, &is_long_preamble, &next_frag_len,  
            array[2], 
            array[3], 
            array[4], 
            array[5], &return_value);
    
    /* filling txvec here */
    __fill_txvec(&txvec, (uu_uint8)format, (uu_uint8)mcs, (uu_uint8)modulation, (uu_uint8)l_datarate, 
                                (uu_uint16)l_length, (uu_uint16)ht_length, (uu_uint8)stbc, (uu_uint8)ch_bw, 
                                (uu_uint8)is_short_gi, (uu_uint8)apep_length, (uu_uint8)num_ext_ss, (uu_uint8)is_long_preamble); 
             
    /* frame duration function call in duration module */
    ret = uu_calc_bcast_duration(&txvec, next_frag_len);
    
    printf("expected duration value %d, calculated duration value %d\n", return_value, ret); 
    /* Verification is start here */
    if (ret == return_value)
    {
        printf("single protection frame duration calculation is success\n");
        uu_wlan_test_success_cnt_g++;
        ret = UU_SUCCESS;
    }
    else
    {
        printf("single protection frame duration calculation is failure\n");
        uu_wlan_test_failure_cnt_g++;
        ret = UU_FAILURE;
    }
    return ret;
} /* __test_handler_for_cal_frame_dur */

static uu_int32 __test_handler_for_fill_resp_rate_dur(uu_char *buf)
{
    uu_int32 ret = UU_FAILURE;
    uu_int32 test_category;
    uu_int32 format;
    uu_int32 mcs;
    uu_int32 modulation;
    uu_int32 l_datarate;
    uu_int32 fc0;
    uu_int32 duration;
    uu_wlan_cp_rx_frame_info_t info; 
    uu_char array[6][40];
    uu_int32 rett_value;
    uu_int32 ctrl_resp_type;
    uu_int32 ctrl_resp_len;
    uu_int32 bssbasic_set;
    printf("buf...%s\n", buf);
    sscanf(buf,"%s %d %s %d %d %d %d %x %d %d %d %s %x %s %s %s %d\n", 
            array[0], &test_category, 
            array[1], &format, &mcs, &modulation, &l_datarate, &fc0, &duration, &ctrl_resp_type, &ctrl_resp_len,
            array[2], &bssbasic_set, 
            array[3], 
            array[4], 
            array[5], &rett_value);

    info.format = (uu_uint8)format;
    info.mcs = (uu_uint8)mcs;
    info.modulation = (uu_uint8)modulation;  
    info.L_datarate = (uu_uint8)l_datarate;
    info.fc0 = (uu_uint8)fc0;
    info.duration = (uu_uint16)duration;

    /*setting BSS service set value */
    BSSBasicRateSet = (uu_uint16)bssbasic_set; 
    printf("return value before fn call %d\n", rett_value);
    memset(&tx_ctl_frame_info_g, 0, sizeof(uu_wlan_tx_frame_info_t));

    ret = uu_wlan_cp_fill_resp_rate_dur(&info, ctrl_resp_type, ctrl_resp_len);
    
    printf("expected duration value %d, calculated duration value %d\n", rett_value, ret);
 
    /* Verification is start here */
    if (ret == rett_value)
    {
        printf("single protection frame duration calculation is success\n");
        uu_wlan_test_success_cnt_g++;
        ret = UU_SUCCESS;
    }
    else
    {
        printf("single protection frame duration calculation is failure\n");
        uu_wlan_test_failure_cnt_g++;
        ret = UU_FAILURE;
    }
    return ret;

} /* __test_handler_for_fill_resp_rate_dur */



uu_int32 uu_wlan_duration_test_handler(uu_int32 test_category, uu_char *buf)
{
    uu_int32 ret = UU_FAILURE;
    dot11_sifs_timer_value = 16;
    /*parameters are reading here from file*/
    switch(test_category)
    {
        /* single protected frame duration */
        case 0:
            {
                return __test_handler_for_single_protection_frame(buf);
            }
            break;
            /* single protection including rtscts/self-cts frames */
        case 1:
            {
                return __test_handler_for_single_pro_rtscts(buf);
            }
            break;
            /* for calculating implicit ba frame duration */
        case 2:
            {
                return __test_handler_for_implicit_ba_frame(buf);
            }
            break;
            /* for broadcast frame duration */
        case 3:
            {
                return __test_handler_for_cal_broadcast_frame_dur(buf);
            }
            break;
        case 4:
            {
                return __test_handler_for_fill_resp_rate_dur(buf);
            }
            /* For Updating the score board at Tx side */
        default:
            {
                printf("its in default test\n");
                return UU_FAILURE;
            }
            break;
    }
    return ret ;
}



/* EOF */

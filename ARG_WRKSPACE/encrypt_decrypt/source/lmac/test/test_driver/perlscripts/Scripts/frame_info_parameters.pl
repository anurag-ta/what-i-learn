#!/usr/bin/perl 


#Filing up of Frame Info Parameters 
sub frame_info_params
{
    our ($txv_byte1,$txv_byte2,$txv_byte3_4,$txv_byte5_6,$txv_byte7,$txv_byte8,$txv_byte9,$txv_byte10_11,$txv_byte12,$txv_byte13_14,$txv_byte15,$txv_byte16,$txv_byte17_33,$finfo_byte1_3,$finfo_byte4,$finfo_byte5,$finfo_byte6,$finfo_byte7_8,$finfo_byte9_12,$info_byte1_2,$info_byte3,$info_byte4,$info_byte5,$info_byte6,$info_byte7,$info_byte8,$info_byte9_12,$info_byte13_45)="";

    $file1="./config_files/frame_info_parameters.txt";# default frame info parameters
    $file2=$global_frameinfo_file;# frame info parameters need to be overwritten in default parmeters.
    open(File,$file1);
    local $/;
    $default=<File>;
    @newline=split("\n",$default);
    foreach$ind(@newline)
    {
        @arg=split(" ",$ind);
        if($arg[0] eq 'FORMAT')
        {
            $format=sprintf("%02b",$arg[1]);
            $txv_byte1=$format.$txv_byte1;
        }
        elsif($arg[0] eq 'CH_BANDWIDTH')
        {
            $chnbw=sprintf("%03b",$arg[1]);
            $txv_byte1=$chnbw.$txv_byte1;
        }
        elsif($arg[0] eq 'N_TX')
        {
            $n_tx=sprintf("%03b",$arg[1]);
            $txv_byte1=$n_tx.$txv_byte1;
        }
        elsif($arg[0] eq 'TX_POWERLEVEL')
        {
            $tx_pwr=sprintf("%07b",$arg[1]);
            $txv_byte2=$tx_pwr.$txv_byte2;
        }
        elsif($arg[0] eq 'RESERVED')
        {
            $reserved=sprintf("%01b",$arg[1]);
            $txv_byte2=$reserved.$txv_byte2;
        }
        elsif($arg[0] eq 'L_DATARATE')
        {
            $l_datarate=sprintf("%04b",$arg[1]);
            $txv_byte3_4=$l_datarate.$txv_byte3_4;
        }
        elsif($arg[0] eq 'L_LENGTH')
        {
            $l_len=sprintf("%012b",$arg[1]);
            $txv_byte3_4=$l_len.$txv_byte3_4;
        }
        elsif($arg[0] eq 'SERVICE')
        {
            $service=sprintf("%016b",$arg[1]);
            $txv_byte5_6=$service;
        }
        elsif($arg[0] eq 'ANTENNA_SET')
        {
            $ant=sprintf("%08b",$arg[1]);
            $txv_byte7=$ant;
        }
        elsif($arg[0] eq 'NUM_EXT_SPATIALSTREAMS')
        {
            $num_ss=sprintf("%02b",$arg[1]);
            $txv_byte8=$num_ss.$txv_byte8;
        }
        elsif($arg[0] eq 'NO_SIG_EXT')
        {
            $no_sig=sprintf("%01b",$arg[1]);
            $txv_byte8=$no_sig.$txv_byte8;
        }
        elsif($arg[0] eq 'CH_OFFSET')
        {
            $chnoff=sprintf("%02b",$arg[1]);
            $txv_byte8=$chnoff.$txv_byte8;
        }
        elsif($arg[0] eq 'MODULATION')
        {
            $mod=sprintf("%03b",$arg[1]);
            $txv_byte8=$mod.$txv_byte8;
        }
        elsif($arg[0] eq 'MCS')
        {
            $mcs=sprintf("%07b",$arg[1]);
            $txv_byte9=$mcs.$txv_byte9;
        }
        elsif($arg[0] eq 'IS_LONG_PREAMBLE')
        {
            $long_preamble=sprintf("%01b",$arg[1]);
            $txv_byte9=$long_preamble.$txv_byte9;
        }
        elsif($arg[0] eq 'HT_LENGTH')
        {
            $ht_len=sprintf("%016b",$arg[1]);
            $txv_byte10_11=$ht_len.$txv_byte10_11;
        }
        elsif($arg[0] eq 'IS_SMOOTHING')
        {
            $smooth=sprintf("%01b",$arg[1]);
            $txv_byte12=$smooth.$txv_byte12;
        }
        elsif($arg[0] eq 'IS_SOUNDING')
        {
            $sound=sprintf("%01b",$arg[1]);
            $txv_byte12=$sound.$txv_byte12;
        }
        elsif($arg[0] eq 'RESERVED1')
        {
            $reserved1=sprintf("%01b",$arg[1]);
            $txv_byte12=$reserved1.$txv_byte12;
        }
        elsif($arg[0] eq 'IS_AGGREGATED')
        {
            $aggr=sprintf("%01b",$arg[1]);
            $txv_byte12=$aggr.$txv_byte12;
        }
        elsif($arg[0] eq 'STBC')
        {
            $stbc=sprintf("%02b",$arg[1]);
            $txv_byte12=$stbc.$txv_byte12;
        }
        elsif($arg[0] eq 'FECCODING')
        {
            $fec=sprintf("%01b",$arg[1]);
            $txv_byte12=$fec.$txv_byte12;
        }
        elsif($arg[0] eq 'GAURDINTERVAL')
        {
            $gi=sprintf("%01b",$arg[1]);
            $txv_byte12=$gi.$txv_byte12;
        }
        elsif($arg[0] eq 'PARTIAL_AID')
        {
            $par_aid=sprintf("%09b",$arg[1]);
            $txv_byte13_14=$par_aid.$txv_byte13_14;
        }
        elsif($arg[0] eq 'IS_BEAMFORMED')
        {
            $beam=sprintf("%01b",$arg[1]);
            $txv_byte13_14=$beam.$txv_byte13_14;
        }
        elsif($arg[0] eq 'NUM_USERS')
        {
            $num_users=sprintf("%02b",$arg[1]);
            $txv_byte13_14=$num_users.$txv_byte13_14;
        }
        elsif($arg[0] eq 'INDICATED_DYN_BW')
        {
            $dyn_bw=sprintf("%01b",$arg[1]);
            $txv_byte13_14=$dyn_bw.$txv_byte13_14;
        }
        elsif($arg[0] eq 'INDICATED_CHN_BW')
        {
            $chn_bw=sprintf("%03b",$arg[1]);
            $txv_byte13_14=$chn_bw.$txv_byte13_14;
        }
        elsif($arg[0] eq 'GROUP_ID')
        {
            $grp_id=sprintf("%06b",$arg[1]);
            $txv_byte15=$grp_id.$txv_byte15;
        }
        elsif($arg[0] eq 'IS_TX_OP_PS_ALLOWED')
        {
            $tx_op=sprintf("%01b",$arg[1]);
            $txv_byte15=$tx_op.$txv_byte15;
        }
        elsif($arg[0] eq 'IS_TIME_OF_DEP_REQ')
        {
            $time=sprintf("%01b",$arg[1]);
            $txv_byte15=$time.$txv_byte15;
        }
        elsif($arg[0] eq 'PADDING_BYTE0')
        {
            $pad0=sprintf("%08b",$arg[1]);
            $txv_byte16=$pad0;
        }
        elsif($arg[0] eq 'TX_VECTOR_USER_PARAMS')
        {
            $user_params=sprintf("%0128b",$arg[1]);
            $txv_byte17_33=$user_params;
        }
        elsif($arg[0] eq 'FRAMELEN')
        {
            $framelen=sprintf("%020b",$arg[1]);
            $finfo_byte1_3=$framelen;
        }
        elsif($arg[0] eq 'BEACON')
        {
            $beacon=sprintf("%01b",$arg[1]);
            $finfo_byte1_3=$beacon.$finfo_byte1_3;
        }
        elsif($arg[0] eq 'RESERVED1_1')
        {
            $reser1_1=sprintf("%03b",$arg[1]);
            $finfo_byte1_3=$reser1_1.$finfo_byte1_3;
        }
        elsif($arg[0] eq 'IS_AMPDU')
        {
            $ampdu=sprintf("%01b",$arg[1]);
            $finfo_byte4=$ampdu.$finfo_byte4;
        }
        elsif($arg[0] eq 'RETRY')
        {
            $retry=sprintf("%01b",$arg[1]);
            $finfo_byte4=$retry.$finfo_byte4;
        }
        elsif($arg[0] eq 'RESERVED2')
        {
            $reser2=sprintf("%06b",$arg[1]);
            $finfo_byte4=$reser2.$finfo_byte4;
        }
        elsif($arg[0] eq 'RESERVED8')
        {
            $reser8=sprintf("%08b",$arg[1]);
            $finfo_byte5=$reser8;
        }
        elsif($arg[0] eq 'KEYTYPE')
        {
            $key=sprintf("%02b",$arg[1]);
            $finfo_byte6=$key.$finfo_byte6;
        }
        elsif($arg[0] eq 'KEYIX')
        {
            $keyix=sprintf("%06b",$arg[1]);
            $finfo_byte6=$keyix.$finfo_byte6;
        }
        elsif($arg[0] eq 'RESERVED3')
        {
            $reser3=sprintf("%016b",$arg[1]);
            $finfo_byte7_8=$reser3.$finfo_byte7_8;
        }
=head
        elsif($arg[0] eq 'TIMESTAMP_MSB')
        {
            $tstamp_msb=sprintf("%032b",$arg[1]);
            $finfo_byte9_12=$tstamp_msb;
        }
        elsif($arg[0] eq 'TIMESTAMP_LSB')
        {
            $timestamp_lsb=sprintf("%032b",$arg[1]);
            $finfo_byte9_12=$timestamp_lsb.$finfo_byte9_12;
        }
        elsif($arg[0] eq 'RX_BCN_TIMESTAMP_MSB_TSF')
        {
            $rx_bcn_msb=sprintf("%032b",$arg[1]);
            $finfo_byte9_12=$rx_bcn_msb.$finfo_byte9_12;
        }
        elsif($arg[0] eq 'RX_BCN_TIMESTAMP_LSB_TSF')
        {
            $rx_bcn_lsb=sprintf("%032b",$arg[1]);
            $finfo_byte9_12=$rx_bcn_lsb.$finfo_byte9_12;
        }
=cut
        elsif($arg[0] eq 'RESERVED16')
        {
            $reser16=sprintf("%016b",$arg[1]);
            $info_byte1_2=$reser16;
        }
        elsif($arg[0] eq 'AC')
        {
            $ac=sprintf("%02b",$arg[1]);
            $info_byte3=$ac.$info_byte3;
        }
        elsif($arg[0] eq 'RESERVED1_2')
        {
            $reser1_2=sprintf("%06b",$arg[1]);
            $info_byte3=$reser1_2.$info_byte3;
        }
        elsif($arg[0] eq 'STBC_FB')
        {
            $stbc_fb=sprintf("%02b",$arg[1]);
            $info_byte4=$stbc_fb.$info_byte4;
        }
        elsif($arg[0] eq 'N_TX_FB')
        {
            $n_tx_fb=sprintf("%03b",$arg[1]);
            $info_byte4=$n_tx_fb.$info_byte4;
        }
        elsif($arg[0] eq 'RESERVED2_1')
        {
            $reser2_1=sprintf("%03b",$arg[1]);
            $info_byte4=$reser2_1.$info_byte4;
        }
        elsif($arg[0] eq 'MCS_FB')
        {
            $mcs_fb=sprintf("%07b",$arg[1]);
            $info_byte5=$mcs_fb.$info_byte5;
        }
        elsif($arg[0] eq 'RESERVED3_1')
        {
            $reser3_1=sprintf("%01b",$arg[1]);
            $info_byte5=$reser3_1.$info_byte5;
        }
        elsif($arg[0] eq 'FALLBACK_RATE')
        {
            $fb_rate=sprintf("%04b",$arg[1]);
            $info_byte6=$fb_rate.$info_byte6;
        }
        elsif($arg[0] eq 'RESERVED4')
        {
            $reser4=sprintf("%04b",$arg[1]);
            $info_byte6=$reser4.$info_byte6;
        }
        elsif($arg[0] eq 'RTSCTS_RATE')
        {
            $rts_cts=sprintf("%04b",$arg[1]);
            $info_byte7=$rts_cts.$info_byte7;
        }
        elsif($arg[0] eq 'RTSCTS_RATE_FB')
        {
            $rts_cts_fb=sprintf("%04b",$arg[1]);
            $info_byte7=$rts_cts_fb.$info_byte7;
        }
        elsif($arg[0] eq 'AGGR_COUNT')
        {
            $aggr_count=sprintf("%08b",$arg[1]);
            $info_byte8=$aggr_count;
        }
        elsif($arg[0] eq 'NUM_STS_FB')
        {
            $num_sts_fb=sprintf("%032b",$arg[1]);
            $info_byte9_12=$num_sts_fb.$info_byte9_12;
        }
        elsif($arg[0] eq 'AMPDU_INFO')
        {
            $ampdu_info=sprintf("%0256b",$arg[1]);
            $info_byte13_45=$ampdu_info;
        }

    }      
    open(File,$file2);
    local $/;
    $data1=<File>;
    @split=split("\n",$data1);
    foreach$msg(@split)
    {
        @arg=split(" ",$msg);
        if($arg[0] eq 'FORMAT')
        {
	    $format_modified = $arg[1];
            $format_changed=sprintf("%02b",$arg[1]);
            $new=substr $txv_byte1,-2,2, "$format_changed";
        }
        elsif($arg[0] eq 'CH_BANDWIDTH')
        {
	our $mody_bw = $arg[1];
            $chnbw_changed=sprintf("%03b",$arg[1]);
            $new=substr $txv_byte1,3,3, "$chnbw_changed";
        }
        elsif($arg[0] eq 'N_TX')
        {
            $n_tx_changed=sprintf("%03b",$arg[1]);
            $new=substr $txv_byte1,0,3, "$n_tx_changed";
        }
        elsif($arg[0] eq 'TX_POWERLEVEL')
        {
            $tx_pwr_changed=sprintf("%07b",$arg[1]);
            $new=substr $txv_byte2,1,7, "$tx_pwr_changed";
        }
        elsif($arg[0] eq 'RESERVED')
        {
            $reserved_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte2,0,1, "$reserved_changed";
        }
        elsif($arg[0] eq 'L_DATARATE')
        {
	    $l_datarate_modified=sprintf("%x",$arg[1]);
            $l_datarate_changed=sprintf("%04b",$arg[1]);
            $new=substr $txv_byte3_4,-4,4, "$l_datarate_changed";
        }
        elsif($arg[0] eq 'L_LENGTH')
        {
            $l_len_changed=sprintf("%012b",$arg[1]);
            $new=substr $txv_byte3_4,0,12, "$l_len_changed";
        }
        elsif($arg[0] eq 'SERVICE')
        {
            $service_changed=sprintf("%016b",$arg[1]);
            $new=substr $txv_byte5_6,0,16, "$service_changed";
        }
        elsif($arg[0] eq 'ANTENNA_SET')
        {
            $ant_changed=sprintf("%08b",$arg[1]);
            $new=substr $txv_byte7,0,8, "$ant_changed";
        }
        elsif($arg[0] eq 'NUM_EXT_SPATIALSTREAMS')
        {
            $num_ss_changed=sprintf("%02b",$arg[1]);
            $new=substr $txv_byte8,-2,2,"$num_ss_changed";
        }
        elsif($arg[0] eq 'NO_SIG_EXT')
        {
            $no_sig_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte8,5,1,"$no_sig_changed";
        }
        elsif($arg[0] eq 'CH_OFFSET')
        {
            $chnoff_changed=sprintf("%02b",$arg[1]);
            $new=substr $txv_byte8,3,2,"$chnoff_changed";
        }
        elsif($arg[0] eq 'MODULATION')
        {
            $mod_changed=sprintf("%03b",$arg[1]);
            $new=substr $txv_byte8,0,3,"$mod_changed";
        }
        elsif($arg[0] eq 'MCS')
        {
	    $ht_mcs_changed = $arg[1];
            $mcs_changed=sprintf("%07b",$arg[1]);
            $new=substr $txv_byte9,1,7,"$mcs_changed";
        }
        elsif($arg[0] eq 'IS_LONG_PREAMBLE')
        {
            $long_preamble_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte9,0,1,"$long_preamble_changed";
        }
        elsif($arg[0] eq 'HT_LENGTH')
        {
            $ht_len_changed=sprintf("%016b",$arg[1]);
            $new=substr $txv_byte10_11,0,16,"$ht_len_changed";
        }
        elsif($arg[0] eq 'IS_SMOOTHING')
        {
            $smooth_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte12,7,1,"$smooth_changed";
        }
        elsif($arg[0] eq 'IS_SOUNDING')
        {
            $sound_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte12,6,1,"$sound_changed";
        }
        elsif($arg[0] eq 'RESERVED1')
        {
            $reserved1_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte12,5,1, "$reserved1_changed";
        }
        elsif($arg[0] eq 'IS_AGGREGATED')
        {
            $aggr_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte12,4,1, "$aggr_changed";
        }
        elsif($arg[0] eq 'STBC')
        {
            $stbc_changed=sprintf("%02b",$arg[1]);
            $new=substr $txv_byte12,2,2, "$stbc_changed";
        }
        elsif($arg[0] eq 'FECCODING')
        {
            $fec_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte12,1,1, "$fec_changed";
        }
        elsif($arg[0] eq 'GAURDINTERVAL')
        {
	   our $mody_gi=$arg[1];
            $gi_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte12,0,1, "$gi_changed";
        }
        elsif($arg[0] eq 'PARTIAL_AID')
        {
            $par_aid_changed=sprintf("%09b",$arg[1]);
            $new=substr $txv_byte13_14,7,9,"$par_aid_changed";
        }
        elsif($arg[0] eq 'IS_BEAMFORMED')
        {
            $beam_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte13_14,6,1,"$beam_changed";
        }
        elsif($arg[0] eq 'NUM_USERS')
        {
            $num_users_changed=sprintf("%02b",$arg[1]);
            $new=substr $txv_byte13_14,4,2,"$num_users_changed";
        }
        elsif($arg[0] eq 'INDICATED_DYN_BW')
        {
            $dyn_bw_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte13_14,3,1,"$dyn_bw_changed";
        }
        elsif($arg[0] eq 'INDICATED_CHN_BW')
        {
            $chn_bw_changed=sprintf("%03b",$arg[1]);
            $new=substr $txv_byte13_14,0,3,"$chn_bw_changed";
        }
        elsif($arg[0] eq 'GROUP_ID')
        {
            $grp_id_changed=sprintf("%06b",$arg[1]);
            $new=substr $txv_byte15,2,6,"$grp_id_changed";
        }
        elsif($arg[0] eq 'IS_TX_OP_PS_ALLOWED')
        {
            $tx_op_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte15,1,1,"$tx_op_changed";
        }
        elsif($arg[0] eq 'IS_TIME_OF_DEP_REQ')
        {
            $time_changed=sprintf("%01b",$arg[1]);
            $new=substr $txv_byte15,0,1,"$time_changed";
        }
        elsif($arg[0] eq 'PADDING_BYTE0')
        {
            $pad0_changed=sprintf("%08b",$arg[1]);
            $new=substr $txv_byte16,0,8,"$pad0_changed";
        }
        elsif($arg[0] eq 'TX_VECTOR_USER_PARAMS')
        {
             $vht_mcs_changed=sprintf("%04x",$arg[1]);
	    $vht_mcs_changed =substr $vht_mcs_changed,0,1;
            $user_params_changed=sprintf("%0128b",$arg[1]);
            $new=substr $txv_byte17_33,0,128, "$user_params_changed";
        }
        elsif($arg[0] eq 'FRAMELEN')
        {
            $framelen_changed=sprintf("%020b",$arg[1]);
            $new=substr $finfo_byte1_3,4,20, "$framelen_changed";
        }
        elsif($arg[0] eq 'BEACON')
        {
            $beacon_changed=sprintf("%01b",$arg[1]);
            $new=substr $finfo_byte1_3,3,1, "$beacon_changed";
        }
        elsif($arg[0] eq 'RESERVED1_1')
        {
            $reser1_1_changed=sprintf("%01b",$arg[1]);
            $new=substr $finfo_byte1_3,0,3, "$reser1_1_changed";
        }
        elsif($arg[0] eq 'IS_AMPDU')
        {
            $ampdu_changed=sprintf("%01b",$arg[1]);
            $new=substr $finfo_byte4,7,1,"$ampdu_changed";
        }
        elsif($arg[0] eq 'RETRY')
        {
            $retry_changed=sprintf("%01b",$arg[1]);
            $new=substr $finfo_byte4,6,1,"$retry_changed";
        }
        elsif($arg[0] eq 'RESERVED2')
        {
            $reser2_changed=sprintf("%06b",$arg[1]);
            $new=substr $finfo_byte4,0,6,"$reser2_changed";
        }
        elsif($arg[0] eq 'RESERVED8')
        {
            $reser8_changed=sprintf("%08b",$arg[1]);
            $new=substr $finfo_byte5,0,8,"$reser8_changed";
        }
        elsif($arg[0] eq 'KEYTYPE')
        {
            $key_changed=sprintf("%02b",$arg[1]);
            $new=substr $finfo_byte6,6,2,"$key_changed";
        }
        elsif($arg[0] eq 'KEYIX')
        {
            $keyix_changed=sprintf("%06b",$arg[1]);
            $new=substr $finfo_byte6,0,6,"$keyix_changed";
        }
        elsif($arg[0] eq 'RESERVED3')
        {
            $reser3_changed=sprintf("%016b",$arg[1]);
            $new=substr $finfo_byte7_8,0,16,"$reser3_changed";
        }
=head
        elsif($arg[0] eq 'TIMESTAMP_MSB')
        {
            $tstamp_msb_changed=sprintf("%032b",$arg[1]);
            $new=substr $finfo_byte9_12,0,32,"$tstamp_msb_changed";
        }
        elsif($arg[0] eq 'TIMESTAMP_LSB')
        {
            $tstamp_lsb_changed=sprintf("%032b",$arg[1]);
            $new=substr $finfo_byte9_12,32,32,"$tstamp_lsb_changed";
        }
        elsif($arg[0] eq 'RX_BCN_TIMESTAMP_MSB_TSF')
        {
            $rx_bcn_msb_changed=sprintf("%032b",$arg[1]);
            $new=substr $finfo_byte9_12,64,32,"$rx_bcn_msb_changed";
        }
        elsif($arg[0] eq 'RX_BCN_TIMESTAMP_LSB_TSF')
        {
            $rx_bcn_lsb_changed=sprintf("%032b",$arg[1]);
            $new=substr $finfo_byte9_12,96,32,"$rx_bcn_lsb_changed";
        }
=cut
        elsif($arg[0] eq 'RESERVED16')
        {
            $reser16_changed=sprintf("%016b",$arg[1]);
            $new=substr $info_byte1_2,0,16,"$reser16_changed";
        }
        elsif($arg[0] eq 'AC')
        {
            $ac_changed=sprintf("%02b",$arg[1]);
            $new=substr $info_byte3,6,2,"$ac_changed";
        }
        elsif($arg[0] eq 'RESERVED1_2')
        {
            $reser1_2_changed=sprintf("%06b",$arg[1]);
            $new=substr $info_byte3,0,6,"$reser1_2_changed";
        }
        elsif($arg[0] eq 'STBC_FB')
        {
            $stbc_fb_changed=sprintf("%02b",$arg[1]);
            $new=substr $info_byte4,6,2,"$stbc_fb_changed";
        }
        elsif($arg[0] eq 'N_TX_FB')
        {
            $n_tx_fb_changed=sprintf("%03b",$arg[1]);
            $new=substr $info_byte4,3,3,"$n_tx_fb_changed";
        }
        elsif($arg[0] eq 'RESERVED2_1')
        {
            $reser2_1_changed=sprintf("%03b",$arg[1]);
            $new=substr $info_byte4,0,3,"$reser2_1_changed";
        }
        elsif($arg[0] eq 'MCS_FB')
        {
            $mcs_fb_changed=sprintf("%07b",$arg[1]);
            $new=substr $info_byte5,1,7,"$mcs_fb_changed";
        }
        elsif($arg[0] eq 'RESERVED3_1')
        {
            $reser3_1_changed=sprintf("%01b",$arg[1]);
            $new=substr $info_byte5,0,1,"$reser3_1_changed";
        }
        elsif($arg[0] eq 'FALLBACK_RATE')
        {
            $fb_rate_changed=sprintf("%04b",$arg[1]);
            $new=substr $info_byte6,4,4,"$fb_rate_changed";
        }
        elsif($arg[0] eq 'RESERVED4')
        {
            $reser4_changed=sprintf("%04b",$arg[1]);
            $new=substr $info_byte6,0,4,"$reser4_changed";
        }
        elsif($arg[0] eq 'RTSCTS_RATE')
        {
            $rts_cts_changed=sprintf("%04b",$arg[1]);
            $new=substr $info_byte7,4,4,"$rts_cts_changed";
        }
        elsif($arg[0] eq 'RTSCTS_RATE_FB')
        {
            $rts_cts_fb_changed=sprintf("%04b",$arg[1]);
            $new=substr $info_byte7,0,4,"$rts_cts_fb_changed";
        }
        elsif($arg[0] eq 'AGGR_COUNT')
        {
            $aggr_count_changed=sprintf("%08b",$arg[1]);
            $new=substr $info_byte8,0,8,"$aggr_count_changed";
        }
        elsif($arg[0] eq 'NUM_STS_FB')
        {
            $num_sts_fb_changed=sprintf("%032b",$arg[1]);
            $new=substr $info_byte9_12,0,32,"$num_sts_fb_changed";
        }
        elsif($arg[0] eq 'AMPDU_INFO')
        {
            $ampdu_info_changed=sprintf("%0256b",$arg[1]);
            $new=substr $info_byte13_45,0,256,"$ampdu_info_changed";
        }
    }
    $hex_byte1=sprintf("%02x",oct("0b".$txv_byte1));
    $hex_byte2=sprintf("%02x",oct("0b".$txv_byte2));
    $hex_byte3_4=sprintf("%04x",oct("0b".$txv_byte3_4));
    @array=unpack("((A2)*)", $hex_byte3_4);
    $hex_byte3_4=$array[1].$array[0]; 
    $hex_byte5_6=sprintf("%04x",oct("0b".$txv_byte5_6));
    @array=unpack("((A2)*)", $hex_byte5_6);
    $hex_byte5_6=$array[1].$array[0]; 
    $hex_byte7=sprintf("%02x",oct("0b".$txv_byte7));
    $hex_byte8=sprintf("%02x",oct("0b".$txv_byte8));
    $hex_byte9=sprintf("%02x",oct("0b".$txv_byte9));
    $hex_byte10_11=sprintf("%04x",oct("0b".$txv_byte10_11));
    @array=unpack("((A2)*)", $hex_byte10_11);
    $hex_byte10_11=$array[1].$array[0]; 
    $hex_byte12=sprintf("%02x",oct("0b".$txv_byte12));
    $hex_byte13_14=sprintf("%04x",oct("0b".$txv_byte13_14));
    @array=unpack("((A2)*)", $hex_byte13_14);
    $hex_byte13_14=$array[1].$array[0]; 
    $hex_byte15=sprintf("%02x",oct("0b".$txv_byte15));
    $hex_byte16=sprintf("%02x",oct("0b".$txv_byte16));
    $hex_byte17_33=sprintf("%032x",oct("0b".$txv_byte17_33));
    @array=unpack("((A2)*)", $hex_byte17_33);
    $hex_byte17_33=$array[15].$array[14].$array[13].$array[12].$array[11].$array[10].$array[9].$array[8].$array[7].$array[6].$array[5].$array[4].$array[3].$array[2].$array[1].$array[0]; 

    $hex_finfo_byte1_3=sprintf("%06x",oct("0b".$finfo_byte1_3));
    @array=unpack("((A2)*)", $hex_finfo_byte1_3);
    $hex_finfo_byte1_3=$array[2].$array[1].$array[0]; 
    $hex_finfo_byte4=sprintf("%02x",oct("0b".$finfo_byte4));
    $hex_finfo_byte5=sprintf("%02x",oct("0b".$finfo_byte5));
    $hex_finfo_byte6=sprintf("%02x",oct("0b".$finfo_byte6));
    $hex_finfo_byte7_8=sprintf("%04x",oct("0b".$finfo_byte7_8));
    @array=unpack("((A2)*)", $hex_finfo_byte7_8);
    $hex_finfo_byte7_8=$array[1].$array[0];
    #$hex_finfo_byte9_12=sprintf("%032x",oct("0b".$finfo_byte9_12));
#    @array=unpack("((A16)*)", $hex_finfo_byte9_12);
#    $hex_finfo_byte9_12=$array[15].$array[14].$array[13].$array[12].$array[11].$array[10].$array[9].$array[8].$array[7].$array[6].$array[5].$array[4].$array[3].$array[2].$array[1].$array[0];


    $hex_info_byte1_2=sprintf("%04x",oct("0b".$info_byte1_2));
    @array=unpack("((A2)*)", $hex_info_byte1_2);
    $hex_info_byte1_2=$array[1].$array[0]; 
    $hex_info_byte3=sprintf("%02x",oct("0b".$info_byte3));
    $hex_info_byte4=sprintf("%02x",oct("0b".$info_byte4));
    $hex_info_byte5=sprintf("%02x",oct("0b".$info_byte5));
    $hex_info_byte6=sprintf("%02x",oct("0b".$info_byte6));
    $hex_info_byte7=sprintf("%02x",oct("0b".$info_byte7));
    $hex_info_byte8=sprintf("%02x",oct("0b".$info_byte8));
    $hex_info_byte9_12=sprintf("%08x",oct("0b".$info_byte9_12));
    @array=unpack("((A2)*)", $hex_info_byte9_12);
    $hex_info_byte9_12=$array[3].$array[2].$array[1].$array[0]; 
    $hex_info_byte13_45=sprintf("%064x",oct("0b".$info_byte13_45));


    $txvect=$hex_byte1.$hex_byte2.$hex_byte3_4.$hex_byte5_6.$hex_byte7.$hex_byte8.$hex_byte9.$hex_byte10_11.$hex_byte12.$hex_byte13_14.$hex_byte15.$hex_byte16.$hex_byte17_33;
    $frame_info=$hex_finfo_byte1_3.$hex_finfo_byte4.$hex_finfo_byte5.$hex_finfo_byte6.$hex_finfo_byte7_8;#.$hex_finfo_byte9_12;
    $other_info=$hex_info_byte1_2.$hex_info_byte3.$hex_info_byte4.$hex_info_byte5.$hex_info_byte6.$hex_info_byte7.$hex_info_byte8.$hex_info_byte9_12.$hex_info_byte13_45;
    $is_aggr=substr($txvect,22,2);
    $newfile=$txvect.$frame_info.$other_info;
    return 0;
}
1 #Not to be removed. Indicates end of the perl module.


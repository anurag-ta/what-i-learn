#!/usr/bin/perl 

#Filling up of UMAC-LMAC interface registers (Global registers)
sub default_registers
{
    our ($timers,$cw_values,$modes,$bi_value,$values,$reserved_one,$reserved,$threshold,$bss,$address,$bssid,$tsf,$ibss_reg,$ps,$registers,$multi)='';
    $file1="./config_files/default_registers.txt";# default registers
        $file2=$global_registers_file;# frame info parameters need to be overwritten in default parmeters.
        open(File,$file1);
    local $/;
    $default=<File>;
    @newline=split("\n",$default);
    foreach$ind(@newline)
    {
        @arg=split(" ",$ind);
        if($arg[0] eq 'SLOT_TIMER')
        {
            $slot=sprintf("%02x",$arg[1]);
            $timers=$slot;
        }
        elsif($arg[0] eq 'SIFS_TIMER')
        {
            $sifs=sprintf("%02x",$arg[1]);
            $timers=$timers.$sifs;
        }
        elsif($arg[0] eq 'DIFS_TIMER')
        {
            $difs=sprintf("%02x",$arg[1]);
            $timers=$timers.$difs;
        }
        elsif($arg[0] eq 'PHY_DELAY')
        {
            $phy_delay=sprintf("%02x",$arg[1]);
            $timers=$timers.$phy_delay;
        }
        elsif($arg[0] eq 'ACK_TIMER')
        {
            $ack=sprintf("%02x",$arg[1]);
            $timers=$timers.$ack;
        }
        elsif($arg[0] eq 'CTS_TIMER')
        {
            $cts=sprintf("%02x",$arg[1]);
            $timers=$timers.$cts;
        }
        elsif($arg[0] eq 'SRC')
        {
            $src=sprintf("%02x",$arg[1]);
            $timers=$timers.$src;
        }
        elsif($arg[0] eq 'LRC')
        {
            $lrc=sprintf("%02x",$arg[1]);
            $timers=$timers.$lrc;
        }
        elsif($arg[0] eq 'CWMIN_VALUE')
        {
            $cwmin=sprintf("%02x",$arg[1]);
            $cw_values=$cwmin;
        }
        elsif($arg[0] eq 'CWMAX_VALUE')
        {
            $cwmax=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmax;
        }
        elsif($arg[0] eq 'CWMIN_AC[0]')
        {
            $cwmin_ac0=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmin_ac0;
        }
        elsif($arg[0] eq 'CWMIN_AC[1]')
        {
            $cwmin_ac1=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmin_ac1;
        }
        elsif($arg[0] eq 'CWMIN_AC[2]')
        {
            $cwmin_ac2=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmin_ac2;
        }
        elsif($arg[0] eq 'CWMIN_AC[3]')
        {
            $cwmin_ac3=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmin_ac3;
        }
        elsif($arg[0] eq 'CWMIN_AC[4]')
        {
            $cwmin_ac4=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmin_ac4;
        }
        elsif($arg[0] eq 'CWMAX_AC[0]')
        {
            $cwmax_ac0=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmax_ac0;
        }
        elsif($arg[0] eq 'CWMAX_AC[1]')
        {
            $cwmax_ac1=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmax_ac1;
        }       
        elsif($arg[0] eq 'CWMAX_AC[2]')
        {
            $cwmax_ac2=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmax_ac2;
        }       
        elsif($arg[0] eq 'CWMAX_AC[3]')
        {
            $cwmax_ac3=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmax_ac3;
        }
        elsif($arg[0] eq 'CWMAX_AC[4]')
        {
            $cwmax_ac4=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$cwmax_ac4;
        }       
        elsif($arg[0] eq 'TXOP_LIMIT[0]')
        {
            $txop_limit0=sprintf("%08x",$arg[1]);
            $cw_values=$cw_values.$txop_limit0;
        }
        elsif($arg[0] eq 'TXOP_LIMIT[1]')
        {
            $txop_limit1=sprintf("%08x",$arg[1]);
            $cw_values=$cw_values.$txop_limit1;
        }
        elsif($arg[0] eq 'TXOP_LIMIT[2]')
        {
            $txop_limit2=sprintf("%08x",$arg[1]);
	    @arr=unpack("(A2)*",$txop_limit2);
	    $txop_limit2=$arr[3].$arr[2].$arr[1].$arr[0];
            $cw_values=$cw_values.$txop_limit2;
        }
        elsif($arg[0] eq 'TXOP_LIMIT[3]')
        {
            $txop_limit3=sprintf("%08x",$arg[1]);
            @arr=unpack("(A2)*",$txop_limit3);
            $txop_limit3=$arr[3].$arr[2].$arr[1].$arr[0];
	    $cw_values=$cw_values.$txop_limit3;
        }
        elsif($arg[0] eq 'TXOP_LIMIT[4]')
        {
            $txop_limit4=sprintf("%08x",$arg[1]);
            $cw_values=$cw_values.$txop_limit4;
        }
        elsif($arg[0] eq 'AIFS_VALUE[0]')
        {
            $aifs_value0=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$aifs_value0;
        }
        elsif($arg[0] eq 'AIFS_VALUE[1]')
        {
            $aifs_value1=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$aifs_value1;
        }
        elsif($arg[0] eq 'AIFS_VALUE[2]')
        {
            $aifs_value2=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$aifs_value2;
        }
        elsif($arg[0] eq 'AIFS_VALUE[3]')
        {
            $aifs_value3=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$aifs_value3;
        }
        elsif($arg[0] eq 'AIFS_VALUE[4]')
        {
            $aifs_value4=sprintf("%02x",$arg[1]);
            $cw_values=$cw_values.$aifs_value4;
        }
        elsif($arg[0] eq 'OP_MODE')
        {
            $op=sprintf("%02x",$arg[1]);
            $modes=$modes.$op;
        }
        elsif($arg[0] eq 'QOS_MODE')
        {
            $qos=sprintf("%02x",$arg[1]);
            $modes=$modes.$qos;
        }
	elsif($arg[0] eq 'RESERVED_ONE')
	{
	    $reserved_one=sprintf("%02x",$arg[1]);
	    
	}
        elsif($arg[0] eq 'BEACON_INTERVAL')
        {
            $bi=sprintf("%08x",$arg[1]);
            $bi_value=$bi;
        }
        elsif($arg[0] eq 'SIGNAL_EXTENSION')
        {
            $signal=sprintf("%02x",$arg[1]);
            $values=$values.$signal;
        }
        elsif($arg[0] eq 'SELF_CTS')
        {
            $self_cts=sprintf("%02x",$arg[1]);
            $values=$values.$self_cts;
        }
        elsif($arg[0] eq 'ACK_FAILURE')
        {
            $ack_fail=sprintf("%02x",$arg[1]);
            $values=$values.$ack_fail;
        }
        elsif($arg[0] eq 'RTS_FAILURE')
        {
            $rts_fail=sprintf("%02x",$arg[1]);
            $values=$values.$rts_fail;
        }
        elsif($arg[0] eq 'RTS_SUCCESS')
        {
            $rts_pass=sprintf("%02x",$arg[1]);
            $values=$values.$rts_pass;
        }
        elsif($arg[0] eq 'FCS_ERROR')
        {
            $fcs_error=sprintf("%02x",$arg[1]);
            $values=$values.$fcs_error;
        }
        elsif($arg[0] eq 'RX_FRAMES')
        {
            $rx_frames=sprintf("%02x",$arg[1]);
            $values=$values.$rx_frames;
        }
        elsif($arg[0] eq 'RX_MULTICAST')
        {
            $rx_mcast=sprintf("%02x",$arg[1]);
            $values=$values.$rx_mcast;
        }
        elsif($arg[0] eq 'RX_BROADCAST')
        {
            $rx_bcast=sprintf("%02x",$arg[1]);
            $values=$values.$rx_bcast;
        }
        elsif($arg[0] eq 'RX_FRAME_US')
        {
            $rx_us=sprintf("%02x",$arg[1]);
            $values=$values.$rx_us;
        }
        elsif($arg[0] eq 'RX_AMPDU_FRAMES')
        {
            $rx_ampdu=sprintf("%02x",$arg[1]);
            $values=$values.$rx_ampdu;
        }
        elsif($arg[0] eq 'RX_AMPDU_SUBFRAMES')
        {
            $rx_subframes=sprintf("%02x",$arg[1]);
            $values=$values.$rx_subframes;
        }
        elsif($arg[0] eq 'RX_PHY_ERROR')
        {
            $rx_phy=sprintf("%02x",$arg[1]);
            $values=$values.$rx_phy;
        }
        elsif($arg[0] eq 'RESERVED')
        {
            $reser=sprintf("%06x",$arg[1]);
            $values=$values.$reser;
        }
        elsif($arg[0] eq 'RTS_THRESHOLD')
        {
            $rts=sprintf("%04x",$arg[1]);
            $threshold=$rts;
        }
        elsif($arg[0] eq 'BSSBASICRATESET')
        {
            $bss_basic=sprintf("%04x",$arg[1]);
            $bss=$bss_basic;
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[0]')
        {
            $sta0=sprintf("%02x",$arg[1]);
            $address=$address.$sta0;
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[1]')
        {
            $sta1=sprintf("%02x",$arg[1]);
            $address=$address.$sta1;
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[2]')
        {
            $sta2=sprintf("%02x",$arg[1]);
            $address=$address.$sta2;
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[3]')
        {
            $sta3=sprintf("%02x",$arg[1]);
            $address=$address.$sta3;
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[4]')
        {
            $sta4=sprintf("%02x",$arg[1]);
            $address=$address.$sta4;
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[5]')
        {
            $sta5=sprintf("%02x",$arg[1]);
            $address=$address.$sta5;
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[0]')
        {
            $bss0=sprintf("%02x",$arg[1]);
            $bssid=$bssid.$bss0;
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[1]')
        {
            $bss1=sprintf("%02x",$arg[1]);
            $bssid=$bssid.$bss1;
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[2]')
        {
            $bss2=sprintf("%02x",$arg[1]);
            $bssid=$bssid.$bss2;
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[3]')
        {
            $bss3=sprintf("%02x",$arg[1]);
            $bssid=$bssid.$bss3;
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[4]')
        {
            $bss4=sprintf("%02x",$arg[1]);
            $bssid=$bssid.$bss4;
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[5]')
        {
            $bss5=sprintf("%02x",$arg[1]);
            $bssid=$bssid.$bss5;
        }
        elsif($arg[0] eq 'RESERVED6')
        {
            $reser6=sprintf("%04x",$arg[1]);
            $bssid=$bssid.$reser6;
        }
=head
        elsif($arg[0] eq 'TSF')
        {
            $tsf=sprintf("%016x",$arg[1]);
            $tsf=$tsf;
        }
        elsif($arg[0] eq 'TSF_UNLOCK')
        {
            $tsf_unlock=sprintf("%02x",$arg[1]);
            $ibss_reg=$ibss_reg.$tsf_unlock;
        }
=cut
        elsif($arg[0] eq 'LAST_BEACON_TX')
        {
            $last_beacon=sprintf("%02x",$arg[1]);
            $ibss_reg=$ibss_reg.$last_beacon;
        }
        elsif($arg[0] eq 'PHY_TX_DELAY')
        {
            $phy_tx_delay=sprintf("%02x",$arg[1]);
            $ibss_reg=$ibss_reg.$phy_tx_delay;
        }
=head
        elsif($arg[0] eq 'IBSS_SEND_BEACON')
        {
            $send_beacon=sprintf("%02x",$arg[1]);
            $ibss_reg=$ibss_reg.$send_beacon;
        }
=cut
        elsif($arg[0] eq 'PS_MODE')
        {
            $ps_mode=sprintf("%02x",$arg[1]);
            $ps=$ps_mode;
        }
        elsif($arg[0] eq 'UAPSD_MODE')
        {
            $uapsd_mode=sprintf("%02x",$arg[1]);
            $ps=$ps.$uapsd_mode;
        }
        elsif($arg[0] eq 'MULTIDATA')
        {
            $multi=sprintf("%02x",$arg[1]);
            $multi=$multi;
        }	

    }

    open(File,$file2);
    local $/;
    $data1=<File>;
    @split=split("\n",$data1);
    foreach$msg(@split)
    {
        @arg=split(" ",$msg);
        if($arg[0] eq 'SLOT_TIMER')
        {
            $slot_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,0,2, "$slot_new";
        }
        elsif($arg[0] eq 'SIFS_TIMER')
        {
            $sifs_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,2,2, "$sifs_new";
        }
        elsif($arg[0] eq 'DIFS_TIMER')
        {
            $difs_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,4,2, "$difs_new";
        }
        elsif($arg[0] eq 'PHY_DELAY')
        {
            $phy_delay_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,6,2, "$phy_delay_new";
        }
        elsif($arg[0] eq 'ACK_TIMER')
        {
            $ack_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,8,2, "$ack_new";
        }
        elsif($arg[0] eq 'CTS_TIMER')
        {
            $cts_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,10,2, "$cts_new";
        }
        elsif($arg[0] eq 'SRC')
        {
            $src_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,12,2, "$src_new";
        }
        elsif($arg[0] eq 'LRC')
        {
            $lrc_new=sprintf("%02x",$arg[1]);
            $new=substr $timers,14,2, "$lrc_new";
        }
        elsif($arg[0] eq 'CWMIN_VALUE')
        {
            $cwmin_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,0,2, "$cwmin_new";
        }
        elsif($arg[0] eq 'CWMAX_VALUE')
        {
            $cwmax_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,2,2, "$cwmax_new";
        }
        elsif($arg[0] eq 'CWMIN_AC[0]')
        {
            $cwmin_ac0_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,4,2, "$cwmin_ac0_new";
        }
        elsif($arg[0] eq 'CWMIN_AC[1]')
        {
            $cwmin_ac1_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,6,2, "$cwmin_ac1_new";
        }
        elsif($arg[0] eq 'CWMIN_AC[2]')
        {
            $cwmin_ac2_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,8,2, "$cwmin_ac2_new";
        }
        elsif($arg[0] eq 'CWMIN_AC[3]')
        {
            $cwmin_ac3_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,10,2, "$cwmin_ac3_new";
        }
        elsif($arg[0] eq 'CWMIN_AC[4]')
        {
            $cwmin_ac4_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,12,2, "$cwmin_ac4_new";
        }
        elsif($arg[0] eq 'CWMAX_AC[0]')
        {
            $cwmax_ac0_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,14,2, "$cwmax_ac0_new";
        }
        elsif($arg[0] eq 'CWMAX_AC[1]')
        {
            $cwmax_ac1_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,16,2, "$cwmax_ac1_new";
        }       
        elsif($arg[0] eq 'CWMAX_AC[2]')
        {
            $cwmax_ac2_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,18,2, "$cwmax_ac2_new";
        }       
        elsif($arg[0] eq 'CWMAX_AC[3]')
        {
            $cwmax_ac3_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,20,2, "$cwmax_ac3_new";
        }       
        elsif($arg[0] eq 'CWMAX_AC[4]')
        {
            $cwmax_ac4_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,22,2, "$cwmax_ac4_new";
        }
        elsif($arg[0] eq 'TXOP_LIMIT[0]')
        {
            $txop_limit0_new=sprintf("%08x",$arg[1]);
            $new=substr $cw_values,24,8, "$txop_limit0_new";
        }
        elsif($arg[0] eq 'TXOP_LIMIT[1]')
        {
            $txop_limit1_new=sprintf("%08x",$arg[1]);
            $new=substr $cw_values,32,8, "$txop_limit1_new";
        }
        elsif($arg[0] eq 'TXOP_LIMIT[2]')
        {
            $txop_limit2_new=sprintf("%08x",$arg[1]);
            $new=substr $cw_values,40,8, "$txop_limit2_new";
        }
        elsif($arg[0] eq 'TXOP_LIMIT[3]')
        {
            $txop_limit3_new=sprintf("%08x",$arg[1]);
            $new=substr $cw_values,48,8, "$txop_limit3_new";
        }
        elsif($arg[0] eq 'TXOP_LIMIT[4]')
        {
            $txop_limit4_new=sprintf("%08x",$arg[1]);
            $new=substr $cw_values,56,8, "$txop_limit4_new";
        }
        elsif($arg[0] eq 'AIFS_VALUE[0]')
        {
            $aifs_value0_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,64,2, "$aifs_value0_new";
        }
        elsif($arg[0] eq 'AIFS_VALUE[1]')
        {
            $aifs_value1_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,66,2, "$aifs_value1_new";
        }
        elsif($arg[0] eq 'AIFS_VALUE[2]')
        {
            $aifs_value2_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,68,2, "$aifs_value2_new";
        }
        elsif($arg[0] eq 'AIFS_VALUE[3]')
        {
            $aifs_value3_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,70,2, "$aifs_value3_new";
        }
        elsif($arg[0] eq 'AIFS_VALUE[4]')
        {
            $aifs_value4_new=sprintf("%02x",$arg[1]);
            $new=substr $cw_values,72,2, "$aifs_value4_new";
        }
        elsif($arg[0] eq 'OP_MODE')
        {
            $op_new=sprintf("%02x",$arg[1]);
            $new=substr $modes,0,2, "$op_new";
        }
        elsif($arg[0] eq 'QOS_MODE')
        {
            $qos_new=sprintf("%02x",$arg[1]);
            $new=substr $modes,2,2, "$qos_new";
        }
        elsif($arg[0] eq 'RESERVED_ONE')
	{
	    $reserved_one_new=sprintf("%02x",$arg[1]) ;
	    $new=substr $reserved_one,0,2, "$reserved_one_new";
	}
        elsif($arg[0] eq 'BEACON_INTERVAL')
        {
            $bi_new=sprintf("%08x",$arg[1]);
            $new=substr $bi_value,0,8, "$bi_new";
        }
        elsif($arg[0] eq 'SIGNAL_EXTENSION')
        {
            $signal_new=sprintf("%02x",$arg[1]);
            $new=substr $values,0,2, "$signal_new";
        }
        elsif($arg[0] eq 'SELF_CTS')
        {
            $self_cts_new=sprintf("%02x",$arg[1]);
            $new=substr $values,2,2, "$self_cts_new";
        }
        elsif($arg[0] eq 'ACK_FAILURE')
        {
            $ack_fail_new=sprintf("%02x",$arg[1]);
            $new=substr $values,4,2, "$ack_fail_new";
        }
        elsif($arg[0] eq 'RTS_FAILURE')
        {
            $rts_fail_new=sprintf("%02x",$arg[1]);
            $new=substr $values,6,2, "$rts_fail_new";
        }
        elsif($arg[0] eq 'RTS_SUCCESS')
        {
            $rts_pass_new=sprintf("%02x",$arg[1]);
            $new=substr $values,8,2, "$rts_pass_new";
        }
        elsif($arg[0] eq 'FCS_ERROR')
        {
            $fcs_error_new=sprintf("%02x",$arg[1]);
            $new=substr $values,10,2, "$fcs_error_new";
        }
        elsif($arg[0] eq 'RX_FRAMES')
        {
            $rx_frames_new=sprintf("%02x",$arg[1]);
            $new=substr $values,12,2, "$rx_frames_new";
        }
        elsif($arg[0] eq 'RX_MULTICAST')
        {
            $rx_mcast_new=sprintf("%02x",$arg[1]);
            $new=substr $values,14,2, "$rx_mcast_new";
        }
        elsif($arg[0] eq 'RX_BROADCAST')
        {
            $rx_bcast_new=sprintf("%02x",$arg[1]);
            $new=substr $values,16,2, "$rx_bcast_new";
        }
        elsif($arg[0] eq 'RX_FRAME_US')
        {
            $rx_us_new=sprintf("%02x",$arg[1]);
            $new=substr $values,18,2, "$rx_us_new";
        }
        elsif($arg[0] eq 'RX_AMPDU_FRAMES')
        {
            $rx_ampdu_new=sprintf("%02x",$arg[1]);
            $new=substr $values,20,2, "$rx_ampdu_new";
        }
        elsif($arg[0] eq 'RX_AMPDU_SUBFRAMES')
        {
            $rx_subframes_new=sprintf("%02x",$arg[1]);
            $new=substr $values,22,2, "$rx_subframes_new";
        }
        elsif($arg[0] eq 'RX_PHY_ERROR')
        {
            $rx_phy_new=sprintf("%02x",$arg[1]);
            $new=substr $values,24,2, "$rx_phy_new";
        }
        elsif($arg[0] eq 'RESERVED')
        {
            $reser_new=sprintf("%06x",$arg[1]);
            $new=substr $values,26,6, "$reser_new";
        }
        elsif($arg[0] eq 'RTS_THRESHOLD')
        {
            $rts_new=sprintf("%04x",$arg[1]);
            $new=substr $threshold,0,4, "$rts_new";
        }
        elsif($arg[0] eq 'BSSBASICRATESET')
        {
            $bss_basic_new=sprintf("%04x",$arg[1]);
            $new=substr $bss,0,4, "$bss_basic_new";
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[0]')
        {
            $sta0_new=sprintf("%02x",$arg[1]);
            $new=substr $address,0,2, "$sta0_new";
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[1]')
        {
            $sta1_new=sprintf("%02x",$arg[1]);
            $new=substr $address,2,2, "$sta1_new";
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[2]')
        {
            $sta2_new=sprintf("%02x",$arg[1]);
            $new=substr $address,4,2, "$sta2_new";
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[3]')
        {
            $sta3_new=sprintf("%02x",$arg[1]);
            $new=substr $address,6,2, "$sta3_new";
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[4]')
        {
            $sta4_new=sprintf("%02x",$arg[1]);
            $new=substr $address,8,2, "$sta4_new";
        }
        elsif($arg[0] eq 'STA_MAC_ADDRESS[5]')
        {
            $sta5_new=sprintf("%02x",$arg[1]);
            $new=substr $address,10,2, "$sta5_new";
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[0]')
        {
            $bss0_new=sprintf("%02x",$arg[1]);
            $new=substr $bssid,0,2, "$bss0_new";
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[1]')
        {
            $bss1_new=sprintf("%02x",$arg[1]);
            $new=substr $bssid,2,2, "$bss1_new";
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[2]')
        {
            $bss2_new=sprintf("%02x",$arg[1]);
            $new=substr $bssid,4,2, "$bss2_new";
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[3]')
        {
            $bss3_new=sprintf("%02x",$arg[1]);
            $new=substr $bssid,6,2, "$bss3_new";
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[4]')
        {
            $bss4_new=sprintf("%02x",$arg[1]);
            $new=substr $bssid,8,2, "$bss4_new";
        }
        elsif($arg[0] eq 'STA_BSSID_ADDRESS[5]')
        {
            $bss5_new=sprintf("%02x",$arg[1]);
            $new=substr $bssid,10,2, "$bss5_new";
        }
        elsif($arg[0] eq 'RESERVED6')
        {
            $reser6_new=sprintf("%04x",$arg[1]);
            $new=substr $bssid,12,4, "$reser6_new";
        }
=head
        elsif($arg[0] eq 'TSF')
        {
            $tsf_new=sprintf("%016x",$arg[1]);
            $new=substr $tsf,0,16, "$tsf_new";
        }
        elsif($arg[0] eq 'TSF_UNLOCK')
        {
            $tsf_unlock_new=sprintf("%02x",$arg[1]);
            $new=substr $ibss_reg,0,2, "$tsf_unlock_new";
        }
=cut
        elsif($arg[0] eq 'LAST_BEACON_TX')
        {
            $last_beacon_new=sprintf("%02x",$arg[1]);
            $new=substr $ibss_reg,0,2, "$last_beacon_new";
        }
        elsif($arg[0] eq 'PHY_TX_DELAY')
        {
            $phy_tx_delay_new=sprintf("%02x",$arg[1]);
            $new=substr $ibss_reg,2,2, "$phy_tx_delay_new";
        }
=head
        elsif($arg[0] eq 'IBSS_SEND_BEACON')
        {
            $send_beacon_new=sprintf("%02x",$arg[1]);
            $new=substr $ibss_reg,6,2, "$send_beacon_new";
        }
=cut
        elsif($arg[0] eq 'PS_MODE')
        {
            $ps_mode_new=sprintf("%02x",$arg[1]);
            $new=substr $ps,0,2, "$ps_mode_new";
        }
        elsif($arg[0] eq 'UAPSD_MODE')
        {
            $uapsd_mode_new=sprintf("%02x",$arg[1]);
            $new=substr $ps,2,2, "$uapsd_mode_new";
        }
        elsif($arg[0] eq 'MULTIDATA')
        {
            $multi_new = sprintf("%02x",$arg[1]);
            $new =substr $multi,0,2, "$multi_new";

        }

    }
    @array=unpack("(A2)*",$threshold);
    $threshold=$array[1].$array[0];

    @array=unpack("(A2)*",$tsf);
    $tsf=$array[7].$array[6].$array[5].$array[4].$array[3].$array[2].$array[1].$array[0];

    @array=unpack("(A2)*",$bi_value);
    $bi_value=$array[3].$array[2].$array[1].$array[0];
    #$registers=$timers.$cw_values.$modes.$bi_value.$values.$threshold.$bss.$address.$bssid.$tsf.$ibss_reg.$ps.$multi; 

    $registers=$timers.$cw_values.$modes.$reserved_one.$bi_value.$values.$threshold.$bss.$address.$bssid.$ibss_reg.$ps.$multi; 
}
1 # Indicates end of the file.

#!/usr/bin/perl 

use IO::Socket::INET;
use IO::Socket::Netlink; # Used for creating Netlink Socket.

$cmd_type = 00;
$SEND_CMD = 1;
$RECV_CMD = 3;
$delimiter = "!";
$num_of_args = $#ARGV + 1;

if ($num_of_args < 1) 
{
  LOG("\nUsage: filename.pl testcasesfile.txt \n");
  exit;
}

$config_protocol = 1; # 0 for UDP socket and 1 for Netlink socket.
$local_addr = $ARGV[1] || "192.168.2.246";
$local_port = $ARGV[2] || "8000";
$peer_addr = $ARGV[3] || "192.168.2.246";
$peer_port = $ARGV[4] || "9000";
$prot = $ARGV[5] || "udp";
my $nl_sock;

sub nl_sock_open
{
	my($protocol,$groups)=@_;
	$nl_sock = IO::Socket::Netlink->new( Protocol => $protocol , Groups => $groups) or warn "socket: $!";
	return $nl_sock;
}
if($config_protocol == 0)
{ 
	socket_open($local_addr,$local_port,$peer_addr,$peer_port,$prot);
}
elsif($config_protocol == 1)
{
	nl_sock_open("24","99");
}


our $file_name_main=$ARGV[0];
open(FILE,$file_name_main);
local $/;
my $file_content_main=<FILE>;
our $counter='0';
our $count_data='0';
our $beacon_count='0'; 
our $data_ack_count='0';
events_handling($file_content_main);

$testcase_started = 0;
$casefailed = 0;
$totaltestcases=0;
$testcasesfailed=0;

sub events_handling
{
    my $event_data=shift;
    @content=split(/\n/,$event_data);
    foreach  $individual_content(@content)
    {
	@argument=split(" ",$individual_content);
	if ($casefailed == 1)
	{	
		if (!(($argument[0] eq 'End') || ($argument[0] eq 'Endcase')))
		{
			next;
		}
		$testcasesfailed++;
		our @fail_testcases_list=();
		push(@fail_testcases_list,$num);
	
	}


	if($argument[0] eq 'send')
	{
		send_operation();
	}
	elsif($argument[0] eq 'recv')
	{
		recv_operation();
	}
	elsif($argument[0] eq 'Testcase')
	{
		testcase_operation();
	}
	elsif($argument[0] eq 'Endcase')
	{
		endcase_operation();
	}
	elsif($argument[0] eq 'END')
	{
		end_operation();
	}	
	elsif($argument[0] eq 'RECV_CMD')
        {
        	recv_phy_ind();
        }
	elsif($argument[0] eq 'GENERATE')
	{
		generate_frames();
	}
	elsif($argument[0] eq 'delay')
	{
		delay_operation();
	}

    }
} 

sub generate_frames
{
	if($argument[1] eq 'BEACON')
	{
		generate_beacon();	
	}
	elsif($argument[1] eq 'RESPONSE')
        {
                response();
        }
	elsif($argument[1] eq 'RECV_RESPONSE')
	{
		enter_recv_state();	
	}
	elsif($argument[1] eq 'PROBEREQUEST')
        {
                generate_proberequest();
        }
	
} 

sub generate_beacon
{
	$filename='../sample_frames/uu_wlan_generate_beacon.txt';
	frame_parse($filename);
}

sub frame_parse
{
	$file_name=shift;
	open(FILE,$file_name);
        $file_content=<FILE>;
        $configname='config.txt';
        open(FILE,$configname);
        local $/; 
        $config_data=<FILE>;
        @conf=split("\n",$config_data);
        $DURATION=$conf[0];
        $DAB=$conf[1]; # for broadcast
        $DA=$conf[2]; # for other destinations
        $SA=$conf[3]; # mac address determined usinf ifconfig(HWADDR)
        $BSSID=$conf[4];
        $SEQNO=$conf[5]; # new seq number(444)
        $DATA=$conf[6];
        $file_content=~s/DAB/$DAB/;
        $file_content=~s/DA/$DA/;
        $file_content=~s/SA/$SA/;
        $file_content=~s/BSSID/$SA/; ### same as source address for broadcast
        $file_content=~s/SEQNO/$SEQNO/;
        $file_content=~s/DURATION/$DURATION/;
        $file_content=~s/DATA/$DATA/;
	crc($file_content);

}

# CRC calculation
sub crc
{
	$conte=shift;
        $conte=~s/CRC//;
	$conte=~s/\s+//g; # TO remove spaces in the content
        my $crc=crc_calculation($conte);
	$checkval=sprintf("%08x\n",$crc);
	@reord=split(//,$checkval);
	$new=$reord[6].$reord[7].$reord[4].$reord[5].$reord[2].$reord[3].$reord[0].$reord[1];
	$conte=~s/\n//g;
	$conte=$conte.$new;
	send_msg($conte);
	if($num eq '001') # '001' for testing beacon frame
	{
		enter_endcase();
	}
}

sub crc_clear
{
	$crc = 0xFFFFFFFF;

}
sub build_table
{
        my $table = [];
        for(my $n = 0; $n < 256; $n++)
        {
        	my $c = $n;
            	for( my $k = 0; $k < 8; $k++ )
            	{
                	$c = ($c >> 1 ) ^ (($c & 1) ? 0xEDB88320 : 0);
            	}
            	$table->[$n] = $c;
        }
        return $table;
}

sub crc_calculation
{
        my($data_ptr) = shift;
        our $crc = 0xFFFFFFFF;
        our $crc_table;
        crc_clear();
        if(!defined $crc_table)
        {
        	$crc_table = build_table();
        }
        my @groups = unpack "(a2)*", $data_ptr;
        foreach $byte(@groups)
        {
            	$byte=hex($byte);
            	$crc = $crc_table->[($crc ^ $byte) & 0xFF] ^ ($crc >> 8);
        }
        return ($crc ^ 0xFFFFFFFF);

} # CRC calculation

sub send_msg
{
        $data=shift;
	$len=length($data);
	LOG("sending message is: $data and Length is: $len\n");	# To perform send operation via sockets you need to create a server socket
	if($config_protocol == 0)
	{
        	#$socket->send($data);
		#send via udp socket
	}
	elsif($config_protocol == 1)
	{
		nlmsg_send("24","99",$data); # '24' protocol id and '99' group id
	}
}

sub parse_msg
{
	$file_name=shift;
	local $/ = undef;
        open(FILE,$file_name) or die $!;
        $message=<FILE>;
        my  @data =split /\+---------\+---------------\+----------\+/,$message;

        foreach $content(@data)
        {
        	$content=~ s/.*//;
               	$content=~ s/.* .*//;
               	$content=~ s/\|....//;
               	$content=~ s/\|//g;
               	$data=$content;
               	send_msg($data);
       	}
}

sub read_phy_content
{
	my($file_name,$phy_ind)=@_;
	local $/;
        open(FILE,$file_name);
        $mess=<FILE>;
 	$data= $phy_ind.$mess;
	$data=~s/\n$//;	#To remove last new line of the $data
        send_msg($data);

}

sub read_content
{
	$fname=shift;
	local $/;
        open(FILE,$fname);
        $mess=<FILE>;
 	$data=$mess;
        send_msg($data);

}

sub send_operation
{
	if($argument[1] eq '-C')
        {
        	parse_msg($argument[2]);
        } 
        elsif($argument[1] eq '-T')
    	{ 
        	read_content($argument[2]);
        }
	elsif(($argument[1] =~/[a-z]/) || ( $argument[1] =~ /[A-Z]/) || ( $argument[1] =~ /[0-9]/))
	{
		$data= $SEND_CMD.$argument[1];
		send_msg($data);
	}
	else
        {
		send_msg();
        }
}

sub recv_phy_ind
{
	if($argument[1] eq 'UU_PHY_DATA_REQ')
	{
		$phy_cmd = $UU_PHY_DATA_REQ;
		$par_data=$argument[2];
		@parsedata=split("$delimiter",$par_data);
		if($phy_cmd eq $parsedata[0])
		{
			$data=$parsedata[1];
			recv_msg($data);
		}
		else
		{
			LOG("The RECV_CMD indication and the indication received are different");
			exit;
		}
	}
	elsif($argument[1] eq 'UU_PHY_TXSTART_REQ')
	{
		$phy_cmd = $UU_PHY_TXSTART_REQ;
		$par_data=$argument[2];
		@parsedata=split("$delimiter",$par_data);
		if($phy_cmd eq $parsedata[0])
		{
			$data=$parsedata[1];
			recv_msg($data);
		}
		else
		{
			LOG("The RECV_CMD indication and the indication received are different");
			exit;
		}
	}
	elsif($argument[1] eq 'UU_PHY_TXEND_REQ')
	{
		$phy_cmd = $UU_PHY_TXEND_REQ;
		recv_msg();
	}
	elsif($argument[1] eq 'UU_PHY_CCARESET_REQ')
	{
		$phy_cmd = $UU_PHY_CCARESET_REQ;
		recv_msg();
	}
}

sub recv_operation
{
	if($argument[1] eq '-C')
	{
		parse_msg_recv($argument[2]);
	}
	elsif($argument[1] eq '-T')
	{
		read_content_recv($argument[2]);
	}
	elsif($argument[1] =~ /[a-z]/)
	{
		$data=$argument[1];
		recv_msg($data);
	}
	elsif($argument[1] =~ /[A-Z]/)
	{
		$data=$argument[1];
		recv_msg($data);
	}
	elsif($argument[1] =~ /[0-9]/)
	{
		$data=$argument[1];
		recv_msg($data);
	}
	else
	{
		recv_msg();
	}
}

sub recv_msg
{			
	my $recv_data=shift;
	if($config_protocol == 0)
	{
		#$client->recv($recv_data,1024);	
		#Recv via udp socket
	}
	elsif($config_protocol == 1)
	{
		eval
		{	
			$SIG{ALRM}=sub{ print "timeout reached, after 20 seconds!\n";$casefailed=1; };
			alarm 20;
                        $nl_sock->recv_nlmsg($recv_data,65536) or die "recv: $!";
			printf "Received type=%d :\nRECV MSG is=%v02x\n",$recv_data->nlmsg_type, $recv_data->nlmsg;
			our $result=sprintf( "%v02x",$recv_data->nlmsg);
                        $test=substr($result,0,2);
                        our $content_msg=$result;
			alarm 0;
                };
	}
}

sub response
{
        $message=$content_msg;
        $typ_sbtype=substr($message,0,2);
	print "$typ_sbtype\n";
        type_subtype($typ_sbtype);
}

sub type_subtype
{
	$bits=shift;
	if($bits eq '00') 	#association request
        {
		gen_association_resp();
        }
        elsif($bits eq '10')	#Association Response frame
        {
                gen_data();
        }
        elsif($bits eq '20') 	#reassociation request
        {
        	gen_reassociation_resp();
        }
        elsif($bits eq '30') 	#reassociation response
        {
        	gen_data();
        }
        elsif($bits eq '40')	#probe request
        {
                generate_proberesponse();
        }
        elsif($bits eq '50') 	#probe response
        {
        	gen_authentication_request();
        }
        elsif($bits eq '80')	#Beacon frame
        {
    	    LOG("\nBEACON FRAME and count is:$beacon_count\n");
	    if ($beacon_count ge '1')
            {
			$beacon_count++;
			recv_msg();
    	    }
	    else
	    {
			$beacon_count++;
		        gen_authentication_request();
	    }
        }
        elsif($bits eq 'a0')	#Disassociation frame
        {
                recv_msg();
        }
        elsif($bits eq 'b0')	#Authentication frame
        {
                print "Authentication request frame received\n";
		gen_authentication_response();
        }
        elsif(($bits eq 'c0') && ($num eq '005' || $num eq '010'))	#Deauthentication frame. for testing based on testcase is
        {
		enter_endcase();
        }
        elsif($bits eq 'c0')	#Deauthentication frame
        {
		recv_msg();
        }
        elsif($bits eq 'd0')	#Action frame
        {
	        LOG("\nACTION FRAME\n");
	}
	elsif($bits eq 'e0')	#Action with no ack
        {
                LOG("\nACTION WITH NO ACK\n");
        }
        elsif($bits eq '84')	#BAR frame
        {
                gen_ba();
        }
        elsif($bits eq '94')	#BA frame
        {
                gen_data();
        }
        elsif($bits eq 'a4')	#PS-POLL
        {
                LOG("\n PS-POLL FRAME\n");
        }
        elsif($bits eq 'b4')	#RTS frame
        {
                gen_cts();
        }
        elsif($bits eq 'c4')	#CTS frame
        {
                LOG("\n CTS FRAME\n");
		gen_data();
        }
        elsif($bits eq 'd4')	#Ack frame
        {
                print "Acknowledgment frame received\n";
		if($data_ack_count eq '1')
		{
        		open(FILE,"endcase.txt");
	        	$file=<FILE>;
	        	events_handling($file);
		}
		else
		{
			recv_msg();
		}
        }
        elsif($bits eq 'e4')	#CF-END
        {
                LOG("\nCF-END\n");
        }
        elsif($bits eq 'f4')	#CF_END+CF-ACK
        {
                LOG("\n CF-END + CF-ACK\n");
        }
        elsif(($bits eq '08') && ($num eq '004' || $num eq '009'))	#Data frame. for testing
        {
		enter_endcase();
	}
        elsif($bits eq '08')	#Data frame
        {
		gen_data();
        }
        elsif($bits eq '18')	#Data+CF-ACK
        {
                recv_msg();
                gen_data();
        }
        elsif($bits eq '28')	#Data+CF-POLL
        {
                LOG("\n Data +CF-POLL frame\n");
                recv_msg();
        }
        elsif($bits eq '38')	#Data+CF-ACK+CF-POLL
        {
                LOG("\n Data+CF-ACK+CF-POLL frame\n");
                recv_msg();
        } 
	elsif($bits eq '48')	#Null(NO Data) 
        {
                print "\nNULL FUNCTION\n";
                recv_msg();
        }
        elsif($bits eq '78')	#Cf-ack+cf-poll
        {
                LOG("\n CF-ACK +CF-POLL frame\n");
                recv_msg();
        }
        elsif($bits eq '88')	#QoS Data
        {
                LOG("\n QoSData frame\n");
                recv_msg();
        }
        elsif($bits eq '98')	#QoS Data+CF-Ack
        {
                LOG("\n QoSData +CF-ACK frame\n");
                recv_msg();
        }
        elsif($bits eq 'a8')	#QoS Data+Cf-poll
        {
                LOG("\n QosData +CF-POLL frame\n");
                recv_msg();
        }
        elsif($bits eq 'e8')	#QoS Cf-poll
        {
     		LOG("\nQOS CF-POLL\n");
                recv_msg();
        }
        elsif($bits eq 'f8')	#QoS cf-ack+cf-poll
        {
                LOG("\n Data +CF-POLL frame\n");
                recv_msg();
        }
        else
        {
                LOG("Received from phy_Stub with unknown type/subtype \n");
                $casefailed=1;
        }

}

sub enter_recv_state
{
	open(FILE,"response.txt");
	$tx_data=<FILE>;
	local $/;
	events_handling($tx_data);
}

sub enter_endcase
{
        open(FILE,"endcase.txt");
        $file=<FILE>;
        events_handling($file);
}

sub seq_num_gen
{
	$message=shift;	
	$sequence_number=substr($message,44,4);
	@seq=split("",$sequence_number);
	$reorder=$seq[2].$seq[3].$seq[0];
	$hexval = hex($reorder);
	$hexval+=1;
	if($hexval <= 9)
	{
        	$decval=sprintf("%03d",$hexval);
        	LOG("$decval\n");
	}
	else
	{
        	$decval=sprintf("%x",$hexval);
        	LOG("$decval\n");
	}

	$fragment_num=$seq[1];
	@split_order=split("",$decval);
	$new_order=$split_order[2].$fragment_num.$split_order[0].$split_order[1];
	$message=~s/$sequence_number/$new_order/;
	LOG("$message\n");
	fcs($message);
}

sub fcs
{
	$content=shift;
	chop($content);
	$prevcrc=substr($content,-8,8);
	$content=~s/$prevcrc//;
	my $crc=crc_calculation($content);
	$fcs=sprintf("%08x\n",$crc);
	@reord=split(//,$fcs);
        $new=$reord[6].$reord[7].$reord[4].$reord[5].$reord[2].$reord[3].$reord[0].$reord[1];
	$content=$content.$new;
	LOG("$content\n");
	send_msg($content);
}

sub generate_proberequest
{
	$file_name="../sample_frames/uu_wlan_management_proberequest.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	send_msg($file_data);
}

sub generate_proberesponse
{
	$file_name="../sample_frames/uu_wlan_proberesponse_dest_04.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	send_msg($file_data);
	if($num eq '006')
	{
		enter_endcase();
	}
	else
	{
		enter_recv_state();
	}
}	

sub gen_authentication_request
{
	$file_name="../sample_frames/uu_wlan_authreq.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	send_msg($file_data);
}

sub gen_authentication_response
{
	$file_name="../sample_frames/uu_wlan_authresp.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	send_msg($file_data);
	if($num eq '002' || $num eq '007')
	{
		enter_endcase();
	}
	else
	{
		enter_recv_state();
	}
}

sub gen_association_request
{
	$file_name="../sample_frames/uu_wlan_management_associationrequest.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	send_msg($file_data);
}

sub gen_data
{
	$file_name="../sample_frames/uu_wlan_data_data.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	send_msg($file_data);
	if( $num eq '011')
	{
		enter_endcase();
	}
}

sub gen_association_resp
{
        $fname='../sample_frames/uu_wlan_generate_reassociationresponse.txt';
        open(FILE,$fname);
        $data=<FILE>;
	send_msg($data);
	if($num eq '003'|| $num eq '008')
	{
		enter_endcase();
	}
	else
	{
		enter_recv_state();
	}
}

sub gen_reassociation_resp
{
        $fname='../sample_frames/uu_wlan_generate_reassociationresponse.txt';
        open(FILE,$fname);
        $data=<FILE>;
	seq_num_gen($data);
}

sub gen_probe_resp
{
        $fname='../sample_frames/uu_wlan_management_proberesponse.txt';
        open(FILE,$fname);
        $data=<FILE>;
	seq_num_gen($data);
}

sub parse_msg_recv
{
        $fname=shift;
        local $/ = undef;
        open(FILE,$fname) or die $!;
        $mess=<FILE>;
        my  @data =split /\+---------\+---------------\+----------\+/,$mess;

        foreach $content(@data)
        {
        	$content=~ s/.*//;
                $content=~ s/.* .*//;
                $content=~ s/\|....//;
                $content=~ s/\|//g;
                $data=$content;
                recv_msg($data);
        }
}

sub read_content_recv
{
        $fname=shift;
        local $/;
        open(FILE,$fname);
        $mess=<FILE>;
        $data=$mess;
        recv_msg($data);
}

sub testcase_operation
{	
	if($argument[1]=~/\d+$/)
	{
		our $num=$argument[1];
	}
  	LOG("TestCase $num \n");
}

sub endcase_operation
{
	$testcase_started = 0;
	$casefailed = 0;
	if($casefailed == 0)
	{
		$totaltestcases++;
	}
	$testcasespassed=$totaltestcases-$testcasesfailed;
        LOG("EndCase $num \n");
}

sub socket_open
{
	my ($localaddr,$localport,$peeraddr,$peerport,$proto)=@_;
	my $socket = new IO::Socket::INET(LocalAddr => $localaddr,
        LocalPort =>  $localport,

        PeerAddr =>  $peeraddr,
        PeerPort => $peerport,

        Proto     => $proto,)
 				or warn "can't create server socket:$!";
		
	return $socket;
}

sub delay_operation
{
        if($argument[1] =~/\d+$/)
        {
                $time=$argument[1];
                sleep($time);
                LOG("Delay operation \n");
        }
        else
        {
                $casefailed = 1;

        }
}

sub nlmsg_send
{
	my($nlmsg_type,$nlmsg_pid,$nlmsg)=@_;
	$nl_sock->send_nlmsg($nl_sock->new_request(
    		nlmsg_type  => $nlmsg_type,
    		nlmsg_pid => $nlmsg_pid,
	    	nlmsg       => $nlmsg,
 	) ) or warn "send: $!";

}
	
sub end_operation
{
	LOG( "\n\tTestcase Summary\n\tTotal Testcases:$totaltestcases\n\tTestcases Passed:$testcasespassed\n\tTestcases Failed:$testcasesfailed\n\tFailed Testcases are: @fail_testcases_list\nEND operation\n");
        exit(1);
}

sub date_time
{
        @months = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
        @weekDays = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
        ($second, $minute, $hour, $dayOfMonth, $month, $yearOffset, $dayOfWeek, $dayOfYear, $daylightSavings) = localtime();
        $year = 1900 + $yearOffset;
        $theTime = "$hour:$minute:$second, $weekDays[$dayOfWeek] $months[$month] $dayOfMonth, $year";
}

sub LOG
{
        $msg=shift;
        local $/=undef;
        open (FILE,'>>testlogfile.log');
        print "".date_time()." $msg";
        print FILE "".date_time().$msg;
        close(FILE);
}


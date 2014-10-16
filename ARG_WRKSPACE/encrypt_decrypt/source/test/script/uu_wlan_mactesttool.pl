#!/usr/bin/perl 

use IO::Socket::INET;
use IO::Socket::Netlink; # Used for creating Netlink Socket.

my($UU_PHY_INVALID_EVENT, $UU_PHY_DATA_REQ, $UU_PHY_DATA_CONF, $UU_PHY_DATA_IND, $UU_PHY_TXSTART_REQ, $UU_PHY_TXSTART_CONF, $UU_PHY_TXEND_REQ, $UU_PHY_TXEND_CONF, $UU_PHY_RXSTART_IND, $UU_PHY_RXEND_IND, $UU_PHY_CCARESET_REQ, $UU_PHY_CCARESET_CONF, $UU_PHY_CCA_IND) = (-01..11);

$cmd_type = 00;
$SEND_CMD = 1;
$PHY_CMD = 2;
$RECV_CMD = 3;
$delimiter = "!";
$num_of_args = $#ARGV + 1;

if ($num_of_args < 1) {
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
	nl_sock_open("25","99");
}


our $file_name_main=$ARGV[0];
open(FILE,$file_name_main);
local $/;
my $file_content_main=<FILE>;
our $counter='0';
our $beacon_count='0'; 
our $data_ack_count='0';
our $probe_counter='0';
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
	elsif($argument[0] eq 'Update') 
	{
		update_operation();
	}
	elsif($argument[0] eq 'Readframe')
	{
		readframe_operation();
	}
	elsif($argument[0] eq 'delay')
	{
		delay_operation();
	}
	elsif($argument[0] eq 'END')
	{
		end_operation();
	}	
	elsif($argument[0] eq 'PHY-IND')
	{
		send_phy_ind();
	}
	elsif($argument[0] eq 'RECV_CMD')
        {
        	recv_phy_ind();
        }
	elsif($argument[0] eq 'GENERATE')
	{
		generate_frames();
	}
    }
}

sub generate_frames
{
	if($argument[1] eq 'BEACON')
	{
		generate_beacon();	
	}
	elsif($argument[1] eq 'AUTHENTICATION')
	{
		generate_authentication();
	}
	elsif($argument[1] eq 'ASSOCIATIONREQUEST')
	{
		generate_associationrequest();
	}
	elsif($argument[1] eq 'REASSOCIATIONREQUEST')
        {
                generate_reassociationrequest();
        }
	elsif($argument[1] eq 'PROBEREQUEST')
        {
                generate_proberequest();
        }
	elsif($argument[1] eq 'DEAUTHENTICATION')
        {
                generate_deauthentication();
        }
	elsif($argument[1] eq 'DISASSOCIATE')
        {
                generate_disassociate();
        }
	elsif($argument[1] eq 'ASSOCIATIONRESPONSE')
	{
		generate_associationresponse();
	}
	elsif($argument[1] eq 'REASSOCIATIONRESPONSE')
	{
		generate_reassociationresponse();
	}
	elsif($argument[1] eq 'PROBERESPONSE')
	{
		generate_proberesponse();
	}
	elsif($argument[1] eq 'RESPONSE')
        {
                response();
        }
	elsif($argument[1] eq 'RECV_RESPONSE')
        {
                enter_recv_state();
        }

}

sub generate_associationresponse
{
	$filename='../sample_frames/uu_wlan_generate_associationresponse.txt';
	frame_parse($filename);
}

sub generate_reassociationresponse
{
        $filename='../sample_frames/uu_wlan_generate_reassociationresponse.txt';
        frame_parse($filename);
}

sub generate_proberesponse
{
        $filename='../sample_frames/uu_wlan_generate_proberesponse.txt';
        frame_parse($filename);
}

sub generate_authentication
{
	$filename='../sample_frames/uu_wlan_generate_authentication.txt';
	frame_parse($filename);	
}

sub generate_disassociate
{
        $filename='../sample_frames/uu_wlan_generate_disassociate.txt';
        frame_parse($filename);
}

sub generate_proberequest
{
        $filename='../sample_frames/uu_wlan_generate_proberequest.txt';
        frame_parse($filename);
}

sub generate_associationrequest
{
        $filename='../sample_frames/uu_wlan_generate_associationrequest.txt';
        frame_parse($filename);
}

sub generate_reassociationrequest
{
        $filename='../sample_frames/uu_wlan_generate_reassociationrequest.txt';
        frame_parse($filename);
}

sub generate_deauthentication
{
        $filename='../sample_frames/uu_wlan_generate_deauthentication.txt';
        frame_parse($filename);
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
        $configname='./config-files/uu_wlan_config.txt';
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
        $file_content=~s/BSSID/$DA/; ### same as source address for broadcast
        $file_content=~s/SEQNO/$SEQNO/;
        $file_content=~s/DURATION/$DURATION/;
        $file_content=~s/DATA/$DATA/;
        #LOG($file_content);
	crc($file_content);

}

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
	start_recv_proc($conte);
}

sub start_recv_proc
{
	$conte=shift;
	$length=(length($conte))/2;
	$hex_length=sprintf("%02x",$length);
	open(FILE,"./config-files/uu_wlan_rxvector_format.txt");
	$filecont=<FILE>;
	$filecont=~s/aa/$hex_length/;
	$filecont=~s/\n//g;
	open(FILE,'>./config-files/uu_wlan_rxvector_generatedframe.txt');
	print FILE "$filecont";
	open(FILE,'>./config-files/uu_wlan_generatedframe.txt');
        print FILE "$conte";
	open(FILE,"./config-files/uu_wlan_rxprimitives.txt");
	local $/;
	$rx_file_data=<FILE>;
	local $/;
	$rx_file_data=~s/rxvector_gen.txt/uu_wlan_rxvector_generatedframe.txt/;
	$rx_file_data=~s/gen.txt/uu_wlan_generatedframe.txt/;
	open(FILE,'>./config-files/uu_wlan_sending_state.txt');
	print FILE "$rx_file_data";
	events_handling($rx_file_data);
	print "TYPE SUBTYPE IS:$typ_sbtype\n";
	print "counter is:$counter\n";
	
	if(($typ_sbtype eq 'b0') && ($num eq '002' || $num eq '007'))
        {
                enter_endcase();
        }
	elsif($typ_sbtype eq 'b0' && $counter eq '0') 
    	{
		$counter="1";
		gen_association_request();
    	}
	elsif($typ_sbtype eq '50' && $probe_counter eq '0')
        {
		if($num eq '006')
		{
			enter_endcase();
		}
		else
		{
			$probe_counter++;
			gen_authentication_request();
    		}
	}
	elsif($typ_subtype eq '50' && $probe_counter gt '0')
	{
		enter_recv_state();
	} 
	elsif($typ_sbtype eq 'b0' && $counter eq '2' )
        {
                enter_endcase();
        }
	elsif(($typ_sbtype eq 'b0') && ($num eq '005' || $num eq '010'))
        {
                $counter="2";
                gen_deauthentication();
        }
	elsif($typ_sbtype eq 'b0' && $counter eq '1')
	{
               	 print "its here\n";
		$counter="2";
		enter_recv_state();
	}
	elsif(($typ_sbtype eq '10') && ($num eq '003'|| $num eq '008'))
        {
                enter_endcase();
        }
    	elsif($typ_sbtype eq '10' && $data_ack_count eq '0')
    	{
		$data_ack_count++;
		gen_data();
		enter_recv_state();
    	}	
	elsif($typ_sbtype eq '08' && $counter eq '2')
        {
                enter_endcase();
        }
	elsif($typ_sbtype eq '08')
        {
                $counter++;
		gen_ack_frame();
        }
	else
	{
        	enter_recv_state();
	}

}

# CRC calculation
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
		nlmsg_send("25","99",$data);
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

sub parse_phy_data
{
	my($file_name,$phy_ind)=@_;
	local $/;
	chomp($phy_ind);
        open(FILE,$file_name) or die $!;
        $mess=<FILE>;
        my  @data =split /\+---------\+---------------\+----------\+/,$mess;

        foreach $content(@data)
        {
        	$content=~ s/.*//;
               	$content=~ s/.* .*//;
               	$content=~ s/\|....//;
               	$content=~ s/\|//g;
		my $n = 2;    # $n is group size.
		my @groups = unpack "a$n" x (length( $content ) /$n ), $content;
		$bytes=join("",@groups);
		$bytes=~ s/^\n//; # To remove first new line of the $bytes [These operations are done in order to concatinate with 
		$bytes=~ s/^\n//; #To remove second new line of the $bytes    $phy_ind in the same line ]
               	$bytes=~ s/^\s+//; #To remove space at the end of the $bytes.
		$data=$phy_ind . $bytes;
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

sub send_phy_ind
{
	if($argument[1] eq 'UU_PHY_INVALID_EVENT')
	{
		$phy_cmd = $UU_PHY_INVALID_EVENT;
	}
	elsif($argument[1] eq 'UU_PHY_DATA_REQ')
	{
		 LOG("Invalid request and should be from mac to phy \n");
                 $casefailed = 1 ;
	}
	elsif($argument[1] eq 'UU_PHY_DATA_CONF')
	{
		$phy_cmd = $UU_PHY_DATA_CONF;
	}
	elsif($argument[1] eq 'UU_PHY_DATA_IND')
	{
		$phy_cmd = $UU_PHY_DATA_IND;
	}
	elsif($argument[1] eq 'UU_PHY_TXSTART_REQ')
	{
		 LOG("Invalid request and should be from mac to phy \n");
	         $casefailed = 1 ;
	}
	elsif($argument[1] eq 'UU_PHY_TXSTART_CONF')
	{
		$phy_cmd = $UU_PHY_TXSTART_CONF;
	}
	elsif($argument[1] eq 'UU_PHY_TXEND_REQ')
	{
		 LOG("Invalid request and should be from mac to phy \n");
                 $casefailed = 1 ;
	}
	elsif($argument[1] eq 'UU_PHY_TXEND_CONF')
	{
		$phy_cmd = $UU_PHY_TXEND_CONF;
	}
	elsif($argument[1] eq 'UU_PHY_RXSTART_IND')
	{
		$phy_cmd = $UU_PHY_RXSTART_IND;
	}
	elsif($argument[1] eq 'UU_PHY_RXEND_IND')
	{
		$phy_cmd = $UU_PHY_RXEND_IND;
	}
	elsif($argument[1] eq 'UU_PHY_CCARESET_REQ')
	{
		 LOG("Invalid requeuest and should be from mac to phy \n");
                 $casefailed = 1 ;
	}
	elsif($argument[1] eq 'UU_PHY_CCARESET_CONF')
	{
		$phy_cmd = $UU_PHY_CCARESET_CONF;
	}
	elsif($argument[1] eq 'UU_PHY_CCA_IND')
	{
		$phy_cmd = $UU_PHY_CCA_IND;
	}
	else
	{
		$phy_cmd = $UU_PHY_INVALID_EVENT;
	}
	LOG("PHY IND CMD is :$phy_cmd \n");

	our $msg = $PHY_CMD.$phy_cmd.$delimiter;

	if($argument[2] eq '-PF') # for Parse file
	{
		parse_phy_data($argument[3], $msg);
	}
	elsif($argument[2] eq '-TF') # for Text file
	{
		read_phy_content($argument[3], $msg);
	}
	elsif($argument[2] eq '-S')
	{
		$state = $argument[3];
		$msg = $msg.$state;
		send_msg($msg);
	}
	elsif(($argument[2] =~/[a-z]/) || ($argument[2] =~/[A-Z]/) || ($argument[2] =~/[0-9]/))
	{	
		$data = $argument[2];
		$msg = $msg.$data;
		send_msg($msg);	
	}	
	else
	{
		send_msg($msg);
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
                        if($test == 30) # 30 represents received starting bytes of data in UU_PHY_DATA_REQ
                        {
                        our $content_msg=$result;
                        }
			alarm 0;

                };

	}
}

sub response
{
        $message=$content_msg;
        $message=~ s/\.//g;
        $message=~s/....//;
        $typ_sbtype=substr($message,0,2);
	print "$typ_sbtype\n";
        type_subtype($typ_sbtype);

}

sub type_subtype
{
	$bits=shift;
	if($bits eq '00') 	#association request
        {
		enter_endcase(); #only for testing need to be removed.
		#gen_association_resp();
        }
        elsif($bits eq '10')	#Association Response frame
        {
                gen_ack_frame($message);
        }
        elsif($bits eq '20') 	#reassociation request
        {
        	gen_reassociation_resp();
        }
        elsif($bits eq '40')	#probe request
        {
                gen_probe_resp();
        }
        elsif($bits eq '50')	#probe response
        {
                gen_ack_frame($message);
        }
	elsif($bits eq '80' && $num eq '100') # for testing probe request
	{
		gen_probe_req();
	}
	elsif($typ_sbtype eq '80' && $num eq '001' )
        {
                enter_endcase();
        }
        elsif($bits eq '80')	#Beacon frame
        {
    	    	LOG("\nBEACON FRAME and count is:$beacon_count\n");
	    	if ($beacon_count ge '1')
	    	{
			$beacon_count++;
			enter_endcase();
		}
		else
		{
			$beacon_count++;
			 gen_authentication_request();
		}
        }
        elsif($bits eq 'a0')	#Disassociation frame
        {
                gen_ack_frame($message);
        }
        elsif($bits eq 'b0')	#Authentication frame
        {
                print "Auth frame received\n";
		gen_ack_frame($message);
        }
        elsif($bits eq 'c0')	#Deauthentication frame
        {
            	print "Broadcasted Deauthentication frame\n";
		if ($beacon_count ge '1')
		{
			$beacon_count++;
			enter_recv_state();
		}
		else
		{
			$beacon_count++;
			 gen_authentication_request();
		}
        }
        elsif($bits eq 'd0')	#Action frame
        {
                gen_ack_frame($message);
        }
	elsif($bits eq 'e0')	#Action with no ack
        {
                LOG("\nACTION WITH NO ACK\n");
        }
        elsif($bits eq 'b4')	#RTS frame
        {
                gen_cts();
        }
        elsif($bits eq 'a4')	#PS-POLL
        {
                gen_ack_frame($message);
        }
        elsif($bits eq 'e4')	#CF-END
        {
                LOG("\nCF-END\n");
        }
        elsif($bits eq 'f4')	#CF_END+CF-ACK
        {
                gen_ack_frame($message);
        }
        elsif($bits eq '08')	#Data frame
        {
                gen_ack_frame($message);
        }
        elsif($bits eq '18')	#Data+CF-ACK
        {
                gen_ack_frame($message);
        }
        elsif($bits eq '28')	#Data+CF-POLL
        {
                gen_ack_frame($message);
        }
        elsif($bits eq '38')	#Data+CF-ACK+CF-POLL
        {
                gen_ack_frame($message);
        }
	elsif($bits eq '48')	#Null(NO Data) 
        {
                print "\nNULL FUNCTION\n";
        }
        elsif($bits eq '78')	#Cf-ack+cf-poll
        {
                gen_ack_frame($message);
        }
        elsif($bits eq '88')	#QoS Data
        {
                gen_ack_frame($message);
        }
        elsif($bits eq '98')	#QoS Data+CF-Ack
        {
                gen_ack_frame($message);
        }
        elsif($bits eq 'a8')	#QoS Data+Cf-poll
        {
          	gen_ack_frame($message);
        }
        elsif($bits eq 'e8')	#QoS Cf-poll
        {
     		LOG("\nQOS CF-POLL\n");
        }
        elsif($bits eq 'f8')	#QoS cf-ack+cf-poll
        {
                gen_ack_frame($message);
        }
        elsif($bits eq 'd4')	#Ack frame
        {
                print "Acknowledgment frame received\n";
		#recv_msg();
		open(FILE, "./config-files/uu_wlan_cca-reset.txt");
		$cca_reset_confirm = <FILE>;
		events_handling($cca_reset_confirm);
		if($data_ack_count eq '10')
		{
			enter_endcase();
		}
		elsif(($data_ack_count eq '1') && ($num eq '004'|| $num eq '009'))
		{
			enter_endcase();
		}
		else
		{
			enter_recv_state();
		}
        }
        else
        {
                LOG("Received from phy_Stub with unknown type/subtype \n");
                $casefailed=1;
        }

}

sub enter_endcase
{
       	open(FILE,"./config-files/uu_wlan_endcase.txt");
	$file=<FILE>;
	events_handling($file);
    
}

sub enter_recv_state
{
	open(FILE,"./config-files/uu_wlan_receiving_state.txt");
	$tx_data=<FILE>;
	local $/;
	events_handling($tx_data);

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
}

sub gen_authentication_request
{
	$file_name="../sample_frames/uu_wlan_management_authenticationrequest.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	start_recv_proc($file_data);
}

sub gen_deauthentication
{
	$file_name="../sample_frames/uu_wlan_management_deauthentication.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	start_recv_proc($file_data);


}

sub gen_association_request
{
	print " its at gen_association request\n";
	$file_name="../sample_frames/uu_wlan_management_associationrequest.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	start_recv_proc($file_data);
}

sub gen_data
{
	print "its in data transmission\n";
	$file_name="../sample_frames/uu_wlan_data_data.txt";
	open(FILE,$file_name);
	$file_data=<FILE>;
	start_recv_proc($file_data);
}

sub gen_association_resp
{
        $fname='../frames/management/uu_wlan_management_associationresponse.txt';
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

sub gen_probe_req
{
        $fname='../sample_frames/uu_wlan_management_proberequest.txt';
        open(FILE,$fname);
        $data=<FILE>;
	start_recv_proc($data);
}

sub gen_ack_frame
{
        $ack_data=shift;
        $ack_data=substr($ack_data,20,12);
        #LOG("Previous frame RA is: $ack_data");
        $filename='../sample_frames/uu_wlan_control_acknowledgement.txt';
        open(FILE,$filename);
        $file_data=<FILE>;
        $file_data=~ s/aa*/$ack_data/ ;
       	##LOG("The generated ack frame is ",$file_data);
	crc($file_data);
}

sub gen_cts
{
        $fname='../sample_frames/cts.txt';
        open(FILE,$fname);
        $data=<FILE>;
        LOG("$data\n");
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

sub update_operation
{
        LOG("Update operation \n");
}

sub readframe_operation
{
        LOG("Readframe operation \n");
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


sub nlmsg_send
{
	my($nlmsg_type,$nlmsg_pid,$nlmsg)=@_;
	#LOG("Sending msg is $nlmsg \n");
	
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


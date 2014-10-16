#!/usr/bin/perl 

#use strict;
#use warnings;

use Time::HiRes qw/ gettimeofday time usleep/; #Used for obtaining time of the day

use IO::Socket::Netlink; #Used for creating Netlink sockets  
use Readonly;#Used for defining Macro's  

# Used to import functions from other files.
require './Scripts/frame_info_parameters.pl'; #for filling frame information structure.
require './Scripts/frame_capabilities.pl'; #for filling frameheader and body fields.
require './Scripts/registers.pl'; #for filling registers.
require './Scripts/excel_generation.pl'; #for auto generation of excel sheets to record test results

# Mac-Phy interface primitive variables.
my($UU_PHY_INVALID_EVENT, $UU_PHY_DATA_REQ, $UU_PHY_DATA_CONF, $UU_PHY_DATA_IND, $UU_PHY_TXSTART_REQ, $UU_PHY_TXSTART_CONF, $UU_PHY_TXEND_REQ, $UU_PHY_TXEND_CONF, $UU_PHY_RXSTART_IND, $UU_PHY_RXEND_IND, $UU_PHY_CCARESET_REQ, $UU_PHY_CCARESET_CONF, $UU_PHY_CCA_IND) = (-01..11);

#configurations
$PHY_CMD = 2; #Header for sending Phy-indications. 
$delimiter = "!";

#Macro's definition
Readonly our $umacstub_protoid => 24;
Readonly our $config_registers_protoid => 25;
Readonly our $phystub_protoid => 29;
Readonly our $group_id => 99;
Readonly our $captured_frames_dir => "./captured_frames";
Readonly our $config_files_dir => "./config_files";
Readonly our $umac_stub => 1;
Readonly our $phy_stub => 0;

#counters
our $testcase_started = 0;
our $casefailed = 0;
our $totaltestcases=0;
our $testcasesfailed=0;
our $flag=0;
our $retry_count=0;
our $state_change_protocol = $umac_stub; # 1 for umac_stub. 0 for phy_stub 


#Determines Number of command line arguments.
my $num_of_args = $#ARGV + 1; 
if ($num_of_args < 1) 
{
    LOG("\nUsage: filename.pl testcasesfile.txt \n");
    exit;
}

our $nl_sock_umac_stub; #Netlink Socket for Test Driver(umac-stub).
our $nl_sock_phy_stub;  #Netlink Socket for Phy-stub.
our $nl_sock_config_registers; #Netlink Socket for Registers

nl_sock_open_umac_stub($umacstub_protoid,$group_id);
nl_sock_open_phy_stub($phystub_protoid,$group_id);
nl_sock_open_config_registers($config_registers_protoid,$group_id);

# Netlink Socket creation for Test driver (umac stub)
sub nl_sock_open_umac_stub
{
    my($protocol,$groups)=@_;
    print "protocol id for Test driver is :$protocol\t groups id is :$groups\n";
    $nl_sock_umac_stub = IO::Socket::Netlink->new( Protocol => $protocol , Groups => $groups) or die "socket: $!";
    return $nl_sock_umac_stub;
}
# Netlink Socket creation for PHY stub
sub nl_sock_open_phy_stub
{
    my($protocol,$groups)=@_;
    print "protocol id for phy stub is :$protocol\t groups id is :$groups\n";
    $nl_sock_phy_stub = IO::Socket::Netlink->new( Protocol => $protocol , Groups => $groups) or die "socket: $!";
    return $nl_sock_phy_stub;
}
# Netlink Socket creation for configuring registers
sub nl_sock_open_config_registers
{
    my($protocol,$groups)=@_;
    print "protocol id for Registers is :$protocol\t groups id is :$groups\n";
    $nl_sock_config_registers = IO::Socket::Netlink->new( Protocol => $protocol , Groups => $groups) or die "socket: $!";
    return $nl_sock_config_registers;
}

# Script execution starts from here.
our $file_name_main=$ARGV[0]; # $ARGV[0] is the text file that contains list of all testcases. 
open(FILE,$file_name_main);
local $/; #access whole file
my $file_content_main=<FILE>;
events_handling($file_content_main);

# Handles events from testcases 
sub events_handling
{
    my $event_data=shift;
    our @content=split(/\n/,$event_data); 

    foreach  $individual_content(@content)
    {
        @member=split(" ",$individual_content);
        if($ARGV[1] eq $member[0]) #execution of a particular test case 
        {
            our $global_testcase_num = $member[0];
            our $global_testcase_name = $member[1];
            our $global_testcase = $member[2];
            our $global_configuration_file = $member[3];
            our $global_frameinfo_file = $member[4];
            our $global_registers_file = $member[5];
            default_registers(); #function to fill registers
            nlmsg_send_config_registers($config_registers_protoid,$group_id,$registers); #Send message through test driver(umac-stub).    
            print "\t\tTestcase Name:$global_testcase_name\n\n";
            frame_info_params(); #function to fill frame info structure
            open(FILE,$global_testcase);
            local $/; #access whole file
            my $file_cont=<FILE>;
            testcase_execution($file_cont);
        }

        elsif($ARGV[1] eq '') #execution of all test cases
        {
            our @input_file = @content;
            foreach $indv_content(@input_file) 
            {
                if($indv_content=~ /^#/) #Test cases starting with "#" are ignored and proceeded further.
                {
                    @ignore_cases=split(" ",$indv_content);
                    our $global_test_num = $ignore_cases[0];
                    our @ignore_cases_list;
                    push(@ignore_cases_list,$global_test_num);
                    next;   
                }
                else
                {
                    @args=split(" ",$indv_content);
                    our $global_testcase_num = $args[0];
                    our $global_testcase_name =$args[1];
                    our $global_testcase= $args[2]; 
                    our $global_configuration_file = $args[3];
                    our $global_frameinfo_file = $args[4]; 
                    our $global_registers_file =$args[5]; 
                    default_registers(); #function to fill registers
                    nlmsg_send_config_registers($config_registers_protoid,$group_id,$registers); #Send message through test driver(umac-stub).    
                    print "\t\tTestcase Name:$global_testcase_name\n\n";
                    frame_info_params();
                    open(FILE,$global_testcase);
                    local $/; #access whole file    
                    my $file_cont=<FILE>;
                    testcase_execution($file_cont);
                    undef @content, @args; #Makes all variables to null after execution of every test case.
                    sleep 1; # Waits for 1 second to excecute next test case. 
                }
            }
        }
        else
        {
# To be filled later as per requirements.
        }

    }#for loop
}

sub testcase_execution
{
    $file_data=shift;
    @cont=split("\n",$file_data);
    foreach  $individual_cont(@cont)
    {
        @argument=split(" ",$individual_cont);
        if ($casefailed == 1)
        {	
            if (!(($argument[0] eq 'END') || ($argument[0] eq 'Endcase')))
            {
                next;
            }
            $testcasesfailed++;
            our @fail_testcases_list;
            push(@fail_testcases_list,$global_testcase_num);
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
        elsif($argument[0] eq 'DELAY')
        {
            usleep($argument[1]);
        }
        elsif($argument[0] eq 'STATE')
        {
            our $present_state=$argument[1];
            present_state($present_state);
        }	
        elsif($argument[0] eq 'END')
        {
            end_operation();
        }	
        elsif($argument[0] eq 'PS_MODE')
        {
            ps_mode($argument[1]);
        }
        elsif($argument[0] eq 'CLEAR_RX') #TODO: Fix this
        {
            $recv_type_rx ='';
        }
        elsif($argument[0] eq 'AC_CHANGE')
        {
            ac_change($argument[1]);
        }
        elsif($argument[0] eq 'Format_change')
        {
            format_change($argument[1]);	   
        }
        elsif($argument[0] eq 'MULTIDATA_CHANGE')
        {
            multidata_change();
        }
    }
}

#To perform multiple data transmissions
sub multidata_change
{
    $changed = shift;
    $multi_new = sprintf("%02x",$arg[1]);
    $new =substr $registers,-2,2, "$multi_new";
    nlmsg_send_config_registers($config_registers_protoid,$group_id,$registers);
}

#For transmitting different format frames in a single testcase
sub format_change
{
    $format_value = shift;
    $format_value = sprintf("%02b",$format_value);
    substr $txv_byte1,-2,2, "$format_value";
    $hex_byte1=sprintf("%02x",oct("0b".$txv_byte1));
    substr $txvect,0,2,$hex_byte1;
    $newfile = $txvect.$frame_info.$other_info;
}

# For transmitting different AC frames- used in testing internal collision
sub ac_change
{
    $changed_ac= shift;
    substr $other_info,5,1,$changed_ac;
    $newfile=$txvect.$frame_info.$other_info;
}


#changing ps mode operation
sub ps_mode
{
    $ps = shift;
    substr $registers,-3,1,$ps;
    nlmsg_send_config_registers($config_registers_protoid,$group_id,$registers);
}

# Perform send command operation
sub send_operation
{
    send_msg();
}

# Perform recv command operation
sub recv_operation
{
    recv_msg();
}

# Perform testcase command operation
sub testcase_operation
{	
    if($argument[1]=~/\d+$/)
    {
        our $num=$argument[1];
    }
    else
    {
        our $num=$global_testcase_num;
    }
    LOG("TestCase $num \n");
}

# Perform Endcase command operation
sub endcase_operation
{
    $testcase_started = 0;
    if($ARGV[1] eq "") { excelsheet(); }
    $casefailed = 0;
    if($casefailed == 0)
    {
        $totaltestcases++;
    }
    $testcasespassed=$totaltestcases-$testcasesfailed;
    LOG("EndCase $num \n");
}

# Perform PHY-IND command operation 
sub send_phy_ind
{
    if($argument[1] eq 'UU_PHY_INVALID_EVENT')
    {
        my $phy_cmd = $UU_PHY_INVALID_EVENT;
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

    our $msg = $PHY_CMD.$phy_cmd.$delimiter;

    if($argument[2] eq '-TF') # for Text file
    {
        read_phy_content($argument[3], $msg);
    }
    elsif(($argument[2] =~/[a-z]/) || ($argument[2] =~/[A-Z]/) || ($argument[2] =~/[0-9]/))
    {	
        my $data = $argument[2];
        $msg = $msg.$data;
        send_msg($msg);	
    }	
    else
    {
        send_msg($msg);
    }
}

# Reads PHY-IND argument from text file and sends it through netlink socket 
sub read_phy_content
{
    my($file_name,$phy_ind)=@_;
    local $/;
    open(FILE,$file_name);
    my $mess=<FILE>;
    my $data= $phy_ind.$mess;
    $data=~s/\n$//;	#To remove last new line of the $data
    send_msg($data);
}

# Performs GENERATE command operations
sub generate_frames
{
    if($argument[1] eq 'BEACON')
    {
        generate_beacon();	
    }
    elsif($argument[1] eq 'IBSS_BEACON')
    {
        generate_ibssbeacon();	
    }
    elsif($argument[1] eq 'MESH_BEACON')
    {
        generate_meshbeacon();	
    }
    elsif($argument[1] eq 'MESH_PREQ')
    {
        generate_meshpreq();	
    }
    elsif($argument[1] eq 'MESH_PREP')
    {
        generate_meshprep();	
    }
    elsif($argument[1] eq 'MESH_PERR')
    {
        generate_meshperr();	
    }
    elsif($argument[1] eq 'AUTHENTICATION_REQUEST')
    {
        gen_authentication_request();		
    }
    elsif($argument[1] eq 'AUTHENTICATION_RESPONSE')
    {
        gen_authentication_response();		
    }
    elsif($argument[1] eq 'ASSOCIATION_REQUEST')
    {
        gen_association_request();		
    }
    elsif($argument[1] eq 'ASSOCIATION_RESPONSE')
    {
        gen_association_resp();
    } 
    elsif($argument[1] eq 'DIASSOCIATION')
    {
        gen_diass();
    } 
    elsif($argument[1] eq 'PROBE_REQUEST')
    {
        gen_probe_request();
    }  
    elsif($argument[1] eq 'PROBE_RESPONSE')
    {
        gen_probe_resp();
    }
    elsif($argument[1] eq 'DEAUTHENTICATION')
    {
        gen_deauthentication();
    }
    elsif($argument[1] eq 'DATA')
    {
        gen_data_ap();
    }
    elsif($argument[1] eq 'AGGR_DATA')
    {
        gen_aggr_data();
    }
    elsif($argument[1] eq 'MESH_AMPDU')
    {
        gen_aggr_data_mesh();
    }
    elsif($argument[1] eq 'AGGR_QOSNULL')
    {
        gen_aggr_qosnull();
    }
    elsif($argument[1] eq 'AGGR_MGMT')
    {
        gen_aggr_mgmt();
    }
    elsif($argument[1] eq 'DATA_RTS')
    {
        gen_data_rts();
    }
    elsif($argument[1] eq 'QOS_DATA')
    {
        gen_data_qos();
    }
    elsif($argument[1] eq 'MESH_DATA')
    {
        gen_data_mesh();
    }
    elsif($argument[1] eq 'CTS')
    {
        gen_cts();
    }
    elsif($argument[1] eq 'BAR')
    {
        gen_bar();
    }
    elsif($argument[1] eq 'PS_ZERO')
    {
        ps_zero();
    }	
    elsif($argument[1] eq 'AGGR_PS_ZERO')
    {
        aggr_ps_zero();
    }
    elsif($argument[1] eq 'MULTIDATA_AC')
    {
        multidata_ac($argument[2],$argument[3]);
    }
    elsif($argument[1] eq 'MULTIDATA_RTS')
    {
        multidata_rts($argument[2]);
    }
    elsif($argument[1] eq 'MULTIDATA')
    {
        multidata($argument[2]);
    }
    elsif($argument[1] eq 'UNICAST')
    {
        unicast();
    }
    elsif($argument[1] eq 'BROADCAST')
    {
        broadcast();
    }
    elsif($argument[1] eq 'BROADCAST_NOACK')
    {
        broadcast_noack()
    }
    elsif($argument[1] eq 'BROADCAST_TODS')
    {
        broadcast_tods()
    }
    elsif($argument[1] eq 'UNICAST_NOACK')
    {
        unicast_noack()
    }
}  

sub unicast_noack()
{
    $filename="$captured_frames_dir/unicast_noack.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}

sub broadcast_tods()
{
    $filename="$captured_frames_dir/multicast_tods1.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}

sub broadcast_noack()
{
    $filename="$captured_frames_dir/multicast_tods1_noack.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}

sub broadcast
{
    $filename="$captured_frames_dir/multicast.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}

sub unicast
{
    $filename="$captured_frames_dir/unicast.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}

sub multidata
{
    $cnt=shift;
    for($i=0;$i<$cnt;$i++)
    {
        $file_name="$captured_frames_dir/cap_qos_65mb_retran.txt";
        open(FILE,$file_name);
        $file_data=<FILE>;
        frame_parse($file_data);
        $content=~s/\n//;
        my $len=length($content);
        our $len=$len;
        our $len_bytes=$len/2;
        our $newlen=$len_bytes;
        $len_hex=sprintf("%04x",$newlen);
        @array=unpack("((A2)*)",$len_hex);
        $len_hex=$array[1].$array[0];
        $newfile=$txvect.$frame_info.$other_info;
        $new=substr $newfile,64,4, "$len_hex";
        my $len_data1=length($content)+length($newfile);
        $len_bytes=$len_data1/2;
        $newlen_data1=$len_bytes;
        $len_hex=sprintf("%02x",$newlen_data1);
        $multi_data=$len_hex.$newfile.$content.$multi_data;
        $multi_data=~s/\n//;
    }
    $cnt=sprintf("%02x",$cnt);
    $multi_data=$cnt.$multi_data;
    if($state_change_protocol == $umac_stub)
    {
        nlmsg_send_testdriver($umacstub_protoid,$group_id,$multi_data); #Send message through test driver(umac-stub).
    }
}
sub multidata_rts
{
    $cnt=shift;
    for($i=0;$i<$cnt;$i++)
    {
        $file_name="$captured_frames_dir/cap_11b_qos.txt";
        open(FILE,$file_name);
        $file_data=<FILE>;
        frame_parse($file_data);
        $content=~s/\n//;
        my $len=length($content);
        our $len=$len;
        our $len_bytes=$len/2;
        our $newlen=$len_bytes;
        $len_hex=sprintf("%04x",$newlen);
        @array=unpack("((A2)*)",$len_hex);
        $len_hex=$array[1].$array[0];
        $newfile=$txvect.$frame_info.$other_info;
        $new=substr $newfile,64,4, "$len_hex";
        my $len_data1=length($content)+length($newfile);
        $len_bytes=$len_data1/2;
        $newlen_data1=$len_bytes;
        $len_hex=sprintf("%02x",$newlen_data1);
        $multi_data=$len_hex.$newfile.$content.$multi_data;
        $multi_data=~s/\n//;
    }
    $multi_data=$cnt.$multi_data;
    if($state_change_protocol == $umac_stub)
    {
        nlmsg_send_testdriver($umacstub_protoid,$group_id,$multi_data); #Send message through test driver(umac-stub).
    }
}

sub multidata_ac
{
    @multi_array=@_;
    for($i=0;$i<=($#multi_array);$i++)
    {
        $changed_ac=$multi_array[$i];
        substr $other_info,5,1,$changed_ac;
        $file_name="$captured_frames_dir/cap_qos_65mb_retran.txt";
        open(FILE,$file_name);
        $file_data=<FILE>;
        frame_parse($file_data);
        $content=~s/\n//;
        my $len=length($content);
        our $len=$len;
        our $len_bytes=$len/2;
        our $newlen=$len_bytes;
        $len_hex=sprintf("%04x",$newlen);
        @array=unpack("((A2)*)",$len_hex);
        $len_hex=$array[1].$array[0];
        $newfile=$txvect.$frame_info.$other_info;
        $new=substr $newfile,64,4, "$len_hex"; 
        my $len_data1=length($content)+length($newfile);
        $len_bytes=$len_data1/2;
        $newlen_data1=$len_bytes;
        $len_hex=sprintf("%02x",$newlen_data1);
        $multi_data=$len_hex.$newfile.$content.$multi_data;
        $multi_data=~s/\n//;
    }
    if($state_change_protocol == $umac_stub)
    {
        nlmsg_send_testdriver($umacstub_protoid,$group_id,$multi_data); #Send message through test driver(umac-stub).
    }

}
# Generates ps frame with ps bit 0
sub ps_zero
{
    $file_name="$captured_frames_dir/ps_bit_zero.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    send_msg($file_data);
#file_process($file_name);
}

sub aggr_ps_zero
{
    $file_name="$captured_frames_dir/aggr_ps2.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    send_msg($file_data);
#file_process($file_name);
}
# Generates beacon frame and sends it through netlink socket
sub generate_beacon
{
    $file_name_beacon="$captured_frames_dir/beacon.txt"; 
    file_process($file_name_beacon);
}

# Generates IBSS beacon frame and sends it through netlink socket
sub generate_ibssbeacon
{
    $file_name_beacon="$captured_frames_dir/ibssbeacon.txt";
#open(FILE,$file_name_beacon);
#$data=<FILE>;
#send_msg($data);
    file_process($file_name_beacon);
}

#Generates Mesh beacon frame 
sub generate_meshbeacon
{
    $file_name_beacon="$captured_frames_dir/meshbeacon.txt";
    file_process($file_name_beacon);
}
#Generates Mesh Path request frame 
sub generate_meshpreq
{
    $filename="$captured_frames_dir/meshpreq.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}
#Generates Mesh Path reply frame 
sub generate_meshprep
{
    $filename="$captured_frames_dir/meshpreply.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}
#Generates Mesh Path error frame 
sub generate_meshperr
{
    $filename="$captured_frames_dir/meshperror.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);
}
# Generates authentication request frame and sends it through netlink socket
sub gen_authentication_request
{
    $file_name="$captured_frames_dir/cap_auth_req_non-qos.txt";
    file_process($file_name);
}

# Generates authentication response frame and sends it through netlink socket
sub gen_authentication_response
{
    $file_name="$captured_frames_dir/cap_auth_res_non-qos.txt"; 
    file_process($file_name);
}

# Generates association request frame and sends it through netlink socket
sub gen_association_request
{
    $file_name="$captured_frames_dir/cap_ass_req_non-qos.txt";
    file_process($file_name);
}

# Generates association response frame and sends it through netlink socket
sub gen_association_resp
{
    $fname="$captured_frames_dir/cap_ass_res_non-qos.txt";
    file_process($fname);
}

# Generates probe request frame and sends it through netlink socket
sub gen_probe_request
{
    $filename="$captured_frames_dir/cap_probe_req_non-qos.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    send_msg($file_data);    
#file_process($filename);
}

# Generates probe response frame and sends it through netlink socket
sub gen_probe_resp
{
    $fname="$captured_frames_dir/cap_probe_res_non-qos.txt";
    file_process($fname);
#open(FILE,$fname);
#$data=<FILE>;
#send_msg($data);
}

#Generates Disassociation frame and sends it through netlink socket
sub gen_diass
{
    $fname="$captured_frames_dir/cap_diass_non-qos.txt";
    open(FILE,$fname);
    $data=<FILE>;
    send_msg($data);
}

# Generates deauthentication frame and sends it through netlink socket 
sub gen_deauthentication
{
    $file_name="$captured_frames_dir/cap_deauth_non-qos.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    send_msg($file_data);
}

# Generates data frame and sends it through netlink socket 
sub gen_data_ap
{
    $file_name="$captured_frames_dir/cap_54mbps_non-qos.txt";#gendata_ap.txt";
    $bcn_value="00";
    $new=substr $newfile,68,2, "$bcn_value"; # beacon is replaced with latest beacon value.
    file_process($file_name); 
}

# Process the input text file
sub file_process
{
    $filename=shift;
    open(FILE,$filename);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data. 
    send_msg($content); # $content is received from fcs function.
}

# Generates Aggregated data
sub gen_aggr_data
{
    $file_name="$captured_frames_dir/aggr_data1.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data1=length($content);
    $len_bytes=$len_data1/2;
    $newlen_data1=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data1);
    $aggr_data=$len_hex.$content;
    $aggr_data=~s/\n// ;
    $file_name="$captured_frames_dir/aggr_data2.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data2=length($content);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$content;
    $aggr_data=~s/\n// ;
    $file_name="$captured_frames_dir/aggr_data3.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data2=length($content);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$content;
    $aggr_data=~s/\n// ;
    $file_name="$captured_frames_dir/aggr_data4.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
#frame_parse($file_data);
#$content=~s/\n// ; # Removes new line from the data.

    my $len_data2=length($file_data);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$file_data;
    $aggr_data=~s/\n// ;
    send_msg($aggr_data); # content is obtained from fcs function.
}

# Generates Aggregated mesh data
sub gen_aggr_data_mesh
{
    $file_name="$captured_frames_dir/mesh_data_4addr.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data1=length($content);
    $len_bytes=$len_data1/2;
    $newlen_data1=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data1);
    $aggr_data=$len_hex.$content;
    $aggr_data=~s/\n// ;
    $file_name="$captured_frames_dir/mesh_data_4addr.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data2=length($content);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$content;
    $aggr_data=~s/\n// ;
    $file_name="$captured_frames_dir/mesh_data_4addr.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data2=length($content);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$content;
    $aggr_data=~s/\n// ;
    $file_name="$captured_frames_dir/mesh_data_4addr.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
#frame_parse($file_data);
#$content=~s/\n// ; # Removes new line from the data.

    my $len_data2=length($file_data);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$file_data;
    $aggr_data=~s/\n// ;
    send_msg($aggr_data); # content is obtained from fcs function.
}

#Generate QoS null data
sub gen_aggr_qosnull
{
    $file_name="$captured_frames_dir/qos_null.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data1=length($content);
    $len_bytes=$len_data1/2;
    $newlen_data1=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data1);
    $aggr_data=$len_hex.$content;
    $aggr_data=~s/\n// ;
    $file_name="$captured_frames_dir/qos_null.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    frame_parse($file_data);
    $content=~s/\n// ; # Removes new line from the data.

    my $len_data2=length($content);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$content;
    $aggr_data=~s/\n// ;
    send_msg($aggr_data);
}

# Aggregation of Management frames
sub gen_aggr_mgmt
{
    $file_name="$captured_frames_dir/action_noack.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
#frame_parse($file_data);
#$content=~s/\n// ; # Removes new line from the data.
    my $len_data1=length($file_data);
    $len_bytes=$len_data1/2;
    $newlen_data1=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data1);
    $aggr_data=$len_hex.$file_data;
    $aggr_data=~s/\n// ;

    $file_name="$captured_frames_dir/action_noack.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
#frame_parse($file_data);
#$content=~s/\n// ; # Removes new line from the data.
    my $len_data2=length($file_data);
    $len_bytes=$len_data2/2;
    $newlen_data2=$len_bytes;
    $len_hex=sprintf("%02x",$newlen_data2);
    $aggr_data=$aggr_data.$len_hex.$file_data;
    $aggr_data=~s/\n// ;
    send_msg($aggr_data);
}

# Generates data frame with exceeded RTS threshold value and sends it through netlink socket 
sub gen_data_rts
{
    $file_name="$captured_frames_dir/cap_qos_65mb.txt";
    file_process($file_name);
}

# Generates QoS data frame and sends it through netlink socket 
sub gen_data_qos
{
    $file_name="$captured_frames_dir/cap_qos_65mb_retran.txt";
    $bcn_value="00"; #Beacon value in frame info structure is turned to 0.
    $new=substr $newfile,68,2, "$bcn_value"; # beacon is replaced with latest beacon value.
    file_process($file_name);
}

# Generates Mesh data frame and sends it through netlink socket 
sub gen_data_mesh
{
    $file_name="$captured_frames_dir/mesh_data_4addr.txt";
    $bcn_value="00"; #Beacon value in frame info structure is turned to 0.
    $new=substr $newfile,68,2, "$bcn_value"; # beacon is replaced with latest beacon value.
    file_process($file_name);
}
# Generates cts frame and sends it through netlink socket 
sub gen_cts
{
    $cts_data=shift;
    $cts_data=substr($ack_data,20,12);
    $filename="$captured_frames_dir/cts.txt";
    open(FILE,$filename);
    $file_data=<FILE>;
    $file_data=~s/aa*/$ack_data/ ;
    fcs_calculation($file_data);
    send_msg($content);
}

# Generates bar frame and sends it through netlink socket 
sub gen_bar
{
    $file_name="$captured_frames_dir/gen_bar.txt";
    open(FILE,$file_name);
    $file_data=<FILE>;
    send_msg($file_data);
}

# Obtains the frame capabilities text file (arg 3) from the list of test cases and parses it. 
sub frame_parse
{
    $file_cont=shift;
    $configname=$global_configuration_file;
    open(FILE,$configname);
    local $/;
    $config_msg=<FILE>;
    frame_manipulation($file_cont,$config_msg);
}

# Manipulates the frame header and body based on their type and sub type
sub frame_manipulation
{	
    my($file_content,$config_data)=@_;
    $type=substr($file_content,0,2); # obtains the sub-type of the frame	
    $authtransno=substr($file_content,52,4); # obtains authentication transaction number. Used to verify whether it is authenticaton request frame or response frame.
    our $file_message=$file_content;
    my @cont=split("FRAMETYPE",$config_data);
    if($type eq '80' || $type eq '50') # Beacon frame or Probe response frame
    {
        $beacon_parse=$cont[1];
        @parse_newline=split("\n",$beacon_parse);
        foreach $ind_cont(@parse_newline)
        {
            our @arg=split(" ",$ind_cont);
            if(!$arg[1] eq '' )
            {
                default_fields($file_content,$config_data);
                beacon_fields($file_content,$config_data);
            }
            else
            {
                default_fields_unchanged($file_content,$config_data);	
                beacon_fields_unchanged($file_content,$config_data);
            }
            undef $type;
        }
    }
    elsif($type eq 'b0' && $authtransno eq '0100') # Authentication request frame
    {
        $auth_parse=$cont[2];
        @parse_newline_auth=split("\n",$auth_parse);
        foreach $ind_cont(@parse_newline_auth)
        {
            our @arg=split(" ",$ind_cont);
            if(!$arg[1] eq '' )
            {
                default_fields($file_content,$config_data);
                authentication_fields($file_content,$config_data);
            }
            else
            {
                default_fields_unchanged($file_content,$config_data);	
                authentication_fields_unchanged($file_content,$config_data);
            }
        }
    }
    elsif($type eq 'b0' && $authtransno eq '0200') # Authentication response frame
    {
        $auth_parse=$cont[3];
        @parse_newline_auth=split("\n",$auth_parse);
        foreach $ind_cont(@parse_newline_auth)
        {
            our @arg=split(" ",$ind_cont);
            if(!$arg[1] eq '' )
            {
                default_fields($file_content,$config_data);
                authentication_fields($file_content,$config_data);
            }
            else
            {
                default_fields_unchanged($file_content,$config_data);	
                authentication_fields_unchanged($file_content,$config_data);
            }
        }
    }
    elsif($type eq '00') # Association request frame
    {
        $asso_req_parse=$cont[4];
        @parse_newline_asso=split("\n",$asso_req_parse);
        foreach $ind_cont(@parse_newline_asso)
        {
            our @arg=split(" ",$ind_cont);
            if(!$arg[1] eq '' )
            {
                default_fields($file_content,$config_data);
                association_request_fields($file_content,$config_data);
            }
            else
            {
                default_fields_unchanged($file_content,$config_data);	
                association_request_fields_unchanged($file_content,$config_data);
            }
        }
    }
    elsif($type eq '10') # Association response frame
    {
        $asso_parse=$cont[5];
        @parse_newline_asso=split("\n",$asso_parse);
        foreach $ind_cont(@parse_newline_asso)
        {
            our @arg=split(" ",$ind_cont);
            if(!$arg[1] eq '' )
            {
                default_fields($file_content,$config_data);
                association_fields($file_content,$config_data);
            }
            else
            {
                default_fields_unchanged($file_content,$config_data);	
                association_fields_unchanged($file_content,$config_data);
            }
        }
    }
    elsif($type eq '08' || $type eq '88' || $type eq 'c8' || $type eq '48') # Data frame or QoS data frame
    {
        $data_parse=$cont[6];
        @parse_newline_data=split("\n",$data_parse);
        foreach $ind_cont(@parse_newline_data)
        {
            our @arg=split(" ",$ind_cont);
            if(!$arg[1] eq '' )
            {
                default_fields($file_content,$config_data);

            }
            else
            {
                default_fields_unchanged($file_content,$config_data);	
            }
        }
    }
    else #For other frames
    {
        return 0;
    }

    if($type eq '08' || $type eq '88'|| $type eq 'c8' || $type eq '48')
    {
        
        $new=substr $file_content,0,52,"$target_file";
        fcs_calculation($file_content);
    }
    else
    {
        fcs_calculation($target_file);
    }
}

# Performs FCS calculation of the frame
sub fcs_calculation
{
    $content=shift;
    my $crc=crc_calculation($content);
    $fcs=sprintf("%08x\n",$crc);
    $content=$content.$fcs;
}

# 32 bit CRC calculation
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
}

# clears CRC
sub crc_clear
{
    $crc = 0xFFFFFFFF;
}

# CRC table
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

# Sends data to netlinks sockets
sub send_msg
{
    my $data=shift;
    my $len=length($data);
    our $len=$len;
    our $len_bytes=$len/2;
    our $newlen=$len_bytes;
    $len_hex=sprintf("%04x",$newlen);
    @array=unpack("((A2)*)",$len_hex);
    $len_hex=$array[1].$array[0];
    $new=substr $newfile,64,4, "$len_hex"; # frameinfolen is replaced with obtained length.
    $data_pack=pack("a$len",$data); 
    $newdata=$newfile.$data_pack;
    LOG("sending message is: $data and Length is: $len\n");	# To perform send operation via sockets you need to create a server socket

        if($state_change_protocol == $umac_stub)
        {
            nlmsg_send_testdriver($umacstub_protoid,$group_id,$newdata); #Send message through test driver(umac-stub).
        }
        else
        {
            nlmsg_send_phystub($phystub_protoid,$group_id,$data); #Send message through phy-stub.
        }

}

sub nlmsg_send_config_registers
{
    my($nlmsg_type,$nlmsg_pid,$nlmsg)=@_;
    $nl_sock_config_registers->send_nlmsg($nl_sock_config_registers->new_request(
                nlmsg_type  => $nlmsg_type,
                nlmsg_pid => $nlmsg_pid,
                nlmsg       => $nlmsg,
                ) ) or warn "send: $!";
}


# Sends message through test driver netlink socket
sub nlmsg_send_testdriver
{
    $state_change_protocol=$phy_stub;
    my($nlmsg_type,$nlmsg_pid,$nlmsg)=@_;

# File operations for generating inputs for tx.
    open(FILE,'>./config_files/input_tx.txt');
    print FILE "$nlmsg";
    open(FILE,"$config_files_dir/input_tx.txt");
    $input_data=<FILE>;
    local $/;
    $input_data=~s/\0/0/g; # replaces every packed bits with 0
#open(FILE,'>>./dummy_input_tx.txt');
#print FILE "$input_data"."\n\n";

        ($seconds, $microseconds) = gettimeofday;
    $start_time_microseconds = $microseconds;
    $start_time_seconds = $seconds;
    print "start time in seconds :   ".$start_time_seconds."\t";
    print "start time in microseconds:   ".$start_time_microseconds."\n";

    $nl_sock_umac_stub->send_nlmsg($nl_sock_umac_stub->new_request(
                nlmsg_type  => $nlmsg_type,
                nlmsg_pid => $nlmsg_pid,
                nlmsg       => $nlmsg,
                ) ) or warn "send: $!";
}

# Sends message through PHY stub netlink socket
sub nlmsg_send_phystub	
{
    my($nlmsg_type,$nlmsg_pid,$nlmsg)=@_;
#open(FILE,'>>./dummy_input_rx.txt');  # File operations for generating inputs for rx.
#print FILE "$nlmsg"."\n\n";
    $nl_sock_phy_stub->send_nlmsg($nl_sock_phy_stub->new_request(
                nlmsg_type  => $nlmsg_type,
                nlmsg_pid => $nlmsg_pid,
                nlmsg       => $nlmsg,
                ) ) or warn "send: $!";
}

# Receives message from netlink sockets
sub recv_msg
{			
    my $recv_data=shift;
    print "Current state is(1 for umac-stub,0 for phy-stub): $state_change_protocol\n";
    if($state_change_protocol == $phy_stub )
    {
        nlmsg_recv_phystub();
    }
    else
    {
        nlmsg_recv_testdriver();
    }
}

# Receive message from PHY stub netlink socket
sub nlmsg_recv_phystub
{
    eval
    {
        $SIG{ALRM}=sub{
            if((($da eq "ff" || $da eq "01")&& ($tods_rx ne "1")) ||(($type_rx eq "88")&&($qos_policy_rx ne "01")) ) 
            {
                print "\nReceived frame is group cast frame and no response will be received from AP\n";	
                $casefailed=0;
            }
            #else
            #{
            #    $casefailed =1;
            #}
            print "timeout reached, after 5 seconds!\n";};#$casefailed=0; }; 
            alarm 5;
            $nl_sock_phy_stub->recv_nlmsg($recv_data,65536) or die "recv:::::::::::::;: $!";

# Timers functionality
            ($seconds, $microseconds) = gettimeofday;
            $end_time_microseconds = $microseconds;
            $end_time_seconds = $seconds;
            print "end time in seconds :   ".$end_time_seconds."\t";
            print "end time in microseconds:   ". $end_time_microseconds."\n";

            if($start_time_seconds != $end_time_seconds)
            {
                if(($end_time_seconds-$start_time_seconds) gt "1")	
                {
                    $time_tmp = $end_time_seconds - $start_time_seconds;
                    $time_tmp = ($time_tmp-1)*1000;
                    $final_diff = (1000000 - $start_time_microseconds) +$end_time_microseconds;
                    $final_diff =($final_diff/1000)+$time_tmp;
                    print "final difference in milliseconds when more than 1second difference:".$final_diff."\n";
                }	
                else
                {
                    $final_diff = (1000000 - $start_time_microseconds) +$end_time_microseconds;
                    $final_diff =$final_diff/1000;
                    print "final difference in milliseconds:".$final_diff."\n";
                }
            } 

            else 
            {
                $diff_microseconds = $end_time_microseconds-$start_time_microseconds;
                $final_diff = $diff_microseconds/1000;
                print "final difference in milliseconds:".$final_diff."\n";
            }

            printf "Received type=%d :\nRECV MSG AT STA IS=%v02x\n",$recv_data->nlmsg_type, $recv_data->nlmsg;
            our $result=sprintf( "%v02x",$recv_data->nlmsg);

#open(FILE,'>>./dummy_input_tx.txt');  # File operations for generating output for tx.
#print FILE "$result"."\n\n";

            $test=substr($result,0,2);
            our $tmp = $tmp.$test; #contains starting index of MAC-PHY interface primitives

                $flag=$flag+1;  
            if(($flag eq "3") && (($tmp ne "333035") || ($tmp eq "3333"))) #333035 represents the order of transmission of TX frame
            {
                if($tmp eq "393330")
                {
                    return;
                }
                else
                {
                    print "Mismatched MAC-PHY primitives---RECEIVED UNEXPECTED VECTOR\n\n";
                    $casefailed=1;
                }
            }
            elsif($flag eq "3")
            {
                $tmp ='';
                $flag=0;
            }

            if($test == 30) # 30 represents received starting bytes of data in UU_PHY_DATA_REQ
            {
                our $content_msg=$result;
                lmac_output_validation_tx($content_msg);
            }
            elsif($test == 33) # 33 represents received starting bytes of tx vector in UU_PHY_TXSTART_REQ
            {
                $retry_count=$retry_count+1;
                our $txvector=$result;
            }
            alarm 0;
            $recv_type_rx='';
};

}

# Receives message from test driver netlink socket
sub nlmsg_recv_testdriver
{
    eval
    {
        $SIG{ALRM}=sub{ print "timeout reached, for recv test driver after 3 seconds!\n";$casefailed=1; };
        alarm 5;
        $nl_sock_umac_stub->recv_nlmsg($recv_data,65536) or die "recv: $!";
        printf "Received type=%d :\nRECV MSG AT AP IS=%v02x\n",$recv_data->nlmsg_type, $recv_data->nlmsg;
        our $result=sprintf( "%v02x",$recv_data->nlmsg);
#open(FILE,'>>./dummy_input_tx.txt');  # File operations for generating output for rx.
#print FILE "$result"."\n\n";
        our $msg_type_ap=substr($result,0,2);
        our $content_msg_ap=$msg_type_ap;
        lmac_output_validation_rx($result);
        alarm 0;

    };

}
#LMAC output validation at receiving side 
sub lmac_output_validation_rx
{
    our ($message,$tods_rx,$type_rx,$recv_type_rx,$da,$ack_policy_rx)='';
    $message=shift;
    $message=~s/\.//g;
    our $tods_rx=substr($message,3,1); #Obtains TODS bit from the received rx message.
    our $da=substr($message,8,2); #Obtains Destination Address starting bytes from data
    $tods_rx= unpack('B4',pack('H',$tods_rx)); #Byte to Bit conversion to obtain TODS bit.
    $tods_rx=substr($tods_rx,3,1); 
    $type_rx=substr($message,0,2);
    $recv_type_rx=substr($message,1,1); 
    $recv_type_rx=unpack('B4',pack('H',$recv_type_rx)); #type of frame (MGMT,CONTROL,DATA)
    $recv_type_rx = substr($recv_type_rx,0,2); #gives received frame type in bits (Mgmt -00, Data -10, Control -01)
    $ack_policy_rx = substr($message,48,1);
    $ack_policy_rx = unpack('B4',pack('H',$ack_policy_rx));
    $ack_policy_rx = substr($ack_policy_rx,1,2);
}

#LMAC output validation at transmission side 
sub lmac_output_validation_tx
{
    our ($message,$frame_type_tx,$type_tx,$tods_tx,$duration,$dest_addr,$datarate,$file_cont,@file_array,$line,$qos_mode,$ack_policy_tx) ='';
    $message=shift;
    $message=~s/\.//g;

#validation variables
    $frame_type_tx=substr($message,5,1);
    $type_tx =substr($message,4,2);                 #Obtains type of the frame (using for response frames only)
    $tods_tx =substr($message,7,1);                 #Obtaining byte where TODS bit presents in transmitting frame.
    $tods_tx = unpack('B4', pack('H', $tods_tx));
    $tods_tx =substr($tods_tx,3,1);                 #Obtaining TODS bit from the byte
    $duration =substr($message,8,4);                #Obtains Duration of the transmitted frame
    $dest_addr =substr($message,12,2);              #Obtains Destination Address starting bytes from data
    $txvector =~s/\.//g;
    $datarate =substr($txvector,9,1);               #Obtains data rate of the received frame from TX vector
    $format_tx = substr($txvector,5,1);
    $format_tx = unpack('B4', pack('H', $format_tx));
    $format_tx = substr($format_tx,2,2);	    #To obtain the format of the transmitting frame
    $qos_mode = substr($modes,2,2);		    #Obtain qos mode for qos frames if the qos_mode register is set to 1
    $ack_policy_tx = substr($message,52,1);	    
    $ack_policy_tx = unpack('B4',pack('H',$ack_policy_tx));
    $ack_policy_tx = substr($ack_policy_tx,1,2);    #Obtains the ack policy of transmitting qos frame
    $is_aggr =substr($txvector,27,1);
    $sifs=substr($timers,2,2);
    our    $src = substr($timers,-3,1); #todo
    our    $lrc = substr($timers,-1,1); #todo

#Reading all duration values for corresponding MCS or L-DATARATE values for RTS frame when different formats are using.
    open(FILE,"$config_files_dir/rts_frame_duration.txt");
    $file_rts_frame_duration=<FILE>;
    local $/;
    @rts_format= split("FORMAT ",$file_rts_frame_duration);#splitting the file based on format
    $ind_num = $#rts_format;	
    @rts_array = @rts_format[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains comment

# For RTS frame, reading all datarates and corresponding duration values of RTS frame - 
# when data frame format is '0'.
    @rts_zero = split("\n",$rts_array[0]); 
    $rts_zeroo = $rts_zero[0];
    $rts_zeroo=~s/\n//g;
    $ind_num = $#rts_zero;
    @rts_format_zero = @rts_zero[1..$ind_num]; #copying all arrays to other array except zeroth index, as it contains '0'.

# For RTS frame, reading all datarates and duration values corresponding to given -
# channel bandwidth and guard interval when format is '1'.
    @rts_format_one_split_chbw = split("CH_BW ",$rts_array[1]);	#splitting based on CH_BW
    $rts_one = $rts_format_one_split_chbw[0];	
    $rts_one=~s/\n//g;		
    $ind_num = $#rts_format_one_split_chbw;	
    @rts_format_one = @rts_format_one_split_chbw[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains '1'.

# For RTS frame, reading all datarates and duration values corresponding to given -
# channel bandwidth and guard interval when format is '2'.
    @rts_format_two_split_chbw = split("CH_BW ",$rts_array[2]); #splitting based on CH_BW
    $rts_two = $rts_format_two_split_chbw[0];
    $rts_two=~s/\n//g;
    $ind_num = $#rts_format_two_split_chbw;
    @rts_format_two = @rts_format_two_split_chbw[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains '2'.

# For RTS frame, reading all datarates and duration values corresponding to given - 
# channel bandwidth and guard interval when format is '2'.
    @rts_format_three_split_chbw = split("CH_BW ",$rts_array[3]); #splitting based on CH_BW
    $rts_three=$rts_format_three_split_chbw[0];
    $rts_three=~s/\n//g;
    $ind_num = $#rts_format_three_split_chbw;	
    @rts_format_three = @rts_format_three_split_chbw[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains '3'.

# Reading all the duration values and MCS or L-DATARATE values for DATA frames -
# when different formats are using
    open(FILE,"$config_files_dir/data_frame_duration.txt");
    $file_cont=<FILE>;
    local $/;
    @data_array=split("FORMAT ",$file_cont); #splitting file based on format key word
    $ind_num = $#data_array;
    @data_array = @data_array[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains comment
    
# For DATA frame, reading all datarates and corresponding duration values of RTS frame -
# when data frame format is '0'.
    @zero = split("\n",$data_array[0]);
    $zeroo = $zero[0];
    $zeroo=~s/\n//g;
    $ind_num =$#zero;
    @format_zero = @zero[1..$ind_num]; #copying all arrays to other array except zeroth index, as it contains '0'.   

# For DATA frame, reading all datarates and duration values corresponding to given -
# channel bandwidth and guard interval when format is '1'.
    @one_chbw = split("CH_BW ",$data_array[1]); #splitting based on CH_BW
    $one = $one_chbw[0];         
    $one=~s/\n//g;             
    $ind_num = $#one_chbw;              
    @format_one = @one_chbw[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains '1'.

# For DATA frame, reading all datarates and duration values corresponding to given -
# channel bandwidth and guard interval when format is '2'.
    @two_chbw = split("CH_BW ",$data_array[2]); #splitting based on CH_BW
    $two = $two_chbw[0];         
    $two=~s/\n//g;                  
    $ind_num = $#two_chbw;              
    @format_two = @two_chbw[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains '2'.
   
# For DATA frame, reading all datarates and duration values corresponding to given -
# channel bandwidth and guard interval when format is '3'.
    @three_chbw = split("CH_BW ",$data_array[3]); #splitting based on CH_BW
    $ind_num = $#three_chbw;
    @format_three = @three_chbw[1..$ind_num]; #Copying all arrays to other array except zeroth index, as it contains '3'.


#If the transmitting frame is RTS, its duration is compared with its transmitting frame format type.
#If the transmitting frame is DATA, its duration is compared with its format type.

    if($type_tx eq 'b4' && $format_modified eq $rts_zeroo)    
    {
	rts_duration_validation(@rts_format_zero);
    }
    elsif($type_tx eq 'b4' && $format_modified eq $rts_one )
    {
	@rts_one_datarates = common(@rts_format_one);
	rts_duration_validation(@rts_one_datarates);
    }
    elsif($type_tx eq 'b4' && $format_modified eq $rts_two && $format_tx eq "10")
    {
	@rts_two_datarates = common(@rts_format_two);
	rts_duration_validation(@rts_two_datarates);
    }
    elsif($type_tx eq 'b4' && $format_modified eq $rts_three)
    {
	@rts_three_datarates = common(@rts_format_three);
	rts_duration_validation(@rts_three_datarates);
    }
    elsif(($format_tx eq "00") && ($zeroo eq '0') && ($frame_type_tx eq "8"))
    {
	duration_validation(@format_zero);
    }  
    elsif($format_tx eq "01" && $one eq '1' && $frame_type_tx eq '8')  
    {
	@one_datarates = common(@format_one);
        duration_validation(@one_datarates);
    }
    elsif($format_tx eq "10" && $two eq '2' && $frame_type_tx eq '8')  
    {
	@two_datarates = common(@format_two);
	duration_validation(@two_datarates);
    }
    elsif($format_tx eq "11" && $frame_type_tx eq '8')
    {
	@three_datarates = common(@format_three);
        duration_validation(@three_datarates);
    }

} #end of lmac_output_validation_tx

# This sub-routine gives mcs and duration values corresponding to given channel bandwidths and guard interval 
sub common
{
    @common_c = @_;
    for($ii=0;$ii<$ind_num;$ii++)
    {
	@split_bw=split("\n",$common_c[$ii]); #splits based on new line character
	@temp_array = split(" ",$split_bw[0]); #split_bw[0] contains ch_bw and GI with corresponding values
	$bwdth = $temp_array[0];
	$given_gi = $temp_array[2];
	$ind=$#split_bw;
	if(($bwdth eq $mody_bw) && ($given_gi eq $mody_gi)) #if the bandwidth and GI matched with given bandwidth and GI then corresponding mcs values and duration values are extracted
	{
		@datarates=@split_bw[1..$ind]; #datarates array contains only mcs values and corresponding duration
	}
    }
    return @datarates;
} #End of common function

# This sub-routine is used to validate RTS duration based on its transmitting frame format,
# MCS values, Channel bandwidth and Guard interval.
sub rts_duration_validation
{
    @rts_array_rates=@_;
    foreach $temp_line(@rts_array_rates)
    {
	@temp_line=split(" ",$temp_line);

# If the RTS frame duration is not matched with its transmitting frame format duration,
# then test-case will be failed
        if($type_tx eq 'b4' && $l_datarate_modified eq $temp_line[0] && $format_modified eq '0')
	{
	    if($duration ne $temp_line[1])
	    {
		$casefailed=1;
		return 0;
	    }

	}
	elsif($type_tx eq 'b4' && $ht_mcs_changed eq $temp_line[0] && $format_modified eq '1')
	{
	   if($duration ne $temp_line[1])
	   {
		$casefailed=1;
		return 0;	
	   }
	}
	elsif($type_tx eq 'b4' && $ht_mcs_changed eq $temp_line[0] && $format_modified eq '2')
	{
	   if($duration ne $temp_line[1])
	   {
		$casefailed =1;
		return 0;
	   }
	}
	elsif($type_tx eq 'b4' && $vht_mcs_changed eq $temp_line[0] && $format_modified eq '3')
	{
	   if($duration ne $temp_line[1])
	   {
		$casefailed =1;
		return 0;
	   }
	}
    }# End of foreach loop
}# End of rts_duration_validation sub-routine

#compares with datarates of the given format
sub duration_validation 
{
    @array_rates = @_;
    foreach $line (@array_rates)
    {
        @line=split(" ",$line);
        if(($datarate eq $line[0] && $format_tx eq "00") || ($vht_mcs_changed eq $line[0] && $format_tx eq "11")||(($format_tx eq "01"||$format_tx eq "10") && $ht_mcs_changed eq $line[0]))
        {
            if(($dest_addr eq "ff" || $dest_addr eq "01") && ($tods_tx ne "1")) #Verifies dur for bcast, tods and noack
            { 
                {($duration cmp "0000") or die "mismatched duration and data rate :$!\n"; $casefailed=1};    
            }

            elsif(($is_aggr ne "8" && $type_tx eq "88")&& ((($ack_policy_tx eq "01")||($ack_policy_tx eq "11")) && ($qos_mode eq "01")))#Verifies for aggregated frames
            {
                {($duration cmp "0000") or die "mismatched duration and data rate :$!\n"; $casefailed=1};    
            }	

            elsif(($recv_type_rx eq "10")||($frame_type_tx eq "8")) # data frames validation
            {
                if(($frame_type_tx eq "8")&&($duration ne $line[1])) #validates duration with datarates(TX)
                {
                    $casefailed =1;
                    return 0;
                }
                elsif($recv_type_rx eq "10") # validates for received data frame
                {
                    if((($type_tx ne "c8")&&($type_tx ne "a4"))&& (($type_tx ne "94")&&($type_tx ne "d4")))
                    {	
                        print "Expecting ACK or BA but received other response frame\n ";
                        $casefailed =1;
                    }
                }

            }

            elsif(($recv_type_rx eq "00")||($frame_type_tx eq "0" && $is_aggr ne "8")) # management frame validation
            {
                if(($frame_type_tx eq "0") &&($duration ne $line[1])) #validates duration with rate (TX)
                {
                    $casefailed =1;
                }
                elsif(($recv_add_rx eq "2")&&($type_tx ne "d4")) #Expecting ACK
                {
                    print"Expecting ACK frame but received other frame\n";
                    $casefailed =1;
                }
            }
        }# end of if statement	
    }# end of foreach loop
}# end of the duration_validation function


# Present state :Test driver or PHY stub 
sub present_state
{
    $state=$present_state;
    $state_change_protocol=$state;
    print "STATE:$state_change_protocol\n";
}

# Parses the testcase  
sub enter_generate_state
{
    $generate=shift;
    open(FILE,"$generate");
    $generate_frame=<FILE>;
    local $/;
    events_handling($generate_frame);
}

# Ends the current testcase
sub enter_endcase
{
    open(FILE,"$config_files_dir/endcase.txt");
    $file=<FILE>;
    events_handling($file);
}

# Ends the script execution
sub end_operation
{
    our $tmp=''; #Resetting flags, count and temp variables after execution of each testcase.
    our $flag=0;
    $format_tx =00;
    $retry_count=0;
    @datarates='';
    LOG( "\n\tTestcase Summary\n\tTotal Testcases:$totaltestcases\n\tTestcases Passed:$testcasespassed\n\tTestcases Failed:$testcasesfailed\n\tFailed Testcases are: @fail_testcases_list\n\tIgnored cases are: @ignore_cases_list\nEND operation\n");
}


# Tells the date and time of testcase execution
sub date_time
{
    our @months = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
    our @weekDays = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
    our ($second, $minute, $hour, $dayOfMonth, $month, $yearOffset, $dayOfWeek, $dayOfYear, $daylightSavings) = localtime();
    $year = 1900 + $yearOffset;
    if($second < 10) {$second="0$second";}
    if($minute < 10) {$minute="0$minute";}
    if($hour < 10) {$hour="0$hour";}
    $theTime = "$hour:$minute:$second, $weekDays[$dayOfWeek] $months[$month] $dayOfMonth, $year";
}

# LOG file generation
sub LOG
{
    $msg=shift;
    local $/=undef;
    open (FILE,'>>testlogfile.log');
    print "".date_time()." $msg";
    print FILE "".date_time().$msg;
    close(FILE);
}



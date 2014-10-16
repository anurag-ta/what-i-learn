#!/usr/bin/perl

sub default_fields
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'FRAMECONTROL')
    {
        $target_file=$arg[1];
    }
    elsif($arg[0] eq 'DURATION')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'RA')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'TA')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'BSSID')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'SEQNO')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'QOSCONTROL')
    {
        $target_file=$target_file.$arg[1];
    }
}
sub default_fields_unchanged
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'FRAMECONTROL')
    {
        $fc=substr($file_content,0,4);
        $target_file=$fc;
    }
    elsif($arg[0] eq 'DURATION')
    {
        $dur=substr($file_content,4,4);
        $target_file=$target_file.$dur;
    }
    elsif($arg[0] eq 'RA')
    {
        $da=substr($file_content,8,12);
        $target_file=$target_file.$da;
    }
    elsif($arg[0] eq 'TA')
    {
        $ta=substr($file_content,20,12);
        $target_file=$target_file.$ta;
    }
    elsif($arg[0] eq 'BSSID')
    {
        $bssid=substr($file_content,32,12);
        $target_file=$target_file.$bssid;
    }
    elsif($arg[0] eq 'SEQNO')
    {
        $seq=substr($file_content,44,4);
        $target_file=$target_file.$seq;
    }
    elsif($arg[0] eq 'QOSCONTROL')
    {
        $qos=substr($file_content,48,4);
        $target_file=$target_file.$qos;
    }
    elsif($arg[0] eq 'QOSCONTROL2')
    {
        $qos2=substr($file_content,60,4);
        $target_file=$target_file.$qos2;
    }
}
sub beacon_fields
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'TIMESTAMP')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'BEACONTIMER')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'CAPABILITY')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'SSID')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'SUPPORTEDRATES')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'DSSSPARAMSET')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'TIM')
    {
        $target_file=$target_file.$arg[1];
    }
}
sub beacon_fields_unchanged
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'TIMESTAMP')
    {
        $ts=substr($file_content,48,16);
        $target_file=$target_file.$ts;
    }
    elsif($arg[0] eq 'BEACONTIMER')
    {
        $bt=substr($file_content,64,4);
        $target_file=$target_file.$bt;
    }
    elsif($arg[0] eq 'CAPABILITY')
    {
        $cap=substr($file_content,68,4);
        $target_file=$target_file.$cap;
    }
    elsif($arg[0] eq 'SSID')
    {
        $ssid=substr($file_content,72,20);
        $target_file=$target_file.$ssid;
    }
    elsif($arg[0] eq 'SUPPORTEDRATES')
    {
        $supp=substr($file_content,92,12);
        $target_file=$target_file.$supp;
    }
    elsif($arg[0] eq 'DSSSPARAMSET')
    {
        $dsss=substr($file_content,104,6);
        $target_file=$target_file.$dsss;
    }
    elsif($arg[0] eq 'TIM')
    {
        $tim=substr($file_content,110,12);
        $target_file=$target_file.$tim;
    }
}
sub authentication_fields
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'AUTHALGNO')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'AUTHTRANSNO')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'STATUS')
    {
        $target_file=$target_file.$arg[1];
    }
}
sub authentication_fields_unchanged
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'AUTHALGNO')
    {
        $algno=substr($file_content,48,4);
        $target_file=$target_file.$algno;
    }
    elsif($arg[0] eq 'AUTHTRANSNO')
    {
        $transno=substr($file_content,52,4);
        $target_file=$target_file.$transno;
    }
    elsif($arg[0] eq 'STATUS')
    {
        $status=substr($file_content,56,4);
        $target_file=$target_file.$status;
    }
}
sub association_fields
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'CAPABILITY')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'STATUS')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'AID')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'SUPPORTEDRATES')
    {
        $target_file=$target_file.$arg[1];
    }

}
sub association_fields_unchanged
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'CAPABILITY')
    {
        $cap=substr($file_content,48,4);
        $target_file=$target_file.$cap;
    }
    elsif($arg[0] eq 'STATUS')
    {
        $status=substr($file_content,52,4);
        $target_file=$target_file.$status;
    }
    elsif($arg[0] eq 'AID')
    {
        $aid=substr($file_content,56,4);
        $target_file=$target_file.$aid;
    }
    elsif($arg[0] eq 'SUPPORTEDRATES')
    {
        $supp=substr($file_content,60,12);
        $target_file=$target_file.$supp;
    }
}
sub association_request_fields
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'CAPABILITY')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'LISTENINTERVAL')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'SSID')
    {
        $target_file=$target_file.$arg[1];
    }
    elsif($arg[0] eq 'SUPPORTEDRATES')
    {
        $target_file=$target_file.$arg[1];
    }
}
sub association_request_fields_unchanged
{
    my($file_content,$config_data)=@_;
    if($arg[0] eq 'CAPABILITY')
    {
        $cap=substr($file_content,48,4);
        $target_file=$target_file.$cap;
    }
    elsif($arg[0] eq 'LISTENINTERVAL')
    {
        $listen=substr($file_content,52,4);
        $target_file=$target_file.$listen;
    }
    elsif($arg[0] eq 'SSID')
    {
        $ssid=substr($file_content,56,20);
        $target_file=$target_file.$ssid;
    }
    elsif($arg[0] eq 'SUPPORTEDRATES')
    {
        $supp=substr($file_content,76,12);
        $target_file=$target_file.$supp;
    }

}
1 #Not to be remove. Indicates end of the perl require module.

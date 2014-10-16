`timescale 1ns/1ns
module uu_acmac_rx_handler_test;


int 		sym_len = 27;
int 		i;

integer fp;
integer fp3,fp2,fp4,fp5;
integer vec;
integer fp1;
integer file;
integer file2;
integer file_2nd;
integer file2_2nd;
integer r;
integer r_2nd;
integer r1 ;
integer r2;
integer r3;
integer r4;
integer testcaseid;
integer testcaseid_0;
integer testcaseid_1;
integer event_type;
integer event_type_0;
integer event_type_1;
integer umac_filter;
integer output_value;
integer output_value_data_ind;
integer count = 0;
string array_0;
string equal_to;
string testcaseid_string;
string testcaseid_string_0;
string testcaseid_string_1;
string event_type_string;
string event_type_string_0;
string event_type_string_1;
string filename_string;
string filename_string_0;
string filename_string_1;
string filename;
string data_filename;
string umac_filter_string;
string output_string;
string output_string_0;
string output_string_1;
string end_ind_file;
string array_1; 
string frame_input_file, array_2, array_3,array_4_output, array_5_return;

reg [100023:0]str;
reg [1023:0]  dir;
reg [9023:0]array_10;
reg [100023:0]str_2nd;

////////////////////////////////////////////////////////////////////////////////////
initial 
 begin
  repeat(11)@(posedge uu_acmac_rx_handler_tb.clk);

      dir = "../../rx/test/vectors/";
      fp=$fopen("../../rx/test/vectors/uu_acmac_start_ind_main.txt","r");   
      fp2 =$fopen("../../rx/test/vectors/uu_acmac_data_ind_main.txt","r");   
      fp5 =$fopen("../../rx/test/vectors/uu_acmac_end_ind_main.txt","r");   
   for(int i=1; i<=sym_len; i++) begin //{
    uu_acmac_rx_handler_tb.sym_no = i;
    uu_acmac_rx_handler_tb.packet_length = 0;
    uu_acmac_rx_handler_tb.not_end_of_file = 0;
    uu_acmac_rx_handler_tb.format = 0;
    uu_acmac_rx_handler_tb.modulation = 0;
    uu_acmac_rx_handler_tb.is_long_preamble = 0;
    uu_acmac_rx_handler_tb.reserved1 = 0;
    uu_acmac_rx_handler_tb.is_L_sigvalid = 0;
    uu_acmac_rx_handler_tb.L_datarate = 0;
    uu_acmac_rx_handler_tb.L_length = 0;
    uu_acmac_rx_handler_tb.is_dyn_bw = 0;
    uu_acmac_rx_handler_tb.indicated_chan_bw = 0;
    uu_acmac_rx_handler_tb.rssi = 0;
    uu_acmac_rx_handler_tb.rcpi = 0;
    uu_acmac_rx_handler_tb.is_smoothing = 0;
    uu_acmac_rx_handler_tb.is_sounding = 0;
    uu_acmac_rx_handler_tb.is_short_GI = 0;
    uu_acmac_rx_handler_tb.stbc = 0;
    uu_acmac_rx_handler_tb.num_ext_ss = 0;
    uu_acmac_rx_handler_tb.mcs = 0;
    uu_acmac_rx_handler_tb.is_fec_ldpc_coding = 0;
    uu_acmac_rx_handler_tb.rx_start_of_frame_offset = 0;
    uu_acmac_rx_handler_tb.rec_mcs = 0;
    uu_acmac_rx_handler_tb.reserved2 = 0;
    uu_acmac_rx_handler_tb.psdu_length = 0;
    uu_acmac_rx_handler_tb.num_sts = 0;
    uu_acmac_rx_handler_tb.is_beamformed = 0;
    uu_acmac_rx_handler_tb.partial_aid = 0;
    uu_acmac_rx_handler_tb.group_id = 0;
    uu_acmac_rx_handler_tb.is_tx_op_ps_allowed = 0;
    uu_acmac_rx_handler_tb.snr = 0;
    uu_acmac_rx_handler_tb.padding_byte = 0;
    
    $display ($time, "//---------------------------------------------------------------//");
    $display ($time, "//------------------**** TEST %0d ****-----------------------------//", i);
    $display ($time, "//---------------------------------------------------------------//");
   fork
    begin //{
      //---calling task for driving inputs 
     driving_inputs();
    end //}
    begin //{
      //---calling task for start ind 
      read_start_ind_file();
      ////---calling task for data ind 
      read_data_ind_file();
      ////---calling task for end ind 
      read_end_ind_file();
    end //}
   join
   end //}
 repeat(100) @(posedge uu_acmac_rx_handler_tb.clk);
 $finish;
end

//--------------TASK TO DRIVE INPUTS --//
 task driving_inputs();
  begin //{
        //----- driving start ind inputs 
        #1;
	uu_acmac_rx_handler_tb.rx_enable = 1;
	uu_acmac_rx_handler_tb.rx_in_frame_valid = 1;
         uu_acmac_rx_handler_tb.rx_in_ev_rxstart = 1;
         @(posedge uu_acmac_rx_handler_tb.clk);
        #1;
         uu_acmac_rx_handler_tb.rx_in_ev_rxstart = 0;
        //-------make frame in valid 0 after 20 clocks
	repeat(19)@(posedge uu_acmac_rx_handler_tb.clk);
         //--------assert data ind signal
        #1;
	     uu_acmac_rx_handler_tb.rx_in_ev_rxdata= 1;
         @(posedge uu_acmac_rx_handler_tb.clk);
        #1;
	     uu_acmac_rx_handler_tb.rx_in_ev_rxdata= 0;
	repeat(uu_acmac_rx_handler_tb.packet_length -1)@(posedge uu_acmac_rx_handler_tb.clk);
        #1;
	     uu_acmac_rx_handler_tb.rx_in_frame_valid = 0;
	     uu_acmac_rx_handler_tb.rx_in_ev_rxend = 1;
         @(posedge uu_acmac_rx_handler_tb.clk);
        #1;
	     uu_acmac_rx_handler_tb.rx_in_ev_rxend = 0;

         //--------assert check output
	     uu_acmac_rx_handler_tb.check_output = 1;
         @(posedge uu_acmac_rx_handler_tb.clk);
        #1;
	     uu_acmac_rx_handler_tb.check_output = 0;

  end //}
 endtask

//--------------TASK TO READ the start indicate file --//
task read_start_ind_file(); //{
begin //{
$display($time," **************** Reading the start indmain vector file******************");
r = $fscanf( fp , "%s %d %s %d %s %s %s %d %s %d" , testcaseid_string , testcaseid , event_type_string , event_type, filename_string, filename, umac_filter_string, umac_filter, output_string, output_value);

#1;
uu_acmac_rx_handler_tb.rx_in_lmac_filter_flag = umac_filter;


$display($time," **************** Reading the start ind vector txt file******************");
	vec = $fopen({dir, filename},"r");
	//vec = $fopen("../../rx/test/vectors/uu_acmac_start_ind_main.txt","r");
               //------reading all 30 lines in the vector text file 
               for (int i = 0; i <= 30 ; i++) 
                begin //{
                r2 = $fscanf( vec , "%s %s %d " ,array_0 ,equal_to, uu_acmac_rx_handler_tb.value);
                     //$display ("tb_rx_handler.value %d ",tb_rx_handler.value);
                 if (i==0)
                  begin
                   uu_acmac_rx_handler_tb.format =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==1)
                  begin
                   uu_acmac_rx_handler_tb.modulation =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==2)
                  begin
                   uu_acmac_rx_handler_tb.is_long_preamble =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==3)
                  begin
                   uu_acmac_rx_handler_tb.reserved1 =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==4)
                  begin
                   uu_acmac_rx_handler_tb.is_L_sigvalid =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==5)
                  begin
                   uu_acmac_rx_handler_tb.L_datarate =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==6)
                  begin
                   uu_acmac_rx_handler_tb.L_length=  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==7)
                  begin
                   uu_acmac_rx_handler_tb.is_dyn_bw =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==8)
                  begin
                   uu_acmac_rx_handler_tb.indicated_chan_bw =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==9)
                  begin
                   uu_acmac_rx_handler_tb.rssi =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==10)
                  begin
                   uu_acmac_rx_handler_tb.rcpi =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==11)
                  begin
                   uu_acmac_rx_handler_tb.is_smoothing =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==12)
                  begin
                   uu_acmac_rx_handler_tb.is_sounding =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==13)
                  begin
                   uu_acmac_rx_handler_tb.is_aggregated =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==14)
                  begin
                   uu_acmac_rx_handler_tb.is_short_GI =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==15)
                  begin
                   uu_acmac_rx_handler_tb.stbc =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==16)
                  begin
                   uu_acmac_rx_handler_tb.num_ext_ss =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==17)
                  begin
                   uu_acmac_rx_handler_tb.mcs =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==18)
                  begin
                   uu_acmac_rx_handler_tb.is_fec_ldpc_coding =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==19)
                  begin
                   uu_acmac_rx_handler_tb.rx_start_of_frame_offset =  uu_acmac_rx_handler_tb.value;
                  end
                if (i==20)
                  begin
                   uu_acmac_rx_handler_tb.rec_mcs =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==21)
                  begin
                   uu_acmac_rx_handler_tb.reserved2 =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==22)
                  begin
                   uu_acmac_rx_handler_tb.psdu_length =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==23)
                  begin
                   uu_acmac_rx_handler_tb.num_sts =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==24)
                  begin
                   uu_acmac_rx_handler_tb.is_beamformed =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==25)
                  begin
                   uu_acmac_rx_handler_tb.partial_aid =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==26)
                  begin
                   uu_acmac_rx_handler_tb.group_id =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==27)
                  begin
                   uu_acmac_rx_handler_tb.is_tx_op_ps_allowed =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==28)
                  begin
                   uu_acmac_rx_handler_tb.snr =  uu_acmac_rx_handler_tb.value;
                  end
                 if (i==29)
                  begin
                   uu_acmac_rx_handler_tb.padding_byte =  uu_acmac_rx_handler_tb.value;
                  end
           end//}
              $fclose(vec);
           for (int i =0; i <= 19 ; i++)
            begin //{
             case(i)
              5'd0 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.reserved1,uu_acmac_rx_handler_tb.is_long_preamble,uu_acmac_rx_handler_tb.modulation,uu_acmac_rx_handler_tb.format};
              5'd1 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.L_datarate,uu_acmac_rx_handler_tb.is_L_sigvalid,uu_acmac_rx_handler_tb.ch_bandwidth};
              5'd2 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.L_length[7:0]};
              5'd3 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.indicated_chan_bw,uu_acmac_rx_handler_tb.is_dyn_bw,uu_acmac_rx_handler_tb.L_length[11:8]};
              5'd4 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.rssi};
              5'd5 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.rcpi};
              5'd6 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.num_ext_ss,uu_acmac_rx_handler_tb.stbc,uu_acmac_rx_handler_tb.is_short_GI,uu_acmac_rx_handler_tb.is_aggregated,uu_acmac_rx_handler_tb.is_sounding,uu_acmac_rx_handler_tb.is_smoothing};
              5'd7 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.is_fec_ldpc_coding,uu_acmac_rx_handler_tb.mcs};
              5'd8 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.rx_start_of_frame_offset};
              5'd9 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.rx_start_of_frame_offset};
              5'd10 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.rx_start_of_frame_offset};
              5'd11 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.rx_start_of_frame_offset};
              5'd12 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.reserved2,uu_acmac_rx_handler_tb.rec_mcs};
              5'd13 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.psdu_length[7:0]};
              5'd14 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.psdu_length[15:8]};
              5'd15 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.is_beamformed,uu_acmac_rx_handler_tb.num_sts,uu_acmac_rx_handler_tb.psdu_length[19:16]};
              5'd16 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.partial_aid[7:0]};
              5'd17 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.is_tx_op_ps_allowed,uu_acmac_rx_handler_tb.group_id,uu_acmac_rx_handler_tb.partial_aid[8]};
              5'd18 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.snr};
              5'd19 : uu_acmac_rx_handler_tb.rx_in_frame = {uu_acmac_rx_handler_tb.padding_byte};
             endcase
                @(posedge uu_acmac_rx_handler_tb.clk);
              #1;
            end //}
            //  #1;
end //}

endtask //}

//--------------TASK TO READ the data indicate file --//
 task read_data_ind_file(); //{
  
$display($time," **************** Reading the data ind main vector file******************");
  r2 = $fscanf( fp2 , "%s %d %s %d %s %s %s %d " , testcaseid_string_0 , testcaseid_0 , event_type_string_0 , event_type_0, filename_string_0, data_filename, output_string_0, output_value_data_ind);
$display ($time, data_filename);
$display($time," **************** Reading the data ind vector file******************");
//------get string from data_ind vector and write to a file
fp4 =$fopen({dir, data_filename},"r");   
//fp4 =$fopen("../../rx/test/vectors/uu_acmac_data_ind_main.txt","r");
file=$fopen("../../rx/test/vectors/file.txt","w");   
r = $fgets(str, fp4);
//$display ($time, "string value is : %0s ",str);
$fwrite(file,"%0s",str);
$fclose(file);
//------open file and use
file2=$fopen("../../rx/test/vectors/file.txt","r");   
//-------reading the file for packet length --------//
   while (~uu_acmac_rx_handler_tb.not_end_of_file)begin //{
   	r4 = $fscanf(file2,"%h",uu_acmac_rx_handler_tb.byte_1);
   	if($feof(file2))begin //{
           uu_acmac_rx_handler_tb.not_end_of_file = 1'b1;
   	end //}
	else begin
   	uu_acmac_rx_handler_tb.packet_length= uu_acmac_rx_handler_tb.packet_length+1;
	end
   end //}
           $fclose(file2);
$display($time,"packet_length %d ", uu_acmac_rx_handler_tb.packet_length);

//-------------reading 2nd time
//$fclose(fp4);

//fp3 =$fopen(data_filename,"r");
fp3 = $fopen("../../rx/test/vectors/file.txt","r");   
 file_2nd = $fopen("../../rx/test/vectors/file_2nd.txt","w");
r_2nd = $fgets(str_2nd, fp3);
$fwrite(file_2nd,"%0s",str_2nd);
$fclose(file_2nd);

file2_2nd=$fopen("../../rx/test/vectors/file_2nd.txt","r");   
uu_acmac_rx_handler_tb.not_end_of_file = 1'b0;
  while (~uu_acmac_rx_handler_tb.not_end_of_file)begin //{
   	r4 = $fscanf(file2_2nd,"%h",uu_acmac_rx_handler_tb.byte_1);
        #1;
        uu_acmac_rx_handler_tb.rx_in_frame = uu_acmac_rx_handler_tb.byte_1;
   	if($feof(file2_2nd))begin //{
           uu_acmac_rx_handler_tb.not_end_of_file = 1'b1;
   	end //}
    @(posedge uu_acmac_rx_handler_tb.clk);
   end //}

 endtask //}

//--------------TASK TO READ the end indicate file --//
task read_end_ind_file(); 
 begin //{
   r3 = $fscanf( fp5 , "%s %d %s %d %s %s %s %d " ,testcaseid_string_1,testcaseid_1,event_type_string_1, event_type_1, filename_string_1, end_ind_file, output_string_1, uu_acmac_rx_handler_tb.output_value_end);
 end //}
endtask

////////////////////////////////////////////////////////////////////////////////////
//-----------CHECKER -------------------//
 always @ (*)
  begin
   if (uu_acmac_rx_handler_tb.check_output)
    begin
   if (uu_acmac_rx_handler_tb.output_value_end == uu_acmac_rx_handler_tb.rx_out_cap_return_value) 
    begin
    $display(" PASS: RETURN VALUE MATCH : %d %d\n", uu_acmac_rx_handler_tb.output_value_end, uu_acmac_rx_handler_tb.rx_out_cap_return_value );
    $display ($time,"************************************************************  ");
    $display ($time,"*************** PASS RETURN VALUE MATCHED ******************  ");
    $display ($time,"************************************************************  ");
    end
   else
    begin
    count++;
    $display(" ERROR: RETURN VALUE MISMATCH : %d %d\n", uu_acmac_rx_handler_tb.output_value_end, uu_acmac_rx_handler_tb.rx_out_cap_return_value );
    $display(" ERROR COUNT: %d\n", count );
    end
   end
  end

endmodule

`timescale 1ns/1ns
`include "../../inc/defines.h"
module uu_acmac_cp_rx_compba_under_agg_imp_test;

int 		sym_len=7;
int 		i;

reg [3:0] rate;
reg [1:0]  format;
reg [2:0]  modulation;
reg [6:0]  mcs;

reg [7:0] byte_25;
reg [7:0] byte_17;

reg [7:0] addr_0;
reg [7:0] addr_1;
reg [7:0] addr_2;
reg [7:0] addr_3;
reg [7:0] addr_4;
reg [7:0] addr_5;
reg [3:0] tid;
reg [2:0] ba_type;
reg       cp_in_ev_rxdata_ind;

integer add_ba;
integer add_ba_scan;
integer fp;
integer fp1;
integer fp2;
integer file;
integer file2;
integer file_2nd;
integer file2_2nd;
integer r;
integer r_2nd;
integer r1 ;
integer r2;
integer rq_type;
integer rq_type_addba;
integer is_aggregated;
integer ac_g; 
integer cts_g;
integer rx_end_ind;
string array_0; 
string array_0_addba; 
reg [15:0] seq_num;
reg [15:0] buff_size;
reg dir;
reg clk;
string array_1; 
string array_1_addba; 
string array_2_addba; 
string array_3_addba; 
string array_4_addba; 
string frame_input_file, array_2, array_3,array_4_output, array_5_return;
assign uu_acmac_cp_tb.mac_addr = {addr_5,addr_4,addr_3,addr_2,addr_1,addr_0};
integer ii;
reg not_end_of_file = 0;
	 reg [1023:0]str;
	 reg [9023:0]array_10;
	 reg [1023:0]str_2nd;

	//Add BA
	task addBA;
	begin//{
            //----------open add_ba test file            
            add_ba =$fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/uu_acmac_addba.txt","r");   
            //----------scan add_ba test file            
            add_ba_scan = $fscanf( add_ba , "%s %d %s %h %h %h %d %d %h %h %h %h %h %h %s %s %s %h" , array_0_addba , rq_type_addba , array_1_addba , buff_size, tid, ba_type, seq_num, dir, addr_0, addr_1,addr_2,addr_3,addr_4, addr_5,array_2_addba, array_3_addba ,array_4_addba, uu_acmac_cp_tb.return_value_addba);

           uu_acmac_cp_tb.win_start  = (seq_num & `MPDU_SEQN_MASK );
           uu_acmac_cp_tb.win_size = buff_size; 
           uu_acmac_cp_tb.win_end = (uu_acmac_cp_tb.win_start + uu_acmac_cp_tb.win_size - 1); 
           uu_acmac_cp_tb.is_used = 1'd1;

           uu_acmac_cp_tb.ba_memory[0] = {29'd0,ba_type};
		 for(ii=1;ii<=32;ii++)
		 	uu_acmac_cp_tb.ba_memory[ii] = ii;
		 uu_acmac_cp_tb.ba_memory[33] = 0;
		 for(ii=1;ii<=32;ii++)
		 	uu_acmac_cp_tb.ba_memory[ii+33] = ii;
		 uu_acmac_cp_tb.ba_memory[66] = {uu_acmac_cp_tb.win_start,16'd0};
		 uu_acmac_cp_tb.ba_memory[67] = {uu_acmac_cp_tb.win_size,uu_acmac_cp_tb.win_end};
        
	end//}
	endtask

	//Del BA
	task delBA;
	begin//{
		 for(ii=0;ii<=67;ii++)
		 	uu_acmac_cp_tb.ba_memory[ii] = 0;
	end//}
	endtask

	//----------------Reading of main file ----------------------//
	task read_line();
	//---------reading main file-----------//
	$display($time," **************** Reading a vector from file******************");

	r = $fscanf( fp , "%s %d %s %d %d %d %d %d %d %d %s %d %s %s %s %s %h" , array_0 , rq_type , array_1 , format, rate, is_aggregated, modulation, mcs, ac_g, cts_g,frame_input_file,rx_end_ind,array_2, array_3,array_4_output, array_5_return,uu_acmac_cp_tb.return_value);

	//------get string and write to a file
	file=$fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/file.txt","w");   
	r = $fgets(str, fp1);
	$fwrite(file,"%0s",str);
	$fclose(file);
	//------open file and use
	file2=$fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/file.txt","r");   
	//-------reading the file for packet length --------//
	   while (~not_end_of_file)begin //{
		r2 = $fscanf(file2,"%h",uu_acmac_cp_tb.byte_1);
		if($feof(file2))begin //{
			 not_end_of_file = 1'b1;
		end //}
		else begin
		uu_acmac_cp_tb.packet_length= uu_acmac_cp_tb.packet_length+1;
		end
	   end //}
			 $fclose(file2);
	endtask//read_line


	//process Rx_CP
	task proc_Rx;
	begin//{
		for(int i=1; i<=sym_len; i++) begin //{
			uu_acmac_cp_tb.packet_length = 0;
                        uu_acmac_cp_tb.sym_no = i;
			not_end_of_file = 0;
			uu_acmac_cp_tb.rx_frame_info_mpdu_0 = 0;
			uu_acmac_cp_tb.txmtd_ra = 48'd0;
			uu_acmac_cp_tb.rcvd_ra = 48'd0;
			uu_acmac_cp_tb.rcvd_resp = 0;
			$display ($time, "//---------------------------------------------------------------//");
			$display ($time, "//------------------**** TEST %d ****-----------------------------//", i);
			$display ($time, "//---------------------------------------------------------------//");
			read_line();
			#1;
			//--------driving data 
			fork
				begin
					read_input_frame;
				end
				begin
					driving_inputs;
				end
			join 
			@(negedge uu_acmac_cp_tb.check_output[19]);
		end	
	end//}
	endtask//proc_Rx

	task read_input_frame();
	begin
		file_2nd = $fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/file_2nd.txt","w");
		r_2nd = $fgets(str_2nd, fp2);	
		$fwrite(file_2nd,"%0s",str_2nd);	
		$fclose(file_2nd);	
		file2_2nd=$fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/file_2nd.txt","r");   	
		wait(cp_in_ev_rxdata_ind);
		begin //{
			for(int j=0; j <= uu_acmac_cp_tb.packet_length; j++) begin //{
				uu_acmac_cp_tb.mem_rx_in_addr2 = j+8'd36; 
				uu_acmac_cp_tb.mem_rx_in_wen = 1; 
				r1 = $fscanf(file2_2nd,"%h",uu_acmac_cp_tb.byte_1);
				uu_acmac_cp_tb.mem_rx_in_data = uu_acmac_cp_tb.byte_1;
				if ( j == 0)
				begin
					uu_acmac_cp_tb.rx_frame_info_mpdu_0 = uu_acmac_cp_tb.byte_1; 
				end
                                if (j == 17) 
                                 begin
                                    byte_17 = uu_acmac_cp_tb.byte_1;
                                 end
                                if (j == 25) 
                                 begin
                                    byte_25 = uu_acmac_cp_tb.byte_1;
                                 end
				if (j == 4)
				begin
					uu_acmac_cp_tb.txmtd_ra[7:0] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 5)
				begin
					uu_acmac_cp_tb.txmtd_ra[15:8] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 6)
				begin
					uu_acmac_cp_tb.txmtd_ra[23:16] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 7)
				begin
					uu_acmac_cp_tb.txmtd_ra[31:24] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 8)
				begin
					uu_acmac_cp_tb.txmtd_ra[39:32] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 9)
				begin	
					uu_acmac_cp_tb.txmtd_ra[47:40] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 10)
				begin
					uu_acmac_cp_tb.txmtd_ta[7:0] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 11)	
				begin
					uu_acmac_cp_tb.txmtd_ta[15:8] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 12)
				begin
					uu_acmac_cp_tb.txmtd_ta[23:16] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 13)
				begin
					uu_acmac_cp_tb.txmtd_ta[31:24] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 14)
				begin
					uu_acmac_cp_tb.txmtd_ta[39:32] = uu_acmac_cp_tb.byte_1;
				end
				if (j == 15)
				begin
					uu_acmac_cp_tb.txmtd_ta[47:40] = uu_acmac_cp_tb.byte_1;
				end
				uu_acmac_cp_tb.count = j;
				@(posedge uu_acmac_cp_tb.clk);
				#1;
			end //}
			$fclose(file2_2nd);
		end //}  
		uu_acmac_cp_tb.mem_rx_in_wen = 0; 
	end
	endtask //read_input_frame

	//------------------------------------------------------//
	//---------------- DRIVING INPUTS TASK------------------//
	//------------------------------------------------------//
	task driving_inputs;
	begin //{
		uu_acmac_cp_tb.cp_en = 1;
		fork
			begin //{
				uu_acmac_cp_tb.cp_in_is_rxend = rx_end_ind;
				uu_acmac_cp_tb.cp_in_waiting_for_cts= cts_g;
				uu_acmac_cp_tb.cp_in_waiting_for_ack= ac_g;
				
				@(posedge uu_acmac_cp_tb.clk);
				#1;
			end //}
			begin //{
				for(int i=0; i <= 19; i++) begin //{
					uu_acmac_cp_tb.mem_rx_in_en2 = 1;
					uu_acmac_cp_tb.mem_rx_in_wen = 1'b1;
					uu_acmac_cp_tb.mem_rx_in_addr2 = i;
					case (i)	
					6'd0 :uu_acmac_cp_tb.mem_rx_in_data = {3'd0,modulation,format}   ;
					6'd1 :uu_acmac_cp_tb.mem_rx_in_data = {rate,4'd0};   
					6'd2 :uu_acmac_cp_tb.mem_rx_in_data= 0;   
					6'd3 :uu_acmac_cp_tb.mem_rx_in_data= 0;   
					6'd4 :uu_acmac_cp_tb.mem_rx_in_data= 0;   
					6'd5 :uu_acmac_cp_tb.mem_rx_in_data= 0;   
					6'd6 :uu_acmac_cp_tb.mem_rx_in_data= {5'd0,is_aggregated,2'd0};   
					6'd7 :uu_acmac_cp_tb.mem_rx_in_data= {1'd0,mcs};   
					6'd8 :uu_acmac_cp_tb.mem_rx_in_data= 0;   
					6'd9 :uu_acmac_cp_tb.mem_rx_in_data= 0;   
					6'd10 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd11 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd12 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd13 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd14 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd15 :uu_acmac_cp_tb.mem_rx_in_data = 0;   		    
					6'd16 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd17 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd18 :uu_acmac_cp_tb.mem_rx_in_data = 0;   
					6'd19 :uu_acmac_cp_tb.mem_rx_in_data = 0;  
					endcase		
					@(posedge uu_acmac_cp_tb.clk);
					#1;
				end //}
				uu_acmac_cp_tb.mem_rx_in_wen = 1'b0;
			end //}
		join
		//---assert data indicate
		#1;
		cp_in_ev_rxdata_ind = 1;
		@(posedge uu_acmac_cp_tb.clk);
		#1;
		cp_in_ev_rxdata_ind = 0;
		repeat(uu_acmac_cp_tb.packet_length)@(posedge uu_acmac_cp_tb.clk);
		#1;
		uu_acmac_cp_tb.mem_rx_in_addr2 = 0;
		uu_acmac_cp_tb.cp_in_process_rx_frame = 1;
		@(posedge uu_acmac_cp_tb.clk);
		#1;
		uu_acmac_cp_tb.cp_in_process_rx_frame = 0;
		
		@(posedge uu_acmac_cp_tb.clk);
	end//}
	endtask//driving_inputs


	
//------------------------------------------------------//
//-----------------------CHECKERS-----------------------//
//------------------------------------------------------//

 //----------Compute the FC value-----------//
 always @(posedge cp_in_ev_rxdata_ind)
 begin  //{
 #1;
  if( ( ((uu_acmac_cp_tb.rx_frame_info_mpdu_0 & `IEEE80211_FC0_TYPE_MASK) == `IEEE80211_FC0_TYPE_MGT) || 
        ((uu_acmac_cp_tb.rx_frame_info_mpdu_0 & `IEEE80211_FC0_TYPE_MASK)== `IEEE80211_FC0_TYPE_RESERVED) ) && 
       (uu_acmac_cp_tb.rx_frame_info_mpdu_0 != (`IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK | `IEEE80211_FC0_TYPE_MGT)))
     begin
 #1;
     uu_acmac_cp_tb.FC = (uu_acmac_cp_tb.rx_frame_info_mpdu_0 & `IEEE80211_FC0_TYPE_MASK); 
     $display ( $time, "rx_frame_info_mpdu_0", uu_acmac_cp_tb.rx_frame_info_mpdu_0);
     $display ( $time, "ENTERED IF CONDITION");
     end
   else
     begin
       uu_acmac_cp_tb.FC = uu_acmac_cp_tb.rx_frame_info_mpdu_0;
       $display ( $time, "ENTERED ELSE CONDITION");
     end
   end //}

  //-----CHECK the outputs ----------//
  always @ (*)
   begin //{
       //------Check return value --------// 
        if (uu_acmac_cp_tb.cp_out_rx_res_en)
         begin //{
          if (uu_acmac_cp_tb.cp_out_rx_res== uu_acmac_cp_tb.return_value) 
           begin //{
             $display ("                  *********************RETURN VALUE MATCHED TO RESPONSE*************");
             begin //{
              #1;
             $display ($time,"CHECKING OUTPUTS");
             //------ Case -1
             if (uu_acmac_cp_tb.FC == (`IEEE80211_FC0_SUBTYPE_RTS | `IEEE80211_FC0_TYPE_CTL))  
              begin //{
                     $display ("ENTERED CASE 1");
                if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                 begin //{
                   if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_CTS | `IEEE80211_FC0_TYPE_CTL ) )
                    begin //{
                      $display ($time,"************************************************************  ");
                      $display ($time,"*********************CASE 1 CTS PASS ***********************  ");
                      $display ($time,"************************************************************  ");
                    end //}
                   else
                    begin //{
                     $display(" ERROR: FC MISMATCH to IEEE80211_FC0_SUBTYPE_CTS or IEEE80211_FC0_SUBTYPE_CTS: %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_CTS,`IEEE80211_FC0_TYPE_CTL );
                    end //}
                 end //}
                 else 
                   begin //{
                     $display ("CASE 1 NOT EXECUTED");
                   end //}
                 
              end //}

             //------ Case -2
                else if ( uu_acmac_cp_tb.FC == (`IEEE80211_FC0_SUBTYPE_CTS | `IEEE80211_FC0_TYPE_CTL) )
                 begin //{
                      $display ($time,"************************************************************  ");
                      $display ("********************************* CASE 2 CTS FRAME PASS ********************** ");
                      $display ($time,"************************************************************  ");
                 end //}

             //------ Case -3
                else if ( uu_acmac_cp_tb.FC == (`IEEE80211_FC0_SUBTYPE_ACK | `IEEE80211_FC0_TYPE_CTL) )
                 begin //{
                      $display ($time,"************************************************************  ");
                      $display ("********************************* CASE 3  ACK PASS *********************  ");
                      $display ($time,"************************************************************  ");
                 end //}

             //------ Case -4
                else if ( uu_acmac_cp_tb.FC == (`IEEE80211_FC0_SUBTYPE_BAR | `IEEE80211_FC0_TYPE_CTL) )
                 begin //{
                  if (uu_acmac_cp_tb.cp_out_rx_res == `UU_BA_SESSION_INVALID )
                   begin //{
                      $display ("*********************************** PASS Received BAR frame case is ba invalid ********* : ",i      );
                   end //}
                  else if (uu_acmac_cp_tb.cp_out_rx_res == `UU_WLAN_RX_HANDLER_SEND_RESP)
                   begin //{
                     if ( ({1'd0,byte_17[0]}) == `UU_WLAN_ACKPOLICY_NORMAL) //FIXME write logic for byte 17 
                      begin //{
                        if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                         begin //{
                           if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_ACK | `IEEE80211_FC0_TYPE_CTL ) )
                            begin //{
                              $display ("************************************ PASS ****************************** : ",i      );
                            end //}
                           else
                            begin //{
                              $display(" ERROR: CASE 4 FC MISMATCH to IEEE80211_FC0_SUBTYPE_ACK or IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_ACK,`IEEE80211_FC0_TYPE_CTL);
                            end //}
                         end //}
                      end //}
                     else
                      begin //{
                       if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                         begin //{
                           if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_BA | `IEEE80211_FC0_TYPE_CTL ) )
                            begin //{
                              $display ("**************************************** PASS ************************  : ",i      );
                            end //}
                           else
                            begin //{
                              $display(" ERROR: CASE 4 FC MISMATCH to IEEE80211_FC0_SUBTYPE_BA or IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_BA,`IEEE80211_FC0_TYPE_CTL);
                            end //}
                         end //}
                      end //}
                   end //}
                  else
                    begin // {
                    $display("           ERROR: RES VALUE FAIL CASE 4 : %d %h\n", i,uu_acmac_cp_tb.cp_out_rx_res);
                    end //}
                 end //}

             //------ Case -5
                else if ( uu_acmac_cp_tb.FC == (`IEEE80211_FC0_SUBTYPE_BA | `IEEE80211_FC0_TYPE_CTL) )
                 begin //{
                  if (uu_acmac_cp_tb.cp_out_rx_res ==  `UU_WLAN_RX_HANDLER_FRAME_INVALID)
                   begin //{
                              $display (" ******************************************* PASS ******************* : ",i      );
                   end //}
                  else if (uu_acmac_cp_tb.cp_out_rx_res ==  `UU_BA_SESSION_INVALID)
                    begin //{
                              $display ("  ********************************PASS Received BA frame case is ba invalid  : ",i      );
                    end //}
                  else if (uu_acmac_cp_tb.cp_out_rx_res ==  `UU_WLAN_RX_HANDLER_BA_RCVD)
                    begin //{
                              $display ("  PASS Received BA frame case ba ack policy is 1 : ",i      );
                    end //}
                  else if (uu_acmac_cp_tb.cp_out_rx_res ==  `UU_WLAN_RX_HANDLER_SEND_RESP) 
                    begin //{
                      if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                         begin //{
                           if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_ACK| `IEEE80211_FC0_TYPE_CTL ) )
                            begin //{
                              $display (" ********************************  PASS  : ",i      );
                            end //}
                           else
                            begin //{
                              $display(" ERROR: FC MISMATCH to IEEE80211_FC0_SUBTYPE_ACK or IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_ACK,`IEEE80211_FC0_TYPE_CTL);
                            end //}
                         end //}
                    end //}
                   else 
                    begin //{
                      $display("           ERROR: RES VALUE FAIL CASE 5 : %d %h\n", i,uu_acmac_cp_tb.cp_out_rx_res);
                    end //}

                 end //}
          
             //------ Case -6
                else if ( uu_acmac_cp_tb.FC == (`IEEE80211_FC0_SUBTYPE_QOS_NULL | `IEEE80211_FC0_TYPE_DATA) )
                 begin //{
                   if ( (byte_25[6:5]) ==  `UU_WLAN_ACKPOLICY_NORMAL)
                    begin //{
                      if (is_aggregated)
                        begin //{
                           if (rx_end_ind)  
                               begin //{
                                 if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                                  begin //{
                                   if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_BA | `IEEE80211_FC0_TYPE_CTL ) )
                                    begin //{
                                      $display ("********************************    PASS  : ",i      );
                                    end //}
                                   else
                                     begin //{
                                        $display(" ERROR: FC MISMATCH to IEEE80211_FC0_SUBTYPE_BA or IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_BA,`IEEE80211_FC0_TYPE_CTL);
                                     end //}
                                   end //}
                               end //}
                           else
                             begin //{
                               if (uu_acmac_cp_tb.cp_out_rx_res ==  `UU_BA_SESSION_INVALID)
                                 begin //{
                                  $display ("  PASS Received implicit QOS frame with ba invalid rx end indication is 0 : ",i      );
                                 end //}
                               else if ((uu_acmac_cp_tb.cp_out_rx_res == `UU_SUCCESS)  || (uu_acmac_cp_tb.cp_out_rx_res == `UU_FAILURE))
                                 begin //{
                                  $display ("  PASS Received implicit QOS frame with rx end indication is 0 : ",i      );
                                 end //}
                               else
                                 begin //{
                                  $display("           ERROR: RES VALUE FAIL CASE 6 : %d %h\n", i,uu_acmac_cp_tb.cp_out_rx_res);
                                 end //}
                             end //}
                        end //}
                      else
                       begin //{
                           if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                            begin //{
                             if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_ACK | `IEEE80211_FC0_TYPE_CTL ) )
                              begin //{
                                $display (" ******************************** PASS  : ",i      );
                              end //}
                             else
                               begin //{
                                  $display(" ERROR: FC MISMATCH to IEEE80211_FC0_SUBTYPE_ACKor IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_ACK,`IEEE80211_FC0_TYPE_CTL);
                               end //}
                             end //}
                       end //}
                    end //}
                  else if ((byte_25[6:5]) ==  `UU_WLAN_ACKPOLICY_NO_ACK)
                    begin //{
                       $display ("  PASS Received Frame with ACK POLICY of NO ACK : ",i      );
                    end //}
                  else if ((byte_25[6:5]) ==  `UU_WLAN_ACKPOLICY_PSMP_ACK)
                    begin //{
                       $display ("  PASS Received Frame with ACK POLICY of NO ACK : ",i      );
                    end //}
                  else
                    begin //{
                      if (uu_acmac_cp_tb.cp_out_rx_res == `UU_BA_SESSION_INVALID)
                        begin //{
                             $display ("  PASS Received QOS under ba frame case is ba invalid : ",i      );
                        end //} 
                       else if ((uu_acmac_cp_tb.cp_out_rx_res == `UU_SUCCESS) || (uu_acmac_cp_tb.cp_out_rx_res == `UU_FAILURE)) 
                         begin //{
                            $display ("  PASS Received QOS under ba: ",i      );
                         end //}
                       else
                        begin //{
                            $display ("  ERROR FAIL CASE 6 : ",i      );
                        end //}
                    end //}
                 end //}

             //------ Case -7
                else if ( uu_acmac_cp_tb.FC == (`IEEE80211_FC0_TYPE_DATA | `IEEE80211_FC0_SUBTYPE_DATA_NULL) )
                 begin //{
                    if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                     begin //{
                      if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_ACK | `IEEE80211_FC0_TYPE_CTL ) )
                       begin //{
                         $display ("******************************** PASS  : ",i      );
                       end //}
                      else
                        begin //{
                           $display(" ERROR: CASE 7 FC MISMATCH to IEEE80211_FC0_SUBTYPE_ACK or IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_ACK,`IEEE80211_FC0_TYPE_CTL);
                        end //}
                      end //}
                 end //}

             //------ Case -8
                else if ( uu_acmac_cp_tb.FC == `IEEE80211_FC0_TYPE_MGT )
                 begin //{
                    if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                     begin //{
                      if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_ACK | `IEEE80211_FC0_TYPE_CTL ) )
                       begin //{
                         $display ("******************************** PASS  : ",i      );
                       end //}
                      else
                        begin //{
                           $display(" ERROR:CASE 8 FC MISMATCH to IEEE80211_FC0_SUBTYPE_ACK or IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_ACK,`IEEE80211_FC0_TYPE_CTL);
                        end //}
                      end //}
                 end //}

             //------ Case -9
                else if ( uu_acmac_cp_tb.FC == `IEEE80211_FC0_TYPE_RESERVED)
                 begin //{
                    if (uu_acmac_cp_tb.rcvd_ra == uu_acmac_cp_tb.txmtd_ta ) 
                     begin //{
                      if (uu_acmac_cp_tb.rcvd_fc ==  (`IEEE80211_FC0_SUBTYPE_BA | `IEEE80211_FC0_TYPE_CTL ) )
                       begin //{
                         $display ("******************************** PASS  : ",i      );
                       end //}
                      else
                        begin //{
                           $display(" ERROR:CASE 9 FC MISMATCH to IEEE80211_FC0_SUBTYPE_BA or IEEE80211_FC0_TYPE_CTL : %d %h %h\n", i,uu_acmac_cp_tb.FC,`IEEE80211_FC0_SUBTYPE_BA,`IEEE80211_FC0_TYPE_CTL);
                        end //}
                      end //}
                 end //}

             //------ Case -10
                else if (uu_acmac_cp_tb.FC == (`IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK | `IEEE80211_FC0_TYPE_MGT))
                  begin
                    $display ("******************************** CASE 10 PASS  : ",i      );
                  end

           end //}
         end //} 
          else
           $display("               ERROR: RETURN VALUE FAIL : %h %h\n", uu_acmac_cp_tb.cp_out_rx_res,uu_acmac_cp_tb.return_value);
          end //} 

  end //}
//--------------CHECK OUTPUT --------------// 
always @ (posedge uu_acmac_cp_tb.clk or negedge uu_acmac_cp_tb.rst_n)
 begin
  if (!uu_acmac_cp_tb.rst_n)
   uu_acmac_cp_tb.check_output_int <= 1'd0;
  else if (!uu_acmac_cp_tb.cp_out_rx_res_en) 
   uu_acmac_cp_tb.check_output_int <= 1'd0;
  else if (uu_acmac_cp_tb.cp_out_rx_res_en)
   begin
   uu_acmac_cp_tb.check_output_int <= 1'd1;
   uu_acmac_cp_tb.rcvd_resp <= uu_acmac_cp_tb.cp_out_rx_res;
   end
 end
always @ (posedge uu_acmac_cp_tb.clk)
 begin
  uu_acmac_cp_tb.check_output <= {uu_acmac_cp_tb.check_output[151:0],uu_acmac_cp_tb.check_output_int}; 
 end
//------------------------------------------------------//
//------------Storing txmtd and received values -------//
//------------------------------------------------------//
initial
 begin
    for(int i=1; i<=sym_len; i++) begin
     @(negedge uu_acmac_cp_tb.clk);
     wait(uu_acmac_cp_tb.cp_out_crc_data_val && uu_acmac_cp_tb.cp_out_crc_en) ;
       uu_acmac_cp_tb.rcvd_fc[7:0] = uu_acmac_cp_tb.cp_out_crc_data;
       @(posedge uu_acmac_cp_tb.clk);
       @(posedge uu_acmac_cp_tb.clk);
       @(posedge uu_acmac_cp_tb.clk);
       @(posedge uu_acmac_cp_tb.clk);
    for (int j=0; j<=5; j++) begin
       @(posedge uu_acmac_cp_tb.clk);
     if (j == 0)
       begin
       uu_acmac_cp_tb.rcvd_ra[7:0] = uu_acmac_cp_tb.cp_out_crc_data;
        end
      if (j == 1)
       begin
       uu_acmac_cp_tb.rcvd_ra[15:8] = uu_acmac_cp_tb.cp_out_crc_data;
        end
      if (j == 2)
       begin
       uu_acmac_cp_tb.rcvd_ra[23:16] = uu_acmac_cp_tb.cp_out_crc_data;
        end
      if (j == 3)
       begin
       uu_acmac_cp_tb.rcvd_ra[31:24] = uu_acmac_cp_tb.cp_out_crc_data;
        end
      if (j == 4)
       begin
       uu_acmac_cp_tb.rcvd_ra[39:32] = uu_acmac_cp_tb.cp_out_crc_data;
        end
      if (j == 5)
       begin
       uu_acmac_cp_tb.rcvd_ra[47:40] = uu_acmac_cp_tb.cp_out_crc_data;
        end
     end
       @(posedge uu_acmac_cp_tb.clk);
    end
 end



	initial begin
		repeat(20)@(uu_acmac_cp_tb.clk);
		fp=$fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/uu_acmac_test_files.txt","r");   
		fp1=$fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/uu_acmac_input_frames.txt","r");   
		fp2=$fopen("../../cp/test/vectors/uu_acmac_ba_rx_path_of_cp/uu_acmac_rx_compba_under_agg_imp/uu_acmac_input_frames.txt","r");  
		addBA();
		proc_Rx();
		delBA();
		repeat(100)@(uu_acmac_cp_tb.clk);
		$finish;
	end

endmodule

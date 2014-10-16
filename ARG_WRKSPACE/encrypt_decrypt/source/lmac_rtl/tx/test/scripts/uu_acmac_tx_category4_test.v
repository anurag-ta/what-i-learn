`timescale 1ns/1ns
`include "../../inc/defines.h"

// The Test for Test Category 4

module uu_acmac_tx_category4_test();


reg  [8*31:0] string0, string1, string2, string3, string4, string5, string6, string7, string8, string9;
string        vector_file;
string        data_file;
string        rts_file;
reg           neof;
integer       testcaseid;
integer       testcategory;
integer       nav;
integer       duration;
integer       qos;
integer       ret;
reg [1 :0]    format;      
reg [2 :0]    ch_bndwdth;  
reg [2 :0]    n_tx;        
reg [6 :0]    txpwr_level; 
reg [3 :0]    L_datarate;  
reg [11:0]    L_length;    
reg [15:0]    service;     
reg [7 :0]    antenna_set; 
reg [1 :0]    num_ext_ss;  
reg           is_no_sig_extn;     
reg [1 :0]    ch_offset;          
reg [2 :0]    modulation;         
reg [6 :0]    mcs;                
reg           is_long_preamble;   
reg [15:0]    ht_length;          
reg           is_smoothing;       
reg           is_sounding;        
reg           is_aggregated;      
reg [1 :0]    stbc;               
reg           is_fec_ldpc_coding; 
reg           is_short_GI;        
reg [8 :0]    partial_aid;        
reg           is_beamformed;      
reg [1 :0]    num_users;          
reg           is_dyn_bw;              
reg [2 :0]    indicated_chan_bw;      
reg [5 :0]    group_id;               
reg           is_txop_psallowed;     
reg           is_time_of_departure_req;
reg [1 :0]    user_position;          
reg [2 :0]    num_sts;                
reg [19:0]    apep_length;            
reg [3 :0]    vht_mcs;                
reg [19:0]    framelen;               
reg           is_ampdu;               
reg           retry;                  
reg [3 :0]    ShortRetryCount;        
reg [3 :0]    LongRetryCount;         
reg [1 :0]    keytype;         
reg [5 :0]    keyix;           
reg [31:0]    timestamp;       
reg [15:0]    desc_id;         
reg [1 :0]     ac;              
reg [1 :0]    stbc_fb;         
reg [2 :0]    n_tx_fb;         
reg [6 :0]    mcs_fb;          
reg [3 :0]    fallback_rate;   
reg [3 :0]    rtscts_rate;     
reg [3 :0]    rtscts_rate_fb;  
reg [7 :0]    aggr_count;      
reg [31:0]    num_sts_fb;      
integer       err_count = 0;
integer       fp;
integer       byte_count;
reg     [19:0] buf_ptr;
reg     [19:0] start_addr;
reg           stask;
string dir = "../../tx/test/vectors/";

// Initial Block for Reading the Test File.
initial 
begin
    repeat (40) @(posedge uu_acmac_tx_tb.clk);

    // Initializing the Registers.
    neof = 1'b0;
    buf_ptr = 20'h0;
    start_addr = 20'h0;

    // Reading the Test File
    // testcaseid: 2 testcategory: 4 input: frame_info: tx_vector.txt frame: beacon.txt rts_cts_frame: rts_frame1.txt nav: 1000 duration: 100 qos: 1 output: 0
    fp = $fopen("../../tx/test/vectors/testcategory4.txt", "r");

    while(~neof)
    begin
        stask = $fscanf(fp, "%s %d %s %d %s %s %s %s %s %s %s %s %d %s %d %s %d %s %d", string0, testcaseid, string1, testcategory, string2, string3, vector_file, string4, data_file, string5, rts_file, string6, nav, string7, duration, string8, qos, string9, ret);

        if( $feof(fp) )
            neof = 1'b1;

        if(!neof)
        begin

        // Set the default values for Parameters
        set_params;
 
        // Reading the First Vector File
        read_vector_file;

        // Writing the Control Frame to the Control Memory
        write_ctrl_packet_tomem(rts_file);

        // Writing the Tx Vector and Packet to Memory
        write_txvector_packet_tomem(data_file);
    
        // Drive the Packet
        test_packet_transmit;

        // Packet Status
        packet_status;

	   uu_acmac_tx_tb.tx_in_clear_mpdu = 1'b1;
        @(negedge uu_acmac_tx_tb.clk);
	   uu_acmac_tx_tb.tx_in_clear_mpdu = 1'b0;

        repeat(20) @(negedge uu_acmac_tx_tb.clk);
        end   
    end
    $fclose(fp);
    $finish();
end

// Task to Read Number of Parameters in the Vector file
task read_vector_num( output [5:0] val );
    reg [8*15:0] param;
    reg [7:0]    equal;
    reg [7:0]    value;
    reg          neof;
    reg [5:0]    count;
    integer      vp;
begin
    neof = 1'b0;
    count = 6'h0;
    vp = $fopen({dir, vector_file}, "r");
    while(~neof)
    begin
        stask = $fscanf(vp, "%s %s %d", param, equal, value);
        
        if($feof(vp))
            neof = 1'b1;
        else
            count = count + 6'h1;
    end
   
    $fclose(vp);

    val = count;
end
endtask

// Task to Read the Length of the Packet from the file
task read_packet_len( string file, output [19:0] packet_len );
    integer    rp;
    reg [7:0]  value;
    reg        neof;
begin 
    neof = 1'b0;
    packet_len  = 20'h0;

    rp = $fopen({dir,file}, "r");
    while(~neof)
    begin
        stask = $fscanf(rp, "%h", value);

        if($feof(rp))
            neof = 1'b1;
        else
            packet_len = packet_len + 20'h1;
    end

    $fclose(rp);
end
endtask

// Task to set the default values
task set_params;
begin    
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_en = 1'b1;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_in_QOS_mode = qos;
    uu_acmac_tx_tb.tx_in_RTS_Threshold = 10;
    uu_acmac_tx_tb.tx_in_SIFS_timer_value = 16;
    uu_acmac_tx_tb.tx_in_cap_ch_bw = 0;   
    uu_acmac_tx_tb.tx_in_cap_ac = 0;    
    uu_acmac_tx_tb.tx_in_cap_ac0_tx_nav = nav;    
    uu_acmac_tx_tb.tx_in_cap_ac1_tx_nav = 0;    
    uu_acmac_tx_tb.tx_in_cap_ac2_tx_nav = 0;    
    uu_acmac_tx_tb.tx_in_cap_ac3_tx_nav = 0;    
    @(negedge uu_acmac_tx_tb.clk);
end
endtask

// Task to Transmit the TX Vector
task txop_txstart_transmit;
begin
    // The TX Vector Start Request Pulse to DUT
    uu_acmac_tx_tb.tx_in_cap_start_req = 0;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_in_cap_start_req = 1;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_in_cap_start_req = 0;
    
    // Response for The TX Vector Start from DUT 
    wait(uu_acmac_tx_tb.tx_out_cap_res_val);
    if(uu_acmac_tx_tb.tx_out_cap_res == `UU_FAILURE)
    begin
    	$display("ERROR: Unable to send TX VECTOR!");
        err_count = err_count + 1;
    end
    else
    begin
	$display("INFO: TX VECTOR sent successfully!");

        repeat(5) @(negedge uu_acmac_tx_tb.clk);

        // Transmit the TX Data
        tx_phy_data_transmit;
        repeat(5) @(negedge uu_acmac_tx_tb.clk);

        // Transmit the Send More Data Request
        tx_send_more_data_transmit;
    end
end
endtask

// Always Block to count the Bytes from the Tx Module
always @(posedge uu_acmac_tx_tb.clk)
begin
    if(uu_acmac_tx_tb.tx_in_cap_data_req)
       byte_count <= 'h0;
    else if(uu_acmac_tx_tb.tx_out_phy_frame_val)
       byte_count <= byte_count + 'h1;
end

// Task to Transmit the TX Data
task tx_phy_data_transmit;
begin
    // The TX Data Request to the DUT 
    uu_acmac_tx_tb.tx_in_cap_data_req = 1'b0;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_in_cap_data_req = 1'b1;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_in_cap_data_req = 1'b0;

    // Response for the TX Data from the DUT
    if(uu_acmac_tx_tb.LMAC_TX.loc_reg_is_ctrl_frame) 
    begin
        wait(byte_count == 'hD);
    end
    else
    begin
        wait(byte_count == (uu_acmac_tx_tb.tx_out_buf_pkt_len - 'h1));
    end

    if(uu_acmac_tx_tb.tx_out_cap_res == `UU_FAILURE) 
        $display("ERROR: Unable to send TX DATA!");
    else
        $display("INFO: TX DATA sent successfully!");
    repeat(10) @(negedge uu_acmac_tx_tb.clk);
end
endtask

// TX Send More Data Request to the DUT
task tx_send_more_data_transmit;
begin
    uu_acmac_tx_tb.tx_in_cap_more_data_req = 1'b0;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_in_cap_more_data_req = 1'b1;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_in_cap_more_data_req = 1'b0;
  
    // Response for the TX Send More Data from the DUT
    wait(uu_acmac_tx_tb.tx_out_cap_res_val);
    if(uu_acmac_tx_tb.tx_out_cap_res == `UU_FAILURE) 
    begin
        $display("ERROR: Unable to send TX VECTOR!");
        err_count = err_count + 1;
    end
    else
    begin
        $display("INFO: TX VECTOR sent successfully!");
        repeat(5) @(negedge uu_acmac_tx_tb.clk);
        // Transmit the TX Data
        tx_phy_data_transmit;

        repeat(5) @(negedge uu_acmac_tx_tb.clk);
    end
end
endtask

task test_packet_transmit;
begin
    // Transmit of TX Vector
    txop_txstart_transmit;
    repeat(20) @(negedge uu_acmac_tx_tb.clk);
end
endtask

// Task to write the Control frame to Control Memory
task write_ctrl_packet_tomem( string ctrl_file );
    reg [19:0]   ctrl_len;
    reg [7 :0]   buf_addr;
    reg [7 :0]   byte0;
    reg [7 :0]   count;
    integer      cp;
begin
    read_packet_len( ctrl_file, ctrl_len );     
    buf_addr = 8'h0;
    count = 8'h0;

    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.mem_ctrl_in_en = 1'b1;
    uu_acmac_tx_tb.mem_ctrl_in_wen = 1'b1; 
    
    while( buf_addr < 8'd88 )
    begin
        //$display("buff_adddr %d",buf_addr);
        // Writing the Tx Vector fields to the Control Memory
        //for( count = 8'd0; count < 8'd32; count = count + 8'd1 )
        if( count < 8'd32 )
        begin
            @(negedge uu_acmac_tx_tb.clk);
            uu_acmac_tx_tb.mem_ctrl_in_addr = buf_addr;
            uu_acmac_tx_tb.mem_ctrl_in_data = 8'h0;
            buf_addr = buf_addr + 8'd1;
		  count = count + 8'd1;
        end 

        // Writing the Control Frame Length to the Control Memory
	   if( count == 8'd32 )
	   begin
            @(negedge uu_acmac_tx_tb.clk);
            uu_acmac_tx_tb.mem_ctrl_in_addr = buf_addr;
            uu_acmac_tx_tb.mem_ctrl_in_data = ctrl_len[7:0];
            buf_addr = buf_addr + 8'd1;
	       count = count + 8'd1;
	   end

        // Writing the Frame Info and AMPDU Lengths to the Control Memory
        //for( count = 8'd33; count < 8'd88; count = count + 8'd1 )
	   if( count >= 8'd33 )
        begin
            @(negedge uu_acmac_tx_tb.clk);
            uu_acmac_tx_tb.mem_ctrl_in_addr = buf_addr;
            uu_acmac_tx_tb.mem_ctrl_in_data = 8'h0;
            buf_addr = buf_addr + 8'd1;
		  count = count + 8'd1;
        end
    end

    cp = $fopen({dir,ctrl_file}, "r");
    count = 8'h0;

    while( count < ctrl_len )
    begin
        @(negedge uu_acmac_tx_tb.clk);
        stask = $fscanf(cp, " %h ", byte0 );
        uu_acmac_tx_tb.mem_ctrl_in_addr = buf_addr;
        uu_acmac_tx_tb.mem_ctrl_in_data = byte0;
        buf_addr = buf_addr + 8'd1;       
        count = count + 8'd1; 
    end

    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.mem_ctrl_in_wen = 1'b0;
    uu_acmac_tx_tb.mem_ctrl_in_addr = 8'h0;
    uu_acmac_tx_tb.tx_in_cp_ctrl_fr_len = ctrl_len;
    $fclose(cp);

    $display("Control Memory Done\n");
end
endtask

// Task to write the Tx Vector and Packet to Memory
task write_txvector_packet_tomem( string data_file );
    reg   [19:0]   len;
    reg   [6:0]    count;
    reg   [7:0]    byte0, byte1, byte2, byte3;
    integer        dp;
begin
    count = 7'h0;
    @(negedge uu_acmac_tx_tb.clk);
    read_packet_len( data_file, len );
    framelen = len;

    while( buf_ptr < ( start_addr + 20'h16 ))
    begin
        @(negedge uu_acmac_tx_tb.clk);
        uu_acmac_tx_tb.tx_buf_in_dma_wen = 1'b1;

        case ( buf_ptr )
        //txvec
    	   ( start_addr + 20'h0 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {L_length, L_datarate, 1'b0, txpwr_level, n_tx, ch_bndwdth, format};
    	   ( start_addr + 20'h1 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {modulation, ch_offset, is_no_sig_extn, num_ext_ss, antenna_set, service};
    	   ( start_addr + 20'h2 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {is_short_GI, is_fec_ldpc_coding, stbc, is_aggregated, 1'b0, is_sounding, is_smoothing, ht_length, is_long_preamble, mcs};
    	   ( start_addr + 20'h3 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {8'h0, is_time_of_departure_req, is_txop_psallowed, group_id, indicated_chan_bw, is_dyn_bw, num_users, is_beamformed, partial_aid};
    	   ( start_addr + 20'h4 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {vht_mcs, apep_length, 2'h0, is_fec_ldpc_coding, num_sts, user_position};
    	   ( start_addr + 20'h5 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = 32'h0;
    	   ( start_addr + 20'h6 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = 32'h0;
    	   ( start_addr + 20'h7 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = 32'h0;
    	   //frameInfo
    	   ( start_addr + 20'h8 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {6'h0, retry, is_ampdu, 4'h0, framelen};
    	   ( start_addr + 20'h9 ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {16'h0, keyix, keytype, 8'h0};
    	   ( start_addr + 20'hA ) : uu_acmac_tx_tb.tx_buf_in_dma_data = timestamp;
    	   //frame
    	   ( start_addr + 20'hB ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {3'h0, n_tx_fb, stbc_fb, 6'h0, ac, 16'h0};
    	   ( start_addr + 20'hC ) : uu_acmac_tx_tb.tx_buf_in_dma_data = {aggr_count, rtscts_rate_fb, rtscts_rate, 4'h0, fallback_rate, 1'b0, mcs_fb};
    	   ( start_addr + 20'hD ) : uu_acmac_tx_tb.tx_buf_in_dma_data = num_sts_fb;
    	   //AMPDU lengths
    	   ( start_addr + 20'hE ) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
    	   ( start_addr + 20'hF ) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
    	   ( start_addr + 20'h10) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
    	   ( start_addr + 20'h11) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
    	   ( start_addr + 20'h12) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
    	   ( start_addr + 20'h13) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
    	   ( start_addr + 20'h14) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
    	   ( start_addr + 20'h15) : uu_acmac_tx_tb.tx_buf_in_dma_data = 0;
        endcase
        //$display(" Buf Ptr = %d, Start Address = %d \n", buf_ptr, start_addr );
        buf_ptr = buf_ptr + 20'h1;
    end

    dp = $fopen({dir,data_file}, "r");
 
    //$display(" Count = %d, Length = %d \n", count, len);

    while( count < len )
    begin  
        @(negedge uu_acmac_tx_tb.clk);
        uu_acmac_tx_tb.tx_buf_in_dma_wen = 1'b1;
        stask = $fscanf(dp, " %h %h %h %h ", byte0, byte1, byte2, byte3 );
        uu_acmac_tx_tb.tx_buf_in_dma_data = { byte3, byte2, byte1, byte0 };   

        if( count == 7'h4 )
            uu_acmac_tx_tb.mac_addr[0] = { (uu_acmac_tx_tb.mac_addr[0]>>32), byte3, byte2, byte1, byte0 };
        else if( count == 7'h8 )
            uu_acmac_tx_tb.mac_addr[0] = ({byte1, byte0}<<32)| uu_acmac_tx_tb.mac_addr[0];

        buf_ptr = buf_ptr + 20'h1;
        count = count + 7'h4;
    end

    start_addr = buf_ptr;
//    start_addr = 7'h0;
//    buf_ptr = 7'h0;
    @(negedge uu_acmac_tx_tb.clk);
    uu_acmac_tx_tb.tx_buf_in_dma_wen = 1'b0;

    $fclose(dp);
end
endtask

// Task to read the Vector File
task read_vector_file;
    reg [8*31:0] param;
    reg [7:0]    equal;
    reg [7:0]    value;
    reg [5:0]    count;
    reg [5:0]    i;
    integer      fv;
begin
    read_vector_num(count);
    $display(" Vector File : %s", {dir, vector_file});
    fv = $fopen({dir, vector_file}, "r");

    $display(" %h\n ", count);

    for(i = 0; i < count; i++)
    begin 
        stask = $fscanf(fv, "%s %s %h", param, equal, value);

        case( i )
            6'h0 : begin  format = value;                   end
            6'h1 : begin  ch_bndwdth = value;               end 
            6'h2 : begin  n_tx = value;                     end
            6'h3 : begin  txpwr_level = value;              end
            6'h4 : begin  L_datarate = value;               end
            6'h5 : begin  L_length = value;                 end
            6'h6 : begin  service = value;                  end
            6'h7 : begin  antenna_set = value;              end
            6'h8 : begin  num_ext_ss = value;               end
            6'h9 : begin  is_no_sig_extn = value;           end
            6'hA : begin  ch_offset = value;                end
            6'hB : begin  modulation = value;               end
            6'hC : begin  mcs = value;                      end
            6'hD : begin  is_long_preamble = value;         end
            6'hE : begin  ht_length = value;                end
            6'hF : begin  is_smoothing = value;             end
            6'h10: begin  is_sounding  = value;             end
            6'h11: begin  is_aggregated = value;            end
            6'h12: begin  stbc = value;                     end
            6'h13: begin  is_fec_ldpc_coding = value;       end
            6'h14: begin  is_short_GI = value;              end
            6'h15: begin  partial_aid = value;              end
            6'h16: begin  is_beamformed = value;            end
            6'h17: begin  num_users = value;                end
            6'h18: begin  is_dyn_bw = value;                end
            6'h19: begin  indicated_chan_bw = value;        end
            6'h1A: begin  group_id = value;                 end
            6'h1B: begin  is_txop_psallowed = value;        end  
            6'h1C: begin  is_time_of_departure_req = value; end 
            6'h1D: begin  user_position = value;            end 
            6'h1E: begin  num_sts = value;                  end
            6'h1F: begin  is_fec_ldpc_coding = value;       end
            6'h20: begin  apep_length = value;              end
            6'h21: begin  vht_mcs = value;                  end
            6'h22: begin  framelen = value;                 end
            6'h23: begin  is_ampdu = value;                 end
            6'h24: begin  retry = value;                    end
            6'h25: begin  ShortRetryCount = value;          end
            6'h26: begin  LongRetryCount = value;           end
            6'h27: begin  keytype = value;                  end
            6'h28: begin  keyix = value;                    end
            6'h29: begin  timestamp = value;                end
            6'h2A: begin  desc_id = value;                  end
            6'h2B: begin  ac = value;                       end
            6'h2C: begin  stbc_fb = value;                  end
            6'h2D: begin  n_tx_fb = value;                  end
            6'h2E: begin  mcs_fb = value;                   end
            6'h2F: begin  fallback_rate = value;            end
            6'h30: begin  rtscts_rate = value;              end
            6'h31: begin  rtscts_rate_fb = value;           end
            6'h32: begin  aggr_count = value;               end
            6'h33: begin  num_sts_fb = value;               end
       endcase         
    end
    $fclose(fv);

end
endtask

task packet_status;
begin
    if(err_count)
    begin
        $display(" Test Case Failed ");
        $display(" Fail Count = %d ", err_count );
    end 
    else
    begin
        $display(" Test Case Passed ");
    end
end
endtask

endmodule

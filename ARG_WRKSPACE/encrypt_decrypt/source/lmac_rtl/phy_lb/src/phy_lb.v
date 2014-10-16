`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:        UURMI Systems Technologies
// Engineer:       Shreeharsha.B.V
// 
// Create Date:    12:11:11 09/06/2013 
// Design Name: 
// Module Name:    PHY_stub 
// Project Name:   ACMAC
//////////////////////////////////////////////////////////////////////////////////

 `define EDGE_OF_RESET posedge rst_n
 `define POLARITY_OF_RESET rst_n
 
module phy_lb
                #( parameter  WIDTH_BYTE  = 8,
                   parameter  WIDTH_WORD = 16,
                   parameter  RSSI_VALUE  = 8'hFF,
                   parameter  RCPI_VALUE  = 8'hFF,
                   parameter  RX_START_FRAME_OFFSET  = 32'h0000_0000,
                   parameter  RECEIVE_MCS = 7'h7F,
                   parameter  SNR_VALUE = 8'hFF,
                   parameter  RESERVED1 = 2'b00,
                   parameter  RESERVED2 = 1'b0						 
                 )
                 ( input                                      clk,
                   input                                      rst_n,
                   input                                      macphy_en,
                   //Tx I/O interface between LMAC and phy
                   input                                      mac2phy_in_txstart_req,//lmac_out_ev_txstart_req
                   input                                      mac2phy_in_txdata_req,// lmac_ev_txdata_req
                   input                                      mac2phy_in_txend_req,// lmac_out_ev_txend_req
                   input                                      mac2phy_in_txdata_val,
                   input          [WIDTH_BYTE-1:0]            mac2phy_in_txdata_frame, // lmac_out_phy_frame
                   output  reg                                phy2mac_out_txstart_confirm, 	// lmac_ev_txstart_confirm
                   output  reg                                phy2mac_out_txdata_confirm,// lmac_in_ev_txdata_confirm
                   output  reg                                phy2mac_out_txend_confirm,// lmac_in_ev_txend_confirm
                   input                                      mac2phy_in_ccareset_req, //lmac_out_ev_ccareset_req
                   output  reg 	                              phy2mac_out_ccareset_confirm,//lmac_in_ev_ccareset_confirm
                   //Rx I/O interface between LMAC and phy
                   output  reg                                phy2mac_out_rxdata_val,
                   output  reg                                phy2mac_out_rxstart_ind,// lmac_in_ev_rxstart_ind
                   output  reg                                phy2mac_out_rxdata_ind,// lmac_in_ev_rxdata_ind
                   output  reg                                phy2mac_out_rxend_ind,//  lmac_in_ev_rxend_ind
                   output  reg    [WIDTH_BYTE-1:0]            phy2mac_out_rxdata // lmac_in_frame
                 );

//******************************************************************************************************************//
//---------------------------------------------------Register Declaration-------------------------------------------// 
//******************************************************************************************************************//

                    // Txvector Buffers
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf1;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf2;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf3;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf4;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf5;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf6;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf7;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf8;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf9;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf10;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf11;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf12;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf13;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf14;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf15;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf16;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf17;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf18;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf19;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf20;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf21;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf22;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf23;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf24;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf25;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf26;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf27;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf28;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf29;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf30;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf31;
                    reg   [WIDTH_BYTE-1:0]                    txvector_buf32;
                    reg   [WIDTH_BYTE-1:0]                    loc_txvector_count; // Txvector counter register
                    reg   [WIDTH_WORD-1:0]                    loc_txdata_count;   // Txdata counter register
                    reg                                       loc_start_txvector; // Start txvector transmission
                    wire                                      loc_mem_wr_en;      // Storage memory write enable 
                    wire                                      loc_mem_rd_en;
                
                   //Rxvector Buffers
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf1;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf2;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf3;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf4;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf5;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf6;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf7;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf8;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf9;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf10;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf11;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf12;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf13;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf14;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf15;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf16;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf17;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf18;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf19;
		    reg   [WIDTH_BYTE-1:0]                    rxvector_buf20;
		    reg   [WIDTH_WORD-1:0]                    thresh_rd_addr;      // Storage memory read threshold
		    reg   [WIDTH_BYTE-1:0]                    loc_count_rxvector;  // Rxvector counter
		    reg   [WIDTH_WORD-1:0]                    loc_count_rxdata;    // Rxdata counter
		    reg                                       loc_start_rxvector;  // Start rxvector reception
                    reg                                       loc_start_rxdata;	   // Start rxdata reception		 
                    reg                                       rxdata_delay;
                    
		    wire  [WIDTH_BYTE-1:0]                    loc_mem_rxdata_out;   

//******************************************************************************************************************//
//--------------------------------------------Concurrent Assignment-------------------------------------------------//
//******************************************************************************************************************//

// Txstart confirm & Txvector buffering
     always@( posedge clk or `EDGE_OF_RESET )
     begin
       if(`POLARITY_OF_RESET )
       begin
          txvector_buf1 <= 8'h0;
          txvector_buf2 <= 8'h0;
          txvector_buf3 <= 8'h0;    
          txvector_buf4 <= 8'h0;
          txvector_buf5 <= 8'h0;
      	  txvector_buf6 <= 8'h0;   
          txvector_buf7 <= 8'h0;    
          txvector_buf8 <= 8'h0;
          txvector_buf9 <= 8'h0;    
          txvector_buf10 <= 8'h0;   
          txvector_buf11 <= 8'h0;   
          txvector_buf12 <= 8'h0;
          txvector_buf13 <= 8'h0;   
          txvector_buf14 <= 8'h0;   
          txvector_buf15 <= 8'h0;   
          txvector_buf16 <= 8'h0;
          txvector_buf17 <= 8'h0;   
          txvector_buf18 <= 8'h0;   
          txvector_buf19 <= 8'h0;   
          txvector_buf20 <= 8'h0;
          txvector_buf21 <= 8'h0;   
          txvector_buf22 <= 8'h0;   
          txvector_buf23 <= 8'h0;   
          txvector_buf24 <= 8'h0; 
          txvector_buf25 <= 8'h0;   
          txvector_buf26 <= 8'h0;   
          txvector_buf27 <= 8'h0;   
          txvector_buf28 <= 8'h0;
          txvector_buf29 <= 8'h0;   
          txvector_buf30 <= 8'h0;   
	  txvector_buf31 <= 8'h0;   
          txvector_buf32 <= 8'h0;
       end
       else if( !macphy_en ) 
       begin
          txvector_buf1 <= 8'h00;   
          txvector_buf2 <= 8'h00;   
          txvector_buf3 <= 8'h00;   
          txvector_buf4 <= 8'h00;
          txvector_buf5 <= 8'h00;   
          txvector_buf6 <= 8'h00;   
          txvector_buf7 <= 8'h00;   
          txvector_buf8 <= 8'h00;
          txvector_buf9 <= 8'h00;   
          txvector_buf10 <= 8'h00;  
          txvector_buf11 <= 8'h00;  
          txvector_buf12 <= 8'h00;
          txvector_buf13 <= 8'h00;  
          txvector_buf14 <= 8'h00;  
          txvector_buf15 <= 8'h00;  
          txvector_buf16 <= 8'h00;
          txvector_buf17 <= 8'h0;   
          txvector_buf18 <= 8'h0;   
          txvector_buf19 <= 8'h0;   
          txvector_buf20 <= 8'h0;
          txvector_buf21 <= 8'h0;   
          txvector_buf22 <= 8'h0;   
          txvector_buf23 <= 8'h0;   
          txvector_buf24 <= 8'h0;
          txvector_buf25 <= 8'h0;   
          txvector_buf26 <= 8'h0;   
          txvector_buf27 <= 8'h0;   
          txvector_buf28 <= 8'h0;
          txvector_buf29 <= 8'h0;   
          txvector_buf30 <= 8'h0;   
          txvector_buf31 <= 8'h0;   
          txvector_buf32 <= 8'h0;
       end
       else if( (loc_txvector_count < 8'd32) && mac2phy_in_txdata_val && 
	        (loc_start_txvector || mac2phy_in_txstart_req )) 
       begin
          case( loc_txvector_count )
            8'h0: txvector_buf1 <= mac2phy_in_txdata_frame;
            8'h1: txvector_buf2 <= mac2phy_in_txdata_frame;
            8'h2: txvector_buf3 <= mac2phy_in_txdata_frame;
            8'h3: txvector_buf4 <= mac2phy_in_txdata_frame;
            8'h4: txvector_buf5 <= mac2phy_in_txdata_frame;
            8'h5: txvector_buf6 <= mac2phy_in_txdata_frame;
            8'h6: txvector_buf7 <= mac2phy_in_txdata_frame;
            8'h7: txvector_buf8 <= mac2phy_in_txdata_frame;
            8'h8: txvector_buf9 <= mac2phy_in_txdata_frame;
            8'h9: txvector_buf10 <= mac2phy_in_txdata_frame;
            8'ha: txvector_buf11 <= mac2phy_in_txdata_frame;
            8'hb: txvector_buf12 <= mac2phy_in_txdata_frame;
            8'hc: txvector_buf13 <= mac2phy_in_txdata_frame;
            8'hd: txvector_buf14 <= mac2phy_in_txdata_frame;
            8'he: txvector_buf15 <= mac2phy_in_txdata_frame;
            8'hf: txvector_buf16 <= mac2phy_in_txdata_frame;
            8'h10: txvector_buf17 <= mac2phy_in_txdata_frame;
            8'h11: txvector_buf18 <= mac2phy_in_txdata_frame;
            8'h12: txvector_buf19 <= mac2phy_in_txdata_frame;
            8'h13: txvector_buf20 <= mac2phy_in_txdata_frame;
            8'h14: txvector_buf21 <= mac2phy_in_txdata_frame;
            8'h15: txvector_buf22 <= mac2phy_in_txdata_frame;
            8'h16: txvector_buf23 <= mac2phy_in_txdata_frame;
            8'h17: txvector_buf24 <= mac2phy_in_txdata_frame;
            8'h18: txvector_buf25 <= mac2phy_in_txdata_frame;
            8'h19: txvector_buf26 <= mac2phy_in_txdata_frame;
            8'h1a: txvector_buf27 <= mac2phy_in_txdata_frame;
            8'h1b: txvector_buf28 <= mac2phy_in_txdata_frame;
            8'h1c: txvector_buf29 <= mac2phy_in_txdata_frame;
            8'h1d: txvector_buf30 <= mac2phy_in_txdata_frame;
            8'h1e: txvector_buf31 <= mac2phy_in_txdata_frame;
            8'h1f: txvector_buf32 <= mac2phy_in_txdata_frame;
          endcase
       end
     end

// Local Tx Vector Register
     always@( posedge clk or `EDGE_OF_RESET)
     begin
       if(`POLARITY_OF_RESET)
          loc_start_txvector <= 1'b0;
       else if( !macphy_en )
          loc_start_txvector <= 1'b0;
       else if( mac2phy_in_txstart_req ) 
          loc_start_txvector <= 1'b1; 
       else if( loc_txvector_count == 8'd31 ) 
          loc_start_txvector <= 1'b0;
     end

// Txstart Confirm From PHY     
    always@( posedge clk or `EDGE_OF_RESET)
    begin
      if(`POLARITY_OF_RESET)
         phy2mac_out_txstart_confirm <= 1'b0;
      else if( !macphy_en )
         phy2mac_out_txstart_confirm <= 1'b0;
      else if ( phy2mac_out_txstart_confirm )
         phy2mac_out_txstart_confirm <= 1'b0;
      else if( loc_txvector_count == 8'd31 )
         phy2mac_out_txstart_confirm <= 1'b1;
    end

//Txvector counter
    always@( posedge clk or `EDGE_OF_RESET)
    begin
      if(`POLARITY_OF_RESET)
         loc_txvector_count <= 8'h00;
      else if( !macphy_en)
         loc_txvector_count <= 1'b0;
      else if((loc_start_txvector || mac2phy_in_txstart_req) && 
               mac2phy_in_txdata_val )
         loc_txvector_count <=  loc_txvector_count + 1;
       else if ( phy2mac_out_txstart_confirm ) 
         loc_txvector_count <= 8'h00;
    end

// Local Txdata counter   	 
    always@( posedge clk or `EDGE_OF_RESET ) 
    begin
      if( `POLARITY_OF_RESET )
        loc_txdata_count <= 16'h0000;
      else if( !macphy_en )
        loc_txdata_count <= 16'h0000;
      else if( mac2phy_in_txdata_req && mac2phy_in_txdata_val )
        loc_txdata_count <= loc_txdata_count + 1;
      else if( mac2phy_in_txend_req )
        loc_txdata_count <= 16'h0;
    end
    
// Threshold Length for the Data
    always@( posedge clk or `EDGE_OF_RESET ) 
    begin
        if( `POLARITY_OF_RESET )
            thresh_rd_addr <= 16'h0000;
        else if( !macphy_en )
            thresh_rd_addr <= 16'h0000;
        else if( mac2phy_in_txend_req )
            thresh_rd_addr <= loc_txdata_count;
    end

// Tx Data Confirm
    always@( posedge clk or `EDGE_OF_RESET ) 
    begin
        if(`POLARITY_OF_RESET )
            phy2mac_out_txdata_confirm <= 1'b0;
        else if( !macphy_en )
          phy2mac_out_txdata_confirm <= 1'b0;
        else if( mac2phy_in_txdata_req && mac2phy_in_txdata_val )
          phy2mac_out_txdata_confirm <= 1'b1;
        else
          phy2mac_out_txdata_confirm <= 1'b0;
    end

//Txend confirm from phy to mac
    always@( posedge clk or  `EDGE_OF_RESET)
    begin
      if(`POLARITY_OF_RESET)
        phy2mac_out_txend_confirm <= 1'b0;
      else if( !macphy_en )
        phy2mac_out_txend_confirm <= 1'b0;
      else if( mac2phy_in_txend_req )
        phy2mac_out_txend_confirm <= 1'b1;
      else
        phy2mac_out_txend_confirm <= 1'b0;
    end

//Rxvector buffer filling
      always@( posedge clk or `EDGE_OF_RESET ) 
      begin
        if(`POLARITY_OF_RESET )
        begin
          rxvector_buf1 <= 8'h00;   
          rxvector_buf2 <= 8'h00;   
          rxvector_buf3 <= 8'h00;    
          rxvector_buf4 <= 8'h00;
          rxvector_buf5 <= 8'h00;   
          rxvector_buf6 <= 8'h00;   
          rxvector_buf7 <= 8'h00;    
          rxvector_buf8 <= 8'h00;
          rxvector_buf9 <= 8'h00;   
          rxvector_buf10 <= 8'h00;  
          rxvector_buf11 <= 8'h00;   
          rxvector_buf12 <= 8'h00;
          rxvector_buf13 <= 8'h00;   
          rxvector_buf14 <= 8'h00;  
          rxvector_buf15 <= 8'h00;   
          rxvector_buf16 <= 8'h00;
          rxvector_buf17 <= 8'h00;   
          rxvector_buf18 <= 8'h00;  
          rxvector_buf19 <= 8'h00;   
          rxvector_buf20 <= 8'h00;
        end
        else if( !macphy_en )
        begin
          rxvector_buf1 <= 8'h00;   
          rxvector_buf2 <= 8'h00;   
          rxvector_buf3 <= 8'h00;    
          rxvector_buf4 <= 8'h00;
          rxvector_buf5 <= 8'h00;   
          rxvector_buf6 <= 8'h00;   
          rxvector_buf7 <= 8'h00;    
          rxvector_buf8 <= 8'h00;
          rxvector_buf9 <= 8'h00;   
          rxvector_buf10 <= 8'h00;  
          rxvector_buf11 <= 8'h00;   
          rxvector_buf12 <= 8'h00;
          rxvector_buf13 <= 8'h00;   
          rxvector_buf14 <= 8'h00;  
          rxvector_buf15 <= 8'h00;   
          rxvector_buf16 <= 8'h00;
          rxvector_buf17 <= 8'h00;   
          rxvector_buf18 <= 8'h00;  
          rxvector_buf19 <= 8'h00;   
          rxvector_buf20 <= 8'h00;
        end
        else if( loc_txvector_count >= 8'd30 )
        begin
          rxvector_buf1 <= { RESERVED1, txvector_buf9[7], txvector_buf8[7:5], txvector_buf1[1:0]};
          rxvector_buf2 <= { txvector_buf3[3:0], RESERVED2, txvector_buf1[4:2]};
          rxvector_buf3 <= { txvector_buf4[3:0], txvector_buf3[7:4]};
          rxvector_buf4 <= { txvector_buf1[4:2], txvector_buf14[4], txvector_buf4[7:4]};
          rxvector_buf5 <= RSSI_VALUE;
          rxvector_buf6 <= RCPI_VALUE;
          rxvector_buf7 <= { txvector_buf8[1:0], txvector_buf12[5:4], txvector_buf12[7], txvector_buf12[3], txvector_buf12[1:0]};
          rxvector_buf8 <= { txvector_buf17[5],txvector_buf12[6], txvector_buf9[6:0]};
          rxvector_buf9 <= 8'hFF;
          rxvector_buf10 <= 8'hFF;
          rxvector_buf11 <= 8'hFF;
          rxvector_buf12 <= 8'hFF;
          rxvector_buf13 <= { 1'b0, RECEIVE_MCS };
          rxvector_buf14 <= 8'hFF;
          rxvector_buf15 <= 8'hFF;
          rxvector_buf16 <= {1'b0,txvector_buf17[6:4],4'b0000};
          rxvector_buf17 <= txvector_buf13[7:0];
          rxvector_buf18 <= { txvector_buf15[6:0], txvector_buf14[0] };
          rxvector_buf19 <= SNR_VALUE;
          rxvector_buf20 <= txvector_buf16;
        end
      end

//CCARESET confirm from phy to MAC
      always@( posedge clk or `EDGE_OF_RESET )
      begin
        if(`POLARITY_OF_RESET )
          phy2mac_out_ccareset_confirm <= 1'b0;
        else if(!macphy_en )
          phy2mac_out_ccareset_confirm <= 1'b0;
        else if( mac2phy_in_ccareset_req )
          phy2mac_out_ccareset_confirm <= 1'b1;
        else
          phy2mac_out_ccareset_confirm <= 1'b0;
      end

// Rxstart indication from phy to MAC
    always@( posedge clk or `EDGE_OF_RESET)
    begin
        if(`POLARITY_OF_RESET)
            phy2mac_out_rxstart_ind <= 1'b0;
        else if( !macphy_en)
            phy2mac_out_rxstart_ind <= 1'b0;
        else if( phy2mac_out_ccareset_confirm)
            phy2mac_out_rxstart_ind <= 1'b1;
        else
            phy2mac_out_rxstart_ind <= 1'b0;
    end	

// Local Start Rx Vector
    always@( posedge clk or `EDGE_OF_RESET)
    begin
        if(`POLARITY_OF_RESET)
            loc_start_rxvector <= 1'b0; 
        else if( !macphy_en)
            loc_start_rxvector <= 1'b0; 
        else if(phy2mac_out_rxstart_ind)
            loc_start_rxvector <= 1'b1; 
        else if( loc_count_rxvector == 8'h13 )
            loc_start_rxvector <= 1'b0;
    end

// Rx Data Indication delay 
    always@( posedge clk or `EDGE_OF_RESET)
    begin
        if(`POLARITY_OF_RESET)
            rxdata_delay <= 1'b0;
        else if(! macphy_en)
            rxdata_delay <= 1'b0;
        else if( loc_count_rxvector == 8'h13 )
            rxdata_delay <= 1'b1;
        else  
            rxdata_delay <= 1'b0;
    end 

//Rxdata indication from phy to MAC
    always@( posedge clk or `EDGE_OF_RESET)
    begin
        if(`POLARITY_OF_RESET)
            phy2mac_out_rxdata_ind <= 1'b0;
        else if(! macphy_en)
            phy2mac_out_rxdata_ind <= 1'b0;
        else
            phy2mac_out_rxdata_ind <= rxdata_delay;
    end

//Start receiving data       
    always@( posedge clk or `EDGE_OF_RESET)
    begin
        if(`POLARITY_OF_RESET)
            loc_start_rxdata <= 1'b0;
        else if(! macphy_en)
            loc_start_rxdata <= 1'b0;
        else if( phy2mac_out_rxdata_ind )
            loc_start_rxdata <= 1'b1;
        else if(loc_count_rxdata == thresh_rd_addr)
            loc_start_rxdata <= 1'b0;
    end

//Rxdata valid request from phy to MAC
    always@( posedge clk or `EDGE_OF_RESET )
    begin
        if(`POLARITY_OF_RESET )
            phy2mac_out_rxdata_val <= 1'b0;
        else if(!macphy_en ) 
            phy2mac_out_rxdata_val <= 1'b0;
        else if( loc_count_rxvector > 8'h13)
            phy2mac_out_rxdata_val <= 1'b0;
        else if( loc_start_rxvector )
            phy2mac_out_rxdata_val <= 1'b1;
        else if(loc_count_rxdata > thresh_rd_addr)
            phy2mac_out_rxdata_val <= 1'b0;
        else if( loc_start_rxdata )
            phy2mac_out_rxdata_val <= 1'b1;
    end

//Rxvector counter	  
    always@( posedge clk or `EDGE_OF_RESET )	  
    begin
        if(`POLARITY_OF_RESET)
            loc_count_rxvector <= 8'h00;
        else if(!macphy_en)
            loc_count_rxvector <= 8'h00;
        else if( loc_start_rxvector )
            loc_count_rxvector <= loc_count_rxvector + 1;
        else
            loc_count_rxvector <= 8'h00;  
    end			

// Rxdata counter
    always@( posedge clk or `EDGE_OF_RESET)
    begin
        if(`POLARITY_OF_RESET )
            loc_count_rxdata <= 16'h0000;
        else if( !macphy_en)
            loc_count_rxdata <= 16'h0000;
        else if( loc_start_rxdata || phy2mac_out_rxdata_ind )
            loc_count_rxdata <= loc_count_rxdata + 1;
        else
            loc_count_rxdata <= 16'h0000;
    end

// RxEnd Indication
    always@( posedge clk or `EDGE_OF_RESET)
    begin
        if(`POLARITY_OF_RESET )
            phy2mac_out_rxend_ind <= 1'b0;
        else if( !macphy_en) 
            phy2mac_out_rxend_ind <= 1'b0;
        else if( loc_count_rxdata > thresh_rd_addr )
            phy2mac_out_rxend_ind <= 1'b1;
        else
            phy2mac_out_rxend_ind <= 1'b0;
    end

//Rxdata from phy to MAC
    always@( posedge clk or `EDGE_OF_RESET )
    begin
      if(`POLARITY_OF_RESET )
        phy2mac_out_rxdata <= 8'h00;
      else if( !macphy_en )
        phy2mac_out_rxdata <= 8'h00;
      else if(loc_start_rxvector )
      begin
        case( loc_count_rxvector )
           8'h0: phy2mac_out_rxdata <= rxvector_buf1;
           8'h1: phy2mac_out_rxdata <= rxvector_buf2;
           8'h2: phy2mac_out_rxdata <= rxvector_buf3;
           8'h3: phy2mac_out_rxdata <= rxvector_buf4;
           8'h4: phy2mac_out_rxdata <= rxvector_buf5;
           8'h5: phy2mac_out_rxdata <= rxvector_buf6;
           8'h6: phy2mac_out_rxdata <= rxvector_buf7;
           8'h7: phy2mac_out_rxdata <= rxvector_buf8;
           8'h8: phy2mac_out_rxdata <= rxvector_buf9;
           8'h9: phy2mac_out_rxdata <= rxvector_buf10;
           8'ha: phy2mac_out_rxdata <= rxvector_buf11;
           8'hb: phy2mac_out_rxdata <= rxvector_buf12;
           8'hc: phy2mac_out_rxdata <= rxvector_buf13;
           8'hd: phy2mac_out_rxdata <= rxvector_buf14;
           8'he: phy2mac_out_rxdata <= rxvector_buf15;
           8'hf: phy2mac_out_rxdata <= rxvector_buf16;
           8'h10: phy2mac_out_rxdata <= rxvector_buf17;
           8'h11: phy2mac_out_rxdata <= rxvector_buf18;
           8'h12: phy2mac_out_rxdata <= rxvector_buf19;
           8'h13: phy2mac_out_rxdata <= rxvector_buf20;
        endcase
      end
      else if( loc_start_rxdata )
        phy2mac_out_rxdata <= loc_mem_rxdata_out;
      else 
        phy2mac_out_rxdata <= 8'h00;
    end
	
  assign loc_mem_wr_en = mac2phy_in_txdata_req ? mac2phy_in_txdata_val : 1'b0 ;
  assign loc_mem_rd_en = ( phy2mac_out_rxdata_ind || loc_start_rxdata ) ? 1'b1 : 1'b0;

//Txdata storage memory
     Data_mem Storage_Mem (
     .clka   ( clk                    ),//Input writeclk
     .wea    ( loc_mem_wr_en          ),//Input write enable
     .addra  ( loc_txdata_count       ),//Input write address [15 : 0]
     .dina   ( mac2phy_in_txdata_frame),//Input write data frame [7 : 0]mac2phy_in_txdata_frame
     .clkb   ( clk                    ),//Input read clk
     .rstb   ( rst_n                  ),//Input reset memory
     .enb    ( loc_mem_rd_en          ),//Input read enable
     .addrb  ( loc_count_rxdata       ),//Input read address  [15 : 0]
     .doutb  ( loc_mem_rxdata_out     )//Output read data [7 : 0]
     );

endmodule  

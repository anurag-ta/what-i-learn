`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 	   Shreeharsha.B.V
// 
// Create Date:    14:01:23 12/02/2013 
// Design Name: 
// Module Name:    tb_fifo_logic 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////

module tb_fifo_logic();
		reg			wr_clk                   ;
		reg			rd_clk                   ;
		reg			rst_n                    ;
		reg			enable                   ;
		//Input ports
		reg			mac2fifo_txstart_req     ;
		reg			mac2fifo_txend_req       ;
		reg   [7 :0]		mac2fifo_phy_frame       ;
		reg			mac2fifo_frame_val	 ;
		reg			phy2fifo_txstart_confirm ;
		reg			phy2fifo_txdata_confirm	 ;
		reg			phy2fifo_txend_confirm	 ;
		//Output ports
		wire			fifo2mac_txstart_confirm ;
		wire			fifo2mac_txdata_confirm  ;
		wire			fifo2mac_txend_confirm	 ;
		wire			fifo2phy_txstart_req     ;
		wire			fifo2phy_frame_val       ;
		wire			fifo2phy_txend_req       ;
		wire  [7 :0]		fifo2phy_phy_frame       ;
		
// Parameter declaration
    parameter	    read_clk = 2000;
    parameter       write_clk = 2000;


// Clock generation
    always #( read_clk/2) rd_clk = ~ rd_clk;
    always #( write_clk/2) wr_clk = ~wr_clk;


//Component instantiation
    uu_fifo_logic fifo (
           .wr_clk                      ( wr_clk                      ), 
           .rd_clk                      ( rd_clk                      ), 
           .rst_n                       ( rst_n                       ), 
           .enable                      ( enable                      ), 
           .mac2fifo_txstart_req        ( mac2fifo_txstart_req        ), 
           .mac2fifo_txend_req          ( mac2fifo_txend_req          ), 
           .mac2fifo_phy_frame          ( mac2fifo_phy_frame          ), 
           .mac2fifo_frame_val		( mac2fifo_frame_val          ), 
           .fifo2mac_txstart_confirm    ( fifo2mac_txstart_confirm    ), 
           .fifo2mac_txdata_confirm	( fifo2mac_txdata_confirm     ), 
           .fifo2mac_txend_confirm      ( fifo2mac_txend_confirm      ), 
           .phy2fifo_txstart_confirm    ( phy2fifo_txstart_confirm    ), 
           .phy2fifo_txdata_confirm     ( phy2fifo_txdata_confirm     ), 
           .phy2fifo_txend_confirm      ( phy2fifo_txend_confirm      ), 
           .fifo2phy_txstart_req        ( fifo2phy_txstart_req        ), 
           .fifo2phy_phy_frame          ( fifo2phy_phy_frame          ), 
           .fifo2phy_frame_val          ( fifo2phy_frame_val          ), 
           .fifo2phy_txend_req          ( fifo2phy_txend_req          )
	);
	
  initial 
  begin
     rst_n = 0;
     #3000;
     rst_n = 1;
  // Module enabling	
     #8000;
     $display("Enable module\n");
     enable = 1;
     @( posedge wr_clk);
     mac2fifo_txstart_req = 1;
     mac2fifo_frame_val = 1; 
     @( posedge wr_clk);
     mac2fifo_txstart_req = 0;
  end

  initial
  begin
   // Initialization	  
     wr_clk = 0;
     rd_clk = 0;
     enable = 0;
     mac2fifo_txstart_req = 0;
     mac2fifo_txend_req = 0;
     mac2fifo_phy_frame = 0;
     mac2fifo_frame_val = 0;
     phy2fifo_txstart_confirm = 0;
     phy2fifo_txdata_confirm = 0;
     phy2fifo_txend_confirm = 0;
     #11000;
   // Process - I
     $display("Read txvector from text file\n");
      read_txvector();
     $display("Read txdata from text file\n");
      read_txdata();
     #6000;
     $display(" Read FIFO\n");
      fifo_read();
	
  #8000;
   // Process - II      
   $display("Repeat the above process\n");
      @( posedge wr_clk);
	mac2fifo_txstart_req = 1; 
	mac2fifo_frame_val = 1 ;
      @( posedge wr_clk);	
        mac2fifo_txstart_req = 0;
     $display("Read txvector from file\n");
      read_txvector();
     $display("Read txdata from file\n");			
      read_txdata();
     #6000;
      fifo_read();
		
  end
	
////////////////////////////////////////////////  Read txvector from file	

  task read_txvector();
  begin : txvector
     integer fp,rv,i;
     rv = 1;
     i = 0;
       fp = $fopen("D:/UURMI/acmac/macphy_intgr/fifo_logic/vector/txvector.txt","r");
       while( rv > 0 & i <= 32 )
       begin
         //mac2fifo_frame_val = 1;
	 @(posedge wr_clk);
         rv = $fscanf(fp,"%h",mac2fifo_phy_frame);	
         $display("txvector = %h, i = %d\n",mac2fifo_phy_frame,i);
         i = i + 1;
        // @(posedge wr_clk);
       end
     $fclose(fp);
     $display("Close fp\n");
       mac2fifo_frame_val = 0;
       mac2fifo_phy_frame = 0;
       phy2fifo_txstart_confirm = 1;
     @( posedge wr_clk);
       phy2fifo_txstart_confirm = 0;	

  end
  endtask

/////////////////////////////////////////////////  Read Tx data from file 

  task read_txdata();
  begin: txdata
     integer ptr,ret,j;
     ret = 1;
     j  = 0;
       ptr = $fopen("D:/UURMI/acmac/macphy_intgr/fifo_logic/vector/txdata.txt","r");
       while( ret  > 0 & j < 7)
       begin
         if( fifo2mac_txdata_confirm == 1 ) 
           mac2fifo_frame_val = 1;
         else 
           mac2fifo_frame_val = 0;

         if( mac2fifo_frame_val == 1 ) begin
           ret = $fscanf(ptr,"%h",mac2fifo_phy_frame);	
           $display("txdata = %h, j = %d\n",mac2fifo_phy_frame, j );
           j = j + 1; 
         end
         @(posedge wr_clk);
       end
         mac2fifo_phy_frame = 0;
         $fclose(ptr);
         $display("Read data file closed\n");
     mac2fifo_frame_val = 0;

  end
  endtask

////////////////////////////////////////////////  Read data from FIFO

  task fifo_read();
  begin : fifo
     integer k,ptr,ret,j,l;
     ret = 1;
     k = 0;
     j = 0;
	  l = 0;
       ptr = $fopen("D:/UURMI/acmac/macphy_intgr/fifo_logic/vector/txdata.txt","r");
       while( ret  > 0 & j <= 32 & k < 72 ) //69
       begin
	 // phy to fifo txdata confirm generation  
         @( posedge wr_clk);	
				 if( ( ( k >= 7 ) & ( k < 18 ) ) |  ( ( k >= 25  ) & ( k < 36 ) ) | ( ( k >= 43  ) & ( k < 54 ) ) | ( ( k >= 61  ) & ( k < 72 ) ) )
			    phy2fifo_txdata_confirm = 0;
		     else
		       phy2fifo_txdata_confirm = 1;
         //if( phy2fifo_txdata_confirm == 1)
          // phy2fifo_txdata_confirm = 0;
         //else
        //   phy2fifo_txdata_confirm = 1;
         // fifo to mac frame valid generation				
         if( fifo2mac_txdata_confirm == 1 ) 
           mac2fifo_frame_val = 1;
         else 
           mac2fifo_frame_val = 0;
         // Scan txdata from file 
         if( mac2fifo_frame_val == 1 ) begin
           ret = $fscanf(ptr,"%h",mac2fifo_phy_frame);	
           $display("txdata = %h, j = %d\n",mac2fifo_phy_frame, j );
           j = j + 1; 
         end
           k = k + 1; 
           $display("k = %d\n",k);
			// @( posedge wr_clk);
       end
         mac2fifo_phy_frame = 0;       
         phy2fifo_txdata_confirm = 0;
         mac2fifo_frame_val = 0;
         mac2fifo_txend_req = 1;
         @(posedge wr_clk);
           mac2fifo_txend_req = 0;
           while ( k < 81 )  begin
			   @(posedge wr_clk);
             if( ( k >= 72  ) & ( k < 79 ))
               phy2fifo_txdata_confirm = 1;
             else
               phy2fifo_txdata_confirm = 0;
               k = k + 1; 
             $display("k = %d", k);
           end
			  phy2fifo_txdata_confirm = 0;
			 @(posedge wr_clk);
           wait( fifo2phy_txend_req == 1 );
             phy2fifo_txend_confirm = 1;
           @(posedge wr_clk);
             phy2fifo_txend_confirm = 0;
           $display("Closed ptr\n");
			$fclose(ptr);
  end
  endtask

endmodule

// This module implements the functionality of Deaggregation of RX Packet in LMAC.                                                                      *

`include "../../inc/defines.h"
 
module uu_acmac_deaggregation
                     #(
                        parameter WIDTH_WORD = 32,
                        parameter WIDTH_HALF_WORD = 16,
                        parameter WIDTH_BYTE = 8
                      )
                      (
                        input                             deagg_clk,            // Connected to System Clock
                        input                             rst_n,                // Connected to System Reset
                        input                             deagg_enable,         // Connected to Module Enable
                        input [WIDTH_BYTE-1:0]            deagg_in_data,        // Input Data from Rx Handler (rx_in_frame)
                        input                             deagg_in_data_valid,  // Input Data Valid from (rx_in_frame_valid)
                        input [1:0]                       deagg_in_frame_format,// Input Frame format from Rx Vector
                                                             
                        output                            deagg_out_sign_valid,    // Signature Valid Pulse to Rx Handler
                        output                            deagg_out_invalid_pulse, // Delimiter invalid Pulse to Rx Handler
                        output reg                        deagg_out_crc8_invalid,  // CRC8 invalid Pulse to Rx Handler
                        output reg                        deagg_out_data_valid,    // Deaggregation Data Valid Signal to Rx Handler
                        output reg [1:0]                  deagg_out_pad_bytes,     // Pad Bytes Calculated to Rx Handler
                        output reg [2:0]                  deagg_out_return_value,  // Deaggregated Subframe Validity return value
                        output reg [WIDTH_HALF_WORD-1:0]  deagg_out_subframe_length // Valid Subframe Length
                     );

// Register Declarations
   reg [1:0]                                            delimiter_count_r; // Internal Register for delimiter counting
   reg [WIDTH_BYTE-1:0]                                 delimiter_crc_r;   // Internal Register for storing the CRC8
   reg [WIDTH_HALF_WORD-1:0]                            data_count_r;      // Internal Register to count the subframe
   reg [WIDTH_HALF_WORD-1:0]                            delimiter_r;       // Internal Register to store the delimiter length field.
   reg                                                  delimiter_valid_r;
   reg [3:0]                                            delimiter_valid_count;

// Wire Declarations
   assign deagg_out_sign_valid = ( deagg_enable && ( delimiter_count_r == 2'h3 ) && 
                                   deagg_in_data_valid && ( deagg_in_data == 8'h4E ))
                                 ? 1'b1 : 1'b0;

   assign deagg_out_invalid_pulse = ( !deagg_out_crc8_invalid && deagg_enable && 
                                    ( delimiter_count_r == 2'h3 ) && deagg_in_data_valid && 
                                    ( deagg_in_data != 8'h4E )) ? 1'b1: 1'b0; 

// Always Block to keep track of Delimiter length.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         delimiter_count_r <= 2'h0; 
      else if( !deagg_enable || delimiter_valid_r)
         delimiter_count_r <= 2'h0; 
      else if( deagg_enable && deagg_in_data_valid && (!deagg_out_data_valid | (deagg_out_return_value == 2) ) && !delimiter_valid_r)
         delimiter_count_r <= delimiter_count_r + 2'h1;
   end
wire [15:0] crc_input = delimiter_valid_r ? 16'h0 : {delimiter_r [7:0] ,deagg_in_data};
// Always Block to check for Delimiter CRC Calculation and Registering
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         delimiter_crc_r <= 8'h0;
      else if( !deagg_enable || deagg_out_sign_valid )
         delimiter_crc_r <= 8'h0;
      else if( deagg_enable && ( delimiter_count_r == 2'h1 ) && 
               deagg_in_data_valid && !deagg_out_data_valid )
         delimiter_crc_r <= crc8_16in ( crc_input );
   end

// Always Block to count the Data received.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         data_count_r <= 16'h0;
      else if( !deagg_enable || !deagg_out_data_valid )
         data_count_r <= 16'h0;
      else if( deagg_enable && 
            (( deagg_out_return_value == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME ) || 
             ( deagg_out_return_value == `UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME )))
         data_count_r <= 16'h0; 
      else if( deagg_enable && deagg_out_data_valid && deagg_in_data_valid )
         data_count_r <= data_count_r + 16'h1;      
   end

// Always Block to store the AMPDU Delimiter.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         delimiter_r <= 16'h0;
      else if( !deagg_enable )
         delimiter_r <= 16'h0;
      else if( deagg_enable && deagg_in_data_valid && !deagg_out_data_valid )
      begin
         if( delimiter_count_r == 2'h0 )
             delimiter_r <= { delimiter_r [15:8], deagg_in_data };
         else if( delimiter_count_r == 2'h1 ) 
             delimiter_r <= { deagg_in_data, delimiter_r [7:0] };
      end
   end 

// Always Block for Deaggregation Data Valid.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         deagg_out_data_valid <= 1'b0;
      else if( !deagg_enable || 
             ( deagg_out_return_value == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME ) || (data_count_r == deagg_out_subframe_length - 'h4))
         deagg_out_data_valid <= 1'b0;
      else if( deagg_enable && deagg_out_sign_valid && !deagg_out_crc8_invalid )
      begin
         if(( delimiter_r[15:2] != 14'h0 ) &&
            ( deagg_in_frame_format == `UU_WLAN_FRAME_FORMAT_VHT ))
              deagg_out_data_valid <= 1'b1;  
         else if(( delimiter_r[15:4] != 12'h0 ) &&
                 ( deagg_in_frame_format != `UU_WLAN_FRAME_FORMAT_VHT ))
              deagg_out_data_valid <= 1'b1; 
      end
   end
   
 
// Always Block for CRC8 Invalid.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         deagg_out_crc8_invalid <= 1'b0;
      else if( !deagg_enable ) 
         deagg_out_crc8_invalid <= 1'b0;
      else if( deagg_enable && deagg_in_data_valid && ( delimiter_count_r == 2'h3 ))
         deagg_out_crc8_invalid <= 1'b0;
      else if( deagg_enable && deagg_in_data_valid && 
             ( delimiter_count_r == 2'h2 ) && ( delimiter_crc_r != deagg_in_data ))
         deagg_out_crc8_invalid <= 1'b1;
   end
     
// Always Block to calculate the Pad Bytes.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         deagg_out_pad_bytes <= 2'h0;
      else if( !deagg_enable )
         deagg_out_pad_bytes <= 2'h0;
      else if( deagg_enable && deagg_out_sign_valid && 
             ( !deagg_out_crc8_invalid ))
      begin
         if( deagg_in_frame_format == `UU_WLAN_FRAME_FORMAT_VHT )
         begin
            case( delimiter_r [3:2] ) 
               2'b00 : deagg_out_pad_bytes <= 2'h0;
               2'b01 : deagg_out_pad_bytes <= 2'h3;
               2'b10 : deagg_out_pad_bytes <= 2'h2;
               2'b11 : deagg_out_pad_bytes <= 2'h1;
               default : deagg_out_pad_bytes <= 2'h0;
            endcase 
         end
         else
         begin
            case( delimiter_r [5:4] ) 
               2'b00 : deagg_out_pad_bytes <= 2'h0;
               2'b01 : deagg_out_pad_bytes <= 2'h3;
               2'b10 : deagg_out_pad_bytes <= 2'h2;
               2'b11 : deagg_out_pad_bytes <= 2'h1;
               default : deagg_out_pad_bytes <= 2'h0;
            endcase 
         end
      end
   end
 
// Always Block to return status from deaggregation module.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         deagg_out_return_value <= 3'h0;
      else if( !deagg_enable )
         deagg_out_return_value <= 3'h0;
      else if( deagg_enable )
      begin
         if( deagg_out_data_valid && deagg_in_data_valid &&
          (( data_count_r + 16'h1 ) == ( deagg_out_subframe_length + 
             deagg_out_pad_bytes - 16'h3 )))
            deagg_out_return_value <= `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME;
         else if( deagg_out_sign_valid && !deagg_out_crc8_invalid ) 
         begin
            if(( delimiter_r[15:2] == 14'h0 ) &&
               ( deagg_in_frame_format == `UU_WLAN_FRAME_FORMAT_VHT ))
                 deagg_out_return_value <= `UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME; 
            else if(( delimiter_r[15:4] == 12'h0 ) &&
                    ( deagg_in_frame_format != `UU_WLAN_FRAME_FORMAT_VHT ))
                 deagg_out_return_value <= `UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME; 
         end
         else if( deagg_out_invalid_pulse || ( deagg_out_sign_valid && deagg_out_crc8_invalid ))
            deagg_out_return_value <= `UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME; 
         else if( deagg_in_data_valid && 
                ( deagg_out_return_value == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME ))
            deagg_out_return_value <= 3'h0; 
      end
   end
   
// Always block for giving valid crc input
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
        delimiter_valid_r  <= 1'b0;
      else if( !deagg_enable ) 
         delimiter_valid_r <= 1'b0;
      else if( deagg_enable && (deagg_out_return_value ==  `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME))
         delimiter_valid_r <= 1'b1;
      else if( delimiter_valid_count == 4'h9 )
         delimiter_valid_r <= 1'b0;
   end
   
// Always block for giving valid delimiter count
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
        delimiter_valid_count  <= 1'b0;
      else if( !deagg_enable ) 
         delimiter_valid_count <= 1'b0;
      else if( deagg_enable && delimiter_valid_r)
         delimiter_valid_count <= delimiter_valid_count + 'h1;
      else if( !delimiter_valid_r  )
         delimiter_valid_count <= 1'b0;
   end

// Always Block for Deaggregation Subframe length.
   always @( posedge deagg_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         deagg_out_subframe_length <= 16'h0;
      else if( !deagg_enable )
         deagg_out_subframe_length <= 16'h0;
      else if( deagg_enable && deagg_out_sign_valid && 
             ( !deagg_out_crc8_invalid ))
      begin
         if( deagg_in_frame_format == `UU_WLAN_FRAME_FORMAT_VHT )
            deagg_out_subframe_length <= { 2'h0, delimiter_r [15:2] };
         else
            deagg_out_subframe_length <= { 4'h0, delimiter_r [15:4] };
      end
   end



// Function to Calculate the CRC8 for 16 Bit Input.
function [7:0] crc8_16in;
   input [15:0] data;
   
   reg [7:0]   crc_in;
   reg [15:0]  d;
   reg [7:0]   c;
   reg [7:0]   c_out;
   integer i;
   begin
      crc_in = 8'h00;
      for (i=0;i<=15;i=i+1)
      d[i] = data[15-i];
      c = crc_in;

      c_out[0] = d[1] ^ d[3] ^ d[7] ^ d[8] ^ d[9] ^ d[15] ^ c[0] ^ c[4] ^ c[6];
      c_out[1] = d[0] ^ d[1] ^ d[2] ^ d[3] ^ d[6] ^ d[9] ^ d[14] ^ d[15] ^ c[1] ^ c[4] ^ c[5] ^ c[6] ^ c[7];
      c_out[2] = d[0] ^ d[2] ^ d[3] ^ d[5] ^ d[7] ^ d[9] ^ d[13] ^ d[14] ^ d[15] ^ c[0] ^ c[2] ^ c[4] ^ c[5] ^ c[7];
      c_out[3] = d[1] ^ d[2] ^ d[4] ^ d[6] ^ d[8] ^ d[12] ^ d[13] ^ d[14] ^ c[1] ^ c[3] ^ c[5] ^ c[6];
      c_out[4] = d[0] ^ d[1] ^ d[3] ^ d[5] ^ d[7] ^ d[11] ^ d[12] ^ d[13] ^ c[0] ^ c[2] ^ c[4] ^ c[6] ^ c[7];
      c_out[5] = d[0] ^ d[2] ^ d[4] ^ d[6] ^ d[10] ^ d[11] ^ d[12] ^ c[1] ^ c[3] ^ c[5] ^ c[7];
      c_out[6] = d[1] ^ d[3] ^ d[5] ^ d[9] ^ d[10] ^ d[11] ^ c[2] ^ c[4] ^ c[6];
      c_out[7] = d[0] ^ d[2] ^ d[4] ^ d[8] ^ d[9] ^ d[10] ^ c[3] ^ c[5] ^ c[7];

      crc8_16in = c_out;
   end
endfunction

endmodule

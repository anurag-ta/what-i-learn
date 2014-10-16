`include "../../inc/defines.h"

module uu_acmac_crc32_8in(
	input             clk,    // Input clock
	input             rst_n,  // Input Reset
	input             crc_en, // Input Enable
	
	input             data_val, // Input Data Valid
	input      [7:0]  data,     // Input Data
	input      [15:0] len,      // Input Length
	output reg        crc_avl,  // Output CRC32 Valid
	output reg [31:0] crc       // Output CRC32
     );
	
	//Internal Registers
	reg [15:0] count;      
	reg [31:0] loc_crc;
	reg [15:0] loc_len;
	
	
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET) begin
			count   <= 0;
			loc_crc <= 0;
			loc_len <= 0;
			crc     <= 0;
			crc_avl <= 0;
		end
		else if(!crc_en) begin
			loc_len <= 0;
			count   <= 0;
			loc_crc <= 0;
			crc_avl <= 0;
			crc     <= 0;
		end
		else begin 
			loc_len <= len;
			if(!count) begin
				if(data_val) begin
					loc_crc <= nextCRC32_D8(data,32'hFFFF_FFFF);
					count   <= count + 16'h1;
				end
			end
			else if (count < loc_len) begin
				if (data_val) begin
					count <= count + 16'h1;
					loc_crc <= nextCRC32_D8(data,loc_crc); 
				end
			end
			else if(count == loc_len)begin
				crc_avl <= 1'b1;
				crc     <= finvrev(loc_crc);
			end
		end
	end
	
	wire [31:0] crc_temp_value = finvrev(loc_crc);
   function [31:0] nextCRC32_D8;

      input [7:0] Data;
      input [31:0] crcreg_int;
      reg [7:0]    dt;
      reg [31:0]   c;
      reg [31:0]   newcrc;
      begin
	 dt = Data;
	 c = crcreg_int;

	 //polynomial 04C11DB7

	 newcrc[0] = dt[7] ^ dt[1] ^ c[30] ^ c[24];
	 newcrc[1] = dt[7] ^ dt[6] ^ dt[1] ^ dt[0] ^ c[31] ^ c[30] ^ c[25] ^ c[24];
	 newcrc[2] = dt[7] ^ dt[6] ^ dt[5] ^ dt[1] ^ dt[0] ^ c[31] ^ c[30] ^ c[26]
           ^ c[25] ^ c[24];
	 newcrc[3] = dt[6] ^ dt[5] ^ dt[4] ^ dt[0] ^ c[31] ^ c[27] ^ c[26] ^ c[25];
	 newcrc[4] = dt[7] ^ dt[5] ^ dt[4] ^ dt[3] ^ dt[1] ^ c[30] ^ c[28] ^ c[27] 
           ^ c[26] ^ c[24];
	 newcrc[5] = dt[7] ^ dt[6] ^ dt[4] ^ dt[3] ^ dt[2] ^ dt[1] ^ dt[0] ^ c[31]
           ^ c[30] ^ c[29] ^ c[28] ^ c[27] ^ c[25] ^ c[24];
	 newcrc[6] = dt[6] ^ dt[5] ^ dt[3] ^ dt[2] ^ dt[1] ^ dt[0] ^ c[31] ^ c[30]
           ^ c[29] ^ c[28] ^ c[26] ^ c[25];
	 newcrc[7] = dt[7] ^ dt[5] ^ dt[4] ^ dt[2] ^ dt[0] ^ c[31] ^ c[29] ^ c[27]
           ^ c[26] ^ c[24];
	 newcrc[8] = dt[7] ^ dt[6] ^ dt[4] ^ dt[3] ^ c[28] ^ c[27] ^ c[25] ^ c[24]
           ^ c[0];
	 newcrc[9] = dt[6] ^ dt[5] ^ dt[3] ^ dt[2] ^ c[29] ^ c[28] ^ c[26] ^ c[25]
           ^ c[1];
	 newcrc[10] = dt[7] ^ dt[5] ^ dt[4] ^ dt[2] ^ c[2] ^ c[29] ^ c[27] ^ c[26]
           ^ c[24];
	 newcrc[11] = dt[7] ^ dt[6] ^ dt[4] ^ dt[3] ^ c[3] ^ c[28] ^ c[27] ^ c[25]
           ^ c[24];
	 newcrc[12] = dt[7] ^ dt[6] ^ dt[5] ^ dt[3] ^ dt[2] ^ dt[1] ^ c[4] ^ c[30] 
           ^ c[29] ^ c[28] ^ c[26] ^ c[25] ^ c[24];
	 newcrc[13] = dt[6] ^ dt[5] ^ dt[4] ^ dt[2] ^ dt[1] ^ dt[0] ^ c[5] ^ c[31]
           ^ c[30] ^ c[29] ^ c[27] ^ c[26] ^ c[25];
	 newcrc[14] = dt[5] ^ dt[4] ^ dt[3] ^ dt[1] ^ dt[0] ^ c[6] ^ c[31] ^ c[30]
           ^ c[28] ^ c[27] ^ c[26];
	 newcrc[15] = dt[4] ^ dt[3] ^ dt[2] ^ dt[0] ^ c[7] ^ c[31] ^ c[29] ^ c[28] 
           ^ c[27];
	 newcrc[16] = dt[7] ^ dt[3] ^ dt[2] ^ c[8] ^ c[29] ^ c[28] ^ c[24];
	 newcrc[17] = dt[6] ^ dt[2] ^ dt[1] ^ c[9] ^ c[30] ^ c[29] ^ c[25];
	 newcrc[18] = dt[5] ^ dt[1] ^ dt[0] ^ c[31] ^ c[30] ^ c[26] ^ c[10];
	 newcrc[19] = dt[4] ^ dt[0] ^ c[31] ^ c[27] ^ c[11];
	 newcrc[20] = dt[3] ^ c[28] ^ c[12];
	 newcrc[21] = dt[2] ^ c[29] ^ c[13];
	 newcrc[22] = dt[7] ^ c[24] ^ c[14];
	 newcrc[23] = dt[7] ^ dt[6] ^ dt[1] ^ c[30] ^ c[25] ^ c[24] ^ c[15];
	 newcrc[24] = dt[6] ^ dt[5] ^ dt[0] ^ c[31] ^ c[26] ^ c[25] ^ c[16];
	 newcrc[25] = dt[5] ^ dt[4] ^ c[27] ^ c[26] ^ c[17];
	 newcrc[26] = dt[7] ^ dt[4] ^ dt[3] ^ dt[1] ^ c[30] ^ c[28] ^ c[27] ^ c[24] 
           ^ c[18];
	 newcrc[27] = dt[6] ^ dt[3] ^ dt[2] ^ dt[0] ^ c[31] ^ c[29] ^ c[28] ^ c[25] 
           ^ c[19];
	 newcrc[28] = dt[5] ^ dt[2] ^ dt[1] ^ c[30] ^ c[29] ^ c[26] ^ c[20];
	 newcrc[29] = dt[4] ^ dt[1] ^ dt[0] ^ c[31] ^ c[30] ^ c[27] ^ c[21];
	 newcrc[30] = dt[3] ^ dt[0] ^ c[31] ^ c[28] ^ c[22];
	 newcrc[31] = dt[2] ^ c[29] ^ c[23];

	 nextCRC32_D8 = newcrc;
      end
   endfunction

   function [31:0] finvrev;
      input [31:0] dta;
      integer 	   i;
      begin 
	 for (i=0;i<32;i=i+1)
	   finvrev[i] = ~dta[31-i];
      end
   endfunction

endmodule
		

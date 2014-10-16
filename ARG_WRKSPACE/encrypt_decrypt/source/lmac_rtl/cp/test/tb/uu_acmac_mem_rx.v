module uu_acmac_mem_rx (
	input             clk             ,
	input             mem_rx_in_en    ,
	input             mem_rx_in_wen   ,
	input      [11:0] mem_rx_in_addr  ,
	input      [7 :0] mem_rx_in_data  ,
	output reg [7 :0] mem_rx_out_data 
	);
	reg [7:0] memory_rx [1535:0]; 
	always @(posedge clk)begin
		if(!mem_rx_in_en)begin
			mem_rx_out_data <= 0;
		end
		else if(!mem_rx_in_wen)begin
			mem_rx_out_data <= memory_rx [mem_rx_in_addr];
		end
		else begin
			mem_rx_out_data <= memory_rx [mem_rx_in_addr];
			memory_rx [mem_rx_in_addr] <= mem_rx_in_data;
		end
	end
endmodule




module uu_acmac_mem_tx_ctrl (
	input             clk                  ,
	input             mem_tx_ctrl_in_en    ,
	input             mem_tx_ctrl_in_wen   ,
	input      [7 :0] mem_tx_ctrl_in_addr  ,
	input      [7 :0] mem_tx_ctrl_in_data  ,
	output reg [7 :0] mem_tx_ctrl_out_data 
	);
	reg [7:0] memory_tx_ctrl [300:0]; 
	always @(posedge clk)begin
		if(!mem_tx_ctrl_in_en)begin
			mem_tx_ctrl_out_data <= 0;
		end
		else if(!mem_tx_ctrl_in_wen)begin
			mem_tx_ctrl_out_data <= memory_tx_ctrl [mem_tx_ctrl_in_addr];
		end
		else begin
			mem_tx_ctrl_out_data <= memory_tx_ctrl [mem_tx_ctrl_in_addr];
			memory_tx_ctrl [mem_tx_ctrl_in_addr] <= mem_tx_ctrl_in_data;
		end
	end
endmodule




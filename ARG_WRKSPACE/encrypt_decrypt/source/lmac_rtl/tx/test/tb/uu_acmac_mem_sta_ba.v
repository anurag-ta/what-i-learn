
module uu_acmac_mem_sta_ba (
	input             clk             ,
	input             mem_tx_in_en    ,
	input      [3 :0] mem_tx_in_wen   ,
	input      [13:0] mem_tx_in_addr  ,
	input      [31:0] mem_tx_in_data  ,
	output reg [31:0] mem_tx_out_data 
	);
	reg [31:0] memory_tx [12012:0]; 
	always @(posedge clk)begin
		if(!mem_tx_in_en)begin
			mem_tx_out_data <= 0;
		end
		else if(!mem_tx_in_wen)begin
			mem_tx_out_data <= memory_tx [mem_tx_in_addr];
		end
		else begin
			mem_tx_out_data <= memory_tx [mem_tx_in_addr];
		end
	end

        always @(posedge clk) 
        begin
	    if(!mem_tx_in_en)
            begin
                memory_tx [ mem_tx_in_addr ] <= 32'h0;
            end
            else 
            begin
                if(mem_tx_in_wen[0])
                begin 
                    memory_tx[mem_tx_in_addr][7:0] <= mem_tx_in_data[7:0];
                end
                if(mem_tx_in_wen[1])
                begin 
                    memory_tx[mem_tx_in_addr][15:8] <= mem_tx_in_data[15:8];
                end
                if(mem_tx_in_wen[2])
                begin 
                    memory_tx[mem_tx_in_addr][23:16] <= mem_tx_in_data[23:16];
                end
                if(mem_tx_in_wen[3])
                begin 
                    memory_tx[mem_tx_in_addr][31:24] <= mem_tx_in_data[31:24];
                end
            end
        end
endmodule




task automatic drive(input int sym_no);

if (sym_no == 1) begin
self_cts_flag = 0;
format = 0; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000000; 
end

if (sym_no == 2) begin
self_cts_flag = 1;
format = 0; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000000; 
end

if (sym_no == 3) begin
self_cts_flag = 0;
format = 1; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000000; 
end

if (sym_no == 4) begin
self_cts_flag = 0;
format = 2; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000010; 
end

if (sym_no == 5) begin
self_cts_flag = 0;
format = 3; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000000; 
end

if (sym_no == 6) begin
self_cts_flag = 1;
format = 1; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000000; 
end

if (sym_no == 7) begin
self_cts_flag = 1;
format = 2; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000010; 
end

if (sym_no == 8) begin
self_cts_flag = 1;
format = 3; 
rate = 1;
data_frame_input = 48'hfcf8f0102004;
postcondition = 16'b00000001_00000000; 
end

endtask
final begin
 if(err==0) begin
  $display("#############################################");
  $display("#############################################\n");
  $display("                TEST PASSED                \n");
  $display("#############################################");
  $display("#############################################\n");
 end else begin
  $display("#############################################");
  $display("#############################################\n");
  $display("                TEST FAILED                \n");
  $display("#############################################");
  $display("#############################################\n");
 end
end

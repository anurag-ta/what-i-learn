//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAP_LMAC state machine verification 														//
// This file contains a verilog test case to test LMAC of CAP module in the STATES_CHAIN							//
// Test Vectors data is taken from a file states_chain_test_cases.txt that is being used in c-design testing					//
// State machine of c-design is mapped to onehot encoding into RTL design.									//
// 																		//	
// Testing procedure :																//
// 1.Open a c-test text file for reading that contains test vectors										//
// 2.Reset and Enable the CAP module.It will be in IDLE state											//
// 3.Read a number that shows how many number of test vectors present in a chain 								//
// 4.Read a vector of chain from file							 							//
// 5.Map c-representation state-machine states to RTL state-machine onehot states 								//
// 6.Based on access category check the preconditions as defined in test vector									//
// 7.Drive inputs as specified in test vector 													//
// 8.Start a time tick generator if needed as per input												//
// 9.Check that the expected state reached by state machine											//
// 10.Based on access category check the post conditions as defined in test vector 							  	//
// 11.Repeat the steps 4 to 10 until all vectors in a chain completed					                			//
// 12.Repeat 2 to 11 until all chains of test vectors in the data file completed								//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

`timescale 1ns/10ps
module uu_acmac_states_chain_test;

integer fp , r ;
int  m = 0,  n = 0; // number of chains m , number of vectors in a chain n : manual editing to m needed as per test vectors file contents
int err = 0;

string test_chain_name, testcase_id , input_word , precondition , postcondition;

int event_type , ac , qos_mode , pre_event_timer_tick_count , post_event_timer_tick_count , frame_valid , frame_id;
int channel_list, event_status;
int nav_val, nav_reset_timer, main_sm_state, supp_sm_state ;
int nav_val_post, nav_reset_timer_post , main_sm_state_post , supp_sm_state_post;
int main_sm_state_onehot , main_sm_state_onehot_post;
int count = 0, return_value = 1, frame_id_reg ;

int rxhandler_ret_value, tx_ready , aifs_val ,backoff_val , tx_prog , tx_nav , ssrc , slrc , cw_val ;
int ack_timer_val , cts_timer_val , eifs_timer_val , cca_status , txop_owner_ac;
int tx_ready_post , aifs_val_post , backoff_val_post , tx_prog_post , tx_nav_post , ssrc_post , slrc_post ;
int cw_val_post , ack_timer_val_post , cts_timer_val_post , eifs_timer_val_post, cca_status_post , txop_owner_ac_post;

reg rts_frame_tb = 1'b0;
reg frame_valid_reg = 1'b0;

int i, j; // for loop local variables 
// The tasks that are specific to this chain test are kept in the test_common_chain.v file 
//`include "../../cap/test/scripts/test_common_chain.v" 

// Test vectors data is taken from a file states_chain_test_cases.txt that is being used in c-design testing
initial begin

     $display( "*********************************  LMAC_CAP_STATES_CHAIN TESTING  *****************************************" );
     //reset_cap();
     //repeat(5) @( negedge uu_acmac_cap_tb.clk ); // This delay is to wait until reset is complete
    	 
     //enable_cap( );

     fp = $fopen( "../../cap/test/vectors/uu_acmac_chain_test_cases.txt" , "r" );
     r = $fscanf( fp, "%d", m ); 

     repeat(5) @( negedge uu_acmac_cap_tb.clk ); // This delay is to wait until reset is complete

     @( negedge uu_acmac_cap_tb.clk );
     $display( " CAP_LMAC_STATES_CHAIN_test_new : Total number of chains = %0d  " , m );

     // Repeating the loop for 1 to m number of chains 
     for( i  = 1; i <=m ; i++ )
     begin 
    	 enable_cap( );

	 r = $fscanf( fp ,  "%d" , n); // number of test vectors in a chain 
         $display( " CAP_LMAC_STATES_CHAIN_test_new :For chain number = %0d => No.of vectors = %0d " , i  ,  n );
         frame_valid_reg = 1'b0;

       	 for( j  = 1; j <= n ; j++ ) // No of iterations in this loop depends on no. of test vectors in a chain 
       	 begin
             $display( " CAP_LMAC_STATES_CHAIN_test_new : Chain number = %0d and Vector number = %0d started " , i  ,  j );
             count = 0;
       
       	     // The c-variables are mapped to verilog variables in the RTL design.
             // So corresponding variables, events etc. read from file and are assigned
       	     read_test_vector_chain();

       	     // The below code is just to display what variables are read from the data file.
       	     display_test_variables_chain();

       	     // State machine of c-design is mapped to onehot encoding into verilog design.so the below code is needed
       	     map_states( );

             // initial context loading  
             load_initial_context(); 

             // Loading the Input Values Read from the Vectors.
             load_input_values();
             // one clock cycle delay just load default inputs to cap 
	     @( negedge uu_acmac_cap_tb.clk ); 

             // Making a frame available for Testing of AC queues
             load_frameinfo_valid();
             // one clock cycle delay just load default inputs to cap 
      	     @( negedge uu_acmac_cap_tb.clk ); 

	     // Before checking pre-conditions, Calculating number of timer ticks are applied to CAP.	
	     if( pre_event_timer_tick_count >= 65534 )begin 
   	         $display( $time , " Before Recalculation event_timer_tick_count=%0d " , pre_event_timer_tick_count );
	         calculate_timer_tick_count( main_sm_state , supp_sm_state , qos_mode , ac , pre_event_timer_tick_count);
   	         $display( $time , " After Recalculating event_timer_tick_count=%0d " , pre_event_timer_tick_count );
	     end 	

             fork
   	         $display($time," Post-timer ticks triggering ");
             
                 uu_acmac_cap_tb.cap_in_ev_timer_tick = 0;
                 if( pre_event_timer_tick_count > 0 )start_pulse_timer( pre_event_timer_tick_count );
     
                 repeat(5) @( negedge uu_acmac_cap_tb.clk );   
                 $display( "*********************************  PRE_CHECKS *****************************************" );
   	         check_pre_conditions_chain();
      	         $display( "*********************************  PRE_CHECKS COMPLETED********************************" );     
                 @( negedge uu_acmac_cap_tb.clk );

                 while(return_value)
                 begin	
   	            // Initial input signal driving 
                    initialize_event_type_inputs();
                              
                    // If a Response is required from PHY.
                    send_phy_response();            
                    @( negedge uu_acmac_cap_tb.clk );
 	
                    // Driving the Input Events from the Vector( Does not drive Timer Tick Events ).
                    drive_event_type_inputs(); 
       	                            	    
                    @( negedge uu_acmac_cap_tb.clk );
                    initialize_event_type_inputs();
                 end
             join

   	     // While inputs are driving into CAP module ,  at the same time . time_tick generator drives pulses at regular intervals of time
       	     fork
	        begin
   	            $display($time," Post-timer ticks triggering ");
       	            uu_acmac_cap_tb.cap_in_ev_timer_tick = 0;
	            if( post_event_timer_tick_count > 0 )start_pulse_timer( post_event_timer_tick_count ); // a task that generates time ticks to cap input
     	        end
 
      	        begin	
       	            @( negedge uu_acmac_cap_tb.clk );
       
       	            initialize_event_type_inputs();

                    $display( $time , " fork_join test flow ended " );
       	        end //a block of statements of fork-join end here
	    
             join // in paralle to test flow the start_pulse_timer() task is invoked.Both threads end here. 
           
             $display( $time , " fork_join all threads ended " );
 
       	     $display( "*********************************  POST_CHECKS *****************************************" );
       	     @( negedge uu_acmac_cap_tb.clk );
       	     // Checking for Post-conditions as defined in the test-vector
             check_post_conditions_chain(); 
       	     $display( "*********************************  POST_CHECKS COMPLETED *******************************" );

             return_value = 1;
             $display( $time , " Chain number = %0d  and Vector number = %0d completed \n " , i , j );
       	     $display( "################################################################################################ \n" );
         end // end of for(int j  = 1;j<  = n;j++) This loop completes a chain of vectors  
       
       	 // Disable the CAP module and reset it for next chain of test vectors
       	 disable_cap();
         @( negedge uu_acmac_cap_tb.clk );
         $display( $time , " Chain number = %0d completed  " , i );
       	 $display( "##################################################################################################################### \n" );

       	 reset_cap();
       	
         repeat(5) @( negedge uu_acmac_cap_tb.clk ); // Wait until the CAP module resets its internal states.
       	
     end // end of for(int i = 1; i <=m ; i++ ) loop .

     report_pass_fail();
     $finish(0);

   end // Initial block end 



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file contains all the tasks that are specific to states_chain_new in addition to test_common.v	   //	
// To reduce source code of test scripts the tasks that are common to all test scripts is put in this file //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This task reads all variables of a vector from test vectors data file 
task read_test_vector_chain();

   $display($time," Reading a vector from file ");
	
r = $fscanf( fp , "%s %d %d %d %d %d %d %d %s %d %h %s %d %d %d %d %s %d %d %d %d " , testcase_id, event_type, ac, qos_mode, pre_event_timer_tick_count, post_event_timer_tick_count, frame_valid, frame_id, input_word, event_status, channel_list, precondition, nav_val, nav_reset_timer, main_sm_state, supp_sm_state, postcondition,  nav_val_post, nav_reset_timer_post, main_sm_state_post, supp_sm_state_post );

endtask

task display_test_variables_chain();
   $display($time," Displaying test variables ");

$display("|------------------------------------------------------------------------------------------------------------------------------|");
// Input variables display
$display( "|\n| Inputs : \n| event_type = %2d , access category = %2d ,qos_mode = %0d , pre_event_timer_tick_count = %2d , post_event_timer_tick_count = %2d ,\n| frame_valid = %2d  " , event_type , ac , qos_mode , pre_event_timer_tick_count , post_event_timer_tick_count , frame_valid );

$display( "| frame_id : %4d , input_word : %s , event_status = %2d , channel_list/rxhandle_ret = %4h ", frame_id, input_word, event_status, channel_list);

// Preconditions display
$display( "|\n| %s  nav_val = %2d ,  nav_reset_timer = %2d ,main_sm_state=%2d , supp_sm_state=%2d ", precondition, nav_val, nav_reset_timer,  main_sm_state, supp_sm_state);

// Post conditions display 
$display( "|\n| %s nav_val_post=%2d , nav_reset_timer_post=%2d, main_sm_state_post:%2d , supp_sm_state_post:%2d \n| ", postcondition,  nav_val_post, nav_reset_timer_post, main_sm_state_post, supp_sm_state_post );
$display("|------------------------------------------------------------------------------------------------------------------------------|");

endtask


task check_pre_conditions_chain();
   $display($time," Checking preconditions ");
   check_common_pre_chain();	
endtask

task check_post_conditions_chain();
   $display($time," Checking postconditions ");
    // Based on access category post conditions are to be verified
    check_common_post_chain();
endtask

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

task send_txready_event();
     event_type = 4'd2; 
endtask

task send_txstart_response();
     event_type = 4'd3;
endtask

task send_txdata_response();
begin
     event_type = 4'd4;
end
endtask

task send_txend_response();
     // This task loads only a part of a frame data into CAP_LMAC
     $display($time," Loading the Input Frame ");
     @( negedge uu_acmac_cap_tb.clk );
     load_input_frame(); 
    event_type = 4'd5;
endtask

task send_cca_reset_request();
    event_type = 4'd11;
endtask

task send_txop_txstart_response();
    event_type = 4'd12;
    count = count + 1;
endtask

/////////////////////////////////////////////////////////////////////////////////////

task check_common_pre_chain();

   if( uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r == nav_reset_timer ) begin
   end else begin 
       err++;
       $display( $time , " ERROR : uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r %0d  nav_reset_timer: %0d \n" , uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r , nav_reset_timer );  
   end

   if( uu_acmac_cap_tb.U_CAP.curr_state == main_sm_state_onehot )begin 
   end else begin 
   err++;
   $display( $time," ERROR : uu_acmac_cap_tb.U_CAP.curr_state = %8b B , but main_sm_state_onehot = %8b B \n", uu_acmac_cap_tb.U_CAP.curr_state , main_sm_state_onehot );  
   end

   repeat(5) @(posedge uu_acmac_cap_tb.clk);
   if( uu_acmac_cap_tb.U_CAP.cap_nav_val_r == nav_val ) begin
   end else begin 
       err++;
       $display( $time , " ERROR : uu_acmac_cap_tb.U_CAP.cap_nav_val_r %0d  nav_val: %0d \n" , uu_acmac_cap_tb.U_CAP.cap_nav_val_r , nav_val );  
   end

endtask // check_common_pre_chain()


task check_common_post_chain();

   if( uu_acmac_cap_tb.U_CAP.cap_nav_val_r == nav_val_post  ) begin
	$display( $time , " PASS : uu_acmac_cap_tb.U_CAP.cap_nav_val_r %0d  nav_val: %0d \n" , uu_acmac_cap_tb.U_CAP.cap_nav_val_r , nav_val_post  );
   end else begin 
       err++;
       $display( $time , " ERROR : uu_acmac_cap_tb.U_CAP.cap_nav_val_r %0d  nav_val: %0d \n" , uu_acmac_cap_tb.U_CAP.cap_nav_val_r , nav_val_post  );  
   end

   if( uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r == nav_reset_timer_post  ) begin
	$display( $time , " PASS : uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r %0d  nav_reset_timer_post : %0d \n" , uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r , nav_reset_timer_post  );
   end else begin 
       err++;
       $display( $time , " ERROR : uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r %0d  nav_reset_timer_post : %0d \n" , uu_acmac_cap_tb.U_CAP.cap_nav_reset_timer_r , nav_reset_timer_post  );  
   end

   repeat(4) @( posedge uu_acmac_cap_tb.clk );
   if( uu_acmac_cap_tb.U_CAP.curr_state==main_sm_state_onehot_post  )begin 
	$display( $time," PASS : uu_acmac_cap_tb.U_CAP.curr_state = %8b B , but main_sm_state_onehot_post  = %8b B \n",uu_acmac_cap_tb.U_CAP.curr_state,main_sm_state_onehot_post  );
   end else begin 
   err++;
   $display( $time," ERROR : uu_acmac_cap_tb.U_CAP.curr_state = %8b B , but main_sm_state_onehot_post  = %8b B \n",uu_acmac_cap_tb.U_CAP.curr_state,main_sm_state_onehot_post  );  
   end

endtask // check_common_post_chain()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

task enable_cap();
     @( negedge uu_acmac_cap_tb.clk );
     $display($time," Enabling CAP module  ");
     uu_acmac_cap_tb.cap_in_enable = 1'b1;
endtask

task disable_cap();
     @( negedge uu_acmac_cap_tb.clk );
     $display($time," Disabling CAP module  ");
     uu_acmac_cap_tb.cap_in_enable = 1'b0;
endtask

task reset_cap();
     #10
     $display($time," Reseting CAP module ");
     uu_acmac_cap_tb.rst_n = 1'b0;
     #60; 
     uu_acmac_cap_tb.rst_n = 1'b1; // Resetting CAP module 
     $display($time," Reset Released ");
endtask


// The c-representation of states are mapped to RTL representation of onehot encoding of states
task map_states();
   $display($time," Mapping states ");

     // The c-representation of states are mapped to RTL representation of onehot encoding of states
  case( main_sm_state )
      3'd0 : begin main_sm_state_onehot = 0; end // IDLE
      3'd1 : begin main_sm_state_onehot = 1; end // EIFS
      3'd2 : begin main_sm_state_onehot = 2; end // CONTEND
      3'd3 : begin // TXOP 
     	     case( supp_sm_state )
                3'd0 : begin main_sm_state_onehot = 4; end  // TXOP 
                3'd1 : begin main_sm_state_onehot = 16; end // TXOP_SEND
                3'd2 : begin main_sm_state_onehot = 32; end // TXOP_WAIT_RX
                3'd3 : begin main_sm_state_onehot = 64; end // TXOP_TXNAVEND
                default : begin main_sm_state_onehot = 4; end
             endcase
     	end
      3'd4 : begin main_sm_state_onehot = 8; end // RX
      default : begin main_sm_state_onehot = 0; end
  endcase 	
  $display("{main_sm_state = %0d ,supp_sm_state = %0d }=> mapped main_sm_state_onehot = %8b  " , main_sm_state ,supp_sm_state , main_sm_state_onehot );

   // The c-representation of states are mapped to RTL representation of onehot encoding of states
   case( main_sm_state_post )
      3'd0 : begin main_sm_state_onehot_post = 0; end // IDLE
      3'd1 : begin main_sm_state_onehot_post = 1; end // EIFS
      3'd2 : begin main_sm_state_onehot_post = 2; end // CONTEND
      3'd3 : begin 
     	   case(supp_sm_state_post)
                   3'd0 : begin main_sm_state_onehot_post = 4; end   // TXOP 
                   3'd1 : begin main_sm_state_onehot_post = 16; end  // TXOP_SEND
                   3'd2 : begin main_sm_state_onehot_post = 32; end  // TXOP_WAIT_RX
                   3'd3 : begin main_sm_state_onehot_post = 64; end  // TXOP_TXNAVEND
                   default : begin main_sm_state_onehot_post = 4; end
                endcase
     	end
      3'd4 : begin main_sm_state_onehot_post = 8; end // RX
      default : begin main_sm_state_onehot_post = 0; end
  endcase     
  $display("{main_sm_state_post = %0d ,supp_sm_state_post = %0d}=> mapped main_sm_state_onehot_post = %8b  " , main_sm_state_post ,supp_sm_state_post , main_sm_state_onehot_post );

endtask

task enable_frameinfo_valid();
    case( ac )
        3'd0 : begin  uu_acmac_cap_tb.cap_in_ac0_txframeinfo_valid = 1; end 
        3'd1 : begin  uu_acmac_cap_tb.cap_in_ac1_txframeinfo_valid = 1; end
        3'd2 : begin  uu_acmac_cap_tb.cap_in_ac2_txframeinfo_valid = 1; end
        3'd3 : begin  uu_acmac_cap_tb.cap_in_ac3_txframeinfo_valid = 1; end
        3'd7 : begin  disable_frameinfo_valid(); end
    endcase
endtask

task disable_frameinfo_valid();
    uu_acmac_cap_tb.cap_in_ac0_txframeinfo_valid = 1'b0; 
    uu_acmac_cap_tb.cap_in_ac1_txframeinfo_valid = 1'b0; 
    uu_acmac_cap_tb.cap_in_ac2_txframeinfo_valid = 1'b0; 
    uu_acmac_cap_tb.cap_in_ac3_txframeinfo_valid = 1'b0; 
endtask

task send_phy_response();
   if( main_sm_state_onehot == 8)  // RECEIVE 
   begin
      if(( event_type == 1 ) || ( event_type == 5 ))
      begin
         if( main_sm_state_onehot_post == 0 )
         begin
            // uu_acmac_cap_tb.cap_in_txready = 0;
             disable_frameinfo_valid(); 
             return_value = 0;
         end
         if( main_sm_state_onehot_post == 8 )
         begin
             disable_frameinfo_valid(); 
            return_value = 0;
         end
         else
         begin
             $display( $time, " Send CCA RESET REQUEST from PHY " ); 
             if(( nav_val == 0 ) || ( nav_reset_timer == 0 ))
                send_cca_reset_request();
                return_value = 0;
         end
      end
      else if( event_type == 6 )
      begin
         if( main_sm_state_onehot_post == 0 )
         begin
           //  uu_acmac_cap_tb.cap_in_txready = 0;
             disable_frameinfo_valid();
         end
         else
             enable_frameinfo_valid();
         return_value = 0;
      end
      else if(( event_type == 9 ) && ( channel_list == 'hff1 ))
      begin
            $display( $time, " Send Tx Start RESPONSE from PHY " );
            send_txstart_response();
            return_value = 1; 
      end
      else if( event_type == 3 )
      begin
         $display( $time, " Send Tx Data Response from PHY " );
         send_txdata_response(); 
         return_value = 1;
      end
      else if( event_type == 4 )
      begin
         $display( $time, " Send Tx End Response from PHY " );
         send_txend_response();
         if( main_sm_state_onehot_post == 0 )
         begin
           //  uu_acmac_cap_tb.cap_in_txready = 0;
             disable_frameinfo_valid(); 
             return_value = 0;
         end
         else
             enable_frameinfo_valid();
         return_value = 1;
      end
      else if(( event_type == 9 ) && 
             (( channel_list == 'hff3 ) || ( channel_list == 'hff4 )))
      begin
         if( main_sm_state_onehot_post == 0 )
         begin
            // uu_acmac_cap_tb.cap_in_txready = 0;
             disable_frameinfo_valid(); 
         end
         else
             enable_frameinfo_valid();
         return_value = 0;
      end
      else
         return_value = 0;
   end
   else if( main_sm_state_onehot == 2)  // CONTEND
   begin
      if( event_type == 1 )
      begin
         if( main_sm_state_onehot_post == 0 )
         begin
            disable_frameinfo_valid();
            return_value = 0; 
         end
         else
         begin
            enable_frameinfo_valid();
            send_txready_event();
            return_value = 0;
         end
      end
      else 
          return_value = 0; 
   end
   else if( main_sm_state_onehot == 1) // EIFS 
   begin
      if( main_sm_state_onehot_post == 0 )
      begin
         // uu_acmac_cap_tb.cap_in_txready = 0;
          return_value = 0;
      end
      else if( event_type == 1 )
      begin
         if(( nav_val == 0 ) || ( nav_reset_timer == 0 ))
         begin
            $display( $time, " Send CCA RESET REQUEST from PHY " ); 
            send_cca_reset_request();
            return_value = 0;
            if( main_sm_state_post == 2 )
            begin
               send_txready_event();
               return_value = 1;
            end           
         end
      end
      else
         return_value = 0;
   end
   else if( main_sm_state_onehot == 4 )  // TXOP
   begin
      if( event_type == 12 ) 
      begin
         if( main_sm_state_onehot_post == 0)
         begin
             disable_frameinfo_valid();
             return_value = 0;
         end
         else if( count == 0 )
         begin
             $display( $time, " Send TXOP Tx Start RESPONSE from PHY " );
             send_txop_txstart_response();
             return_value = 1;
         end
         else       
         begin
             $display( $time, " Send Tx Start RESPONSE from PHY " );
             send_txstart_response();
             return_value = 1;
         end
      end
      else if( event_type == 3)
      begin
         $display( $time, " Send Tx Data Response from PHY " );
         send_txdata_response(); 
         return_value = 1;
      end
      else if( event_type == 4 )
      begin
         $display( $time, " Send Tx End Response from PHY " );
         send_txend_response();
         if( main_sm_state_onehot_post == 0 )
             disable_frameinfo_valid(); 
         return_value = 0;
      end
      else
         return_value = 0;
   end
   else if( main_sm_state_onehot == 16 )  // TXOP_SEND
   begin
       if( main_sm_state_onehot_post == 16 )
          enable_frameinfo_valid(); 
        return_value = 0;
   end
   else if( main_sm_state_onehot == 32 )  // TXOP_WAIT_RX
   begin
      if(( event_type == 9 ) && ( channel_list == 'hff1 ))
      begin
          if( main_sm_state_onehot_post == 2 )
          begin
              enable_frameinfo_valid();
          end
          $display( $time, " Send Tx Start RESPONSE from PHY " );
         send_txstart_response();
         return_value = 1; 
      end
      else if(( event_type == 9 ) && ( channel_list == 'hff3 ))
      begin
          if( main_sm_state_onehot_post == 0 ) 
          begin
             disable_frameinfo_valid();
          end
          $display( $time, " Send Tx Start RESPONSE from PHY " );
          return_value = 0;
      end
      else if(( event_type == 9) && ( channel_list == 'hff0 ))
      begin
         if( main_sm_state_onehot_post == 64 )
         begin
             //disable_frameinfo_valid();
         end   
         return_value = 0; 
      end
      else if( event_type == 3)
      begin
         $display( $time, " Send Tx Data Response from PHY " );
         send_txdata_response(); 
         return_value = 1;
      end
      else if( event_type == 4 )
      begin
         $display( $time, " Send Tx End Response from PHY " );
         send_txend_response();
         return_value = 1;
      end
      else if( event_type == 5 )
      begin
         $display( $time, " Send CCA RESET REQUEST from PHY " ); 
         if(( nav_val == 0 ) || ( nav_reset_timer == 0 ))
            send_cca_reset_request();
            return_value = 0;
      end
      else 
         return_value = 0;
   end
   else if( main_sm_state_onehot == 64 ) // TXOP_WAIT_TXNAV_END
   begin
      if( event_type == 1 )
      begin
         if( main_sm_state_onehot_post == 0 )
         begin
            uu_acmac_cap_tb.cap_in_enable = 0;
            disable_frameinfo_valid(); 
         end
         return_value = 0;
      end
      else
         return_value = 0;
   end
   else 
      return_value = 0;
endtask

task load_frameinfo_valid();
    disable_frameinfo_valid();
 
    // pre-timer ticks are applied before tx_ready event.So the txframeinfo_valid should be driven here. 
    if(( event_type == 2 ) || ( event_type == 12 ) || 
       ( main_sm_state_onehot == 32 ) || ( main_sm_state_onehot_post != 0 ))
    begin
        enable_frameinfo_valid();               
    end

endtask

task load_initial_context();
   $display($time," Initial context loading ");
   uu_acmac_cap_tb.cap_in_slot_timer_value  = 1;
   uu_acmac_cap_tb.cap_in_sifs_timer_value  = 16;
   uu_acmac_cap_tb.cap_in_difs_value  = 16+(2 * 1); // cap_in_sifs_timer_value + 2* cap_in_slot_timer_value ;
   uu_acmac_cap_tb.cap_in_eifs_value  = 16+(2 * 1) + 1 + 25; // cap_in_difs_timer_value + cap_in_ack_timer_value ;

   uu_acmac_cap_tb.cap_in_phy_rxstart_delay  = 25;
   uu_acmac_cap_tb.cap_in_ack_timer_value = 1 + 25; // cap_in_slot_timer_value + cap_in_phy_rxstart_delay;
   uu_acmac_cap_tb.cap_in_cts_timer_value =  1 + 25; // cap_in_slot_timer_value + cap_in_phy_rxstart_delay;
   
   uu_acmac_cap_tb.cap_in_cwmin_value  = 15;
   uu_acmac_cap_tb.cap_in_cwmax_value  = 1023;
   
   uu_acmac_cap_tb.cap_in_ac0_cwmin_value  = 15; 
   uu_acmac_cap_tb.cap_in_ac0_cwmax_value  = 1023; 
   uu_acmac_cap_tb.cap_in_ac0_aifs_value  = 7; 
   
   uu_acmac_cap_tb.cap_in_ac1_cwmin_value  = 15; 
   uu_acmac_cap_tb.cap_in_ac1_cwmax_value  = 1023; 
   uu_acmac_cap_tb.cap_in_ac1_aifs_value  = 3; ;
   
   uu_acmac_cap_tb.cap_in_ac2_cwmin_value  = 7; ;
   uu_acmac_cap_tb.cap_in_ac2_cwmax_value  = 15; ;
   uu_acmac_cap_tb.cap_in_ac2_aifs_value  = 2; ;  
   
   uu_acmac_cap_tb.cap_in_ac3_cwmin_value  = 3; 
   uu_acmac_cap_tb.cap_in_ac3_cwmax_value  = 7; 
   uu_acmac_cap_tb.cap_in_ac3_aifs_value  = 2; 
   
   uu_acmac_cap_tb.cap_in_txvec_format  = 0;
   uu_acmac_cap_tb.cap_in_rts_threshold_len  = 1500;

   uu_acmac_cap_tb.cap_in_mac_addr  = 48'h021111111102; 
   uu_acmac_cap_tb.cap_in_rxhandler_ret_value = 0 ;

   uu_acmac_cap_tb.cap_in_short_retry_count = 8'h8;
   uu_acmac_cap_tb.cap_in_long_retry_count = 8'h8;

endtask

task load_input_values();

   uu_acmac_cap_tb.cap_in_qos_mode = qos_mode;
   uu_acmac_cap_tb.cap_in_cca_status = { 1'b0, ( event_status[0] | channel_list[0] ) };
   uu_acmac_cap_tb.cap_in_frame_valid  = 0;
       	    
   // input argument is considered as channel_list or rxhandler_ret_vale based on event_type = 6	
   if( event_type == 6 )begin 
       uu_acmac_cap_tb.cap_in_channel_list  = channel_list;
       uu_acmac_cap_tb.cap_in_rxhandler_ret_value = 0 ;
   end
   else begin 
       uu_acmac_cap_tb.cap_in_channel_list  = 1'b0;
       uu_acmac_cap_tb.cap_in_rxhandler_ret_value = channel_list ;
   end	

   if( frame_valid )
   begin
       frame_id_reg = frame_id; 
       frame_valid_reg = 1'b1;
   end

   uu_acmac_cap_tb.cap_in_txvec_chbw  = 1;

endtask

// The pre/pro timer ticks count is re-calculated if it is 65535 value
task calculate_timer_tick_count( input int main_sm_state ,input int supp_sm_state, input int qos_mode , input int ac ,inout int event_timer_tick_count );
   $display($time," Invoked calculate_timer_tick_count task with event_timer_tick_count=%0d  ",event_timer_tick_count);
	case(main_sm_state)	
	   4'd0 : begin event_timer_tick_count=0; end // idle 
	   4'd1 : begin 
			if(event_timer_tick_count==65535) begin event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_eifs_r; end
			else begin event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_eifs_r - 2; end
			 
		  end // eifs state
	   4'd2 : begin // contend state
		     if(event_timer_tick_count==65535)begin
		        case(ac)
			   3'h0 : begin event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ac0_aifs_r + uu_acmac_cap_tb.U_CAP.cap_ac0_backoff_r; 
                                  uu_acmac_cap_tb.cap_in_ac0_txframeinfo_valid = 1'b1;  end  
			   3'h1 : begin event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ac1_aifs_r + uu_acmac_cap_tb.U_CAP.cap_ac1_backoff_r; 
                                  uu_acmac_cap_tb.cap_in_ac1_txframeinfo_valid = 1'b1;  end  
			   3'h2 : begin event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ac2_aifs_r + uu_acmac_cap_tb.U_CAP.cap_ac2_backoff_r;  
                                  uu_acmac_cap_tb.cap_in_ac2_txframeinfo_valid = 1'b1;  end  
			   3'h3 : begin event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ac3_aifs_r + uu_acmac_cap_tb.U_CAP.cap_ac3_backoff_r;  
                                  uu_acmac_cap_tb.cap_in_ac3_txframeinfo_valid = 1'b1;  end  
		        endcase 
		      end
		      else 
			 event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ack_timer_r - 2; 
		  end 
	   4'd3 : begin //TXOP state
		     case(supp_sm_state) 
			2'h0 : begin event_timer_tick_count=0;end  
			2'h1 : begin event_timer_tick_count=0;end  
			2'h2 : begin  // main_sm_state = TXOP and supp_sm_state = TXOP_WAIT_RX 
				   rts_frame_tb = uu_acmac_cap_tb.U_CAP.rts_frame;
				   
				   if(rts_frame_tb==1'b1)
                                   begin
					if(event_timer_tick_count==65535) 
                                        begin
                                             if( supp_sm_state_post != 3 )
                                                 event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_cts_timer_r + 1; 
                                             else
                                                 event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_cts_timer_r; 
                                        end
					else 
                                        begin 
                                                 event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_cts_timer_r - 2; 
                                        end
				   end
				   else 
                                   begin
				   	if(event_timer_tick_count==65535) 
                                        begin 
                                             if( supp_sm_state_post != 3 )
                                                 event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ack_timer_r + 1; 
                                             else
                                                 event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ack_timer_r; 
                                        end
					else  
                                        begin 
                                                 event_timer_tick_count = uu_acmac_cap_tb.U_CAP.cap_ack_timer_r - 2; 
                                        end
				   end
	 
			       end  // wait_for_rx state 
			2'h3: begin //TXOP TXNAVEND
				 if(event_timer_tick_count==65535) begin 
			            case(ac)
		                       2'h0 : begin 
					         if(uu_acmac_cap_tb.U_CAP.cap_out_ac0_txnav_value==0)event_timer_tick_count=1;
						 else event_timer_tick_count=uu_acmac_cap_tb.U_CAP.cap_out_ac0_txnav_value;
					       end  
		               	       2'h1 : begin 
					         if(uu_acmac_cap_tb.U_CAP.cap_out_ac1_txnav_value==0)event_timer_tick_count=1;
						 else event_timer_tick_count=uu_acmac_cap_tb.U_CAP.cap_out_ac1_txnav_value;
					      end  
		               	       2'h2 : begin 
					         if(uu_acmac_cap_tb.U_CAP.cap_out_ac2_txnav_value==0)event_timer_tick_count=1;
						 else event_timer_tick_count=uu_acmac_cap_tb.U_CAP.cap_out_ac2_txnav_value;
					      end  
		               	       2'h3 : begin 
					         if(uu_acmac_cap_tb.U_CAP.cap_out_ac3_txnav_value==0)event_timer_tick_count=1;
						 else event_timer_tick_count=uu_acmac_cap_tb.U_CAP.cap_out_ac3_txnav_value;
					      end  
		                     endcase
				 end
				 else begin
				    event_timer_tick_count=ack_timer_val-1;
				 end
			      end  
	   	        default : begin event_timer_tick_count=0;end 
		     endcase	
		  end 
	   4'd4 : begin 
		     if(event_timer_tick_count==65535)event_timer_tick_count = nav_val + 1;
		     else
		        event_timer_tick_count = ack_timer_val-2; 	 
		     	
		  end // receiving state 
	   default : begin event_timer_tick_count=0;end 
	endcase
   $display( $time , " Recalculated event_timer_tick_count=%0d " , event_timer_tick_count );
endtask

//This task generates time ticks pulses at every 1us interval .This task is invoked in parallel with test flow  
task start_pulse_timer( input int n );
    $display( $time," start_pulse_timer task() invoked with number of pulses =%0d ",n );
    while( n > 0  )
    begin
         repeat(199)@( negedge uu_acmac_cap_tb.clk );
            uu_acmac_cap_tb.cap_in_ev_timer_tick=1'b1;

         @( negedge uu_acmac_cap_tb.clk );
            uu_acmac_cap_tb.cap_in_ev_timer_tick=1'b0;

         n=n-1;	
         $display( $time," start_pulse_timer task() generated a pulse ,remaining number of pulses = %0d",n );
    end
    $display( $time," start_pulse_timer task() ended " );
endtask

// This task is to drive initial values to inputs that are dependent on event_type
task initialize_event_type_inputs();
     $display($time," Setting event_type based inputs to all zeros");
     uu_acmac_cap_tb.cap_in_ev_mac_reset = 0; 
     uu_acmac_cap_tb.cap_in_ev_timer_tick = 0;	
     uu_acmac_cap_tb.cap_in_ev_txready = 0;
     uu_acmac_cap_tb.cap_in_ev_txstart_confirm = 0;
     uu_acmac_cap_tb.cap_in_ev_txdata_confirm = 0;
     uu_acmac_cap_tb.cap_in_ev_txend_confirm = 0;
     uu_acmac_cap_tb.cap_in_ev_phyenergy_ind = 0;
     uu_acmac_cap_tb.cap_in_ev_rxstart_ind = 0;
     uu_acmac_cap_tb.cap_in_ev_rxdata_ind = 0;
     uu_acmac_cap_tb.cap_in_ev_rxend_ind = 0;
     uu_acmac_cap_tb.cap_in_ev_phyrx_error = 0;
     uu_acmac_cap_tb.cap_in_ev_ccareset_confirm = 0;

endtask

task drive_event_type_inputs();
    $display($time," Driving event_type based input ");
// Based on event type : pre/post timer tics and driving inputs will differ , so the case st. is to be elborated as per c-test
   case( event_type )
         4'd0  : begin uu_acmac_cap_tb.cap_in_ev_mac_reset = 1;          end  
         4'd1  : begin                                               end
         4'd2  : begin uu_acmac_cap_tb.cap_in_ev_txready = 1;            end     
         4'd3  : begin uu_acmac_cap_tb.cap_in_ev_txstart_confirm = 1;    end  
         4'd4  : begin uu_acmac_cap_tb.cap_in_ev_txdata_confirm = 1;     end  
         4'd5  : begin uu_acmac_cap_tb.cap_in_ev_txend_confirm = 1;      end  
         4'd6  : begin uu_acmac_cap_tb.cap_in_ev_phyenergy_ind =1;       end  
         4'd7  : begin uu_acmac_cap_tb.cap_in_ev_rxstart_ind = 1;        end  
         4'd8  : begin uu_acmac_cap_tb.cap_in_ev_rxdata_ind = 1;         end  
         4'd9  : begin uu_acmac_cap_tb.cap_in_ev_rxend_ind = 1;          end  
         4'd10 : begin uu_acmac_cap_tb.cap_in_ev_phyrx_error = 1;        end  
         4'd11 : begin uu_acmac_cap_tb.cap_in_ev_ccareset_confirm = 1;   end  
       default : begin initialize_event_type_inputs();       end 
   endcase	  
endtask


// This task is to be updated to load an input frame 
task load_input_frame();
   if( frame_valid_reg && (( event_type == 8 ) || ( event_type == 4 )))
   begin
       // This task is to check that few bytes of input frames loads into LMAC 	
       // header_len_calculate();

       $display($time," Loading input frame ");
       @( negedge uu_acmac_cap_tb.clk );
       if( frame_id_reg == 32'd1001 )
           ack_frame_generate();
       else if( frame_id_reg == 32'd1002 )
           multicast_frame_generate();
       else if( frame_id_reg == 32'd1003 )
           invalid_baframe_generate();
       else if( frame_id_reg == 32'd1004 )
           valid_baframe_generate();
       else if( frame_id_reg == 32'd1005 )
           cts_rxframe_generate();
       else if( frame_id_reg == 32'd1006 )
           cts_frame_generate();
       else if( frame_id_reg == 32'd1007 )
           invalid_data_generate();
       else if(( frame_id_reg == 32'd1008 ) || ( frame_id_reg == 32'd1009 ))
           valid_data_generate();
       else if( frame_id_reg == 32'd1010 )
           qos_data_generate();
       else if( frame_id_reg == 32'd1011 )
           rts_frame_generate();
       $display( $time, " Generated Frame " );
   end 
endtask

task ack_frame_generate();
    //d4|00|0f|00|02|11|11|11|11|02|ff|ff|ff|
 
    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'hd4;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h0f;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task multicast_frame_generate();
    //d4|00|0f|00|11|11|11|11|11|02|ff|ff|ff|
 
    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'hd4;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h0f;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task invalid_baframe_generate();
    // b4|88|0f|00|02|11|11|11|11|02|22|22|22|22|22|22|01|60|00|00|10|20|30|40|50|60|70|80|ff|ff|ff|
 
    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'hb4;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h88;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h0f;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h01;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h60;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h10;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h20;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h30;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h40;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h50;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h60;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h70;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h80;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task  valid_baframe_generate();
// 08|88|0f|00|02|11|11|11|11|02|22|22|22|22|22|22|01|60|00|00|10|20|30|40|50|60|70|80|ff|ff|ff|ff|
 
    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'h08;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h88;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h0f;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h01;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h60;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h10;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h20;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h30;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h40;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h50;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h60;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h70;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h80;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task cts_rxframe_generate();
    // c4|00|0f|00|22|22|22|22|22|22|ff|ff|ff|

    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'hc4;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h0f;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task cts_frame_generate();
// c4|00|0f|00|11|11|11|11|11|11|ff|ff|ff|

    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'hc4;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h0f;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task invalid_data_generate();
    // 88|00|0f|00|ff|11|11|11|11|02|ff|ff|ff|

    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'h88;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h0f;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task valid_data_generate();
    // 08|00|20|00|f0|ff|ff|ff|ff|ff|22|22|22|22|22|22|33|44|33|44|55|66|77|44|33|00|00|00|00| 

    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'h08;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h20;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hf0;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h33;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h44;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h33;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h44;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h55;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h66;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h77;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h44;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h33;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task qos_data_generate();
    // 88|00|20|00|f0|ff|ff|ff|ff|ff|22|22|22|22|22|22|fc|f8|f0|10|20|02|01|01|60|00|44|33|44|55|66|77|44|33|00|00|00|00|

    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'h88;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h20;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hf0;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hfc;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hf8;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hf0;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h10;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h20;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h02;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h01;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h01;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h60;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h44;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h33;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h44;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h55;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h66;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h77;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h44;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h33;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

task rts_frame_generate();
    // b4|00|f0|00|22|22|11|11|11|11|33|33|22|22|22|22|ff|ff|ff|ff| 

    uu_acmac_cap_tb.cap_in_frame_valid  = 1;
    uu_acmac_cap_tb.cap_in_frame  = 8'hb4;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hf0;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h00;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h11;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h33;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h33;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'h22;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame  = 8'hff;
    @( negedge uu_acmac_cap_tb.clk );
    uu_acmac_cap_tb.cap_in_frame_valid  = 0;
endtask

//This task reports PASS/FAIL based on total number of errors during all checks
task report_pass_fail();
   $display($time," Reporting pass/fail ");
   if( err == 0 ) begin
        $display( "############################################################" );
        $display( "## LMAC_CAP_TEST PASSED ,total errors = %0d            #####",err );
        $display( "############################################################" );
   	end else begin
        $display( "############################################################" );
        $display( "## LMAC_CAP_TEST FAILED ,total errors = %0d            #####",err );
        $display( "############################################################" );
   end

endtask

endmodule








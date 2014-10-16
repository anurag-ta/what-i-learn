#ifndef __UU_WLAN_RX_TEST_MAIN_H__
#define __UU_WLAN_RX_TEST_MAIN_H__ 

#include "uu_datatypes.h"


#define FRAME_HEX_DELIMITER   '|'
#define FRAME_END_DELIMITER   '@'
#define FREAD_BUFF_SIZE       1000 
#define INPUT_SCRIPT_FILE     "uu_wlan_test_script.txt"

/* rx_end_status is the reurn_val from phy_end_ind */
extern uu_int32 rx_end_status;

/* Expected output values for test-input */
extern uu_uint64 expected_tsf_g;
extern uu_bool ps_enable_g;
extern uu_bool dtim_bcn_g;
extern uu_bool eosp_g;
extern uu_bool more_data_g;

/** Function to read the frame from file 
 * For processing of test-input one i/p is frame to get that from file 
 * @param[in] file which contains frame 
 * @param[in] array- the frame in file is stored into this after processing of function 
 * Returns length of frame
 */
extern uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame);

/** Function to process every test-input
 * After reading of every test-input from input file it is called for processing  
 * @param[in] ev_type(7, 8, or 9 i.e rx_start, data, end indiactions respectively) 
 * @param[in] buffer contains i/ps and expected o/ps
 * Returns UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 */
extern uu_int32 uu_wlan_rx_test_handler(uu_int32 event_type, uu_char *buf);

/** Function to get expected output values from file
 * In this file one is i/p and remaining all are expected o/ps
 * @param[in] input_file which contains expected o/p values
 * returns UU_FAILURE if file is not existed, UU_SUCCESS on other case
 */
extern uu_int32 uu_wlan_test_get_expected_output(uu_char *input_file);

#endif /* __UU_WLAN_RX_TEST_MAIN_H__ */


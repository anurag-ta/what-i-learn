#ifndef __UU_WLAN_TEST_MAIN_H__
#define __UU_WLAN_TEST_MAIN_H__ 

#include "uu_datatypes.h"

#define FRAME_HEX_DELIMITER   '|'
#define FRAME_END_DELIMITER   '@'
#define FREAD_BUFF_SIZE       1000 
#define INPUT_SCRIPT_FILE     "uu_wlan_test_script.txt"

/** To maintain the success_rate of testing
 * uu_wlan_test_success_cnt_g is the no.of inputs passed succesfully
 * uu_wlan_test_failure_cnt_g is count of failed inputs
*/
extern uu_int32 uu_wlan_test_success_cnt_g;
extern uu_int32 uu_wlan_test_failure_cnt_g;

enum test_category
{
    /*Processing of rcvd frame*/
    RX_PATH =1,
    /*for testing of BA establishment*/
    ADD_BA,
    /*for deleting BA*/
    DEL_BA,
    /*for testing score-board updation*/
    SB_UPDATE,
    /*testing of BA-response generation*/
    BAR_GEN,
    /*Upon receiving rts frame cts frame generation*/
    TX_PATH
};

/**	function to read the frame from file 
 * For processing of testcase one i/p is frame, to get that from file 
 * @param[in] file which contains frame 
 * @param[in] to store frame into this array 
 * Returns length of frame
 */
extern uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame);

/**	function to process every testinput
 * After reading of every testinput from input file it is called for processing	
 * @param[in] test_category_id(1-6) 
 * @param[in] buffer contains i/ps and expected o/ps
 * Returns UU_FAILURE if fails or UU_SUCCESS in success
 */
extern uu_int32 uu_wlan_cp_test_handler(uu_int32 rq_type, uu_char *buf);

#endif  /* __UU_WLAN_TEST_MAIN_H__ */


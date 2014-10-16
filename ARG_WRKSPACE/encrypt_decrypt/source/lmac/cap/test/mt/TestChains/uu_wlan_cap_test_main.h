#ifndef __UU_WLAN_CAP_TEST_MAIN_H__
#define __UU_WLAN_CAP_TEST_MAIN_H__ 

#include "uu_datatypes.h"

#define FRAME_HEX_DELIMITER   '|'
#define FRAME_END_DELIMITER   '@'
#define FREAD_BUFF_SIZE       5000 
#define INPUT_SCRIPT_FILE     "uu_wlan_test_script.txt"


/** This function is to read the frame from file 
 * For processing of testcase one i/p is frame to get that from file 
 * @param[in] file which contains frame 
 * @param[in] to store frame into this array 
 * Return length of frame
 */
extern uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame);

/** This function process every testcase
 * After reading of every testcase from input file it is called for processing  
 * @param[in]  event_type ranges from 1-13, can be other than 1-13 also but no action will be performed 
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 */
extern uu_int32 uu_wlan_cap_test_handler(uu_int32 event_type, uu_char *buf);

#endif  /* __UU_WLAN_CAP_TEST_MAIN_H__ */

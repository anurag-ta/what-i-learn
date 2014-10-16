#ifndef __UU_WLAN_TX_TEST_MAIN_H__
#define __UU_WLAN_TX_TEST_MAIN_H__ 

#include "uu_datatypes.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_reg.h"

#define FRAME_HEX_DELIMITER   '|'
#define FRAME_END_DELIMITER   '@'
#define FREAD_BUFF_SIZE       1000 
#define INPUT_SCRIPT_FILE     "uu_wlan_test_script.txt"


/* Possible test_categories for transmisssion of frames */
enum test_category_id
{
	/* For transmission of single frame */
    TEST_SINGLE_FRAME = 1,
	/* For transmission of two frames */
    TEST_TWO_FRAME,
	/* For generation aggr packet from three frames */
    TEST_THREE_FRAME_AGGR,
	/* For transmission of single frame which is of length grater than rts_threshold */
    TEST_SINGLE_FRAME_THRESHOLD,
	/* For testing of reaggregation multiple times */
    TEST_RE_AGGR,
	/* Testing of wrap-aroungd case for mpdus */	
    TEST_WRAP_AROUND_MPDU,
	/* For generation of ps_poll frame */
    TEST_PS_POLL,
	/* For generation of qos_null data frame */
    TEST_QOS_NULL_DATA
};


/* File which contains rts frame testing txm. of frame > threshold */
extern uu_char rts_cts_fname[100];

/* frame_phy is the frame received at phy end*/
extern uu_uchar frame_phy[65535];

/** To maintain the success_rate of testcase
 * uu_wlan_test_success_cnt_g is the no.of cases passed succesfully
 * uu_wlan_test_failure_cnt_g is count of failed cases
 */
extern uu_int32 uu_wlan_test_success_cnt_g;
extern uu_int32 uu_wlan_test_failure_cnt_g;


/** This function process every testcase
 * After reading of every testcase from input file it is called for processing  
 * @param[in] test_category_id(1-8) 
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 */
extern uu_int32 uu_wlan_tx_test_handler(uu_char *buf);

/** This function is to read the frame from file 
 * For processing of testcase one i/p is frame to get that from file 
 * @param[in] file which contains frame 
 * @param[in] to store frame into this array 
 * Return length of frame
 */
extern uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame);

#endif /* __UU_WLAN_TX_TEST_MAIN_H__ */


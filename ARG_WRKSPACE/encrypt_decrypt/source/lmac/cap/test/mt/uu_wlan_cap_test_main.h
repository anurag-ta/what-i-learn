#ifndef __UU_WLAN_CAP_TEST_MAIN_H__
#define __UU_WLAN_CAP_TEST_MAIN_H__ 

#include "uu_datatypes.h"
#include "uu_wlan_rx_if.h"

#define FRAME_HEX_DELIMITER   '|'
#define FRAME_END_DELIMITER   '@'
#define FREAD_BUFF_SIZE       5000 
#define INPUT_SCRIPT_FILE     "uu_wlan_test_script.txt"


extern uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame);
extern uu_int32 uu_wlan_cap_test_handler(uu_int32 event_type, uu_char *buf);

extern void uu_wlan_validate_output(uu_int32 ret,uu_char *file_name,uu_char *buf);
extern void uu_wlan_phy_txstart_req(unsigned char *data, int len);
extern void uu_wlan_phy_data_req(uu_uchar data, uu_uchar *confirm_flag);
extern void uu_wlan_phy_txend_req(void);
extern void uu_wlan_phy_ccareset_req(void);
extern int uu_wlan_rx_handle_phy_rxstart(uu_wlan_rx_vector_t  *rx_vec);
extern int uu_wlan_rx_handle_phy_dataind(uu_uchar  data);
extern int uu_wlan_rx_handle_phy_rxend(uu_uchar  rxend_stat);

extern void uu_wlan_set_default_reg_values(void);
#endif  /* __UU_WLAN_CP_TEST_MAIN_H__ */

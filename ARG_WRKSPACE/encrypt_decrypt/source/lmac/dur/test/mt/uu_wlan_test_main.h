#ifndef __UU_WLAN_TEST_MAIN_H__
#define __UU_WLAN_TEST_MAIN_H__ 

#include "uu_datatypes.h"

#define FRAME_HEX_DELIMITER   '|'
#define FRAME_END_DELIMITER   '@'
#define FREAD_BUFF_SIZE       1000 
#define OUTPUT_CAP_FILE       "output.log"
#define INPUT_SCRIPT_FILE     "uu_wlan_test_script.txt"
#if 1
typedef struct uu_wlan_cp_rx_frame_info
{
    //uu_frame_details_t frameInfo;
    /** Byte 0 of Frame Control field.
     * Special case: Set to IEEE80211_FC0_TYPE_RESERVED, to trigger check for
     *   BA generation (like 'is_ampdu_end'), when:
     *      - Rx failed while receiving some MPDU of an AMPDU.
     *      - Last MPDU has got validation error.
     */
    uu_uint8 fc0; /* 8 bits */

    /** QoS ack policy - 0(normal ack / implicit-BA), 1(no ack), 2(PSMP ack), 3(MPDU under BA).
     * If used with BAR and BA frames, only 0 & 1 are applicable.
     */
    uu_uint8 qos_ack_policy; /* 2 bits */

    uu_uint16 bar_fc_field; /* 2 bytes */
    uu_uint8 bar_type; /* 2 bits */
    uu_uint8 tid; /* 4 bits */

    /* Duration of the received packet */
    uu_uint16 duration; /* 16 bits */

    /* Sequence number of the packet */
    uu_uint16 scf; /* 16 bits */
      
    /* sender address */
    uu_uchar ta[6]; /* 6 bytes */

    /* receipent address */
    uu_uchar ra[6]; /* 6 bytes */

    /** Format of the PPDU (non-HT, HT-MF, HT-GF, VHT) */
    uu_uint8 format; /* 2 bits */

    /** L_datarate 
     * NON-HT: Indicates the rate used to transmit the PSDU in megabits per second.
     * HT-MF: Indicates the data rate value that is in the L-SIG. This use is defined in 9.13.4.
     * HT-GF: Not present.
     * used in calculating Duration
     */
    uu_uint8 L_datarate; /* 4 bits */

     /** MCS 
     * HT-MF and HT-GF: Selects the modulation and coding scheme used in the transmission of the
     * packet. The value used in each MCS is the index defined in 20.6.
     * Integer: range 0 to 76. Values of 77 to 127 are reserved.
     * The interpretation of the MCS index is defined in 20.6.
     * used in calculating Duration
     */
    uu_uint8 mcs; /* 7 bits */
    /** NON-HT-MODULATION
     * used in calculating Duration
     */
    uu_uint8 modulation; /* 3 bits */

    /** Aggregation 
     * HT-MF and HT-GF: Indicates whether the PSDU contains an A-MPDU.
     *    Value 0 indicates that the packet is not aggregated and 1 indicates aggregated packet.
     */
    uu_uint8 is_aggregated; /* 1 bit */

    /** Indicates end of AMPDU (Rx-End from PHY, during AMPDU receiving).
     * CP module has to check whether an Implicit-BA has to be generated.
     */
    uu_uint8 is_ampdu_end; /* 1 bit */
    //uu_uint32 psdu_length;/* 20 bits */
} uu_wlan_cp_rx_frame_info_t;
extern uu_wlan_tx_frame_info_t tx_ctl_frame_info_g;
#endif
extern uu_int32 uu_wlan_test_success_cnt_g;
extern uu_int32 uu_wlan_test_failure_cnt_g;

extern uu_void  CAPTURE_STRING(uu_char *string);
extern uu_void  CAPTURE_FRAME(uu_uchar *mpdu, uu_int32 length);
extern uu_int32 uu_wlan_test_get_frame_from_file(uu_char *file_name, uu_uchar *frame);
extern uu_int32 uu_wlan_duration_test_handler(uu_int32 rq_type, uu_char *buf);

#endif  /* __UU_WLAN_TEST_MAIN_H__ */
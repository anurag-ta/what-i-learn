/* Currently this file is not being used.
Merged into uu_wlan_mac_stub.h */

#ifndef __UU_UMAC_STUB_STUN_NL_H__
#define __UU_UMAC_STUB_STUN_NL_H__

#define UU_PHY_STUB_HDR_SZ   40
#define UU_PHY_STUB_MAX_MPDU_SZ (4096+UU_PHY_STUB_HDR_SZ)  
#define UU_PHY_STUB_TDMAC_FC_BYTE  0x88 /* Changed for testing */


extern int  uu_umac_stub_nl_send_frame(unsigned char *frame, int len);

extern int uu_umac_stub_start_rx_proc(unsigned char *data, int len);

#endif
/* EOF */



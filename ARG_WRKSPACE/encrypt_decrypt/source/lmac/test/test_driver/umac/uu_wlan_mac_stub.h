
#ifndef __UU_WLAN_MAC_STUB_H__
#define __UU_WLAN_MAC_STUB_H__

extern int uu_umac_stub_indication(unsigned char *data, int len);

extern int  uu_umac_stub_nl_send_frame(unsigned char *frame, int len);

extern int uu_umac_config_registers(unsigned char *data,int len);

extern uu_void uu_wlan_umac_cbk(uu_int8 ind);

#endif /* __UU_WLAN_MAC_STUB_H__ */

/* EOF */


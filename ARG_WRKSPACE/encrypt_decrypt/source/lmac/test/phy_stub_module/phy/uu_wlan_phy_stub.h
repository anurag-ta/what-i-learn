
#ifndef __UU_WLAN_PHY_STUB_H__
#define __UU_WLAN_PHY_STUB_H__


typedef enum phy_events {
	UU_PHY_INVALID_EVENT = -1,
	UU_PHY_DATA_REQ,
	UU_PHY_DATA_CONF,
	UU_PHY_DATA_IND,
	UU_PHY_TXSTART_REQ,
	UU_PHY_TXSTART_CONF,
	UU_PHY_TXEND_REQ,
	UU_PHY_TXEND_CONF,
	UU_PHY_RXSTART_IND,
	UU_PHY_RXEND_IND,
	UU_PHY_CCARESET_REQ,
	UU_PHY_CCARESET_CONF,
	UU_PHY_CCA_IND
} phy_events_t;



extern int uu_phy_stub_indication(unsigned char *data, int len);

extern int uu_phy_stub_proc_nl_rx_frame(unsigned char *data, int len);

extern int uu_wlan_phy_stub_start(void);

extern int uu_wlan_phy_stub_stop(void);

extern void uu_wlan_phy_stub_schedule_work(void);

#endif /* __UU_WLAN_PHY_STUB_H__ */

/* EOF */


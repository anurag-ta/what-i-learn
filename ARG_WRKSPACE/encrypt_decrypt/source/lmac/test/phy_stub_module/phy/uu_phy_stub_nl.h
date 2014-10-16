#ifndef __UU_PHY_STUB_NL_H__
#define __UU_PHY_STUB_NL_H__

#define UU_PHY_STUB_HDR_SZ   40
#define UU_PHY_STUB_MAX_MPDU_SZ (4096+UU_PHY_STUB_HDR_SZ)  
#define UU_PHY_STUB_TDMAC_FC_BYTE  0x88 /* Changed for testing */



typedef struct phy_stub_proto_ops {
    int (*PhyIndCBK) (int ind, 
            unsigned char *data,
            int len);	
} phy_stub_proto_ops_t;

extern phy_stub_proto_ops_t *phy_ops_gp;

extern int uu_phy_stub_nl_reg_ops(phy_stub_proto_ops_t *ops);

extern int uu_phy_stub_nl_dereg_ops(void);

extern int  uu_phy_stub_nl_send_frame(unsigned char *frame, int len);

extern int uu_phy_stub_start_rx_proc(unsigned char *data, int len);

#endif /* __UU_PHY_STUB_NL_H__ */

/* EOF */



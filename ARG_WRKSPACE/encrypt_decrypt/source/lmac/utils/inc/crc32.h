
#ifndef  __FCS_HEADER_H__
#define  __FCS_HEADER_H__

#include "uu_datatypes.h"

//extern uu_uint32 uu_wlan_get_crc(uu_uint32 crc, const uu_void *buf, uu_uint16 size);

extern uu_void   uu_wlan_fill_crc(uu_uint32 crc, uu_uchar *buf, uu_uint16 size);

extern uu_bool   uu_wlan_is_crc_valid(uu_uint32 crc, const uu_uchar *buf, uu_uint16 size);

extern uu_uint8  uu_wlan_get_crc_8bit(uu_uint16 data);

extern uu_bool   uu_wlan_is_8bitCrc_valid(uu_uint16 data, uu_uint8 crc);


//uu_uint32 IEEESTA_crc32_fun(unsigned int crc ,const void *buff,uu_uint16 size);


#endif /* __FCS_HEADER_H__ */

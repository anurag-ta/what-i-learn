#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <unistd.h>
#endif

#include "uu_datatypes.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_lmac_sta_info.h"
#include "uu_wlan_sta_test_main.h"


/* Simulation of umac_functionality filling structure and sending commands to LMAC */
static uu_int32 __send_addba_info(uu_uint8 tid, uu_uint16 bufsz, uu_uint16 ssn, uu_uint8 type, uu_uchar dir);
static uu_void __send_delba_info(uu_uint8 tid, uu_uint8 dir);
static uu_int32 __add_station(uu_int16 aid, uu_wlan_ch_bndwdth_type_t bndwdth);
static uu_int32 __del_station(uu_int16 aid);


uu_int32 uu_wlan_sta_test_handler(uu_char *buf)
{
    uu_int32 ret;
    uu_uint32 id, ev_type;
    uu_uint8 type, tid;
    uu_uint8 bufsz;
    uu_uint16 aid, ssn;
    char file[64];
    uu_uchar *addr, is_dir_tx;
    char string[100];
    char line[100];
    int value;
    static int sta_count = 0;
    static int ba_count = 0;
    FILE* fp = NULL;
    uu_wlan_asso_sta_info_t* sta = NULL;
    uu_wlan_sta_addr_index_t* sta_addr;
    uu_wlan_ba_ses_context_t* ba_table;
    int ch_bndwdth;

    sscanf(buf, "testcaseid: %d ev_type: %d filename: %s", &id, &ev_type, file);
    printf("after reading testcase\n");
    printf(" %d %d %s", id, ev_type, file);

    fp = fopen(file, "r");
    if(fp == NULL)
    {
        printf("\n fopen() Error!!!\n");
        return UU_FAILURE;
    }

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s = %x", string, &value);

        if(strcmp(string, "aid") == 0)
        {
            sscanf(line, "%s = %hu", string, &aid);
        }
        else if(strcmp(string, "bndwidth") == 0)
        {
            sscanf(line, "%s = %d", string, &ch_bndwdth);
        }
        else if(strcmp(string, "addr[0]") == 0)
        {
            uu_dot11_sta_mac_addr_r[0] = value;
        }
        else if(strcmp(string, "addr[1]") == 0)
        {
            uu_dot11_sta_mac_addr_r[1] = value;
        }
        else if(strcmp(string, "addr[2]") == 0)
        {
            uu_dot11_sta_mac_addr_r[2] = value;
        }
        else if(strcmp(string, "addr[3]") == 0)
        {
            uu_dot11_sta_mac_addr_r[3] = value;
        }
        else if(strcmp(string, "addr[4]") == 0)
        {
            uu_dot11_sta_mac_addr_r[4] = value;
        }
        else if(strcmp(string, "addr[5]") == 0)
        {
            uu_dot11_sta_mac_addr_r[5] = value;
        }
        else if(strcmp(string, "tid") == 0)
        {
            sscanf(line, "%s = %hhu", string, &tid);
        }
        else if(strcmp(string, "dir") == 0)
        {
            sscanf(line, "%s = %hhu", string, &is_dir_tx);
        }
        else if(strcmp(string, "ssn") == 0)
        {
            sscanf(line, "%s = %hu", string, &ssn);
        }
        else if(strcmp(string, "type") == 0)
        {
            sscanf(line,"%s = %hhu", string, &type);
        }
        else if(strcmp(string, "bufsz") == 0)
        {
            sscanf(line,"%s = %hhu", string, &bufsz);
        }
    } /* while */

    switch(ev_type)
    {
        case 0: /* ADD-STA */
            {
                ret = __add_station(aid, ch_bndwdth);
                printf("return value after sta add is %d \n", ret);
                sta = uu_wlan_lmac_get_sta(uu_dot11_sta_mac_addr_r);
                //printf("STA aid is %d bwd is %x and recved sta aid is %d bwd is %x\n", aid, ch_bndwdth, sta->aid, sta->ch_bndwdth);
                printf("sta_count is %d %d\n", sta_count, UU_WLAN_MAX_ASSOCIATIONS);
                if (sta == NULL)
                {
                    if(sta_count >= UU_WLAN_MAX_ASSOCIATIONS)
                    {
                        return UU_SUCCESS;
                    }
                    else if ((ret != UU_SUCCESS) && (isMulticastAddr(uu_dot11_sta_mac_addr_r) || isNullAddr(uu_dot11_sta_mac_addr_r)))
                    {
                        return UU_SUCCESS;
                    }
                    else
                    {
                        return UU_FAILURE;
                    }
                }
                else
                {
                    sta_count++;
                    return UU_SUCCESS;
                }
                break;
            }

        case 1: /* ADD-BA */
            {
                //printf("Flag is %x \n", uu_wlan_ba_info_is_used_g); 
                ret = __send_addba_info(tid, bufsz, ssn, type, is_dir_tx);
                sta = uu_wlan_lmac_get_sta(uu_dot11_sta_mac_addr_r);
                ba_table = uu_wlan_get_ba_contxt_of_sta(uu_dot11_sta_mac_addr_r, tid, is_dir_tx);

                if ((sta == NULL) && (ret == UU_FAILURE))
                {
                    return UU_SUCCESS;
                }
                else if ((sta->ba_cnxt[is_dir_tx][tid] >= UU_WLAN_MAX_BA_SESSIONS) && (ret == UU_FAILURE))//ba_table==NULL)
                {
                    printf("Negative case --- pass \n");
                    return UU_SUCCESS;
                }
                else if ((ba_table->ses.type == type) && (ba_table->ses.sb.winstart == 0) && (ba_table->ses.sb.winsize == bufsz))
                {
                    printf("%d %d %d %d %d\n", ba_table->ses.type, ba_table->ses.sb.winstart, ba_table->ses.sb.winsize, type, bufsz);
                    return UU_SUCCESS;
                }
                else
                {
                    return UU_FAILURE;
                }
                break;
            }
        case 2: /* DEL-BA */
            {
                __send_delba_info(tid, is_dir_tx);
                sta = uu_wlan_lmac_get_sta(uu_dot11_sta_mac_addr_r);
                ba_table = uu_wlan_get_ba_contxt_of_sta(uu_dot11_sta_mac_addr_r, tid, is_dir_tx);
                if ((sta != NULL) && (sta->ba_cnxt[is_dir_tx][tid] == UU_WLAN_INVALID_BA_SESSION))
                {
                    printf("ba session is deleted");
                    return UU_SUCCESS;
                }
                else if(sta == NULL)
                {
                    printf("sta is null & ba session is deleted");
                    return UU_SUCCESS;
                }
                else
                {
                    return UU_FAILURE;
                }
                break;
            }
        case 3: /* DEL-STA */
            {
                sta = uu_wlan_lmac_get_sta(uu_dot11_sta_mac_addr_r);
                __del_station(aid);
                if(sta!=NULL)
                {
                    sta = uu_wlan_lmac_get_sta(uu_dot11_sta_mac_addr_r);
                    if(sta == NULL)
                    {
                        sta_count--;
                        return UU_SUCCESS;
                    }
                    else
                    {
                        return UU_FAILURE;
                    }
                }
                else if(sta == NULL)
                {
                    sta = uu_wlan_lmac_get_sta(uu_dot11_sta_mac_addr_r);
                    if(sta == NULL)
                    {
                        return UU_SUCCESS;
                    }
                    else
                    {
                        return UU_FAILURE;
                    }
                }
                break;
            }       
    } /* switch */
} /* uu_wlan_sta_test_handler */


static uu_int32 __add_station(uu_int16 aid, uu_wlan_ch_bndwdth_type_t bndwdth)
{
    uu_int32 ret;
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_ADD;

    /** Filling of station info */
    cmd_info.cmd_data.asso_add.aid = aid;
    cmd_info.cmd_data.asso_add.ch_bndwdth = bndwdth;

    memcpy(cmd_info.cmd_data.asso_add.addr, uu_dot11_sta_mac_addr_r, 6);
    ret = uu_wlan_lmac_config(&cmd_info);
    /** Filling associated station information */

    return ret;
}


static uu_int32 __del_station(uu_int16 aid)
{
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_DEL;

    /** Filling of station info */
    cmd_info.cmd_data.asso_del.aid = aid;

    memcpy(cmd_info.cmd_data.asso_del.addr, uu_dot11_sta_mac_addr_r, 6);

    printf("%x %x %x %x %x %x add before del called \n", cmd_info.cmd_data.asso_del.addr[0], cmd_info.cmd_data.asso_del.addr[1], cmd_info.cmd_data.asso_del.addr[2], cmd_info.cmd_data.asso_del.addr[3], cmd_info.cmd_data.asso_del.addr[4], cmd_info.cmd_data.asso_del.addr[5]);
    uu_wlan_lmac_config(&cmd_info);
    /** Filling associated station information */
    return 0;
}


/* Sends the addba command to LMAC, LMAC adds the BA information to BA table */
static uu_int32 __send_addba_info(uu_uint8 tid, uu_uint16 bufsz, uu_uint16 ssn, uu_uint8 type, uu_uchar dir)
{
    uu_int32 ret;
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_BA_SESS_ADD;

    /** Filling of ADDBA info */
    memcpy(cmd_info.cmd_data.addba.addr, uu_dot11_sta_mac_addr_r, 6);
    cmd_info.cmd_data.addba.tid = tid;
    cmd_info.cmd_data.addba.is_dir_tx = dir;
    cmd_info.cmd_data.addba.ssn = ssn;
    cmd_info.cmd_data.addba.type = type;
    cmd_info.cmd_data.addba.bufsz = bufsz;

    printk("UMAC-ADDBA: STA Addr: %x:%x:%x:%x:%x:%x, TID:%d, Buffer Size:%d, SSN=%d, type=%d, dir:%d\n",
            cmd_info.cmd_data.addba.addr[0], cmd_info.cmd_data.addba.addr[1], cmd_info.cmd_data.addba.addr[2], cmd_info.cmd_data.addba.addr[3],
            cmd_info.cmd_data.addba.addr[4], cmd_info.cmd_data.addba.addr[5], cmd_info.cmd_data.addba.tid, cmd_info.cmd_data.addba.bufsz,
            cmd_info.cmd_data.addba.ssn, cmd_info.cmd_data.addba.type, cmd_info.cmd_data.addba.is_dir_tx);

    ret = uu_wlan_lmac_config(&cmd_info);
    printf("add ba ret value %d", ret);
    return ret;
} /* uu_wlan_umac_send_addba_info */


/* Sends the delba command to LMAC, LMAC removes the BA information from BA table */
static uu_void __send_delba_info(uu_uint8 tid, uu_uint8 dir)
{
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_BA_SESS_DEL;

    /** Filling of DELBA info */
    memcpy(cmd_info.cmd_data.delba.addr, uu_dot11_sta_mac_addr_r, 6);
    cmd_info.cmd_data.delba.tid = tid;
    cmd_info.cmd_data.delba.is_dir_tx = dir;
    printk("UMAC-DELBA:  STA Addr: %x:%x:%x:%x:%x:%x, TID:%d \n",
            cmd_info.cmd_data.delba.addr[0], cmd_info.cmd_data.delba.addr[1], cmd_info.cmd_data.delba.addr[2], cmd_info.cmd_data.delba.addr[3],
            cmd_info.cmd_data.delba.addr[4], cmd_info.cmd_data.delba.addr[5], cmd_info.cmd_data.delba.tid);
    uu_wlan_lmac_config(&cmd_info);
} /* uu_wlan_umac_send_delba_info */



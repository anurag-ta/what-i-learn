/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_ps.h                                           **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                          **
 ** All rights reserved.                                                      **
 ** http://www.uurmi.com                                                      **
 **                                                                           **
 ** All information contained herein is property of Uurmi Systems             **
 ** unless otherwise explicitly mentioned.                                    **
 **                                                                           **
 ** The intellectual and technical concepts in this file are proprietary      **
 ** to Uurmi Systems and may be covered by granted or in process national     **
 ** and international patents and are protect by trade secrets and            **
 ** copyright law.                                                            **
 **                                                                           **
 ** Redistribution and use in source and binary forms of the content in       **
 ** this file, with or without modification are not permitted unless          **
 ** permission is explicitly granted by Uurmi Systems.                        **
 **                                                                           **
 ******************************************************************************/
/* Contains interface declarations for Power-Save module */

#include "uu_datatypes.h"

extern uu_uint32 uu_wlan_lmac_dtim_period_g;
extern uu_bool uu_wlan_lmac_dtim_bcn_received_g;

extern uu_void uu_wlan_ps_handle_rx_frame(uu_uchar *mpdu, uu_int32 dest);

extern uu_void uu_wlan_ps_handle_idle_entry(uu_void);
extern uu_void uu_wlan_ps_idle_exit(uu_void);

/** Checks whetehr STA is in sleep mode */
extern uu_bool uu_wlan_is_sta_in_sleep(uu_void);

/** Sets a sleeping STA in awake state.
 * Called by CAP, when the STA is sleeping and there is a Tx ready indication in idle state.
 * Also called when DTIM timer expires.
 */
extern uu_void uu_wlan_set_sta_awake(uu_void);

/** Checks whether LMAC is using power-save.
 * On setting/resetting this flag, actual PS operation starts/ends.
 * Behavior depends on UMAC enabling power-save mode & DTIM.
 * Set by Rx handler, after receiving first DTIM beacon immediately after switching to PS mode.
 * Reset in two ways after disabling PS, by any of these:
 *   - if there is Tx data to send
 *   - after DTIM timer expiry (for sending null frame with PM bit set to zero)
 */
extern uu_bool uu_wlan_lmac_is_ps_enabled(uu_void);

/** Disables the power save mode in LMAC by resetting uu_wlan_lmac_ps_enabled_g.
  * called by CAP ,during Tx ready indication if PS is deconfigured.
  * called in Rx handler after DTIM expiry if PS is deconfigured.
 **/
/**
 * Called after UMAC disables the power-save mode.
 */
extern uu_void uu_wlan_lmac_disable_ps(uu_void);

/** Checks whether DTIM beacon is received (uu_wlan_lmac_dtim_bcn_received_g flag).
 * Called in entering CAP IDLE state (before going to sleep), to check whether DTIM beacon is received.
 * If not set, STA will not go to sleep.
 */
extern uu_bool uu_wlan_lmac_is_dtim_bcn_received(uu_void);

/** Returns whether there is more data to receive (uu_wlan_lmac_ps_more_data_g flag).
 * Called in entering CAP IDLE state (before going to sleep), to check whether more data has to be received.
 */
extern uu_bool uu_wlan_lmac_is_ps_more_data(uu_void);

/* EOF */


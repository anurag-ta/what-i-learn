/**************************************************************************
 **                                                                      **
 ** File name :  uu_wlan_mgmt_action.h                                   **
 **                                                                      **
 ** Copyright © 2013, Uurmi Systems                                      **
 ** All rights reserved.                                                 **
 ** http://www.uurmi.com                                                 **
 **                                                                      **
 ** All information contained herein is property of Uurmi Systems        **
 ** unless otherwise explicitly mentioned.                               **
 **                                                                      **
 ** The intellectual and technical concepts in this file are proprietary **
 ** to Uurmi Systems and may be covered by granted or in process national**
 ** and international patents and are protect by trade secrets and       **
 ** copyright law.                                                       **
 **                                                                      **
 ** Redistribution and use in source and binary forms of the content in  **
 ** this file, with or without modification are not permitted unless     **
 ** permission is explicitly granted by Uurmi Systems.                   **
 **                                                                      **
 *************************************************************************/

#ifndef __UU_WLAN_MGMT_ACTION_H__
#define __UU_WLAN_MGMT_ACTION_H__

#ifdef UU_WLAN_DFS

#include "uu_wlan_lmac_if.h"
#include "uu_datatypes.h"
#include "uu_wlan_main.h"

/*
 ** This includes all the measurement types implemented.
 **
 ** For Spectrum Measurement
 ** 1. Basic Request/Report
 **
 ** For Radio Measurement
 ** 1. Channel Load Request/Report
 **
 */

/* ----------------------------------- SPECTRUM MANAGEMENT ACTION FRAME ---------------------------------------- */

/**
 * Defining bits for Basic Measurement Report Map field
 * Reference to P802.11 REV_mb D12, Fig 8-143.
 *
 * BSS bit is set to 1 when at least one valid MPDU was received in the channel during
 * the measurement period from another BSS. Otherwise, the BSS bit is set to 0.
 *
 * OFDM Preamble bit is set to 1 when at least one sequence of short
 * training symbols, else 0.
 *
 * Unidentified Signal Bit is set to 1 when, in the channel during the
 * measurement period, there is significant power detected that is not
 * characterized as radar, an OFDM preamble, or a valid MPDU. Else 0.
 *
 * Radar bit, which is set to 1 when radar was detected operating in
 * the channel during the measurement period, else 0.
 *
 * Unmeasured bit, which is set to 1 when this channel has not been measured.
 * Otherwise, the Unmeasured bit is set to 0. When the Unmeasured field is
 * equal to 1, all the other bit fields are set to 0.
 */

/**
 * If another bss is detected, then make the 0th bit as 1. TBD by LMAC.
 * If ofdm_preamble is detected, then make the 1st bit as 1. TBD by PHY.
 * If Unidentified signal is detected, then make the 2nd bit as 1. TBD by PHY.
 * If radar is detected, then make the 3rd bit as 1. TBD by PHY.
 * No need of radar detection in non-DFS channel, TBD by UMAC.
 * If none of them is 1, then unmeasured will be 1. TBD by UMAC/FWK.
 */

/**
 * Structure for basic rep formation.
 * Reference P802.11 REV_mb D12 Fig.8-142.
 */
typedef struct uu_wlan_basic_rep_map
{
    /** Bit 0 */
    uu_uint8            bss : 1;

    /** Bit 1 */
    uu_uint8            ofdm_preamble : 1;

    /** Bit 2 */
    uu_uint8            unidentified_signal : 1;

    /** Bit 3 */
    uu_uint8            radar : 1;

    /** Bit 4 */
    uu_uint8            unmeasured : 1;

    /** Bit 5-7 */
    uu_uint8            reserved : 3;
} uu_wlan_basic_rep_map_t;

/**
 * Measurement Request Mode Types
 * Refer to P802.11 REV_mb D12 Fig 8-105
 */
#define UU_WLAN_MEAS_REQ_PARALLEL   0x01
#define UU_WLAN_MEAS_REQ_ENABLE     0x02
#define UU_WLAN_MEAS_REQ_REQUEST    0x04
#define UU_WLAN_MEAS_REQ_REPORT     0x08
#define UU_WLAN_MEAS_REQ_DUR_MAND   0x10
#define UU_WLAN_MEAS_REQ_RESERVED   0xe0

/**
 * Reference P802.11 REV_mb D12 , basic request(Fig.8-106),
 * cca request(Fig.8-107), rpi_histogram request(Fig.8-108).
 * This structure is applicable for spectrum measurement
 * request: basic, cca and rpi_histogram request.
 */
typedef struct uu_wlan_spec
{
    uu_uint8 channel_number;
    uu_uint64 meas_start_time;
    uu_uint16 meas_duration;
} __attribute__((packed)) uu_wlan_spec_t;

/**
 * Structure for basic report formation.
 * Reference P802.11 REV_mb D12 Fig.8-142.
 */
typedef struct uu_wlan_basic_report
{
    uu_wlan_spec_t basic_msr;
    uu_wlan_basic_rep_map_t map;
} __attribute__((packed)) uu_wlan_basic_report_t;


/* ----------------------------------- RADIO MANAGEMENT ACTION FRAME ---------------------------------------- */


/**
 * Reference P802.11 REV_mb D12 section
 * 8.4.1.11. Action field Table 8-38.
 */
#define UU_WLAN_CATEGORY_RADIO_MGMT    5

/** Reference P802.11 REV_mb D12 Table 8-206. */
typedef enum uu_wlan_radio_mgmt_actioncode
{
    UU_WLAN_ACTION_RADIO_MSR_REQ = 0,
    UU_WLAN_ACTION_RADIO_MSR_RPRT = 1,
    UU_WLAN_ACTION_LINK_MSR_REQ = 2,
    UU_WLAN_ACTION_LINK_MSR_RPRT = 3,
    UU_WLAN_ACTION_NEIG_MSR_REQ = 4,
    UU_WLAN_ACTION_NEIG_MSR_RPRT = 5,
} uu_wlan_radio_mgmt_actioncode_t;

/** Refer to P802.11 REV-mb D12, Table: 8-81 */
#define UU_WLAN_RADIO_MSR_RPRT_TYPE_CHAN_LOAD   3

/** Refer to P802.11 REV-mb D12, section 10.11.9.3 */
#define CHAN_LOAD(channel_busy_time, meas_duration) INT(channel_busy_time * 255, meas_duration * 1024)

/**
 * Common parameters of the radio
 * measurement request frames
 */
typedef struct uu_wlan_radio_req
{
    uu_uint8 operating_class;
    uu_uint8 channel_number;
    uu_uint16 rand_interval;
    uu_uint16 meas_duration;
} __attribute__((packed)) uu_wlan_radio_req_t;

/**
 * Common parameters of the radio
 * measurement report frames
 */
typedef struct uu_wlan_radio_rep
{
    uu_uint8 operating_class;
    uu_uint8 channel_number;
    uu_uint64 act_meas_start_time;
    uu_uint16 meas_duration;
} __attribute__((packed)) uu_wlan_radio_rep_t;

/**
 * NOTE : Since optional elements may be present
 * in the radio measurement requests,
 * so we can't define them the same way.
 */

/**
 * Structure for channel_load request formation.
 * Reference P802.11 REV_mb D12 Fig.8-109.
 */
typedef struct uu_wlan_channel_load_request
{
    uu_wlan_radio_req_t clreq_msr;
    /** optional subelements */
} __attribute__((packed)) uu_wlan_channel_load_request_t;


/**
 * Structure for channel_load report formation.
 * Reference P802.11 REV_mb D12 Fig.8-146.
 */
typedef struct uu_wlan_channel_load_report
{
    uu_wlan_radio_rep_t clrep_msr;
    uu_uint8 channel_load;
    /** optional subelements */
} __attribute__((packed)) uu_wlan_channel_load_report_t;


/* ----------------------------------- MEASUREMENT ACTION ELEMENT ---------------------------------------- */

/**
 * Defining the Measurement Report element format
 * Reference P802.11 REV_mb D12 Fig.8-140, 8-104.
 */
typedef struct uu_wlan_measurement_element
{
    uu_uint8 element_id;
    uu_uint8 length;
    uu_uint8 meas_token;
    union
    {
        uu_uint8 meas_report_mode;
        uu_uint8 meas_request_mode;
    } mode;
    uu_uint8 meas_type;
    union
    {
        /** For request frame elements */
        union
        {
            /**
             * Spectrum Measurement Request: Basic Request,
             * CCA request, RPI Histogram Request
             */
            uu_wlan_spec_t spec_req;

            /** Radio Measurement Requests */
            /** Notes: Applicable only for uu_wlan_radio_measurement_req_element */
            uu_wlan_channel_load_request_t chanload_request;
        } request /** Variable */;
        /** For report frame elements */
        union
        {
            /** Spectrum Measurement Reports */
            uu_wlan_basic_report_t basic_report;
            /** Radio Measurement Reports */
            uu_wlan_channel_load_report_t chanload_report;
        } report;
    } u;
} __attribute__((packed)) uu_wlan_measurement_element_t;

typedef struct uu_wlan_radio_measurement_req_element
{
    uu_int16 repetitions;
    uu_wlan_measurement_element_t rreq_element;
} __attribute__((packed)) uu_wlan_radio_measurement_req_element_t;

/**
 * This defines the structure for TPC
 * Request element/frame
 */
typedef struct uu_wlan_tpc_req_element
{
    uu_uint8 element_id;
    uu_uint8 length;
} __attribute__((packed)) uu_wlan_tpc_req_element_t;

typedef struct uu_wlan_tpc_rep_element
{
    uu_uint8 element_id;
    uu_uint8 length;
    /* Coded as a 2’s complement signed integer,
     * units of dBm, maximum tolerance for the
     * transmit power value is ± 5 dB */
    uu_int8 transmit_power;
    /* Coded as a 2’s complement signed integer,
     * units of dB */
    uu_int8 link_margin;
} __attribute__((packed)) uu_wlan_tpc_rep_element_t;

/**
 * Defining the Spectrum Measurement Request/Report frame format
 * Reference P802.11 REV_mb D12 section 8.5.2.2 and 8.5.2.3.
 * Defining the Spectrum Management TPC Request/Report frame format
 * Reference P802.11 REV_mb D12 section 8.5.2.4 and 8.5.2.5.
 */
typedef struct uu_wlan_measurement_frame
{
    uu_uint8 category;
    /** Valid for both spectrum and radio management action  and tpc frames */
    uu_uint8 mgmt_action;
    uu_uint8 dialog_token;
    union
    {
        /** Applicable only for radio measurement request frame */
        uu_wlan_radio_measurement_req_element_t radio_req_element;

        /**
         * Applicable for spectrum measurement request/report
         * and radio measurement report
         */
        uu_wlan_measurement_element_t meas_element;

        /* TPC Request frame element */
        uu_wlan_tpc_req_element_t tpc_req_element;

        /* TPC Report frame element */
        uu_wlan_tpc_rep_element_t tpc_rep_element;
    } u;
} __attribute__((packed)) uu_wlan_measurement_frame_t;

/* ----------------------------------- MANAGEMENT ACTION FRAME ---------------------------------------- */


/**
 * Management frame that is defined
 only for action frame for supporting
 measurement request/report. */
/* Reference to /include/linux/ieee80211.h
   struct ieee80211_mgmt */
typedef struct uu_mgmt_action_frame
{
    uu_uint16 frame_control;
    uu_uint16 duration;
    uu_uint8 da[ETH_ALEN];
    uu_uint8 sa[ETH_ALEN];
    uu_uint8 bssid[ETH_ALEN];
    uu_uint16 seq_ctrl;
    /** Measurement Request/Report frame */
    uu_wlan_measurement_frame_t uu_meas_frame;
} __attribute__((packed)) uu_mgmt_action_frame_t;


#endif /* UU_WLAN_DFS */

#endif /* __UU_WLAN_MGMT_ACTION_H__ */

/* EOF */


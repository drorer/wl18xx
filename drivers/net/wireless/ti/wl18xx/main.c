/*
 * This file is part of wl18xx
 *
 * Copyright (C) 2011 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/ip.h>

#include "../wlcore/wlcore.h"
#include "../wlcore/debug.h"
#include "../wlcore/io.h"
#include "../wlcore/acx.h"
#include "../wlcore/tx.h"
#include "../wlcore/rx.h"
#include "../wlcore/io.h"
#include "../wlcore/boot.h"

#include "reg.h"
#include "conf.h"
#include "acx.h"
#include "tx.h"
#include "wl18xx.h"
#include "io.h"
#include "version.h"
#include "debugfs.h"

#define WL18XX_RX_CHECKSUM_MASK      0x40

static char *ht_mode_param = "wide";
static char *board_type_param = "hdk";
static bool dc2dc_param = false;
static int n_antennas_2_param = 1;
static int n_antennas_5_param = 1;
static bool checksum_param = false;
static bool enable_11a_param = true;
static int low_band_component = -1;
static int low_band_component_type = -1;
static int high_band_component = -1;
static int high_band_component_type = -1;
static int pwr_limit_reference_11_abg = -1;
static bool disable_yield_fix = true;
static bool enable_pad_last_frame = true;
static int Psat = -1;
static int low_power_val = 0xff;
static int med_power_val = 0xff;
static int high_power_val = 0xff;

static const u8 wl18xx_rate_to_idx_2ghz[] = {
	/* MCS rates are used only with 11n */
	15,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS15 */
	14,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS14 */
	13,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS13 */
	12,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS12 */
	11,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS11 */
	10,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS10 */
	9,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS9 */
	8,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS8 */
	7,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS7 */
	6,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS6 */
	5,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS5 */
	4,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS4 */
	3,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS3 */
	2,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS2 */
	1,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS1 */
	0,                             /* WL18XX_CONF_HW_RXTX_RATE_MCS0 */

	11,                            /* WL18XX_CONF_HW_RXTX_RATE_54   */
	10,                            /* WL18XX_CONF_HW_RXTX_RATE_48   */
	9,                             /* WL18XX_CONF_HW_RXTX_RATE_36   */
	8,                             /* WL18XX_CONF_HW_RXTX_RATE_24   */

	/* TI-specific rate */
	CONF_HW_RXTX_RATE_UNSUPPORTED, /* WL18XX_CONF_HW_RXTX_RATE_22   */

	7,                             /* WL18XX_CONF_HW_RXTX_RATE_18   */
	6,                             /* WL18XX_CONF_HW_RXTX_RATE_12   */
	3,                             /* WL18XX_CONF_HW_RXTX_RATE_11   */
	5,                             /* WL18XX_CONF_HW_RXTX_RATE_9    */
	4,                             /* WL18XX_CONF_HW_RXTX_RATE_6    */
	2,                             /* WL18XX_CONF_HW_RXTX_RATE_5_5  */
	1,                             /* WL18XX_CONF_HW_RXTX_RATE_2    */
	0                              /* WL18XX_CONF_HW_RXTX_RATE_1    */
};

static const u8 wl18xx_rate_to_idx_5ghz[] = {
	/* MCS rates are used only with 11n */
	15,                           /* WL18XX_CONF_HW_RXTX_RATE_MCS15 */
	14,                           /* WL18XX_CONF_HW_RXTX_RATE_MCS14 */
	13,                           /* WL18XX_CONF_HW_RXTX_RATE_MCS13 */
	12,                           /* WL18XX_CONF_HW_RXTX_RATE_MCS12 */
	11,                           /* WL18XX_CONF_HW_RXTX_RATE_MCS11 */
	10,                           /* WL18XX_CONF_HW_RXTX_RATE_MCS10 */
	9,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS9 */
	8,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS8 */
	7,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS7 */
	6,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS6 */
	5,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS5 */
	4,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS4 */
	3,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS3 */
	2,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS2 */
	1,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS1 */
	0,                            /* WL18XX_CONF_HW_RXTX_RATE_MCS0 */

	7,                             /* WL18XX_CONF_HW_RXTX_RATE_54   */
	6,                             /* WL18XX_CONF_HW_RXTX_RATE_48   */
	5,                             /* WL18XX_CONF_HW_RXTX_RATE_36   */
	4,                             /* WL18XX_CONF_HW_RXTX_RATE_24   */

	/* TI-specific rate */
	CONF_HW_RXTX_RATE_UNSUPPORTED, /* WL18XX_CONF_HW_RXTX_RATE_22   */

	3,                             /* WL18XX_CONF_HW_RXTX_RATE_18   */
	2,                             /* WL18XX_CONF_HW_RXTX_RATE_12   */
	CONF_HW_RXTX_RATE_UNSUPPORTED, /* WL18XX_CONF_HW_RXTX_RATE_11   */
	1,                             /* WL18XX_CONF_HW_RXTX_RATE_9    */
	0,                             /* WL18XX_CONF_HW_RXTX_RATE_6    */
	CONF_HW_RXTX_RATE_UNSUPPORTED, /* WL18XX_CONF_HW_RXTX_RATE_5_5  */
	CONF_HW_RXTX_RATE_UNSUPPORTED, /* WL18XX_CONF_HW_RXTX_RATE_2    */
	CONF_HW_RXTX_RATE_UNSUPPORTED, /* WL18XX_CONF_HW_RXTX_RATE_1    */
};

static const u8 *wl18xx_band_rate_to_idx[] = {
	[IEEE80211_BAND_2GHZ] = wl18xx_rate_to_idx_2ghz,
	[IEEE80211_BAND_5GHZ] = wl18xx_rate_to_idx_5ghz
};

enum wl18xx_hw_rates {
	WL18XX_CONF_HW_RXTX_RATE_MCS15 = 0,
	WL18XX_CONF_HW_RXTX_RATE_MCS14,
	WL18XX_CONF_HW_RXTX_RATE_MCS13,
	WL18XX_CONF_HW_RXTX_RATE_MCS12,
	WL18XX_CONF_HW_RXTX_RATE_MCS11,
	WL18XX_CONF_HW_RXTX_RATE_MCS10,
	WL18XX_CONF_HW_RXTX_RATE_MCS9,
	WL18XX_CONF_HW_RXTX_RATE_MCS8,
	WL18XX_CONF_HW_RXTX_RATE_MCS7,
	WL18XX_CONF_HW_RXTX_RATE_MCS6,
	WL18XX_CONF_HW_RXTX_RATE_MCS5,
	WL18XX_CONF_HW_RXTX_RATE_MCS4,
	WL18XX_CONF_HW_RXTX_RATE_MCS3,
	WL18XX_CONF_HW_RXTX_RATE_MCS2,
	WL18XX_CONF_HW_RXTX_RATE_MCS1,
	WL18XX_CONF_HW_RXTX_RATE_MCS0,
	WL18XX_CONF_HW_RXTX_RATE_54,
	WL18XX_CONF_HW_RXTX_RATE_48,
	WL18XX_CONF_HW_RXTX_RATE_36,
	WL18XX_CONF_HW_RXTX_RATE_24,
	WL18XX_CONF_HW_RXTX_RATE_22,
	WL18XX_CONF_HW_RXTX_RATE_18,
	WL18XX_CONF_HW_RXTX_RATE_12,
	WL18XX_CONF_HW_RXTX_RATE_11,
	WL18XX_CONF_HW_RXTX_RATE_9,
	WL18XX_CONF_HW_RXTX_RATE_6,
	WL18XX_CONF_HW_RXTX_RATE_5_5,
	WL18XX_CONF_HW_RXTX_RATE_2,
	WL18XX_CONF_HW_RXTX_RATE_1,
	WL18XX_CONF_HW_RXTX_RATE_MAX,
};

static struct wlcore_conf wl18xx_conf = {
	.sg = {
		.params = {
			[CONF_SG_ACL_BT_MASTER_MIN_BR] = 10,
			[CONF_SG_ACL_BT_MASTER_MAX_BR] = 180,
			[CONF_SG_ACL_BT_SLAVE_MIN_BR] = 10,
			[CONF_SG_ACL_BT_SLAVE_MAX_BR] = 180,
			[CONF_SG_ACL_BT_MASTER_MIN_EDR] = 10,
			[CONF_SG_ACL_BT_MASTER_MAX_EDR] = 80,
			[CONF_SG_ACL_BT_SLAVE_MIN_EDR] = 10,
			[CONF_SG_ACL_BT_SLAVE_MAX_EDR] = 80,
			[CONF_SG_ACL_WLAN_PS_MASTER_BR] = 8,
			[CONF_SG_ACL_WLAN_PS_SLAVE_BR] = 8,
			[CONF_SG_ACL_WLAN_PS_MASTER_EDR] = 20,
			[CONF_SG_ACL_WLAN_PS_SLAVE_EDR] = 20,
			[CONF_SG_ACL_WLAN_ACTIVE_MASTER_MIN_BR] = 20,
			[CONF_SG_ACL_WLAN_ACTIVE_MASTER_MAX_BR] = 35,
			[CONF_SG_ACL_WLAN_ACTIVE_SLAVE_MIN_BR] = 16,
			[CONF_SG_ACL_WLAN_ACTIVE_SLAVE_MAX_BR] = 35,
			[CONF_SG_ACL_WLAN_ACTIVE_MASTER_MIN_EDR] = 32,
			[CONF_SG_ACL_WLAN_ACTIVE_MASTER_MAX_EDR] = 50,
			[CONF_SG_ACL_WLAN_ACTIVE_SLAVE_MIN_EDR] = 28,
			[CONF_SG_ACL_WLAN_ACTIVE_SLAVE_MAX_EDR] = 50,
			[CONF_SG_ACL_ACTIVE_SCAN_WLAN_BR] = 10,
			[CONF_SG_ACL_ACTIVE_SCAN_WLAN_EDR] = 20,
			[CONF_SG_ACL_PASSIVE_SCAN_BT_BR] = 75,
			[CONF_SG_ACL_PASSIVE_SCAN_WLAN_BR] = 15,
			[CONF_SG_ACL_PASSIVE_SCAN_BT_EDR] = 27,
			[CONF_SG_ACL_PASSIVE_SCAN_WLAN_EDR] = 17,
			/* active scan params */
			[CONF_SG_AUTO_SCAN_PROBE_REQ] = 170,
			[CONF_SG_ACTIVE_SCAN_DURATION_FACTOR_HV3] = 50,
			[CONF_SG_ACTIVE_SCAN_DURATION_FACTOR_A2DP] = 100,
			/* passive scan params */
			[CONF_SG_PASSIVE_SCAN_DURATION_FACTOR_A2DP_BR] = 800,
			[CONF_SG_PASSIVE_SCAN_DURATION_FACTOR_A2DP_EDR] = 200,
			[CONF_SG_PASSIVE_SCAN_DURATION_FACTOR_HV3] = 200,
			/* passive scan in dual antenna params */
			[CONF_SG_CONSECUTIVE_HV3_IN_PASSIVE_SCAN] = 0,
			[CONF_SG_BCN_HV3_COLLISION_THRESH_IN_PASSIVE_SCAN] = 0,
			[CONF_SG_TX_RX_PROTECTION_BWIDTH_IN_PASSIVE_SCAN] = 0,
			/* general params */
			[CONF_SG_STA_FORCE_PS_IN_BT_SCO] = 1,
			[CONF_SG_ANTENNA_CONFIGURATION] = 0,
			[CONF_SG_BEACON_MISS_PERCENT] = 60,
			[CONF_SG_DHCP_TIME] = 5000,
			[CONF_SG_RXT] = 1200,
			[CONF_SG_TXT] = 1000,
			[CONF_SG_ADAPTIVE_RXT_TXT] = 1,
			[CONF_SG_GENERAL_USAGE_BIT_MAP] = 3,
			[CONF_SG_HV3_MAX_SERVED] = 6,
			[CONF_SG_PS_POLL_TIMEOUT] = 10,
			[CONF_SG_UPSD_TIMEOUT] = 10,
			[CONF_SG_CONSECUTIVE_CTS_THRESHOLD] = 2,
			[CONF_SG_STA_RX_WINDOW_AFTER_DTIM] = 5,
			[CONF_SG_STA_CONNECTION_PROTECTION_TIME] = 30,
			/* AP params */
			[CONF_AP_BEACON_MISS_TX] = 3,
			[CONF_AP_RX_WINDOW_AFTER_BEACON] = 10,
			[CONF_AP_BEACON_WINDOW_INTERVAL] = 2,
			[CONF_AP_CONNECTION_PROTECTION_TIME] = 0,
			[CONF_AP_BT_ACL_VAL_BT_SERVE_TIME] = 25,
			[CONF_AP_BT_ACL_VAL_WL_SERVE_TIME] = 25,
			/* CTS Diluting params */
			[CONF_SG_CTS_DILUTED_BAD_RX_PACKETS_TH] = 0,
			[CONF_SG_CTS_CHOP_IN_DUAL_ANT_SCO_MASTER] = 0,
		},
		.state = CONF_SG_PROTECTIVE,
	},
	.rx = {
		.rx_msdu_life_time           = 512000,
		.packet_detection_threshold  = 0,
		.ps_poll_timeout             = 15,
		.upsd_timeout                = 15,
		.rts_threshold               = IEEE80211_MAX_RTS_THRESHOLD,
		.rx_cca_threshold            = 0,
		.irq_blk_threshold           = 0xFFFF,
		.irq_pkt_threshold           = 0,
		.irq_timeout                 = 600,
		.queue_type                  = CONF_RX_QUEUE_TYPE_LOW_PRIORITY,
	},
	.tx = {
		.tx_energy_detection         = 0,
		.sta_rc_conf                 = {
			.enabled_rates       = 0,
			.short_retry_limit   = 10,
			.long_retry_limit    = 10,
			.aflags              = 0,
		},
		.ac_conf_count               = 4,
		.ac_conf                     = {
			[CONF_TX_AC_BE] = {
				.ac          = CONF_TX_AC_BE,
				.cw_min      = 15,
				.cw_max      = 63,
				.aifsn       = 3,
				.tx_op_limit = 0,
			},
			[CONF_TX_AC_BK] = {
				.ac          = CONF_TX_AC_BK,
				.cw_min      = 15,
				.cw_max      = 63,
				.aifsn       = 7,
				.tx_op_limit = 0,
			},
			[CONF_TX_AC_VI] = {
				.ac          = CONF_TX_AC_VI,
				.cw_min      = 15,
				.cw_max      = 63,
				.aifsn       = CONF_TX_AIFS_PIFS,
				.tx_op_limit = 3008,
			},
			[CONF_TX_AC_VO] = {
				.ac          = CONF_TX_AC_VO,
				.cw_min      = 15,
				.cw_max      = 63,
				.aifsn       = CONF_TX_AIFS_PIFS,
				.tx_op_limit = 1504,
			},
		},
		.max_tx_retries = 100,
		.ap_aging_period = 300,
		.tid_conf_count = 4,
		.tid_conf = {
			[CONF_TX_AC_BE] = {
				.queue_id    = CONF_TX_AC_BE,
				.channel_type = CONF_CHANNEL_TYPE_EDCF,
				.tsid        = CONF_TX_AC_BE,
				.ps_scheme   = CONF_PS_SCHEME_LEGACY,
				.ack_policy  = CONF_ACK_POLICY_LEGACY,
				.apsd_conf   = {0, 0},
			},
			[CONF_TX_AC_BK] = {
				.queue_id    = CONF_TX_AC_BK,
				.channel_type = CONF_CHANNEL_TYPE_EDCF,
				.tsid        = CONF_TX_AC_BK,
				.ps_scheme   = CONF_PS_SCHEME_LEGACY,
				.ack_policy  = CONF_ACK_POLICY_LEGACY,
				.apsd_conf   = {0, 0},
			},
			[CONF_TX_AC_VI] = {
				.queue_id    = CONF_TX_AC_VI,
				.channel_type = CONF_CHANNEL_TYPE_EDCF,
				.tsid        = CONF_TX_AC_VI,
				.ps_scheme   = CONF_PS_SCHEME_LEGACY,
				.ack_policy  = CONF_ACK_POLICY_LEGACY,
				.apsd_conf   = {0, 0},
			},
			[CONF_TX_AC_VO] = {
				.queue_id    = CONF_TX_AC_VO,
				.channel_type = CONF_CHANNEL_TYPE_EDCF,
				.tsid        = CONF_TX_AC_VO,
				.ps_scheme   = CONF_PS_SCHEME_LEGACY,
				.ack_policy  = CONF_ACK_POLICY_LEGACY,
				.apsd_conf   = {0, 0},
			},
		},
		.frag_threshold              = IEEE80211_MAX_FRAG_THRESHOLD,
		.tx_compl_timeout            = 350,
		.tx_compl_threshold          = 10,
		.basic_rate                  = CONF_HW_BIT_RATE_1MBPS,
		.basic_rate_5                = CONF_HW_BIT_RATE_6MBPS,
		.tmpl_short_retry_limit      = 10,
		.tmpl_long_retry_limit       = 10,
		.tx_stuck_timeout            = 5000,
	},
	.conn = {
		.wake_up_event               = CONF_WAKE_UP_EVENT_DTIM,
		.listen_interval             = 1,
		.suspend_wake_up_event       = CONF_WAKE_UP_EVENT_N_DTIM,
		.suspend_listen_interval     = 3,
		.bcn_filt_mode               = CONF_BCN_FILT_MODE_ENABLED,
		.bcn_filt_ie_count           = 2,
		.bcn_filt_ie = {
			[0] = {
				.ie          = WLAN_EID_CHANNEL_SWITCH,
				.rule        = CONF_BCN_RULE_PASS_ON_APPEARANCE,
			},
			[1] = {
				.ie          = WLAN_EID_HT_INFORMATION,
				.rule        = CONF_BCN_RULE_PASS_ON_CHANGE,
			},
		},
		.synch_fail_thold            = 10,
		.bss_lose_timeout            = 100,
		.beacon_rx_timeout           = 10000,
		.broadcast_timeout           = 20000,
		.rx_broadcast_in_ps          = 1,
		.ps_poll_threshold           = 10,
		.bet_enable                  = CONF_BET_MODE_ENABLE,
		.bet_max_consecutive         = 50,
		.psm_entry_retries           = 8,
		.psm_exit_retries            = 16,
		.psm_entry_nullfunc_retries  = 3,
		.dynamic_ps_timeout          = 40,
		.forced_ps                   = false,
		.keep_alive_interval         = 55000,
		.max_listen_interval         = 20,
	},
	.itrim = {
		.enable = false,
		.timeout = 50000,
	},
	.pm_config = {
		.host_clk_settling_time = 5000,
		.host_fast_wakeup_support = false
	},
	.roam_trigger = {
		.trigger_pacing               = 1,
		.avg_weight_rssi_beacon       = 20,
		.avg_weight_rssi_data         = 10,
		.avg_weight_snr_beacon        = 20,
		.avg_weight_snr_data          = 10,
	},
	.scan = {
		.min_dwell_time_active        = 7500,
		.max_dwell_time_active        = 30000,
		.min_dwell_time_passive       = 100000,
		.max_dwell_time_passive       = 100000,
		.num_probe_reqs               = 2,
		.split_scan_timeout           = 50000,
	},
	.sched_scan = {
		/*
		 * Values are in TU/1000 but since sched scan FW command
		 * params are in TUs rounding up may occur.
		 */
		.base_dwell_time              = 7500,
		.max_dwell_time_delta         = 22500,
		/* based on 250bits per probe @1Mbps */
		.dwell_time_delta_per_probe   = 2000,
		/* based on 250bits per probe @6Mbps (plus a bit more) */
		.dwell_time_delta_per_probe_5 = 350,
		.dwell_time_passive           = 100000,
		.dwell_time_dfs               = 150000,
		.num_probe_reqs               = 2,
		.rssi_threshold               = -90,
		.snr_threshold                = 0,
	},
	.ht = {
		.rx_ba_win_size = 10,
		.tx_ba_win_size = 64,
		.inactivity_timeout = 10000,
		.tx_ba_tid_bitmap = CONF_TX_BA_ENABLED_TID_BITMAP,
	},
	.mem = {
		.num_stations                 = 1,
		.ssid_profiles                = 1,
		.rx_block_num                 = 40,
		.tx_min_block_num             = 40,
		.dynamic_memory               = 1,
		.min_req_tx_blocks            = 45,
		.min_req_rx_blocks            = 22,
		.tx_min                       = 27,
	},
	.fm_coex = {
		.enable                       = true,
		.swallow_period               = 5,
		.n_divider_fref_set_1         = 0xff,       /* default */
		.n_divider_fref_set_2         = 12,
		.m_divider_fref_set_1         = 148,
		.m_divider_fref_set_2         = 0xffff,     /* default */
		.coex_pll_stabilization_time  = 0xffffffff, /* default */
		.ldo_stabilization_time       = 0xffff,     /* default */
		.fm_disturbed_band_margin     = 0xff,       /* default */
		.swallow_clk_diff             = 0xff,       /* default */
	},
	.rx_streaming = {
		.duration                      = 150,
		.queues                        = 0x1,
		.interval                      = 20,
		.always                        = 0,
	},
	.fwlog = {
		.mode                         = WL12XX_FWLOG_ON_DEMAND,
		.mem_blocks                   = 2,
		.severity                     = 0,
		.timestamp                    = WL12XX_FWLOG_TIMESTAMP_DISABLED,
		.output                       = WL12XX_FWLOG_OUTPUT_HOST,
		.threshold                    = 0,
	},
	.rate = {
		.rate_retry_score = 32000,
		.per_add = 8192,
		.per_th1 = 2048,
		.per_th2 = 4096,
		.max_per = 8100,
		.inverse_curiosity_factor = 5,
		.tx_fail_low_th = 4,
		.tx_fail_high_th = 10,
		.per_alpha_shift = 4,
		.per_add_shift = 13,
		.per_beta1_shift = 10,
		.per_beta2_shift = 8,
		.rate_check_up = 2,
		.rate_check_down = 12,
		.rate_retry_policy = {
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00,
		},
	},
	.hangover = {
		.recover_time               = 0,
		.hangover_period            = 20,
		.dynamic_mode               = 1,
		.early_termination_mode     = 1,
		.max_period                 = 20,
		.min_period                 = 1,
		.increase_delta             = 1,
		.decrease_delta             = 2,
		.quiet_time                 = 4,
		.increase_time              = 1,
		.window_size                = 16,
	},
};

static struct wl18xx_priv_conf wl18xx_default_priv_conf = {
	.phy = {
		.phy_standalone			= 0x00,
		.primary_clock_setting_time	= 0x05,
		.clock_valid_on_wake_up		= 0x00,
		.secondary_clock_setting_time	= 0x05,
		.rdl				= 0x01,
		.auto_detect			= 0x00,
		.dedicated_fem			= FEM_NONE,
		.low_band_component		= COMPONENT_2_WAY_SWITCH,
		.low_band_component_type	= 0x05,
		.high_band_component		= COMPONENT_2_WAY_SWITCH,
		.high_band_component_type	= 0x09,
		.tcxo_ldo_voltage		= 0x00,
		.xtal_itrim_val			= 0x04,
		.srf_state			= 0x00,
		.io_configuration		= 0x01,
		.sdio_configuration		= 0x00,
		.settings			= 0x00,
		.enable_clpc			= 0x00,
		.enable_tx_low_pwr_on_siso_rdl	= 0x00,
		.rx_profile			= 0x00,
		.pwr_limit_reference_11_abg	= 0xc8,
		.Psat				= 0,
		.low_power_val			= 0x00,
		.med_power_val			= 0x0a,
		.high_power_val			= 0x1e,
	},
};

static const struct wlcore_partition_set wl18xx_ptable[PART_TABLE_LEN] = {
	[PART_TOP_PRCM_ELP_SOC] = {
		.mem  = { .start = 0x00A02000, .size  = 0x00010000 },
		.reg  = { .start = 0x00807000, .size  = 0x00005000 },
		.mem2 = { .start = 0x00800000, .size  = 0x0000B000 },
		.mem3 = { .start = 0x00000000, .size  = 0x00000000 },
	},
	[PART_DOWN] = {
		.mem  = { .start = 0x00000000, .size  = 0x00014000 },
		.reg  = { .start = 0x00810000, .size  = 0x0000BFFF },
		.mem2 = { .start = 0x00000000, .size  = 0x00000000 },
		.mem3 = { .start = 0x00000000, .size  = 0x00000000 },
	},
	[PART_BOOT] = {
		.mem  = { .start = 0x00700000, .size = 0x0000030c },
		.reg  = { .start = 0x00802000, .size = 0x00014578 },
		.mem2 = { .start = 0x00B00404, .size = 0x00001000 },
		.mem3 = { .start = 0x00C00000, .size = 0x00000400 },
	},
	[PART_WORK] = {
		.mem  = { .start = 0x00800000, .size  = 0x000050FC },
		.reg  = { .start = 0x00B00404, .size  = 0x00001000 },
		.mem2 = { .start = 0x00C00000, .size  = 0x00000400 },
		.mem3 = { .start = 0x00000000, .size  = 0x00000000 },
	},
	[PART_PHY_INIT] = {
		.mem  = { .start = 0x80926000, .size =
				sizeof(struct wl18xx_mac_and_phy_params) },
		.reg  = { .start = 0x00000000, .size = 0x00000000 },
		.mem2 = { .start = 0x00000000, .size = 0x00000000 },
		.mem3 = { .start = 0x00000000, .size = 0x00000000 },
	},
	[PART_PHY_PDSP_WA] = {
		.mem  = { .start = 0x00940100, .size  = 0x00000200 },
		.reg  = { .start = 0x00953000, .size  = 0x00018000 },
		.mem2 = { .start = 0x00000000, .size  = 0x00000000 },
		.mem3 = { .start = 0x00000000, .size  = 0x00000000 },
	},
};

static const struct wl18xx_phy_addresses wl18xx_phy_addresses_pg1 = {
	.phy_hram_rd_en = WL18XX_PHY_HRAM_RD_EN_PER_RAM,
	.pdsp_ctrl_reg = WL18XX_PDSP_CONTROL_REG_PG1,
	.pdsp_ram = WL18XX_FDSP_RAM,
	.prcm_bt_pwr_rst = WL18XX_PRCM_BT_PWR_RST,
	.ip_sel_ov_en = WL18XX_IP_SEL_OV_EN,
	.ip_ov_en = WL18XX_IP_OV_EN,
};

static const struct wl18xx_phy_addresses wl18xx_phy_addresses_pg2 = {
	.phy_hram_rd_en = WL18XX_PHY_HRAM_RD_EN_PER_RAM,
	.pdsp_ctrl_reg = WL18XX_PDSP_CONTROL_REG_PG2,
	.pdsp_ram = WL18XX_FDSP_RAM,
	.prcm_bt_pwr_rst = WL18XX_PRCM_BT_PWR_RST,
	.ip_sel_ov_en = WL18XX_IP_SEL_OV_EN,
	.ip_ov_en = WL18XX_IP_OV_EN,
};

static const int wl18xx_rtable[REG_TABLE_LEN] = {
	[REG_ECPU_CONTROL]		= WL18XX_REG_ECPU_CONTROL,
	[REG_INTERRUPT_NO_CLEAR]	= WL18XX_REG_INTERRUPT_NO_CLEAR,
	[REG_INTERRUPT_ACK]		= WL18XX_REG_INTERRUPT_ACK,
	[REG_COMMAND_MAILBOX_PTR]	= WL18XX_REG_COMMAND_MAILBOX_PTR,
	[REG_EVENT_MAILBOX_PTR]		= WL18XX_REG_EVENT_MAILBOX_PTR,
	[REG_INTERRUPT_TRIG]		= WL18XX_REG_INTERRUPT_TRIG_H,
	[REG_INTERRUPT_MASK]		= WL18XX_REG_INTERRUPT_MASK,
	[REG_PC_ON_RECOVERY]		= WL18XX_SCR_PAD4,
	[REG_CHIP_ID_B]			= WL18XX_REG_CHIP_ID_B,
	[REG_CMD_MBOX_ADDRESS]		= WL18XX_CMD_MBOX_ADDRESS,

	/* data access memory addresses, used with partition translation */
	[REG_SLV_MEM_DATA]		= WL18XX_SLV_MEM_DATA,
	[REG_SLV_REG_DATA]		= WL18XX_SLV_REG_DATA,

	/* raw data access memory addresses */
	[REG_RAW_FW_STATUS_ADDR]	= WL18XX_FW_STATUS_ADDR,
};

static const struct wl18xx_clk_cfg wl18xx_clk_table[NUM_CLOCK_CONFIGS] = {
	[CLOCK_CONFIG_16_2_M]	= { 7,  104,  801, 4,  true },
	[CLOCK_CONFIG_16_368_M]	= { 9,  132, 3751, 4,  true },
	[CLOCK_CONFIG_16_8_M]	= { 7,  100,    0, 0, false },
	[CLOCK_CONFIG_19_2_M]	= { 8,  100,    0, 0, false },
	[CLOCK_CONFIG_26_M]	= { 13, 120,    0, 0, false },
	[CLOCK_CONFIG_32_736_M]	= { 9,  132, 3751, 4,  true },
	[CLOCK_CONFIG_33_6_M]	= { 7,  100,    0, 0, false },
	[CLOCK_CONFIG_38_468_M]	= { 8,  100,    0, 0, false },
	[CLOCK_CONFIG_52_M]	= { 13, 120,    0, 0, false },
};

/* TODO: maybe move to a new header file? */
#define WL18XX_FW_PG1_NAME "ti-connectivity/wl18xx-fw-pg1.1.bin"
#define WL18XX_FW_PG2_NAME "ti-connectivity/wl18xx-fw.bin"

static int wl18xx_identify_chip(struct wl1271 *wl)
{
	int ret = 0;
	struct wl18xx_priv *priv = wl->priv;

	switch (wl->chip.id) {
	case CHIP_ID_185x_PG20:
		wl1271_debug(DEBUG_BOOT, "chip id 0x%x (185x PG20)",
				 wl->chip.id);
		wl->sr_fw_name = WL18XX_FW_PG2_NAME;
		/* wl18xx uses the same firmware for PLT */
		wl->plt_fw_name = WL18XX_FW_PG2_NAME;
		wl->quirks |= WLCORE_QUIRK_NO_ELP |
				  WLCORE_QUIRK_FWLOG_NOT_IMPLEMENTED |
				  WLCORE_QUIRK_RX_BLOCKSIZE_ALIGN;
		if (enable_pad_last_frame) {
			wl->quirks |= WLCORE_QUIRK_TX_PAD_LAST_FRAME;
			wl->quirks &= ~WLCORE_QUIRK_TX_BLOCKSIZE_ALIGN;
		}

		memcpy(&priv->phy_addresses, &wl18xx_phy_addresses_pg2,
			sizeof(struct wl18xx_phy_addresses));

		/* TODO: need to blocksize alignment for RX/TX separately? */
		break;
	case CHIP_ID_185x_PG10:
		wl1271_debug(DEBUG_BOOT, "chip id 0x%x (185x PG10)",
			     wl->chip.id);
		wl->sr_fw_name = WL18XX_FW_PG1_NAME;
		/* wl18xx uses the same firmware for PLT */
		wl->plt_fw_name = WL18XX_FW_PG1_NAME;
		wl->quirks |= WLCORE_QUIRK_NO_ELP |
			      WLCORE_QUIRK_FWLOG_NOT_IMPLEMENTED |
			      WLCORE_QUIRK_RX_BLOCKSIZE_ALIGN;

		memcpy(&priv->phy_addresses, &wl18xx_phy_addresses_pg1,
			sizeof(struct wl18xx_phy_addresses));

		/* PG 1.0 has some problems with MCS_13, so disable it */
		wl->ht_cap[IEEE80211_BAND_2GHZ].mcs.rx_mask[1] &= ~BIT(5);

		/* TODO: need to blocksize alignment for RX/TX separately? */
		break;
	default:
		wl1271_warning("unsupported chip id: 0x%x", wl->chip.id);
		ret = -ENODEV;
		goto out;
	}

out:
	return ret;
}

static void wl18xx_set_clk(struct wl1271 *wl)
{
	u32 clk_freq;

	wlcore_set_partition(wl, &wl->ptable[PART_TOP_PRCM_ELP_SOC]);

	/* TODO: PG2: apparently we need to read the clk type */

	clk_freq = wl18xx_top_reg_read(wl, PRIMARY_CLK_DETECT);
	wl1271_debug(DEBUG_BOOT, "clock freq %d (%d, %d, %d, %d, %s)", clk_freq,
		     wl18xx_clk_table[clk_freq].n, wl18xx_clk_table[clk_freq].m,
		     wl18xx_clk_table[clk_freq].p, wl18xx_clk_table[clk_freq].q,
		     wl18xx_clk_table[clk_freq].swallow ? "swallow" : "spit");

	wl18xx_top_reg_write(wl, PLLSH_WCS_PLL_N, wl18xx_clk_table[clk_freq].n);
	wl18xx_top_reg_write(wl, PLLSH_WCS_PLL_M, wl18xx_clk_table[clk_freq].m);

	if (wl18xx_clk_table[clk_freq].swallow) {
		/* first the 16 lower bits */
		wl18xx_top_reg_write(wl, PLLSH_WCS_PLL_Q_FACTOR_CFG_1,
				     wl18xx_clk_table[clk_freq].q &
				     PLLSH_WCS_PLL_Q_FACTOR_CFG_1_MASK);
		/* then the 16 higher bits, masked out */
		wl18xx_top_reg_write(wl, PLLSH_WCS_PLL_Q_FACTOR_CFG_2,
				     (wl18xx_clk_table[clk_freq].q >> 16) &
				     PLLSH_WCS_PLL_Q_FACTOR_CFG_2_MASK);

		/* first the 16 lower bits */
		wl18xx_top_reg_write(wl, PLLSH_WCS_PLL_P_FACTOR_CFG_1,
				     wl18xx_clk_table[clk_freq].p &
				     PLLSH_WCS_PLL_P_FACTOR_CFG_1_MASK);
		/* then the 16 higher bits, masked out */
		wl18xx_top_reg_write(wl, PLLSH_WCS_PLL_P_FACTOR_CFG_2,
				     (wl18xx_clk_table[clk_freq].p >> 16) &
				     PLLSH_WCS_PLL_P_FACTOR_CFG_2_MASK);
	} else {
		wl18xx_top_reg_write(wl, PLLSH_WCS_PLL_SWALLOW_EN,
				     PLLSH_WCS_PLL_SWALLOW_EN_VAL2);
	}
}

static void wl18xx_boot_soft_reset(struct wl1271 *wl)
{
	/* disable Rx/Tx */
	wl1271_write32(wl, WL18XX_ENABLE, 0x0);

	/* disable auto calibration on start*/
	wl1271_write32(wl, WL18XX_SPARE_A2, 0xffff);
}

static int wl18xx_pdsp_reset(struct wl1271 *wl)
{
	int status;
	struct wl18xx_priv *priv = wl->priv;
	struct wl18xx_phy_addresses *phy_addr = &priv->phy_addresses;

	wl1271_debug(DEBUG_BOOT, "reset PDSP");

	wl1271_write32(wl, phy_addr->pdsp_ctrl_reg, WL18XX_PDSP_ENABLE);
	wl1271_write32(wl, phy_addr->pdsp_ctrl_reg, WL18XX_PDSP_DISABLE);
	wl1271_write32(wl, phy_addr->pdsp_ctrl_reg, WL18XX_PDSP_ENABLE);
	wl1271_write32(wl, phy_addr->pdsp_ctrl_reg, WL18XX_PDSP_DISABLE);

	/* validate PDSP reset */
	wl1271_write32(wl, phy_addr->pdsp_ram, WL18XX_FDSP_RAM_VAL);
	status = wl1271_read32(wl, phy_addr->pdsp_ram);
	if (status == WL18XX_FDSP_RAM_VAL) {
		wl1271_debug(DEBUG_BOOT, "OCP bridge ready");
		return 0;
	}

	return -EAGAIN;
}

static int wl18xx_phy_core_reset(struct wl1271 *wl)
{
	int ocp_state;
	struct wl18xx_priv *priv = wl->priv;
	struct wl18xx_phy_addresses *phy_addr = &priv->phy_addresses;

	wl1271_debug(DEBUG_BOOT, "reset wl phy core");

	wlcore_set_partition(wl, &wl->ptable[PART_PHY_PDSP_WA]);
	ocp_state = wl1271_read32(wl, phy_addr->phy_hram_rd_en);
	if (!ocp_state)
		return -EIO;

	if (ocp_state == WL18XX_H_RAM_ENABLED)
		return wl18xx_pdsp_reset(wl);

	wlcore_set_partition(wl, &wl->ptable[PART_TOP_PRCM_ELP_SOC]);

	wl18xx_top_reg_write(wl, phy_addr->ip_sel_ov_en, WL18XX_WL_PHY_PWR_REQ);
	wl18xx_top_reg_write(wl, phy_addr->ip_ov_en, WL18XX_WL_PHY_PWR_REQ);

	wl18xx_top_reg_write(wl, phy_addr->ip_sel_ov_en, WL18XX_RM_OVERRIDE);
	wl18xx_top_reg_write(wl, phy_addr->ip_ov_en, WL18XX_RM_OVERRIDE);

	return -EAGAIN;
}

static int wl18xx_bt_core_reset(struct wl1271 *wl)
{
	int ocp_state, status;
	struct wl18xx_priv *priv = wl->priv;
	struct wl18xx_phy_addresses *phy_addr = &priv->phy_addresses;

	wl1271_debug(DEBUG_BOOT, "reset bt core");

	wlcore_set_partition(wl, &wl->ptable[PART_PHY_PDSP_WA]);
	ocp_state = wl1271_read32(wl, phy_addr->phy_hram_rd_en);
	if (!ocp_state)
		return -EIO;

	if (ocp_state == WL18XX_H_RAM_ENABLED)
		return wl18xx_pdsp_reset(wl);

	wlcore_set_partition(wl, &wl->ptable[PART_TOP_PRCM_ELP_SOC]);
	/* check if BT is disabled */
	status = wl18xx_top_reg_read(wl, phy_addr->prcm_bt_pwr_rst);
	if (status != 0)
		return -EACCES;

	wl18xx_top_reg_write(wl, phy_addr->ip_sel_ov_en, WL18XX_WL_PHY_PWR_REQ);
	wl18xx_top_reg_write(wl, phy_addr->ip_ov_en, WL18XX_BT_PWR_REQ);

	wl18xx_top_reg_write(wl, phy_addr->ip_sel_ov_en,
			WL18XX_BT_WL_PHY_PWR_REQ);
	wl18xx_top_reg_write(wl, phy_addr->ip_ov_en, WL18XX_BT_WL_PHY_PWR_REQ);

	wl18xx_top_reg_write(wl, phy_addr->ip_sel_ov_en, WL18XX_RM_OVERRIDE);
	wl18xx_top_reg_write(wl, phy_addr->ip_ov_en, WL18XX_RM_OVERRIDE);
	return -EAGAIN;
}

static int wl18xx_release_ocp_bridge(struct wl1271 *wl)
{
	int ocp_state, i, status;
	struct wl18xx_priv *priv = wl->priv;
	struct wl18xx_phy_addresses *phy_addr = &priv->phy_addresses;

	wl1271_debug(DEBUG_BOOT, "release OCP bridge");

	for (i = 0; i < WL18XX_ACCESS_OCP_MAX_RETRIES; i++) {
		status = wl18xx_phy_core_reset(wl);
		if (!status)
			goto out;
		else if (status == -EIO)
			return status;
	}

	for (i = 0; i < WL18XX_ACCESS_OCP_MAX_RETRIES; i++) {
		status = wl18xx_bt_core_reset(wl);
		if (!status)
			goto out;
		else if (status == -EIO)
			return status;
		else if (status == -EACCES)
			break;
	}

	wlcore_set_partition(wl, &wl->ptable[PART_PHY_PDSP_WA]);
	ocp_state = wl1271_read32(wl, phy_addr->phy_hram_rd_en);
	if (ocp_state == WL18XX_H_RAM_ENABLED)
		goto out;

	wl1271_error("OCP bridge is not accessible - please reset device");
	return -EIO;

out:
	wlcore_set_partition(wl, &wl->ptable[PART_TOP_PRCM_ELP_SOC]);
	return 0;
}

static int wl18xx_pre_boot(struct wl1271 *wl)
{
	wl18xx_set_clk(wl);

	if (wl->chip.id == CHIP_ID_185x_PG20) {
		int ret;
		if (disable_yield_fix)
			goto elp_wakeup;
		ret = wl18xx_release_ocp_bridge(wl);
		if (ret < 0)
			return ret;
	}

	if (wl->chip.id == CHIP_ID_185x_PG10) {
		int ret;
		ret = wl18xx_release_ocp_bridge(wl);
		if (ret < 0)
			return ret;
	}

elp_wakeup:
	/* Continue the ELP wake up sequence */
	wl1271_write32(wl, WL18XX_WELP_ARM_COMMAND, WELP_ARM_COMMAND_VAL);
	udelay(500);

	wlcore_set_partition(wl, &wl->ptable[PART_BOOT]);

	/* Disable interrupts */
	wlcore_write_reg(wl, REG_INTERRUPT_MASK, WL1271_ACX_INTR_ALL);

	wl18xx_boot_soft_reset(wl);

	return 0;
}

static void wl18xx_pre_upload(struct wl1271 *wl)
{
	u32 tmp;

	wlcore_set_partition(wl, &wl->ptable[PART_BOOT]);

	/* TODO: check if this is all needed */
	wl1271_write32(wl, WL18XX_EEPROMLESS_IND, WL18XX_EEPROMLESS_IND);

	tmp = wlcore_read_reg(wl, REG_CHIP_ID_B);

	wl1271_debug(DEBUG_BOOT, "chip id 0x%x", tmp);

	tmp = wl1271_read32(wl, WL18XX_SCR_PAD2);
}

static void wl18xx_set_mac_and_phy(struct wl1271 *wl)
{
	struct wl18xx_priv *priv = wl->priv;
	struct wl18xx_conf_phy *phy = &priv->conf.phy;
	struct wl18xx_mac_and_phy_params params;

	memset(&params, 0, sizeof(params));

	params.phy_standalone = phy->phy_standalone;
	params.rdl = phy->rdl;
	params.enable_clpc = phy->enable_clpc;
	params.enable_tx_low_pwr_on_siso_rdl =
		phy->enable_tx_low_pwr_on_siso_rdl;
	params.auto_detect = phy->auto_detect;
	params.dedicated_fem = phy->dedicated_fem;
	params.low_band_component = phy->low_band_component;
	params.low_band_component_type =
		phy->low_band_component_type;
	params.high_band_component = phy->high_band_component;
	params.high_band_component_type =
		phy->high_band_component_type;
	params.number_of_assembled_ant2_4 =
		n_antennas_2_param;
	params.number_of_assembled_ant5 =
		n_antennas_5_param;
	params.external_pa_dc2dc = dc2dc_param;
	params.tcxo_ldo_voltage = phy->tcxo_ldo_voltage;;
	params.xtal_itrim_val = phy->xtal_itrim_val;
	params.srf_state = phy->srf_state;
	params.io_configuration = phy->io_configuration;
	params.sdio_configuration = phy->sdio_configuration;
	params.settings = phy->settings;
	params.rx_profile = phy->rx_profile;
	params.primary_clock_setting_time =
		phy->primary_clock_setting_time;
	params.clock_valid_on_wake_up =
		phy->clock_valid_on_wake_up;
	params.secondary_clock_setting_time =
		phy->secondary_clock_setting_time;
	params.pwr_limit_reference_11_abg =
		phy->pwr_limit_reference_11_abg;

	params.Psat = phy->Psat;
	params.low_power_val = phy->low_power_val;
	params.med_power_val = phy->med_power_val;
	params.high_power_val = phy->high_power_val;

	params.board_type = priv->board_type;

	wlcore_set_partition(wl, &wl->ptable[PART_PHY_INIT]);
	wl1271_write(wl, WL18XX_PHY_INIT_MEM_ADDR, (u8*)&params,
		     sizeof(params), false);
}

static void wl18xx_enable_interrupts(struct wl1271 *wl)
{
	wlcore_write_reg(wl, REG_INTERRUPT_MASK, WL1271_ACX_ALL_EVENTS_VECTOR);

	wlcore_enable_interrupts(wl);
	wlcore_write_reg(wl, REG_INTERRUPT_MASK,
			 WL1271_ACX_INTR_ALL & ~(WL1271_INTR_MASK));
}

static int wl18xx_boot(struct wl1271 *wl)
{
	int ret;

	ret = wl18xx_pre_boot(wl);
	if (ret < 0)
		goto out;

	wl18xx_pre_upload(wl);

	ret = wlcore_boot_upload_firmware(wl);
	if (ret < 0)
		goto out;

	wl18xx_set_mac_and_phy(wl);

	ret = wlcore_boot_run_firmware(wl);
	if (ret < 0)
		goto out;

	wl18xx_enable_interrupts(wl);

out:
	return ret;
}

static void wl18xx_trigger_cmd(struct wl1271 *wl, int cmd_box_addr,
			       void *buf, size_t len)
{
	struct wl18xx_priv *priv = wl->priv;

	memcpy(priv->cmd_buf, buf, len);
	memset(priv->cmd_buf + len, 0, WL18XX_CMD_MAX_SIZE - len);

	wl1271_write(wl, cmd_box_addr, priv->cmd_buf, WL18XX_CMD_MAX_SIZE,
		     false);
}

static void wl18xx_ack_event(struct wl1271 *wl)
{
	wlcore_write_reg(wl, REG_INTERRUPT_TRIG, WL18XX_INTR_TRIG_EVENT_ACK);
}

static u32 wl18xx_calc_tx_blocks(struct wl1271* wl, u32 len, u32 spare_blks)
{
	u32 blk_size = WL18XX_TX_HW_BLOCK_SIZE;
	return (len + blk_size - 1) / blk_size + spare_blks;
}

static void
wl18xx_set_tx_desc_blocks(struct wl1271 *wl, struct wl1271_tx_hw_descr *desc,
			  u32 blks, u32 spare_blks)
{
	desc->wl18xx_mem.total_mem_blocks = blks;
	desc->wl18xx_mem.reserved = 0;
}

static void
wl18xx_set_tx_desc_data_len(struct wl1271 *wl, struct wl1271_tx_hw_descr *desc,
			    struct sk_buff *skb)
{
	desc->length = cpu_to_le16(skb->len);

	wl1271_debug(DEBUG_TX, "tx_fill_hdr: hlid: %d "
		     "len: %d life: %d mem: %d", desc->hlid,
		     le16_to_cpu(desc->length),
		     le16_to_cpu(desc->life_time),
		     desc->wl18xx_mem.total_mem_blocks);
}

static enum wl_rx_buf_align
wl18xx_get_rx_buf_align(struct wl1271 *wl, u32 rx_desc)
{
	if (rx_desc & RX_BUF_PADDED_PAYLOAD)
		return WLCORE_RX_BUF_PADDED;

	return WLCORE_RX_BUF_ALIGNED;
}

static u32 wl18xx_get_rx_packet_len(struct wl1271 *wl, void *rx_data,
				    u32 data_len)
{
	struct wl1271_rx_descriptor *desc = rx_data;

	/* invalid packet */
	if (data_len < sizeof(*desc))
		return 0;

	return data_len - sizeof(*desc);
}

static void wl18xx_tx_immediate_completion(struct wl1271 *wl)
{
	wl18xx_tx_immediate_complete(wl);
}

static int wl18xx_set_host_cfg_bitmap(struct wl1271 *wl, u32 extra_mem_blk)
{
	int ret;
	u32 sdio_align_size = 0;
	u32 host_cfg_bitmap = HOST_IF_CFG_RX_FIFO_ENABLE |
			      HOST_IF_CFG_ADD_RX_ALIGNMENT;

	/* Enable Tx SDIO padding */
	if (wl->quirks & (WLCORE_QUIRK_TX_BLOCKSIZE_ALIGN |
					WLCORE_QUIRK_TX_PAD_LAST_FRAME)) {
		host_cfg_bitmap |= HOST_IF_CFG_TX_PAD_TO_SDIO_BLK;
		sdio_align_size = WL12XX_BUS_BLOCK_SIZE;
	}

	/* Enable Rx SDIO padding */
	if (wl->quirks & WLCORE_QUIRK_RX_BLOCKSIZE_ALIGN) {
		host_cfg_bitmap |= HOST_IF_CFG_RX_PAD_TO_SDIO_BLK;
		sdio_align_size = WL12XX_BUS_BLOCK_SIZE;
	}

	ret = wl18xx_acx_host_if_cfg_bitmap(wl, host_cfg_bitmap,
					    sdio_align_size, extra_mem_blk,
					    WL18XX_HOST_IF_LEN_SIZE_FIELD);
	if (ret < 0)
		return ret;

	return 0;
}

static int wl18xx_hw_init(struct wl1271 *wl)
{
	int ret;
	struct wl18xx_priv *priv = wl->priv;

	/* (re)init private structures. Relevant on recovery as well. */
	priv->last_fw_rls_idx = 0;
	priv->extra_spare_vif_count = 0;

	/* set the default amount of spare blocks in the bitmap */
	ret = wl18xx_set_host_cfg_bitmap(wl, WL18XX_TX_HW_BLOCK_SPARE);
	if (ret < 0)
		return ret;

	if (checksum_param) {
		ret = wl18xx_acx_set_checksum_state(wl);
		if (ret != 0)
			return ret;
	}

	return ret;
}

static void wl18xx_set_tx_desc_csum(struct wl1271 *wl,
				    struct wl1271_tx_hw_descr *desc,
				    struct sk_buff *skb)
{
	u32 ip_hdr_offset;
	struct iphdr *ip_hdr;

	if (!checksum_param) {
		desc->wl18xx_checksum_data = 0;
		return;
	}

	if (skb->ip_summed != CHECKSUM_PARTIAL) {
		desc->wl18xx_checksum_data = 0;
		return;
	}

	ip_hdr_offset = skb_network_header(skb) - skb_mac_header(skb);
	if (WARN_ON(ip_hdr_offset >= (1<<7))) {
		desc->wl18xx_checksum_data = 0;
		return;
	}

	desc->wl18xx_checksum_data = ip_hdr_offset << 1;

	/* FW is interested only in the LSB of the protocol  TCP=0 UDP=1 */
	ip_hdr = (void *)skb_network_header(skb);
	desc->wl18xx_checksum_data |= (ip_hdr->protocol & 0x01);
}

static int wl18xx_init_vif(struct wl1271* wl, struct wl12xx_vif *wlvif)
{
	struct ieee80211_vif *vif = wl12xx_wlvif_to_vif(wlvif);

	if (vif)
		ieee80211_set_netdev_features(vif, NETIF_F_IP_CSUM);

	return 0;
}

static void wl18xx_set_rx_csum(struct wl1271 *wl,
			       struct wl1271_rx_descriptor *desc,
			       struct sk_buff *skb)
{
	if (desc->status & WL18XX_RX_CHECKSUM_MASK)
		skb->ip_summed = CHECKSUM_UNNECESSARY;
}

/*
 * TODO: instead of having these two functions to get the rate mask,
 * we should modify the wlvif->rate_set instead
 */
static u32 wl18xx_sta_get_ap_rate_mask(struct wl1271 *wl,
				       struct wl12xx_vif *wlvif)
{
	u32 hw_rate_set = wlvif->rate_set;

	if (wlvif->channel_type == NL80211_CHAN_HT40MINUS ||
	    wlvif->channel_type == NL80211_CHAN_HT40PLUS) {
		wl1271_debug(DEBUG_ACX, "using wide channel rate mask");
		hw_rate_set |= CONF_TX_RATE_USE_WIDE_CHAN;

		/* we don't support MIMO in wide-channel mode */
		hw_rate_set &= ~CONF_TX_MIMO_RATES;
	}

	return hw_rate_set;
}

static u32 wl18xx_ap_get_mimo_wide_rate_mask(struct wl1271 *wl,
					     struct wl12xx_vif *wlvif)
{
	if ((wlvif->channel_type == NL80211_CHAN_HT40MINUS ||
	     wlvif->channel_type == NL80211_CHAN_HT40PLUS) &&
	    !strcmp(ht_mode_param, "wide")) {
		wl1271_debug(DEBUG_ACX, "using wide channel rate mask");
		return CONF_TX_RATE_USE_WIDE_CHAN;
	} else if (!strcmp(ht_mode_param, "mimo")) {
		wl1271_debug(DEBUG_ACX, "using MIMO rate mask");

		/*
		 * PG 1.0 has some problems with MCS_13, so disable it
		 *
		 * TODO: instead of hacking this in here, we should
		 * make it more general and change a bit in the
		 * wlvif->rate_set instead.
		 */
		if (wl->chip.id == CHIP_ID_185x_PG10)
			return CONF_TX_MIMO_RATES & ~CONF_HW_BIT_RATE_MCS_13;

		return CONF_TX_MIMO_RATES;
	} else {
		return 0;
	}
}

static void wl18xx_conf_init(struct wl1271 *wl)
{
	struct wl18xx_priv *priv = wl->priv;

	/* apply driver default configuration */
	memcpy(&wl->conf, &wl18xx_conf, sizeof(wl18xx_conf));

	/* apply default private configuration */
	memcpy(&priv->conf, &wl18xx_default_priv_conf, sizeof(priv->conf));
}

static int wl18xx_plt_init(struct wl1271 *wl)
{
	wl1271_write32(wl, WL18XX_SCR_PAD8, WL18XX_SCR_PAD8_PLT);

	return wl->ops->boot(wl);
}

static s8 wl18xx_get_pg_ver(struct wl1271 *wl)
{
	u32 fuse;

	wlcore_set_partition(wl, &wl->ptable[PART_TOP_PRCM_ELP_SOC]);

        fuse = wl1271_read32(wl, WL18XX_REG_FUSE_DATA_1_3);
        fuse = (fuse & WL18XX_PG_VER_MASK) >> WL18XX_PG_VER_OFFSET;

	wlcore_set_partition(wl, &wl->ptable[PART_BOOT]);

	return (s8)fuse;
}

static void wl18xx_get_mac(struct wl1271 *wl)
{
	u32 mac1, mac2;

	wlcore_set_partition(wl, &wl->ptable[PART_TOP_PRCM_ELP_SOC]);

	mac1 = wl1271_read32(wl, WL18XX_REG_FUSE_BD_ADDR_1);
	mac2 = wl1271_read32(wl, WL18XX_REG_FUSE_BD_ADDR_2);

	/* these are the two parts of the BD_ADDR */
	wl->fuse_oui_addr = ((mac2 & 0xffff) << 8) +
		((mac1 & 0xff000000) >> 24);
	wl->fuse_nic_addr = (mac1 & 0xffffff);

	wlcore_set_partition(wl, &wl->ptable[PART_DOWN]);
}

static int wl18xx_handle_static_data(struct wl1271 *wl,
				     struct wl1271_static_data *static_data)
{
	struct wl18xx_static_data_priv *static_data_priv =
		(struct wl18xx_static_data_priv *) static_data->priv;

	wl1271_info("PHY firmware version: %s", static_data_priv->phy_version);

	return 0;
}

static int wl18xx_get_spare_blocks(struct wl1271 *wl, bool is_gem)
{
	struct wl18xx_priv *priv = wl->priv;

	/* If we have VIFs requiring extra spare, indulge them */
	if (priv->extra_spare_vif_count)
		return WL18XX_TX_HW_EXTRA_BLOCK_SPARE;

	return WL18XX_TX_HW_BLOCK_SPARE;
}

static int wl18xx_set_key(struct wl1271 *wl, enum set_key_cmd cmd,
			  struct ieee80211_vif *vif,
			  struct ieee80211_sta *sta,
			  struct ieee80211_key_conf *key_conf)
{
	struct wl18xx_priv *priv = wl->priv;
	bool change_spare = false;
	int ret;

	/*
	 * when adding the first or removing the last GEM/TKIP interface,
	 * we have to adjust the number of spare blocks.
	 */
	change_spare = (key_conf->cipher == WL1271_CIPHER_SUITE_GEM ||
		key_conf->cipher == WLAN_CIPHER_SUITE_TKIP) &&
		((priv->extra_spare_vif_count == 0 && cmd == SET_KEY) ||
		 (priv->extra_spare_vif_count == 1 && cmd == DISABLE_KEY));

	/* no need to change spare - just regular set_key */
	if (!change_spare)
		return wlcore_set_key(wl, cmd, vif, sta, key_conf);

	/*
	 * stop the queues and flush to ensure the next packets are
	 * in sync with FW spare block accounting
	 */
	wlcore_stop_queues(wl, WLCORE_QUEUE_STOP_REASON_SPARE_BLK);
	wl1271_tx_flush(wl);

	ret = wlcore_set_key(wl, cmd, vif, sta, key_conf);
	if (ret < 0)
		goto out;

	/* key is now set, change the spare blocks */
	if (cmd == SET_KEY) {
		ret = wl18xx_set_host_cfg_bitmap(wl,
					WL18XX_TX_HW_EXTRA_BLOCK_SPARE);
		if (ret < 0)
			goto out;

		priv->extra_spare_vif_count++;
	} else {
		ret = wl18xx_set_host_cfg_bitmap(wl,
					WL18XX_TX_HW_BLOCK_SPARE);
		if (ret < 0)
			goto out;

		priv->extra_spare_vif_count--;
	}

out:
	wlcore_wake_queues(wl, WLCORE_QUEUE_STOP_REASON_SPARE_BLK);
	return ret;
}

static struct wlcore_ops wl18xx_ops = {
	.identify_chip	= wl18xx_identify_chip,
	.boot		= wl18xx_boot,
	.plt_init	= wl18xx_plt_init,
	.trigger_cmd	= wl18xx_trigger_cmd,
	.ack_event	= wl18xx_ack_event,
	.calc_tx_blocks = wl18xx_calc_tx_blocks,
	.set_tx_desc_blocks = wl18xx_set_tx_desc_blocks,
	.set_tx_desc_data_len = wl18xx_set_tx_desc_data_len,
	.get_rx_buf_align = wl18xx_get_rx_buf_align,
	.get_rx_packet_len = wl18xx_get_rx_packet_len,
	.tx_immediate_completion = wl18xx_tx_immediate_completion,
	.tx_delayed_completion = NULL,
	.hw_init	= wl18xx_hw_init,
	.set_tx_desc_csum = wl18xx_set_tx_desc_csum,
	.init_vif = wl18xx_init_vif,
	.set_rx_csum = wl18xx_set_rx_csum,
	.sta_get_ap_rate_mask = wl18xx_sta_get_ap_rate_mask,
	.ap_get_mimo_wide_rate_mask = wl18xx_ap_get_mimo_wide_rate_mask,
	.get_pg_ver	= wl18xx_get_pg_ver,
	.get_mac	= wl18xx_get_mac,
	.debugfs_init	= wl18xx_debugfs_add_files,
	.handle_static_data	= wl18xx_handle_static_data,
	.get_spare_blocks = wl18xx_get_spare_blocks,
	.set_key	= wl18xx_set_key,
};

/* HT cap appropriate for wide channels */
struct ieee80211_sta_ht_cap wl18xx_siso40_ht_cap = {
	.cap = IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40 |
	       IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_DSSSCCK40,
	.ht_supported = true,
	.ampdu_factor = IEEE80211_HT_MAX_AMPDU_16K,
	.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
	.mcs = {
		.rx_mask = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		.rx_highest = cpu_to_le16(150),
		.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
		},
};

/* HT cap appropriate for SISO 20 */
struct ieee80211_sta_ht_cap wl18xx_siso20_ht_cap = {
	.cap = IEEE80211_HT_CAP_SGI_20,
	.ht_supported = true,
	.ampdu_factor = IEEE80211_HT_MAX_AMPDU_16K,
	.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
	.mcs = {
		.rx_mask = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		.rx_highest = cpu_to_le16(72),
		.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
		},
};

/* HT cap appropriate for MIMO rates in 20mhz channel */
static struct ieee80211_sta_ht_cap wl18xx_mimo_ht_cap_2ghz = {
	.cap = IEEE80211_HT_CAP_SGI_20,
	.ht_supported = true,
	.ampdu_factor = IEEE80211_HT_MAX_AMPDU_16K,
	.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
	.mcs = {
		.rx_mask = { 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, },
		.rx_highest = cpu_to_le16(144),
		.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
		},
};

static struct ieee80211_sta_ht_cap wl18xx_mimo_ht_cap_5ghz = {
	.cap = IEEE80211_HT_CAP_SGI_20,
	.ht_supported = true,
	.ampdu_factor = IEEE80211_HT_MAX_AMPDU_16K,
	.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
	.mcs = {
		.rx_mask = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		.rx_highest = cpu_to_le16(72),
		.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
		},
};

int __devinit wl18xx_probe(struct platform_device *pdev)
{
	struct wl1271 *wl;
	struct ieee80211_hw *hw;
	struct wl18xx_priv *priv;

	hw = wlcore_alloc_hw(sizeof(*priv));
	if (IS_ERR(hw)) {
		wl1271_error("can't allocate hw");
		return PTR_ERR(hw);
	}

	wl = hw->priv;
	priv = wl->priv;
	wl->ops = &wl18xx_ops;
	wl->ptable = wl18xx_ptable;
	wl->rtable = wl18xx_rtable;
	wl->num_tx_desc = 32;
	wl->num_rx_desc = 16;
	wl->band_rate_to_idx = wl18xx_band_rate_to_idx;
	wl->hw_tx_rate_tbl_size = WL18XX_CONF_HW_RXTX_RATE_MAX;
	wl->hw_min_ht_rate = WL18XX_CONF_HW_RXTX_RATE_MCS0;
	wl->fw_status_priv_len = sizeof(struct wl18xx_fw_status_priv);
	wl->stats.fw_stats_len = sizeof(struct wl18xx_acx_statistics);
	wl->static_data_priv_len = sizeof(struct wl18xx_static_data_priv);

	if (!strcmp(ht_mode_param, "wide")) {
		memcpy(&wl->ht_cap[IEEE80211_BAND_2GHZ],
		       &wl18xx_siso40_ht_cap,
		       sizeof(wl18xx_siso40_ht_cap));
		memcpy(&wl->ht_cap[IEEE80211_BAND_5GHZ],
		       &wl18xx_siso40_ht_cap,
		       sizeof(wl18xx_siso40_ht_cap));
	} else if (!strcmp(ht_mode_param, "mimo")) {
		memcpy(&wl->ht_cap[IEEE80211_BAND_2GHZ],
		       &wl18xx_mimo_ht_cap_2ghz,
		       sizeof(wl18xx_mimo_ht_cap_2ghz));
		memcpy(&wl->ht_cap[IEEE80211_BAND_5GHZ],
		       &wl18xx_mimo_ht_cap_5ghz,
		       sizeof(wl18xx_mimo_ht_cap_5ghz));
	} else if (!strcmp(ht_mode_param, "siso20")) {
		memcpy(&wl->ht_cap[IEEE80211_BAND_2GHZ],
		       &wl18xx_siso20_ht_cap,
		       sizeof(wl18xx_siso20_ht_cap));
		memcpy(&wl->ht_cap[IEEE80211_BAND_5GHZ],
		       &wl18xx_siso20_ht_cap,
		       sizeof(wl18xx_siso20_ht_cap));
	} else {
		wl1271_error("invalid ht_mode '%s'", ht_mode_param);
		goto out_free;
	}

	wl18xx_conf_init(wl);

	if (!strcmp(board_type_param, "fpga")) {
		priv->board_type = BOARD_TYPE_FPGA_18XX;
	} else if (!strcmp(board_type_param, "hdk")) {
		priv->board_type = BOARD_TYPE_HDK_18XX;
		/* HACK! Just for now we hardcode HDK to 0x06 */
		priv->conf.phy.low_band_component_type = 0x06;
	} else if (!strcmp(board_type_param, "dvp")) {
		priv->board_type = BOARD_TYPE_DVP_18XX;
	} else if (!strcmp(board_type_param, "evb")) {
		priv->board_type = BOARD_TYPE_EVB_18XX;
	} else if (!strcmp(board_type_param, "com8")) {
		priv->board_type = BOARD_TYPE_COM8_18XX;
		/* HACK! Just for now we hardcode COM8 to 0x06 */
		priv->conf.phy.low_band_component_type = 0x06;
	} else {
		wl1271_error("invalid board type '%s'", board_type_param);
		goto out_free;
	}

	/*
	 * If the module param is not set, update it with the one from
	 * conf.  If it is set, overwrite conf with it.
	 */
	if (low_band_component == -1)
		low_band_component = priv->conf.phy.low_band_component;
	else
		priv->conf.phy.low_band_component = low_band_component;
	if (low_band_component_type == -1)
		low_band_component_type =
			priv->conf.phy.low_band_component_type;
	else
		priv->conf.phy.low_band_component_type = 
			low_band_component_type;

	if (high_band_component == -1)
		high_band_component = priv->conf.phy.high_band_component;
	else
		priv->conf.phy.high_band_component = high_band_component;
	if (high_band_component_type == -1)
		high_band_component_type =
			priv->conf.phy.high_band_component_type;
	else
		priv->conf.phy.high_band_component_type = 
			high_band_component_type;

	if (pwr_limit_reference_11_abg == -1)
		pwr_limit_reference_11_abg =
			priv->conf.phy.pwr_limit_reference_11_abg;
	else
		priv->conf.phy.pwr_limit_reference_11_abg =
			pwr_limit_reference_11_abg;

	if (Psat == -1)
		Psat = priv->conf.phy.Psat;
	else
		priv->conf.phy.Psat = Psat;

	if (low_power_val == 0xff)
		low_power_val = priv->conf.phy.low_power_val;
	else
		priv->conf.phy.low_power_val = low_power_val;

	if (med_power_val == 0xff)
		med_power_val = priv->conf.phy.med_power_val;
	else
		priv->conf.phy.med_power_val = med_power_val;

	if (high_power_val == 0xff)
		high_power_val = priv->conf.phy.high_power_val;
	else
		priv->conf.phy.high_power_val = high_power_val;

	if (!checksum_param) {
		wl18xx_ops.set_rx_csum = NULL;
		wl18xx_ops.init_vif = NULL;
	}

	wl->enable_11a = enable_11a_param;

	return wlcore_probe(wl, pdev);

out_free:
	wlcore_free_hw(wl);
	return -EINVAL;
}

static const struct platform_device_id wl18xx_id_table[] __devinitconst = {
	{ "wl18xx", 0 },
	{  } /* Terminating Entry */
};
MODULE_DEVICE_TABLE(platform, wl18xx_id_table);

static struct platform_driver wl18xx_driver = {
	.probe		= wl18xx_probe,
	.remove		= __devexit_p(wlcore_remove),
	.id_table	= wl18xx_id_table,
	.driver = {
		.name	= "wl18xx_driver",
		.owner	= THIS_MODULE,
	}
};

static int __init wl18xx_init(void)
{
	wl1271_info("wl18xx driver version: %s", wl18xx_git_head);

	/* TODO: should check the module params validity here */

	return platform_driver_register(&wl18xx_driver);
}
module_init(wl18xx_init);

static void __exit wl18xx_exit(void)
{
	platform_driver_unregister(&wl18xx_driver);
}
module_exit(wl18xx_exit);

module_param_named(ht_mode, ht_mode_param, charp, S_IRUSR);
MODULE_PARM_DESC(ht_mode, "Force HT mode: wide (default), mimo or siso20");

module_param_named(board_type, board_type_param, charp, S_IRUSR);
MODULE_PARM_DESC(board_type, "Board type: fpga, hdk (default), evb, com8 or "
		 "dvp");

module_param_named(dc2dc, dc2dc_param, bool, S_IRUSR);
MODULE_PARM_DESC(dc2dc, "External DC2DC: boolean (defaults to false)");

module_param_named(n_antennas_2, n_antennas_2_param, uint, S_IRUSR);
MODULE_PARM_DESC(n_antennas_2, "Number of installed 2.4GHz antennas: 1 (default) or 2");

module_param_named(n_antennas_5, n_antennas_5_param, uint, S_IRUSR);
MODULE_PARM_DESC(n_antennas_5, "Number of installed 5GHz antennas: 1 (default) or 2");

module_param_named(checksum, checksum_param, bool, S_IRUSR);
MODULE_PARM_DESC(checksum, "Enable TCP checksum: boolean (defaults to false)");

module_param_named(enable_11a, enable_11a_param, bool, S_IRUSR);
MODULE_PARM_DESC(enable_11a, "Enable 11a (5GHz): boolean (defaults to true)");

module_param(low_band_component, uint, S_IRUSR);
MODULE_PARM_DESC(low_band_component, "Low band component: u8 "
		 "(default is 0x01)");

module_param(low_band_component_type, uint, S_IRUSR);
MODULE_PARM_DESC(low_band_component_type, "Low band component type: u8 "
		 "(default is 0x05 or 0x06 depending on the board_type)");

module_param(high_band_component, uint, S_IRUSR);
MODULE_PARM_DESC(high_band_component, "High band component: u8, "
		 "(default is 0x01)");

module_param(high_band_component_type, uint, S_IRUSR);
MODULE_PARM_DESC(high_band_component_type, "High band component type: u8 "
		 "(default is 0x09)");

module_param(pwr_limit_reference_11_abg, uint, S_IRUSR);
MODULE_PARM_DESC(pwr_limit_reference_11_abg, "Power limit reference: u8 "
		 "(default is 0xc8)");

module_param(disable_yield_fix, bool, S_IRUSR);
MODULE_PARM_DESC(disable_yield_fix, "disable yield issue workaround: bool "
		 "(default is false)");

module_param(enable_pad_last_frame, bool, S_IRUSR);
MODULE_PARM_DESC(enable_pad_last_frame, "enable last sdio packet padding: "
		"bool (default is true)");

module_param(Psat, uint, S_IRUSR);
MODULE_PARM_DESC(Psat, "Psat: u8 "
		 "(default is 0)");

module_param(low_power_val, uint, S_IRUSR);
MODULE_PARM_DESC(low_power_val, "low_power_val: u8 "
		 "(default is 0x00)");

module_param(med_power_val, uint, S_IRUSR);
MODULE_PARM_DESC(med_power_val, "med_power_val: u8 "
		 "(default is 0x0a)");

module_param(high_power_val, uint, S_IRUSR);
MODULE_PARM_DESC(high_power_val, "high_power_val: u8 "
		 "(default is 0x1e)");

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Luciano Coelho <coelho@ti.com>");
MODULE_FIRMWARE(WL18XX_FW_PG1_NAME);

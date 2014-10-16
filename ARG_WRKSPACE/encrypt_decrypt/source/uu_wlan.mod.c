#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xeb13df47, "module_layout" },
	{ 0x8848dc62, "uu_dot11_self_cts_r" },
	{ 0x87936e77, "Operating_chbndwdth" },
	{ 0xef170d48, "ieee80211_rx_irqsafe" },
	{ 0x146785c3, "kmalloc_caches" },
	{ 0x59e7b10e, "dot11_long_retry_count" },
	{ 0x15692c87, "param_ops_int" },
	{ 0x286b3843, "ieee80211_queue_work" },
	{ 0xb75c56d1, "eth_change_mtu" },
	{ 0x1637ff0f, "_raw_spin_lock_bh" },
	{ 0xe3121d5e, "dot11_ack_failure_count_g" },
	{ 0xa0fa0ec7, "ieee80211_beacon_get_tim" },
	{ 0xa908c89b, "dot11_signal_extension" },
	{ 0x448eac3e, "kmemdup" },
	{ 0xa50bbf79, "ieee80211_unregister_hw" },
	{ 0x6c32d663, "uu_wlan_tsf_r" },
	{ 0xd7aba0e9, "uu_wlan_last_beacon_tx_r" },
	{ 0x808bf54e, "uu_wlan_rx_status_info_g" },
	{ 0x726b4377, "Center_frequency" },
	{ 0xff0f8aef, "__netdev_alloc_skb" },
	{ 0x3a70d08b, "ieee80211_stop_queue" },
	{ 0xc35f756, "dot11_fcs_error_count_g" },
	{ 0x860c56ce, "BSSBasicRateSet" },
	{ 0x2b13993d, "dot11_slot_timer_value" },
	{ 0xe4f987cb, "dot11_cts_timer_value" },
	{ 0x77cdb933, "dot11_cwmin_value" },
	{ 0xeb0f9407, "dot11_difs_value" },
	{ 0x3a5e28f7, "uu_wlan_tx_write_buffer" },
	{ 0x2c26cf7a, "uu_dot11_sta_mac_addr_r" },
	{ 0x42f5f984, "dot11_sifs_timer_value" },
	{ 0x27e1a049, "printk" },
	{ 0x71b420be, "dot11_BeaconInterval" },
	{ 0x449ad0a7, "memcmp" },
	{ 0x89cc2e3e, "uu_dot11_sta_bssid_r" },
	{ 0xcedd742e, "free_netdev" },
	{ 0xd8050e7a, "register_netdev" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x3e9170be, "uu_wlan_lmac_mode_switch" },
	{ 0xfdfa5ebf, "uu_wlan_lmac_stop" },
	{ 0xda793363, "uu_wlan_tx_status_info_g" },
	{ 0xc67b9002, "uu_wlan_lmac_shutdown" },
	{ 0x5ec209c9, "device_create" },
	{ 0x1ef8f32f, "uu_wlan_tx_status_flags_g" },
	{ 0x137bcef9, "dev_kfree_skb_any" },
	{ 0xc4c5201, "Channel_hwvalue" },
	{ 0xb911a2df, "ieee80211_stop_tx_ba_cb_irqsafe" },
	{ 0xc061eeea, "uu_wlan_lmac_start" },
	{ 0x31d78a61, "uu_lmac_reg_ops" },
	{ 0xba63339c, "_raw_spin_unlock_bh" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x5f065cc3, "ieee80211_tx_status_irqsafe" },
	{ 0x10db8755, "dot11_aPhyRxStartDelay" },
	{ 0xb302afa7, "uu_wlan_lmac_filter_flag_g" },
	{ 0xf00ec856, "alloc_netdev_mqs" },
	{ 0x63eec029, "uu_dot11_op_mode_r" },
	{ 0x2279e479, "uu_wlan_tx_get_available_buffer" },
	{ 0xe2c07a9e, "dot11_rts_threshold_value" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x951d29da, "dot11_cwmax_value_ac" },
	{ 0xe85f43dc, "ether_setup" },
	{ 0xecf0652c, "uu_wlan_lmac_config" },
	{ 0xc104bde3, "kmem_cache_alloc_trace" },
	{ 0x84c52d06, "dot11_short_retry_count" },
	{ 0x127d9c90, "ieee80211_wake_queue" },
	{ 0x474ffab7, "uu_wlan_rx_buff_base_addr_reg_g" },
	{ 0x8347ea7b, "ieee80211_register_hw" },
	{ 0xf72bbadb, "eth_validate_addr" },
	{ 0x860881a3, "dot11_txop_limit_value" },
	{ 0x8cf26c7d, "dot11_rts_success_count_g" },
	{ 0x37a0cba, "kfree" },
	{ 0x86363918, "uu_wlan_rx_status_flags_g" },
	{ 0x69acdf38, "memcpy" },
	{ 0x8c085833, "ieee80211_alloc_hw" },
	{ 0xed7049c2, "dot11_aifs_value" },
	{ 0x53e42909, "uu_wlan_phy_tx_delay_r" },
	{ 0xc40fae79, "class_destroy" },
	{ 0x76c12387, "dot11_cwmax_value" },
	{ 0x11c8d6de, "ieee80211_free_hw" },
	{ 0xccf3b9e4, "dot11_rts_failure_count_g" },
	{ 0xb0ee02a0, "device_unregister" },
	{ 0xdedb39c0, "unregister_netdev" },
	{ 0x47472548, "uu_wlan_lmac_init" },
	{ 0x3229a9a3, "dot11_cwmin_value_ac" },
	{ 0x371fa388, "uu_wlan_ps_mode_r" },
	{ 0x648a1c99, "consume_skb" },
	{ 0xe9d110a7, "dot11_ack_timer_value" },
	{ 0x82f770cf, "skb_put" },
	{ 0xc3a44a61, "eth_mac_addr" },
	{ 0xd7b54629, "__class_create" },
	{ 0x83a24c29, "ieee80211_start_tx_ba_cb_irqsafe" },
	{ 0x7351db5d, "uu_dot11_qos_mode_r" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=lmac_wlan,mac80211";


MODULE_INFO(srcversion, "D452D05B125E68458B885AE");

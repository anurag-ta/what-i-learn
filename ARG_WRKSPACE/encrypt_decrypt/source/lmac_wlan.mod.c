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
	{ 0xd2b09ce5, "__kmalloc" },
	{ 0xf9a482f9, "msleep" },
	{ 0xadaabe1b, "pv_lock_ops" },
	{ 0x67f1ee6a, "hrtimer_forward" },
	{ 0xedbaf048, "hrtimer_cancel" },
	{ 0xc87c1f84, "ktime_get" },
	{ 0x593a99b, "init_timer_key" },
	{ 0xc5c89993, "kthread_create_on_node" },
	{ 0x7d11c268, "jiffies" },
	{ 0xd5f2172f, "del_timer_sync" },
	{ 0x27e1a049, "printk" },
	{ 0x449ad0a7, "memcmp" },
	{ 0xbe2c0274, "add_timer" },
	{ 0xa735db59, "prandom_u32" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0x2d178924, "hrtimer_start" },
	{ 0xc8f71554, "wake_up_process" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xd52bf1ce, "_raw_spin_lock" },
	{ 0x37a0cba, "kfree" },
	{ 0x69acdf38, "memcpy" },
	{ 0x8125c3b4, "hrtimer_init" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C945EC4E45EE6595A849FEC");

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
	{ 0xf9a482f9, "msleep" },
	{ 0x27e1a049, "printk" },
	{ 0xd532e5cc, "netlink_kernel_release" },
	{ 0x1873cf67, "netlink_unicast" },
	{ 0xaf7b2fe8, "init_net" },
	{ 0x743d42c6, "__alloc_skb" },
	{ 0xe82fe689, "netlink_broadcast" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xc6902977, "__netlink_kernel_create" },
	{ 0x69acdf38, "memcpy" },
	{ 0xa0c10209, "__nlmsg_put" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "9F78CB25C0B539FE58DC13B");

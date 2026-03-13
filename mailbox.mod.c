#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x6106f7fb, "proc_create" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x4829a47e, "memcpy" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xe2964344, "__wake_up" },
	{ 0x92997ed8, "_printk" },
	{ 0x1000e51, "schedule" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0xdcb764ad, "memset" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x8ec23266, "proc_set_user" },
	{ 0x25532bfc, "proc_set_size" },
	{ 0x418c10ec, "__register_chrdev" },
	{ 0xa0e24b52, "remove_proc_entry" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x98cf60b3, "strlen" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7CAFB9D0306980CAB0FA8A4");

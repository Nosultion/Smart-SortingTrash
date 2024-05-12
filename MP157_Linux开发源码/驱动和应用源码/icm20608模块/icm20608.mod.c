#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x3e549f1d, "module_layout" },
	{ 0x43fbdf86, "driver_unregister" },
	{ 0xf2010e8b, "__spi_register_driver" },
	{ 0x66640d77, "spi_setup" },
	{ 0xaed8af67, "device_create" },
	{ 0xb47e05b2, "__class_create" },
	{ 0x24ade49f, "cdev_add" },
	{ 0xf5787540, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x74f8c57c, "devm_kmalloc" },
	{ 0xc5850110, "printk" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x189c5980, "arm_copy_to_user" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x9d669763, "memcpy" },
	{ 0x37a0cba, "kfree" },
	{ 0xee307cd, "spi_sync" },
	{ 0x5f754e5a, "memset" },
	{ 0x2d6fcc06, "__kmalloc" },
	{ 0x7a4fec84, "kmem_cache_alloc_trace" },
	{ 0xf23ce8b, "kmalloc_caches" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x4202ea9c, "class_destroy" },
	{ 0x7ac52d5a, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x644b2a6e, "cdev_del" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

MODULE_INFO(depends, "");


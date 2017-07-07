#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/of.h>

#include <asm/uaccess.h>
#include <asm/io.h>

static int drv_probe(struct platform_device *dev)
{
	printk("drv_probe.\n");
	
	return 0;
}

static int drv_remove(struct platform_device *dev)
{
	printk("drv_remove.\n");
	
	return 0;
}

static const struct platform_device_id plat_id_table[] = {
	{ .name		= "my-platform", },
	{ }
};

static struct platform_driver pdrv = {
	.driver = {
		.name = "platform",
	},
	.probe = drv_probe,
	.remove = drv_remove,
	.id_table = plat_id_table,
};

static int __init my_dri_init(void)
{
	int ret = 0;
	
	
	
	ret = platform_driver_register(&pdrv);
	return 0;
}

static void __exit my_dri_exit(void)
{
	platform_driver_unregister(&pdrv);
}

module_init(my_dri_init);
module_exit(my_dri_exit);

MODULE_LICENSE("GPL");

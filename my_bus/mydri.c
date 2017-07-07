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

struct resource* res;

static int drv_probe(struct platform_device *dev)
{
	res = platform_get_resource(dev, IORESOURCE_IRQ, 0);
	printk("irq number is %d.\n", res->start);
	printk("drv_probe.\n");
	
	return 0;
}

static int drv_remove(struct platform_device *dev)
{
	printk("drv_remove.\n");
	
	return 0;
}

static const struct of_device_id key_of_ids[] = {
	{ .compatible = "fs4412,keys" },
	{},
};

static struct platform_driver pdrv = {
	.driver = {
		.name = "platform",
		.of_match_table = key_of_ids,
	},
	.probe = drv_probe,
	.remove = drv_remove,
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

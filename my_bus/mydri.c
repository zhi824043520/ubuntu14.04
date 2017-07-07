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

extern struct bus_type bus;

static int my_probe(struct device *dev)
{
	printk("my_probe.\n");
	
	return 0;
}

static int my_remove(struct device *dev)
{
	printk("my_remove.\n");
	
	return 0;
}

struct device_driver drv = {
	.name 	= 	"my_test_dev",
	.bus 	= 	&bus,
	.probe	= 	my_probe,
	.remove =	my_remove,
};

static int __init my_dri_init(void)
{
	int ret = 0;
	
	ret = driver_register(&drv);
	return 0;
}

static void __exit my_dri_exit(void)
{
	driver_unregister(&drv);
}

module_init(my_dri_init);
module_exit(my_dri_exit);

MODULE_LICENSE("GPL");

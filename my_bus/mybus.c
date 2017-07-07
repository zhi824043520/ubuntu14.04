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

static int my_match(struct device *dev, struct device_driver *drv)
{
	printk("my_match.\n");
	
	return 1;
}

static int my_resume(struct device *dev)
{
	
	return 0;
}

struct bus_type bus = {
	.name 	= 	"my_test_bus",
	.match 	= 	my_match,
	.resume	= 	my_resume,
};

EXPORT_SYMBOL(bus);

static int __init my_bus_init(void)
{
	int ret = 0;
	
	ret = bus_register(&bus);
	return 0;
}

static void __exit my_bus_exit(void)
{
	bus_unregister(&bus);
}

module_init(my_bus_init);
module_exit(my_bus_exit);

MODULE_LICENSE("GPL");

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

void my_release(struct device *dev)
{
	
}

static struct device dev = {
	.init_name 	= 	"my_test_dev",
	.bus 		= 	&bus,
	.release	=	my_release,
};

static int __init my_dev_init(void)
{
	int ret = 0;
	
	ret = device_register(&dev);
	return 0;
}

static void __exit my_dev_exit(void)
{
	device_unregister(&dev);
}

module_init(my_dev_init);
module_exit(my_dev_exit);

MODULE_LICENSE("GPL");

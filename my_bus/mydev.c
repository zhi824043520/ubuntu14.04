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

static void dev_release(struct device *dev)
{
	printk("dev_release.\n");
	
	return;
}

static struct platform_device pdev = {
	.name = "my-platform",
	.dev = {
		.release = dev_release,
	},
};

static int __init my_dev_init(void)
{
	int ret = 0;
	
	ret = platform_device_register(&pdev);
	return 0;
}

static void __exit my_dev_exit(void)
{
	platform_device_unregister(&pdev);
}

module_init(my_dev_init);
module_exit(my_dev_exit);

MODULE_LICENSE("GPL");

#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of_irq.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/of.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#define GPX1BASS 0x11000C20
#define DEVICE_NAME "key"

static struct key_t {
	wait_queue_head_t wait_queue;
	struct device_node *key_node;
	struct	device* key_device;
	struct 	class* key_class;
	struct tasklet_struct t;	
	void __iomem *gpx1bass;
	struct resource* res;
	struct resource* io;
	int wake_flag;
	int irq_nr;
	int major;
	int flag;
} *key;

static ssize_t key_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	if ((key->flag == 0) && (file->f_flags & O_NONBLOCK)) {	// 按键没有按下
		return -ETXTBSY;
	}
	wait_event_interruptible(key->wait_queue, key->wake_flag);
	//copy_to_user(user_buf, &key->flag, 4);
	put_user(key->flag, user_buf);
	
	key->wake_flag = 0;

	
	return 0;
}

void do_taskle(unsigned long argv)
{
	if ((readl(key->gpx1bass + 4)&0x2) >> 1 == 0x1) {
		key->flag = 0;
	} else {
		key->flag = 1;
	}
}

static irqreturn_t key_irq(int irq, void *dev_id)
{
	wake_up_interruptible(&key->wait_queue);
	key->wake_flag = 1;
	
	tasklet_schedule(&key->t);
	
	return IRQ_HANDLED;
}

static const struct file_operations key_fops = {
	.owner		= THIS_MODULE,
	.read		= key_read,
};

static int drv_probe(struct platform_device *dev)
{
	unsigned int reg;
	int ret = 0;
	
	key = (struct key_t *)kzalloc(sizeof(struct key_t) ,GFP_KERNEL);
	if (NULL == key) {
		printk("kzmalloc is error.\n");
	}
	
	init_waitqueue_head(&key->wait_queue);
	
	tasklet_init(&key->t, do_taskle, 0);
	key->res = platform_get_resource(dev, IORESOURCE_IRQ, 0);
	
	key->major = register_chrdev(0, DEVICE_NAME, &key_fops);
	if (key->major < 0) {
		printk("register_chrdev is error.\n");
		return -1;
	}
	
	key->key_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(key->key_class)) {
		printk("class_create is error.\n");
		return -1;
	}
	
	key->key_device = device_create(key->key_class, NULL, MKDEV(key->major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(key->key_device)) {
		printk("device_create is error.\n");
		return -1;
	}
	

	key->io = platform_get_resource(dev, IORESOURCE_MEM, 0);
	
	key->gpx1bass = ioremap(key->io->start, 8);
	
	reg = readl(key->gpx1bass);
	reg &= ~(0xf << 4);
	reg |= (0xf << 4);
	writel(reg, key->gpx1bass);
	
	ret = request_irq(key->res->start, key_irq, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "key_irq", NULL);
	if (ret < 0) {
		printk("request_irq is error.\n");
		return -1;
	}
	
	printk("irq number is %d.\n", key->res->start);
	printk("drv_probe.\n");
	
	return 0;
}

static int drv_remove(struct platform_device *dev)
{
	free_irq(key->irq_nr, NULL);
	iounmap(key->gpx1bass);
	device_destroy(key->key_class, MKDEV(key->major, 0));
	class_destroy(key->key_class);
	unregister_chrdev(key->major, DEVICE_NAME);
	kfree(key);
	printk("drv_remove.\n");
	
	return 0;
}

static const struct of_device_id key_of_ids[] = {
	{ .compatible = "fs4412,keys" },
	{},
};

static struct platform_driver pdrv = {
	.driver = {
		.name = "myplatform",
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

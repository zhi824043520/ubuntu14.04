#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of_irq.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/of.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#define NAME "mykeys"

#define KEY2_UP		0
#define KEY2_DOWN 	1
#define KEY3_UP		0
#define KEY3_DOWN 	1

struct keys_dtb_t {
	struct resource *irq_nr[2];
	struct resource *reg;
};

static struct keys_t {
	struct fasync_struct *keys_fasync_queue[2];
	wait_queue_head_t wait_queue[2];
	struct device *keys_device;
	struct keys_dtb_t keys_dtb;
	struct class *keys_class;
	int key2_flag, key3_flag;
	void __iomem *gpx1base;
	int wake_flag;
	int dev_nr;
	int stat[2];
	
} *keys;

static ssize_t keys_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	if (MINOR(file->f_inode->i_rdev) == 0) {
		//printk("key2.\n");
		if ((keys->key2_flag == 0) && (file->f_flags & O_NONBLOCK)) {	// 按键没有按下
			return -ETXTBSY;
		}
		wait_event_interruptible(keys->wait_queue[0], keys->wake_flag);
		
		if (keys->stat[0] == KEY2_UP || keys->stat[0] == KEY2_DOWN) {
			put_user(keys->stat[0], user_buf);
		}
		
	} else if (MINOR(file->f_inode->i_rdev) == 1) {
		//printk("key3.\n");
		if ((keys->key3_flag == 0) && (file->f_flags & O_NONBLOCK)) {	// 按键没有按下
			return -ETXTBSY;
		}
		wait_event_interruptible(keys->wait_queue[1], keys->wake_flag);
		
		if (keys->stat[1] == KEY3_UP || keys->stat[1] == KEY3_DOWN) {
			put_user(keys->stat[1], user_buf);
		}
	}
	
	keys->wake_flag = 0;
	
	return 0;
}

static int keys_fasync (int fd, struct file *file, int on)
{
	if (MINOR(file->f_inode->i_rdev) == 0) {
		return fasync_helper(fd, file, on, &keys->keys_fasync_queue[0]);
	} else if (MINOR(file->f_inode->i_rdev) == 1) {
		return fasync_helper(fd, file, on, &keys->keys_fasync_queue[1]);
	}
	
	return -1;
}

static struct file_operations keys_fops = {
	.owner	= THIS_MODULE,
	
	.read 	= keys_read,
	.fasync = keys_fasync,
};

static irqreturn_t do_keys_irq(int irq, void *dev_id)
{	
	switch (irq) {
		case 168:
		wake_up_interruptible(&keys->wait_queue[0]);
		if ((readl(keys->gpx1base + 4)&0x2) >> 1 == 0x1) {
			keys->stat[0] = KEY2_UP;
			keys->key2_flag = 0;
		} else {
			keys->stat[0] = KEY2_DOWN;
			keys->key2_flag = 1;
		}
		kill_fasync(&keys->keys_fasync_queue[0], SIGIO, 0);
			//printk("irq_nr = %d.\n", irq);
		break;
		
		case 169:
		wake_up_interruptible(&keys->wait_queue[1]);
		if ((readl(keys->gpx1base + 4)&0x3) >> 2 == 0x1) {
			keys->stat[1] = KEY3_UP;
			keys->key3_flag = 0;
		} else {
			keys->stat[1] = KEY3_DOWN;
			keys->key3_flag = 1;
		}
		kill_fasync(&keys->keys_fasync_queue[1], SIGIO, 0);
			//printk("irq_nr = %d.\n", irq);
		break;
	}
	
	keys->wake_flag = 1;
	
	return IRQ_HANDLED;
}

static int keys_probe(struct platform_device *pdev)
{
	unsigned int reg = 0;
	int ret = 0;
	int i = 0;
	
	keys = (struct keys_t *)kzalloc(sizeof(struct keys_t), GFP_KERNEL);
	if (NULL == keys) {
		printk("kzalloc is error.\n");
		
		return -ENOMEM;
	}
	
	//keys->wake_flag = 1;
	init_waitqueue_head(&keys->wait_queue[0]);
	init_waitqueue_head(&keys->wait_queue[1]);
	
	keys->dev_nr = register_chrdev(0, NAME, &keys_fops);
	if (keys->dev_nr <= 0) {
		printk("register_chrdev is error.\n");
		return -ENXIO;
	}
	
	keys->keys_class  = class_create(THIS_MODULE, NAME);
	
	for (i = 0; i < 2; i++) {
		keys->keys_device = device_create(keys->keys_class, NULL, MKDEV(keys->dev_nr, i), NULL, "key%d", i);
	}
	
	keys->keys_dtb.reg = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	
	//printk("reg = 0x%x.\n", keys->keys_dtb.reg->start);
	
	for (i = 0; i < 2; i++) {
		keys->keys_dtb.irq_nr[i] = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		//printk("irq_nr =%d.\n", keys->keys_dtb.irq_nr[i]->start);
	}
	
	keys->gpx1base = ioremap(keys->keys_dtb.reg->start, 8);
	
	reg = readl(keys->gpx1base);
	reg |= 0xff << 4;
	writel(reg, keys->gpx1base);
	
	ret = request_irq(keys->keys_dtb.irq_nr[0]->start, do_keys_irq, 
						IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "key2", NULL);
	if (ret) {
		printk("request_irq is error.\n");
	}	
	
	ret = request_irq(keys->keys_dtb.irq_nr[1]->start, do_keys_irq, 
						IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "key3", NULL);
	if (ret) {
		printk("request_irq is error.\n");
	}	
	
	printk("keys_probe.\n");
	
	return 0;
}

static int keys_remove(struct platform_device *pdev)
{
	int i = 0;

	free_irq(keys->keys_dtb.irq_nr[0]->start, NULL);
	free_irq(keys->keys_dtb.irq_nr[1]->start, NULL);
	
	iounmap(keys->gpx1base);
	
	for (i = 0; i < 2; i++) 
		device_destroy(keys->keys_class, MKDEV(keys->dev_nr, i));
	
	class_destroy(keys->keys_class);
	unregister_chrdev(keys->dev_nr,NAME);
	kfree(keys);

	printk("keys_remove.\n");
	return 0;
}

static const struct of_device_id keys_match[] = {
	{ .compatible = "fs4412,mykey", },
	{ },
};

static struct platform_driver keys_driver = {
	.probe = keys_probe,
	.remove = keys_remove,
	.driver = {
		.name = "fs4412-keys",
		.owner = THIS_MODULE,
		.of_match_table = keys_match,
	},
};

static int __init fs4412_key_init(void)
{
	return platform_driver_register(&keys_driver);;
}

static void __exit fs4412_key_exit(void)
{
	platform_driver_unregister(&keys_driver);
}

module_init(fs4412_key_init);
module_exit(fs4412_key_exit);

MODULE_LICENSE("GPL");
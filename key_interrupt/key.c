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
#include <asm/irq.h>
#include <asm/io.h>

#define GPX1BASS 0x11000C20
#define DEVICE_NAME "key"

static struct key_t {
	struct fasync_struct *key_async_queue;
	wait_queue_head_t wait_queue;
	struct device_node *key_node;
	struct	device* key_device;
	struct 	class* key_class;	
	void __iomem *gpx1bass;
	struct resource io;
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

static int key_fasync(int fd, struct file *filp, int on)
{
	return fasync_helper(fd, filp, on, &key->key_async_queue); 
}

static const struct file_operations key_fops = {
	.owner		= THIS_MODULE,
	.read		= key_read,
	.fasync  	= key_fasync,
};

// 中断处理
static irqreturn_t key_irq(int irq, void *dev_id)
{
	wake_up_interruptible(&key->wait_queue);
	key->wake_flag = 1;
	
	if ((readl(key->gpx1bass + 4)&0x2) >> 1 == 0x1) {
		key->flag = 0;
	} else {
		key->flag = 1;
	}
	
	kill_fasync(&key->key_async_queue, SIGIO, 0);
	
	return IRQ_HANDLED;
}

static int __init my_key_init(void)
{
	unsigned int reg;
	int ret;
	
	key = (struct key_t *)kzalloc(sizeof(struct key_t) ,GFP_KERNEL);
	if (NULL == key) {
		printk("kzmalloc is error.\n");
		goto out1;
	}
	
	key->wake_flag = 0;
	init_waitqueue_head(&key->wait_queue);
	
	key->major = register_chrdev(0, DEVICE_NAME, &key_fops);
	if (key->major < 0) {
		printk("register_chrdev is error.\n");
		goto out2;
	}
	
	key->key_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(key->key_class)) {
		printk("class_create is error.\n");
		goto out3;
	}
	
	key->key_device = device_create(key->key_class, NULL, MKDEV(key->major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(key->key_device)) {
		printk("device_create is error.\n");
		goto out4;
	}
	
	// 获得设备树根节点下的keys节点
	key->key_node = of_find_node_by_path("/keys");
	
	// 得到设备树reg属性的值(这里是寄存器地址)
	of_address_to_resource(key->key_node, 0, &key->io);
	// 得到设备树中的中断号
	key->irq_nr = irq_of_parse_and_map(key->key_node, 0);
	
	// key->gpx1bass = ioremap(GPX1BASS, 8);
	key->gpx1bass = ioremap(key->io.start, 8);
	
	reg = readl(key->gpx1bass);
	reg &= ~(0xf << 4);
	reg |= (0xf << 4);
	writel(reg, key->gpx1bass);
	
	ret = request_irq(key->irq_nr, key_irq, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "key_irq", NULL);
	if (ret < 0) {
		printk("request_irq is error.\n");
		goto out5;
	}
	
	printk("key_init.\n");
	
	return 0;
	
out5:
	iounmap(key->gpx1bass);
	device_destroy(key->key_class, MKDEV(key->major, 0));
out4:
	class_destroy(key->key_class);
out3:
	unregister_chrdev(key->major, DEVICE_NAME);
out2:
	kfree(key);
out1:
	return -1;
	
}

static void __exit my_ket_exit(void)
{
	free_irq(key->irq_nr, NULL);
	iounmap(key->gpx1bass);
	device_destroy(key->key_class, MKDEV(key->major, 0));
	class_destroy(key->key_class);
	unregister_chrdev(key->major, DEVICE_NAME);
	kfree(key);
	
	printk("ket_exit.\n");
}

module_init(my_key_init);
module_exit(my_ket_exit);

MODULE_LICENSE("GPL");

#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/of.h>

#define NAME "myleds"

struct My_leds {
	// 设备名 对应设备树中的label标签
	const char *name;
	// GPIO端口号 对应设备树中的gpios标签
	unsigned int gpio_nr;
};

static struct leds_t {
	wait_queue_head_t wait_queue;
	struct device *leds_device;
	struct class *leds_class;
	struct My_leds led[4];
	int wake_flag;
	int dev_nr;
} *leds;

static long leds_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if ((leds->wake_flag == 0) && (file->f_flags & O_NONBLOCK)) {	// 按键没有按下
		return -ETXTBSY;
	}
	
	wait_event_interruptible(leds->wait_queue, leds->wake_flag);
	leds->wake_flag = 0;
	
	if ((_IOC_TYPE(cmd) != 'M')) {
		return -EPERM;
	}
	
	switch (_IOC_NR(cmd)) {
	case 1:
		if (arg == 0) {
			gpio_set_value(leds->led[0].gpio_nr, 0);
			// printk("LED1 OFF.\n");
		} else if (arg == 1) {
			gpio_set_value(leds->led[0].gpio_nr, 1);
			// printk("LED1 ON.\n");
		}
		break;
	case 2:
		if (arg == 0) {
			gpio_set_value(leds->led[1].gpio_nr, 0);
			// printk("LED2 OFF.\n");
		} else if (arg == 1) {	
			gpio_set_value(leds->led[1].gpio_nr, 1);
			// printk("LED2 ON.\n");
		}
		break;
	case 3:
		if (arg == 0) {
			gpio_set_value(leds->led[2].gpio_nr, 0);
			// printk("LED3 OFF.\n");
		} else if (arg == 1) {	
			gpio_set_value(leds->led[2].gpio_nr, 1);
			// printk("LED3 ON.\n");
		}
		break;
	case 4:
		if (arg == 0) {
			gpio_set_value(leds->led[3].gpio_nr, 0);
			// printk("LED4 OFF.\n");
		} else if (arg == 1) {	
			gpio_set_value(leds->led[3].gpio_nr, 1);
			// printk("LED4 ON.\n");
		}
		break;
	default:
		printk("cmd is error.\n");
		break;
	}
	
	leds->wake_flag = 1;
	wake_up_interruptible(&leds->wait_queue);
	
	return 0;
}

static int leds_close(struct inode *inode, struct file *file)
{
	int i = 0;
	
	for (i = 0; i < 4; i++) {
		gpio_set_value(leds->led[i].gpio_nr, 0);
	}
	
	return 0;
}

static struct file_operations leds_fops = {
	.owner			= THIS_MODULE,
	
	.unlocked_ioctl = leds_ioctl,
	.release		= leds_close,
};

static int leds_probe(struct platform_device *pdev)
{
	struct device_node *child;
	unsigned int ret = 0;
	const char *string;
	int i = 0;
	
	leds = (struct leds_t *)kzalloc(sizeof(struct leds_t), GFP_KERNEL);
	if (NULL == leds) {
		printk("kzalloc is error.\n");
		
		return -ENOMEM;
	}
	
	leds->wake_flag = 1;
	init_waitqueue_head(&leds->wait_queue);
	
	leds->dev_nr = register_chrdev(0, NAME, &leds_fops);
	if (leds->dev_nr <= 0) {
		printk("register_chrdev is error.\n");
		return -ENXIO;
	}
	
	leds->leds_class  = class_create(THIS_MODULE, NAME);
	leds->leds_device = device_create(leds->leds_class, NULL, MKDEV(leds->dev_nr, 0), NULL, NAME);
	
	for_each_child_of_node(pdev->dev.of_node, child) {
		if (pdev->dev.of_node) {
			// 获得gpio端口号 也就是"gpios"标签
			leds->led[i].gpio_nr = of_get_named_gpio(child, "gpios", 0);
			
			// 获得节点名 也就是"label"标签 并将首地址存入string
			ret = of_property_read_string(child, "label", &string);
			// 如果没有指定"label"标签 则使用子节点的名字
			leds->led[i].name = (ret == 0) ? string : child->name;
			//printk("gpio_nr = %d.\n", leds->led[i].gpio_nr);
		}
		
		i++;
	}
	
	gpio_request(leds->led[0].gpio_nr, "led2");
	gpio_request(leds->led[1].gpio_nr, "led3");
	gpio_request(leds->led[2].gpio_nr, "led4");
	gpio_request(leds->led[3].gpio_nr, "led5");
	
	// 将申请到的gpio号设置为输出模式 并且使管脚输出低电平
	for (i = 0; i < 4; i++)
		gpio_direction_output(leds->led[i].gpio_nr, 0);
	
	printk("leds_probe.\n");
	return 0;
}

static int leds_remove(struct platform_device *pdev)
{
	int i = 0;
	
	for (i = 0; i < 4; i++) {
		gpio_set_value(leds->led[i].gpio_nr, 0);
		gpio_free(leds->led[i].gpio_nr);
	}
	
	device_destroy(leds->leds_class, MKDEV(leds->dev_nr, 0));
	class_destroy(leds->leds_class);
	unregister_chrdev(leds->dev_nr,NAME);
	kfree(leds);
	
	printk("leds_remove.\n");
	return 0;
}

static const struct of_device_id leds_match[] = {
	{ .compatible = "fs4412,leds", },
	{ },
};

static struct platform_driver leds_driver = {
	.probe = leds_probe,
	.remove = leds_remove,
	.driver = {
		.name = "fs4412-leds",
		.owner = THIS_MODULE,
		.of_match_table = leds_match,
	},
};

static int __init fs4412_led_init(void)
{
	return platform_driver_register(&leds_driver);;
}

static void __exit fs4412_led_exit(void)
{
	platform_driver_unregister(&leds_driver);
}

module_init(fs4412_led_init);
module_exit(fs4412_led_exit);

MODULE_LICENSE("GPL");

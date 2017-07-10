#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>

static int mpu6050_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	printk("mpu6050 addr: 0x%x.\n", i2c->addr);
	printk("mpu6050_i2c_probe.\n");
	
	return 0;
}

static int mpu6050_i2c_remove(struct i2c_client *i2c)
{
	printk("mpu6050_i2c_remove.\n");
	return 0;
}

static const struct of_device_id mpu6050_i2c_match[] = {
	{ .compatible = "fs4412,mpu6050", },
	{ },
};

static const struct i2c_device_id mpu6050_i2c_ids[] = {
	{ }
};

static struct i2c_driver my_i2c_driver = {
	.driver = {
		   .name = "mpu6050",
		   .owner = THIS_MODULE,
		   .of_match_table = mpu6050_i2c_match,
	},
	.probe 		= mpu6050_i2c_probe,
	.remove 	= mpu6050_i2c_remove,
	.id_table 	= mpu6050_i2c_ids,
};

static int __init my_i2c_init(void)
{
	return i2c_add_driver(&my_i2c_driver);
}

static void __exit my_i2c_exit(void)
{
	i2c_del_driver(&my_i2c_driver);
}

module_init(my_i2c_init);
module_exit(my_i2c_exit);

MODULE_LICENSE("GPL v2");

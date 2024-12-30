/* $Id: pca9551.c 51161 2008-05-08 22:57:25Z mdr $ */
/*
 * pca9551.c - LED driver with programmable blink rates.
 *
 * Copyright 2006 Xiotech Corporation
 *
 * Bryan Holty (Bryan_Holty@Xiotech.com)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Supports following chips:
 * 
 * PCA9551
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>

#define ID	"pca9551: "

/* Addresses to scan */
static unsigned short normal_i2c[] =
    {0x60, I2C_CLIENT_END};

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(pca9551);

/* PCA9551 Registers */
#define PCA9551_REG_INPUT	0x00	/* RO State of device pins */
#define PCA9551_REG_PSC0	0x01	/* Frequency Prescalar 0 */
#define PCA9551_REG_PWM0	0x02	/* Duty Cycle of Blink 0 */
#define PCA9551_REG_PSC1	0x03	/* Frequency Prescalar 1 */
#define PCA9551_REG_PWM1	0x04	/* Duty Cycle of Blink 1 */
#define PCA9551_REG_LS0		0x05	/* LED 0 - LED 3 Selector */
#define PCA9551_REG_LS1		0x06	/* LED 4 - LED 7 Selector */

static int pca9551_detect(struct i2c_adapter *adapter, int addr, int kind);

struct pca9551_data {
	struct i2c_client client;
	struct mutex update_lock;
};

/* following are the sysfs callback functions */
static inline ssize_t pca9551_show_register(struct device *dev, u32 reg_addr,
		char *buf)
{
	int ret = -1;
	struct i2c_client *client = to_i2c_client(dev);
	struct pca9551_data *data = i2c_get_clientdata(client);
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_read_byte_data(client, reg_addr);
	mutex_unlock(&data->update_lock);
	if (ret < 0)
		return -EBUSY;
	return sprintf(buf, "%d\n", ret);
}

static inline ssize_t pca9551_store_register(struct device *dev, u32 reg_addr,
		const char *buf, size_t count)
{
	int ret = -1;
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 0);
	struct pca9551_data *data = i2c_get_clientdata(client);
	if (val > 0xff)
		return -EINVAL;
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, reg_addr, val);
	mutex_unlock(&data->update_lock);
	if (ret < 0)
		return ret;
	return count;
}

static inline ssize_t pca9551_led_show(struct device *dev,
		u32 led, char *buf)
{
	int ret = -1;
	struct i2c_client *client = to_i2c_client(dev);
	struct pca9551_data *data = i2c_get_clientdata(client);
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_read_byte_data(client, PCA9551_REG_LS0 + (led / 4));
	mutex_unlock(&data->update_lock);
	if (ret < 0)
		return -EBUSY;
	return sprintf(buf, "%d\n", ((ret >> (2 * (led % 4))) & 0x03));
}

static inline ssize_t pca9551_led_store(struct device *dev,
		u32 led, const char *buf, size_t count)
{
	int ret = -1;
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 0);
	struct pca9551_data *data = i2c_get_clientdata(client);
	if (val > 0x03)
		return -EINVAL;
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_read_byte_data(client, PCA9551_REG_LS0 + (led / 4));
	if (ret < 0) {
		mutex_unlock(&data->update_lock);
		return -EBUSY;
	}
	val = (val << (2 * (led % 4))) |
		((u32)ret & ~(0x03 << (2 * (led % 4))));
	ret = i2c_smbus_write_byte_data(client,
		PCA9551_REG_LS0 + (led / 4), val);
	mutex_unlock(&data->update_lock);
	if (ret < 0)
		return ret;
	return count;
}

/* Define the device attributes */
#define PCA9551_ENTRY_RO(name,num,reg) \
static ssize_t show_in##reg##_input(struct device *dev, \
		struct device_attribute *attr, char *buf) \
{ \
	return pca9551_show_register(dev, reg, buf); \
} \
static DEVICE_ATTR(name##num, S_IRUGO, show_in##reg##_input, NULL);


#define PCA9551_ENTRY_RW(name,num,reg) \
static ssize_t show_rw##reg##_io(struct device *dev, \
		struct device_attribute *attr, char *buf) \
{ \
	return pca9551_show_register(dev, reg, buf); \
} \
static ssize_t store_rw##reg##_io(struct device *dev, \
		struct device_attribute *attr, const char *buf, size_t count) \
{ \
	return pca9551_store_register(dev, reg, buf, count); \
} \
static DEVICE_ATTR(name##num, S_IRUGO | S_IWUSR, \
		show_rw##reg##_io, store_rw##reg##_io);


#define PCA9551_ENTRY_LED_RW(ledid) \
static ssize_t show_led##ledid##_io(struct device *dev, \
		struct device_attribute *attr, char *buf) \
{ \
	return pca9551_led_show(dev, ledid, buf); \
} \
static ssize_t store_led##ledid##_io(struct device *dev, \
		struct device_attribute *attr, const char *buf, size_t count) \
{ \
	return pca9551_led_store(dev, ledid, buf, count); \
} \
static DEVICE_ATTR(led##ledid, S_IRUGO | S_IWUSR, \
		show_led##ledid##_io, store_led##ledid##_io);


PCA9551_ENTRY_RO(input, 0, PCA9551_REG_INPUT);
PCA9551_ENTRY_RW(blink_period, 0, PCA9551_REG_PSC0);
PCA9551_ENTRY_RW(blink_period, 1, PCA9551_REG_PSC1);
PCA9551_ENTRY_RW(blink_cycle, 0, PCA9551_REG_PWM0);
PCA9551_ENTRY_RW(blink_cycle, 1, PCA9551_REG_PWM1);
PCA9551_ENTRY_LED_RW(0);
PCA9551_ENTRY_LED_RW(1);
PCA9551_ENTRY_LED_RW(2);
PCA9551_ENTRY_LED_RW(3);
PCA9551_ENTRY_LED_RW(4);
PCA9551_ENTRY_LED_RW(5);
PCA9551_ENTRY_LED_RW(6);
PCA9551_ENTRY_LED_RW(7);

static int pca9551_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, pca9551_detect);
}

static int pca9551_detach_client(struct i2c_client *client)
{
	int err;

	if ((err = i2c_detach_client(client)))
		return err;

	kfree(i2c_get_clientdata(client));
	return 0;
}

/* This is the driver that will be inserted */
static struct i2c_driver pca9551_driver = {
	.driver = {
		.name	= "pca9551",
	},
	.attach_adapter	= pca9551_attach_adapter,
	.detach_client	= pca9551_detach_client,
};

/* This function is called by i2c_probe */
static int pca9551_detect(struct i2c_adapter *adapter, int addr, int kind)
{
	struct i2c_client *new_client;
	struct pca9551_data *data;
	s32 res;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		goto exit;

	/* OK. For now, we presume we have a valid client. We now create the
	   client structure, even though we cannot fill it completely yet. */
	if (!(data = kzalloc(sizeof(struct pca9551_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	new_client = &data->client;
	i2c_set_clientdata(new_client, data);
	new_client->addr = addr;
	mutex_init(&data->update_lock);
	new_client->adapter = adapter;
	new_client->driver = &pca9551_driver;
	new_client->flags = 0;
	strlcpy(new_client->name, "pca9551", I2C_NAME_SIZE);

	printk(KERN_WARNING ID
		"checking 0x%02X, func 0x%08X, class 0x%08X, kind %d\n",
		(u8)addr, (u32)i2c_get_functionality(adapter),
		(u32)adapter->class, kind);

	/* If this is a probe, verify that is correct */
	if (kind < 0)
	{
		/* Detection: the pca9551 only has 7 registers (0-6).
		   Read the seven and ensure they work, ensure the 8th fails */
		/* Look for initial PCA9551_REG_INPUT setting */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_INPUT);
		if (res <= 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_INPUT\n");
			goto exit_kfree;
		}
		/* Look for initial PCA9551_REG_PSC0 setting */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_PSC0);
		if (res < 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_PSC0\n");
			goto exit_kfree;
		}
		/* Look for initial PCA9551_REG_PWM0 setting */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_PWM0);
		if (res < 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_PWM0\n");
			goto exit_kfree;
		}
		/* Look for initial PCA9551_REG_PSC1 setting */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_PSC1);
		if (res < 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_PSC1\n");
			goto exit_kfree;
		}
		/* Look for initial PCA9551_REG_PWM1 setting */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_PWM1);
		if (res < 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_PWM1\n");
			goto exit_kfree;
		}
		/* Look for initial PCA9551_REG_LS0 setting */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_LS0);
		if (res < 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_LS0\n");
			goto exit_kfree;
		}
		/* Look for initial PCA9551_REG_LS1 setting */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_LS1);
		if (res < 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_LS1\n");
			goto exit_kfree;
		}
		/* Ensure an additional register read fails */
		res = i2c_smbus_read_byte_data(new_client, PCA9551_REG_LS1+1);
		if (res >= 0) {
			printk(KERN_WARNING ID
				"Detection failed PCA9551_REG_LS1+1\n");
			goto exit_kfree;
		}
	}

	/* Tell the I2C layer a new client has arrived */
	if ((err = i2c_attach_client(new_client)))
		goto exit_kfree;

	dev_warn(&new_client->dev, "Registered @ 0x%02X\n", (u8)addr);

	/* Initialize device to defaults. */
	/* Defaults from data sheet. */
	i2c_smbus_write_byte_data(new_client, PCA9551_REG_PSC0, 0xFF);
	i2c_smbus_write_byte_data(new_client, PCA9551_REG_PSC1, 0xFF);
	i2c_smbus_write_byte_data(new_client, PCA9551_REG_PWM0, 0x80);
	i2c_smbus_write_byte_data(new_client, PCA9551_REG_PWM1, 0x80);
	i2c_smbus_write_byte_data(new_client, PCA9551_REG_LS0, 0x55);
	i2c_smbus_write_byte_data(new_client, PCA9551_REG_LS1, 0x55);

	/* Create sysfs files */
	device_create_file(&new_client->dev, &dev_attr_input0);
	device_create_file(&new_client->dev, &dev_attr_blink_period0);
	device_create_file(&new_client->dev, &dev_attr_blink_period1);
	device_create_file(&new_client->dev, &dev_attr_blink_cycle0);
	device_create_file(&new_client->dev, &dev_attr_blink_cycle1);
	device_create_file(&new_client->dev, &dev_attr_led0);
	device_create_file(&new_client->dev, &dev_attr_led1);
	device_create_file(&new_client->dev, &dev_attr_led2);
	device_create_file(&new_client->dev, &dev_attr_led3);
	device_create_file(&new_client->dev, &dev_attr_led4);
	device_create_file(&new_client->dev, &dev_attr_led5);
	device_create_file(&new_client->dev, &dev_attr_led6);
	device_create_file(&new_client->dev, &dev_attr_led7);

	return 0;

exit_kfree:
	kfree(data);
exit:
	return err;
}

static int __init pca9551_init(void)
{
	return i2c_add_driver(&pca9551_driver);
}

static void __exit pca9551_exit(void)
{
	i2c_del_driver(&pca9551_driver);
}

MODULE_AUTHOR("Xiotech Corporation");
MODULE_DESCRIPTION("PCA9551 driver");
MODULE_LICENSE("GPL");

module_init(pca9551_init);
module_exit(pca9551_exit);

/**
** vi:sw=8 ts=8 noexpandtab
**/


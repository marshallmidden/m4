/* $Id: m1z3.c 51161 2008-05-08 22:57:25Z mdr $ */
/*
 * m1z3.c - Zippy Redundant Switching Power Supply.
 *
 * Copyright 2006 Xiotech Corporation
 *
 * Bryan Holty (Bryan_Holty@Xiotech.com)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Supports following power supply:
 * m1z3
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/random.h>

#define ID	"m1z3: "

/* M1Z3 Registers */
#define M1Z3_VOLTAGE_3_3	0x10	/* RO 3.3 Voltage */
#define M1Z3_VOLTAGE_5		0x11	/* RO 5 Voltage */
#define M1Z3_VOLTAGE_12		0x12	/* RO 12 Voltage */
#define M1Z3_TEMPERATURE	0x14	/* RO Temperature in deg C */
#define M1Z3_STATUS		0x92	/* RO Power Supply Status */

#define M1Z3_ID_HI		0x5A	/* ID high byte */
#define M1Z3_ID_LO		0x5B	/* ID low byte */

#define M1Z3_MUXADDR		0x70	/* Address if PCA9540 mux */

/* Addresses to scan */
static unsigned short normal_i2c[] = {M1Z3_MUXADDR, 0x2d, I2C_CLIENT_END};

static int m1z3_detect(struct i2c_adapter *adapter, int addr, int kind);

struct m1z3_data {
	struct i2c_client client;
	struct i2c_client muxclient;
	struct class_device *class_dev;
};

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(m1z3);

static struct m1z3_data *mydata;


/* Provide for access through a pca9540 mux */

static s32 set_mux(u8 value)
{
	struct i2c_adapter *adapter = mydata->muxclient.adapter;

	if (!adapter)
		return 0;

	return adapter->algo->smbus_xfer(adapter, mydata->muxclient.addr,
		mydata->muxclient.flags & I2C_M_TEN,
		I2C_SMBUS_WRITE, value, I2C_SMBUS_BYTE, NULL);
}

static void	rdelay(void)
{
	u8	random;

	get_random_bytes(&random, sizeof(random));

	udelay((random & 1) * 100 + 1);
}

static s32 mux_smbus_read_byte(u32 reg)
{
	struct i2c_adapter *adapter;
	union i2c_smbus_data data;
	s32	res;
	int	retry;

	adapter = mydata->client.adapter;
	if (!mydata || !adapter)
		return -ENODEV;
	for (retry = 0; retry < 4; ++retry, up(&adapter->bus_lock), rdelay()) {
		down(&adapter->bus_lock);
		if (set_mux(5))
			continue;

		res = adapter->algo->smbus_xfer(adapter, mydata->client.addr,
			mydata->client.flags & I2C_M_TEN,
			I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data);
		if (res) {
			set_mux(0);
			continue;
		}
		if (set_mux(0))
			continue;

		up(&adapter->bus_lock);
		if (retry)
			printk(KERN_INFO ID "Retry=%d\n", retry);
		return data.byte & 0x0FF;
	}
	printk(KERN_INFO ID "Retries exhausted\n");
	return -1;
}

/* following are the sysfs callback functions */
/* RO registers */
static ssize_t show_register(struct device *dev, u32 reg, char *buf)
{
	return sprintf(buf, "%d\n", mux_smbus_read_byte(reg));
}

#define M1Z3_RO_REGISTER(name,reg) \
static ssize_t show_in##reg##_input(struct device *dev, \
		struct device_attribute *attr, char *buf) \
{ \
	return show_register(dev, reg, buf); \
} \
static DEVICE_ATTR(name, S_IRUGO, show_in##reg##_input, NULL);

M1Z3_RO_REGISTER(voltage_3_3, M1Z3_VOLTAGE_3_3);
M1Z3_RO_REGISTER(voltage_5, M1Z3_VOLTAGE_5);
M1Z3_RO_REGISTER(voltage_12, M1Z3_VOLTAGE_12);
M1Z3_RO_REGISTER(temperature, M1Z3_TEMPERATURE);

/* RO Status */
static ssize_t show_status(struct device *dev, u32 supply, char *buf)
{
	int ret = -1;
	ret = mux_smbus_read_byte(M1Z3_STATUS);
	return sprintf(buf, "%d\n", ((ret >> supply) & 0x01));
}

#define M1Z3_RO_STATUS(supply) \
static ssize_t show_in##supply##_status(struct device *dev, \
		struct device_attribute *attr, char *buf) \
{ \
	return show_status(dev, supply, buf); \
} \
static DEVICE_ATTR(status##supply, S_IRUGO, show_in##supply##_status, NULL);

M1Z3_RO_STATUS(0);
M1Z3_RO_STATUS(1);
M1Z3_RO_STATUS(2);


static int m1z3_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, m1z3_detect);
}

static int m1z3_detach_client(struct i2c_client *client)
{
	struct m1z3_data *data = i2c_get_clientdata(client);
	int err;

	if (data && client == &data->client)
		hwmon_device_unregister(data->class_dev);

	if ((err = i2c_detach_client(client)))
		return err;

	client->adapter = 0;

	if (data->client.adapter == 0 && data->muxclient.adapter == 0) {
		kfree(data);
		mydata = 0;
	}

	return 0;
}

/* This is the driver that will be inserted */
static struct i2c_driver m1z3_driver = {
	.driver	= {
		.name	= "m1z3",
	},
	.attach_adapter	= m1z3_attach_adapter,
	.detach_client	= m1z3_detach_client,
};


/* This function is called by i2c_probe */
static int m1z3_detect(struct i2c_adapter *adapter, int addr, int kind)
{
	struct i2c_client *new_client;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		goto exit;

	/* OK. For now, we presume we have a valid client. We now create the
	   client structure, even though we cannot fill it completely yet. */
	if (!mydata && !(mydata = kzalloc(sizeof(*mydata), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	if (addr == M1Z3_MUXADDR) {
		new_client = &mydata->muxclient;
		strlcpy(new_client->name, "pca9540", I2C_NAME_SIZE);
	} else {
		new_client = &mydata->client;
		strlcpy(new_client->name, "m1z3", I2C_NAME_SIZE);
	}

	if (!new_client->adapter) {
		i2c_set_clientdata(new_client, mydata);
		new_client->addr = addr;
		new_client->adapter = adapter;
		new_client->driver = &m1z3_driver;
		new_client->flags = 0;
	} else
		printk(KERN_WARNING ID "%s adapter already set\n",
			new_client->name);

	printk(KERN_WARNING ID
		"checking 0x%02X, func 0x%08X, class 0x%08X, kind %d\n",
		(u8)addr, (u32)i2c_get_functionality(adapter),
		(u32)adapter->class, kind);

	/* If this is a probe, verify that is correct */
	if (kind < 0 ) {
		s32 reg;

		switch (addr) {
		case M1Z3_MUXADDR:
			reg = i2c_smbus_read_byte(new_client);
			printk(KERN_INFO ID "Mux read returned %d (%02x)\n",
				reg, reg);
			if (reg < 0)
				new_client->adapter = 0;
			break;

		default:
			/* Detection: Check the ID that matches this chip. */
			reg = mux_smbus_read_byte(M1Z3_ID_HI);
			if (reg != 0x02) {
				printk(KERN_WARNING ID
					"Detection failed M1Z3_ID_HI\n");
				goto exit_kfree;
			}
			/* Detection: Check the ID that matches this chip. */
			reg = mux_smbus_read_byte(M1Z3_ID_LO);
			if (reg != 0x04) {
				printk(KERN_WARNING ID
					"Detection failed M1Z3_ID_LO\n");
				goto exit_kfree;
			}
		}
	}

	if (!new_client->adapter) {
		printk(KERN_WARNING ID "No mux\n");
		return 0;
	}

	/* Tell the I2C layer a new client has arrived */
	if ((err = i2c_attach_client(new_client)))
		goto exit_kfree;

	dev_warn(&new_client->dev, "Registered @ 0x%02X\n", (u8)addr);

	if (addr == M1Z3_MUXADDR)
		return 0;

	/* Register with hwmon */
	mydata->class_dev = hwmon_device_register(&new_client->dev);
	if (IS_ERR(mydata->class_dev)) {
		err = PTR_ERR(mydata->class_dev);
		goto exit_unreg_i2c;
	}

	/* Create sysfs files */
	device_create_file(&new_client->dev, &dev_attr_voltage_3_3);
	device_create_file(&new_client->dev, &dev_attr_voltage_5);
	device_create_file(&new_client->dev, &dev_attr_voltage_12);
	device_create_file(&new_client->dev, &dev_attr_temperature);
	device_create_file(&new_client->dev, &dev_attr_status0);
	device_create_file(&new_client->dev, &dev_attr_status1);
	device_create_file(&new_client->dev, &dev_attr_status2);

	return 0;

exit_unreg_i2c:
	i2c_detach_client(new_client);

exit_kfree:
	new_client->adapter = 0;
	if (mydata->client.adapter == 0 && mydata->muxclient.adapter == 0) {
		kfree(mydata);
		mydata = 0;
	} else
		printk(KERN_WARNING ID "no kfree, client.adapter=%p, "
			"muxclient.adapter=%p\n", mydata->client.adapter,
			mydata->muxclient.adapter);

exit:
	return err;
}

static int __init m1z3_init(void)
{
	return i2c_add_driver(&m1z3_driver);
}

static void __exit m1z3_exit(void)
{
	i2c_del_driver(&m1z3_driver);
}

MODULE_AUTHOR("Xiotech Corporation");
MODULE_DESCRIPTION("m1z3 driver");
MODULE_LICENSE("GPL");

module_init(m1z3_init);
module_exit(m1z3_exit);

/**
** vi:sw=8 ts=8 noexpandtab
**/


/*
 * coretemp.c - Linux kernel module for hardware monitoring
 *
 * Copyright (C) 2007 Rudolf Marek <r.marek@assembler.cz>
 *
 * Inspired from many hwmon drivers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/hwmon.h>
#include <linux/sysfs.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <asm/msr.h>
#include <asm/processor.h>

#define DRVNAME	"coretemp"

typedef enum { SHOW_TEMP, SHOW_TJMAX, SHOW_TTARGET, SHOW_LABEL,
		SHOW_NAME } SHOW;

/*
 * Functions declaration
 */

static struct coretemp_data *coretemp_update_device(struct device *dev);

struct coretemp_data {
	struct class_device *hwmon_dev;
	struct mutex update_lock;
	const char *name;
	u32 id;
	char valid;		/* zero until following fields are valid */
	unsigned long last_updated;	/* in jiffies */
	int temp;
	int tjmax;
	int ttarget;
	u8 alarm;
};


static inline int rdmsr_eio(u32 reg, u32 *eax, u32 *edx)
{
        int err;

        err = rdmsr_safe(reg, eax, edx);
        if (err)
                err = -EIO;
        return err;
}

#ifdef	CONFIG_SMP
struct msr_command {
	int cpu;
	int err;
	u32 reg;
	u32 data[2];
};

static void msr_smp_rdmsr(void *cmd_block)
{
	struct msr_command *cmd = (struct msr_command *)cmd_block;

	if (cmd->cpu == smp_processor_id())
		cmd->err = rdmsr_eio(cmd->reg, &cmd->data[0], &cmd->data[1]);
}

static inline int do_rdmsr(int cpu, u32 reg, u32 * eax, u32 * edx)
{
	struct msr_command cmd;
	int ret;

	preempt_disable();
	if (cpu == smp_processor_id()) {
		ret = rdmsr_eio(reg, eax, edx);
	} else {
		cmd.cpu = cpu;
		cmd.reg = reg;

		smp_call_function(msr_smp_rdmsr, &cmd, 1, 1);

		*eax = cmd.data[0];
		*edx = cmd.data[1];

		ret = cmd.err;
	}
	preempt_enable();
	return ret;
}
#else
static inline int do_rdmsr(int cpu, u32 reg, u32 *eax, u32 *edx)
{
	return rdmsr_eio(reg, eax, edx);
}
#endif /* CONFIG_SMP */


/*
 * Sysfs stuff
 */

static ssize_t show_name(struct device *dev, struct device_attribute
			  *devattr, char *buf)
{
	int ret;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct coretemp_data *data = dev_get_drvdata(dev);

	if (attr->index == SHOW_NAME)
		ret = sprintf(buf, "%s\n", data->name);
	else	/* show label */
		ret = sprintf(buf, "Core %d\n", data->id);
	return ret;
}

static ssize_t show_alarm(struct device *dev, struct device_attribute
			  *devattr, char *buf)
{
	struct coretemp_data *data = coretemp_update_device(dev);
	/* read the Out-of-spec log, never clear */
	return sprintf(buf, "%d\n", data->alarm);
}

static ssize_t show_temp(struct device *dev,
			 struct device_attribute *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct coretemp_data *data = coretemp_update_device(dev);
	int err;

	if (attr->index == SHOW_TEMP)
		err = data->valid ? sprintf(buf, "%d\n", data->temp) : -EAGAIN;
	else if (attr->index == SHOW_TJMAX)
		err = sprintf(buf, "%d\n", data->tjmax);
	else
		err = sprintf(buf, "%d\n", data->ttarget);
	return err;
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp, NULL,
			  SHOW_TEMP);
static SENSOR_DEVICE_ATTR(temp1_crit, S_IRUGO, show_temp, NULL,
			  SHOW_TJMAX);
static SENSOR_DEVICE_ATTR(temp1_max, S_IRUGO, show_temp, NULL,
			  SHOW_TTARGET);
static DEVICE_ATTR(temp1_crit_alarm, S_IRUGO, show_alarm, NULL);
static SENSOR_DEVICE_ATTR(temp1_label, S_IRUGO, show_name, NULL, SHOW_LABEL);
static SENSOR_DEVICE_ATTR(name, S_IRUGO, show_name, NULL, SHOW_NAME);

static struct attribute *coretemp_attributes[] = {
	&sensor_dev_attr_name.dev_attr.attr,
	&sensor_dev_attr_temp1_label.dev_attr.attr,
	&dev_attr_temp1_crit_alarm.attr,
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp1_crit.dev_attr.attr,
	NULL
};

static const struct attribute_group coretemp_group = {
	.attrs = coretemp_attributes,
};

static struct coretemp_data *coretemp_update_device(struct device *dev)
{
	struct coretemp_data *data = dev_get_drvdata(dev);

	mutex_lock(&data->update_lock);

	if (!data->valid || time_after(jiffies, data->last_updated + HZ)) {
		u32 eax, edx;

		data->valid = 0;
		do_rdmsr(data->id, MSR_IA32_THERM_STATUS, &eax, &edx);
		data->alarm = (eax >> 5) & 1;
		/* update only if data has been valid */
		if (eax & 0x80000000) {
			data->temp = data->tjmax - (((eax >> 16)
							& 0x7f) * 1000);
			data->valid = 1;
		} else {
			dev_dbg(dev, "Temperature data invalid (0x%x)\n", eax);
		}
		data->last_updated = jiffies;
	}

	mutex_unlock(&data->update_lock);
	return data;
}

static int __devinit adjust_tjmax(struct cpuinfo_x86 *c, u32 id, struct device *dev)
{
	/* The 100C is default for both mobile and non mobile CPUs */

	int tjmax = 100000;
	int ismobile = 1;
	int err;
	u32 eax, edx;

	/* Early chips have no MSR for TjMax */

	if ((c->x86_model == 0xf) && (c->x86_mask < 4)) {
		ismobile = 0;
	}

	if ((c->x86_model > 0xe) && (ismobile)) {

		/* Now we can detect the mobile CPU using Intel provided table
		   http://softwarecommunity.intel.com/Wiki/Mobility/720.htm
		   For Core2 cores, check MSR 0x17, bit 28 1 = Mobile CPU
		*/

		err = do_rdmsr(id, 0x17, &eax, &edx);
		if (err) {
			dev_warn(dev,
				 "Unable to access MSR 0x17, assuming desktop"
				 " CPU\n");
			ismobile = 0;
		} else if (!(eax & 0x10000000)) {
			ismobile = 0;
		}
	}

	if (ismobile) {

		err = do_rdmsr(id, 0xee, &eax, &edx);
		if (err) {
			dev_warn(dev,
				 "Unable to access MSR 0xEE, for Tjmax, left"
				 " at default");
		} else if (eax & 0x40000000) {
			tjmax = 85000;
		}
	} else {
		dev_warn(dev, "Using relative temperature scale!\n");
	}

	return tjmax;
}

static int __devinit coretemp_probe(struct platform_device *pdev)
{
	struct coretemp_data *data;
	struct cpuinfo_x86 *c = &(cpu_data)[pdev->id];
	int err;
	u32 eax, edx;

	if (!(data = kzalloc(sizeof(struct coretemp_data), GFP_KERNEL))) {
		err = -ENOMEM;
		dev_err(&pdev->dev, "Out of memory\n");
		goto exit;
	}

	data->id = pdev->id;
	data->name = "coretemp";
	mutex_init(&data->update_lock);

	/* test if we can access the THERM_STATUS MSR */
	err = do_rdmsr(data->id, MSR_IA32_THERM_STATUS, &eax, &edx);
	if (err) {
		dev_err(&pdev->dev,
			"Unable to access THERM_STATUS MSR, giving up\n");
		goto exit_free;
	}

	/* Check if we have problem with errata AE18 of Core processors:
	   Readings might stop update when processor visited too deep sleep,
	   fixed for stepping D0 (6EC).
	*/

	if ((c->x86_model == 0xe) && (c->x86_mask < 0xc)) {
		/* check for microcode update */
		do_rdmsr(data->id, MSR_IA32_UCODE_REV, &eax, &edx);
		if (edx < 0x39) {
			err = -ENODEV;
			dev_err(&pdev->dev,
				"Errata AE18 not fixed, update BIOS or "
				"microcode of the CPU!\n");
			goto exit_free;
		}
	}

	data->tjmax = adjust_tjmax(c, data->id, &pdev->dev);
	platform_set_drvdata(pdev, data);

	/* read the still undocumented IA32_TEMPERATURE_TARGET it exists
	   on older CPUs but not in this register */

	if (c->x86_model > 0xe) {
		err = do_rdmsr(data->id, 0x1a2, &eax, &edx);
		if (err) {
			dev_warn(&pdev->dev, "Unable to read"
					" IA32_TEMPERATURE_TARGET MSR\n");
		} else {
			data->ttarget = data->tjmax -
					(((eax >> 8) & 0xff) * 1000);
			err = device_create_file(&pdev->dev,
					&sensor_dev_attr_temp1_max.dev_attr);
			if (err)
				goto exit_free;
		}
	}

	if ((err = sysfs_create_group(&pdev->dev.kobj, &coretemp_group)))
		goto exit_dev;

	data->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		dev_err(&pdev->dev, "Class registration failed (%d)\n",
			err);
		goto exit_class;
	}

	return 0;

exit_class:
	sysfs_remove_group(&pdev->dev.kobj, &coretemp_group);
exit_dev:
	device_remove_file(&pdev->dev, &sensor_dev_attr_temp1_max.dev_attr);
exit_free:
	kfree(data);
exit:
	return err;
}

static int __devexit coretemp_remove(struct platform_device *pdev)
{
	struct coretemp_data *data = platform_get_drvdata(pdev);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&pdev->dev.kobj, &coretemp_group);
	device_remove_file(&pdev->dev, &sensor_dev_attr_temp1_max.dev_attr);
	platform_set_drvdata(pdev, NULL);
	kfree(data);
	return 0;
}

static struct platform_driver coretemp_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = DRVNAME,
	},
	.probe = coretemp_probe,
	.remove = __devexit_p(coretemp_remove),
};

struct pdev_entry {
	struct list_head list;
	struct platform_device *pdev;
	unsigned int cpu;
};

static LIST_HEAD(pdev_list);
static DEFINE_MUTEX(pdev_list_mutex);

static int __cpuinit coretemp_device_add(unsigned int cpu)
{
	int err;
	struct platform_device *pdev;
	struct pdev_entry *pdev_entry;

	pdev = platform_device_alloc(DRVNAME, cpu);
	if (!pdev) {
		err = -ENOMEM;
		printk(KERN_ERR DRVNAME ": Device allocation failed\n");
		goto exit;
	}

	pdev_entry = kzalloc(sizeof(struct pdev_entry), GFP_KERNEL);
	if (!pdev_entry) {
		err = -ENOMEM;
		goto exit_device_put;
	}

	err = platform_device_add(pdev);
	if (err) {
		printk(KERN_ERR DRVNAME ": Device addition failed (%d)\n",
		       err);
		goto exit_device_free;
	}

	pdev_entry->pdev = pdev;
	pdev_entry->cpu = cpu;
	mutex_lock(&pdev_list_mutex);
	list_add_tail(&pdev_entry->list, &pdev_list);
	mutex_unlock(&pdev_list_mutex);

	return 0;

exit_device_free:
	kfree(pdev_entry);
exit_device_put:
	platform_device_put(pdev);
exit:
	return err;
}

#ifdef CONFIG_HOTPLUG_CPU
static void coretemp_device_remove(unsigned int cpu)
{
	struct pdev_entry *p, *n;
	mutex_lock(&pdev_list_mutex);
	list_for_each_entry_safe(p, n, &pdev_list, list) {
		if (p->cpu == cpu) {
			platform_device_unregister(p->pdev);
			list_del(&p->list);
			kfree(p);
		}
	}
	mutex_unlock(&pdev_list_mutex);
}

static int __cpuinit coretemp_cpu_callback(struct notifier_block *nfb,
				 unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long) hcpu;

	switch (action) {
	case CPU_ONLINE:
	case CPU_DOWN_FAILED:
		coretemp_device_add(cpu);
		break;
	case CPU_DOWN_PREPARE:
		coretemp_device_remove(cpu);
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block coretemp_cpu_notifier __refdata = {
	.notifier_call = coretemp_cpu_callback,
};
#endif				/* !CONFIG_HOTPLUG_CPU */

static int __init coretemp_init(void)
{
	int i, err = -ENODEV;
	struct pdev_entry *p, *n;

	/* quick check if we run Intel */
	if ((cpu_data)[0].x86_vendor != X86_VENDOR_INTEL)
		goto exit;

	err = platform_driver_register(&coretemp_driver);
	if (err)
		goto exit;

	for_each_online_cpu(i) {
		struct cpuinfo_x86 *c = &(cpu_data)[i];

		/* check if family 6, models 0xe, 0xf, 0x16, 0x17, 0x1A */
		if ((c->cpuid_level < 0) || (c->x86 != 0x6) ||
		    !((c->x86_model == 0xe) || (c->x86_model == 0xf) ||
			(c->x86_model == 0x16) || (c->x86_model == 0x17) ||
			(c->x86_model == 0x1A))) {

			/* supported CPU not found, but report the unknown
			   family 6 CPU */
			if ((c->x86 == 0x6) && (c->x86_model > 0xf))
				printk(KERN_WARNING DRVNAME ": Unknown CPU "
					"model %x\n", c->x86_model);
			continue;
		}

		err = coretemp_device_add(i);
		if (err)
			goto exit_devices_unreg;
	}
	if (list_empty(&pdev_list)) {
		err = -ENODEV;
		goto exit_driver_unreg;
	}

#ifdef CONFIG_HOTPLUG_CPU
	register_hotcpu_notifier(&coretemp_cpu_notifier);
#endif
	return 0;

exit_devices_unreg:
	mutex_lock(&pdev_list_mutex);
	list_for_each_entry_safe(p, n, &pdev_list, list) {
		platform_device_unregister(p->pdev);
		list_del(&p->list);
		kfree(p);
	}
	mutex_unlock(&pdev_list_mutex);
exit_driver_unreg:
	platform_driver_unregister(&coretemp_driver);
exit:
	return err;
}

static void __exit coretemp_exit(void)
{
	struct pdev_entry *p, *n;
#ifdef CONFIG_HOTPLUG_CPU
	unregister_hotcpu_notifier(&coretemp_cpu_notifier);
#endif
	mutex_lock(&pdev_list_mutex);
	list_for_each_entry_safe(p, n, &pdev_list, list) {
		platform_device_unregister(p->pdev);
		list_del(&p->list);
		kfree(p);
	}
	mutex_unlock(&pdev_list_mutex);
	platform_driver_unregister(&coretemp_driver);
}

MODULE_AUTHOR("Rudolf Marek <r.marek@assembler.cz>");
MODULE_DESCRIPTION("Intel Core temperature monitor");
MODULE_LICENSE("GPL");

module_init(coretemp_init)
module_exit(coretemp_exit)

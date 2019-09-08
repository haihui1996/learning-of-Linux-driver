/////////////////////////////////////////////////////////////////////////////
//@FileName:  led_drv.c
//@Path:      D:\Linux\SourceCode\linux-2.6.38\linux-2.6.38\drivers\Mini2440\9th_led_bus_drv_dev
//@Description: 【驱动程序分离分层概念】：一个设备的驱动程序可以分为硬件设备驱动和事件驱动，
//      硬件设备驱动是指和硬件直接相关的部分，而事件驱动是指从设备驱动中抽离出来的稳定的部分
//      （和函数的概念相似），此处为后者。用于分配、设置、注册一个platform_driver
//@Copyright (c) 2019 Haihui Deng
//@Author     haihui.deng@longsys.com 2019/09/08
/////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>

static int major;
static struct calss *cls;
static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int led_open(struct inode *inode, struct file *file)
{
    /* 配置为输出 */
    *gpio_con &= ~(0x03 << (pin*2));
    *gpio_con |= (0x01 << (pin*2));
    return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("first_drv_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// 点灯
		*gpio_dat &= ~(1<<pin);
	}
	else
	{
		// 灭灯
		*gpio_dat |= (1<<pin);
	}
	
	return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open   = led_open,
    .write  = led_write,
};

static int led_probe(struct platform_device *pdev)
{
    struct resource *res;

    /* 根据platform_device的资源进行ioremap */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    gpio_con = ioremap(res->start, res->end - res->start + 1);
    gpio_dat = gpio_con + 1;

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    pin = res->start;

    /* 注册字符设备驱动程序 */
    printk("led_probe, found led\n");
    major = register_chrdev(0, "myled", &led_fops);
    cls = class_create(THIS_MODULE, "myled");
    device_create(cls, NULL, MKDEV(major, 0), NULL, "led");
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    /* 卸载字符设备驱动程序 */
    /* iounmap */
    printk("led_remove, remove led\n");

    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, "myled");
    iounmap(gpio_con);

    return 0;
}

struct platform_driver led_drv = {
    .probe  = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "myled",
    }
};

static int led_drv_init(void)
{
    platform_driver_register(&led_drv);
    return 0;
}

static void led_drv_exit(void)
{
    platform_driver_unregister(&led_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");

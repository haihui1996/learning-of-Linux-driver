/////////////////////////////////////////////////////////////////////////////
//@FileName:  led_dev.c
//@Path:      D:\Linux\SourceCode\linux-2.6.38\linux-2.6.38\drivers\Mini2440\9th_led_bus_drv_dev
//@Description:  【驱动程序分离分层概念】：一个设备的驱动程序可以分为硬件设备驱动和事件驱动，
//      硬件设备驱动是指和硬件直接相关的部分，而事件驱动是指从设备驱动中抽离出来的稳定的部分
//      （和函数的概念相似），此处为前者。用于分配、设置、注册一个platform_device
//@Copyright (c) 2019 Haihui Deng
//@Author     haihui.deng@longsys.com 2019/09/08
/////////////////////////////////////////////////////////////////////////////
#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

static srtuct resource led_resource[] = {
    [0] = {
        .start  =   0x56000010,
        .end    =   0x56000010 + 8 - 1,
        .flags  =   IORESOURCE_MEM,
    },
    [1] = {
        .start  =   5,
        .end    =   5,
        .flags  =   IORESOURCE_IRQ,
    }
};

static void led_release(struct device *dev)
{

}

static struct platform_device led_dev = {
    .name = "myled",
    .id = -1,
    .num_resources = led_resource,
    .dev = {
        .release = led_release,
    },
};

static int led_dev_init(void)
{
    platform_device_register(&led_dev);
    return 0;
}

static void led_dev_exit(void)
{
    platform_device_unregister(&led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
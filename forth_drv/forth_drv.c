#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <mach/gpio-fns.h>
#include <linux/sched.h>
#include <linux/poll.h>

/**
 * poll 机制
 */

static struct class *forthdrv_class;
static struct device *forthdrv_class_device;


// volatile unsigned long *gpfcon


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志，中断服务程序将它置一，forth_drv_read将它清零 */
static volatile int ev_press = 0;

struct pin_desc
{
    unsigned int pin;
    unsigned int key_val;
};

/* 键值：按下时，0x01,0x02,0x03,0x04 */
/* 键值：松开时，0x81,0x82,0x83,0x84 */
static unsigned char key_val;

/**
 * k1,k2,k3,k4对应GPG0,GPG3,GPG5,GPG6
 */
struct pin_desc pins_desc[4] = {
    {S3C2410_GPG0_EINT8, 0x01},
    {S3C2410_GPG3_EINT11, 0x02},
    {S3C2410_GPG5_EINT13, 0x03},
    {S3C2410_GPG6_EINT14, 0x04},
};

/**
 * 确定按键值
 */

static irqreturn_t buttons_irq(int irq, void * dev_id)
{
    struct pin_desc * pindesc = (struct pin_desc *)dev_id;
    unsigned int pinval;

    pinval = s3c2410_gpio_getpin(pindesc->pin);
    if (pinval)
    {
        /* 松开 */
        key_val = 0x80 | pindesc->key_val;
    }
    else
    {
        /* 按下 */
        key_val = pindesc->key_val;
    }
    ev_press = 1;   /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠进程 */

    return IRQ_HANDLED;
}

static int forth_drv_open(struct inode *inode, struct file * file)
{
    /* 注册中断服务 */
    request_irq(IRQ_EINT8, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k1", &pins_desc[0]);
    request_irq(IRQ_EINT11, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k2", &pins_desc[0]);
    request_irq(IRQ_EINT13, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k3", &pins_desc[0]);
    request_irq(IRQ_EINT14, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k4", &pins_desc[0]);
    return 0;
}

ssize_t forth_drv_read(struct file * file, char __user *buf, size_t size, loff_t *ppos)
{
    if (size != 1)
        return -EINVAL;

    /* 如果没有按键动作，休眠 */
    wait_event_interruptible(button_waitq, ev_press);

    /* 如果有按键有动作，返回键值 */
    copy_to_user(buf, &key_val, 1);
    ev_press = 0;
    return 1;
}

int forth_drv_close(struct inode * inode, struct file *file)
{
    free_irq(IRQ_EINT8, &pins_desc[0]);
    free_irq(IRQ_EINT11, &pins_desc[1]);
    free_irq(IRQ_EINT13, &pins_desc[2]);
    free_irq(IRQ_EINT14, &pins_desc[3]);
    return 0;
}

static unsigned forth_drv_poll(struct file * file, poll_table * wait)
{
    unsigned int mask = 0;
    poll_wait(file, &button_waitq, wait);

    if (ev_press)
        mask |= POLLIN | POLLRDNORM;
    
    return mask;
}

static struct file_operations forth_drv_fops = {
    .owner  = THIS_MODULE,
    .open   = forth_drv_open,
    .read   = forth_drv_read,
    .release = forth_drv_close,
    .poll   = forth_drv_poll,
};

int major;
static int forth_drv_init(void)
{
    major = register_chrdev(0, "forth_drv", &forth_drv_fops);
    forthdrv_class = class_create(THIS_MODULE, "forth_drv");
    forthdrv_class_device = device_create(forthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons");
    return 0;
}

static void forth_drv_exit(void)
{
    unregister_chrdev(major, "forth_drv");
    device_unregister(forthdrv_class_device);
    class_destroy(forthdrv_class);
    return 0;
}

module_init(forth_drv_init);
module_exit(forth_drv_exit);

MODULE_LICENSE("GPL");
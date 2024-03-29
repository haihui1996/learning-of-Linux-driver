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

static struct class *thirddrv_class;
static struct device *thirddrv_class_dev;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志，中断服务成将他置一，third_drv_read 将它清零 */
static volatile int ev_press = 0;

struct pin_desc {
    unsigned int pin;
    unsigned int key_val;
};

/* 键值：按下时，0x01, 0x02, 0x03, 0x04 */
/* 键值：松开时，0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

/**
 * K1,K2,K3,K4对应，GPG0、3、5、6
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
static irqreturn_t buttons_irq(int irq, void *dev_id)
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

    ev_press = 1; /* 表示发生了中断 */
    wake_up_interruptible(&button_waitq); /* 唤醒休眠的进程 */
    
    /*
    IRQ_REPLAY：被禁止的中断号上又产生了中断，这个中断是不会被处理的，
    当这个中断号被允许产生中断时，会将这个未被处理的中断转为IRQ_REPLAY。
     */
    return IRQ_HANDLED;
}

static int third_drv_open(struct inode *inode, struct file *file)
{
    /* 申请中断号，并设置为双边沿触发 */
    request_irq(IRQ_EINT8, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K1", &pins_desc[0]);
    request_irq(IRQ_EINT11, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K2", &pins_desc[1]);
    request_irq(IRQ_EINT13, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K3", &pins_desc[2]);
    request_irq(IRQ_EINT14, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K4", &pins_desc[3]);
    return 0;
}

static ssize_t third_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    if (size != 1)
    {
        return -EINVAL;
    }
    /* 如果按键没有动作，休眠 */
    wait_event_interruptible(button_waitq, ev_press);

    /* 如果案件有动作，返回按键值 */
    copy_to_user(buf, &key_val, 1);
    ev_press = 0;

    return 1;    
}

static int third_drv_close(struct inode *inode, struct file *file)
{
    free_irq(IRQ_EINT8, &pins_desc[0]);
    free_irq(IRQ_EINT11, &pins_desc[1]);
    free_irq(IRQ_EINT13, &pins_desc[2]);
    free_irq(IRQ_EINT14, &pins_desc[3]);
    return 0;
}

static struct file_operations third_drv_fops = {
    .owner = THIS_MODULE,
    .open = third_drv_open,
    .read = third_drv_read,
    .release = third_drv_close,
};

int major;
static int third_drv_init(void)
{
    major = register_chrdev(0, "third_drv", &third_drv_fops);
    thirddrv_class = class_create(THIS_MODULE, "third_drv");
    thirddrv_class_dev = device_create(thirddrv_class, NULL, MKDEV(major,0), NULL, "buttons");

    return 0;
}

static void third_drv_exit(void)
{
    unregister_chrdev(major, "third_drv");
    device_unregister(thirddrv_class_dev);
    class_destroy(thirddrv_class);
}

module_init(third_drv_init);
module_exit(third_drv_exit);

MODULE_LICENSE("GPL");
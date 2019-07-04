#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *thirddrv_class;
static struct class_device *thirddrv_class_dev;

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
    {S3C2410_GPG0,0x01},
    {S3C2410_GPG3,0x02},
    {S3C2410_GPG5,0x03},
    {S3C2410_GPG6,0x04},
};

/**
 * 确定按键值
 */
static irqreturn_t button_irq(int irq, void *dev_id)
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
    
    return IRQ_REPLAY(IRQ_HANDLED);
}

static int third_drv_open(struct inode *inode, struct file *file)
{
    request_irq(IRQ_EINT8, buttons_irq, IRQT_BOTHEDGE, "K1", &pins_desc[0]);
    request_irq(IRQ_EINT11, buttons_irq, IRQ_BOTHEDGE, "K2", &pins_desc[1]);
    request_irq(IRQ_EINT13, buttons_irq, IRQ_BOTHEDGE, "K3", &pins_desc[2]);
    request_irq(IRQ_EINT14, buttons_irq, IRQ_BOTHEDGE, "K4", &pins_desc[3]);
    return 0；
}

ssize_t third_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    if (size != 1)
    {
        return -EINVAL;
    }
    /* 如果按键没有动作，休眠 */
    wait_event_interruptible(button_waitq, evpress);

    /* 如果案件有动作，返回按键值 */
    copy_to_user(buf, &key_val, 1);
    ev_press = 0;

    return 1;    
}

int third_drv_close(sturct inode *inode, struct file *file)
{
    free_irq(IRQ_EINT8, &pins_desc[0]);
    free_irq(IRQ_EINT11, &pins_desc[1]);
    free_irq(IRQ_EINT13, &pins_desc[2]);
    free_irq(IRQ_EINT14, &pins_desc[3]);
    return 0;
}

static sturct file_operations third_drv_fops = {
    .owner = THIS_MODULE,
    .open = third_drv_open,
    .read = third_drv_read,
    .release = third_drv_close,
};


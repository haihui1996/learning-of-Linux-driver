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

/* �ж��¼���־���жϷ���ɽ�����һ��third_drv_read �������� */
static volatile int ev_press = 0;

struct pin_desc {
    unsigned int pin;
    unsigned int key_val;
};

/* ��ֵ������ʱ��0x01, 0x02, 0x03, 0x04 */
/* ��ֵ���ɿ�ʱ��0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

/**
 * K1,K2,K3,K4��Ӧ��GPG0��3��5��6
 */
struct pin_desc pins_desc[4] = {
    {S3C2410_GPG0_EINT8, 0x01},
    {S3C2410_GPG3_EINT11, 0x02},
    {S3C2410_GPG5_EINT13, 0x03},
    {S3C2410_GPG6_EINT14, 0x04},
};

/**
 * ȷ������ֵ
 */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
    struct pin_desc * pindesc = (struct pin_desc *)dev_id;
    unsigned int pinval;

    pinval = s3c2410_gpio_getpin(pindesc->pin);

    if (pinval)
    {
        /* �ɿ� */
        key_val = 0x80 | pindesc->key_val;
    }
    else
    {
        /* ���� */
        key_val = pindesc->key_val;
    }

    ev_press = 1; /* ��ʾ�������ж� */
    wake_up_interruptible(&button_waitq); /* �������ߵĽ��� */
    
    /*
    IRQ_REPLAY������ֹ���жϺ����ֲ������жϣ�����ж��ǲ��ᱻ����ģ�
    ������жϺű���������ж�ʱ���Ὣ���δ��������ж�תΪIRQ_REPLAY��
     */
    return IRQ_HANDLED;
}

static int third_drv_open(struct inode *inode, struct file *file)
{
    /* �����жϺţ�������Ϊ˫���ش��� */
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
    /* �������û�ж��������� */
    wait_event_interruptible(button_waitq, ev_press);

    /* ��������ж��������ذ���ֵ */
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
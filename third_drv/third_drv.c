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
    {S3C2410_GPG0,0x01},
    {S3C2410_GPG3,0x02},
    {S3C2410_GPG5,0x03},
    {S3C2410_GPG6,0x04},
};

/**
 * ȷ������ֵ
 */
static irqreturn_t button_irq(int irq, void *dev_id)
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
    
    return IRQ_REPLAY(IRQ_HANDLED);
}
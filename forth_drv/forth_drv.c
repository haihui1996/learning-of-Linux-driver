#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>




static struct class *forthdrv_class;
static struct class_device *forthdrv_class_device;


// volatile unsigned long *gpfcon


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־���жϷ����������һ��forth_drv_read�������� */
static volatile int ev_press = 0;

struct pin_desc
{
    unsigned int pin;
    unsigned int key_val;
};

/* ��ֵ������ʱ��0x01,0x02,0x03,0x04 */
/* ��ֵ���ɿ�ʱ��0x81,0x82,0x83,0x84 */
static unsigned char key_val;

/**
 * k1,k2,k3,k4��ӦGPG0,GPG3,GPG5,GPG6
 */
struct pin_desc pins_desc[4] = {
    {S3C2410_GPG0, 0x01},
	{S3C2410_GPG3, 0x02},
	{S3C2410_GPG5, 0x03},
	{S3C2410_GPG6, 0x04},
};

/**
 * ȷ������ֵ
 */

static irqreturn_t buttons_irq(int irq, void * dev_id)
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
    ev_press = 1;   /* ��ʾ�жϷ����� */
    wake_up_interruptible(&button_waitq);   /* �������߽��� */

    return IRQ_HANDLED;
}

static int forth_drv_open(struct inode *inode, struct file * file)
{
    request_irq(INQ)
}
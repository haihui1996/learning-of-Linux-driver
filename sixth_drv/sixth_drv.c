/////////////////////////////////////////////////////////////////////////////
//@FileName:  sixth_drv.c
//@Path:      d:\Linux\SourceCode\linux-2.6.38\linux-2.6.38\drivers\Mini2440\sixth_drv
//@Description: ʹ��ԭ�Ӳ����ͻ�����ʵ��������ͬ�����⹦�ܣ���ͬһʱ��ֻ����һ������ʹ�ø�����
//@Copyright (c) 2019 Haihui Deng
//@Author     haihui.deng@longsys.com 2019/08/03
/////////////////////////////////////////////////////////////////////////////
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

static struct class *sixth_class;
static struct device *sixthdrv_class_dev;

/* ����ȴ����� */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־���жϷ����������1, read����������0 */
static volatile int ev_press = 0;

/* �첽�����ṹ��ָ�� */
static struct fasnyc_struct *button_async;

struct pin_desc
{
    unsigned int pin;
    unsigned int key_val;
};

/* ����ֵ */
static unsigned char key_val;


/* �ܽŶ��� */
struct pin_desc pins_desc[4] = 
{
    {S3C2410_GPG0_EINT8, 0x01},
    {S3C2410_GPG3_EINT11, 0x02},
    {S3C2410_GPG5_EINT13, 0x03},
    {S3C2410_GPG6_EINT14, 0x04}
};

/* ���廥���� */
static DECLARE_MUTEX(button_lock);

/**
***************************************************************************
*@brief:  �жϷ������ȷ������ֵ
*@param:  irq:�жϺţ�dev_id:
*@return: 
*@warning: 
*@Author      haihui.deng@longsys.com 2019/08/03
***************************************************************************
*/
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
    struct pin_desc * pindesc = (struct pin_desc *)dev_id;
    unsigned int pinval;

    /* ��ȡ�����ź� */
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
    ev_press = 1;/* ��ʾ�жϷ����� */
    
    /* �������߽��� */
    wake_up_interruptible(&button_waitq);

    /* �����ź� */
    kill_fasync(&button_async, SIGIO, POLL_IN);

    return IRQ_HANDLED;
}

/**
***************************************************************************
*@brief:  open
*@param:  
*@return: 
*@warning: 
*@Author      haihui.deng@longsys.com 2019/08/04
***************************************************************************
*/
static int sixth_drv_open(struct inode *inode, struct file *file)
{
    /* ��������ȡ */
    if (file->f_flags & O_NONBLOCK)
    {
        /* ������û�б��ͷţ��򷵻��豸��æ */
        if (down_trylock(&button_lock))
            return -EBUSY;
    }
    else /* ������ȡ */
    {
        /* ��ȡ�ź��� */
        down(&button_lock);
    }

    /* ע���жϷ��� */
    request_irq(IRQ_EINT8, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k1", &pins_desc[0]);
    request_irq(IRQ_EINT11, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k2", &pins_desc[0]);
    request_irq(IRQ_EINT13, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k3", &pins_desc[0]);
    request_irq(IRQ_EINT14, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "k4", &pins_desc[0]);
    
    return 0;
}

/**
***************************************************************************
*@brief:  read
*@param:  
*@return: 
*@warning: 
*@Author      haihui.deng@longsys.com 2019/08/04
***************************************************************************
*/
ssize_t sixth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    /* ������Ч���жϣ�ֻ�����ȡһ���ֽ���Ϣ */
    if(size != 1)
        return -EINVAL;
    
    if (file->f_flags & O_NONBLOCK)
    {
        if(!ev_press)
            return -EAGAIN;
    }
    else
    {
        /* ���û�а������������� */
        wait_event_interrupt(button_waitq, ev_press);
    }

    /* ����а������������ؼ�ֵ */
    copy_to_user(buf, &key_val, 1);
    ev_press = 0; /* �жϱ�־�������� */

    return size;   
}

/**
***************************************************************************
*@brief:  close
*@param:  
*@return: 
*@warning: 
*@Author      haihui.deng@longsys.com 2019/08/04
***************************************************************************
*/
int sixth_drv_close(struct inode *inode, struct file *file)
{
    /* �ͷ��ж� */
    free_irq(IRQ_EINT8,&pins_desc[0]);
    free_irq(IRQ_EINT11,&pins_desc[1]);
    free_irq(IRQ_EINT13,&pins_desc[2]);
    free_irq(IRQ_EINT14,&pins_desc[3]);

    /* �ͷŻ����� */
    up(&button_lock);

    return 0;
}

/**
***************************************************************************
*@brief:  poll
*@param:  
*@return: 
*@warning: 
*@Author      haihui.deng@longsys.com 2019/08/04
***************************************************************************
*/
static unsigned sixth_drv_poll(struct file *file, poll_table * wait)
{
    unsigned int mask = 0;
    poll_wait(file, &button_waitq, wait);

    if(ev_press)
        mask |= POLLIN | POLLRDNORM;
    return mask;
}

/**
***************************************************************************
*@brief:  fasync
*@param:  
*@return: 
*@warning:
*@Author      haihui.deng@longsys.com 2019/08/04
***************************************************************************
*/
static int sixth_drv_fasync(int fd, struct file *filp, int on)
{
    printk("driver:sixth_drv_fasync\n");
    return fasync_helper (fd, filp, on, &button_async);
}

/* �����ļ������ṹ�� */
static  struct file_operations sixth_drv_fops ={
    .owner = THIS_MODULE,
    .open   = sixth_drv_open,
    .read   = sixth_drv_read,
    .release    = sixth_drv_close,
    .poll   = sixth_drv_poll,
    .fasync = sixth_drv_fasync,
};

int major;
/**
***************************************************************************
*@brief:  ģ���ʼ������
*@param:  
*@return: 
*@warning:
*@Author      haihui.deng@longsys.com 2019/08/04
***************************************************************************
*/
static int sixth_drv_init(void)
{
    /* ע���ַ��豸���������豸�� */
    major = register_chrdev(0, "sixth_drv", &sixth_drv_fops);

    sixth_class = class_create(THIS_MODULE, "sixth_drv");

    sixthdrv_class_dev = device_create(sixth_class, NULL, MKDEV(major, 0), NULL, "buttons");

    return 0;
}

/**
***************************************************************************
*@brief:  ģ���ͷź���
*@param:  
*@return: 
*@warning:
*@Author      haihui.deng@longsys.com 2019/08/04
***************************************************************************
*/
static void sixth_drv_exit(void)
{
    unregister_chrdev (major, "sixth_drv");
    
    device_unregister(sixthdrv_class_dev);

    class_destroy(sixth_class);

    retrurn 0;
}


module_init(sixth_drv_init);

module_exit(sixth_drv_exit);

MODULE_LICENSE("GPL");
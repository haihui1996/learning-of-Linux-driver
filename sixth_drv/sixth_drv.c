/////////////////////////////////////////////////////////////////////////////
//@FileName:  sixth_drv.c
//@Path:      d:\Linux\SourceCode\linux-2.6.38\linux-2.6.38\drivers\Mini2440\sixth_drv
//@Description: 使用原子操作和互斥锁实现驱动的同步互斥功能，即同一时间只允许一个程序使用该驱动
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

/* 定义等待队列 */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志，中断服务程序将它置1, read函数将它清0 */
static volatile int ev_press = 0;

/* 异步操作结构体指针 */
static struct fasnyc_struct *button_async;

struct pin_desc
{
    unsigned int pin;
    unsigned int key_val;
};

/* 按键值 */
static unsigned char key_val;


/* 管脚定义 */
struct pin_desc pins_desc[4] = 
{
    {S3C2410_GPG0_EINT8, 0x01},
    {S3C2410_GPG3_EINT11, 0x02},
    {S3C2410_GPG5_EINT13, 0x03},
    {S3C2410_GPG6_EINT14, 0x04}
};

/* 定义互斥锁 */
static DECLARE_MUTEX(button_lock);

/**
***************************************************************************
*@brief:  中断服务程序，确定按键值
*@param:  irq:中断号，dev_id:
*@return: 
*@warning: 
*@Author      haihui.deng@longsys.com 2019/08/03
***************************************************************************
*/
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
    struct pin_desc * pindesc = (struct pin_desc *)dev_id;
    unsigned int pinval;

    /* 获取引脚信号 */
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
    ev_press = 1;/* 表示中断发生了 */
    
    /* 唤醒休眠进程 */
    wake_up_interruptible(&button_waitq);

    /* 发送信号 */
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
    /* 非阻塞读取 */
    if (file->f_flags & O_NONBLOCK)
    {
        /* 互斥锁没有被释放，则返回设备正忙 */
        if (down_trylock(&button_lock))
            return -EBUSY;
    }
    else /* 阻塞读取 */
    {
        /* 获取信号量 */
        down(&button_lock);
    }

    /* 注册中断服务 */
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
    /* 参数有效性判断，只允许读取一个字节信息 */
    if(size != 1)
        return -EINVAL;
    
    if (file->f_flags & O_NONBLOCK)
    {
        if(!ev_press)
            return -EAGAIN;
    }
    else
    {
        /* 如果没有按键动作，休眠 */
        wait_event_interrupt(button_waitq, ev_press);
    }

    /* 如果有按键动作，返回键值 */
    copy_to_user(buf, &key_val, 1);
    ev_press = 0; /* 中断标志变量清零 */

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
    /* 释放中断 */
    free_irq(IRQ_EINT8,&pins_desc[0]);
    free_irq(IRQ_EINT11,&pins_desc[1]);
    free_irq(IRQ_EINT13,&pins_desc[2]);
    free_irq(IRQ_EINT14,&pins_desc[3]);

    /* 释放互斥锁 */
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

/* 定义文件操作结构体 */
static  struct file_operations sixth_drv_fops ={
    .owner = THIS_MODULE,
    .open   = sixth_drv_open,
    .read   = sixth_drv_read,
    .release    = sixth_drv_close,
    .poll   = sixth_drv_poll,
    .fasync = sixth_drv_fasync,
};


/* 参考driver/input/keyboard/gpio_keys.c */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

static struct input_dev *buttons_dev;   /* input 结构体指针 */
struct pin_desc{
    int irq;
    char *name;
    unsigned int pin;
    unsigned int key_val;
};

struct pin_desc pins_desc[4] = {
    {S3C2410_GPG0_EINT8, "K1", S3C2410_GPG0, KEY_L},
    {S3C2410_GPG3_EINT11, "K2", S3C2410_GPG3, KEY_S},
    {S3C2410_GPG5_EINT13, "K3", S3C2410_GPG5, KEY_ENTER},
    {S3C2410_GPG6_EINT14, "K4", S3C2410_GPG6, KEY_LEFTSHIFT},
};

static struct pin_desc * irq_pd;
static struct timer_list buttons_timer; /* 用于定时消抖动 */

/**
***************************************************************************
*@brief:  
*@param:  
*@return: 
*@warning:
*@Author      haihui.deng@longsys.com 2019/09/05
***************************************************************************
*/
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
    /* 10ms 后启动定时器 */
    irq_pd = (struct pin_desc * )dev_id;
    mod_timer(&buttons_timer, jiffies+ HZ / 100);
    return IRQ_HANDLED;
}

/**
***************************************************************************
*@brief:  定时器中断处理函数
*@param:  
*@return: 
*@warning:
*@Author      haihui.deng@longsys.com 2019/09/05
***************************************************************************
*/
static void buttons_timer_function(unsigned long data)
{
    struct pin_desc * pindesc = irq_pd;
    unsigned int pinval;

    if (!pinval)
        return;

    /* 获取引脚值：参数为引脚号，返回引脚电压 */
    pinval = s3c2410_gpio_getpin(pindesc->pin);

    if (pinval)
    {
        /* 松开: 最后一个参数： 0-松开，1-按下 */
        /**
         * input_event() - report new input event
         * @dev: device that generated the event
         * @type: type of the event
         * @code: event code
         * @value: value of the event
        */
        input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
        input_sync(buttons_dev);
    }
    else
    {
        /* 按下 */
        input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
        input_sync(buttons_dev);
    }  
}


static int buttons_init(void)
{
    /* 1. 分配一个input_dev结构体 */
    buttons_dev = input_allocate_device();

    /* 2. 初始化input_dev结构体 */
    /* 2.1 能产生哪类事件 */
    set_bit(EV_KEY, buttons_dev->evbit); // 按键类事件
    set_bit(EV_REP, buttons_dev->evbit); // 自动重复类事件（按键需要重复扫描键盘判断按下）

    /* 2.2 能产生这类操作里的哪些事件：L S ENTER LEFTSHIT */
    set_bit(KEY_L, buttons_dev->keybit);
    set_bit(KEY_S, buttons_dev->keybit);
    set_bit(KEY_ENTER, buttons_dev->keybit);
    set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

    /* 3. 注册 */
    input_register_device(buttons_dev);
    
    /* 4. 硬件相关操作 */
    /* 4.1 定时器防抖动 */
    init_timer(&buttons_timer);
    buttons_timer.function = buttons_timer_function;
    add_timer(&buttons_timer);  /* 启动定时器 */

    /* 申请中断 */
    for ( i = 0; i < 4; i++)
    {
        request_irq(pins_desc[i].irq, buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,\
            pins_desc[i].name, &pins_desc[i])
    }
    
    return 0;
}

static void buttons_exit(void)
{

}


module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
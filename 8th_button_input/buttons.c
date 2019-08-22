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

static int buttons_init(void)
{
    /* 1. 分配一个input_dev结构体 */
    buttons_dev = input_allocate_device();

    /* 2. 设置 */
    /* 2.1 能产生哪类事件 */
    set_bit(EV_KEY, buttons_dev->evbit);
    set_bit(EV_REP, buttons_dev->evbit);

    /* 2.2 能产生这类操作里的那些事件：L S ENTER LEFTSHIT */
    set_bit(KEY_L, buttons_dev->keybit);

    /* 3. 注册 */
    
    /* 4. 硬件相关操作 */
    return 0;
}

static void buttons_exit(void)
{

}


module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
/* �ο�driver/input/keyboard/gpio_keys.c */

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

static struct input_dev *buttons_dev;   /* input �ṹ��ָ�� */

static int buttons_init(void)
{
    /* 1. ����һ��input_dev�ṹ�� */

    /* 2. ���� */

    /* 3. ע�� */
    
    /* 4. Ӳ����ز��� */
    return 0;
}

static void buttons_exit(void)
{

}


module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
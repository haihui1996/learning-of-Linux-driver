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
struct pin_desc{
    int irq;
    char *name;
    unsigned int pin;
    unsigned int key_val;
};

struct pin_desc pins_desc[4] = {
    {IRQ_EINT8, "K1", S3C2410_GPG0, KEY_L},
    {IRQ_EINT8, "K2", S3C2410_GPG3, KEY_S},
    {IRQ_EINT8, "K3", S3C2410_GPG5, KEY_ENTER},
    {IRQ_EINT8, "K4", S3C2410_GPG6, KEY_LEFTSHIFT},
};


static int buttons_init(void)
{
    /* 1. ����һ��input_dev�ṹ�� */
    buttons_dev = input_allocate_device();

    /* 2. ���� */
    /* 2.1 �ܲ��������¼� */
    set_bit(EV_KEY, buttons_dev->evbit);
    set_bit(EV_REP, buttons_dev->evbit);

    /* 2.2 �ܲ���������������Щ�¼���L S ENTER LEFTSHIT */
    set_bit(KEY_L, buttons_dev->keybit);
    set_bit(KEY_S, buttons_dev->keybit);
    set_bit(KEY_ENTER, buttons_dev->keybit);
    set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

    /* 3. ע�� */
    input_register_device(buttons_dev);
    
    /* 4. Ӳ����ز��� */
    init_timer()
    return 0;
}

static void buttons_exit(void)
{

}


module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
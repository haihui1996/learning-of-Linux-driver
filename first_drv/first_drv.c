#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
 
static struct class *firstdrv_class;
static struct device	*firstdrv_class_dev;

volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;

static int first_drv_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/*
	 * LED1,LED2,LED4对应GPB5、GPB6、GPB7、GPB8
	 */
	/* 配置GPB5,6,7,8为输出 */
	*gpbcon &= ~((0x3<<(5*2)) | (0x3<<(6*2)) | (0x3<<(7*2)) | (0x3<<(8*2)));
	*gpbcon |= ((0x1<<(5*2)) | (0x1<<(6*2)) | (0x1<<(7*2)) | (0x1<<(8*2)));
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    int val;

    printk("first_drv_write\n");

    copy_from_user(&val, buf, count);

    if (val == 1)
    {
        // turn on led
        *gpbdat &= ~((1<<5) | (1<<6) | (1<<7) | (1<<8) );
    }
    else
    {
        // turn off led
        *gpbdat |= (1<<5) | (1<<6) | (1<<7) | (1<<8);
    }
    
    return 0;
    
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,     
	.write	=	first_drv_write,	   
};


int major;
static int first_drv_init(void)
{
	major = register_chrdev(0, "first_drv", &first_drv_fops); // 注册, 告诉内核

	firstdrv_class = class_create(THIS_MODULE, "firstdrv");

	firstdrv_class_dev = device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/xyz */

	gpbcon = (volatile unsigned long *)ioremap(0x56000010, 16);
	gpbdat = gpbcon + 1;

	return 0;
}

static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv"); // 卸载

	device_unregister(firstdrv_class_dev);
	class_destroy(firstdrv_class);
	iounmap(gpbcon);
}

module_init(first_drv_init);
module_exit(first_drv_exit);


MODULE_LICENSE("GPL");


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/reg-gpio.h>
#include <asm/hardware.h>

static struct class *seconddrv_class;
static struct class_device *seconddrv_class_dev;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static int second_drv_open(struct inode *inode, struct file *file)
{
    /**
     * K1,k2,k3,k4∂‘”¶GPG0°¢3°¢5°¢6
     */
    /* ≈‰÷√GPIO */
    *gpgcon &= 
}
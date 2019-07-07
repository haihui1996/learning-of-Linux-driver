#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>

static struct class *seconddrv_class;
static struct device *seconddrv_class_dev;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static int second_drv_open(struct inode *inode, struct file *file)
{
    /**
     * K1,k2,k3,k4对应GPG0、3、5、6
     */
    /* 配置GPIO */
    *gpgcon &= ~((0x03<< (0*2)) | (0x03 << (3*2)) | (0x03<<5*2) | (0x03 << (6*2)));

    return 0;
}

ssize_t second_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    /* 返回四个引脚电平 */
    unsigned char key_val[4];
    int regval;

    if(size != sizeof(key_val))
        return -EINVAL;

    regval = *gpgdat;
    key_val[0] = (regval & (1<<0)) ? 1 : 0;
    key_val[1] = (regval & (1<<3)) ? 1 : 0;
    key_val[2] = (regval & (1<<5)) ? 1 : 0;
    key_val[3] = (regval & (1<<6)) ? 1 : 0;

    copy_to_user(buf, key_val, sizeof(key_val));

    return sizeof(key_val);
}

static struct file_operations second_drv_fops = {
    .owner = THIS_MODULE,
    .open = second_drv_open,
    .read = second_drv_read,
};

int major;
static int second_drv_init(void)
{
    major = register_chrdev(0, "second_drv", &second_drv_fops);
    seconddrv_class = class_create(THIS_MODULE, "second_drv");
    seconddrv_class_dev = device_create(seconddrv_class, NULL, MKDEV(major, 0), NULL,"buttons"); /* /dev/bottons */
    gpgcon = (volatile unsigned long*)ioremap(0x56000060, 16);
    gpgdat = gpgcon + 1;
    return 0;
}

static void second_drv_exit(void)
{
    unregister_chrdev(major, "second_drv");
    device_unregister(seconddrv_class_dev);
    class_destroy(seconddrv_class);
}

module_init(second_drv_init);
module_exit(second_drv_exit);

MODULE_LICENSE("GPL");
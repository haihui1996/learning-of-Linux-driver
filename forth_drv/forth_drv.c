#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>




static struct class *forthdrv_class;
static struct class_device *forthdrv_class_device;


// volatile unsigned long *gpfcon


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־���жϷ����������һ��forth_drv_read�������� */

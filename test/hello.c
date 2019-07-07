#include <linux/module.h>
#include <linux/kernel.h>
// #include <linux/>


static void hello_init(void)
{
    printk("hello world!\n");
}

static void hello_exit(void)
{
    printk("goodbye,kill world");
}



module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");

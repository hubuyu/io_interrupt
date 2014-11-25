#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/atomic.h>
#include <asm/unistd.h>

static int Button_Major = 0;
static struct class *Button_driver_class;

static unsigned int key_valu;

static DECLARE_WAIT_QUEUE_HEAD(button_watg);
static volatile char ev_press = 0;

static unsigned char pin_val[5] = {9,1,2,3,4};

static irqreturn_t button_handler (int irq, void *dev_id)
{
    unsigned char *keydes = (unsigned char *) dev_id;
    key_valu = *keydes;

    printk("\n the irq number is: %d\n",irq);
    ev_press = 1;
    wake_up_interruptible(&button_watg);
        return IRQ_HANDLED;
}

static int s3c24xx_button_open(struct inode *inode, struct file *file)
{
        printk("s3c24xx_button_open\n");
        request_irq(gpio_to_irq(25), button_handler, 0x00000002, "S0", &pin_val[0]);
    return 0;
}

static int s3c24xx_button_close(struct inode *inode, struct file *file)
{
        printk("s3c24xx_button_close\n");
        free_irq(gpio_to_irq(25), &pin_val[0]);
    return 0;
}

static int s3c24xx_button_read(struct file *file, char __user *buff,
                                size_t count, loff_t *offp)
{	int err ;
    wait_event_interruptible(button_watg, ev_press);

    err = copy_to_user(buff, &key_valu, 1);
    ev_press = 0;

    return err;
}

static struct file_operations button_fops = {
    .owner   =   THIS_MODULE,
    .open    =   s3c24xx_button_open,
    .read    =   s3c24xx_button_read,
    .release =   s3c24xx_button_close,
};

static int __init button_init(void)
{
    Button_Major = register_chrdev(Button_Major, "button_driver", &button_fops);

        Button_driver_class = class_create(THIS_MODULE, "button_driver");
        device_create(Button_driver_class, NULL, MKDEV(Button_Major, 0),
                                            NULL, "button_driver");

        printk("driver_init\n");
    return 0;
}

static void __exit button_exit(void)
{
    unregister_chrdev(Button_Major, "button_driver");

        device_destroy(Button_driver_class, MKDEV(Button_Major, 0) );
        class_destroy(Button_driver_class);

        printk("driver_release\n");
}

module_init(button_init);
module_exit(button_exit);

MODULE_AUTHOR("yuanlai");
MODULE_DESCRIPTION("helper2416 button Driver");
MODULE_LICENSE("GPL");

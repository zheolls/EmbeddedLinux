#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <asm/hardware.h>
#include <asm/sizes.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/ide.h>
#include <asm/arch/regs-mem.h>
#include <asm/arch/irqs.h>
#include <asm-arm/arch-s3c2410/regs-gpio.h>

#define DEVICE_NAME "s3c2410_led"

#define DEVICE_MAJOR 251
#define DEVICE_MINOR 0
struct cdev *mycdev;
struct class_simple *myclass;
dev_t devno;

#define LED_TUBE_PHY_START 0x08000100
#define LED_DIG_PHY_START 0x08000110
#define LED_TUBE_IOCTRL 0x11
#define LED_DIG_IOCTRL 0x12
#define LED_BASE 0x08000100
static int s3c2410_led_tube_base;
static int s3c2410_led_dig_base;
static int ledMajor = 0;
#define LED_MINOR 1
MODULE_LICENSE("Dual BSD/GPL");

static int s3c2410_led_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned int arg)
{
    printk("DOT buffer is %x\n", arg >> 8);
    printk("DOT buffer is %x\n", arg);
    switch (cmd)
    {
    case LED_DIG_IOCTRL:
        *(volatile unsigned short *)(s3c2410_led_dig_base + 2) = arg;
        *(volatile unsigned short *)(s3c2410_led_dig_base) = arg >> 8;
        break;
    default:
        return printk("your command is not exist");
    }

    return 0;
}

static ssize_t s3c2410_led_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    int i;
    unsigned char mdata[16];
    if (copy_from_user(mdata, buf, 10))
    {
        return -EFAULT;
    }
    for (i = 0; i < 8; i++)
    {

        *((volatile unsigned char *)(s3c2410_led_tube_base + i * 2)) = mdata[i];
    }
    return 0;
}

static int s3c2410_led_open(struct inode *inode, struct file *filp)
{

    printk("led device open sucess!\n");
    return 0;
}

static int s3c2410_led_release(struct inode *inode, struct file *filp)
{

    printk("led device release\n");
    return 0;
}

static struct file_operations s3c2410_led_fops = {
    owner : THIS_MODULE,
    open : s3c2410_led_open,
    ioctl : s3c2410_led_ioctl,
    write : s3c2410_led_write,
    release : s3c2410_led_release
};

int __init s3c2410_led_init(void)
{
    int ret;
    s3c2410_led_tube_base = (unsigned int)ioremap(LED_TUBE_PHY_START, 0x016);
    s3c2410_led_dig_base = (unsigned int)ioremap(LED_DIG_PHY_START, 0x04); //
    writel(readl(S3C2410_BWSCON) & (~(S3C2410_BWSCON_ST1 | S3C2410_BWSCON_WS1 |
                                      S3C2410_BWSCON_DW1_8)) |
               (S3C2410_BWSCON_ST1 | S3C2410_BWSCON_WS1 | S3C2410_BWSCON_DW1_16),
           S3C2410_BWSCON);
    writel((S3C2410_BANKCON_Tacs4 | S3C2410_BANKCON_Tcos4 | S3C2410_BANKCON_Tacc14 |
            S3C2410_BANKCON_Tcoh4 | S3C2410_BANKCON_Tcah4 | S3C2410_BANKCON_Tacp6 | S3C2410_BANKCON_PMC8),
           S3C2410_BANKCON1);
    writel(readl(S3C2410_GPACON) | (0x1 << 12), S3C2410_GPACON);

    int err;

    devno = MKDEV(DEVICE_MAJOR, DEVICE_MINOR);
    mycdev = cdev_alloc();
    cdev_init(mycdev, &s3c2410_led_fops);
    err = cdev_add(mycdev, devno, 1);
    if (err != 0)
        printk("s3c2410 motor device register failed!\n");

    myclass = class_create(THIS_MODULE, "s3c2410-led");
    if (IS_ERR(myclass))
    {
        printk("Err: failed in creating class.\n");
        return -1;
    }

    class_device_create(myclass, NULL, MKDEV(DEVICE_MAJOR, DEVICE_MINOR), NULL, DEVICE_NAME "%d", DEVICE_MINOR);
    printk(DEVICE_NAME "\tdevice initialized\n");
    return 0;
}

void __exit s3c2410_led_exit(void)
{
    cdev_del(mycdev);
    class_device_destroy(myclass, devno);
    class_destroy(myclass);
}

module_init(s3c2410_led_init);
module_exit(s3c2410_led_exit);

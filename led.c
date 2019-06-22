//#ifdef MODULE
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/poll.h>    /* COPY_TO_USER */
#include <asm-arm/system.h>     /* cli(), *_flags */
#include <linux/cdev.h>

#include <asm-arm/arch/regs-gpio.h>
#include <asm-arm/hardware.h>
#include <asm-arm/io.h>	/*writel,readl*/

#define DEVICE_NAME	"UP-LED"
#define LEDRAW_MINOR	0
#define GPCCON (*(volatile unsigned int *)S3C2410_GPCCON)
#define GPCDAT (*(volatile unsigned int *)S3C2410_GPCDAT)

struct cdev *mycdev;
dev_t devno;

static void gpio_init()
{
	GPCCON=(GPCCON|0x5400)&0xffff57ff;
}

static int led_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd){
		case 1:
			if(arg==1)
				GPCDAT=GPCDAT&0xffffffdf;
			if(arg==0)
				GPCDAT=GPCDAT|0x20;
			break;
		case 2:
         if(arg==1)
				GPCDAT=GPCDAT&0xffffffbf;
         if(arg==0)
				GPCDAT=GPCDAT|0x40;
         break;
      case 3:
         if(arg==1)
				GPCDAT=GPCDAT&0xffffff7f;
         if(arg==0)
				GPCDAT=GPCDAT|0x80;
         break;
		default:
			printk("error cmd number\n");break;
	}
	return 0;
}           


/*************************************************************************************/
static struct file_operations led_fops = {
	owner:	THIS_MODULE,
	ioctl:	led_ioctl,
};

/*************************************************************************************
Leds initialized and register device function
*************************************************************************************/
static int __init led_init(void)
{
	int result,err;

	result=alloc_chrdev_region(&devno,LEDRAW_MINOR,1,DEVICE_NAME);

        if(result < 0) printk(KERN_ERR "can't get device number \n");
        else
		printk("get device number\n");

	gpio_init();
	
        mycdev = cdev_alloc();
 
        cdev_init(mycdev,&led_fops);
        err=cdev_add(mycdev,devno,1);
        if (err < 0)
                printk(KERN_ERR "can't add led device");

	return 0;
}

/*************************************************************************************/
#ifdef MODULE
void __exit  led_exit(void)
{
	cdev_del(mycdev);
	unregister_chrdev_region(devno,1);
}
module_exit(led_exit);
#endif

/*************************************************************************************/
module_init(led_init);

MODULE_LICENSE("Dual BSD/GPL");
//#endif		// MODULE


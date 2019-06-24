/*
 * s3c2410-dcmotor.c
 *
 * DC motor driver for UP-NETARM2410-S DA
 *
 * Author: Wang bin <wbinbuaa@163.com>
 * Date  : $Date: 2005/7/26 15:50:00 $ 
 * Description : UP-NETARM2410-S
 * $Revision: 0.1.0 $
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * History:
 *
 * 
 */

//#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/kdev_t.h> MAJOR MKDEV
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mm.h>
//#include <linux/poll.h>
//#include <linux/spinlock.h>
//#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
				/* copy_from_user */
//#include <asm/sizes.h>
#include <asm-arm/arch-s3c2410/regs-gpio.h>
#include <asm-arm/plat-s3c/regs-timer.h>
#include <asm-arm/arch/map.h>
//#include <asm/hardware.h>
#include <linux/device.h>
#include <linux/cdev.h>
#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

/* debug macros */
//#undef DEBUG
#define DEBUG
#ifdef DEBUG
#define DPRINTK(x...) {printk(__FUNCTION__"(%d): ",__LINE__);printk(##x);}
#else
#define DPRINTK(x...) (void)(0)
#endif

#define DEVICE_NAME				"s3c2410-dc-motor"
#define DCMRAW_MINOR	1		//direct current motor

#define DCM_IOCTRL_SETPWM 			(0x10)
#define DCM_TCNTB0					(163840)
#define DCM_TCFG0					(2)
#define DEVICE_MAJOR 252
#define DEVICE_MINOR  0

struct cdev *mycdev;
struct class *myclass;
dev_t devno;

#define tout01_enable() \
	({      writel((readl(S3C2410_GPBCON)&(~ 0xf)),S3C2410_GPBCON);		\
		writel((readl(S3C2410_GPBCON)| 0xa/*0x6*/),S3C2410_GPBCON);  })   // by sprife  /*tout00  tout01 off ,tout02,tout03 open;*/

#define tout01_disable() \
	({ 	writel(readl(S3C2410_GPBCON) &(~ 0xf),S3C2410_GPBCON);		\
		writel(readl(S3C2410_GPBCON) | 0x5,S3C2410_GPBCON); 		\
		writel(readl(S3C2410_GPBUP) &~0x3,S3C2410_GPBUP); 	})
	
/* deafault divider value=1/16		*/
/* deafault prescaler = 1/2 ;			*/
/* Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value} */
#define dcm_stop_timer()	({ writel(readl(S3C2410_TCON) &( ~0x1),S3C2410_TCON); })    /*  12bit ,16bit=0*/

#define DPRINTREG() \
	({  													\
	printk("S3C2410_GPBCON %x\n", S3C2410_GPBCON);					\
	printk("S3C2410_TCFG0 %x\n", S3C2410_TCFG0); 						\
	printk("S3C2410_TCFG1 %x\n", S3C2410_TCFG1); 						\
	printk("S3C2410_TCNTB0 %x\n", S3C2410_TCNTB(0)); 						\ 						\
	printk("S3C2410_TCON %x\n", S3C2410_TCON); 							\
	printk("GPBCON %x\n", S3C2410_GPBCON);					\
	printk("\n");										\
})


static void dcm_start_timer()
{
         writel(readl(S3C2410_TCFG0) & ~(0x00ff0000),S3C2410_TCFG0); 
          writel(readl(S3C2410_TCFG0) | (DCM_TCFG0),S3C2410_TCFG0); 
            writel(readl(S3C2410_TCFG1) & ~(0xf),S3C2410_TCFG1); 
 //TCFG1 |= 0x3300;
           writel(DCM_TCNTB0,S3C2410_TCNTB(0)); 
           // writel(DCM_TCNTB0,S3C2410_TCNTB(3));
           writel(DCM_TCNTB0/2,S3C2410_TCMPB(0)); 
          // writel(DCM_TCNTB0/2,S3C2410_TCMPB(3)) ; 
           writel(readl(S3C2410_TCON) &~(0xf),S3C2410_TCON);
            writel(readl(S3C2410_TCON) |(0x2),S3C2410_TCON);
            writel(readl(S3C2410_TCON) &~(0xf),S3C2410_TCON); 
           writel(readl(S3C2410_TCON) |(0x19),S3C2410_TCON); 
}
 


static int s3c2410_dcm_open(struct inode *inode, struct file *filp)
{
//	MOD_INC_USE_COUNT;
	printk( "S3c2410 DC Motor device open now!\n");
	tout01_enable();
	dcm_start_timer();
	return 0;
}

static int s3c2410_dcm_release(struct inode *inode, struct file *filp)
{
//	MOD_DEC_USE_COUNT;
	printk( "S3c2410 DC Motor device release!\n");
	tout01_disable();
	dcm_stop_timer();
	return 0;
}

static int dcm_setpwm(int v)
{// by sprife
	return (writel((DCM_TCNTB0/2 + v),S3C2410_TCMPB(0/*2*/)));
}
static int s3c2410_dcm_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd){
		
	/*********write da 0 with (*arg) ************/		
	case DCM_IOCTRL_SETPWM:
		return dcm_setpwm((int)arg);	
	}
	return 0;	
}

static struct file_operations s3c2410_dcm_fops = {
	owner:	THIS_MODULE,
	open:	s3c2410_dcm_open,
	ioctl:	s3c2410_dcm_ioctl,
	release:	s3c2410_dcm_release,
};


#ifdef CONFIG_DEVFS_FS
static devfs_handle_t devfs_dcm_dir, devfs_dcm0;
#endif
int __init s3c2410_dcm_init(void)
{
	int err;
        unsigned ret;
	devno = MKDEV(DEVICE_MAJOR, DEVICE_MINOR);
        mycdev = cdev_alloc();
        cdev_init(mycdev, &s3c2410_dcm_fops);
        err = cdev_add(mycdev, devno, 1);
        if (err != 0)
           printk("s3c2410 motor device register failed!\n");
 
       myclass = class_create(THIS_MODULE, "s3c2410-dc-motor");
       if(IS_ERR(myclass)) {
       printk("Err: failed in creating class.\n");
          return -1;
}
 
        class_device_create(myclass,NULL, MKDEV(DEVICE_MAJOR,DEVICE_MINOR), NULL, DEVICE_NAME"%d",DEVICE_MINOR);
        printk (DEVICE_NAME"\tdevice initialized\n");
        return 0;
        
}

module_init(s3c2410_dcm_init);

//#ifdef MODULE
void __exit s3c2410_dcm_exit(void)
{
        #if 0
        unsigned long base;
	unsigned char tmp;

	base = ioremap(0x56000010, 0x400);
	tmp = readb(base);
 //	printk("tmp is 0x%02x\n", tmp);
	tmp &= 0xfe;
	writeb(tmp, base);
	tmp = readb(base);
//	printk("tmp is 0x%02x\n", tmp);
	iounmap(base);
        #endif

//	iounmap(base_addr);
	cdev_del(mycdev);
       class_device_destroy(myclass,devno);
       class_destroy(myclass);

}

module_exit(s3c2410_dcm_exit);
//#endif

MODULE_LICENSE("GPL"); 


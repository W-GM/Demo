#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: watchdogctl.c
作者	  	: 左忠凯
版本	   	: V1.0
描述	   	: 采用pinctrl和gpio子系统驱动WATCHDOGCTL。
其他	   	: 无
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/7/13 左忠凯创建
***************************************************************/
#define WATCHDOGCTL_CNT			1		  	/* 设备号个数 */
#define WATCHDOGCTL_NAME		"watchdogctl"	/* 名字 */
#define LOW 				0			/* 关 */
#define HIGH 				1			/* 开 */

/* watchdogctl设备结构体 */
struct watchdogctl_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
	int watchdogctl_gpio;			/* watchdogctl所使用的GPIO编号		*/
};

struct watchdogctl_dev watchdogctl;	/* watchdogctl设备 */

struct timer_list watchdogctl_timer;

void timer_function(unsigned long data)
{
	//将灯的状态取反
	int watchdogctl_status = gpio_get_value(watchdogctl.watchdogctl_gpio);   //获取gpio电平的状态
	
	watchdogctl_status = watchdogctl_status ?0:1;
	printk(KERN_ERR "status >> %d\n", watchdogctl_status);
	gpio_set_value(watchdogctl.watchdogctl_gpio,watchdogctl_status);   //设置gpio的输出的电平

	mod_timer(&watchdogctl_timer,jiffies+3000);  //HZ（宏）等价于CONFIG_HZ  
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int watchdogctl_open(struct inode *inode, struct file *filp)
{
#if 1
	printk(KERN_ERR "--------------------\n");
	//2.添加定时器
	watchdogctl_timer.expires = jiffies + 3000; //HZ(宏)  <==>  CONFIG_HZ
	watchdogctl_timer.function = timer_function;
	watchdogctl_timer.data = 0;
	init_timer(&watchdogctl_timer);

	add_timer(&watchdogctl_timer);
#endif

	//filp->private_data = &watchdogctl; /* 设置私有数据 */
	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t watchdogctl_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t watchdogctl_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
#if 0
	int retvalue;
	unsigned char databuf[1];
	unsigned char watchdogctlstat;
	struct watchdogctl_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	watchdogctlstat = databuf[0];		/* 获取状态值 */

	if(watchdogctlstat == HIGH) {	
		gpio_set_value(dev->watchdogctl_gpio, 0);	/* 打开WATCHDOGCTL */
	} else if(watchdogctlstat == LOW) {
		gpio_set_value(dev->watchdogctl_gpio, 1);	/* 关闭WATCHDOGCTL */
	}
#endif	
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int watchdogctl_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations watchdogctl_fops = {
	.owner = THIS_MODULE,
	.open = watchdogctl_open,
	.read = watchdogctl_read,
	.write = watchdogctl_write,
	.release = 	watchdogctl_release,
};

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init watchdogctl_init(void)
{
	int ret = 0;

	printk(KERN_ERR "------init--------------\n");

	/* 设置WATCHDOGCTL所使用的GPIO */
	/* 1、获取设备节点：watchdogctl */
	watchdogctl.nd = of_find_node_by_path("/watchdogctl");
	if(watchdogctl.nd == NULL) {
		printk("watchdogctl node not find!\r\n");
		return -EINVAL;
	} else {
		printk("watchdogctl node find!\r\n");
	}

	/* 2、 获取设备树中的gpio属性，得到WATCHDOGCTL所使用的WATCHDOGCTL编号 */
	watchdogctl.watchdogctl_gpio = of_get_named_gpio(watchdogctl.nd, "watchdogctl-gpio", 0);
	if(watchdogctl.watchdogctl_gpio < 0) {
		printk("can't get watchdogctl-gpio");
		return -EINVAL;
	}
	printk("watchdogctl-gpio num = %d\r\n", watchdogctl.watchdogctl_gpio);

	/* 3、设置GPIO1_IO03为输出，并且输出高电平，默认关闭WATCHDOGCTL */
	ret = gpio_direction_output(watchdogctl.watchdogctl_gpio, 1);
	if(ret < 0) {
		printk("can't set gpio!\r\n");
	}

#if 1
	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (watchdogctl.major) {		/*  定义了设备号 */
		watchdogctl.devid = MKDEV(watchdogctl.major, 0);
		register_chrdev_region(watchdogctl.devid, WATCHDOGCTL_CNT, WATCHDOGCTL_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&watchdogctl.devid, 0, WATCHDOGCTL_CNT, WATCHDOGCTL_NAME);	/* 申请设备号 */
		watchdogctl.major = MAJOR(watchdogctl.devid);	/* 获取分配号的主设备号 */
		watchdogctl.minor = MINOR(watchdogctl.devid);	/* 获取分配号的次设备号 */
	}
	printk("watchdogctl major=%d,minor=%d\r\n",watchdogctl.major, watchdogctl.minor);	
	
	/* 2、初始化cdev */
	watchdogctl.cdev.owner = THIS_MODULE;
	cdev_init(&watchdogctl.cdev, &watchdogctl_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&watchdogctl.cdev, watchdogctl.devid, WATCHDOGCTL_CNT);

	/* 4、创建类 */
	watchdogctl.class = class_create(THIS_MODULE, WATCHDOGCTL_NAME);
	if (IS_ERR(watchdogctl.class)) {
		return PTR_ERR(watchdogctl.class);
	}

	/* 5、创建设备 */
	watchdogctl.device = device_create(watchdogctl.class, NULL, watchdogctl.devid, NULL, WATCHDOGCTL_NAME);
	if (IS_ERR(watchdogctl.device)) {
		return PTR_ERR(watchdogctl.device);
	}

	printk(KERN_ERR "------123--------------\n");

#endif
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit watchdogctl_exit(void)
{
	printk(KERN_ERR "------exit--------------\n");
	del_timer(&watchdogctl_timer);

	/* 注销字符设备驱动 */
	cdev_del(&watchdogctl.cdev);/*  删除cdev */
	unregister_chrdev_region(watchdogctl.devid, WATCHDOGCTL_CNT); /* 注销设备号 */

	device_destroy(watchdogctl.class, watchdogctl.devid);
	class_destroy(watchdogctl.class);
}

module_init(watchdogctl_init);
module_exit(watchdogctl_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h> /* 注册字符设备驱动 */
#include <linux/gpio.h> /* gpio子系统 */
#include <linux/device.h> /* 自动创建设备节点 */
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>


#include <linux/of_address.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ide.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <asm/mach/map.h>


#define CNAME "watchdogctl"

struct watchdog
{
    struct device_node *nd;
    int gpio;
}watchdog;

struct watchdogctl
{
    int major; /* 主设备号 */
    struct class *cls;
    struct device *dev;
    char kbuf[2];

}watchdogctl;


/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int watchdogctl_open(struct inode *inode, struct file *filp)
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
static ssize_t watchdogctl_write(struct file *filp, const char __user *buf, size_t size, loff_t *offt)
{
    if(size > sizeof(watchdogctl.kbuf))
    {
        size = sizeof(watchdogctl.kbuf);
    }

    /* 从用户空间拷贝数据 */
    if(copy_from_user(watchdogctl.kbuf, buf, size))
    {
        printk("copy from user error\n");
        return -EINVAL;
    }

    /* 设置gpio的输出的电平 */
    gpio_set_value(watchdog.gpio, watchdogctl.kbuf[0]);

	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int watchdogctl_close(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations fops = {
	//.owner = THIS_MODULE,
	.open = watchdogctl_open,
	//.read = watchdogctl_read,
	.write = watchdogctl_write,
	.release = 	watchdogctl_close,
};

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init watchdogctl_init(void)
{
    watchdogctl.major = 0;

    /* 注册字符设备驱动 */
    watchdogctl.major = register_chrdev(watchdogctl.major, CNAME, &fops);
    if(watchdogctl.major < 0)
    {
        printk("register char device error");
        return watchdogctl.major;
    }

    /* 设置WATCHDOG所使用的GPIO */
    /* 1.获取设备节点 */
    watchdog.nd = of_find_node_by_path("/watchdog");
    if (watchdog.nd == NULL) {
        printk("watchdog node not find!\r\n");
        return -EINVAL;
    } else {
        printk("watchdog node find!\r\n");
    }

    /* 2.获取设备树中的gpio属性,得到WATCHDOG所使用的编号 */
    watchdog.gpio = of_get_named_gpio(watchdog.nd, "watchdog-gpio", 0);
    if (watchdog.gpio < 0) 
    {
        printk("can't get watchdog-gpio");
        return -EINVAL;
    }

    printk("watchdog-gpio num = %d\r\n", watchdog.gpio);

    /* 3.申请要使用的gpio,并初始化 */
    if(gpio_request(watchdog.gpio, NULL))
    {
        printk("gpio requst %d error", watchdog.gpio);
        return -1;
    }

    /* 4.设置GPIO5_IO8为输出,并且输出高电平,默认关闭WATCHDOG */
    if(gpio_direction_output(watchdog.gpio, 1) < 0)
    {
        printk("can't set gpio!\r\n");
        return -1;
    }

    /* 自动创建设备节点 */
    /* 1.创建目录 */
    watchdogctl.cls = class_create(THIS_MODULE, CNAME);
    if(IS_ERR(watchdogctl.cls))
    {
        printk("class create error\n");
        return PTR_ERR(watchdogctl.cls);
    }

    /* 2.创建文件 */
    watchdogctl.dev = 
        device_create(watchdogctl.cls, NULL, MKDEV(watchdogctl.major, 0), NULL, CNAME);
    if(IS_ERR(watchdogctl.dev))
    {
        printk("device create error\n");
        return PTR_ERR(watchdogctl.dev);
    }

    return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit watchdogctl_exit(void)
{
    /* 删除文件 */
    device_destroy(watchdogctl.cls, MKDEV(watchdogctl.major, 0));
    /* 删除目录 */
    class_destroy(watchdogctl.cls);

    /* 释放gpio号 */
    gpio_free(watchdog.gpio);

    /* 注销字符设备驱动 */
    unregister_chrdev(watchdogctl.major, CNAME);
}

module_init(watchdogctl_init);
module_exit(watchdogctl_exit);
MODULE_LICENSE("GPL");
//MODULE_AUTHOR("zuozhongkai");
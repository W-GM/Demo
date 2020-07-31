#include <linux/timer.h>
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

int watchdogctl_status = 1;

struct timer_list watchdogctl_timer;
void timer_function(unsigned long data)
{
    watchdogctl_status = watchdogctl_status ? 0 : 1;
    gpio_set_value(watchdogctl.watchdogctl_gpio, watchdogctl_status);      // 设置gpio的输出的电平

    mod_timer(&watchdogctl_timer, jiffies + 1000);        // 休眠10秒
}

/*
 * @description	: 驱动入口函数
 * @param               : 无
 * @return              : 无
 */
static int __init watchdogctl_init(void)
{
    int ret = 0;

    /* 设置WATCHDOGCTL所使用的GPIO */
    /* 1、获取设备节点：watchdogctl */
    watchdogctl.nd = of_find_node_by_path("/watchdogctl");

    if (watchdogctl.nd == NULL) {
        printk("watchdogctl node not find!\r\n");
        return -EINVAL;
    } else {
        printk("watchdogctl node find!\r\n");
    }

    /* 2、 获取设备树中的gpio属性，得到WATCHDOGCTL所使用的WATCHDOGCTL编号 */
    watchdogctl.watchdogctl_gpio = of_get_named_gpio(watchdogctl.nd,
                                                     "watchdogctl-gpio",
                                                     0);

    if (watchdogctl.watchdogctl_gpio < 0) {
        printk("can't get watchdogctl-gpio");
        return -EINVAL;
    }
    printk("watchdogctl-gpio num = %d\r\n", watchdogctl.watchdogctl_gpio);

     // 1.申请要使用的gpio，初始化
    ret = gpio_request(watchdogctl.watchdogctl_gpio, NULL);

    if (ret) {
        printk("gpio requst %d error", watchdogctl.watchdogctl_gpio);
        return ret;
    }

    /* 3、设置GPIO5_IO8为输出，并且输出高电平，默认关闭WATCHDOGCTL */
    ret = gpio_direction_output(watchdogctl.watchdogctl_gpio, 1);

    if (ret < 0) {
        printk("can't set gpio!\r\n");
    }

    // 2.添加定时器
    watchdogctl_timer.expires = jiffies + 1000; // 休眠10秒
    watchdogctl_timer.function = timer_function;
    watchdogctl_timer.data = 0;
    init_timer(&watchdogctl_timer);

    add_timer(&watchdogctl_timer);

}

static void __exit watchdogctl_exit(void)
{
    del_timer(&watchdogctl_timer);

    gpio_free(watchdogctl.watchdogctl_gpio);

}

module_init(watchdogctl_init);
module_exit(watchdogctl_exit);
MODULE_LICENSE("GPL");

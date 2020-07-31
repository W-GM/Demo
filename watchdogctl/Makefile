KERNELDIR:=/home/wgm/SVN-CQ-PROJECT/长庆油田井场RTU项目/项目资料/硬件资料/米尔核心版资料/米尔/04-Source/MYiR-iMX-Linux
#KERNELDIR:=/home/wgm/MYiR-iMX-Linux/
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
obj-m:=watchdogctl.o
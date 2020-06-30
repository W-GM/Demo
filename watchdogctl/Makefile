KERNELDIR:=/home/wgm/文档/硬件资料/米尔/04-Source/MYiR-iMX-Linux
#KERNELDIR:=/home/wgm/MYiR-iMX-Linux/
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
obj-m:=watchdogctl.o
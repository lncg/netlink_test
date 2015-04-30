MODULE_NAME := k
$(MODULE_NAME)-objs := kernel.o
obj-m := $(MODULE_NAME).o

ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	gcc -g -o user user.c -lpthread
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	-@rmmod $(MODULE_NAME).ko
	insmod $(MODULE_NAME).ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf user

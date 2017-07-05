ifeq ($(KERNELRELEASE),)

BUILD = arm-linux-gcc
KERNELDIR ?= /home/linux/lzh/bsp/kernel/my_kernel/linux-3.14
# KERNELDIR := /lib/modules/$(shell uname -r)/build/
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

app:
	$(BUILD) -o app1.elf app1.c
	$(BUILD) -o app2.elf app2.c

cp:
	cp *.ko ~/lzh/rootfs/rootfs_fst/root/
	cp *.elf ~/lzh/rootfs/rootfs_fst/root/

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules clean
	rm -rf *.elf
	# $(MAKE) -C $(KERNELDIR) M=$(PWD) moduleses_install clean

.PHONY: modules modules_install clean

else
	obj-m := key.o

endif

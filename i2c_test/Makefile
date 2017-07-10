ifeq ($(KERNELRELEASE),)

BUILD = arm-linux-gcc
KERNELDIR ?= /home/linux/lzh/bsp/kernel/my_kernel/linux-3.14
#KERNELDIR ?= /lib/modules/3.13.0-32-generic/build/
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

app:
	$(BUILD) -o app.elf app.c

cp:
	cp *.ko ~/lzh/rootfs/rootfs_fst/root/
	cp app.elf ~/lzh/rootfs/rootfs_fst/root/

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules clean
	rm -rf *.elf
	# $(MAKE) -C $(KERNELDIR) M=$(PWD) moduleses_install clean
	# rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules_install clean

else
	obj-m := my_i2c.o

endif

#obj-$(CONFIG_WEBAD) += webad.o
obj-m := webad_kernel.o
webad_kernel-objs := webad.o
KERNELDIR :=/lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	-rm -f config.h

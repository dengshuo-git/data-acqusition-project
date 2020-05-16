EXTRA_CFLAGS  = -Wno-error -I../../include 
obj-m	:= driver.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 
	cp driver.ko ../../bin

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c *.mod.o .tmp_versions *.order *.symvers


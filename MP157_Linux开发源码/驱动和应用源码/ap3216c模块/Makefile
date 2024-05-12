KERNELDIR := /home/book/alientek_linux/my_linux/linux-5.4.31

CURRENT_PATH := $(shell pwd)

obj-m := ap3216c.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
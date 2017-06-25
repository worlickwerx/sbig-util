KERNEL_VERSION ?= $(shell uname -r)
KERNEL_PATH ?= /lib/modules/$(KERNEL_VERSION)/build

obj-m += sbiglpt.o

sbiglpt-y = \
	ksbiglptmain.o \
	ksbiglpt.o

all: modules

modules clean:
	make -C $(KERNEL_PATH) M=$(shell pwd) $@

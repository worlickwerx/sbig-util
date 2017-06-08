KERNEL_VERSION ?= $(shell uname -r)

obj-m += sbiglpt.o

sbiglpt-y = \
	ksbiglptmain.o \
	ksbiglpt.o

all: modules

modules clean:
	make -C /lib/modules/$(KERNEL_VERSION)/build M=$(shell pwd) $@

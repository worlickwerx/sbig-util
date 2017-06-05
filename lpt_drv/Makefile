KERNEL_VERSION ?= $(shell uname -r)

ccflags-y := -D_CHATTY_=1

obj-m += sbiglptmod.o

sbiglptmod-y = \
	ksbiglptmain.o \
	ksbiglptd0.o \
	ksbiglptd1.o \
       	ksbiglptd2.o \
	ksbiglpthw.o \
	ksbiglpt.o

all: modules

modules clean:
	make -C /lib/modules/$(KERNEL_VERSION)/build M=$(shell pwd) $@

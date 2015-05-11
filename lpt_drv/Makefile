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
	make -C /lib/modules/`uname -r`/build M=$(shell pwd) $@

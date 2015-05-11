##########################################################################
# Makefile   : Kernel LPT modules 
# Author     : Jan Soldan, 251 65 Ondrejov 236, Czech Republic
# Contributor: Matt Longmire, SBIG
# Version    : 2.02, 17 March 2005
##########################################################################
SBIG_MOD            = sbigmod
SBIG_LPT_MOD_NAME   = sbiglptmod
##########################################################################
SBIG_LPT_MOD_SRC    = ksbiglptmain.c ksbiglptd0.c   ksbiglptd1.c \
                      ksbiglptd2.c   ksbiglpthw.c   ksbiglpt.c
SBIG_LPT_MOD_OBJ    = ksbiglptmain.o ksbiglptd0.o   ksbiglptd1.o \
                      ksbiglptd2.o   ksbiglpthw.o   ksbiglpt.o

##########################################################################
SRC_SBIG_DIR        = module/sbig/
REL_DIR             = release/
REL_MOD_DIR         = ${REL_DIR}module/
REL_SBIG_DIR        = ${REL_MOD_DIR}sbig/
REL_LIB_DIR         = ${REL_DIR}lib/
##########################################################################
INC     = -I${SRC_SBIG_DIR}
LIB     = -L${REL_SBIG_DIR}

MOD_INC = ${INC} -I/lib/modules/`uname -r`/build/include 
LIB_INC = ${INC}
MOD_FLG = -D__KERNEL__ ${MOD_INC} -DMODULE -Wall -O2 -fno-strict-aliasing
APP_FLG = -O2 ${LIB_INC} -Wall 
LIB_FLG = -O2 ${LIB_INC} -fPIC

ifdef CONFIG_SMP
MOD_FLG += -D__SMP__ -DSMP
endif

ifdef CONFIG_MODVERSIONS
MOD_FLG += -DMODVERSIONS
include ${MOD_INC}/linux/modversions.h
endif

MOD_CC = gcc
LIB_CC = g++
APP_CC = g++
##########################################################################
# Make all using gcc 3.XX compiler
##########################################################################
all:	${SBIG_MOD}
	#------------------------------------------------- 
	# The End of Makefile
##########################################################################
# SBIG LPT modules
##########################################################################
${SBIG_MOD}: ${SBIG_LPT_MOD_OBJ}
	# generate SBIG LPT kernel modules
	cd ${REL_DIR}; ld -r ${SBIG_LPT_MOD_OBJ}                \
	-o ../${REL_SBIG_DIR}${SBIG_LPT_MOD_NAME}.o
#	cd ${REL_DIR}; ld -r ${SBIG_USB_MOD_OBJ}                \
#	-o ../${REL_SBIG_DIR}${SBIG_USB_MOD_NAME}.o
	#---------------------------------
ksbiglptmain.o: ${SRC_SBIG_DIR}ksbiglptmain.c
		${MOD_CC} ${MOD_FLG}                            \
		-o ${REL_DIR}ksbiglptmain.o                     \
		-c ${SRC_SBIG_DIR}ksbiglptmain.c

ksbiglptd0.o:   ${SRC_SBIG_DIR}ksbiglptd0.c
		${MOD_CC} ${MOD_FLG}                            \
		-o ${REL_DIR}ksbiglptd0.o                       \
		-c ${SRC_SBIG_DIR}ksbiglptd0.c

ksbiglptd1.o:   ${SRC_SBIG_DIR}ksbiglptd1.c
		${MOD_CC} ${MOD_FLG}                            \
		-o ${REL_DIR}ksbiglptd1.o                       \
		-c ${SRC_SBIG_DIR}ksbiglptd1.c

ksbiglptd2.o:   ${SRC_SBIG_DIR}ksbiglptd2.c
		${MOD_CC} ${MOD_FLG}                            \
		-o ${REL_DIR}ksbiglptd2.o                       \
		-c ${SRC_SBIG_DIR}ksbiglptd2.c

ksbiglpthw.o:   ${SRC_SBIG_DIR}ksbiglpthw.c
		${MOD_CC} ${MOD_FLG}                            \
		-o ${REL_DIR}ksbiglpthw.o                       \
		-c ${SRC_SBIG_DIR}ksbiglpthw.c

ksbiglpt.o:     ${SRC_SBIG_DIR}ksbiglpt.c
		${MOD_CC} ${MOD_FLG}                            \
		-o ${REL_DIR}ksbiglpt.o                         \
		-c ${SRC_SBIG_DIR}ksbiglpt.c
##########################################################################








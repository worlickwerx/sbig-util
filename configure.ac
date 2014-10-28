##
# Prologue
##
AC_INIT(m4_esyscmd([awk '/Name:/ {printf "%s",$2; exit}' META]),
        m4_esyscmd([awk '/Version:/ {printf "%s",$2; exit}' META]))
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([NEWS])
AC_CANONICAL_SYSTEM
X_AC_META
X_AC_EXPAND_INSTALL_DIRS

##
# Automake support
##
AM_INIT_AUTOMAKE([subdir-objects])
AM_CONFIG_HEADER([config/config.h])
AM_MAINTAINER_MODE

AC_DEFINE([_GNU_SOURCE], 1,
          [Define _GNU_SOURCE so that we get all necessary prototypes])

##
# Checks for programs
##
AC_PROG_CC
AM_PROG_CC_C_O
if test "$GCC" = yes; then
  GCCWARN="-Wall -Werror"
  AC_SUBST([GCCWARN])
fi
AC_PROG_RANLIB
AC_PROG_LIBTOOL

##
# Checks for header files.
##
AC_HEADER_STDC
AC_CHECK_HEADERS( \
  usb.h
)
AS_VAR_IF([ac_cv_header_usb_h],[yes],,[
  AC_MSG_ERROR([libusb header usb.h not found])
])

##
# Checks for typedefs, structures, and compiler characteristics
## 
AC_C_BIGENDIAN
AC_C_CONST

##
# Set $sdk_libdir according to architecture.
##
AC_MSG_CHECKING([for usable sdk libdir])
case "$host_cpu" in
    i386|i686)	sdk_libdir="sdk/libs/i386" ;;
    x86_64)	sdk_libdir="sdk/libs/x86_64" ;;
    armv6l)	sdk_libdir="sdk/libs/arm_v6" ;;
    armv7l)	sdk_libdir="sdk/libs/arm_v7" ;;
    armv8l)	sdk_libdir="sdk/libs/arm_v8" ;;
    *)		AC_MSG_RESULT([no]) ;;
esac
AC_MSG_RESULT([yes])
AC_SUBST([sdk_libdir])

##
# Checks for library functions
##
AC_CHECK_FUNCS( \
  getopt_long \
  vsnprintf \
  vsscanf \
)
X_AC_CHECK_COND_LIB(dl, dlerror)
X_AC_CHECK_COND_LIB(m, sqrt)
X_AC_CHECK_COND_LIB(cfitsio, ffopen)

##
# Epilogue
##
AC_CONFIG_FILES( \
  Makefile \
  src/Makefile \
  src/common/Makefile \
  src/common/libsbig/Makefile \
  src/common/libutil/Makefile \
  src/cmd/Makefile \
)

AC_OUTPUT

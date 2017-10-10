##
# Prologue
##
AC_INIT([sbig-util],
        m4_esyscmd([git describe --always | awk '/.*/ {sub(/^v/, ""); printf "%s",$1; exit}']))
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([NEWS])
AC_CANONICAL_SYSTEM
X_AC_EXPAND_INSTALL_DIRS

##
# Automake support
##
AM_INIT_AUTOMAKE([subdir-objects])
AM_CONFIG_HEADER([config/config.h])
AM_MAINTAINER_MODE

AC_DEFINE([_GNU_SOURCE], 1,
          [Define _GNU_SOURCE so that we get all necessary prototypes])

PKG_PROG_PKG_CONFIG

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

##
# Checks for packages
##
PKG_CHECK_MODULES([LIBUSB], [libusb-1.0], [], [])

X_AC_SBIGUDRV

##
# Checks for typedefs, structures, and compiler characteristics
##
AC_C_BIGENDIAN
AC_C_CONST

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
  src/common/libini/Makefile \
  src/cmd/Makefile \
)

AC_OUTPUT

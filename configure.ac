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
  pthread.h \
  getopt.h \
)

##
# Checks for typedefs, structures, and compiler characteristics
## 
AC_C_BIGENDIAN
AC_C_CONST

##
# Set $sdk_libdir and $sdk_includedir according to architecture.
##
AC_MSG_CHECKING([for usable sdk libdir])
sdk_top=sdk/LinuxDevKit.2014-01-10
case "$host_cpu" in
    i386|i686)	sdk_libdir="$sdk_top"/x86/c/lib32 ;;
    x86_64)	sdk_libdir="$sdk_top"/x86/c/lib64 ;;
    arm*)	sdk_libdir="$sdk_top"/arm/c/lib ;;
    *)		AC_MSG_RESULT([no]) ;;
esac
AC_MSG_RESULT([yes])
AC_SUBST([sdk_libdir])
sdk_includedir="$sdk_libdir/../testapp"
AC_SUBST([sdk_includedir])

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

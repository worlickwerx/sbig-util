AC_DEFUN([X_AC_SBIGUDRV], [
  AC_CHECK_HEADER([sbigudrv.h])
  AC_CHECK_HEADER([libsbig/sbigudrv.h],[CFLAGS="${CFLAGS} -I/usr/include/libsbig"])
  ]
)

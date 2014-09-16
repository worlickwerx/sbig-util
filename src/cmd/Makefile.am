AM_CFLAGS = @GCCWARN@

sbigbindir = $(libexecdir)/sbig-util

AM_CPPFLAGS = \
	-DPATH_SBIGUDRV=\"$(libdir)/sbig-util/libsbigudrv.so\" \
	-DPATH_SBIGUDRV_BUILD=\"$(top_srcdir)/$(sdk_libdir)/libsbigudrv.so\" \
	-DEXEC_DIR=\"$(sbigbindir)\" \
	-I$(top_srcdir)/src/common/libsbig

bin_PROGRAMS = sbig

sbigbin_PROGRAMS = \
	sbig-info

LDADD = $(top_builddir)/src/common/libsbig/libsbig.la

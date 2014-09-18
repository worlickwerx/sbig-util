AM_CFLAGS = @GCCWARN@

sbigbindir = $(libexecdir)/sbig-util

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPATH_SBIGUDRV=\"$(libdir)/sbig-util/libsbigudrv.so\" \
	-DPATH_SBIGUDRV_BUILD=\"$(top_srcdir)/$(sdk_libdir)/libsbigudrv.so\" \
	-DEXEC_DIR=\"$(sbigbindir)\"

bin_PROGRAMS = sbig

sbigbin_PROGRAMS = \
	sbig-info

LDADD = \
	$(top_builddir)/src/common/libsbig/libsbig.la \
	$(top_builddir)/src/common/libutil/libutil.la \
	-lm $(LIBDL)

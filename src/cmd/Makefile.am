AM_CFLAGS = @GCCWARN@

sbigbindir = $(libexecdir)/sbig-util

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DEXEC_DIR=\"$(sbigbindir)\"

bin_PROGRAMS = sbig

sbigbin_PROGRAMS = \
	sbig-info \
	sbig-cfw \
	sbig-snap \
	sbig-cooler \
	sbig-focus \
	sbig-find

LDADD = \
	$(top_builddir)/src/common/libsbig/libsbig.la \
	$(top_builddir)/src/common/libutil/libutil.la \
	$(top_builddir)/src/common/libini/libini.la \
	$(LIBM) $(LIBDL) $(LIBCFITSIO)

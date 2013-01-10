AM_CPPFLAGS = -I$(top_srcdir)/lib

ACLOCAL_AMFLAGS = -I m4
GNU_STANDARD_FILES = README.markdown COPYING AUTHORS ChangeLog NEWS

pcfiles = libsearpc.pc

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

EXTRA_DIST = $(GNU_STANDARD_FILES)

EXTRA_DIST += libsearpc.pc.in

if COMPILE_DEMO
  MAKE_DEMO = demo
endif

SUBDIRS = json-glib lib pysearpc ${MAKE_DEMO} tests

install-data-local:
	sed -i "s|(DESTDIR)|${DESTDIR}|g" $(pcfiles)
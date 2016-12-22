AM_CPPFLAGS = -I$(top_srcdir)/lib

ACLOCAL_AMFLAGS = -I m4
GNU_STANDARD_FILES = README.markdown AUTHORS ChangeLog NEWS

pcfiles = libsearpc.pc

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

EXTRA_DIST = $(GNU_STANDARD_FILES) LICENSE.txt

EXTRA_DIST += libsearpc.pc.in

if COMPILE_DEMO
  MAKE_DEMO = demo
endif

SUBDIRS = lib pysearpc ${MAKE_DEMO} tests

install-data-local:
if MACOS
	sed -i '' -e "s|(DESTDIR)|${DESTDIR}|g" $(pcfiles)
else
	${SED} -i "s|(DESTDIR)|${DESTDIR}|g" $(pcfiles)
endif

dist-hook:
	git log -1 > $(distdir)/latest_commit

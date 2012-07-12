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

distcheck-hook:
	chmod 755 $(distdir)
	mkdir -p $(distdir)/_build/demo $(distdir)/_build/tests
	cp $(distdir)/demo/rpc_table.py $(distdir)/_build/demo/
	cp $(distdir)/tests/rpc_table.py $(distdir)/_build/tests/

distcleancheck:
	-rm $(distdir)/_build/demo/rpc_table.py*
	-rm $(distdir)/_build/tests/rpc_table.py*

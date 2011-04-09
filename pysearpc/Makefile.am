EXTRA_DIST = pysearpc-demo-client.py

AM_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include \
	@GLIB2_CFLAGS@ \
	@PYGOBJECT_CFLAGS@ \
	-I/usr/include/python$(PYTHON_VERSION)

if COMPILE_PYTHON
pkgpyexec_PYTHON = __init__.py client.py gen-dfun-macro.py

# generating the fcallfret module
pkgpyexec_LTLIBRARIES = fcallfret.la

fcallfret_la_LDFLAGS = -module -avoid-version -export-symbols-regex initfcallfret
fcallfret_la_SOURCES = fcallfret.c fcallfret.h
fcallfret_la_LIBADD = @GLIB2_LIBS@ @GOBJECT_LIBS@ $(top_builddir)/libsearpc.la

CLEANFILES = fcallfret.h

fcallfret.c: gen-dfun-macro.py
	python $< gen-searpcclient > fcallfret.h
.PHONY: fcallfret.c
endif



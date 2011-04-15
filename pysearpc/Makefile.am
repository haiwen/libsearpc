EXTRA_DIST = pysearpc-demo-client.py

genrpc_files = pysearpc-gen-dfun-macro.py rpc_table.py

AM_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	@GLIB2_CFLAGS@ @PYGOBJECT_CFLAGS@ \
	-I/usr/include/python$(PYTHON_VERSION) \
	-I${top_builddir}/lib -I${top_srcdir}/lib

pkgpyexec_PYTHON = __init__.py client.py pysearpc-gen-dfun-macro.py

# generating the fcallfret module
pkgpyexec_LTLIBRARIES = fcallfret.la

fcallfret_la_LDFLAGS = -module -avoid-version -export-symbols-regex initfcallfret
fcallfret_la_SOURCES = fcallfret.c fcallfret.h
fcallfret_la_LIBADD = @GLIB2_LIBS@ @GOBJECT_LIBS@ $(top_builddir)/lib/libsearpc.la

EXTRA_fcallfret_la_SOURCES = ${genrpc_files}

CLEANFILES = fcallfret.h

fcallfret.c: ${genrpc_files}
	python $< gen-searpcclient > fcallfret.h

.PHONY: fcallfret.c
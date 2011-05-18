
genrpc_files = pygencode.py rpc_table.py

AM_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	@GLIB2_CFLAGS@ @PYGOBJECT_CFLAGS@ \
	-I/usr/include/python$(PYTHON_VERSION) \
	-I${top_builddir}/lib \
	-I${top_srcdir}/lib

pkgpyexec_PYTHON = __init__.py client.py

BUILT_SOURCES = fcallfret.h

# generating the fcallfret module
pkgpyexec_LTLIBRARIES = fcallfret.la

fcallfret_la_LDFLAGS = -module -avoid-version -export-symbols-regex initfcallfret
fcallfret_la_SOURCES = fcallfret.c fcallfret.h
fcallfret_la_LIBADD = @GLIB2_LIBS@ @GOBJECT_LIBS@ $(top_builddir)/lib/libsearpc.la

dist_data_DATA = ${genrpc_files}

fcallfret.h: ${genrpc_files}
	python pygencode.py > fcallfret.h

DISTCLEANFILES = fcallfret.h
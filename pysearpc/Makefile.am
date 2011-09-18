
genrpc_files = pygencode.py rpc_table.py

AM_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	@GLIB2_CFLAGS@ @PYGOBJECT_CFLAGS@ \
	-I/usr/include/python$(PYTHON_VERSION) \
	-I${top_builddir}/lib \
	-I${top_srcdir}/lib

pysearpcdir=${pyexecdir}/pysearpc

pysearpc_PYTHON = __init__.py client.py

BUILT_SOURCES = fcallfret.h

# generating the fcallfret module
pysearpc_LTLIBRARIES = fcallfret.la

fcallfret_la_LDFLAGS = -module -avoid-version -export-symbols-regex initfcallfret  -no-undefined
fcallfret_la_SOURCES = fcallfret.c fcallfret.h
fcallfret_la_LIBADD = @GLIB2_LIBS@ @GOBJECT_LIBS@ $(top_builddir)/lib/libsearpc.la @LIB_PYTHON@

EXTRA_DIST = ${genrpc_files}

fcallfret.h: ${genrpc_files}
	python pygencode.py > fcallfret.h

rpc_table.py: ../lib/rpc_table.py
	cp -f ../lib/rpc_table.py .

DISTCLEANFILES = fcallfret.h rpc_table.py

AM_CFLAGS = @GLIB_CFLAGS@ \
	@JSON_GLIB_CFLAGS@ \
	-I${top_builddir}/lib \
	-I${top_srcdir}/lib \
	-DG_LOG_DOMAIN=\"Searpc\"

lib_LTLIBRARIES = libsearpc.la

include_HEADERS = searpc-client.h searpc-server.h searpc-utils.h searpc.h

libsearpc_la_SOURCES = searpc-client.c searpc-server.c

libsearpc_la_LDFLAGS = -version-info 1:2:0  -no-undefined

libsearpc_la_LIBADD = @GLIB_LIBS@ @JSON_GLIB_LIBS@

dist_bin_SCRIPTS = searpc-codegen.py

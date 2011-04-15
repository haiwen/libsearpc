
rpc_headers = fcall-impr.h  searpc-fcall.h  searpc-dfun.h  searpc-signature.h searpc-marshal.h

genrpc_files = gen-dfun-macro.py rpc_table.py

# rpc_headers = fcall-impr.h searpc-fcall.h searpc-dfun.h searpc-signature.h searpc-marshal.h

AM_CFLAGS = @GLIB2_CFLAGS@ @GOBJECT_CFLAGS@  @JSON_GLIB_CFLAGS@ -I${top_builddir}/lib -I${top_srcdir}/lib

# we need to generate the headers first
BUILT_SOURCES = genrpc

pkglib_LTLIBRARIES = libsearpc.la

include_HEADERS = searpc-client.h searpc-server.h  searpc-utils.h

libsearpc_la_SOURCES = searpc-client.c searpc-server.c

EXTRA_libsearpc_la_SOURCES = ${genrpc_files}

libsearpc_la_LDFLAGS = -Wl,-z -Wl,defs

libsearpc_la_LIBADD = @GLIB2_LIBS@  @GOBJECT_LIBS@ @JSON_GLIB_LIBS@


genrpc: ${rpc_headers} ${genrpc_files}


${rpc_headers}: ${genrpc_files}
	@echo "[libsearpc]: generating rpc header files"
	python $< gen-fcall > fcall-impr.h
	python $< gen-fcall-declare > searpc-fcall.h
	python $< gen-dfun-macro > searpc-dfun.h
	python $< gen-signature > searpc-signature.h
	python $< gen-marshal > searpc-marshal.h
	@echo "[libsearpc]: done"

CLEANFILES = ${rpc_headers}

AM_CFLAGS = @GLIB_CFLAGS@ \
	@JANSSON_CFLAGS@ \
	-I${top_builddir}/lib \
	-I${top_srcdir}/lib

if MACOS
if COMPILE_UNIVERSAL
AM_CFLAGS += -arch x86_64 -arch arm64
endif
endif

if COMPILE_EPOLL
AM_CFLAGS += -DCOMPILE_EPOLL
endif

lib_LTLIBRARIES = libsearpc.la

include_HEADERS = searpc-client.h searpc-server.h searpc-utils.h searpc.h searpc-named-pipe-transport.h

libsearpc_la_SOURCES = searpc-client.c searpc-server.c searpc-utils.c searpc-named-pipe-transport.c

libsearpc_la_LDFLAGS = -version-info 1:2:0  -no-undefined

libsearpc_la_LIBADD = @GLIB_LIBS@ @JANSSON_LIBS@ -lpthread

dist_bin_SCRIPTS = searpc-codegen.py

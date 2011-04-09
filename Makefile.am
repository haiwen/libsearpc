ACLOCAL_AMFLAGS = -I m4
GNU_STANDARD_FILES = README README.markdown COPYING COPYING.LESSER AUTHORS ChangeLog News
EXTRA_DIST = $(GNU_STANDARD_FILES) rpc_table.py gen-dfun-macro.py
SUBDIRS = tests . pysearpc

AM_CFLAGS = @GLIB2_CFLAGS@ @GOBJECT_CFLAGS@  \
	-I$(top_srcdir)/include -I$(top_srcdir)/include/ccnet \
	-I$(top_builddir)/include \
	@JSON_GLIB_CFLAGS@

noinst_PROGRAMS = searpc-demo-server searpc-demo-client

searpc_demo_server_SOURCES = searpc-demo-server.c searpc-demo-packet.h

searpc_demo_server_LDADD = ./libsearpc.la

searpc_demo_client_SOURCES = searpc-demo-client.c searpc-demo-packet.h

searpc_demo_client_LDADD = ./libsearpc.la

# we need to generate the headers first
BUILT_SOURCES = genrpc


pkglib_LTLIBRARIES = libsearpc.la

include_HEADERS = searpc-client.h searpc-server.h  searpc-utils.h

libsearpc_la_SOURCES = searpc-client.c searpc-server.c

libsearpc_la_LDFLAGS = -Wl,-z -Wl,defs

libsearpc_la_LIBADD = @GLIB2_LIBS@  @GOBJECT_LIBS@ \
	@JSON_GLIB_LIBS@

genrpc:	gen-dfun-macro.py
	python gen-dfun-macro.py gen-fcall > fcall-impr.h
	python gen-dfun-macro.py gen-fcall-declare > searpc-fcall.h
	python gen-dfun-macro.py gen-dfun-macro > searpc-dfun.h
	python gen-dfun-macro.py gen-signature > searpc-signature.h
	python gen-dfun-macro.py gen-marshal > searpc-marshal.h

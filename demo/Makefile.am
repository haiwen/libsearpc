generated_sources = searpc-signature.h marshal.h

AM_CFLAGS = @GLIB_CFLAGS@ \
	-I${top_srcdir}/lib \
	-I${top_srcdir}/json-glib

# we need to generate the first
BUILT_SOURCES = gensource

noinst_HEADERS = marshal.h

noinst_PROGRAMS = searpc-demo-server searpc-demo-client searpc-async-client

searpc_demo_server_SOURCES = searpc-demo-server.c searpc-demo-packet.h

searpc_demo_server_LDADD = ${top_builddir}/lib/libsearpc.la @LIB_WS32@ \
	@GLIB_CFLAGS@ \
	${top_builddir}/json-glib/json-glib/libsearpc-json-glib.la

searpc_demo_client_SOURCES = searpc-demo-client.c searpc-demo-packet.h

searpc_demo_client_LDADD = ${top_builddir}/lib/libsearpc.la @LIB_WS32@ \
	@GLIB_CFLAGS@

searpc_async_client_SOURCES = demo-async-client.c searpc-demo-packet.h

searpc_async_client_LDADD = ${top_builddir}/lib/libsearpc.la @LIB_WS32@ \
	@GLIB_CFLAGS@

EXTRA_DIST = rpc_table.py

gensource: ${generated_sources}

${generated_sources}: rpc_table.py
	@echo "[libsearpc]: generating rpc header files"
	python ${top_srcdir}/lib/searpc-codegen.py rpc_table.py
	@echo "[libsearpc]: done"

DISTCLEANFILES = ${generated_sources}

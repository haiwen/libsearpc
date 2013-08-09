generated_sources = searpc-signature.h searpc-marshal.h

AM_CFLAGS = @GLIB_CFLAGS@ \
	-I${top_srcdir}/lib

# we need to generate the first
BUILT_SOURCES = gensource

noinst_PROGRAMS = searpc-demo-server searpc-demo-client searpc-async-client

searpc_demo_server_SOURCES = test-object.c searpc-demo-server.c searpc-demo-packet.h

searpc_demo_server_LDADD = ${top_builddir}/lib/libsearpc.la @LIB_WS32@ \
	@GLIB_LIBS@ @JANSSON_LIBS@

searpc_demo_client_SOURCES = test-object.c searpc-demo-client.c searpc-demo-packet.h

searpc_demo_client_LDADD = ${top_builddir}/lib/libsearpc.la @LIB_WS32@ \
	@GLIB_LIBS@ @JANSSON_LIBS@

searpc_async_client_SOURCES = demo-async-client.c searpc-demo-packet.h

searpc_async_client_LDADD = ${top_builddir}/lib/libsearpc.la @LIB_WS32@ \
	@GLIB_LIBS@ @JANSSON_LIBS@

EXTRA_DIST = rpc_table.py

gensource: ${generated_sources}

${generated_sources}: ${top_srcdir}/demo/rpc_table.py
	@echo "[libsearpc]: generating rpc header files"
	@PYTHON@ ${top_srcdir}/lib/searpc-codegen.py ${top_srcdir}/demo/rpc_table.py
	@echo "[libsearpc]: done"

clean-local:
	rm -f ${generated_sources}
	rm -f rpc_table.pyc


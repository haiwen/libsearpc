AM_CFLAGS = @GLIB2_CFLAGS@ @GOBJECT_CFLAGS@ @JSON_GLIB_CFLAGS@ -I${top_builddir}/lib -I${top_srcdir}/lib

noinst_PROGRAMS = searpc-demo-server searpc-demo-client searpc-async-client

searpc_demo_server_SOURCES = searpc-demo-server.c searpc-demo-packet.h

searpc_demo_server_LDADD = ${top_builddir}/lib/libsearpc.la

searpc_demo_client_SOURCES = searpc-demo-client.c searpc-demo-packet.h

searpc_demo_client_LDADD = ${top_builddir}/lib/libsearpc.la

searpc_async_client_SOURCES = demo-async-client.c searpc-demo-packet.h

searpc_async_client_LDADD = ${top_builddir}/lib/libsearpc.la



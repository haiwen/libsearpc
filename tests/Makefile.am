
AM_CFLAGS = @GLIB2_CFLAGS@  @GOBJECT_CFLAGS@ @JSON_GLIB_CFLAGS@ \
	-I${top_builddir}/lib -I${top_srcdir}/lib


check_PROGRAMS = test-searpc

test_searpc_SOURCES = test-searpc.c
test_searpc_LDADD = @GLIB2_LIBS@  @GOBJECT_LIBS@ \
    $(top_builddir)/lib/libsearpc.la	
test_searpc_LDFLAGS = -static

TESTS = test-searpc

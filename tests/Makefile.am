
AM_CFLAGS = @GLIB_CFLAGS@ \
	-I${top_srcdir}/lib

check_PROGRAMS = test-searpc

test_searpc_SOURCES = test-searpc.c
test_searpc_LDADD = @GLIB_LIBS@  \
    $(top_builddir)/lib/libsearpc.la	
test_searpc_LDFLAGS = -static

TESTS = test-searpc

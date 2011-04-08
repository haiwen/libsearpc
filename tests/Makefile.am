
AM_CFLAGS = -I$(top_srcdir)/include @GLIB2_CFLAGS@  @GOBJECT_CFLAGS@ \
	-I$(top_srcdir)/lib -I$(top_srcdir)/lib/searpc \
	@JSON_GLIB_CFLAGS@

check_PROGRAMS = test-searpc

test_searpc_SOURCES = test-searpc.c
test_searpc_LDADD = @GLIB2_LIBS@  @GOBJECT_LIBS@ \
    $(top_builddir)/libsearpc.la	
test_searpc_LDFLAGS = -static

TESTS = test-searpc

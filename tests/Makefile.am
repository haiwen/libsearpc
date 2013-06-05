generated_sources = searpc-signature.h searpc-marshal.h

AM_CFLAGS = @GLIB_CFLAGS@ \
	@JSON_GLIB_CFLAGS@ \
	-I${top_srcdir}/lib

# we need to generate the first
BUILT_SOURCES = gensource

check_PROGRAMS = test-searpc

test_searpc_SOURCES = test-searpc.c
test_searpc_LDADD = @GLIB_LIBS@ @JSON_GLIB_LIBS@ \
    $(top_builddir)/lib/libsearpc.la
test_searpc_LDFLAGS = -static

TESTS = test-searpc

EXTRA_DIST = rpc_table.py

gensource: ${generated_sources}

${generated_sources}: $(top_srcdir)/tests/rpc_table.py
	@echo "[libsearpc]: generating rpc header files"
	@PYTHON@ ${top_srcdir}/lib/searpc-codegen.py $(top_srcdir)/tests/rpc_table.py
	@echo "[libsearpc]: done"

clean-local:
	rm -f ${generated_sources}
	rm -f rpc_table.pyc


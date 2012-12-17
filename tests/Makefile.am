generated_sources = searpc-signature.h searpc-marshal.h

AM_CFLAGS = @GLIB_CFLAGS@ \
	-I${top_srcdir}/lib \
	-I${top_srcdir}/json-glib

# we need to generate the first
BUILT_SOURCES = gensource

check_PROGRAMS = test-searpc

test_searpc_SOURCES = test-searpc.c
test_searpc_LDADD = @GLIB_LIBS@  \
    $(top_builddir)/lib/libsearpc.la \
    ${top_builddir}/json-glib/json-glib/libsearpc-json-glib.la
test_searpc_LDFLAGS = -static

TESTS = test-searpc

EXTRA_DIST = rpc_table.py

gensource: ${generated_sources}

${generated_sources}: $(top_srcdir)/tests/rpc_table.py
	@echo "[libsearpc]: generating rpc header files"
	@PYTHON@ ${top_srcdir}/lib/searpc-codegen.py $(top_srcdir)/tests/rpc_table.py
	@echo "[libsearpc]: done"

clean-local:
	-rm ${generated_sources}


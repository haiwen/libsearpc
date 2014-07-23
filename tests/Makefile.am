generated_sources = searpc-signature.h searpc-marshal.h
clar_suite_sources = clar.suite

AM_CFLAGS = @GLIB_CFLAGS@ \
	@JANSSON_CFLAGS@ \
	-I${top_srcdir}/lib

# we need to generate the first
BUILT_SOURCES = gensource

check_PROGRAMS = test-searpc

test_searpc_SOURCES = searpc.c clar.c main.c
test_searpc_LDADD = @GLIB_LIBS@  \
    @JANSSON_LIBS@ \
    $(top_builddir)/lib/libsearpc.la
test_searpc_LDFLAGS = -static

TESTS = test-searpc

EXTRA_DIST = rpc_table.py generate.py

gensource: ${generated_sources} ${clar_suite_sources}

rpc_table.stamp: ${top_srcdir}/tests/rpc_table.py ${top_srcdir}/lib/searpc-codegen.py
	@rm -f rpc_table.tmp
	@touch rpc_table.tmp
	@echo "[libsearpc]: generating rpc header files"
	@PYTHON@ ${top_srcdir}/lib/searpc-codegen.py ${top_srcdir}/tests/rpc_table.py
	@echo "[libsearpc]: done"
	@mv -f rpc_table.tmp $@

${generated_sources}: rpc_table.stamp

${clar_suite_sources}: $(top_srcdir)/tests/generate.py
	@PYTHON@ $(top_srcdir)/tests/generate.py

clean-local:
	rm -f ${generated_sources}
	rm -f rpc_table.pyc
	rm -f rpc_table.stamp
	rm -f rpc_table.tmp
	rm -f generate.pyc


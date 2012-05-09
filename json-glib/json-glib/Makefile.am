include $(top_srcdir)/build/autotools/Makefile.am.silent
include $(top_srcdir)/build/autotools/Makefile.am.gtest

if ENABLE_GLIB_TEST
# build this directory *before* the tests/
SUBDIRS = . tests
endif

DIST_SUBDIRS = tests

NULL =

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = \
	-DPREFIX=\""$(prefix)"\" 	\
	-DLIBDIR=\""$(libdir)"\" 	\
	-DJSON_COMPILATION=1		\
	-DJSON_DISABLE_DEPRECATED	\
	-DG_LOG_DOMAIN=\"Json\" 	\
	$(JSON_DEBUG_CFLAGS) 		\
	$(NULL)

AM_CFLAGS = $(JSON_CFLAGS) $(MAINTAINER_CFLAGS) -fPIC

BUILT_SOURCES =

CLEANFILES =
DISTCLEANFILES = json-version.h

source_h = \
	$(top_srcdir)/json-glib/json-builder.h		\
	$(top_srcdir)/json-glib/json-generator.h 	\
	$(top_srcdir)/json-glib/json-gobject.h 		\
	$(top_srcdir)/json-glib/json-parser.h 		\
	$(top_srcdir)/json-glib/json-path.h		\
	$(top_srcdir)/json-glib/json-reader.h		\
	$(top_srcdir)/json-glib/json-types.h 		\
	$(top_srcdir)/json-glib/json-gvariant.h		\
	$(NULL)

source_h_private = \
	$(top_srcdir)/json-glib/json-debug.h		\
	$(top_srcdir)/json-glib/json-gobject-private.h	\
	$(top_srcdir)/json-glib/json-scanner.h 		\
	$(top_srcdir)/json-glib/json-types-private.h	\
	$(NULL)

source_c = \
	$(srcdir)/json-array.c 		\
	$(srcdir)/json-builder.c	\
	$(srcdir)/json-debug.c		\
	$(srcdir)/json-gboxed.c		\
	$(srcdir)/json-generator.c 	\
	$(srcdir)/json-gobject.c 	\
	$(srcdir)/json-node.c 		\
	$(srcdir)/json-object.c 	\
	$(srcdir)/json-parser.c 	\
	$(srcdir)/json-path.c		\
	$(srcdir)/json-reader.c		\
	$(srcdir)/json-scanner.c 	\
	$(srcdir)/json-serializable.c	\
	$(srcdir)/json-gvariant.c	\
	$(NULL)

# glib-mkenums rules
glib_enum_h = json-enum-types.h
glib_enum_c = json-enum-types.c
glib_enum_headers = $(source_h)
include $(top_srcdir)/build/autotools/Makefile.am.enums

# glib-genmarshal rules
glib_marshal_list = json-marshal.list
glib_marshal_prefix = _json_marshal
include $(top_srcdir)/build/autotools/Makefile.am.marshal

lib_LTLIBRARIES = libsearpc-json-glib.la

libsearpc_json_glib_la_LIBADD = $(JSON_LIBS)
libsearpc_json_glib_la_SOURCES = $(source_c) $(source_h) $(source_h_private) $(BUILT_SOURCES)
libsearpc_json_glib_la_LDFLAGS = $(JSON_LT_LDFLAGS) -export-dynamic -export-symbols-regex "^json.*" -rpath $(libdir)

jsonincludedir = $(includedir)/searpc/json-glib
jsoninclude_DATA = \
	$(source_h) 					\
	$(top_builddir)/json-glib/json-enum-types.h 	\
	$(top_builddir)/json-glib/json-version.h	\
	$(top_srcdir)/json-glib/json-glib.h		\
	$(NULL)

EXTRA_DIST += json-version.h.in json-glib.h json-version.h

TESTS_ENVIRONMENT = srcdir="$(srcdir)" json_all_c_sources="$(source_c)"

EXTRA_DIST += json-glib.symbols

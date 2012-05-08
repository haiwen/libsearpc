# bump micro_version to the next even number for each point release
# bump micro_version to the next odd number after each release
m4_define([json_major_version], [0])
m4_define([json_minor_version], [14])
m4_define([json_micro_version], [2])

m4_define([json_version], [json_major_version.json_minor_version.json_micro_version])

m4_define([json_release_status],
          [m4_if(m4_eval(json_micro_version % 2), [1], [git],
                 [m4_if(m4_eval(json_minor_version % 2), [1], [snapshot],
                                                              [release])])])

# bump up by 1 for every micro release with no API changes, otherwise
# set to 0. after release, bump up by 1
m4_define([json_interface_age], [2])
m4_define([json_binary_age], [m4_eval(100 * json_minor_version + json_micro_version)])

m4_define([lt_current], [m4_eval(100 * json_minor_version + json_micro_version - json_interface_age)])
m4_define([lt_revision], [json_interface_age])
m4_define([lt_age], [m4_eval(json_binary_age - json_interface_age)])

m4_define([glib_req_version], [2.26])

AC_PREREQ([2.63])

AC_INIT([json-glib],
        [json_version],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=json-glib],
        [json-glib],
        [http://live.gnome.org/JsonGlib])

AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([json-glib/json-glib.h])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build/autotools])

AM_INIT_AUTOMAKE([1.11 no-define foreign -Wno-portability dist-xz no-dist-gzip tar-ustar])

AM_SILENT_RULES([yes])
AM_PATH_GLIB_2_0
AM_PROG_CC_C_O

LT_PREREQ([2.2.6])
LT_INIT([disable-shared])
dnl LT_INIT([disable-static])

# Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

JSON_MAJOR_VERSION=json_major_version
JSON_MINOR_VERSION=json_minor_version
JSON_MICRO_VERSION=json_micro_version
JSON_VERSION=json_version
JSON_RELEASE_STATUS=json_release_status
AC_SUBST(JSON_MAJOR_VERSION)
AC_SUBST(JSON_MICRO_VERSION)
AC_SUBST(JSON_MINOR_VERSION)
AC_SUBST(JSON_VERSION)
AC_SUBST(JSON_RELEASE_STATUS)

JSON_LT_CURRENT=lt_current
JSON_LT_REVISION=lt_revision
JSON_LT_AGE=lt_age
JSON_LT_VERSION="$JSON_LT_CURRENT:$JSON_LT_REVISION:$JSON_LT_AGE"
JSON_LT_LDFLAGS="-version-info $JSON_LT_VERSION"

AC_CANONICAL_HOST
AC_MSG_CHECKING([if building for some Win32 platform])
AS_CASE([$host],

        [*-*-mingw*|*-*-cygwin*],
        [
          JSON_LT_LDFLAGS="$JSON_LT_LDFLAGS -no-undefined"
          platform_win32=yes
        ],

        [platform_win32=no]
)
AC_MSG_RESULT([$platform_win32])

AC_SUBST(JSON_LT_LDFLAGS)

GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
AC_SUBST(GLIB_PREFIX)

PKG_CHECK_MODULES(JSON, [gobject-2.0 >= glib_req_version gio-2.0])
AC_SUBST(JSON_CFLAGS)
AC_SUBST(JSON_LIBS)

AM_CONDITIONAL(ENABLE_GLIB_TEST, [test "x$enable_glibtest" = "xyes"])

dnl = Enable debug level ======================================================

m4_define([debug_default], [m4_if(m4_eval(json_minor_version % 2), [1], [yes], [minimum])])

AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                              [turn on debugging @<:@default=]debug_default[@:>@])],
              [],
              [enable_debug=debug_default])

AS_CASE([$enable_debug],

        [yes],
        [
          test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
          JSON_DEBUG_CFLAGS="-DJSON_ENABLE_DEBUG"
        ],

        [minimum],
        [
          JSON_DEBUG_CFLAGS="-DJSON_ENABLE_DEBUG -DG_DISABLE_CAST_CHECKS"
        ],

        [no],
        [
          JSON_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
        ],

        [AC_MSG_ERROR([Unknown argument to --enable-debug])]
)

AC_SUBST(JSON_DEBUG_CFLAGS)

GETTEXT_PACKAGE="json-glib-1.0"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],
                   ["$GETTEXT_PACKAGE"],
                   [The prefix for our gettext translation domains.])

AC_CONFIG_FILES([
        Makefile
        build/Makefile
        build/autotools/Makefile
        json-glib/Makefile
        json-glib/json-version.h
        json-glib/tests/Makefile
])

AC_OUTPUT

dnl === Summary ===============================================================

echo ""
echo " Json-GLib - $VERSION (${JSON_RELEASE_STATUS})"
echo ""
echo " • Prefix: ${prefix}"
echo ""
echo " • Debug level: ${enable_debug}"
echo " • Compiler flags: ${CFLAGS}"
echo ""
echo ""
echo " • Enable test suite: ${enable_glibtest}"
echo ""

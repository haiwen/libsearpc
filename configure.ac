#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([libsearpc], [1.0], [freeplant@gmail.com])
AC_CONFIG_SRCDIR([searpc-server.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])


AM_INIT_AUTOMAKE([1.9])
# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AM_PATH_PYTHON([2.4])
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_CHECK_FUNCS([memset socket strerror strndup])


# glib and json-glib
GLIB_REQUIRED=2.16.0
JSON_GLIB_REQUIRED=0.10.2
PYGOBJECT_REQUIRED=2.0

PKG_CHECK_MODULES(GLIB2, [glib-2.0 >= $GLIB_REQUIRED])
AC_SUBST(GLIB2_CFLAGS)
AC_SUBST(GLIB2_LIBS)

PKG_CHECK_MODULES(GOBJECT, [gobject-2.0 >= $GLIB_REQUIRED])
AC_SUBST(GOBJECT_CFLAGS)
AC_SUBST(GOBJECT_LIBS)

PKG_CHECK_MODULES(JSON_GLIB, [json-glib-1.0 >= $JSON_GLIB_REQUIRED])
AC_SUBST(JSON_GLIB_CFLAGS)
AC_SUBST(JSON_GLIB_LIBS)

PKG_CHECK_MODULES(PYGOBJECT, [pygobject-2.0 >= $PYGOBJECT_REQUIRED])
AC_SUBST(PYGOBJECT_CFLAGS)
AC_SUBST(PYGOBJECT_LIBS)

AC_CONFIG_FILES([Makefile
                 pysearpc/Makefile
                 tests/Makefile])
AC_OUTPUT

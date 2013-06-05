#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([libsearpc], [1.1.0], [freeplant@gmail.com])
AC_CONFIG_SRCDIR([lib/searpc-server.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])


AM_INIT_AUTOMAKE([1.9 foreign])
AC_MINGW32
# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for header files.
#AC_CHECK_HEADERS([arpa/inet.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_C_INLINE
#AC_TYPE_SIZE_T
#AC_TYPE_SSIZE_T
#AC_TYPE_UINT16_T

# Checks for library functions.
#AC_CHECK_FUNCS([memset socket strerror strndup])

# Options about demos and pysearpc

# option: compile-demo
# default: yes 
AC_ARG_ENABLE([compile-demo],
[AS_HELP_STRING([--enable-compile-demo],
[compile demo programs @<:@default: yes@:>@])],
[compile_demo=${enableval}], [compile_demo=yes])

AM_CONDITIONAL([COMPILE_DEMO], [test x${compile_demo} = xyes])

AC_ARG_ENABLE(server-pkg, 
    AC_HELP_STRING([--enable-server-pkg], [enable static compile]),
    [server_pkg=$enableval],[server_pkg="no"])

dnl - check if the macro WIN32 is defined on this compiler.
AC_MSG_CHECKING(for WIN32)

if test "$MINGW32" = yes; then
  bwin32=true
  LIB_WS32=-lws2_32
  AC_SUBST(LIB_WS32)
  AC_MSG_RESULT(compile in mingw32)
fi

AM_CONDITIONAL([WIN32], [test "$MINGW32" = "yes"])
AC_SUBST(WIN32)

AC_MSG_CHECKING(for Mac)

if test ${TERM_PROGRAM} = "Apple_Terminal" ; then
   bmac=yes
fi

if test x$bmac = "xyes"; then
  server_pkg=no
  AC_MSG_RESULT(compile in mac)
fi

AM_CONDITIONAL([MACOS], [test "$bmac" = "yes"])
AC_SUBST(MACOS)

# Checks for libraries.

GLIB_REQUIRED=2.26.0
JSON_GLIB_REQUIRED=0.14.2

# check and subst gobject
PKG_CHECK_MODULES(GLIB, [gobject-2.0 >= $GLIB_REQUIRED])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(JSON_GLIB, [json-glib-1.0 >= $JSON_GLIB_REQUIRED])
AC_SUBST(JSON_GLIB_CFLAGS)
AC_SUBST(JSON_GLIB_LIBS)

AM_PATH_PYTHON([2.4])
if test "$bwin32" = true; then
   if test x$PYTHON_DIR != x; then
   # set pyexecdir to somewhere like /c/Python26/Lib/site-packages
      pyexecdir=${PYTHON_DIR}/Lib/site-packages
      pythondir=${pyexecdir}
      pkgpyexecdir=${pyexecdir}/${PACKAGE}
      pkgpythondir=${pythondir}/${PACKAGE}
   fi
fi

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 demo/Makefile
                 pysearpc/Makefile
                 libsearpc.pc
                 tests/Makefile])
AC_OUTPUT

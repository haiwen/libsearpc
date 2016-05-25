#!/bin/bash
# Run this to generate all the initial makefiles, etc.

: ${AUTOCONF=autoconf}
: ${AUTOHEADER=autoheader}
: ${AUTOMAKE=automake}
: ${ACLOCAL=aclocal}
if test "$(uname)" != "Darwin"; then
    : ${LIBTOOLIZE=libtoolize}
else
    : ${LIBTOOLIZE=glibtoolize}
fi
: ${LIBTOOL=libtool}

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

cd $srcdir
PROJECT=libsearpc
CONFIGURE=configure.ac

DIE=0

($AUTOCONF --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

($AUTOMAKE --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have automake installed to compile $PROJECT."
	echo "Get ftp://sourceware.cygnus.com/pub/automake/automake-1.7.tar.gz"
	echo "(or a newer version if it is available)"
	DIE=1
}

if test "$(uname)" != "Darwin"; then
(grep "^AC_PROG_LIBTOOL" $CONFIGURE >/dev/null) && {
  ($LIBTOOL --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`libtool' installed to compile $PROJECT."
    echo "Get ftp://ftp.gnu.org/pub/gnu/libtool-1.4.tar.gz"
    echo "(or a newer version if it is available)"
    DIE=1
  }
}
fi


if test "$DIE" -eq 1; then
	exit 1
fi

dr=`dirname .`
echo processing $dr
aclocalinclude="$aclocalinclude -I m4"

if test x"$MSYSTEM" = x"MINGW32"; then
    aclocalinclude="$aclocalinclude -I /mingw32/share/aclocal"
elif test "$(uname)" = "Darwin"; then
    aclocalinclude="$aclocalinclude -I /opt/local/share/aclocal"
fi


echo "Creating $dr/aclocal.m4 ..."
test -r $dr/aclocal.m4 || touch $dr/aclocal.m4
echo "Running glib-gettextize...  Ignore non-fatal messages."
echo "no" | glib-gettextize --force --copy

echo "Making $dr/aclocal.m4 writable ..."
test -r $dr/aclocal.m4 && chmod u+w $dr/aclocal.m4

echo "Running $LIBTOOLIZE..."
$LIBTOOLIZE --force --copy

echo "Running $ACLOCAL $aclocalinclude ..."
$ACLOCAL $aclocalinclude

echo "Running $AUTOHEADER..."
$AUTOHEADER

echo "Running $AUTOMAKE --gnu $am_opt ..."
$AUTOMAKE --add-missing --gnu $am_opt

echo "Running $AUTOCONF ..."
$AUTOCONF

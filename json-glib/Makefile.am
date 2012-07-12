include $(top_srcdir)/build/autotools/Makefile.am.gtest
include $(top_srcdir)/build/autotools/Makefile.am.silent

ACLOCAL_AMFLAGS = -I build/autotools

SUBDIRS = json-glib build

CLEANFILES = $(pcfiles) test-report.xml

DISTCHECK_CONFIGURE_FLAGS = --enable-maintainer-flags

dist-hook:
	@if test -d "$(srcdir)/.git"; then \
	  echo Generating ChangeLog ; \
	  ( cd "$(srcdir)" \
	    && $(top_srcdir)/build/missing --run git log --stat ) > ChangeLog.tmp \
	  && mv -f ChangeLog.tmp $(top_distdir)/ChangeLog \
	  || ( rm -f ChangeLog.tmp; \
	       echo Failed to generate ChangeLog >&2 ); \
	else \
	  echo A git checkout is required to generate a ChangeLog >&2; \
	fi

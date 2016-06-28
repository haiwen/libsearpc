/*
 * Copyright (c) Vicent Marti. All rights reserved.
 *
 * This file is part of clar, distributed under the ISC license.
 * For full terms see the included COPYING file.
 */

#include <glib.h>
#include <glib-object.h>

#include "clar_test.h"

/*
 * Minimal main() for clar tests.
 *
 * Modify this with any application specific setup or teardown that you need.
 * The only required line is the call to `clar_test(argc, argv)`, which will
 * execute the test suite.  If you want to check the return value of the test
 * application, main() should return the same value returned by clar_test().
 */

#ifdef _WIN32
int __cdecl main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init ();
#endif

#ifndef WIN32
    signal (SIGPIPE, SIG_IGN);
#endif
	/* Run the test suite */
	return clar_test(argc, argv);
}

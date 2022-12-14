/*
 * Support library.
 *
 *     strlwr - Convert string to lower case.
 *
 * $Author: klarenw $
 * $Date: 1998-12-27 21:51:17 +0100 (Sun, 27 Dec 1998) $
 * $Revision: 134 $
 * $Log$
 * Revision 1.2  1998/12/27 20:51:17  klarenw
 * Added pattern matching functions and error code conversion functions.
 * Fixed bugs in file buffering functions and strlwr functions.
 *
 * Revision 1.1  1997/12/24 22:44:35  klarenw
 * First version. Library contains only strlwr.c.
 */

#include "gup.h"
#include "support.h"

#if !defined(HAVE_STRLWR)

/*
 * char *strlwr(char *string)
 *
 * Convert string to lower case.
 */

char *strlwr(char *string)
{
	char *ptr = string;

	while(*ptr)
	{
		*ptr = (char) tolower(*ptr);
		ptr++;
	}

	return string;
}

#endif




FILE* assert_redir_fptr = NULL;

int report_assertion_failure(const char* msg, ...)
{
	va_list parm;

	va_start(parm, msg);

	if (!assert_redir_fptr)
		assert_redir_fptr = stderr;

	vfprintf(assert_redir_fptr, msg, parm);

	va_end(parm);

#if defined(_WIN32)
	// invoke debugger:
	DebugBreak();
#endif

	abort();

	return 0;
}


/*
 * Support library.
 *
 *     Support functions.
 *
 * $Author: klarenw $
 * $Date: 1998-12-27 21:48:08 +0100 (Sun, 27 Dec 1998) $
 * $Revision: 132 $
 * $Log$
 * Revision 1.2  1998/12/27 20:48:08  klarenw
 * Removed settings.h. New names for error codes.
 *
 * Revision 1.1  1997/12/24 22:49:50  klarenw
 * Created structure for commandline options and library for missing
 * functions.
 */

#include "gup.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef HAVE_STRLWR
extern char *strlwr(char *string);
#endif

int match_pattern(const char *name, const char *pattern);

#ifdef __cplusplus
}
#endif

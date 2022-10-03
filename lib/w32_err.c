/*
 * Support library.
 *
 *     w32_err - Convert Win32 error codes to GUP error codes.
 *
 * $Author: wout $
 * $Date: 2000-07-30 17:51:01 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 180 $
 * $Log$
 * Revision 1.1  2000/07/30 15:51:01  wout
 * Added Win32 support.
 *
 */

#include <windows.h>
#include <stdio.h>

#include "gup_err.h"
#include "w32_err.h"

/*
 * gup_result gup_conv_win32_err(DWORD error)
 *
 * Convert a Win32 error code to a GUP error code. Note that if error
 * is 0, GUP_UNKNOWN is returned. Call this function only when
 * there is an error.
 *
 * Parameters:
 *
 * error	- Win32 error code <> 0.
 *
 * Result: GUP error code.
 */

gup_result gup_conv_win32_err(DWORD error)
{
	switch(error)
	{
	case ERROR_FILE_NOT_FOUND:			/* File does not exist. */
	case ERROR_PATH_NOT_FOUND:			/* Path not found. */
		return GUP_NOENT;
	case ERROR_TOO_MANY_OPEN_FILES:		/* Too many open files. */
		return GUP_MFILE;
	case ERROR_ACCESS_DENIED:			/* Access denied. */
		return GUP_ACCESS;
	case ERROR_FILE_EXISTS:				/* File already exists. */
		return GUP_EXIST;
	case ERROR_DISK_FULL:				/* No space left on device. */
		return GUP_NOSPC;
	default:							/* Unknown errors. */
#if 1
		printf("Unknown Win32 error code %ld. Please report.\r\n", error);
#endif
		return GUP_UNKNOWN;
	}
}

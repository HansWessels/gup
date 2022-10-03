/*
 * Include files.
 *
 *     Win32 to GUP result / error codes conversion.
 *
 * $Author: wout $
 * $Date: 2000-07-30 17:51:01 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 180 $
 * $Log$
 * Revision 1.1  2000/07/30 15:49:24  wout
 * Added Win32 support.
 *
 */

#ifndef __GUP_WIN32_ERRORS_H__
#define __GUP_WIN32_ERRORS_H__

#ifdef __cplusplus
extern "C"
{
#endif

gup_result gup_conv_win32_err(DWORD error);	/* Convert Win32 error code to GUP error code. */
#ifdef __cplusplus
}
#endif

#endif

/*
 * Archive manager.
 *
 *     OS specific declarations.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:36:29 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 203 $
 * $Log$
 * Revision 1.1  2000/09/03 12:36:29  wout
 * Include file with system dependent type definitions of the archive
 * manager.
 *
 */

#if (OS == OS_UNIX)
#include "../../arcman/include/unix/arcman.h"
#elif (OS == OS_WIN32)
#include "../../arcman/include/win32/arcman.h"
#else
#error "This OS is not supported."
#endif

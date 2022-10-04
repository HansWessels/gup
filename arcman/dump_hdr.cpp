/*
 * Archive manager.
 *
 *     *DUMP main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:03:31 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 215 $
 * $Log$
 * Revision 1.6  2000/09/03 13:01:59  wout
 * Better multiple volume support.
 *
 * Revision 1.5  2000/07/30 15:19:50  wout
 * Added functions update_file_attributes and update_ext_headers.
 * Added partial support for extended headers.
 *
 * Revision 1.4  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.3  1998/03/26 19:59:25  klarenw
 * Updated to last version of the compression engine. Cleaner interface.
 *
 * Revision 1.2  1998/01/03 19:24:42  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:49  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#include "gup.h"

#if ENABLE_DUMP_OUTPUT_MODES

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "sysdep/arcman.h"
#include "header.h"
#include "dump_hdr.h"
#include "gup_err.h"
#include "os.h"

/*****************************************************************************
 *																			 *
 * Member functions of the dump_mainheader class.							 *
 *																			 *
 *****************************************************************************/

dump_mainheader::dump_mainheader(const char *comment) : arj_mainheader(comment), arc_output_size(0)
{
	TRACE_ME();
}

dump_mainheader::dump_mainheader(const dump_mainheader& from) : arj_mainheader(from), arc_output_size(0)
{
	TRACE_ME();
}

dump_mainheader::~dump_mainheader()
{
	TRACE_ME();
}

// ====================================================================================================

dump_fileheader::dump_fileheader(const char *filename, const char *comment) : arj_fileheader(filename, comment)
{
	TRACE_ME();
}

dump_fileheader::dump_fileheader(const char *filename, const char *comment, const osstat *stat) : arj_fileheader(filename, comment, stat)
{
	TRACE_ME();
}

dump_fileheader::~dump_fileheader()
{
	TRACE_ME();
}


#endif // ENABLE_DUMP_OUTPUT_MODES


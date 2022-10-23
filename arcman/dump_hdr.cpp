/*
 * Archive manager.
 *
 *     *DUMP main and file header classes.
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

dump_mainheader::dump_mainheader(const char *archive_filepath, const char *comment) 
: 	arj_mainheader(comment), 
	archive_path(archive_filepath), 
	archive_output_size(0), 
	current_file_pack_start_offset(0), 
	archive_ctime(0)
{
	TRACE_ME_EX("archive_path = %s", archive_path.c_str());
}

dump_mainheader::dump_mainheader(const dump_mainheader& from) 
: 	arj_mainheader(from), 
	archive_path(from.archive_path), 
	archive_output_size(from.archive_output_size), 
	current_file_pack_start_offset(from.current_file_pack_start_offset), 
	archive_ctime(from.archive_ctime),
	archive_comment(from.archive_comment),
	archive_volume_path(from.archive_volume_path), 
	archive_metafile_path(from.archive_metafile_path), 
	
{
	TRACE_ME_EX("archive_path = %s", archive_path.c_str());
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


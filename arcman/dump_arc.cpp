/*
 * Archive manager.
 *
 *     *DUMP archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:59:48 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 213 $
 * $Log$
 * Revision 1.8  2000/09/03 12:59:48  wout
 * Better multiple volume support. Added check on the maximum size of a
 * header while writing a header to file. Added function
 * skip_compressed_data().
 *
 * Revision 1.7  2000/07/30 15:15:43  wout
 * Use gup_io_write_announce, gup_io_get_current etc. instead of
 * read and write, to write and read file headers and archive
 * headers.
 * Removed the buf_getw and buf_getl functions.
 *
 * Revision 1.6  2000/07/16 17:11:10  hwessels
 * Updated to GUP 0.0.4
 *
 * Revision 1.5  1998/12/28 14:58:08  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.4  1998/03/26 19:59:24  klarenw
 * Updated to last version of the compression engine. Cleaner interface.
 *
 * Revision 1.3  1998/03/17 18:13:53  klarenw
 * Adapted to the new filebuffering scheme used by encode() and
 * decode().
 *
 * Revision 1.2  1998/01/03 19:24:41  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:48  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#include "gup.h"

#if ENABLE_DUMP_OUTPUT_MODES

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "arc_util.h"
#include "gup_err.h"
#include "compress.h"
#include "gup_io.h"
#include "options.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "archive.h"
#include "arcctl.h"
#include "dump_hdr.h"
#include "dump_arc.h"
#include "crc.h"
#include "os.h"
#include "support.h"


/*
 * int get_arj_suffix_cnt(const char *filename, char **suffix_ptr)
 *
 * Determine the index of the next volume from the filename of
 * the archive. If the suffix of the filename is '.arj' the index
 * of the next volume is 1. If the suffix is '.axx' the index of the
 * the next volume is xx + 1. For other suffixes multiple volume
 * archives are not supported. In this case the result is 0.
 *
 * Parameters:
 *
 * filename		- filename of archive.
 * suffix_ptr	- if multiple volume is supported, this pointer
 *				  points to the last two characters of the filename
 *				  (where the volume counter is).
 *
 * Result: 0 if multiple volumes are not supported, otherwise the
 *		   index of the next volume.
 */

static int get_arj_suffix_cnt(const char *filename, char **suffix_ptr)
{
	register int arj_suff;
	register char *name_ptr;

	/*
	 * Note in the following code that on some operating systems
	 * (UNIX, Atari ST with MiNT, Apple) that the filename can
	 * more than one '.' and both uppercase and lowercase characters.
	 * Both '.arj' and '.ARJ' are considered to be valid suffixes
	 * of ARJ archives. When the filename of the archive contains
	 * more than one '.', the part after the last '.' is considered
	 * to be the suffix.
	 */

	name_ptr = get_name(filename);

	if ((*suffix_ptr = strrchr(name_ptr, '.')) == NULL)
		arj_suff = 0;					/* No suffix, multiple volume not supported. */
	else
	{
		(*suffix_ptr) += 2;				/* Skip 'a' or 'A'. */

		if (match_pattern(name_ptr, "*.[aA][rR][jJ]"))
		{
			/*
			 * Suffix is '.arj'. Next volume index is 1.
			 */

			arj_suff = 1;
		}
		else
		{
			if (match_pattern(name_ptr, "*.[aA][0-9][0-9]"))
			{
				/*
				 * The archive suffix is '.axx', where xx is a two
				 * digit number. Set the next volume counter to xx + 1.
				 */

				arj_suff = atoi(*suffix_ptr) + 1;
			}
			else
				arj_suff = 0;			/* Multiple volume not supported for this suffix. */
		}
	}

	return arj_suff;
}


/*****************************************************************************
 *																			 *
 * Constructor and destructor.										 		 *
 * 																			 *
 *****************************************************************************/

dump_archive::dump_archive(void)
{
	TRACE_ME();
}

dump_archive::~dump_archive(void)
{
	TRACE_ME();
}



/*****************************************************************************
 *																			 *
 * GUP I/O virtualization functions.														 *
 * 																			 *
 *****************************************************************************/

// We're virtualizing the core gup I/O functions here, the idea being: 
// when we want to output arbitrary encodings of the packed data (the *DUMP modes)
// we can achieve such most easily when we keep everything else intact and
// "just find a way to hook into the raw I/O and reroute and encode it the way
// we want right now": that's what the {bin,asm,c}dump_archive classes are 
// going to accomplish for us, but they need a bit of a leg up before they *can*:
// that's us, the *base class*, hooking into raw archive file I/O by providing
// these conveniently same-name-as-the-C-functions-that-do-the-actual-work
// virtual(=overridable) methods!

buf_fhandle_t *dump_archive::gup_io_open(const char *name, unsigned char *buf_start, unsigned long buf_size, int omode, gup_result *result)
{
	TRACE_ME();
	return ::gup_io_open(name, buf_start, buf_size, omode, result);
}

gup_result dump_archive::gup_io_close(buf_fhandle_t *file)
{
	TRACE_ME();
	return ::gup_io_close(file);
}
// Seek to a position in the file.
gup_result dump_archive::gup_io_seek(buf_fhandle_t *file, long offset, int seekmode, long *new_pos)
{
	TRACE_ME();
	return ::gup_io_seek(file, offset, seekmode, new_pos);
}
// Return the current position in the file.
gup_result dump_archive::gup_io_tell(buf_fhandle_t *file, long *fpos)
{
	TRACE_ME();
	return ::gup_io_tell(file, fpos);
}
// Write data to a file.
gup_result dump_archive::gup_io_write(buf_fhandle_t *file, const void *buffer, unsigned long count, unsigned long *real_count)
{
	TRACE_ME();
	return ::gup_io_write(file, buffer, count, real_count);
}
// Read data from a file.
gup_result dump_archive::gup_io_read(buf_fhandle_t *file, void *buffer, unsigned long count, unsigned long *real_count)
{
	TRACE_ME();
	return ::gup_io_read(file, buffer, count, real_count);
}
// Make sure there at least 'count' bytes free in the file buffer. If necessary the file buffer is flushed.
gup_result dump_archive::gup_io_write_announce(buf_fhandle_t *file, unsigned long count)
{
	TRACE_ME();
	return ::gup_io_write_announce(file, count);
}
// Fill the file buffer.
gup_result dump_archive::gup_io_fill(buf_fhandle_t *file)
{
	TRACE_ME();
	return ::gup_io_fill(file);
}
// Get a pointer to the current position in the file buffer and return the number of bytes that can be written into the buffer or can be read from the buffer.
uint8 *dump_archive::gup_io_get_current(buf_fhandle_t *file, unsigned long *bytes_left)
{
	TRACE_ME();
	return ::gup_io_get_current(file, bytes_left);
}
// Set the pointer in the file buffer to a new position.
void dump_archive::gup_io_set_current(buf_fhandle_t *file, uint8 *new_pos)
{
	TRACE_ME();
	return ::gup_io_set_current(file, new_pos);
}
// Set the current position in the file to the given value. Any bytes after this position that are in the buffer are discarded.
void dump_archive::gup_io_set_position(buf_fhandle_t *file, long position)
{
	TRACE_ME();
	return ::gup_io_set_position(file, position);
}

//===================================================================================
//===================================================================================
//===================================================================================

/*****************************************************************************
 *																			 *
 * Constructor and destructor.										 		 *
 * 																			 *
 *****************************************************************************/

bindump_archive::bindump_archive()
{
	TRACE_ME();
}

bindump_archive::~bindump_archive()
{
	TRACE_ME();
}



//===================================================================================
//===================================================================================
//===================================================================================

/*****************************************************************************
 *																			 *
 * Constructor and destructor.										 		 *
 * 																			 *
 *****************************************************************************/

cdump_archive::cdump_archive()
{
	TRACE_ME();
}

cdump_archive::~cdump_archive()
{
	TRACE_ME();
}



//===================================================================================
//===================================================================================
//===================================================================================

/*****************************************************************************
 *																			 *
 * Constructor and destructor.										 		 *
 * 																			 *
 *****************************************************************************/

asmdump_archive::asmdump_archive()
{
	TRACE_ME();
}

asmdump_archive::~asmdump_archive()
{
	TRACE_ME();
}

#endif // ENABLE_DUMP_OUTPUT_MODES


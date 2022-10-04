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
#include <stdint.h>
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

#define DEBUG_DUMP_HEADERS 0

/*****************************************************************************
 *																			 *
 * Constructor and destructor.										 		 *
 * 																			 *
 *****************************************************************************/

dump_archive::dump_archive() : cur_main_hdr(NULL)
{
	TRACE_ME();
}

dump_archive::~dump_archive()
{
	TRACE_ME();
	
	delete cur_main_hdr;
}

/*****************************************************************************
 *																			 *
 * Functions for writing an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result dump_archive::write_end_of_volume(int mv)
 *
 * Write end of archive.
 *
 * Parameters:
 *
 * mv		- 1 if the volume is part of a multiple volume archive and
 *				the volume is not the last volume of the archive.
 */

gup_result dump_archive::write_end_of_volume(int mv)
{
	TRACE_ME();

	gup_result result;
	long arc_len;
	unsigned long bytes_left;

	size_t hdr_len_est = 512;
	char *hdr_buf = new char[hdr_len_est];
	snprintf(hdr_buf, hdr_len_est, "\n\n/*\n\
	END OF DUMP\n\
*/\n\n");

	size_t hdr_len = strlen(hdr_buf);	
	if ((result = gup_io_write_announce(file, hdr_len)) == GUP_OK)
	{
		uint8 *p = gup_io_get_current(file, &bytes_left);
		
		memcpy(p, hdr_buf, hdr_len);
		p += hdr_len;
		gup_io_set_current(file, p);
	}

	delete[] hdr_buf;

	if ((result = gup_io_flush_header(file)) != GUP_OK)
		return result;

	/*
	 * Determine the length of the archive.
	 */

	if ((result = tell(arc_len)) != GUP_OK)
		return result;

	/*
	 * Update the main header.
	 */

	if (cur_main_hdr == NULL)
		return GUP_INTERNAL;

#if !DEBUG_DUMP_HEADERS
	if ((result = seek(0, SEEK_SET)) != GUP_OK)
		return result;
#endif

	/*
	 * Set the length of the archive.
	 */

	cur_main_hdr->arc_output_size = arc_len;

	{
		uint16 old_mv_mode;

		/*
		 * Trick to prevent write_main_header() from changing 'mv_bytes_left'
		 * This should be changed !!!
		 */

		old_mv_mode = st.pack_str.mv_mode;
		st.pack_str.mv_mode = 0;

		result = write_main_header(cur_main_hdr);

		st.pack_str.mv_mode = old_mv_mode;
	}

	return result;
//	return arj_archive::write_end_of_volume(mv);
}

/*
 * gup_result dump_archive::write_main_header(const mainheader *header)
 *
 * Write a main header to the archive.
 */

gup_result dump_archive::write_main_header(const mainheader *header)
{
	TRACE_ME();

	gup_result result;
	char *filename;
	const char *src;
	uint16 fspecpos = 0;
	unsigned long total_hdr_size, bytes_left;

	if (!opened || !rw)
		return GUP_INTERNAL;

	src = cur_volume->name();
	const char *comment = header->get_comment();
	if (!comment)
		comment = "";

	if (!cur_main_hdr)
		cur_main_hdr = new dump_mainheader(comment);

	filename = arj_conv_from_os_name(src, fspecpos, PATHSYM_FLAG);

		//header->host_os;		/* Host os. */
		//arj_conv_from_os_time(header->ctime);	/* Creation time. */
	uint32_t t = arj_conv_from_os_time(gup_time());	/* Modification time (now). */
		//header->arc_size;	/* Archive size. */
		fspecpos;		/* File spec position in file name. */

	size_t hdr_len_est = strlen(comment) + strlen(src) + 512;
	char *hdr_buf = new char[hdr_len_est];
	snprintf(hdr_buf, hdr_len_est, "/*\n\
	DUMP name: %s\n\
	DUMP filename: %s\n\
	comment: %s\n\
	creation time: %12u\n\
	archive size: %12lu\n\
*/\n\n", src, filename, comment, (unsigned int)t, (unsigned long)cur_main_hdr->arc_output_size);

	size_t hdr_len = strlen(hdr_buf);	
	if ((result = gup_io_write_announce(file, hdr_len)) == GUP_OK)
	{
		uint8 *p = gup_io_get_current(file, &bytes_left);
		
		memcpy(p, hdr_buf, hdr_len);
		p += hdr_len;
		gup_io_set_current(file, p);
	}

	delete[] filename;
	delete[] hdr_buf;

	return result;
//	return arj_archive::write_main_header(header);
}

gup_result dump_archive::write_file_header(const fileheader *header)
{
	TRACE_ME();

	gup_result result;
	char *name_ptr;
	unsigned long total_hdr_size, bytes_left;
   	const char *src;
   	uint16 fspec_pos;

	if (!opened || !rw)
		return GUP_INTERNAL;

	if (cur_main_hdr == NULL)
		return GUP_INTERNAL;

	if ((result = tell(header_pos)) != GUP_OK)	/* Store current position for
										   use by 'write_file_tailer'. */
		return result;

	src = header->get_filename();
	const char *comment = header->get_comment();
	if (!comment)
		comment = "";
	header->get_file_stat();

	name_ptr = arj_conv_from_os_name(src, fspec_pos, PATHSYM_FLAG);

		header->method;	/* Packing mode. */
		header->file_type;	/* File type. */

//		header->orig_time_stamp;	/* Time stamp. */
		header->compsize;	/* Compressed size. */
		header->origsize;	/* Original size. */
		header->file_crc;	/* File CRC. */
		fspec_pos;			/* File spec position in filename. */
//		header->orig_file_mode;	/* File attributes. */
//		header->host_data;	/* Host data. */

			header->offset;	/* Extended file position. */

	size_t hdr_len_est = strlen(comment) + strlen(src) + 512;
	char *hdr_buf = new char[hdr_len_est];
	TRACE_ME();
	snprintf(hdr_buf, hdr_len_est, "/*\n\
	FILE name: %s\n\
	FILE filename: %s\n\
	comment: %s\n\
	creation time:                 \n\
	filesize uncompressed: %12lu\n\
	filesize packed:       %12lu\n\
	CRC:                     0x%08lx\n\
*/\n\n", src, name_ptr, comment,
		(unsigned long)header->origsize,
		(unsigned long)header->compsize,
		(unsigned long)header->file_crc
);

	TRACE_ME();
	size_t hdr_len = strlen(hdr_buf);	
	if ((result = gup_io_write_announce(file, hdr_len)) == GUP_OK)
	{
		uint8 *p = gup_io_get_current(file, &bytes_left);
		
		memcpy(p, hdr_buf, hdr_len);
		p += hdr_len;
		gup_io_set_current(file, p);
	}

  	delete[] name_ptr;
	delete[] hdr_buf;

	// now make sure we've written our stuff to disk, so we can be assured
	// that any subsequent write/flush is actual packed data!
	if ((result = gup_io_flush_header(file)) != GUP_OK)
		return result;

	if ((result = tell(cur_main_hdr->current_file_pack_start_offset)) != GUP_OK)
		return result;
	
	return result;

	return GUP_OK;
//	return arj_archive::write_file_header(header);
}

gup_result dump_archive::write_file_trailer(const fileheader *header)
{
	TRACE_ME();
	long current_pos;
	gup_result result;
	unsigned long bytes_left;
	unsigned long binsize_read;

	if (cur_main_hdr == NULL)
		return GUP_INTERNAL;

	// now make sure we've written our stuff to disk, so we can be assured
	// that any subsequent write/flush is actual header rewriting!
	// All flushed data to disk right now is packed data!
	if ((result = gup_io_flush_packed_data(file)) != GUP_OK)
		return result;

	if ((result = tell(current_pos)) != GUP_OK)
		return result;

	unsigned long binsize = current_pos - cur_main_hdr->current_file_pack_start_offset;
	
	// go to start of binary packed chunk in file:
	if ((result = seek(cur_main_hdr->current_file_pack_start_offset, SEEK_SET)) != GUP_OK)
		return result;

	uint8_t *binbuf = new uint8_t[binsize];

	if ((result = ::gup_io_reload(file, binbuf, binsize, &binsize_read)) != GUP_OK)
		return result;

	if ((result = seek(cur_main_hdr->current_file_pack_start_offset, SEEK_SET)) != GUP_OK)
		return result;

printf("############### BINBUF READ: %lu, SIZE WANTED: %lu\n", binsize_read, binsize);

	// now we have the binary packed data in file/buffer.
	// What must be done next is fetch that binary data, copy it into
	// a temporary buffer and then rewind the output file to the start
	// of the packed data zone and rewrite the output, now using the desired
	// output encoding/format:
	char buf[4096];
	memset(buf, 'x', 4096);
	if ((result = gup_io_write_announce(file, 4096)) == GUP_OK)
	{
		uint8 *p = gup_io_get_current(file, &bytes_left);
		
		memcpy(p, buf, 4096);
		p += 4096;
		gup_io_set_current(file, p);
	}


	if ((result = tell(current_pos)) != GUP_OK)
		return result;

#if !DEBUG_DUMP_HEADERS
	if ((result = seek(header_pos, SEEK_SET)) != GUP_OK)
		return result;
#endif

	{
		uint16 old_mv_mode;

		/*
		 * Trick to prevent write_file_header() from changing 'mv_bytes_left'
		 * This should be changed !!!
		 */

		old_mv_mode = st.pack_str.mv_mode;
		st.pack_str.mv_mode = 0;

		result = write_file_header(header);

		st.pack_str.mv_mode = old_mv_mode;

		if (result != GUP_OK)
			return result;
	}

#if !DEBUG_DUMP_HEADERS
	return seek(current_pos, SEEK_SET);
#endif
	return GUP_OK;
//	return arj_archive::write_file_trailer(header);
}



gup_result dump_archive::gup_io_flush_header(buf_fhandle_t *file)
{
	TRACE_ME();
	return ::gup_io_flush(file);
}

gup_result dump_archive::gup_io_flush_packed_data(buf_fhandle_t *file)
{
	TRACE_ME();
	return ::gup_io_flush(file);
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


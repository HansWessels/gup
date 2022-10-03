/*
 * Archive manager.
 *
 *     Base archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:49:40 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 211 $
 * $Log$
 * Revision 1.7  2000/09/03 12:49:40  wout
 * Better multiple volume support. Added GZIP support.
 *
 * Revision 1.6  2000/07/30 14:55:18  wout
 * Use gup_io_set_position instead of seek to set the correct position
 * in the output archive after encode.
 *
 * Revision 1.5  1998/12/28 14:58:08  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.4  1998/03/26 19:59:23  klarenw
 * Updated to last version of the compression engine. Cleaner interface.
 *
 * Revision 1.3  1998/03/17 18:13:52  klarenw
 * Adapted to the new filebuffering scheme used by encode() and
 * decode().
 *
 * Revision 1.2  1998/01/03 19:24:41  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:47  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#include "gup.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif

#include "gup_err.h"
#include "compress.h"
#include "gup_io.h"
#include "options.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "archive.h"
#include "arj_hdr.h"
#include "arj_arc.h"
#include "gz_hdr.h"
#include "gz_arc.h"
#include "lha_hdr.h"
#include "lha_arc.h"
#include "dump_hdr.h"
#include "dump_arc.h"
#include "os.h"
#include "support.h"

/*
 * Typedefs.
 */

typedef struct
{
	int handle;							/* File handle of source file. */
	archive *arc;
	uint32 crc;
} READ_CRC_STRUCT;

typedef struct
{
	int handle;							/* File handle of destination file. */
	archive *arc;
	uint32 crc;
} WRITE_CRC_STRUCT;

/*****************************************************************************
 *																			 *
 * Member functions of the volume class.									 *
 * 																			 *
 *****************************************************************************/

volume::volume(const char *name, const char *tmp_name)
{
	volume_name = new char[strlen(name) + 1];
	strcpy(volume_name, name);

	if (tmp_name)
	{
		tmp_volume_name = new char[strlen(tmp_name) + 1];
		strcpy(tmp_volume_name, tmp_name);
	}
	else
		tmp_volume_name = NULL;

	next = NULL;
}

volume::~volume(void)
{
	delete[] volume_name;
	if (tmp_volume_name)
		delete[] tmp_volume_name;
}

/*****************************************************************************
 *																			 *
 * Member functions of the volume_list class.								 *
 * 																			 *
 *****************************************************************************/

volume_list::volume_list(void)
{
	first_ptr = NULL;
	cur_ptr = NULL;
	last_ptr = NULL;
}

volume_list::~volume_list(void)
{
	clear();
}

volume *volume_list::add(const char *name, const char *tmp_name)
{
	volume *item;

	item = new volume(name, tmp_name);
	if (first_ptr == NULL)
		first_ptr = item;
	if (last_ptr != NULL)
		last_ptr->next = item;
	last_ptr = item;

	return item;
}

void volume_list::clear(void)
{
	volume *ptr, *next;

	/*
	 * Delete all items in the list.
	 */

	ptr = first_ptr;

	while (ptr)
	{
		next = ptr->next;
		delete ptr;
		ptr = next;
	}

	first_ptr = cur_ptr = last_ptr = NULL;
}

volume *volume_list::first(void)
{
	/*
	 * Reset current pointer and return pointer to the first
	 * item in the list.
	 */

	cur_ptr = first_ptr;
	return first_ptr;
}

volume *volume_list::next(void)
{
	/*
	 * Return the next item in the list.
	 */

	if (cur_ptr != NULL)
	{
		cur_ptr = cur_ptr->next;
		return cur_ptr;
	}
	else
		return NULL;
}

/*****************************************************************************
 *																			 *
 * Utility functions for the archive class.									 *
 * 																			 *
 *****************************************************************************/

static gup_result buf_write_crc(long count, void *buffer, void *propagator)
{
	WRITE_CRC_STRUCT *com = (WRITE_CRC_STRUCT *) propagator;
	long result;

	com->crc = com->arc->calc_crc((uint8 *) buffer, count, com->crc);

	if ((result = write(com->handle, buffer, count)) != count)
	{
		if (result == -1)
			return gup_conv_err(errno);
		else
			return GUP_WRITE_ERROR;
	}
	else
		return GUP_OK;
}

static gup_result buf_write_nocrc(long count, void *buffer, void *propagator)
{
	WRITE_CRC_STRUCT *com = (WRITE_CRC_STRUCT *) propagator;
	long result;

	if ((result = write(com->handle, buffer, count)) != count)
	{
		if (result == -1)
			return gup_conv_err(errno);
		else
			return GUP_WRITE_ERROR;
	}
	else
		return GUP_OK;
}

static gup_result buf_write_crc_test(long count, void *buffer, void *propagator)
{
	WRITE_CRC_STRUCT *com = (WRITE_CRC_STRUCT *) propagator;

	com->crc = com->arc->calc_crc((uint8 *) buffer, count, com->crc);

	return GUP_OK;
}

static gup_result buf_write_nocrc_test(long count, void *buffer, void *propagator)
{
	(void) count;
	(void) buffer;
	(void) propagator;

	return GUP_OK;
}

static long buf_read_crc(long count, void *buf, void *propagator)
{
	READ_CRC_STRUCT *com = (READ_CRC_STRUCT *) propagator;

	long res = read(com->handle, buf, count);

	if(res > 0)
		com->crc = com->arc->calc_crc((uint8 *) buf, res, com->crc);

	return res;
}

static gup_result gup_buf_write_announce(long count, buf_fhandle_t *bw_buf, void *propagator)
{
	(void) propagator;

	return gup_io_write_announce(bw_buf, count);
}

static gup_result gup_buf_fill(buf_fhandle_t *br_buf, void *propagator)
{
	(void) propagator;

	return gup_io_fill(br_buf);
}

static void *gmalloc(unsigned long size, void *propagator)
{
	(void) propagator;
	return malloc(size);
}

static void gfree(void* ptr, void *propagator)
{
	(void) propagator;
	free(ptr);
}

/*****************************************************************************
 *																			 *
 * Member functions of the archive class.									 *
 * 																			 *
 *****************************************************************************/

archive::archive(void)
{
	archive_name = NULL;

	/*
	 * Set flags to defaults.
	 */

	opened = 0;
	rw = 0;
	handle_valid = 0;

	/*
	 * Set options to defaults.
	 */

	opt_repair = 0;
}

archive::~archive(void)
{
}

/*****************************************************************************
 *																			 *
 * Functions for writing an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result archive::create_archive(const char *name, OPTIONS *options,
 *									  unsigned long first_volume_size, int use_temp,
 *									  GUPMSG *msgfunc)
 *
 * Open an archive for writing.
 */

gup_result archive::create_archive(const char *name, OPTIONS *options,
								   unsigned long first_volume_size, int use_temp,
								   GUPMSG *msgfunc)
{
	gup_result result;
	char *tmp_name;

	if (opened)
		return GUP_INTERNAL;

	errno = 0;

	/*
	 * Duplicate the archive name.
	 */

	archive_name = new char[strlen(name) + 1];
	strcpy(archive_name, name);

	if (use_temp)
	{
		/*
		 * Generate a temporary filename for the first volume of the archive.
		 */

		tmp_name = new char[L_tmpnam + 1];

		if (tmpnam(tmp_name) == NULL)
		{
			delete[] tmp_name;
			delete[] archive_name;

			return GUP_TMPNAME;
		}

		cur_volume = volumes.add(name, tmp_name);
		delete[] tmp_name;				/* Free allocated space. */
	}
	else
		cur_volume = volumes.add(name, NULL);

	/*
	 * Initiate pack structure.
	 */

	st.pack_str.bufbase = NULL;

	st.pack_str.mode = (uint16) options->mode;	/* Packing mode. */
	st.pack_str.speed = 0;				/* Packing speed. */
	st.pack_str.jm = (uint16) options->jm;	/* jm mode. */
	st.pack_str.speed = (uint16) options->speed;	/* Speed. */
	st.pack_str.small_code = 0;

	st.pack_str.mv_mode = (uint16) options->mv_mode;
	st.pack_str.mv_next = 0;			/* Volume break flag. */
	st.pack_str.mv_bytes_left = 0;
	st.pack_str.mv_bits_left = 0;

	st.pack_str.print_progres = msgfunc->print_progress;
	st.pack_str.pp_propagator = msgfunc->pp_propagator;
	st.pack_str.init_message = msgfunc->init_message;
	st.pack_str.im_propagator = msgfunc->im_propagator;

	st.pack_str.buf_read_crc = buf_read_crc;

	st.pack_str.buf_write_announce = gup_buf_write_announce;
	st.pack_str.bw_propagator = (archive *) this;

	st.pack_str.gmalloc = gmalloc;
	st.pack_str.gm_propagator = NULL;
	st.pack_str.gfree = gfree;
	st.pack_str.gf_propagator = NULL;

	/*
	 * Set the size of the volume. The size of the main header and
	 * end of volume mark are not taken into acount.
	 * This should be changed !!!
	 */

	st.pack_str.mv_bytes_left = first_volume_size;

	if ((result = init_encode(&st.pack_str)) != GUP_OK)
	{
		volumes.clear();
		delete[] archive_name;

		return result;
	}

	/*
	 * Open the archive for writing.
	 */

	rw = 1;								/* Set read write flag before
										   calling 'open_volume'. */
	use_temp_files = use_temp;

	if ((result = open_volume()) != GUP_OK)
	{
		free_encode(&st.pack_str);		/* Free buffer. */
		volumes.clear();
		delete[] archive_name;

		return result;
	}

	opened = 1;							/* Archive is open. */

	return GUP_OK;
}

/*
 * gup_result archive::write_main_header(const char *comment)
 *
 * Create a main header and write it to the archive.
 */

gup_result archive::write_main_header(const char *comment)
{
	mainheader *header;
	gup_result result;

	if (!opened)
		return GUP_INTERNAL;

	header = init_main_header(comment);
	result = write_main_header(header);
	delete header;

	return result;
}

/*****************************************************************************
 *																			 *
 * Functions for reading an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result archive::open_archive(const char *name, OPTIONS *options,
 *									GUPMSG *msgfunc)
 *
 * Open an archive for reading.
 */

gup_result archive::open_archive(const char *name, OPTIONS *options,
								 GUPMSG *msgfunc)
{
	gup_result result;

	if (opened)
		return GUP_INTERNAL;

	errno = 0;

	/*
	 * Duplicate the archive name.
	 */

	archive_name = new char[strlen(name) + 1];
	strcpy(archive_name, name);

	cur_volume = volumes.add(name, NULL);

	/*
	 * Initiate pack structure.
	 */

	st.unpack_str.gmalloc = gmalloc;
	st.unpack_str.gm_propagator = NULL;
	st.unpack_str.gfree = gfree;
	st.unpack_str.gf_propagator = NULL;

	if ((result = init_decode(&st.unpack_str)) != GUP_OK)
	{
		volumes.clear();
		delete[] archive_name;

		return result;
	}

	if (options->no_crc_checking)
	{
		if (options->no_write_data)
			st.unpack_str.write_crc = buf_write_nocrc_test;
		else
			st.unpack_str.write_crc = buf_write_nocrc;
	}
	else
	{
		if (options->no_write_data)
			st.unpack_str.write_crc = buf_write_crc_test;
		else
			st.unpack_str.write_crc = buf_write_crc;
	}

	st.unpack_str.buf_fill = gup_buf_fill;
	st.unpack_str.br_propagator = (archive *) this;

	st.unpack_str.print_progres = msgfunc->print_progress;
	st.unpack_str.pp_propagator = msgfunc->pp_propagator;

	/*
	 * Copy options that are needed later.
	 */

	opt_repair = options->repair;
	opt_no_write = options->no_write_data;

	/*
	 * Open the archive for reading.
	 */

	rw = 0;								/* Set read write flag before
										   calling 'open_volume'. */
	if ((result = open_volume()) != GUP_OK)
	{
		free_decode(&st.unpack_str);	/* Free buffer. */
		volumes.clear();
		delete[] archive_name;

		return result;
	}

	opened = 1;							/* Archive is open. */

	return GUP_OK;
}

/*****************************************************************************
 *																			 *
 * General functions.														 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result archive::close_volume(void)
 *
 * Memeber function to close the current volume. Only the file descriptor
 * is closed. This function should only be used by derived classes.
 */

gup_result archive::close_volume(void)
{
	if (handle_valid)
	{
		gup_result result;

		result = gup_io_close(file);
		if (!rw)
			delete[] file_buffer;
		handle_valid = 0;

		return result;
	}
	else
		return GUP_INTERNAL;			/* Internal error. Volume not open. */
}

/*
 * gup_result archive::open_volume(void)
 *
 * Memeber function to open the current volume. Only the file descriptor
 * is created. This function should only be used by derived classes.
 */

gup_result archive::open_volume(void)
{
	if (!handle_valid)
	{
		if (rw)
		{
			gup_result result;

			if ((file = gup_io_open(cur_volume->name(), st.pack_str.buffer_start,
									st.pack_str.buffer_size, rw, &result)) == NULL)
				return result;
			st.pack_str.bw_buf = file;
		}
		else
		{
			const int buffer_size = 8 * 65536;
			gup_result result;

			file_buffer = new unsigned char[buffer_size];
			if ((file = gup_io_open(cur_volume->name(), file_buffer, buffer_size,
									rw, &result)) == NULL)
				return result;
			st.unpack_str.br_buf = file;
		}

		handle_valid = 1;

		return GUP_OK;
	}
	else
		return GUP_INTERNAL;						/* Volume already open. */
}

/*
 * gup_result archive::close_archive(int ok)
 *
 * Close archive. If the archive was opened for writing and 'ok' is 0,
 * delete all volumes in the archive. Otherwise close the last volume
 * and move if necessary the volumes to their final place.
 *
 * This function is also called by the destructor of the 'archive'
 * class if the archive is still opened (flag 'opened' is true) when
 * the destructor is called. The destructor calls this function with
 * 'ok' is 0 (i.e. delete all temporary files).
 */

gup_result archive::close_archive(int ok)
{
	gup_result result;

	if (opened)
	{
		if (rw)
		{
			volume *volume;

			/*
			 * Archive is open for writing.
			 */

			result = close_volume();

			free_encode(&st.pack_str);	/* Free buffer. */

			if (ok)
			{
				/*
				 * Archive is ok. Move the temporary volumes to their
				 * final place.
				 */

				if (use_temp_files)
				{
					gup_result tmp_result = GUP_OK;

					for (volume = volumes.first(); (volume != NULL) && (tmp_result == GUP_OK);
								volume = volumes.next())
						tmp_result = move_file(volume->tmp_name(), volume->name());

					if (result == GUP_OK)
						result = tmp_result;
				}
			}
			else
			{
				/*
				 * Archive is not ok. Delete all volumes.
				 */

				for (volume = volumes.first(); volume != NULL; volume = volumes.next())
					unlink(volume->tmp_name());
			}
		}
		else
		{
			/*
			 * Archive is open for reading.
			 */

			result = close_volume();
			free_decode(&st.unpack_str);	/* Free buffer. */
		}

		volumes.clear();
		delete[] archive_name;

		opened = 0;
	}
	else
		result = GUP_INTERNAL;

	return result;
}

/*****************************************************************************
 *																			 *
 * CRC functions.															 *
 * 																			 *
 *****************************************************************************/

/*
 * uint32 archive::post_proc_crc(uint32 crc)
 *
 * Post process the calculated CRC. This function is called by
 * archive::encode() and archive::decode() to perform any archive
 * type specific operations on the CRC calculated by the CRC
 * functions.
 *
 * Parameters:
 *
 * crc	- CRC calculated by the GUP CRC functions.
 *
 * Result: post processed CRC.
 */

uint32 archive::post_process_crc(uint32 crc)
{
	return crc;
}

/*****************************************************************************
 *																			 *
 * Multiple volume support.													 *
 * 																			 *
 *****************************************************************************/

unsigned long archive::get_mv_bytes_left(void)
{
	return 0;
}

/*
 * int archive::mv_break(void)
 *
 * This function should be called after encode() to check if the
 * file has been compressed completely or if a multiple volume
 * break has occured.
 *
 * Result: 0 if the file was compressed completely, 1 if a multiple
 *         volume break has occured.
 */

int archive::mv_break(void)
{
	return 0;
}

/*****************************************************************************
 *																			 *
 * File I/O functions.														 *
 * 																			 *
 *****************************************************************************/

gup_result archive::seek(long offset, int whence)
{
	if (handle_valid)
	{
		long dummy;

		return gup_io_seek(file, offset, whence, &dummy);
	}
	else
		return GUP_INTERNAL;			/* Internal error. Volume not open. */
}

gup_result archive::tell(long &offset)
{
	if (handle_valid)
		return gup_io_tell(file, &offset);
	else
		return GUP_INTERNAL;			/* Internal error. Volume not open. */
}

gup_result archive::read(void *buf, unsigned long len, unsigned long &real_len)
{
	if (handle_valid)
	{
		gup_result result;

		if ((result = gup_io_read(file, buf, len, &real_len)) != GUP_OK)
			return result;
		return (real_len == len) ? GUP_OK : GUP_READ_ERROR;
	}
	else
		return GUP_INTERNAL;			/* Internal error. Volume not open. */
}

gup_result archive::write(void *buf, unsigned long len, unsigned long &real_len)
{
	if (handle_valid)
	{
		gup_result result;

		if ((result = gup_io_write(file, buf, len, &real_len)) != GUP_OK)
			return result;
		return (real_len == len) ? GUP_OK : GUP_WRITE_ERROR;
	}
	else
		return GUP_INTERNAL;			/* Internal error. Volume not open. */
}

gup_result archive::getb(unsigned char &ch)
{
	unsigned long dummy;

	return read(&ch, 1, dummy);
}

/*****************************************************************************
 *																			 *
 * Compression and decompression functions.									 *
 * 																			 *
 *****************************************************************************/

gup_result archive::decode(fileheader *header, int outfile)
{
	gup_result result = GUP_OK;
	long start;
	WRITE_CRC_STRUCT wcs;

	wcs.crc = init_crc();
	wcs.handle = outfile;
	wcs.arc = (archive *) this;

	st.unpack_str.wc_propagator = &wcs;
	st.unpack_str.mode = (uint16) header->method;
	st.unpack_str.origsize = header->origsize;

	result = tell(start);

	if (result == GUP_OK)
	{
		if ((result = ::decode(&st.unpack_str)) == GUP_OK)
		{
			result = seek(start + header->compsize, SEEK_SET);

			/*
			 * Set the CRC calculated while decompressing.
			 */

			header->real_crc = post_process_crc(wcs.crc);
		}
	}

	return result;
}

/*
 * gup_result archive::encode(fileheader *header, int infile)
 *
 * Pack a (chunk) of a file.
 *
 * header->offset should be set to the correct value before calling
 * this function.
 */

gup_result archive::encode(fileheader *header, int infile)
{
	gup_result result;
	long start;
	READ_CRC_STRUCT rcs;

	rcs.crc = init_crc();
	rcs.handle = infile;
	rcs.arc = (archive *) this;

	st.pack_str.brc_propagator = &rcs;

	if ((result = tell(start)) == GUP_OK)
		result = ::encode(&st.pack_str);

	if (result == GUP_OK)
	{
		/*
		 * Check if the packed size is larger than the original size.
		 * If true, pack again in mode 0.
		 */

		if ((st.pack_str.packed_size > st.pack_str.bytes_packed) || (st.pack_str.bytes_packed == 0))
		{
			uint16 orig_method = st.pack_str.mode;

			st.pack_str.mode = STORE;

			if (st.pack_str.mv_mode)
				st.pack_str.mv_bytes_left += st.pack_str.bytes_packed;

			lseek(infile, header->offset, SEEK_SET);

			rcs.crc = init_crc();

			if ((result = seek(start, SEEK_SET)) == GUP_OK)
			{
				// TODO: init progress
				result = ::encode(&st.pack_str);
			}

			st.pack_str.mode = orig_method;
			header->method = STORE;
		}

		/*
		 * When creating a multiple volume archive, check if all remaining
		 * data of this file has been packed. If not true, recalculate the
		 * CRC of the part of the file that has been packed.
		 */

		if (st.pack_str.mv_mode && (result == GUP_OK) &&
			st.pack_str.mv_next)
		{
			lseek(infile, header->offset, SEEK_SET);
			rcs.crc = init_crc();

			if ((result = re_crc(st.pack_str.bytes_packed, &st.pack_str)) == GUP_OK)
				lseek(infile, header->offset + st.pack_str.bytes_packed, SEEK_SET);
		}

		header->compsize = st.pack_str.packed_size;
		header->origsize = st.pack_str.bytes_packed;
		header->totalsize += st.pack_str.bytes_packed;
		header->stat.length = header->totalsize;
		header->file_crc = header->real_crc = post_process_crc(rcs.crc);

		gup_io_set_position(file, start + header->compsize);
	}

	return result;
}

/*****************************************************************************
 *																			 *
 * Utility functions.														 *
 * 																			 *
 *****************************************************************************/

/*
 * archive *new_archive(archive_type type)
 *
 * Create a instance of the archive class.
 *
 * type	- type of archive to create an instance of.
 *
 * Result: pointer to instance or NULL if an error occured.
 */

archive *new_archive(archive_type type)
{
	switch(type)
	{
	case AT_ARJ:
		return new arj_archive;
	case AT_LHA:
		return new lha_archive;
	case AT_GZIP:
		return new gzip_archive;
	case AT_BINDUMP:
		return new bindump_archive;
	case AT_ASMDUMP:
		return new asmdump_archive;
	case AT_CDUMP:
		return new cdump_archive;
	default:
		return NULL;
	}
}

/*
 * archive_type get_arc_type(const char *filename)
 *
 * Determine what kind of archive the file with name 'filename' is.
 *
 * Parameters:
 *
 * filename	- name of the file.
 *
 * Result: type of the archive.
 */

archive_type get_arc_type(const char *filename)
{
	if (match_pattern(filename, "*.[aA][rR][jJ]"))
		return AT_ARJ;
	else if (match_pattern(filename, "*.[lL][zZ][hH]"))
		return AT_LHA;
	else if (match_pattern(filename, "*.[gG][zZ]"))
		return AT_GZIP;
	else if (match_pattern(filename, "*.[bB][iI][nN][dD][uU][mM][pP]"))
		return AT_BINDUMP;
	else if (match_pattern(filename, "*.[aA][sS][mM][dD][uU][mM][pP]"))
		return AT_ASMDUMP;
	else if (match_pattern(filename, "*.[cC][dD][uU][mM][pP]"))
		return AT_CDUMP;
	else
		return AT_UNKNOWN;				/* Default is ARJ. */
}

/*
 * Archive manager.
 *
 *     ARJ archive class.
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
#include "arj_hdr.h"
#include "arj_arc.h"
#include "crc.h"
#include "os.h"
#include "support.h"

/*
 * Defines.
 */

#define MAXSFX			25000L			/* Size of self-extracting prefix. */

#define HEADER_ID 		0xEA60U

#define HEADERSIZE_MAX	2610

#define MAX_HDR_SIZE	65535			/* Maximum size of a header. */

/*
 * Local variables.
 */

static uint32 *crc_table = NULL;
static int crc_use_cnt = 0;

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

/*
 * static gup_result get_crc_table(void)
 *
 * Return a pointer to the CRC table. If necessary allocate
 * and initiate the CRC table.
 *
 * Result: pointer to the CRC table.
 */

static gup_result get_crc_table(void)
{
	if (crc_use_cnt == 0)
	{
		uint32 poly = CRC32_POLY;
		if ((crc_table = (uint32 *) make_crc32_table(&poly)) == NULL)
			return GUP_NOMEM;
	}

	crc_use_cnt++;

	return GUP_OK;
}

/*
 * static void free_crc_table(void)
 *
 * Free the CRC table if it is not being used anymore.
 */

static void free_crc_table(void)
{
	crc_use_cnt--;
	if (crc_use_cnt == 0)
		free_crc32_table(crc_table);	/* Free CRC table. */
}

/*****************************************************************************
 *																			 *
 * Constructor and destructor.										 		 *
 * 																			 *
 *****************************************************************************/

arj_archive::arj_archive(void)
{
	cur_main_hdr = NULL;
	last_volume = 1;					/* Default archive is not multiple volume. */
}

arj_archive::~arj_archive(void)
{
	/*
	 * If the archive is still open, close it. If the archive was
	 * opened for writing, do not consider the archive to be valid.
	 */

	if (opened)
		close_archive(0);

	if (cur_main_hdr != NULL)
		delete cur_main_hdr;
}

/*****************************************************************************
 *																			 *
 * Functions for writing an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result arj_archive::create_archive(const char *name, OPTIONS *options,
 *										  unsigned long first_volume_size, int use_temp,
 *										  GUPMSG *msgfunc)
 *
 * Open an archive for writing.
 */

gup_result arj_archive::create_archive(const char *name, OPTIONS *options,
									   unsigned long first_volume_size, int use_temp,
									   GUPMSG *msgfunc)
{
	gup_result result;

	/*
	 * Allocate CRC table.
	 */

	if ((result = get_crc_table()) != GUP_OK)
		return result;

	arj_suff = 0;						/* Reset suffix counter for mul. vol. */

	/*
	 * Open the archive.
	 */

	if ((result = archive::create_archive(name, options, first_volume_size, use_temp, msgfunc)) != GUP_OK)
		free_crc_table();

	return result;
}

/*
 * gup_result arj_archive::create_next_volume(unsigned long volume_size)
 *
 * Create the next volume of an archive. The following is done:
 *
 * - close the current volume
 * - generate the filename for the volume
 * - generate a temporary filename for the volume
 * - create a file descriptor for the volume
 * - write the main header to the volume
 */

gup_result arj_archive::create_next_volume(unsigned long volume_size)
{
	gup_result result;
	char *new_volume, *suffix_ptr, *tmp_name;

	/*
	 * Duplicate the archive name.
	 */

	new_volume = new char[strlen(cur_volume->name()) + 8];
	strcpy(new_volume, cur_volume->name());

	/*
	 * Change the suffix.
	 */

	arj_suff = get_arj_suffix_cnt(new_volume, &suffix_ptr);
	sprintf(suffix_ptr, "%02d", arj_suff);

	if (use_temp_files)
	{
		/*
		 * Generate a temporary filename for the volume.
		 */

		tmp_name = new char[L_tmpnam + 1];

		if (tmpnam(tmp_name) == NULL)
		{
			delete[] tmp_name;
			delete[] new_volume;
			return GUP_TMPNAME;
		}

		cur_volume = volumes.add(new_volume, tmp_name);
		delete[] tmp_name;				/* Free allocated space. */
	}
	else
		cur_volume = volumes.add(new_volume, NULL);

	delete[] new_volume;

	if ((result = open_volume()) != GUP_OK)
		return result;

	/*
	 * Set the size of the volume. The size of the main header and
	 * end of volume mark are not taken into acount.
	 * This should be changed !!!
	 */

	st.pack_str.mv_next = 0;			/* Volume break flag. */
	st.pack_str.mv_bytes_left = volume_size;	/* Size of this volume. */

	return archive::write_main_header((const char *) NULL);
}

/*
 * gup_result arj_archive::write_end_of_volume(int mv)
 *
 * Write end of archive.
 *
 * Parameters:
 *
 * mv		- 1 if the volume is part of a multiple volume archive and
 *				the volume is not the last volume of the archive.
 */

gup_result arj_archive::write_end_of_volume(int mv)
{
	uint8 *ptr;
	long arc_len;
	gup_result result;
	unsigned long bytes_left;

	/*
	 * Write the end of archive marker to the archive.
	 */

	if ((result = gup_io_write_announce(file, 4)) != GUP_OK)
		return result;

	ptr = gup_io_get_current(file, &bytes_left);

	put_word_i(ptr, HEADER_ID);
	put_word_i(ptr, 0);

	gup_io_set_current(file, ptr);

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

	if ((result = seek(0, SEEK_SET)) != GUP_OK)
		return result;

	if (mv)
	{
		/*
		 * Set the multiple volume flag in the main header of the
		 * archive.
		 */

		cur_main_hdr->flags |= VOLUME_FLAG;
	}

	/*
	 * Set the length of the archive.
	 */

	cur_main_hdr->arc_size = arc_len;

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
}

/*
 * gup_result arj_archive::write_main_header(const mainheader *header)
 *
 * Write a main header to the archive.
 */

gup_result arj_archive::write_main_header(const mainheader *header)
{
	const arj_mainheader *main_hdr;
	int new_allocated = FALSE;
	gup_result result;
	char *filename;
	const char *comment, *src;
	uint16 fspecpos;
	unsigned long total_hdr_size, bytes_left;

	if (!opened || !rw)
		return GUP_INTERNAL;

	switch(header->get_mhdr_type())
	{
	case MHDR_ARJ:						/* Header is already an ARJ main header. */
		main_hdr = (const arj_mainheader *) header;
		break;
	default:							/* Other main header, create an ARJ main header. */
		main_hdr = (const arj_mainheader *) init_main_header(header->get_comment());
		new_allocated = TRUE;
		break;
	}

	/*
	 * If 'cur_main_hdr' is NULL, it is the first time this function
	 * is called for the current archive. Make a copy of the main header.
	 */

	if (cur_main_hdr == NULL)
		cur_main_hdr = new arj_mainheader(*main_hdr);

	src = main_hdr->get_filename(&fspecpos);
	filename = arj_conv_from_os_name(src, fspecpos, main_hdr->flags & PATHSYM_FLAG);

	total_hdr_size = 4 + 30 + strlen(filename) + 1;
	comment = main_hdr->get_comment();
	total_hdr_size += (comment != NULL) ? strlen(comment) + 1 : 1;
	total_hdr_size += 6;				/* Header CRC and first extended header size. */

	if ((result = gup_io_write_announce(file, total_hdr_size)) == GUP_OK)
	{
		uint8 *p, *start;
		unsigned long crc, len;

		p = start = gup_io_get_current(file, &bytes_left);

		put_word_i(p, HEADER_ID);		/* Write header ID. */
		put_word_i(p, 0);				/* Write basic header size. */
		*p++ = 30;						/* First header size. */
		*p++ = main_hdr->arj_nbr;		/* Archiver version number. */
		*p++ = main_hdr->arj_x_nbr;		/* Minimum archiver version to depack. */
		*p++ = main_hdr->host_os;		/* Host os. */
		*p++ = main_hdr->flags;			/* ARJ flags. */
		*p++ = main_hdr->arj_sec_nbr;	/* Security version. */
		*p++ = COMMENT_TYPE;			/* File type. */
		*p++ = 0;						/* Dummy. */
		put_long_i(p, arj_conv_from_os_time(main_hdr->ctime));	/* Creation time. */
		put_long_i(p, arj_conv_from_os_time(gup_time()));	/* Modification time (now). */
		put_long_i(p, main_hdr->arc_size);	/* Archive size. */
		put_long_i(p, 0L);				/* Security enveloppe file position. */
		put_word_i(p, fspecpos);		/* File spec position in file name. */
		put_word_i(p, (uint16) main_hdr->arj_secenv_len);	/* Length in bytes of security enveloppe data. */
		put_word_i(p, 0);				/* Not used. */

		/*
		 * Put filename.
		 */

		src = filename;
		while(*src)
			*p++ = *src++;
		*p++ = 0;

		/*
		 * Put comment.
		 */

		if ((comment = main_hdr->get_comment()) != NULL)
		{
			while (*comment)
				*p++ = *comment++;
		}

		*p++ = 0;

		/*
		 * Calculate and write CRC.
		 */

		len = p - start - 4;

		if (len <= MAX_HDR_SIZE)
		{
			crc = ~crc32(start + 4, len, (uint32) -1, crc_table);
			put_long_i(p, crc);

			put_word(start + 2, len);

			put_word_i(p, 0);			/* First extended header size. */

   			gup_io_set_current(file, p);

			/*
			 * If in multiple volume mode, update the number of bytes left
			 * in the volume. Here also space needed at the end of the archive
			 * should be substracted.
			 */

			if (st.pack_str.mv_mode)
			{
				st.pack_str.mv_bytes_left -= (p - start);	/* Size of main header. */
				st.pack_str.mv_bytes_left -= 4;	/* Size of end of volume marker. */
			}
		}
		else
			result = GUP_HDR_SIZE_OVF;
	}

	delete[] filename;

	if (new_allocated)
		delete (arj_mainheader *) main_hdr;

	return result;
}

gup_result arj_archive::write_file_header(const fileheader *header)
{
	const arj_fileheader *file_hdr;
	int new_allocated = FALSE;
	gup_result result;
	char *name_ptr;
	unsigned long total_hdr_size, bytes_left;
   	const char *src;
   	uint16 fspec_pos;

	if (!opened || !rw)
		return GUP_INTERNAL;

	if ((result = tell(header_pos)) != GUP_OK)	/* Store current position for
										   use by 'write_file_tailer'. */
		return result;

	switch(header->get_fhdr_type())
	{
	case FHDR_ARJ:						/* Header is already an ARJ file header. */
		file_hdr = (const arj_fileheader *) header;
		break;
	default:							/* Other file header, create an ARJ main header. */
		file_hdr = (const arj_fileheader *) init_file_header(header->get_filename(),
						header->get_comment(), header->get_file_stat());
		new_allocated = TRUE;
		break;
	}

	src = file_hdr->get_filename(fspec_pos);
	name_ptr = arj_conv_from_os_name(src, fspec_pos, file_hdr->flags & PATHSYM_FLAG);

	total_hdr_size = file_hdr->get_header_len();

	if ((result = gup_io_write_announce(file, total_hdr_size)) == GUP_OK)
	{
		uint8 *p, *start;
		unsigned long crc, len;

		p = start = gup_io_get_current(file, &bytes_left);

		put_word_i(p, HEADER_ID);		/* Write header ID. */
		put_word_i(p, 0);				/* Write basic header size. */

		if (file_hdr->flags & EXTFILE_FLAG)
			*p++ = 34;				   	/* First header size includes extended file position. */
		else
			*p++ = 30;					/* First header size. */

		*p++ = file_hdr->arj_nbr;	   	/* Archiver version number. */
		*p++ = file_hdr->arj_x_nbr;		/* Minimum archiver version to depack. */
		*p++ = (uint8) file_hdr->host_os;	/* Host os. */
		*p++ = file_hdr->flags;		   	/* ARJ flags. */
		*p++ = (uint8) file_hdr->method;	/* Packing mode. */
		*p++ = (uint8) file_hdr->file_type;	/* File type. */
		*p++ = 0xE3;				   	/* Reserved (extra garble addition). */

		put_long_i(p, file_hdr->orig_time_stamp);	/* Time stamp. */
		put_long_i(p, file_hdr->compsize);	/* Compressed size. */
		put_long_i(p, file_hdr->origsize);	/* Original size. */
		put_long_i(p, file_hdr->file_crc);	/* File CRC. */
		put_word_i(p, fspec_pos);			/* File spec position in filename. */
		put_word_i(p, (uint16) file_hdr->orig_file_mode);	/* File attributes. */
		put_word_i(p, file_hdr->host_data);	/* Host data. */

		if (file_hdr->flags & EXTFILE_FLAG)
			put_long_i(p, file_hdr->offset);	/* Extended file position. */

		/*
		 * Put filename.
		 */

		src = name_ptr;
		while(*src)
			*p++ = *src++;
		*p++ = 0;

		/*
		 * Put comment.
		 */

		src = file_hdr->get_comment();

		if (src != NULL)
		{
			while(*src)
				*p++ = *src++;
		}

		*p++ = 0;

		/*
		 * Calculate and write CRC.
		 */

		len = p - start - 4;

		if (len <= MAX_HDR_SIZE)
		{
			crc = ~crc32(start + 4, len, (uint32) -1, crc_table);
			put_long_i(p, crc);

			put_word(start + 2, len);	/* Header length. */

			put_word_i(p, 0);			/* First extended header size. */

   			gup_io_set_current(file, p);

			/*
			 * If in multiple volume mode, update the number of bytes left
			 * in the volume. The length of a file trailer should also
			 * be substracted here.
			 */

			if (st.pack_str.mv_mode)
				st.pack_str.mv_bytes_left -= (p - start);
		}
		else
			result = GUP_HDR_SIZE_OVF;
	}

   	delete[] name_ptr;

	if (new_allocated)
		delete (arj_fileheader *) file_hdr;

	return result;
}

gup_result arj_archive::write_file_trailer(const fileheader *header)
{
	long current_pos;
	gup_result result;

	if ((result = tell(current_pos)) != GUP_OK)
		return result;

	if ((result = seek(header_pos, SEEK_SET)) != GUP_OK)
		return result;

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

	return seek(current_pos, SEEK_SET);
}

/*****************************************************************************
 *																			 *
 * Functions for reading an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result arj_archive::open_archive(const char *name, OPTIONS *options,
 *										GUPMSG *msgfunc)
 *
 * Open an archive for reading.
 */

gup_result arj_archive::open_archive(const char *name, OPTIONS *options,
									 GUPMSG *msgfunc)
{
	gup_result result;

	/*
	 * Allocate CRC table.
	 */

	if ((result = get_crc_table()) != GUP_OK)
		return result;

	arj_suff = 0;						/* Reset suffix counter for mul. vol. */

	/*
	 * Open the archive.
	 */

	if ((result = archive::open_archive(name, options, msgfunc)) != GUP_OK)
		free_crc_table();

	return result;
}

/*
 * gup_result arj_archive::open_next_volume(void)
 *
 * Open the next volume of an archive. The following is done:
 *
 * - generate the filename for the volume
 * - create a file descriptor for the volume
 * - read the main header to the volume
 */

gup_result arj_archive::open_next_volume(void)
{
	gup_result result;
	char *new_volume, *suffix_ptr;
	mainheader *mainheader;

	/*
	 * Duplicate the archive name.
	 */

	new_volume = new char[strlen(cur_volume->name()) + 8];
	strcpy(new_volume, cur_volume->name());

	/*
	 * Change the suffix.
	 */

	arj_suff = get_arj_suffix_cnt(new_volume, &suffix_ptr);
	sprintf(suffix_ptr, "%02d", arj_suff);

	cur_volume = volumes.add(new_volume, NULL);
	delete[] new_volume;

	if ((result = open_volume()) != GUP_OK)
		return result;

	if ((mainheader = read_main_header(result)) == NULL)
		return result;

	delete mainheader;

	return result;
}

/*
 * gup_result arj_archive::find_header(int first, uint16 &arj_header_id,
 *									   uint16 &arj_header_size)
 *
 * Find a header in the ARJ file.
 *
 * first			- first header in file flag.
 * arj_header_id	- contains on return the ARJ header ID.
 * arj_header_size	- contains on return the total length of the header.
 *
 * Result codes:
 * GUP_OK				- no error
 * GUP_EOF				- if end of file reached
 * GUP_NO_ARCHIVE		- if nothing found which looks like a valid header
 * GUP_BROKEN			- if the archive might be broken
 * GUP_HDR_CRC_FAULT	- checksum error in header, hdr_type is valid.
 * GUP_HDR_POSITION		- header not found at expected position.
 */

gup_result arj_archive::find_header(int first, uint16 &arj_header_id,
									uint16 &arj_header_size)
{
	uint8 *buf_ptr;
	unsigned long bytes_left;
	long arcpos, startpos;
	gup_result result;

	if (opt_repair)
		first = FALSE;					/* Don't care about ARJ_SFX length! */

	/*
	 * Get current position and bytes left in buffer.
	 */

	buf_ptr = gup_io_get_current(file, &bytes_left);

	if ((result = tell(arcpos)) != GUP_OK)
		return result;
	startpos = arcpos;

	while ((!first) || (arcpos < MAXSFX))
	{
		int found = FALSE;
		unsigned long hdr_length;

		/*
		 * Scan for header ID.
		 */

		hdr_length = 4;					/* Length of the header ID and the header length. */

		do
		{
			/*
			 * Make sure the buffer contains sufficient bytes
			 */

			if (bytes_left < hdr_length)
			{
				if ((result = gup_io_fill(file)) != GUP_OK)
					return result;

				buf_ptr = gup_io_get_current(file, &bytes_left);
			}

			if (bytes_left >= hdr_length)
			{
				/*
				 * Check if header found. If true, stop the loop.
				 */

				if (get_word(buf_ptr) == HEADER_ID)
				{
					arj_header_id = HEADER_ID;
					found = TRUE;
				}
				else
				{
					if (first && (arcpos >= MAXSFX))
						return GUP_NO_ARCHIVE;

					buf_ptr++;
					bytes_left--;
					arcpos++;

					gup_io_set_current(file, buf_ptr);	/* Set new file position. */
				}
			}
			else
			{
				/*
				 * End of archive found. If first is true, we were looking
				 * for the main header, but nothing found. Return
				 * GUP_NO_ARCHIVE in this case. Otherwise return GUP_EOF.
				 */

				return (!first) ? GUP_EOF : GUP_NO_ARCHIVE;
			}
		} while (!found);

		/*
		 * Header ID found. Read the length of the header.
		 */

		arj_header_size = get_word(buf_ptr + 2);

		/*
		 * following has VERY strict check since 11/12/93: UNARJ crashed on
		 * some non-ARJ files
		 */

		if ((arj_header_size <= HEADERSIZE_MAX) &&
		   	((arj_header_size > 0) || (!first && (arj_header_size == 0))))
		{
			uint32 real_crc;
			uint32 crc;

			/*
			 * Read header and CRC. Return error if the CRC is not correct.
			 */

			if (arj_header_size)
			{
				hdr_length += arj_header_size + 6;	/* Header size + CRC + extended header size. */

				if (bytes_left < hdr_length)
				{
					if ((result = gup_io_fill(file)) != GUP_OK)
						return result;

					buf_ptr = gup_io_get_current(file, &bytes_left);
				}

				if (bytes_left >= hdr_length)
				{
					int crc_error, i, next_hdr_size = 0;

					crc = ~crc32(buf_ptr + 4, arj_header_size, (uint32) -1, crc_table);
					real_crc = get_longword(buf_ptr + 4 + arj_header_size);

					crc_error = (crc != real_crc) ? TRUE : FALSE;

					/*
					 * Check if the total header length does not exceed
					 * the maximum length of this implementation and if
					 * the entire header can be read from the archive.
					 */

					i = arj_header_size + 8;

					while ((hdr_length <= HEADERSIZE_MAX) &&
						   (hdr_length <= bytes_left) &&
						   ((next_hdr_size = get_word(buf_ptr + i)) != 0))
					{
						hdr_length += next_hdr_size + 6;	/* Add length of CRC and next header length. */
						i += next_hdr_size + 6;

						if (bytes_left < hdr_length)
						{
							if ((result = gup_io_fill(file)) != GUP_OK)
								return result;

							buf_ptr = gup_io_get_current(file, &bytes_left);
						}
						/*
						 * !!! check CRC of extended headers.
						 */
					}

					if ((next_hdr_size == 0) &&
						(hdr_length <= HEADERSIZE_MAX) &&
						(hdr_length <= bytes_left))
					{
						/*
						 * Valid header found. If found at the correct
						 * position return GUP_OK. If opt_repair is set and
						 * the position is not correct, return
						 * GUP_HDR_POSITION, otherwise return GUP_BROKEN.
						 */

						if (crc_error)
							return GUP_HDR_CRC_FAULT;

						return (!first && (arcpos != startpos)) ?
							    ((opt_repair) ? GUP_HDR_POSITION : GUP_BROKEN) :
								GUP_OK;
					}
				}
			}
			else
				return (!first && (arcpos != startpos)) ?
					((opt_repair) ? GUP_HDR_POSITION : GUP_BROKEN) : GUP_OK;
		}

		buf_ptr++;
		bytes_left--;
		arcpos++;

		gup_io_set_current(file, buf_ptr);	/* Set new file position. */
	}

	return GUP_NO_ARCHIVE;
}

/*
 * mainheader *arj_archive::read_main_header(gup_result &result)
 *
 * Read the main header from the archive. Note that even if there
 * is an error (result != GUP_OK), the result does not have to be
 * NULL. This is the case if it might be possible to extract some
 * usefull information from the fileheader (like the file name).
 *
 * Result codes:
 *
 * GUP_OK				- no error
 * GUP_EOF				- unexpected end of file
 * GUP_HDR_CRC_FAULT	- CRC error in header
 * GUP_NO_ACRHIVE		- file is not an ARJ archive
 * GUP_BROKEN			- archive is broken, but it might be possible
 * 						  to recover data from the archive
 */

mainheader *arj_archive::read_main_header(gup_result &result)
{
	uint16 arj_header_id;
	uint16 arj_header_size;

	result = find_header(TRUE, arj_header_id, arj_header_size);

	if ((result == GUP_OK) && (arj_header_size == 0))
	{
		result = GUP_BROKEN;
		return NULL;
	}

	if ((result == GUP_OK) || ((result == GUP_HDR_CRC_FAULT) &&
							   opt_repair && (arj_header_size > 0) && 
							   (arj_header_size <= HEADERSIZE_MAX)))
	{
		arj_mainheader *main_hdr;
		uint8 *buf_ptr, *ptr;
		uint16 fspec_pos;
		int first_hdr_size;
		uint16 extheadersize;
		unsigned long bytes_left;

		/*
		 * No error or CRC error. Create a arj_mainheader object using
		 * the data read from file.
		 */

		main_hdr = new arj_mainheader(NULL);

   		buf_ptr = gup_io_get_current(file, &bytes_left);
		ptr = buf_ptr + 4;

		first_hdr_size = get_byte_i(ptr);	/* First header size. */
		main_hdr->arj_nbr = get_byte_i(ptr);	/* Archiver version number. */
		main_hdr->arj_x_nbr = get_byte_i(ptr);	/* Minimum archiver version to depack. */
		main_hdr->host_os = get_byte_i(ptr);	/* Host OS. */
		main_hdr->flags = get_byte_i(ptr);	/* Archive flags. */

		main_hdr->arj_sec_nbr = get_byte_i(ptr);	/* Security version. */
		if ((get_byte_i(ptr) != COMMENT_TYPE) && (result == GUP_OK))
			result = GUP_BROKEN;		/* File type (should be COMMENT_TYPE). */
		(void) get_byte_i(ptr);			/* Dummy. */

		main_hdr->ctime = arj_conv_to_os_time(get_longword_i(ptr));
		main_hdr->mtime = arj_conv_to_os_time(get_longword_i(ptr));
		main_hdr->arc_size = get_longword_i(ptr);
		main_hdr->arj_secenv_fpos = get_longword_i(ptr);	/* Security envelope filepos. */
		fspec_pos = get_word_i(ptr);	/* Filespec position in filename. */
		main_hdr->arj_secenv_len = get_word_i(ptr);	/* Security envelope filepos. */
		(void) get_word_i(ptr);			/* Host data. */

		if (result != GUP_HDR_CRC_FAULT)
		{
			char *os_name;

			/*
			 * Copy filename.
			 */

			ptr = buf_ptr + 4 + first_hdr_size;

#if ((OS == OS_ATARI) || (OS == OS_UNIX))
			if (main_hdr->host_os == OS_MSDOS)
				strlwr((char *) ptr);
#elif ((OS != OS_MSDOS) && (OS != OS_WIN32))
#error "This OS is not supported."
#endif

			os_name = arj_conv_to_os_name((char *) ptr, fspec_pos, main_hdr->flags & PATHSYM_FLAG);
			main_hdr->set_filename(os_name, fspec_pos);
			delete[] os_name;
			ptr += strlen((const char *) ptr) + 1;

			/*
			 * Copy comment.
			 */

			main_hdr->set_comment((const char *) ptr);
			ptr += strlen((const char *) ptr) + 1;

			last_volume = (main_hdr->flags & VOLUME_FLAG) ? 0 : 1;
		}

		ptr = buf_ptr + 4 + arj_header_size + 4;

		/*
		 * Skip extended headers.
		 */

		extheadersize = get_word_i(ptr);

		while (extheadersize != 0)
		{
			ptr += extheadersize + 4;	/* Skip extended header and CRC. */
			extheadersize = get_word_i(ptr);
		}

		gup_io_set_current(file, ptr);

		return (mainheader *) main_hdr;
	}
	else
		return NULL;
}

/*
 * fileheader *arj_archive::read_file_header(gup_result &result)
 *
 * Read the file header from the archive. Note that even if there
 * is an error (result != GUP_OK), the result does not have to be
 * NULL. This is the case if it might be possible to extract some
 * usefull information from the fileheader (like the file name).
 *
 * Result codes:
 *
 * GUP_OK				- no error
 * GUP_EOF				- unexpected end of file
 * GUP_HDR_CRC_FAULT	- CRC error in header
 * GUP_NO_ACRHIVE		- file is not an ARJ archive
 * GUP_BROKEN			- archive is broken, but it might be possible
 * 						  to recover data from the archive
 * GUP_HDR_POSITION		- header not found at expected position. The
 *						  header data is valid.
 * GUP_END_VOLUME		- end of archive reached
 * GUP_END_ARCHIVE		- end of volume reached
 */

fileheader *arj_archive::read_file_header(gup_result &result)
{
	uint16 arj_header_id;
	uint16 arj_header_size;

	result = find_header(FALSE, arj_header_id, arj_header_size);

	if (arj_header_size == 0)
	{
		result = (last_volume) ? GUP_END_ARCHIVE : GUP_END_VOLUME;
		return NULL;
	}

	if ((result == GUP_OK) || ((result == GUP_HDR_POSITION) && opt_repair) ||
		((result == GUP_HDR_CRC_FAULT) && opt_repair && (arj_header_size > 0) &&
		(arj_header_size <= HEADERSIZE_MAX)))
	{
		arj_fileheader *file_hdr;
		uint16 fspec_pos;
		int first_hdr_size;
		uint16 extheadersize;
		uint8 *buf_ptr, *ptr;
		unsigned long bytes_left;

		/*
		 * No error or CRC error. Create a arj_fileheader object using
		 * the data read from file.
		 */

		file_hdr = new arj_fileheader(NULL, NULL);

   		buf_ptr = gup_io_get_current(file, &bytes_left);
		ptr = buf_ptr + 4;

		first_hdr_size = get_byte_i(ptr);	/* First header size. */
		file_hdr->arj_nbr = get_byte_i(ptr);	/* Archiver version number. */
		file_hdr->arj_x_nbr = get_byte_i(ptr);	/* Minimum archiver version to depack. */
		file_hdr->host_os = (int) get_byte_i(ptr);	/* Host OS. */
		file_hdr->flags = get_byte_i(ptr);	/* Archive flags. */

		file_hdr->method = get_byte_i(ptr);	/* Packing method. */
		file_hdr->file_type = (ftype) get_byte_i(ptr);	/* File type. */
		(void) get_byte_i(ptr);			/* Dummy. */

		file_hdr->orig_time_stamp = get_longword_i(ptr);
		file_hdr->stat.mtime = arj_conv_to_os_time(file_hdr->orig_time_stamp);
		file_hdr->compsize = get_longword_i(ptr);
		file_hdr->origsize = file_hdr->stat.length = get_longword_i(ptr);
		file_hdr->file_crc = get_longword_i(ptr);
		fspec_pos = get_word_i(ptr);
		file_hdr->orig_file_mode = get_word_i(ptr);
		file_hdr->stat.file_mode = mode_dos_to_os((uint8) file_hdr->orig_file_mode);
		file_hdr->host_data = get_word_i(ptr);

		if (file_hdr->flags & EXTFILE_FLAG)
			file_hdr->offset = get_longword_i(ptr);
		else
			file_hdr->offset = 0;

#if ((OS == OS_UNIX) || (OS == OS_WIN32))
		file_hdr->stat.atime = file_hdr->stat.ctime = file_hdr->stat.mtime;
#elif (OS != OS_MSDOS)
#error "This OS is not supported."
#endif

		if (result != GUP_HDR_CRC_FAULT)
		{
			char *os_name;

			/*
			 * Copy filename.
			 */

			ptr = buf_ptr + 4 + first_hdr_size;

#if ((OS == OS_ATARI) || (OS == OS_UNIX))
			if (file_hdr->host_os == OS_MSDOS)
				strlwr((char *) ptr);
#elif ((OS != OS_MSDOS) && (OS != OS_WIN32))
#error "This OS is not supported."
#endif

			os_name = arj_conv_to_os_name((char *) ptr, fspec_pos, file_hdr->flags & PATHSYM_FLAG);
			file_hdr->set_filename(os_name, fspec_pos);
			delete[] os_name;
			ptr += strlen((const char *) ptr) + 1;

			/*
			 * Copy comment.
			 */

			file_hdr->set_comment((const char *) ptr);
			ptr += strlen((const char *) ptr) + 1;
		}

		ptr = buf_ptr + 4 + arj_header_size + 4;

		/*
		 * Skip extended headers.
		 */

		extheadersize = get_word_i(ptr);

		while (extheadersize != 0)
		{
			ptr += extheadersize + 4;	/* Skip extended header and CRC. */
			extheadersize = get_word_i(ptr);
		}

		gup_io_set_current(file, ptr);

		return (fileheader *) file_hdr;
	}
	else
		return NULL;
}

gup_result arj_archive::read_file_trailer(fileheader *header)
{
	(void) header;

	return GUP_OK;
}

/*****************************************************************************
 *																			 *
 * General functions.														 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result arj_archive::close_archive(int ok)
 *
 * Close archive. If the archive was opened for writing and 'ok' is 0,
 * delete all volumes in the archive. Otherwise close the last volume
 * and move if necessary the volumes to their final place.
 *
 * This function is also called by the destructor of the 'ARCHIVE'
 * class if the archive is still opened (flag 'opened' is true) when
 * the destructor is called. The destructor calls this function with
 * 'ok' is 0 (i.e. delete all temporary files).
 */

gup_result arj_archive::close_archive(int ok)
{
	gup_result result = GUP_OK, tmp_result;

	if (opened)
	{
		if (rw && ok)
			result = write_end_of_volume(0);

		tmp_result = archive::close_archive(ok);

		if (result == GUP_OK)
			result = tmp_result;

		if (rw)
		{
			if (cur_main_hdr != NULL)
				delete cur_main_hdr;
			cur_main_hdr = NULL;
		}

		free_crc_table();
	}
	else
		result = GUP_INTERNAL;

	return result;
}

/*
 * gup_result arj_archive::close_curr_volume(void)
 *
 * Close the current volume.
 *
 * Only call this function if the current volume is not the last volume
 * of the archive. If the volume is the last, use 'close_archive'.
 */

gup_result arj_archive::close_curr_volume(void)
{
	gup_result result;

	if (opened)
	{
		if (rw)
		{
			gup_result tmp_result;

			result = write_end_of_volume(1);

			if (cur_main_hdr != NULL)
				delete cur_main_hdr;
			cur_main_hdr = NULL;

			tmp_result = close_volume();
			if (result == GUP_OK)
				result = tmp_result;

			return result;
		}
		else
			return close_volume();
	}
	else
		return GUP_INTERNAL;
}

/*
 * mainheader *arj_archive::init_main_header(const char *comment)
 *
 * Allocate an arj_mainheader structure and initiate it.
 * The result is a NULL pointer if there is an error (archive not
 * opened).
 */

mainheader *arj_archive::init_main_header(const char *comment)
{
	arj_mainheader *main_hdr;

	if (!opened)
		return NULL;

	main_hdr = new arj_mainheader(comment);
	main_hdr->set_filename(cur_volume->name());

	return (mainheader *) main_hdr;
}

fileheader *arj_archive::init_file_header(const char *filename,
										  const char *comment,
										  const osstat *stat)
{
	arj_fileheader *file_hdr;

	if (!opened)
		return NULL;

	/*
	 * If 'stat' is NULL use the defaults from 'stat' in 'file_hdr'.
	 * Otherwise use the values from 'stat'.
	 */

	if (stat == NULL)
		file_hdr = new arj_fileheader(filename, comment);
	else
		file_hdr = new arj_fileheader(filename, comment, stat);

	return (fileheader *) file_hdr;
}

/*****************************************************************************
 *																			 *
 * CRC functions.															 *
 * 																			 *
 *****************************************************************************/

uint32 arj_archive::init_crc(void)
{
	return arj_init_crc();
}

uint32 arj_archive::calc_crc(uint8 *buf, long len, uint32 crc_in)
{
	return crc32(buf, len, crc_in, crc_table);
}

/*
 * uint32 arj_archive::post_proc_crc(uint32 crc)
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

uint32 arj_archive::post_process_crc(uint32 crc)
{
	return ~crc;
}

/*****************************************************************************
 *																			 *
 * Multiple volume support.													 *
 * 																			 *
 *****************************************************************************/

unsigned long arj_archive::get_mv_bytes_left(void)
{
	return st.pack_str.mv_bytes_left;
}

int arj_archive::mv_break(void)
{
	return st.pack_str.mv_next;
}

/*****************************************************************************
 *																			 *
 * Compression and decompression functions.									 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result arj_archive::skip_compressed_data(fileheader *header)
 *
 * Skip the compressed data of a file.
 *
 * Parameters:
 *
 * header	- file header of the file.
 *
 * Result: GUP_OK if no error.
 */

gup_result arj_archive::skip_compressed_data(fileheader *header)
{
	return seek(header->compsize, SEEK_CUR);
}

/*
 * gup_result arj_archive::encode(fileheader *header, int infile)
 *
 * Pack a (chunk) of a file.
 */

gup_result arj_archive::encode(fileheader *header, int infile)
{
	gup_result result;

	if ((result = archive::encode(header, infile)) == GUP_OK)
	{
		/*
		 * Set the VOLUME_FLAG in the file header if a multiple
		 * volume break has occured.
		 */

		if ((st.pack_str.mv_mode) && (st.pack_str.mv_next))
			((arj_fileheader *) header)->flags |= VOLUME_FLAG;
	}

	return result;
}

/*****************************************************************************
 *																			 *
 * Archive control function.												 *
 * 																			 *
 *****************************************************************************/

gup_result arj_archive::arcctl(int function, ... )
{
	gup_result result = GUP_INVAL;
	va_list parm;
	int *iptr;

	va_start(parm, function);

	switch(function)
	{
	case ARC_FILE_TYPES:
		iptr = va_arg(parm, int *);
		*iptr = ARC_FT_REG_FILE | ARC_FT_DIR;
		result = GUP_OK;
		break;

	case ARC_MV_MODE:
		iptr = va_arg(parm, int *);
		*iptr = 1;
		result = GUP_OK;
		break;
	}

	va_end(parm);

	return result;
}

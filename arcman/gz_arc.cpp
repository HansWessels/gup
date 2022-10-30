/*
 * Archive manager.
 *
 *     GZIP archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:12:46 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 222 $
 * $Log$
 * Revision 1.1  2000/09/03 13:12:46  wout
 * GZIP support.
 *
 */

#include "gup.h"

#include "arc_util.h"
#include "gup_err.h"
#include "compress.h"
#include "gup_io.h"
#include "options.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "archive.h"
#include "arcctl.h"
#include "gz_hdr.h"
#include "gz_arc.h"
#include "crc.h"
#include "os.h"
#include "support.h"

/*
 * Defines.
 */

#define MAXSFX					0		/* Size of self-extracting prefix. */
#define HEADERSIZE_MAX			4096	/* Maximum length of a header allowed
										   by this implementation. */
#define MAX_EXT_HDR_SIZE		65535	/* Maximum size of extended headers. */

#define HEADER_ID				0x8B1F	/* GZIP header ID. */

#define HDR_FLAGS				3		/* Offset of flags in header. */

/*
 * Local variables.
 */

static uint32 *crc_table = NULL;
static int crc_use_cnt = 0;

/*
 * gup_result get_crc_table(void)
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
		if ((crc_table = make_crc32_table(&poly)) == NULL)
			return GUP_NOMEM;
	}

	crc_use_cnt++;

	return GUP_OK;
}

/*
 * void free_crc_table(void)
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
 * Functions for writing an archive.										 *
 * 																			 *
 *****************************************************************************/

gzip_archive::gzip_archive(void)
{
}

gzip_archive::~gzip_archive(void)
{
	/*
	 * If the archive is still open, close it. If the archive was
	 * opened for writing, do not consider the archive to be valid.
	 */

	if (opened)
		close_archive(0);
}

/*****************************************************************************
 *																			 *
 * Functions for writing an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result gzip_archive::create_archive(const char *name, OPTIONS *options,
 *										   unsigned long first_volume_size, int use_temp,
 *										   GUPMSG *msgfunc)
 *
 * Open an archive for writing.
 */

gup_result gzip_archive::create_archive(const char *name, OPTIONS *options,
										unsigned long first_volume_size, int use_temp,
										GUPMSG *msgfunc)
{
	gup_result result;

	/*
	 * Allocate CRC table.
	 */

	if ((result = get_crc_table()) != GUP_OK)
		return result;

	/*
	 * Open the archive.
	 */

	if ((result = archive::create_archive(name, options, first_volume_size, use_temp, msgfunc)) != GUP_OK)
		free_crc_table();

	return result;
}

/*
 * gup_result gzip_archive::create_next_volume(unsigned long volume_size)
 *
 * Create the next volume of an archive. Return an error, because
 * multiple volume GZIP archives are not supported.
 */

gup_result gzip_archive::create_next_volume(unsigned long volume_size)
{
	(void) volume_size;

	return GUP_INTERNAL;
}

/*
 * gup_result gzip_archive::write_end_of_volume(int mv)
 *
 * Write end of archive.
 *
 * Parameters:
 *
 * mv		- 1 if the volume is part of a multiple volume archive and
 *				the volume is not the last volume of the archive.
 */

gup_result gzip_archive::write_end_of_volume(int mv)
{
	(void) mv;

	return GUP_OK;
}

/*
 * gup_result gzip_archive::write_main_header(const mainheader *header)
 *
 * Write a main header to the archive.
 */

gup_result gzip_archive::write_main_header(const mainheader *header)
{
	(void) header;

	return GUP_OK;
}

/*
 * gup_result conv_os_to_gzip(int host_os, uint8 &gzip_os)
 *
 * Convert a GUP internal OS id to a GZIP OS id.
 *
 * Parameters:
 *
 * host_os	- Host id in GUP format.
 * gzip_os	- Host id in GZIP format.
 *
 * Result:
 *
 * GUP_OK 	- no error
 */

static gup_result conv_os_to_gzip(int host_os, uint8 &gzip_os)
{
	switch(host_os)
	{
	case OS_MSDOS:
		gzip_os = 0;
		break;
	case OS_AMIGA:
		gzip_os = 1;
		break;
	case OS_VAX_VMS:
		gzip_os = 2;
		break;
	case OS_UNIX:
		gzip_os = 3;
		break;
	case OS_ATARI:
		gzip_os = 5;
		break;
	case OS_OS_2:
		gzip_os = 6;
		break;
	case OS_MAC:
		gzip_os = 7;
		break;
	case OS_WIN32:
		gzip_os = 11;
		break;
	case OS_PRIMOS:
		gzip_os = 15;
		break;
	default:
		return GUP_INTERNAL;
	}

	return GUP_OK;
}

/*
 * gup_result gzip_archive::write_ext_headers(const gzip_fileheader *file_hdr,
 *											  uint8 *&ptr)
 *
 * Utility function for write_file_header(). Write the extended
 * headers of a file header to the archive.
 *
 * Result: GUP_OK if no error. GUP_HDR_SIZE_OVF if an extended
 *         header exceeds the maximum length.
 */

gup_result gzip_archive::write_ext_headers(const gzip_fileheader *file_hdr,
										   uint8 *&ptr)
{
	const extended_header *cur_hdr;
	uint32 ext_hdr_type;
	const uint8 *ext_hdr_data;
	unsigned long ext_hdr_len;
	uint8 *p = ptr, *extra_ptr;
	long extra_field_len;

	extra_ptr = p;
	put_word_i(p, 0);					/* Place holder for extra field length. */

	/*
	 * Write extended headers.
	 */

	cur_hdr = file_hdr->first_ext_hdr(ext_hdr_type, ext_hdr_data, ext_hdr_len);

	while(cur_hdr)
	{
		if (ext_hdr_len > MAX_EXT_HDR_SIZE)
			return GUP_HDR_SIZE_OVF;

		put_word_i(p, (uint16) ext_hdr_type);	/* Write sub field ID. */
		put_word_i(p, (uint16) ext_hdr_len);	/* Sub field length. */
		while (ext_hdr_len-- > 0)
			*p++ = *ext_hdr_data++;

		cur_hdr = file_hdr->next_ext_hdr(cur_hdr, ext_hdr_type, ext_hdr_data,
										 ext_hdr_len);
	}

	extra_field_len = (long)(p - extra_ptr) - 2;

	if (extra_field_len > MAX_EXT_HDR_SIZE)
		return GUP_HDR_SIZE_OVF;

	put_word(extra_ptr, (uint16) extra_field_len);	/* Write extra field length. */

	ptr = p;

	return GUP_OK;
}

gup_result gzip_archive::write_file_header(const fileheader *header)
{
	const gzip_fileheader *file_hdr;
	int new_allocated = FALSE;
	gup_result result;
	char *name_ptr;
	unsigned long total_hdr_size, bytes_left;

	if (!opened || !rw)
		return GUP_INTERNAL;

	switch(header->get_fhdr_type())
	{
	case FHDR_GZIP:						/* Header is already a GZIP file header. */
		file_hdr = (const gzip_fileheader *) header;
		break;
	default:							/* Other file header, create a GZIP main header. */
		file_hdr = (const gzip_fileheader *) init_file_header(header->get_filename(),
						header->get_comment(), header->get_file_stat());
		new_allocated = TRUE;
		break;
	}

	name_ptr = gzip_conv_from_os_name(file_hdr->get_filename());

	total_hdr_size = file_hdr->get_header_len();

	if ((result = gup_io_write_announce(file, total_hdr_size)) == GUP_OK)
	{
		uint8 *p, *start;
		const uint8 *s;

		p = start = gup_io_get_current(file, &bytes_left);

		put_word_i(p, HEADER_ID);		/* Write header ID. */
		*p++ = 8;						/* Write method (always 8, deflate). */
		*p++ = file_hdr->flags;			/* Write flags. */
		put_long_i(p, file_hdr->orig_time_stamp);	/* Write time stamp. */
		*p++ = file_hdr->extra_flags;	/* Write extra flags. */

		if ((result = conv_os_to_gzip(file_hdr->host_os, *p)) == GUP_OK)
		{
			p++;

			if (file_hdr->flags & GZ_CONT)
				put_word_i(p, file_hdr->part_number);

			if (file_hdr->flags & GZ_EXTRA_FIELD)
				result = write_ext_headers(file_hdr, p);

			if (result == GUP_OK)
			{
				if (file_hdr->flags & GZ_ORIG_FNAME)
				{
					s = (const uint8 *) name_ptr;
					while(*s)
						*p++ = *s++;
				}

				if (file_hdr->flags & GZ_COMMENT)
				{
					s = (const uint8 *) file_hdr->get_comment();
					while(*s)
						*p++ = *s++;
				}

				gup_io_set_current(file, p);
			}
		}
	}

	/*
	 * Free all allocated temporary space.
	 */

	delete[] name_ptr;

	if (new_allocated)
		delete (gzip_fileheader *) file_hdr;

	return result;
}

gup_result gzip_archive::write_file_trailer(const fileheader *header)
{
	gup_result result;
	unsigned long bytes_left;

	if ((result = gup_io_write_announce(file, 8)) == GUP_OK)
	{
		uint8 *p;

		p = gup_io_get_current(file, &bytes_left);

		put_long_i(p, header->file_crc);	/* Write the CRC. */
		put_long_i(p, header->origsize);	/* Write uncompressed size. */

		gup_io_set_current(file, p);
	}

	return result;
}

/*****************************************************************************
 *																			 *
 * Functions for reading an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result gzip_archive::open_archive(const char *name, OPTIONS *options,
 *										 GUPMSG *msgfunc)
 *
 * Open an archive for reading.
 */

gup_result gzip_archive::open_archive(const char *name, OPTIONS *options,
									  GUPMSG *msgfunc)
{
	gup_result result;

	/*
	 * Allocate CRC table.
	 */

	if ((result = get_crc_table()) != GUP_OK)
		return result;

	/*
	 * Open the archive.
	 */

	if ((result = archive::open_archive(name, options, msgfunc)) != GUP_OK)
		free_crc_table();

	return result;
}

/*
 * gup_result gzip_archive::open_next_volume(void)
 *
 * Open the next volume of an archive. Return an error, because
 * multiple volume GZIP archives are not supported.
 */

gup_result gzip_archive::open_next_volume(void)
{
	return GUP_INTERNAL;
}

/*
 * gup_result gzip_archive::find_header(int &hdr_len)
 *
 * Find a header in the GZIP file. If this function completes
 * successfully, the file buffer contains the entire header.
 *
 * hdr_len		- contains on return the total length of the header.
 *
 * Result codes:
 * GUP_OK				- no error
 * GUP_EOF				- if end of file reached
 * GUP_END_ARCHIVE		- end of archive reached.
 * GUP_END_VOLUME		- end of volume reached.
 * GUP_NO_ARCHIVE		- if nothing found which looks like a valid header
 * GUP_BROKEN			- if the archive might be broken
 * GUP_HDR_CRC_FAULT	- checksum error in header, hdr_type is valid.
 * GUP_HDR_POSITION		- header not found at expected position.
 */

gup_result gzip_archive::find_header(int &hdr_len)
{
	uint8 *buf_ptr;
	unsigned long bytes_left;
	long arcpos, startpos;
	gup_result result;

	/*
	 * Get current position and bytes left in buffer.
	 */

	buf_ptr = gup_io_get_current(file, &bytes_left);

	if ((result = tell(arcpos)) != GUP_OK)
		return result;
	startpos = arcpos;

	while (arcpos <= MAXSFX)
	{
		int found = FALSE;
		unsigned long hdr_length;
		uint8 flags;

		/*
		 * Scan for header ID.
		 */

		hdr_length = 10;				/* Length of the header ID and the header length. */

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
					found = TRUE;
				else
				{
					if (arcpos >= MAXSFX)
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
				 * End of archive found. Return GUP_NO_ARCHIVE.
				 */

				return GUP_NO_ARCHIVE;
			}
		} while (!found);

		/*
		 * Header ID found.
		 */

		flags = buf_ptr[HDR_FLAGS];

		if (flags & GZ_CONT)
			hdr_length += 2;			/* Length of part number. */

		if (flags & GZ_EXTRA_FIELD)
		{
			hdr_length += 2;			/* Extra field length. */

			if (bytes_left < hdr_length)
			{
				if ((result = gup_io_fill(file)) != GUP_OK)
					return result;

				buf_ptr = gup_io_get_current(file, &bytes_left);
			}

			if (bytes_left >= hdr_length)
				hdr_length += get_word(buf_ptr + hdr_length - 2);
			else
				found = FALSE;			/* No valid header. */
		}

		if ((flags & GZ_ORIG_FNAME) && found)
		{
			do
			{
				hdr_length += 1;

				if (bytes_left < hdr_length)
				{
					if ((result = gup_io_fill(file)) != GUP_OK)
						return result;

					buf_ptr = gup_io_get_current(file, &bytes_left);
				}

				if (bytes_left < hdr_length)
					found = FALSE;		/* No valid header. */
			} while (found && (buf_ptr[hdr_length - 1] != 0));
		}

		if ((flags & GZ_COMMENT) && found)
		{
			do
			{
				hdr_length += 1;

				if (bytes_left < hdr_length)
				{
					if ((result = gup_io_fill(file)) != GUP_OK)
						return result;

					buf_ptr = gup_io_get_current(file, &bytes_left);
				}

				if (bytes_left < hdr_length)
					found = FALSE;		/* No valid header. */
			} while (found && (buf_ptr[hdr_length - 1] != 0));
		}

		if (found && (hdr_length <= HEADERSIZE_MAX))
		{
			hdr_len = (int) hdr_length;

			return (arcpos != startpos) ?
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
 * mainheader *gzip_archive::read_main_header(gup_result &result)
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
 * GUP_NO_ACRHIVE		- file is not an GZIP archive
 * GUP_BROKEN			- archive is broken, but it might be possible
 *						  to recover data from the archive
 */

mainheader *gzip_archive::read_main_header(gup_result &result)
{
	int hdr_size;

	/*
	 * GZIP has no main header. Just try to find the first file header
	 * to determine if the archive is valid.
	 */

	result = find_header(hdr_size);

	if ((result == GUP_OK) || (result == GUP_HDR_POSITION))
	{
		gzip_mainheader *main_hdr;

		main_hdr = new gzip_mainheader(NULL);

		return (mainheader *) main_hdr;
	}
	else
		return NULL;
}

/*
 * gup_result conv_os_to_gup(uint8 gzip_os, int &host_os)
 *
 * Convert a GZIP OS id to a GUP internal OS id.
 *
 * Parameters:
 *
 * gzip_os	- Host id in GZIP format.
 * host_os	- Host id in GUP format.
 *
 * Result:
 *
 * GUP_OK 	- no error
 */

static gup_result conv_os_to_gup(uint8 gzip_os, int &host_os)
{
	switch(gzip_os)
	{
	case 0:
		host_os = OS_MSDOS;
		break;
	case 1:
		host_os = OS_AMIGA;
		break;
	case 2:
		host_os = OS_VAX_VMS;
		break;
	case 3:
		host_os = OS_UNIX;
		break;
	case 5:
		host_os = OS_ATARI;
		break;
	case 6:
		host_os = OS_OS_2;
		break;
	case 7:
		host_os = OS_MAC;
		break;
	case 11:
		host_os = OS_WIN32;
		break;
	case 15:
		host_os = OS_PRIMOS;
		break;
	default:
		return GUP_HDR_UNKNOWN_OS;
	}

	return GUP_OK;
}

/*
 * fileheader *gzip_archive::read_file_header(gup_result &result)
 *
 * Read the file header from the archive. Note that even if there
 * is an error (result != GUP_OK), the result does not have to be
 * NULL. This is the case if it might be possible to extract some
 * usefull information from the fileheader (like the file name).
 *
 * Result codes:
 *
 * GUP_OK					- no error
 * GUP_EOF					- unexpected end of file
 * GUP_HDR_CRC_FAULT		- CRC (checksum) error in header
 * GUP_NO_ACRHIVE			- file is not an ARJ archive
 * GUP_BROKEN				- archive is broken, but it might be possible
 * 							  to recover data from the archive
 * GUP_HDR_POSITION			- header not found at expected position. The
 *							  header data is valid.
 * GUP_END_VOLUME			- end of volume reached
 * GUP_END_ARCHIVE			- end of archive marker read
 * GUP_HDR_UNKNOWN_METHOD	- unknown method
 * GUP_HDR_UNKNOWN_OS		- unknown OS id
 */

fileheader *gzip_archive::read_file_header(gup_result &result)
{
	int hdr_size;

	result = find_header(hdr_size);

	if ((result == GUP_OK) || ((result == GUP_HDR_POSITION) && opt_repair))
	{
		gzip_fileheader *file_hdr;
		uint8 *ptr;
		unsigned long bytes_left;
		gup_result tmp_result;

		ptr = gup_io_get_current(file, &bytes_left);

		/*
		 * No error. Create a gzip_fileheader object using
		 * the data read from file.
		 */

		file_hdr = new gzip_fileheader(NULL, NULL);

		ptr += 2;						/* Skip header ID. */

		if (*ptr++ != 8)
		{
			if (result == GUP_OK)
				result = GUP_HDR_UNKNOWN_METHOD;	/* Unknown method. */
		}

		file_hdr->method = GZIP;		/* Method is always GZIP. */
		file_hdr->flags = *ptr++;		/* Get flags. */
		file_hdr->orig_time_stamp = get_longword_i(ptr);
		file_hdr->stat.mtime = gzip_conv_to_os_time(file_hdr->orig_time_stamp);
		file_hdr->extra_flags = *ptr++;	/* Get extra flags. */

		if ((tmp_result = conv_os_to_gup(*ptr++, file_hdr->host_os)) != GUP_OK)
		{
			if (result == GUP_OK)
				result = tmp_result;
		}

		if (file_hdr->flags & GZ_CONT)
			file_hdr->part_number = get_word_i(ptr);
		else
			file_hdr->part_number = 0;

		if (file_hdr->flags & GZ_EXTRA_FIELD)
		{
			unsigned long extra_length;
			uint32 ext_hdr_type;
			unsigned long ext_hdr_size;

			extra_length = get_word_i(ptr);

			while (extra_length > 0)
			{
				ext_hdr_type = get_word_i(ptr);	/* Get sub field id. */
				ext_hdr_size = get_word_i(ptr);	/* Get sub field length. */

				if ((ext_hdr_size + 4) <= extra_length)
				{
					file_hdr->add_ext_hdr(ext_hdr_type, ptr, ext_hdr_size);
					ptr += ext_hdr_size;	/* Skip sub field data. */
					extra_length -= ext_hdr_size + 4;
				}
				else
				{
					if (result == GUP_OK)
						result = GUP_HDR_ERROR;
					break;
				}
			}

			if (extra_length > 0)
				ptr += extra_length;
		}

		if (file_hdr->flags & GZ_ORIG_FNAME)
		{
			char *os_name;

			os_name = gzip_conv_to_os_name((char *) ptr);
			file_hdr->set_filename(os_name);
			delete[] os_name;

			while (*ptr != 0)
				ptr++;
			ptr++;
		}
		else
			file_hdr->set_filename("unknown");

		if (file_hdr->flags & GZ_COMMENT)
		{
			file_hdr->set_comment((char *) ptr);

			while (*ptr != 0)
				ptr++;
			ptr++;
		}

		file_hdr->file_type = BINARY_TYPE;	/* File type. */
		file_hdr->offset = 0;

#if (OS == OS_UNIX) || (OS == OS_WIN32)
		file_hdr->stat.atime = file_hdr->stat.ctime = file_hdr->stat.mtime;
#elif (OS != OS_MSDOS)
#error "This OS is not supported."
#endif

		gup_io_set_current(file, ptr);

		return (fileheader *) file_hdr;
	}
	else
		return NULL;
}

gup_result gzip_archive::read_file_trailer(fileheader *header)
{
	uint8 *buf_ptr;
	unsigned long bytes_left;
	gup_result result;
	gzip_fileheader *file_hdr;

	if (header->get_fhdr_type() != FHDR_GZIP)
		return GUP_INTERNAL;

	file_hdr = (gzip_fileheader *) header;

	buf_ptr = gup_io_get_current(file, &bytes_left);

	/*
	 * Make sure the buffer contains sufficient bytes.
	 */

	if (bytes_left < 8)
	{
		if ((result = gup_io_fill(file)) != GUP_OK)
			return result;

		buf_ptr = gup_io_get_current(file, &bytes_left);
	}

	if (bytes_left >= 8)
	{
		file_hdr->file_crc = (uint32) get_word_i(buf_ptr);
		file_hdr->origsize = file_hdr->stat.length = get_longword_i(buf_ptr);

		gup_io_set_current(file, buf_ptr);

		return GUP_OK;
	}
	else
		return GUP_EOF;
}

/*****************************************************************************
 *																			 *
 * General functions.														 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result gzip_archive::close_archive(int ok)
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

gup_result gzip_archive::close_archive(int ok)
{
	gup_result result = GUP_OK, tmp_result;

	if (opened)
	{
		if (rw && ok)
			result = write_end_of_volume(0);

		tmp_result = archive::close_archive(ok);

		if (result == GUP_OK)
			result = tmp_result;

		free_crc_table();
	}
	else
		result = GUP_INTERNAL;

	return result;
}

/*
 * gup_result gzip_archive::close_curr_volume(void)
 *
 * Close the current volume.
 *
 * Only call this function if the current volume is not the last volume
 * of the archive. If the volume is the last, use 'close_archive'.
 */

gup_result gzip_archive::close_curr_volume(void)
{
	return GUP_INTERNAL;
}

/*
 * mainheader *gzip_archive::init_main_header(const char *comment)
 *
 * Allocate an gzip_mainheader structure and initiate it.
 * The result is a NULL pointer if there is an error (archive not
 * opened).
 */

mainheader *gzip_archive::init_main_header(const char *comment)
{
	gzip_mainheader *main_hdr;

	if (!opened)
		return NULL;

	main_hdr = new gzip_mainheader(comment);

	return (mainheader *) main_hdr;
}

fileheader *gzip_archive::init_file_header(const char *filename,
										   const char *comment,
										   const osstat *stat)
{
	gzip_fileheader *file_hdr;

	if (!opened)
		return NULL;

	/*
	 * If 'stat' is NULL use the defaults from 'stat' in 'file_hdr'.
	 * Otherwise use the values from 'stat'.
	 */

	if (stat == NULL)
		file_hdr = new gzip_fileheader(filename, comment);
	else
		file_hdr = new gzip_fileheader(filename, comment, stat);

	return (fileheader *) file_hdr;
}

/*****************************************************************************
 *																			 *
 * CRC functions.															 *
 * 																			 *
 *****************************************************************************/

uint32 gzip_archive::init_crc(void)
{
	return arj_init_crc();
}

uint32 gzip_archive::calc_crc(uint8 *buf, long len, uint32 crc_in)
{
	return crc32(buf, len, crc_in, crc_table);
}

/*****************************************************************************
 *																			 *
 * Compression and decompression functions.									 *
 * 																			 *
 *****************************************************************************/

static gup_result buf_write_nocrc_test(long count, void *buffer, void *propagator)
{
	(void) count;
	(void) buffer;
	(void) propagator;

	return GUP_OK;
}

static void print_progress(unsigned long delta_size, void *pp_propagator)
{
	(void) delta_size;
	(void) pp_propagator;
}

/*
 * gup_result gzip_archive::skip_compressed_data(fileheader *header)
 *
 * Skip the compressed data of a file.
 *
 * Parameters:
 *
 * header	- file header of the file.
 *
 * Result: GUP_OK if no error.
 */

gup_result gzip_archive::skip_compressed_data(fileheader *header)
{
	gup_result result;
	gup_result (*orig_write_crc)(long count, void *buf, void *wc_propagator);
	void (*orig_print_progress)(unsigned long delta_size, void* pp_propagator);

	orig_write_crc = st.unpack_str.write_crc;
	st.unpack_str.write_crc = buf_write_nocrc_test;

	orig_print_progress = st.unpack_str.print_progres;
	st.unpack_str.print_progres = print_progress;

	result = decode(header, 0);

	st.unpack_str.print_progres = orig_print_progress;
	st.unpack_str.write_crc = orig_write_crc;

	return result;
}

gup_result gzip_archive::decode(fileheader *header, int outfile)
{
	gup_result result = GUP_OK;
	long start, cur_pos;
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
			result = tell(cur_pos);

			if (result == GUP_OK)
			{
				header->compsize = cur_pos - start;

				/*
				 * Set the CRC calculated while decompressing.
				 */

				header->real_crc = post_process_crc(wcs.crc);
			}
		}
	}

	return result;
}

/*****************************************************************************
 *																			 *
 * Archive control function.												 *
 * 																			 *
 *****************************************************************************/

gup_result gzip_archive::arcctl(int function, ... )
{
	gup_result result = GUP_INVAL;
	va_list parm;
	int *iptr;

	va_start(parm, function);

	switch(function)
	{
	case ARC_FILE_TYPES:
		iptr = va_arg(parm, int *);
		*iptr = ARC_FT_REG_FILE;
		result = GUP_OK;
		break;

	case ARC_MV_MODE:
		iptr = va_arg(parm, int *);
		*iptr = 0;
		result = GUP_OK;
		break;
	}

	va_end(parm);

	return result;
}

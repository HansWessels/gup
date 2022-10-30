/*
 * Archive manager.
 *
 *     LHA archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:07:34 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 217 $
 * $Log$
 * Revision 1.5  2000/09/03 13:07:34  wout
 * Added checks on the size of headers while writing headers. Moved
 * writing of extended headers to a separate function. Added function
 * skip_compressed_data().
 *
 * Revision 1.4  2000/08/27 10:27:46  hans
 * Replaced STORE with LHA_LH0_
 *
 * Revision 1.3  2000/07/30 15:23:09  wout
 * Use gup_io_write_announce, gup_io_get_current etc. instead of
 * read and write, to write and read file headers and archive
 * headers.
 *
 * Revision 1.2  2000/07/16 17:11:10  hwessels
 * Updated to GUP 0.0.4
 *
 * Revision 1.1  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
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
#include "lha_hdr.h"
#include "lha_arc.h"
#include "crc.h"
#include "os.h"
#include "support.h"

/*
 * Defines.
 */

#define MAXSFX					4096	/* Size of self-extracting prefix. */
#define MAX_ARC_COMM_LEN		1024	/* Maximum length of archive comment. */

#define HEADERSIZE_MAX			4096	/* Maximum length of a header allowed
										   by this implementation. */

#define HDR_SIZE				0		/* Offset of header size in header. */
#define HDR_CHECKSUM			1		/* Offset of checksum in header. */
#define HDR_METHOD_ID			2		/* Offset of method ID in header. */
#define HDR_PACKED_SIZE			7		/* Offset of packed size. */
#define HDR_LEVEL				20		/* Offset of header level in header. */
#define HDR_NAME_LENGTH			21		/* Offset of name length in header. */
#define HDR_NAME				22		/* Offset of name in header. */
#define HDR_LV1_NEXT_HDR_SIZE	25		/* Offset of first next header size in
										   a level 1 header. The name length
										   should be added. */
#define HDR_LV2_NEXT_HDR_SIZE	24		/* Offset of first next header size in
										   a level 2 header. */

#define DELIM	((char) 0xFF)			/* LHA path seperator. */

#define MAX_LV0_HDR_SIZE		255		/* Maximum size of level 0 headers. */
#define MAX_LV1_HDR_SIZE		255		/* Maximum size of level 1 headers. */
#define MAX_LV2_HDR_SIZE		65535	/* Maximum size of level 2 headers. */
#define MAX_EXT_HDR_SIZE		65535	/* Maximum size of extended headers. */

/*
 * Local variables.
 */

static uint16 *crc_table = NULL;
static int crc_use_cnt = 0;

static const struct
{
	const char *method_str;
	int method;
} method_strings [] =
{
	{ "-lhd-", LHA_LHD_ },
	{ "-lh0-", LHA_LH0_ },
	{ "-lh1-", LHA_LH1_ },
	{ "-lh2-", LHA_LH2_ },
	{ "-lh3-", LHA_LH3_ },
	{ "-lh4-", LHA_LH4_ },
	{ "-lh5-", LHA_LH5_ },
	{ "-lh6-", LHA_LH6_ },
	{ "-lh7-", LHA_LH7_ },
	{ "-lzs-", LHA_LZS_ },
	{ "-lz4-", LHA_LZ4_ },
	{ "-lz5-", LHA_LZ5_ },
	{ "-afx-", LHA_AFX_ },
	{ NULL, -1 }
};

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
	  uint16 poly = ANSI_CRC16_POLY;
		if ((crc_table = (uint16 *) make_crc16_table(&poly)) == NULL)
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
		free_crc16_table(crc_table);	/* Free CRC table. */
}

/*
 * uint8 calc_checksum(uint8 *buffer, unsigned long len)
 *
 * Calculate the checksum of a LHA header.
 *
 * Parameters:
 *
 * buffer	- pointer to buffer with header.
 * len		- length of header.
 *
 * Result: checksum.
 */

static uint8 calc_checksum(uint8 *buffer, unsigned long len)
{
	uint8 sum = 0;

	while (len-- > 0)
		sum += *buffer++;

	return sum;
}

/*****************************************************************************
 *																			 *
 * Functions for writing an archive.										 *
 * 																			 *
 *****************************************************************************/

lha_archive::lha_archive(void)
{
	lha_hdr_level = 2;
}

lha_archive::~lha_archive(void)
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
 * gup_result lha_archive::create_archive(const char *name, OPTIONS *options,
 *										  unsigned long first_volume_size, int use_temp,
 *										  GUPMSG *msgfunc)
 *
 * Open an archive for writing.
 */

gup_result lha_archive::create_archive(const char *name, OPTIONS *options,
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
 * gup_result lha_archive::create_next_volume(unsigned long volume_size)
 *
 * Create the next volume of an archive. Return an error, because
 * multiple volume LHA archives are not supported.
 */

gup_result lha_archive::create_next_volume(unsigned long volume_size)
{
	(void) volume_size;

	return GUP_INTERNAL;
}

/*
 * gup_result lha_archive::write_end_of_volume(int mv)
 *
 * Write end of archive.
 *
 * Parameters:
 *
 * mv		- 1 if the volume is part of a multiple volume archive and
 *				the volume is not the last volume of the archive.
 */

gup_result lha_archive::write_end_of_volume(int mv)
{
	gup_result result;
	uint8 *ptr;
	unsigned long bytes_left;

	(void) mv;

	/*
	 * Write the end of archive marker to the archive.
	 */

	if ((result = gup_io_write_announce(file, 1)) != GUP_OK)
		return result;

	ptr = gup_io_get_current(file, &bytes_left);

	*ptr++ = 0;

	gup_io_set_current(file, ptr);

	return result;
}

/*
 * gup_result lha_archive::write_main_header(const mainheader *header)
 *
 * Write a main header to the archive.
 */

gup_result lha_archive::write_main_header(const mainheader *header)
{
	const lha_mainheader *main_hdr;
	int new_allocated = FALSE;
	gup_result result;
	const char *comment;

	if (!opened || !rw)
		return GUP_INTERNAL;

	switch(header->get_mhdr_type())
	{
	case MHDR_LHA:						/* Header is already an LHA main header. */
		main_hdr = (const lha_mainheader *) header;
		break;
	default:							/* Other main header, create an LHA main header. */
		main_hdr = (const lha_mainheader *) init_main_header(header->get_comment());
		new_allocated = TRUE;
		break;
	}

	/*
	 * Put comment.
	 */

	if (((comment = main_hdr->get_comment()) != NULL) && (strlen(comment) > 0))
	{
		unsigned long total_hdr_size;

		/*
		 * Calculate total size.
		 */

		if ((total_hdr_size = strlen(comment)) > MAX_ARC_COMM_LEN)
			total_hdr_size = 5 + MAX_ARC_COMM_LEN + 1;
		else
			total_hdr_size += 5 + 1;

		/*
		 * Write comment.
		 */

		if ((result = gup_io_write_announce(file, total_hdr_size)) == GUP_OK)
		{
			uint8 *p;
			unsigned long bytes_left;
			const char *src;
			int len = 0;

			p = gup_io_get_current(file, &bytes_left);

			src = "-com-";
			while (*src)
				*p++ = *src++;

			while ((*comment) && (len < MAX_ARC_COMM_LEN))
			{
				*p++ = *comment++;
				len++;
			}

			*p++ = 0;

			gup_io_set_current(file, p);
		}
	}
	else
		result = GUP_OK;

	if (new_allocated)
		delete (lha_mainheader *) main_hdr;

	return result;
}

/*
 * gup_result conv_method_to_str(int method, char *str)
 *
 * Convert an internally used method number to a lha method string.
 *
 * Parameters:
 *
 * method	- method number.
 * str		- pointer to destination string.
 *
 * Result:
 *
 * GUP_OK		- No error.
 * GUP_INTERNAL	- Method not supported by LHA. Should not happen.
 */

static gup_result conv_method_to_str(int method, char *str)
{
	int i = 0;

	while ((method_strings[i].method_str != NULL) &&
		   (method_strings[i].method != method))
		i++;

	if (method_strings[i].method != -1)
	{
		const char *src = method_strings[i].method_str;

		while (*src)
			*str++ = *src++;

		return GUP_OK;
	}
	else
		return GUP_INTERNAL;
}

/*
 * gup_result conv_os_to_lha(int host_os, uint8 &lha_os)
 *
 * Convert a GUP internal OS id to a LHA OS id.
 *
 * Parameters:
 *
 * host_os	- Host id in GUP format.
 * lha_os	- Host id in LHA format.
 *
 * Result:
 *
 * GUP_OK 	- no error
 */

static gup_result conv_os_to_lha(int host_os, uint8 &lha_os)
{
	switch(host_os)
	{
	case OS_MSDOS:
		lha_os = 'M';
		break;
	case OS_OS_2:
		lha_os = '2';
		break;
	case OS_OS9:
		lha_os = '9';
		break;
	case OS_OS_68K:
		lha_os = 'K';
		break;
	case OS_OS_386:
		lha_os = '3';
		break;
	case OS_HUMAN:
		lha_os = 'H';
		break;
	case OS_UNIX:
		lha_os = 'U';
		break;
	case OS_CP_M:
		lha_os = 'C';
		break;
	case OS_FLEX:
		lha_os = 'F';
		break;
	case OS_MAC:
		lha_os = 'm';
		break;
	case OS_RUNSER:
		lha_os = 'R';
		break;
	case OS_TOWNOS:
		lha_os = 'T';
		break;
	case OS_XOSK:
		lha_os = 'X';
		break;
	case OS_AMIGA:
		lha_os = 'A';
		break;
	case OS_ATARI:
		lha_os = 'a';
		break;
	default:
		return GUP_INTERNAL;
	}

	return GUP_OK;
}

/*
 * gup_result lha_archive::write_ext_headers(void)
 *
 * Utility function for write_file_header(). Write the extended
 * headers of a file header to the archive.
 *
 * Result: GUP_OK if no error. GUP_HDR_SIZE_OVF if an extended
 *         header exceeds the maximum length.
 */

gup_result lha_archive::write_ext_headers(const lha_fileheader *file_hdr,
										  const char *name_ptr,
										  const char *path_ptr,
										  uint8 *&ptr)
{
	const char *comm_ptr;
	const extended_header *cur_hdr;
	uint32 ext_hdr_type;
	const uint8 *ext_hdr_data;
	unsigned long ext_hdr_len;
	uint8 *p = ptr;

	/*
	 * Put file name.
	 */

	if (file_hdr->lha_hdr_level == 2)
	{
		int name_len = (int) strlen(name_ptr);
		const char *s = name_ptr;

		if ((name_len + 3) > MAX_EXT_HDR_SIZE)
			return GUP_HDR_SIZE_OVF;

		put_word_i(p, (uint16) (name_len + 3));
		*p++ = 1;
		while (name_len-- > 0)
			*p++ = *s++;
	}

	/*
	 * Put directory name.
	 */

	if (path_ptr != NULL)
	{
		int path_len = (int) strlen(path_ptr);
		const char *s = path_ptr;

		if ((path_len + 3) > MAX_EXT_HDR_SIZE)
			return GUP_HDR_SIZE_OVF;

		put_word_i(p, (uint16) (path_len + 3));
		*p++ = 2;
		while (path_len-- > 0)
			*p++ = *s++;
	}

	/*
	 * Put comment.
	 */

	if ((comm_ptr = file_hdr->get_comment()) != NULL)
	{
		int comm_len = (int) strlen(comm_ptr);

		if ((comm_len + 3) > MAX_EXT_HDR_SIZE)
			return GUP_HDR_SIZE_OVF;

		if (comm_len > 0)
		{
			put_word_i(p, (uint16) (comm_len + 3));
			*p++ = 0x3F;

			while(comm_len-- > 0)
				*p++ = *comm_ptr++;			   
		}
	}

	/*
	 * Put other extended headers.
	 */

	cur_hdr = file_hdr->first_ext_hdr(ext_hdr_type, ext_hdr_data, ext_hdr_len);

	while(cur_hdr)
	{
		if ((ext_hdr_len + 2) > MAX_EXT_HDR_SIZE)
			return GUP_HDR_SIZE_OVF;

		put_word_i(p, (uint16)(ext_hdr_len + 2));
		while (ext_hdr_len-- > 0)
			*p++ = *ext_hdr_data++;

		cur_hdr = file_hdr->next_ext_hdr(cur_hdr, ext_hdr_type, ext_hdr_data,
										 ext_hdr_len);
	}

	put_word_i(p, 0);					/* End of extended headers. */

	ptr = p;

	return GUP_OK;
}

gup_result lha_archive::write_file_header(const fileheader *header)
{
	const lha_fileheader *file_hdr;
	int new_allocated = FALSE;
	gup_result result;
	char *name_ptr, *path_ptr;
	unsigned long total_hdr_size, bytes_left;

	if (!opened || !rw)
		return GUP_INTERNAL;

	if ((result = tell(header_pos)) != GUP_OK)
		return result;

	switch(header->get_fhdr_type())
	{
	case FHDR_LHA:						/* Header is already an LHA file header. */
		file_hdr = (const lha_fileheader *) header;
		break;
	default:							/* Other file header, create an LHA main header. */
		file_hdr = (const lha_fileheader *) init_file_header(header->get_filename(),
						header->get_comment(), header->get_file_stat());
		new_allocated = TRUE;
		break;
	}

	if (file_hdr->file_type == SYMLINK_TYPE)
	{
		char *name;

		name = new char[strlen(file_hdr->get_filename()) +
						strlen(file_hdr->get_linkname()) + 2];

		strcpy(name, file_hdr->get_filename());
		strcat(name, "|");
		strcat(name, file_hdr->get_linkname());

		lha_conv_from_os_name(name, file_hdr->host_os, file_hdr->lha_hdr_level,
							  0, name_ptr, path_ptr);

		delete[] name;
	}
	else
		lha_conv_from_os_name(file_hdr->get_filename(), file_hdr->host_os, file_hdr->lha_hdr_level,
							  (file_hdr->file_type == DIR_TYPE), name_ptr, path_ptr);

	total_hdr_size = file_hdr->get_header_len();

	if ((result = gup_io_write_announce(file, total_hdr_size)) == GUP_OK)
	{
		uint8 *p, *start;

		p = start = gup_io_get_current(file, &bytes_left);

		p += 2;							/* Skip header size and sum for the time being. */

		if ((result = conv_method_to_str(((file_hdr->file_type == DIR_TYPE) ||
										  (file_hdr->file_type == SYMLINK_TYPE)) ? LHA_LHD_ : file_hdr->method,
										 (char *) p)) == GUP_OK)
		{
			p += 5;						/* Skip method. */

			put_long_i(p, file_hdr->compsize);	/* Compressed size. */
			put_long_i(p, file_hdr->origsize);	/* Original size. */
			put_long_i(p, file_hdr->orig_time_stamp);	/* Time stamp. */
			*p++ = file_hdr->msdos_file_mode;	/* File mode. */
			*p++ = file_hdr->lha_hdr_level;		/* Header level. */

			/*
			 * If the header level is 0 or 1, copy the file name.
			 */

			if (file_hdr->lha_hdr_level < 2)
			{
				uint8 *src = (uint8 *) name_ptr;
				*p++ = (uint8) strlen(name_ptr);	/* Put length of name. */

				while(*src)
					*p++ = *src++;
			}

			if (file_hdr->has_crc())
				put_word_i(p, (uint16) file_hdr->file_crc);	/* File CRC. */

			/*
			 * If the header level is 1 or 2 or if the header level is 0 and
			 * the OS is not OS_GENERIC, write the OS id. If the header level
			 * is 0, write also the OS extensions.
			 */

			if ((file_hdr->lha_hdr_level > 0) || (file_hdr->host_os != OS_GENERIC))
			{
				if ((result = conv_os_to_lha(file_hdr->host_os, *p)) == GUP_OK)
				{
					p++;

					if (file_hdr->lha_hdr_level == 0)
					{
						*p++ = (uint8) file_hdr->minor_version;

						if (file_hdr->host_os == OS_UNIX)
						{
							put_long_i(p, file_hdr->unix_time_stamp);
							put_word_i(p, file_hdr->unix_file_mode);
							put_word_i(p, file_hdr->unix_uid);
							put_word_i(p, file_hdr->unix_gid);
						}
					}
				}
			}

			if (result == GUP_OK)
			{
				unsigned long hdr_len = 0;

				if (file_hdr->lha_hdr_level == 0)
				{
					/*
					 * Level 0 header. Determine length and calculate checksum.
					 */

					hdr_len = (unsigned long)(p - &start[HDR_METHOD_ID]);
					if (hdr_len <= MAX_LV0_HDR_SIZE)
					{
						start[HDR_SIZE] = (uint8) hdr_len;
						start[HDR_CHECKSUM] = calc_checksum(start + HDR_METHOD_ID, hdr_len);
					}
					else
						result = GUP_HDR_SIZE_OVF;
				}
				else
				{
					uint8 *ext_hdr_start = NULL;
					uint16 *hdr_crc_ptr = NULL;

					/*
					 * Level 1 or 2 header. Write extended headers.
					 */

					if (file_hdr->lha_hdr_level == 1)
					{
						ext_hdr_start = &p[2];	/* Include first extended header size. */
						hdr_len = (unsigned long)(ext_hdr_start - &start[HDR_METHOD_ID]);
					}

					/*
					 * If the header is a level 2 header, reserve space for
					 * the header CRC.
					 */

					if (file_hdr->lha_hdr_level == 2)
					{
						put_word_i(p, 5);
						*p++ = 0;
						hdr_crc_ptr = (uint16 *) p;
						put_word_i(p, 0);
					}

					if ((result = write_ext_headers(file_hdr, name_ptr,
													path_ptr, p)) == GUP_OK)
					{
						if (file_hdr->lha_hdr_level == 1)
						{
							/*
							 * Level 1 header. Update compressed size, header length and
							 * checksum.
							 */

							if (hdr_len <= MAX_LV1_HDR_SIZE)
							{
								put_long(&start[HDR_PACKED_SIZE], file_hdr->compsize + (p - ext_hdr_start));
								start[HDR_SIZE] = (uint8) hdr_len;
								start[HDR_CHECKSUM] = calc_checksum(start + HDR_METHOD_ID, hdr_len);
							}
							else
								result = GUP_HDR_SIZE_OVF;
						}
						else
						{
							uint32 crc;

							/*
							 * Level 2 header. Update header length and header CRC.
							 */

							hdr_len = (unsigned long)(p - start);
							if (hdr_len <= MAX_LV2_HDR_SIZE)
							{
								put_word(start, (uint16) hdr_len);

								crc = init_crc();
								crc = calc_crc(start, hdr_len, crc);

								put_word((uint8 *) hdr_crc_ptr, (uint16) crc);
							}
							else
								result = GUP_HDR_SIZE_OVF;
						}
					}
				}

				if (result == GUP_OK)
					gup_io_set_current(file, p);
			}
		}
	}

	/*
	 * Free all allocated temporary space.
	 */

	delete[] name_ptr;
	if (path_ptr != NULL)
		delete[] path_ptr;

	if (new_allocated)
		delete (lha_fileheader *) file_hdr;

	return result;
}

gup_result lha_archive::write_file_trailer(const fileheader *header)
{
	long current_pos;
	gup_result result;

	if ((result = tell(current_pos)) != GUP_OK)
		return result;

	if ((result = seek(header_pos, SEEK_SET)) != GUP_OK)
		return result;

	result = write_file_header(header);

	if (result != GUP_OK)
		return result;

	return seek(current_pos, SEEK_SET);
}

/*****************************************************************************
 *																			 *
 * Functions for reading an archive.										 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result lha_archive::open_archive(const char *name, OPTIONS *options,
 *										GUPMSG *msgfunc)
 *
 * Open an archive for reading.
 */

gup_result lha_archive::open_archive(const char *name, OPTIONS *options,
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
 * gup_result lha_archive::open_next_volume(void)
 *
 * Open the next volume of an archive. Return an error, because
 * multiple volume LHA archives are not supported.
 */

gup_result lha_archive::open_next_volume(void)
{
	return GUP_INTERNAL;
}

/*
 * gup_result lha_archive::find_header(int first, int &hdr_type, int &hdr_len)
 *
 * Find a header in the LHA file. If this function completes
 * successfully, the file buffer contains the entire header.
 *
 * first		- first header in file flag.
 * hdr_type		- contains on return the header type:
 *				  0 - main header (archive comment) found
 *				  1 - level 0 header
 *				  2 - level 1 header
 *				  3 - level 2 header
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

gup_result lha_archive::find_header(int first, int &hdr_type, int &hdr_len)
{
	uint8 *buf_ptr;
	unsigned long bytes_left;
	long arcpos;
	static const char comment[] = "-com-";
	int cont = TRUE, loop_cnt = 0;
	gup_result result;

	/*
	 * Get current position and bytes left in buffer.
	 */

	buf_ptr = gup_io_get_current(file, &bytes_left);

	do
	{
		/*
		 * If searching for the first header of the archive, check if
		 * there is a valid main header at the current position.
		 */

		if (first)
		{
			if (bytes_left < 6)
			{
				if ((result = gup_io_fill(file)) != GUP_OK)
					return result;

				buf_ptr = gup_io_get_current(file, &bytes_left);
			}

			if ((bytes_left >= 6) && (memcmp(buf_ptr, comment, strlen(comment)) == 0))
			{
				int i = 0;

				/*
				 * Scan for null character terminating the comment.
				 * Stop if the length of the comment exceeds the
				 * maximum LHA comment length.
				 */

				while ((i <= MAX_ARC_COMM_LEN) &&
					   ((unsigned long) (5 + i) < bytes_left) &&
					   (buf_ptr[5 + i] != 0))
				{
					i++;

					if (bytes_left <= (unsigned long) (5 + i))
					{
						if ((result = gup_io_fill(file)) != GUP_OK)
							return result;
						buf_ptr = gup_io_get_current(file, &bytes_left);
					}
				}

				if ((i <= MAX_ARC_COMM_LEN) &&
					((unsigned long) (5 + i) < bytes_left) &&
					(buf_ptr[5 + i] == 0))
				{
					hdr_type = 0;		/* Main header found. */
					hdr_len = 6 + i;	/* Size of header. */
					return GUP_OK;
				}
			}
		}

		/*
		 * Check if there is a file header at the current position.
		 * First make sure there are enough bytes in the buffer for
		 * the first part of the header.
		 */

		if (bytes_left < 22)
		{
			if ((result = gup_io_fill(file)) != GUP_OK)
				return result;

			buf_ptr = gup_io_get_current(file, &bytes_left);
		}

		if ((bytes_left >= 22) &&				/* Check if enough bytes are available. */
			/*(buf_ptr[HDR_LEVEL] >= 0) && */
			(buf_ptr[HDR_LEVEL] <= 2) &&		/* Check for valid header levels. */
			(buf_ptr[HDR_METHOD_ID] == '-') &&	/* Check for '-' at start and end of the method ID. */
			(buf_ptr[HDR_METHOD_ID+4] == '-'))
		{
			unsigned long hdr_size;
			int hdr_level;

			hdr_level = buf_ptr[HDR_LEVEL];
			hdr_size = (hdr_level == 2) ? get_word(buf_ptr + HDR_SIZE) : buf_ptr[HDR_SIZE];

			if (hdr_level == 2)
			{
				/*
				 * Header level 2. Check if the header length is correct.
				 * If true return with result code GUP_OK.
				 */

				if (hdr_size < HEADERSIZE_MAX)
				{
					if (bytes_left < hdr_size)
					{
						if ((result = gup_io_fill(file)) != GUP_OK)
							return result;

						buf_ptr = gup_io_get_current(file, &bytes_left);
					}

					if (bytes_left >= hdr_size)
					{
						int i = HDR_LV2_NEXT_HDR_SIZE;
						uint16 next_hdr_size = 0;
						unsigned long total_size = HDR_LV2_NEXT_HDR_SIZE + 2;
						uint8 *hdr_crc_ptr = NULL;

						while ((total_size <= hdr_size) && ((next_hdr_size = get_word(buf_ptr + i)) != 0))
						{
							if ((next_hdr_size == 5) && (buf_ptr[i+2] == 0))
								hdr_crc_ptr = &buf_ptr[i+3];
							total_size += next_hdr_size;
							i += next_hdr_size;
						}

						if ((next_hdr_size == 0) && (total_size == hdr_size))
						{
							static const uint8 zero[2] = { 0, 0 };

							/*
							 * Valid header found. If found at the correct
							 * position or if opt_repair is set, return
							 * GUP_OK, otherwise return GUP_BROKEN.
							 */

							hdr_type = 3;	/* Level 2 header. */
							hdr_len = (int) hdr_size;	/* Size of header. */

							if (first || (loop_cnt == 0) || opt_repair)
							{
								if (hdr_crc_ptr != NULL)
								{
									uint16 crc, real_crc;

									/*
									 * Check the header CRC.
									 */

									crc = crc16(buf_ptr, hdr_crc_ptr - buf_ptr, lha_init_crc(), crc_table);
									crc = crc16(zero, 2, crc, crc_table);
									crc = crc16(hdr_crc_ptr + 2, (buf_ptr + hdr_size) -
												  (hdr_crc_ptr + 2), crc, crc_table);

									real_crc = get_word(hdr_crc_ptr);

									return (crc != real_crc) ? GUP_HDR_CRC_FAULT :
										   ((!first && (loop_cnt != 0)) ? GUP_HDR_POSITION : GUP_OK);
								}
								else
									return (!first && (loop_cnt != 0)) ? GUP_HDR_POSITION : GUP_OK;
							}
							else
								return GUP_BROKEN;
						}
					}
				}
			}
			else
			{
				/*
				 * Header level 0 or 1. Check checksum and length. If
				 * correct return with result code GUP_OK.
				 */

				if (bytes_left < (hdr_size + 2))
				{
					if ((result = gup_io_fill(file)) != GUP_OK)
						return result;

					buf_ptr = gup_io_get_current(file, &bytes_left);
				}

				if (bytes_left >= (hdr_size + 2))
				{
					uint8 checksum;
					int checksum_error;
					unsigned long name_length;

					checksum = calc_checksum(buf_ptr + HDR_METHOD_ID, hdr_size);
					checksum_error = (checksum == buf_ptr[HDR_CHECKSUM]) ? FALSE : TRUE;

					name_length = buf_ptr[HDR_NAME_LENGTH];

					if (hdr_level == 0)
					{
						/*
						 * Level 0 header. Check for valid length.
						 */

						if (((hdr_size - name_length) == 20) ||
							((hdr_size - name_length) == 22) ||
							((hdr_size - name_length) == 34))
						{
							hdr_type = 1;	/* Level 0 header. */
							hdr_len = (int) hdr_size + 2;	/* Size of header. */

							if (first || (loop_cnt == 0) || opt_repair)
								return (checksum_error) ? GUP_HDR_CRC_FAULT :
									   ((!first && (loop_cnt != 0)) ? GUP_HDR_POSITION : GUP_OK);
							else
								return GUP_BROKEN;
						}
					}
					else
					{
						/*
						 * Level 1 header. Check for valid lengths of
						 * the header and the extension headers.
						 */

						if ((hdr_size - name_length) == 25)
						{
							int i = HDR_LV1_NEXT_HDR_SIZE + (int) name_length;
							uint16 next_hdr_size = 0;
							unsigned long total_size = hdr_size + 2;

							/*
							 * Check if the total header length does not exceed
							 * the maximum length of this implementation and if
							 * the entire header can be read from the archive.
							 */

							while ((total_size <= HEADERSIZE_MAX) &&
								   (total_size <= bytes_left) &&
								   ((next_hdr_size = get_word(buf_ptr + i)) != 0))
							{
								total_size += next_hdr_size;
								i += next_hdr_size;

								if (bytes_left < total_size)
								{
									if ((result = gup_io_fill(file)) != GUP_OK)
										return result;

									buf_ptr = gup_io_get_current(file, &bytes_left);
								}
							}

							if ((next_hdr_size == 0) &&
								(total_size <= HEADERSIZE_MAX) &&
								(total_size <= bytes_left))
							{
								/*
								 * Valid header found. If found at the correct
								 * position or if opt_repair is set, return
								 * GUP_OK, otherwise return GUP_BROKEN.
								 */

								hdr_type = 2;	/* Level 1 header. */
								hdr_len = (int) total_size;	/* Size of header. */

								if (first || (loop_cnt == 0) || opt_repair)
									return (checksum_error) ? GUP_HDR_CRC_FAULT :
										   ((!first && (loop_cnt != 0)) ? GUP_HDR_POSITION : GUP_OK);
								else
									return GUP_BROKEN;
							}
						}
					}
				}
			}
		}

		/*
		 * Check if scan should be continued starting at the next file
		 * position. If true, increase the current position in the file.
		 */

		if ((result = tell(arcpos)) != GUP_OK)
			return result;

		if ((first) && (arcpos >= MAXSFX) && (!opt_repair))
			cont = FALSE;
		else
		{
			if (bytes_left < 1)			/* Length of end of archive marker. */
			{
				if ((result = gup_io_fill(file)) != GUP_OK)
					return result;

				buf_ptr = gup_io_get_current(file, &bytes_left);
			}

			if (bytes_left > 1)
			{
				buf_ptr++;
				bytes_left--;
				gup_io_set_current(file, buf_ptr);
				loop_cnt++;
			}
			else
			{
				if ((bytes_left == 1) && (loop_cnt == 0) && (*buf_ptr == '\0'))
				{
					/*
					 * Valid end of archive marker found. Return GUP_END_ARCHIVE.
					 */

					return GUP_END_ARCHIVE;
				}
				else
				{
					/*
					 * No more bytes left in the file.If first is true, the
					 * end of the file has been reached before a valid header
					 * has been found, return GUP_NO_ARCHIVE. Otherwise return
					 * GUP_EOF.
					 */

					return (!first) ? GUP_EOF : GUP_NO_ARCHIVE;
				}
			}
		}
	} while (cont);

	return GUP_NO_ARCHIVE;
}

/*
 * mainheader *lha_archive::read_main_header(gup_result &result)
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
 * GUP_NO_ACRHIVE		- file is not an LHA archive
 * GUP_BROKEN			- archive is broken, but it might be possible
 *						  to recover data from the archive
 */

mainheader *lha_archive::read_main_header(gup_result &result)
{
	int hdr_type, hdr_size;

	result = find_header(TRUE, hdr_type, hdr_size);

	if ((result == GUP_OK) ||
		((result == GUP_HDR_CRC_FAULT) && opt_repair))
	{
		lha_mainheader *main_hdr;
		uint8 *buf_ptr;
		unsigned long bytes_left;
		const char *comment = NULL;

		/*
		 * No error or CRC error or end of archive. Create a lha_mainheader
		 * object using the data read from file.
		 */

		if (hdr_type == 0)				/* Check if header is archive comment. */
		{
			buf_ptr = gup_io_get_current(file, &bytes_left);

			buf_ptr += 5;				/* Skip '-com-'. */
			comment = (const char *) buf_ptr;
			buf_ptr += strlen(comment) + 1;

			gup_io_set_current(file, buf_ptr);
		}
		else
			result = GUP_OK;			/* No main header, therefore always result OK. */

		main_hdr = new lha_mainheader(comment);

		return (mainheader *) main_hdr;
	}
	else
	{
		/*
		 * If the result is GUP_END_ARCHIVE or GUP_END_VOLUME, set
		 * the result to GUP_NO_ARCHIVE.
		 */

		if ((result == GUP_END_ARCHIVE) || (result == GUP_END_VOLUME))
			result = GUP_NO_ARCHIVE;
		return NULL;
	}
}

/*
 * int conv_str_to_method(const char *str)
 *
 * Convert an lha method string to an internally used method number.
 *
 * Parameters:
 *
 * str	- pointer to string.
 *
 * Result: method or -1 if the method is unknown.
 */

static int conv_str_to_method(const char *str)
{
	int i = 0;

	while ((method_strings[i].method_str != NULL) &&
		   (strncmp(method_strings[i].method_str, str, 5) != 0))
		i++;

	return method_strings[i].method;
}

/*
 * gup_result conv_os_to_gup(uint8 lha_os, int &host_os)
 *
 * Convert an OS id read from a header to a GUP internal OS id.
 *
 * Parameters:
 *
 * lha_os	- Host id in LHA format.
 * host_os	- Host id in GUP format.
 *
 * Result:
 *
 * GUP_OK 				- no error
 * GUP_HDR_UNKNOWN_OS	- unknown OS id.
 */

static gup_result conv_os_to_gup(uint8 lha_os, int &host_os)
{
	switch(lha_os)
	{
	case 'M':
		host_os = OS_MSDOS;
		break;
	case '2':
		host_os = OS_OS_2;
		break;
	case '9':
		host_os = OS_OS9;
		break;
	case 'K':
		host_os = OS_OS_68K;
		break;
	case '3':
		host_os = OS_OS_386;
		break;
	case 'H':
		host_os = OS_HUMAN;
		break;
	case 'U':
		host_os = OS_UNIX;
		break;
	case 'C':
		host_os = OS_CP_M;
		break;
	case 'F':
		host_os = OS_FLEX;
		break;
	case 'm':
		host_os = OS_MAC;
		break;
	case 'R':
		host_os = OS_RUNSER;
		break;
	case 'T':
		host_os = OS_TOWNOS;
		break;
	case 'X':
		host_os = OS_XOSK;
		break;
	case 'A':
		host_os = OS_AMIGA;
		break;
	case 'a':
		host_os = OS_ATARI;
		break;
	default:
		return GUP_HDR_UNKNOWN_OS;
	}

	return GUP_OK;
}

/*
 * fileheader *lha_archive::read_file_header(gup_result &result)
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

fileheader *lha_archive::read_file_header(gup_result &result)
{
	int hdr_type, hdr_size;

	result = find_header(FALSE, hdr_type, hdr_size);

	if ((result == GUP_OK) || (((result == GUP_HDR_CRC_FAULT) ||
								(result == GUP_HDR_POSITION)) && opt_repair))
	{
		lha_fileheader *file_hdr;
		uint8 *ptr;
		unsigned long bytes_left;
		int name_length = 0, path_length = 0;
		char *name = NULL, *path = NULL, *os_name, *p;

		if (hdr_type == 0)
		{
			/*
			 * Main header found. Return invalid archive.
			 */

			result = GUP_NO_ARCHIVE;
			return NULL;
		}

		ptr = gup_io_get_current(file, &bytes_left);

		/*
		 * No error or CRC error. Create a lha_fileheader object using
		 * the data read from file.
		 */

		file_hdr = new lha_fileheader(NULL, NULL, (uint8)(hdr_type - 1));

		ptr += 2;						/* Skip header length and checksum. */

		if ((file_hdr->method = conv_str_to_method((const char *) ptr)) < 0)
		{
			if (result == GUP_OK)
				result = GUP_HDR_UNKNOWN_METHOD;	/* Unknown method. */
			file_hdr->method = LHA_LH0_;
		}

		ptr += 5;						/* Skip method. */

		if (file_hdr->method == LHA_LHD_)
			file_hdr->file_type = DIR_TYPE;	/* File type. */
		else
			file_hdr->file_type = BINARY_TYPE;	/* File type. */

		file_hdr->compsize = get_longword_i(ptr);
		file_hdr->origsize = file_hdr->stat.length = get_longword_i(ptr);
		file_hdr->offset = 0;

		file_hdr->orig_time_stamp = get_longword_i(ptr);
		file_hdr->stat.mtime = lha_conv_to_os_time(file_hdr->orig_time_stamp, hdr_type - 1);

		file_hdr->msdos_file_mode = *ptr++;
		if (file_hdr->file_type == DIR_TYPE)
			file_hdr->msdos_file_mode |= 0x10;	/* Make sure the file attributes are correct. */
		file_hdr->stat.file_mode = mode_dos_to_os(file_hdr->msdos_file_mode);

		ptr++;							/* Skip header level. */

		if (hdr_type <= 2)				/* Level 0 or level 1 header. */
		{
			/*
			 * Store pointer to filename.
			 */

			name_length = *ptr++;
			name = (char *) ptr;
			ptr += name_length;
		}

		/*
		 * Get CRC and host OS if available.
		 */

		if ((hdr_type > 1) || ((hdr_type == 1) && ((hdr_size - name_length) > 22)))
		{
			/*
			 * Header contains a file CRC. Get it.
			 */

			file_hdr->flg_has_crc = TRUE;
			file_hdr->file_crc = (uint32) get_word_i(ptr);

			if ((hdr_type != 1) || ((hdr_size - name_length) == 36))
			{
				/*
				 * Header contains host OS ID. Get it.
				 */

				gup_result tmp_result;

				if ((tmp_result = conv_os_to_gup(*ptr++, file_hdr->host_os)) != GUP_OK)
				{
					if (result == GUP_OK)
						result = tmp_result;
					file_hdr->host_os = OS_GENERIC;
				}
			}
			else
				file_hdr->host_os = OS_GENERIC;
		}
		else
		{
			file_hdr->flg_has_crc = FALSE;	/* No CRC. */
			file_hdr->host_os = OS_GENERIC;
		}

		/*
		 * Get extended data from header.
		 */

		if (hdr_type == 1)
		{
			/*
			 * Level 0 header. If the OS is UNIX read the UNIX
			 * extensions of the level 0 header.
			 */

			if (file_hdr->host_os == OS_UNIX)
			{
				file_hdr->minor_version = *ptr++;
				file_hdr->unix_time_stamp = get_longword_i(ptr);
				file_hdr->unix_file_mode = get_word_i(ptr);
				file_hdr->unix_uid = get_word_i(ptr);
				file_hdr->unix_gid = get_word_i(ptr);
#if (OS == OS_UNIX)
				file_hdr->stat.file_mode = mode_unix_to_os(file_hdr->unix_file_mode);
				file_hdr->stat.mtime = time_unix_to_os(file_hdr->unix_time_stamp);
				file_hdr->stat.uid = file_hdr->unix_uid;
				file_hdr->stat.gid = file_hdr->unix_gid;
#endif
			}
		}
		else
		{
			uint16 ext_hdr_size;
			uint8 *start;

			/*
			 * Level 1 or 2 header.
			 */

			ext_hdr_size = get_word_i(ptr);
			start = ptr;

			while(ext_hdr_size != 0)
			{
				uint8 ext_hdr_type;
				uint8 old_ch;
#if (OS == OS_UNIX)
				int uid_valid = FALSE, gid_valid = FALSE;
#endif

				ext_hdr_type = *ptr;

				if ((ext_hdr_type > 2) && (ext_hdr_type != 0x3F))
				{
					/*
					 * Store every extended header except extended header
					 * type 0, 1, 2 and 0x3F. These extended headers are
					 * reconstructed from other data in the header class.
					 */

					file_hdr->add_ext_hdr(ext_hdr_type, ptr, ext_hdr_size - 2);
				}

				/*
				 * Extract data from the extended header.
				 */

				switch(ext_hdr_type)
				{
				case 1:				/* Filename. */
					name = (char *) (ptr + 1);
					name_length = ext_hdr_size - 3;
					break;
				case 2:				/* Directory. */
					path = (char *) (ptr + 1);
					path_length = ext_hdr_size - 3;
					break;
				case 0x3F:			/* Comment. */
					old_ch = *(ptr + ext_hdr_size - 2);
					*(ptr + ext_hdr_size - 2) = 0;
					file_hdr->set_comment((const char *)(ptr + 1));
					*(ptr + ext_hdr_size - 2) = old_ch;
					break;
#if (OS == OS_MSDOS) || (OS == OS_HUMAN) || (OS == OS_WIN32)
				case 0x40:			/* MS-DOS file attributes. */
					if ((file_hdr->host_os == OS_MSDOS) ||
						(file_hdr->host_os == OS_HUMAN) ||
						(file_hdr->host_os == OS_GENERIC))
						file_hdr->stat.file_mode = mode_dos_to_os((uint8) get_word(ptr + 1));
					break;
#endif
#if (OS == OS_UNIX)
				case 0x50:			/* UNIX file mode. */
					if (file_hdr->host_os == OS_UNIX)
						file_hdr->stat.file_mode = mode_unix_to_os(get_word(ptr + 1));
					break;
				case 0x51:			/* UNIX gid and uid. */
					if (file_hdr->host_os == OS_UNIX)
					{
						/*
						 * We always prefer the uid and gid from extended
						 * headers 0x52 and 0x53, but only if the user
						 * and groupname were valid.
						 */

						if (gid_valid == FALSE)
							file_hdr->stat.gid = get_word(ptr + 1);
						if (uid_valid == FALSE)
							file_hdr->stat.uid = get_word(ptr + 3);
					}
					break;
				case 0x52:			/* UNIX group name. */
					if (file_hdr->host_os == OS_UNIX)
					{
						char *grp_name;
						struct group *grp;

						grp_name = new char[ext_hdr_size - 2];
						strncpy(grp_name, (const char *)(ptr + 1), ext_hdr_size - 3);
						grp_name[ext_hdr_size - 3] = 0;

#ifdef HAVE_GETGRNAM_R
						char buffer[2048];
						struct group grp_buf;

						if (getgrnam_r(grp_name, &grp_buf, buffer, sizeof(buffer),
									   &grp) == 0)
#else
						if ((grp = getgrnam(grp_name)) != NULL)
#endif
						{
							gid_valid = TRUE;
							file_hdr->stat.gid = grp->gr_gid;
						}

						delete grp_name;
					}
					break;
				case 0x53:			/* UNIX user name. */
					if (file_hdr->host_os == OS_UNIX)
					{
						char *usr_name;
						struct passwd *password;

						usr_name = new char[ext_hdr_size - 2];
						strncpy(usr_name, (const char *)(ptr + 1), ext_hdr_size - 3);
						usr_name[ext_hdr_size - 3] = 0;

#ifdef HAVE_GETPWNAM_R
						char buffer[2048];
						struct passwd pwd;

						if (getpwnam_r(usr_name, &pwd, buffer, sizeof(buffer),
									   &password) == 0)
#else
						if ((password = getpwnam(usr_name)) != NULL)
#endif
						{
							uid_valid = TRUE;
							file_hdr->stat.uid = password->pw_uid;
						}

						delete usr_name;
					}
					break;
				case 0x54:			/* UNIX modification time. */
					if (file_hdr->host_os == OS_UNIX)
						file_hdr->stat.mtime = time_unix_to_os(get_longword(ptr + 1));
					break;
#endif
				}

				ptr += ext_hdr_size - 2;	/* Skip header. */

				ext_hdr_size = get_word_i(ptr);
			}

			if (hdr_type == 2)
			{
				/*
				 * Level 1 header. Correct the packed size.
				 */

				file_hdr->compsize -= (unsigned long)(ptr - start);
			}
		}

#if (OS == OS_UNIX) || (OS == OS_WIN32)
		file_hdr->stat.atime = file_hdr->stat.ctime = file_hdr->stat.mtime;
#elif (OS != OS_MSDOS)
#error "This OS is not supported."
#endif

		/*
		 * Get filename. Check if the name contains a '|'. If true the file
		 * is a symbolic link.
		 */

		os_name = lha_conv_to_os_name(name, name_length, path, path_length, file_hdr->host_os);

		if (((p = strchr(os_name, '|')) != NULL) && (file_hdr->file_type == DIR_TYPE))
		{
			*p++ = 0;
			file_hdr->file_type = SYMLINK_TYPE;
			file_hdr->set_linkname(p);
		}

		file_hdr->set_filename(os_name);
		delete[] os_name;

		gup_io_set_current(file, ptr);

		return (fileheader *) file_hdr;
	}
	else
		return NULL;
}

gup_result lha_archive::read_file_trailer(fileheader *header)
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
 * gup_result lha_archive::close_archive(int ok)
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

gup_result lha_archive::close_archive(int ok)
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
 * gup_result lha_archive::close_curr_volume(void)
 *
 * Close the current volume.
 *
 * Only call this function if the current volume is not the last volume
 * of the archive. If the volume is the last, use 'close_archive'.
 */

gup_result lha_archive::close_curr_volume(void)
{
	return GUP_INTERNAL;
}

/*
 * mainheader *lha_archive::init_main_header(const char *comment)
 *
 * Allocate an lha_mainheader structure and initiate it.
 * The result is a NULL pointer if there is an error (archive not
 * opened).
 */

mainheader *lha_archive::init_main_header(const char *comment)
{
	lha_mainheader *main_hdr;

	if (!opened)
		return NULL;

	main_hdr = new lha_mainheader(comment);

	return (mainheader *) main_hdr;
}

fileheader *lha_archive::init_file_header(const char *filename,
										  const char *comment,
										  const osstat *stat)
{
	lha_fileheader *file_hdr;

	if (!opened)
		return NULL;

	/*
	 * If 'stat' is NULL use the defaults from 'stat' in 'file_hdr'.
	 * Otherwise use the values from 'stat'.
	 */

	if (stat == NULL)
		file_hdr = new lha_fileheader(filename, comment, lha_hdr_level);	/* By default level 2 header. */
	else
		file_hdr = new lha_fileheader(filename, comment, stat, lha_hdr_level);	/* By default level 2 header. */

	return (fileheader *) file_hdr;
}

/*****************************************************************************
 *																			 *
 * CRC functions.															 *
 * 																			 *
 *****************************************************************************/

uint32 lha_archive::init_crc(void)
{
	return lha_init_crc();
}

uint32 lha_archive::calc_crc(uint8 *buf, long len, uint32 crc_in)
{
	return (uint32) crc16(buf, len, (uint16) crc_in, crc_table);
}

/*****************************************************************************
 *																			 *
 * Compression and decompression functions.									 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result lha_archive::skip_compressed_data(fileheader *header)
 *
 * Skip the compressed data of a file.
 *
 * Parameters:
 *
 * header	- file header of the file.
 *
 * Result: GUP_OK if no error.
 */

gup_result lha_archive::skip_compressed_data(fileheader *header)
{
	return seek(header->compsize, SEEK_CUR);
}

/*****************************************************************************
 *																			 *
 * Archive control function.												 *
 * 																			 *
 *****************************************************************************/

gup_result lha_archive::arcctl(int function, ... )
{
	gup_result result = GUP_INVAL;
	va_list parm;
	int ivalue, *iptr;

	va_start(parm, function);

	switch(function)
	{
	case ARC_FILE_TYPES:
		iptr = va_arg(parm, int *);
		*iptr = ARC_FT_REG_FILE | ARC_FT_DIR | ARC_FT_SYMLINK;
		result = GUP_OK;
		break;

	case ARC_MV_MODE:
		iptr = va_arg(parm, int *);
		*iptr = 0;
		result = GUP_OK;
		break;

	case ARC_LHA_HDR_LEVEL:
		ivalue = va_arg(parm, int);
		if ((ivalue >= 0) && (ivalue <= 2))
		{
			lha_hdr_level = ivalue;
			result = GUP_OK;
		}
		break;
	}

	va_end(parm);

	return result;
}

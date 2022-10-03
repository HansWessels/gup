/*
 * Archive manager.
 *
 *     GZIP main and file header classes.
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

#include <string.h>
#include <sys/types.h>

#if (OS == OS_UNIX)
#include <grp.h>
#include <pwd.h>
#endif

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "arc_util.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "gz_hdr.h"
#include "gup_err.h"
#include "os.h"

/*****************************************************************************
 *																			 *
 * Member functions of the gzip_mainheader class.							 *
 *																			 *
 *****************************************************************************/

gzip_mainheader::gzip_mainheader(const char *comment) : mainheader(comment)
{
	hdr_type = MHDR_GZIP;
}

gzip_mainheader::gzip_mainheader(const gzip_mainheader& from) : mainheader(from)
{
}

gzip_mainheader::~gzip_mainheader(void)
{
}

/*****************************************************************************
 *																			 *
 * Member functions of the gzip_fileheader class.							 *
 *																			 *
 *****************************************************************************/

/*
 * void gzip_fileheader::update_file_attributes(void)
 *
 * Update all variables in that are related to file attributes.
 * 'stat' should be set before calling this function, because the
 * variables are updated based on the information in 'stat'.
 */

void gzip_fileheader::update_file_attributes(void)
{
	orig_time_stamp = gzip_conv_from_os_time(stat.mtime);
}

/*
 * void gzip_fileheader::init(const char *comment)
 *
 * Initialise all variables in the gzip_fileheader class.
 *
 * Parameters:
 *
 * comment	- pointer to file comment.
 */

void gzip_fileheader::init(const char *comment)
{
	hdr_type = FHDR_GZIP;

	/*
	 * Set defaults for GUP.
	 */

#if (OS == OS_MSDOS) || (OS == OS_UNIX) || (OS == OS_WIN32)
	host_os = OS;
#else
#error "This OS is not supported."
#endif

	flags = GZ_ORIG_FNAME;				/* Store original file name by default. */
	if (comment != NULL)
		flags |= GZ_COMMENT;			/* Store comment. */
	extra_flags = 0;
	part_number = 0;					/* Multi part volume part number. */

	update_file_attributes();			/* Set all variables related to file attributes. */
	update_ext_hdrs();					/* Create extended headers. */
}

gzip_fileheader::gzip_fileheader(const char *filename, const char *comment) : fileheader(filename, comment)
{
	init(comment);
}

gzip_fileheader::gzip_fileheader(const char *filename, const char *comment,
								 const osstat *stat) : fileheader(filename, comment, stat)
{
	init(comment);
}

gzip_fileheader::~gzip_fileheader(void)
{
}

/*
 * int gzip_fileheader::get_header_len(void)
 *
 * Get the length the header would have on disk.
 */

int gzip_fileheader::get_header_len(void) const
{
	const char *comment;
	char *name;
	int fname_len, com_len, total_len = 0;
	const extended_header *cur_hdr;
	uint32 cur_type;
	const uint8 *cur_data;
	unsigned long cur_len;

	comment = get_comment();
	com_len = (comment != NULL) ? (int) strlen(comment) + 1 : 0;

	name = gzip_conv_from_os_name(get_filename());
	fname_len = (int) strlen(name) + 1;
	delete[] name;

	total_len = 10 + com_len;

	if (flags & GZ_CONT)
		total_len += 2;					/* Length of part number. */

	if (flags & GZ_ORIG_FNAME)
		total_len += fname_len;			/* Length of file name. */

	/*
	 * Determine the length of the extended headers.
	 */

	if (flags & GZ_EXTRA_FIELD)
	{
		total_len += 2;					/* Extra field length. */

		cur_hdr = first_ext_hdr(cur_type, cur_data, cur_len);

		while(cur_hdr != NULL)
		{
			total_len += 4 + (int) cur_len;	/* Sub field length. */
			cur_hdr = next_ext_hdr(cur_hdr, cur_type, cur_data, cur_len);
		}
	}

	return total_len;
}

/*
 * int gzip_fileheader::has_crc(void)
 *
 * Return if this fileheader contained a file CRC or not.
 *
 * Result: 1 if this header contains a file CRC, 0 if not.
 */

int gzip_fileheader::has_crc(void) const
{
	return 1;
}

/*
 * void gzip_fileheader::update_ext_hdrs(void)
 *
 * Update all extended headers based on the contents
 * of 'stat'.
 */

void gzip_fileheader::update_ext_hdrs(void)
{
	const extended_header *curhdr, *nexthdr;
	uint32 curtype, nexttype;
	const uint8 *data;
	unsigned long len;

	/*
	 * Delete all existing extended headers.
	 */

	curhdr = first_ext_hdr(curtype, data, len);

	while (curhdr != NULL)
	{
		nexthdr = next_ext_hdr(curhdr, nexttype, data, len);
		del_ext_hdr(curtype);

		curtype = nexttype;
		curhdr = nexthdr;
	}

	/*
	 * Create new extended headers.
	 */

	/*
	 * Check if there still are extended headers.
	 */

	if (first_ext_hdr(curtype, data, len))
		flags |= GZ_EXTRA_FIELD;
	else
		flags &= ~GZ_EXTRA_FIELD;
}

/*
 * void gzip_fileheader::set_file_stat(const osstat *stat)
 *
 * Change the file status. Update all variables which depend
 * on the file status (mode amd time in DOS format). The host
 * OS is changed to the current OS.
 *
 * Parameters:
 *
 * stat	- new file status.
 */

void gzip_fileheader::set_file_stat(const osstat *stat)
{
#if (OS == OS_MSDOS) || (OS == OS_UNIX) || (OS == OS_WIN32)
	host_os = OS;
#else
#error "This OS is not supported."
#endif

	fileheader::set_file_stat(stat);

	update_file_attributes();
	update_ext_hdrs();
}

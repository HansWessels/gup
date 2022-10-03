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

dump_mainheader::dump_mainheader(const char *comment) : mainheader(comment)
{
	hdr_type = MHDR_ARJ;

#if (OS == OS_MSDOS) || (OS == OS_UNIX) || (OS == OS_WIN32)
	host_os = OS;
#else
#error "This OS is not supported."
#endif

	filename = NULL;

	arj_nbr = 6;
	arj_x_nbr = 1;
	flags = 0;
	flags = PATHSYM_FLAG;

	arj_sec_nbr = 0;
	arj_secenv_len = 0;
	arj_secenv_fpos = 0;

	ctime = mtime = gup_time();
	arc_size = 0;
}

dump_mainheader::dump_mainheader(const dump_mainheader& from) : mainheader(from)
{
	if (from.filename == NULL)
		filename = NULL;
	else
	{
		filename = new char[strlen(from.filename) + 1];
		strcpy(filename, from.filename);
	}

	fspecpos_in_fname = from.fspecpos_in_fname;

	arj_nbr = from.arj_nbr;
	arj_x_nbr = from.arj_x_nbr;
	host_os = from.host_os;
	flags = from.flags;

	arj_sec_nbr = from.arj_sec_nbr;
	arj_secenv_len = from.arj_secenv_len;
	arj_secenv_fpos = from.arj_secenv_fpos;

	ctime = from.ctime;
	mtime = from.mtime;

	arc_size = from.arc_size;
}

dump_mainheader::~dump_mainheader(void)
{
	if (filename != NULL)
		delete[] filename;
}

/*
 * void dump_mainheader::set_filename(const char *filename)
 *
 * Change the filename. The position of the fspec is determined
 * automatically by this version.
 */

void dump_mainheader::set_filename(const char *filename)
{
	uint16 fspecpos;

	fspecpos = (uint16)(get_name(filename) - filename);
	set_filename(filename, fspecpos);
}

/*
 * void dump_mainheader::set_filename(const char *new_filename, uint16 fspecpos)
 *
 * Change the filename.
 */

void dump_mainheader::set_filename(const char *new_filename, uint16 fspecpos)
{
	if (filename != NULL)
	{
		delete[] filename;
		filename = NULL;
	}

	filename = new char[strlen(new_filename) + 1];
	copy_filename(filename, new_filename);

	fspecpos_in_fname = fspecpos;
}

/*****************************************************************************
 *																			 *
 * Member functions of the dump_fileheader class.							 *
 *																			 *
 *****************************************************************************/

/*
 * void dump_fileheader::update_file_attributes(void)
 *
 * Update all variables in that are related to file attributes.
 * 'stat' should be set before calling this function, because the
 * variables are updated based on the information in 'stat'.
 */

void dump_fileheader::update_file_attributes(void)
{
	orig_file_mode = mode_os_to_dos(stat.file_mode);
	orig_time_stamp = arj_conv_from_os_time(stat.mtime);
}

/*
 * void dump_fileheader::init(const char *filename)
 *
 * Initialise all variables in the dump_fileheader class.
 *
 * Parameters:
 *
 * filename	- File name of the file.
 */

void dump_fileheader::init(const char *filename)
{
	set_filename(filename);

	hdr_type = FHDR_ARJ;

#if (OS == OS_MSDOS) || (OS == OS_UNIX) || (OS == OS_WIN32)
	host_os = OS;
#else
#error "This OS is not supported."
#endif

	arj_nbr = 6;
	arj_x_nbr = 1;
	flags = 0;
	host_data = 0;
	flags = PATHSYM_FLAG;

	update_file_attributes();
	update_ext_hdrs();
}

dump_fileheader::dump_fileheader(const char *filename, const char *comment) : fileheader(NULL, comment)
{
	init(filename);
}

dump_fileheader::dump_fileheader(const char *filename, const char *comment,
							   const osstat *stat) : fileheader(NULL, comment, stat)
{
	init(filename);
}

dump_fileheader::~dump_fileheader(void)
{
}

/*
 * void dump_fileheader::set_filename(const char *filename)
 *
 * Change the filename. The position of the fspec is determined
 * automatically by this version.
 */

void dump_fileheader::set_filename(const char *filename)
{
	uint16 fspecpos;

	fspecpos = (uint16) ((filename == NULL) ? 0 : (get_name(filename) - filename));
	set_filename(filename, fspecpos);
}

/*
 * void dump_fileheader::set_filename(const char *new_filename, uint16 fspecpos)
 *
 * Change the filename.
 */

void dump_fileheader::set_filename(const char *new_filename, uint16 fspecpos)
{
	fileheader::set_filename(new_filename);
	fspecpos_in_fname = fspecpos;
}

/*
 * int dump_fileheader::get_header_len(void)
 *
 * Get the length the header would have on disk.
 */

int dump_fileheader::get_header_len(void) const
{
	int len;
	uint16 fspec_pos;
	char *arj_name;
	const char *os_name, *comment;
	const extended_header *cur_hdr;
	uint32 cur_type;
	const uint8 *cur_data;
	unsigned long cur_len;

	os_name = get_filename(fspec_pos);
	arj_name = arj_conv_from_os_name(os_name, fspec_pos, flags & PATHSYM_FLAG);
	comment = get_comment();

	len = 4 + 30 + ((flags & EXTFILE_FLAG) ? 4 : 0);
	len += strlen(arj_name) + 1;		/* Filename. */
	len += (comment != NULL) ? strlen(comment) + 1 : 1;	/* Comment. */
	len += 4;							/* Header CRC. */

	cur_hdr = first_ext_hdr(cur_type, cur_data, cur_len);

	while(cur_hdr != NULL)
	{
		len += 2 + (int) cur_len + 4;	/* Length + extended header + CRC. */
		cur_hdr = next_ext_hdr(cur_hdr, cur_type, cur_data, cur_len);
	}

	len += 2;							/* End of extended headers. */

	delete[] arj_name;

	return len;
}

/*
 * int dump_fileheader::has_crc(void)
 *
 * Return if this fileheader contained a file CRC or not.
 *
 * Result: 1 if this header contains a file CRC, 0 if not.
 */

int dump_fileheader::has_crc(void) const
{
	return 1;
}

/*
 * void dump_fileheader::update_ext_hdrs(void)
 *
 * Update all extended headers based on the contents
 * of 'stat'.
 */

void dump_fileheader::update_ext_hdrs(void)
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
	 * Create new extended headers. Currently none.
	 */
}

/*
 * void dump_fileheader::set_file_stat(const osstat *stat)
 *
 * Change the file status. Update all variables which depend
 * on the file status (mode and time in DOS format). The host
 * OS is changed to the current OS.
 *
 * Parameters:
 *
 * stat	- new file status.
 */

void dump_fileheader::set_file_stat(const osstat *stat)
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

/*
 * void dump_fileheader::mv_set_segment_cnt(int count)
 *
 * Indicates to the archive manager which segment of a file that is
 * split over multiple volumes is going to be compressed. This
 * function should be called before write_file_header().
 *
 * Parameters:
 *
 * count	- segment counter.
 */

void dump_fileheader::mv_set_segment_cnt(int count)
{
	/*
	 * Set the EXTFILE_FLAG if the count is larger than zero
	 * (if the segment is not the first segment.
	 */

	if (count > 0)
		flags |= EXTFILE_FLAG;

	/*
	 * Reset VOLUME_FLAG. If needed, it will be set again by
	 * encode().
	 */

	flags &= ~VOLUME_FLAG;

	/*
	 * Set 'offset' (the offset in the file of the next segment) to
	 * 'totalsize' (the total length of all previous segments).
	 */

	offset = totalsize;
}

/*
 * int fileheader::mv_is_continuation(void)
 *
 * Indicates if this file header is the header of the continuation
 * of a previous segment of the same file that has been split over
 * multiple volumes.
 *
 * Result: 0 if not a continuation, 1 if a continuation.
 */

int dump_fileheader::mv_is_continuation(void)
{
	return (flags & EXTFILE_FLAG) ? 1 : 0;
}

#endif // ENABLE_DUMP_OUTPUT_MODES


/*
 * Archive manager.
 *
 *     Base main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:03:31 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 215 $
 * $Log$
 * Revision 1.6  2000/09/03 13:03:31  wout
 * Better multiple volume support.
 *
 * Revision 1.5  2000/07/30 15:21:43  wout
 * Use better defaults for UNIX file mode. Added Win32 support.
 *
 * Revision 1.4  2000/07/16 17:11:10  hwessels
 * Updated to GUP 0.0.4
 *
 * Revision 1.3  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.2  1998/03/26 19:59:25  klarenw
 * Updated to last version of the compression engine. Cleaner interface.
 *
 * Revision 1.1  1997/12/24 22:54:49  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#include "gup.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#if (OS == OS_UNIX)
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#endif

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "gup_err.h"
#include "compress.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "os.h"

/*
 * Member functions of the extended_header class.
 */

extended_header::extended_header(void)
{
	next = NULL;
	hdr_data = NULL;
}

extended_header::~extended_header(void)
{
	if (hdr_data != NULL)
		delete[] hdr_data;
}

void extended_header::set_header_data(uint32 type, const uint8 *data, unsigned long len)
{
	hdr_type = type;
	hdr_data = new uint8[len];
	memcpy(hdr_data, data, len);
	hdr_len = len;
}

void extended_header::get_header_data(uint32 &type, const uint8 *&data, unsigned long &len) const
{
	type = hdr_type;
	data = hdr_data;
	len = hdr_len;
}

/*
 * Member functions of the mainheader class.
 */

mainheader::mainheader(const char *com)
{
	comment = NULL;
	set_comment(com);
}

mainheader::mainheader(const mainheader& from)
{
	if (from.comment == NULL)
		comment = NULL;
	else
	{
		comment = new char[strlen(from.comment) + 1];
		strcpy(comment, from.comment);
	}

	hdr_type = from.hdr_type;
}

mainheader::~mainheader(void)
{
	if (comment != NULL)
		delete[] comment;
}

/*
 * mainheader::void set_comment(const char *new_comment)
 *
 * Change the comment string. If 'new_comment' is NULL the old
 * comment is removed without allocating a new one.
 */

void mainheader::set_comment(const char *new_comment)
{
	if (comment != NULL)
	{
		delete[] comment;
		comment = NULL;
	}

	if (new_comment != NULL)
	{
		comment = new char[strlen(new_comment) + 1];
		strcpy(comment, new_comment);
	}
}

/*
 * Member functions of the fileheader class.
 */

void fileheader::init(const char *name, const char *com)
{
	filename = NULL;
	linkname = NULL;
	comment = NULL;
	ext_hdr_list = NULL;

	host_os = OS;
	file_type = gup_file_type(&stat);

	method = STORE;
	compsize = 0;
	origsize = stat.length;
	totalsize = 0;
	file_crc = real_crc = 0;
	offset = 0;

	fileheader::set_filename(name);
	set_comment(com);
}

fileheader::fileheader(const char *name, const char *com)
{
	/*
	 * Initiate 'stat'.
	 */

#if (OS == OS_UNIX)
	stat.mtime = stat.atime = stat.ctime = gup_time();
	stat.file_mode = mode_unix_to_os(S_IRUSR | S_IWUSR);
	stat.uid = getuid();
	stat.gid = getgid();

	stat.device = 0;
	stat.inode = 0;
	stat.link_cnt = 0;
	stat.length = 0;
#elif (OS == OS_MSDOS)
	stat.file_mode = mode_dos_to_os(0x20);
	stat.mtime = gup_time();

	stat.device = 0;
	stat.length = 0;
#elif (OS == OS_WIN32)
	stat.file_mode = mode_dos_to_os(0x20);
	stat.mtime = stat.atime = stat.ctime = gup_time();

	stat.length = 0;
#else
#error "This OS is not supported."
#endif

	init(name, com);
}

fileheader::fileheader(const char *name, const char *com, const osstat *st)
{
	stat = *st;
	init(name, com);
}

fileheader::~fileheader(void)
{
	extended_header *hdr_ptr, *next_ptr;

	if (comment != NULL)
		delete[] comment;

	if (filename != NULL)
		delete[] filename;

	hdr_ptr = ext_hdr_list;

	while(hdr_ptr != NULL)
	{
		next_ptr = hdr_ptr->next;
		delete hdr_ptr;
		hdr_ptr = next_ptr;
	}
}

/*
 * fileheader::void set_filename(const char *new_filename)
 *
 * Change the file name. If 'new_filename' is NULL the old
 * file name is removed without allocating a new one.
 */

void fileheader::set_filename(const char *new_filename)
{
	if (filename != NULL)
	{
		delete[] filename;
		filename = NULL;
	}

	if (new_filename != NULL)
	{
		filename = new char[strlen(new_filename) + 1];
		copy_filename(filename, new_filename);
	}
}

/*
 * fileheader::void set_linkname(const char *new_linkname)
 *
 * Change the link name. If 'new_linkname' is NULL the old
 * link name is removed without allocating a new one. The
 * file type is set to 'SYMLINK_TYPE'.
 */

void fileheader::set_linkname(const char *new_linkname)
{
	if (file_type != SYMLINK_TYPE)
		return;

	if (linkname != NULL)
	{
		delete[] linkname;
		linkname = NULL;
	}

	if (new_linkname != NULL)
	{
		linkname = new char[strlen(new_linkname) + 1];
		strcpy(linkname, new_linkname);
	}
}

/*
 * fileheader::void set_comment(const char *new_comment)
 *
 * Change the comment string. If 'new_comment' is NULL the old
 * comment is removed without allocating a new one.
 */

void fileheader::set_comment(const char *new_comment)
{
	if (comment != NULL)
	{
		delete[] comment;
		comment = NULL;
	}

	if (new_comment != NULL)
	{
		comment = new char[strlen(new_comment) + 1];
		strcpy(comment, new_comment);
	}
}

void fileheader::add_ext_hdr(uint32 ext_hdr_type, uint8 *ptr, unsigned long ext_hdr_len)
{
	extended_header *new_hdr, *hdr_ptr, *prev_ptr;

	new_hdr = new extended_header;
	new_hdr->set_header_data(ext_hdr_type, ptr, ext_hdr_len);

	hdr_ptr = ext_hdr_list;
	prev_ptr = NULL;

	while(hdr_ptr != NULL)
	{
		uint32 cur_type;
               const uint8 *cur_data;
		unsigned long cur_len;

		hdr_ptr->get_header_data(cur_type, cur_data, cur_len);

		if (cur_type == ext_hdr_type)
		{
			new_hdr->next = hdr_ptr->next;
			if (prev_ptr == NULL)
				ext_hdr_list = new_hdr;
			else
				prev_ptr->next = new_hdr;
			delete hdr_ptr;

			return;
		}

		prev_ptr = hdr_ptr;
		hdr_ptr = hdr_ptr->next;
	}

	if (prev_ptr == NULL)
		ext_hdr_list = new_hdr;
	else
		prev_ptr->next = new_hdr;
}

void fileheader::del_ext_hdr(uint32 ext_hdr_type)
{
	extended_header *hdr_ptr, *prev_ptr;

	hdr_ptr = ext_hdr_list;
	prev_ptr = NULL;

	while(hdr_ptr != NULL)
	{
		uint32 cur_type;
               const uint8 *cur_data;
		unsigned long cur_len;

		hdr_ptr->get_header_data(cur_type, cur_data, cur_len);

		if (cur_type == ext_hdr_type)
		{
			if (prev_ptr == NULL)
				ext_hdr_list = hdr_ptr->next;
			else
				prev_ptr->next = hdr_ptr->next;
			delete hdr_ptr;

			return;
		}

		prev_ptr = hdr_ptr;
		hdr_ptr = hdr_ptr->next;
	}
}

const extended_header *fileheader::first_ext_hdr(uint32 &type, const uint8 *&data,
												 unsigned long &len) const
{
	if (ext_hdr_list == NULL)
		return NULL;
	else
	{
		ext_hdr_list->get_header_data(type, data, len);
		return ext_hdr_list;
	}
}

const extended_header *fileheader::next_ext_hdr(const extended_header *current, uint32 &type,
												const uint8 *&data, unsigned long &len) const
{
	if ((current == NULL) || ((current = current->next) == NULL))
		return NULL;
	else
	{
		current->get_header_data(type, data, len);
		return current;
	}
}

/*
 * void fileheader::set_file_stat(const osstat *stat)
 *
 * Set the file status.
 *
 * Parameters:
 *
 * Pointer to file status structure.
 */

void fileheader::set_file_stat(const osstat *newstat)
{
	stat = *newstat;
}

/*
 * void fileheader::mv_set_segment_cnt(int count)
 *
 * Indicates to the archive manager which segment of a file that is
 * split over multiple volumes is going to be compressed. This
 * function should be called before write_file_header().
 *
 * Parameters:
 *
 * count	- segment counter.
 */

void fileheader::mv_set_segment_cnt(int count)
{
	(void) count;
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

int fileheader::mv_is_continuation(void)
{
	return 0;
}

/*
 * Archive manager.
 *
 *     LHA UNIX support functions.
 *
 * $Author: wout $
 * $Date: 2000-07-30 17:04:37 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 163 $
 * $Log$
 * Revision 1.2  2000/07/30 15:04:37  wout
 * Improvements in the conversion of filenames from OS to LHA format:
 * - If the file is a directory only use path name, leave the name
 *   empty.
 * - If the header level is 2, use 0xFF as seperator instead the
 *   platform specific path seperator.
 *
 * Revision 1.1  1998/12/28 14:58:10  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 */

#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "gup.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "support.h"
#include "gup_err.h"
#include "arc_util.h"
#include "os.h"

#define DELIM	((char) 0xFF)

uint32 lha_conv_from_os_time(ostime time, int hdr_type)
{
	if (hdr_type < 2)
		return time_unix_to_dos(time.time);
	else
		return time.time;
}

ostime lha_conv_to_os_time(uint32 lha_time, int hdr_type)
{
	ostime time;

	if (hdr_type < 2)
		time.time = time_dos_to_unix(lha_time);
	else
		time.time = lha_time;

	return time;
}

/*
 * char *copy_conv_fname_os(char *dest, const char *src, unsigned long len, int host_os)
 *
 * Convert a filename read from the archive to OS specific format and copy it.
 *
 * Parameters:
 *
 * dest		- destination
 * src		- source
 * len		- length of src
 * host_os	- OS id read from file header
 *
 * Result: pointer to end of dest.
 */

static char *copy_conv_fname_os(char *dest, const char *src, unsigned long len, int host_os)
{
	char *ptr = dest;

	switch(host_os)
	{
	case OS_UNIX:
		while(len > 0)
		{
			if (*src == DELIM)
				*ptr++ = '/';
			else
				*ptr++ = *src;
			src++;
			len--;
		}
		*ptr = 0;
		break;
	case OS_MAC:
		while(len > 0)
		{
			if (*src == DELIM)
				*ptr++ = '/';
			else if (*src == ':')
				*ptr++ = '/';
			else if (*src == '/')
				*ptr++ = ':';
			else
				*ptr++ = *src;
			src++;
			len--;
		}
		*ptr = 0;
		break;
	default:							/* OS_MSDOS, OS_GENERIC. */
		if (len >= 3)
		{
			if ((src[1] == ':') && (src[2] == '\\'))
				src += 3;
		}

		while(len > 0)
		{
			if (*src == DELIM)
				*ptr++ = '/';
			else if (*src == '\\')
				*ptr++ = '/';
			else
				*ptr++ = *src;
			src++;
			len--;
		}
		*ptr = 0;
		strlwr(dest);
		break;
	}

	return ptr;
}

/*
 * char *lha_conv_to_os_name(const char *lha_name, unsigned long name_length,
 *							 const char *lha_path, unsigned long path_length,
 *							 int host_os)
 *
 * Convert a file name read from a file header to OS format. The file name is
 * returned in a string allocated with new. Trailing slashes are removed
 * from the file name.
 *
 * Parameters:
 *
 * lha_name		- filename
 * name_length	- length of filename
 * lha_path		- name of directory
 * path_length	- length of name of directory
 * host_os		- OS id from file header
 *
 * Result: file name in OS format.
 */

char *lha_conv_to_os_name(const char *lha_name, unsigned long name_length,
						  const char *lha_path, unsigned long path_length,
						  int host_os)
{
	char *os_name, *dest;

	os_name = new char[name_length + path_length + 2];

	dest = os_name;

	dest = copy_conv_fname_os(os_name, lha_path, path_length, host_os);
	dest = copy_conv_fname_os(dest, lha_name, name_length, host_os);

	if (dest[-1] == '/')
		dest[-1] = 0;

	return os_name;
}

/*
 * void copy_conv_fname_lha(char *dest, const char *src, unsigned long len,
 *							int host_os, int hdr_level)
 *
 * Convert a filename from OS format to LHA format and copy it.
 *
 * Parameters:
 *
 * dest			- destination
 * src			- source
 * len			- length of src
 * host_os		- OS id from file header
 * hdr_level	- header level
 */

static void copy_conv_fname_lha(char *dest, const char *src, unsigned long len,
								int host_os, int hdr_level)
{
	char *ptr = dest;

	if (hdr_level == 2)
	{
		while(len > 0)
		{
			if (*src == '/')
				*ptr++ = DELIM;
			else
				*ptr++ = *src;
			src++;
			len--;
		}
		*ptr = 0;
	}
	else
	{
		switch(host_os)
		{
		case OS_UNIX:
			if (len > 0)
				strncpy(dest, src, len);
			ptr = dest + len;
			*ptr = 0;
			break;

		case OS_MAC:
			while(len > 0)
			{
				if (*src == ':')
					*ptr++ = '/';
				else if (*src == '/')
					*ptr++ = ':';
				else
					*ptr++ = *src;
				src++;
				len--;
			}
			*ptr = 0;
			break;

	 	default:						/* OS_MSDOS, OS_GENERIC. */
			while(len > 0)
			{
				if (*src == '/')
					*ptr++ = '\\';
				else
					*ptr++ = *src;
				src++;
				len--;
			}
			*ptr = 0;
			break;
		}
	}
}

/*
 * static void add_path_seperator(char *name, int host_os)
 *
 * Add a path seperator to the end of a file name.
 *
 * Parameters:
 *
 * name			- file name
 * host_os		- OS id from file header
 * hdr_level	- header level
 */

static void add_path_seperator(char *name, int host_os, int hdr_level)
{
	size_t length;

	length = strlen(name);

	if (hdr_level == 2)
	{
		if (name[length - 1] != DELIM)
		{
			name[length] = DELIM;
			name[length + 1] = 0;
		}
	}
	else
	{
		switch(host_os)
		{
		case OS_UNIX:
			if (name[length - 1] != '/')
			{
				name[length] = '/';
				name[length + 1] = 0;
			}
			break;

		 case OS_MAC:
			if (name[length - 1] != ':')
			{
				name[length] = ':';
				name[length + 1] = 0;
			}
			break;

		 default:						/* OS_MSDOS, OS_GENERIC. */
			if (name[length - 1] != '\\')
			{
				name[length] = '\\';
				name[length + 1] = 0;
			}
			break;
		}
	}
}

/*
 * void lha_conv_from_os_name(const char *os_name, int host_os, int hdr_level,
 *							  char *&lha_name, char *&lha_path)
 *
 * Convert a file name in OS format to LHA format and split the file name if the
 * header level is 1 or 2.
 *
 * os_name		- name of file in OS format.
 * host_os		- OS id from file header (i.e. the requested format).
 * hdr_level	- header level.
 * is_dir		- indicates if the file is a directory. If <> 0, add a
 * 				  trailing slash to the filename.
 * lha_name		- file name (level 1 and 2 headers) or path name (level 0 header).
 * lha_path		- directory name (level 1 and 2 headers). If the directory
 *				  name is empty or if the header level is 0, lha_path is NULL.
 */

void lha_conv_from_os_name(const char *os_name, int host_os, int hdr_level,
						   int is_dir, char *&lha_name, char *&lha_path)
{
	if (hdr_level == 0)
	{
		lha_name = new char[strlen(os_name) + 2];
		copy_conv_fname_lha(lha_name, os_name, strlen(os_name), host_os,
							hdr_level);
		lha_path = NULL;

		if (is_dir)
			add_path_seperator(lha_name, host_os, hdr_level);
	}
	else
	{
		const char *name_ptr;

		/*
		 * Copy the file name. If the file is a directory, the
		 * file name will be empty.
		 */

		if (is_dir)
			name_ptr = os_name + strlen(os_name);
		else
			name_ptr = get_name(os_name);
		lha_name = new char[strlen(name_ptr) + 1];
		copy_conv_fname_lha(lha_name, name_ptr, strlen(name_ptr), host_os,
							hdr_level);

		/*
		 * If the file name contained a path, copy and convert the
		 * path. Make sure the path is terminated with a path
		 * seperator.
		 */

		if (name_ptr != os_name)
		{
			lha_path = new char[(name_ptr - os_name) + 2];
			copy_conv_fname_lha(lha_path, os_name, (name_ptr - os_name),
								host_os, hdr_level);

			if (is_dir)
				add_path_seperator(lha_path, host_os, hdr_level);
		}
		else
			lha_path = NULL;
	}
}

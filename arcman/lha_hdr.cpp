/*
 * Archive manager.
 *
 *     LHA main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-07-30 17:25:03 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 170 $
 * $Log$
 * Revision 1.3  2000/07/30 15:25:03  wout
 * Added function update_file_attributes.
 *
 * Revision 1.2  2000/07/16 17:11:10  hwessels
 * Updated to GUP 0.0.4
 *
 * Revision 1.1  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
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
#include "lha_hdr.h"
#include "gup_err.h"
#include "os.h"

/*****************************************************************************
 *																			 *
 * Member functions of the lha_mainheader class.							 *
 *																			 *
 *****************************************************************************/

lha_mainheader::lha_mainheader(const char *comment) : mainheader(comment)
{
	hdr_type = MHDR_LHA;
}

lha_mainheader::lha_mainheader(const lha_mainheader& from) : mainheader(from)
{
}

lha_mainheader::~lha_mainheader(void)
{
}

/*****************************************************************************
 *																			 *
 * Member functions of the lha_fileheader class.							 *
 *																			 *
 *****************************************************************************/

/*
 * void lha_fileheader::update_file_attributes(void)
 *
 * Update all variables in that are related to file attributes.
 * 'stat' should be set before calling this function, because the
 * variables are updated based on the information in 'stat'.
 */

void lha_fileheader::update_file_attributes(void)
{
	orig_time_stamp = lha_conv_from_os_time(stat.mtime, lha_hdr_level);
	msdos_file_mode = mode_os_to_dos(stat.file_mode);

	minor_version = 0;

#if (OS == OS_UNIX)
	if (lha_hdr_level == 0)
	{
		unix_file_mode = mode_os_to_unix(stat.file_mode);
		unix_uid = stat.uid;
		unix_gid = stat.gid;
		unix_time_stamp = time_os_to_unix(stat.mtime);
	}
#endif
}

/*
 * void lha_fileheader::init(uint8 hdr_level)
 *
 * Initialise all variables in the lha_fileheader class.
 *
 * Parameters:
 *
 * hdr_level	- LHA header level of this header.
 */

void lha_fileheader::init(uint8 hdr_level)
{
	hdr_type = FHDR_LHA;

	/*
	 * Set defaults for GUP.
	 */

#if (OS == OS_MSDOS) || (OS == OS_UNIX)
	host_os = OS;
#elif (OS == OS_WIN32)
	host_os = OS_MSDOS;					/* Use OS id MSDOS on Win32 systems. */
#else
#error "This OS is not supported."
#endif

	flg_has_crc = 1;
	lha_hdr_level = hdr_level;

	update_file_attributes();			/* Set all variables related to file attributes. */
	update_ext_hdrs();					/* Create extended headers. */
}

lha_fileheader::lha_fileheader(const char *filename, const char *comment) : fileheader(filename, comment)
{
	init(2);
}

lha_fileheader::lha_fileheader(const char *filename, const char *comment,
							   uint8 hdr_level) : fileheader(filename, comment)
{
	init(hdr_level);
}

lha_fileheader::lha_fileheader(const char *filename, const char *comment,
							   const osstat *stat) : fileheader(filename, comment, stat)
{
	init(2);
}

lha_fileheader::lha_fileheader(const char *filename, const char *comment,
							   const osstat *stat, uint8 hdr_level) : fileheader(filename, comment, stat)
{
	init(hdr_level);
}

lha_fileheader::~lha_fileheader(void)
{
}

/*
 * int lha_fileheader::get_header_len(void)
 *
 * Get the length the header would have on disk.
 */

int lha_fileheader::get_header_len(void) const
{
	const char *comment;
	char *name_ptr, *path_ptr;
	int fname_len, pname_len, com_len, total_len = 0;
	const extended_header *cur_hdr;
	uint32 cur_type;
	const uint8 *cur_data;
	unsigned long cur_len;

	comment = get_comment();

	if (file_type == SYMLINK_TYPE)
	{
		char *name;

		name = new char[strlen(get_filename()) +
						strlen(get_linkname()) + 2];

		strcpy(name, get_filename());
		strcat(name, "|");
		strcat(name, get_linkname());

		lha_conv_from_os_name(name, host_os, lha_hdr_level, 0, name_ptr,
							  path_ptr);

		delete[] name;
	}
	else
		lha_conv_from_os_name(get_filename(), host_os, lha_hdr_level,
							  (file_type == DIR_TYPE), name_ptr, path_ptr);

	fname_len = (int) strlen(name_ptr);
	delete[] name_ptr;

	if (path_ptr != NULL)
	{
		pname_len = (int) strlen(path_ptr);
		delete[] path_ptr;
	}
	else
		pname_len = 0;

	com_len = (comment != NULL) ? (int) strlen(comment) : 0;

	switch(lha_hdr_level)
	{
	case 0:
		if (flg_has_crc)
		{
			if (host_os == OS_UNIX)
				total_len = 36;
			else
				total_len = 24;
		}
		else
			total_len = 22;
		return total_len + pname_len + fname_len;
	case 1:
		total_len = 27 + fname_len;
		if (pname_len != 0)
			total_len += 3 + pname_len;
		break;
	case 2:
		total_len = 26 + 3 + fname_len;
		if (pname_len != 0)
			total_len += 3 + pname_len;
		break;
	default:
		return 0;						/* Unknown header level. */
	}

	/*
	 * Level 1 or 2 header. Determine the length of the extended
	 * headers.
	 */

	if (com_len)
		total_len += 3 + com_len;

	cur_hdr = first_ext_hdr(cur_type, cur_data, cur_len);

	while(cur_hdr != NULL)
	{
		total_len += 2 + (int) cur_len;
		cur_hdr = next_ext_hdr(cur_hdr, cur_type, cur_data, cur_len);
	}

	return total_len;
}

/*
 * int lha_fileheader::has_crc(void)
 *
 * Return if this fileheader contained a file CRC or not.
 *
 * Result: 1 if this header contains a file CRC, 0 if not.
 */

int lha_fileheader::has_crc(void) const
{
	return flg_has_crc;
}

/*
 * void lha_fileheader::update_ext_hdrs(void)
 *
 * Update all extended headers based on the contents
 * of 'stat'.
 */

void lha_fileheader::update_ext_hdrs(void)
{
	const extended_header *curhdr, *nexthdr;
	uint32 curtype, nexttype;
	const uint8 *data;
	unsigned long len;
	uint8 buffer[2048];
#if defined(HAVE_GETGRGID_R) || defined (HAVE_GETPWUID_R)
	char tmp[2048];
#endif

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

	if (lha_hdr_level == 0)
		return;

	/*
	 * Create new extended headers.
	 */

#if (OS == OS_UNIX)
	buffer[0] = 0x50;
	put_word(&buffer[1], (uint16) mode_os_to_unix(stat.file_mode));
	add_ext_hdr(0x50, buffer, 3);

	buffer[0] = 0x51;
	put_word(&buffer[1], (uint16) stat.gid);
	put_word(&buffer[3], (uint16) stat.uid);
	add_ext_hdr(0x51, buffer, 5);

	struct group *grp;
#ifdef HAVE_GETGRGID_R
	struct group grp_buf;

	if (getgrgid_r(stat.gid, &grp_buf, tmp, sizeof(tmp), &grp) == 0)
#else
	if ((grp = getgrgid(stat.gid)) != NULL)
#endif
	{
		buffer[0] = 0x52;
		strcpy((char *) &buffer[1], grp->gr_name);
		add_ext_hdr(0x52, buffer, strlen(grp->gr_name) + 1);
	}

	struct passwd *password;
#ifdef HAVE_GETPWNAM_R
	struct passwd pwd;

	if (getpwuid_r(stat.uid, &pwd, tmp, sizeof(tmp), &password) == 0)
#else
	if ((password = getpwuid(stat.uid)) != NULL)
#endif
	{
		buffer[0] = 0x53;
		strcpy((char *) &buffer[1], password->pw_name);
		add_ext_hdr(0x53, buffer, strlen(password->pw_name) + 1);
	}

	if (lha_hdr_level == 1)
	{
		buffer[0] = 0x54;
		put_long(&buffer[1], (uint32) time_os_to_unix(stat.mtime));
		add_ext_hdr(0x54, buffer, 5);
	}
#elif (OS == OS_MSDOS) || (OS == OS_WIN32)
	buffer[0] = 0x40;
	put_word(&buffer[1], (uint16) mode_os_to_dos(stat.file_mode));
	add_ext_hdr(0x40, buffer, 3);
#else
#error "This OS is not supported."
#endif
}

/*
 * void lha_fileheader::set_file_stat(const osstat *stat)
 *
 * Change the file status. Update all variables which depend
 * on the file status (mode amd time in DOS format). The host
 * OS is changed to the current OS.
 *
 * Parameters:
 *
 * stat	- new file status.
 */

void lha_fileheader::set_file_stat(const osstat *stat)
{
#if (OS == OS_MSDOS) || (OS == OS_UNIX)
	host_os = OS;
#elif (OS == OS_WIN32)
	host_os = OS_MSDOS;					/* Use OS id MSDOS on Win32 systems. */
#else
#error "This OS is not supported."
#endif

	fileheader::set_file_stat(stat);

	update_file_attributes();
	update_ext_hdrs();
}

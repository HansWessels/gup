/*
 * Archive manager.
 *
 *     GZIP Win32 support functions.
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

#if (OS == OS_WIN32)

#include "sysdep/arcman.h"
#include "header.h"
#include "support.h"
#include "gup_err.h"
#include "arc_util.h"
#include "os.h"

uint32 gzip_conv_from_os_time(ostime time)
{
	WORD tmp_time, tmp_date;

	/*
	 * !!! This should be replaced by a function that directly
	 * converts a time in Win32 format to a time in UNIX format.
	 */

	FileTimeToDosDateTime(&time.time, &tmp_date, &tmp_time);

	return time_dos_to_unix(((uint32) tmp_date) << 16 | (uint32) tmp_time);
}

ostime gzip_conv_to_os_time(uint32 gzip_time)
{
	ostime time;
	uint32 dos_time;

	/*
	 * !!! This should be replaced by a function that directly
	 * converts a time in UNIX format to a time in Win32 format.
	 */


	dos_time = time_unix_to_dos(gzip_time);
	DosDateTimeToFileTime((WORD)(dos_time >> 16), (WORD) dos_time, &time.time);

	return time;
}

/*
 * char *gzip_conv_to_os_name(const char *gzip_name)
 *
 * Convert a file name read from a file header to OS format. The file name is
 * returned in a string allocated with new.
 *
 * Parameters:
 *
 * gzip_name		- filename
 *
 * Result: file name in OS format.
 */

char *gzip_conv_to_os_name(const char *gzip_name)
{
	char *os_name;

	os_name = new char[strlen(gzip_name) + 1];
	strcpy(os_name, gzip_name);

	return os_name;
}

/*
 * void gzip_conv_from_os_name(const char *os_name)
 *
 * Convert a file name in OS format to gzip format.
 *
 * os_name		- name of file in OS format.
 *
 * Result: file name in gzip format.
 */

char *gzip_conv_from_os_name(const char *os_name)
{
	const char *name_ptr;
	char *gzip_name;

	name_ptr = get_name(os_name);

	gzip_name = new char[strlen(name_ptr) + 1];
	strcpy(gzip_name, name_ptr);

	return gzip_name;
}

#endif // OS

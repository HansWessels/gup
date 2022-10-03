/*
 * Archive manager.
 *
 *     ARJ Win32 support functions.
 *
 * $Author: wout $
 * $Date: 2000-07-30 16:27:17 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 157 $
 * $Log$
 * Revision 1.1  2000/07/30 14:27:17  wout
 * Added Win32 support.
 *
 */

#include "gup.h"

#include <string.h>
#include <sys/types.h>
#include <windows.h>

#include "arc_util.h"
#include "gup_err.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "os.h"

uint32 arj_conv_from_os_time(ostime time)
{
	WORD tmp_time, tmp_date;

	FileTimeToDosDateTime(&time.time, &tmp_date, &tmp_time);

	return ((uint32) tmp_date) << 16 | (uint32) tmp_time;
}

ostime arj_conv_to_os_time(uint32 arj_time)
{
	ostime time;

	DosDateTimeToFileTime((WORD)(arj_time >> 16), (WORD) arj_time, &time.time);

	return time;
}

/*
 * char *arj_conv_to_os_name(char *arj_name, uint16 &fspec_pos, int pathsym_flag)
 *
 * Convert a filename in ARJ format to OS format.
 */

char *arj_conv_to_os_name(const char *arj_name, uint16 &fspec_pos, int pathsym_flag)
{
	char *os_name, *dest;
	const char *src;

	(void) fspec_pos;

	os_name = new char[strlen(arj_name) + 1];
	src = arj_name;
	dest = os_name;

	while (*src)
	{
		if ((*src == '/') && pathsym_flag)
		{
			*dest++ = '\\';
			src++;
		}
		else
			*dest++ = *src++;
	}

	*dest = 0;

	return os_name;
}

/*
 * char *arj_conv_from_os_name(const char *arj_name, uint16 &fspec_pos, int pathsym_flag)
 *
 * Convert a filename in OS format to ARJ format.
 */

char *arj_conv_from_os_name(const char *os_name, uint16 &fspec_pos, int pathsym_flag)
{
	char *arj_name, *dest;
	const char *src;

	(void) fspec_pos;

	arj_name = new char[strlen(os_name) + 1];
	src = os_name;
	dest = arj_name;

	while (*src)
	{
		if ((*src == '\\') && pathsym_flag)
		{
			*dest++ = '/';
			src++;
		}
		else
			*dest++ = *src++;
	}

	*dest = 0;

	return arj_name;
}

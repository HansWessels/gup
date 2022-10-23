/*
 * Archive manager.
 *
 *     ARJ UNIX support functions.
 *
 * $Author: klarenw $
 * $Date: 1998-12-28 15:58:10 +0100 (Mon, 28 Dec 1998) $
 * $Revision: 136 $
 * $Log$
 * Revision 1.2  1998/12/28 14:58:10  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.1  1997/12/24 22:54:50  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "gup.h"
#include "arc_util.h"
#include "gup_err.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "os.h"

uint32 arj_conv_from_os_time(ostime time)
{
	return time_unix_to_dos(time.time);
}

ostime arj_conv_to_os_time(uint32 arj_time)
{
	ostime time;

	time.time = time_dos_to_unix(arj_time);

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

	os_name = new char[strlen(arj_name) + 1];
	src = arj_name;
	dest = os_name;

	if ((arj_name[1] == ':') && (arj_name[2] == '\\'))
	{
		src += 3;
		fspec_pos -= 3;
	}

	while (*src)
	{
		if ((*src == '\\') && !pathsym_flag)
		{
			*dest++ = '/';
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
		if ((*src == '/') && !pathsym_flag)
		{
			*dest++ = '\\';
			src++;
		}
		else
			*dest++ = *src++;
	}

	*dest = 0;

	return arj_name;
}

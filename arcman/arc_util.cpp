/*
 * Archive manager.
 *
 *     Utility functions.
 *
 * $Author: hwessels $
 * $Date: 2000-07-16 19:13:22 +0200 (Sun, 16 Jul 2000) $
 * $Revision: 143 $
 * $Log$
 * Revision 1.2  2000/07/16 17:11:10  hwessels
 * Updated to GUP 0.0.4
 *
 * Revision 1.1  1998/12/28 14:58:07  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 */

#include "gup.h"

#include "arc_util.h"

/*****************************************************************************
 *																			 *
 * Time conversion functions. Note that times in UNIX format are always		 *
 * in UTC and times in DOS format are always in local time.					 *
 * 																			 *
 *****************************************************************************/

/*
 * uint32 time_unix_to_dos(uint32 time)
 *
 * Convert a time in 32 bit UNIX format to a time in 32 bit MS-DOS
 * format.
 *
 * Parameters:
 *
 * time		- time in UNIX format.
 *
 * Result: time in MS-DOS format. The upper 16 bits contain the date,
 *         the lower 16 bits contain the time.
 */

uint32 time_unix_to_dos(uint32 time)
{
	struct tm *os_time;
	time_t tmp = (time_t) time;
	uint32 dos_time;

#ifdef HAVE_LOCALTIME_R
	struct tm time_buf;

	os_time = localtime_r(&tmp, &time_buf);
#else
	os_time = localtime(&tmp);
#endif

	dos_time = ((unsigned long) os_time->tm_sec / 2) |
			   ((unsigned long) os_time->tm_min << 5) |
			   ((unsigned long) os_time->tm_hour << 11) |
			   ((unsigned long) os_time->tm_mday << 16) |
			   (((unsigned long) os_time->tm_mon + 1) << 21) |
			   (((unsigned long) os_time->tm_year - 80) << 25);

	return dos_time;
}

/*
 * uint32 time_dos_to_unix(uint32 time)
 *
 * Convert a time in 32 bit MS-DOS format to a time in 32 bit UNIX
 * format.
 *
 * Parameters:
 *
 * time		- time in MS-DOS format. The upper 16 bits contain the date,
 *			  the lower 16 bits contain the time.
 *
 * Result: time in UNIX format.
 */

uint32 time_dos_to_unix(uint32 time)
{
	struct tm os_time;
	uint32 unix_time;

	os_time.tm_sec = (time & 0x1F) * 2;
	os_time.tm_min = (time >> 5) & 0x3F;
	os_time.tm_hour = (time >> 11) & 0x1F;
	os_time.tm_mday = (time >> 16) & 0x1F;
	os_time.tm_mon = ((time >> 21) & 0xF) - 1;
	os_time.tm_year = (time >> 25) + 80;
	os_time.tm_isdst = -1;

	unix_time = (uint32) mktime(&os_time);

	return unix_time;
}

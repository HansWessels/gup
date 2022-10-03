/*
 * Module:       atari.c
 * Description:  OS dependent functions for Atari
 *
 * $Author: wklaren $
 * $Date: 1996-08-07 14:00:26 +0200 (Wed, 07 Aug 1996) $
 * $Revision: 33 $
 * $Log$
 * Revision 1.1  1996/08/07 11:59:38  wklaren
 * Eerste versie van de ARJ shell. Deze versie maakt nog gebruik
 * van de xdialog++ bibliotheek.
 *
 */

#include <errno.h>
#include <limits.h>
#include <osbind.h>
#include <sys\stat.h>
#include <stdio.h>
#include <support.h>			/* MiNT lib specific functions. */
#include <unistd.h>
#include <utime.h>

#include "arj.h"
#include "os_dep.h"

typedef struct
{
	word time;
	word date;
} DOSTIME;

static int err_tab[40] =
{
	EINVAL, ENOENT, ENOENT, EMFILE, EACCES, EBADF, 0, ENOMEM, EFAULT, 0,
	0, 0, 0, 0, EIO, 0, EXDEV, 0, 0, 0,
	0, 0, 0, 0, 0, 0, EACCES, EACCES, 0, 0,
	0, 0, ERANGE, EINTERNAL, ENOEXEC, EFAULT, 0, 0, 0, 0
};

/*
 * void __tos2c(int error)
 *
 * Convert a TOS error code to a C error code.
 *
 * Parameters:
 *
 * error  - TOS error code
 *
 * Result: C error code
 */

static void __tos2c(int error)
{
	if (error < 0)
	{
		if ((error > -32) || (error < -72))
			errno = EIO;
		else
			errno = err_tab[-error - 32L];
	}
}

/*
 * arj_result set_file_info(const char *filename, ARJ_HEADER *header)
 *
 * Set the file attributes and the date to the correct value.
 *
 * Parameters:
 *
 * filename  - name of the file
 * header    - archive header of the file
 *
 * Result: ARJ_OK        if no error occured
 *         ARJ_C_ERROR   error in C function, error code is in 'errno'.
 */

arj_result set_file_info(const char *filename, ARJ_HEADER *header)
{
	if (header->host_os == OS_UNIX)
	{
		/*
		 * The file was packed on a Unix computer. Use the Unix
		 * functions to change the mode, the date, owner and
		 * group of the file.
		 */

		struct utimbuf dtim;

		if (chmod(filename, header->sdata.file.unix_mode))
			return ARJ_C_ERROR;

		dtim.actime = header->sdata.file.unix_atime;
		dtim.modtime = header->sdata.file.unix_mtime;

		if (utime(filename, &dtim))
			return ARJ_C_ERROR;

		if (chown(filename, header->sdata.file.unix_uid,
							header->sdata.file.unix_gid))
			return ARJ_C_ERROR;
	}
	else if ((header->host_os == OS_ATARI) || (header->host_os == OS_MSDOS))
	{
		/*
		 * The file was packed on an Atari or a MS-DOS computer.
		 * Just set the file attributes and the date using the
		 * standard TOS functions.
		 */

		int handle;
		DOSTIME dtim;
		char _filename[PATH_MAX];

		_unx2dos(filename, _filename);
		(void) Fattrib(_filename, 1, header->sdata.file.file_mode);

		if ((handle = Fopen(_filename, 0)) <= 0)
		{
			__tos2c(handle);
			return ARJ_C_ERROR;
		}

		dtim.time = (word)(header->sdata.file.time_stamp & 0xFFFF);
		dtim.date = (word)((header->sdata.file.time_stamp >> 16) & 0xFFFF);

		Fdatime(&dtim, handle, 1);
		Fclose(handle);
	}

	return ARJ_OK;
}

/*
 * arj_result make_writeable(const char *filename)
 *
 * Reset the read only bit of a file.
 *
 * Parameters:
 *
 * filename  - name of the file.
 *
 * Result: ARJ_OK
 */

arj_result make_writeable(const char *filename)
{
	char _filename[PATH_MAX];

	_unx2dos(filename, _filename);
	(void) Fattrib(_filename, 1, Fattrib(_filename, 0, 0) & ~0x01);

	return ARJ_OK;
}

/*
 * int is_readonly(const char *filename)
 *
 * Check if the read only bit of a file is set.
 *
 * Parameters:
 *
 * filename  - name of the file.
 *
 * Result: 1 if the file is read only, 0 otherwise.
 */

int is_readonly(const char *filename)
{
	char _filename[PATH_MAX];

	_unx2dos(filename, _filename);
	return (Fattrib(_filename, 0, 0) & 0x01) ? 1 : 0;
}

/*
 * int is_hidden(ARJ_HEADER *header)
 *
 * Check if the given file is a hidden file.
 *
 * Parameters:
 *
 * header  - archive header of the file.
 *
 * Result: 1 if the file is a hidden file, 0 otherwise.
 */

int is_hidden(ARJ_HEADER *header)
{
	if ((header->host_os == OS_ATARI) || (header->host_os == OS_MSDOS))
		return (header->sdata.file.file_mode & (FA_SYSTEM | FA_HIDDEN));
	else
		return 0;
}

/*
 * unsigned long convert_time(unsigned long arj_time)
 *
 * Convert a time in arj format to OS dependent format.
 *
 * Parameters:
 *
 * arj_time	- time in arj format
 *
 * Result: time in OS dependent format.
 */

#define SECS_PER_MIN		60L
#define SECS_PER_HOUR		3600L
#define SECS_PER_DAY		86400L
#define SECS_PER_YEAR		31536000L
#define SECS_PER_LEAPYEAR	(SECS_PER_YEAR + SECS_PER_DAY)
#define SECS_PER_4YEAR		(3 * SECS_PER_YEAR + SECS_PER_LEAPYEAR)

static int mth_start[13] = { 0L, 31L, 59L, 90L, 120L, 151L, 181L, 212L, 243L, 273L, 304L, 334L, 365L };

unsigned long convert_arj_time(unsigned long arj_time)
{
	int sec, min, hour;
	int mday, mon, year;
	time_t y, s;

	sec = (unsigned long)((arj_time & 0x1F) * 2);
	min = (unsigned long)((arj_time >> 5) & 0x3F);
	hour = (unsigned long)((arj_time >> 11) & 0x1F);

	mday = (unsigned long)((arj_time >> 16) & 0x1F);
	mon = (unsigned long)(((arj_time >> 21) & 0xF) - 1);
	year = (unsigned long)((arj_time >> 25) + 80);

	y = mday + mth_start[mon] +
		(((year % 4L) != 0L) ? 0L : ((mon > 1L) ? 1L : 0L));

	s = (sec) + (min * SECS_PER_MIN) + (hour * SECS_PER_HOUR) +
		(y * SECS_PER_DAY) + ((year - 70L) * SECS_PER_YEAR) +
		((year - 69L) / 4L) * SECS_PER_DAY;

	return s;
}

unsigned long convert_to_arj_time(unsigned long time)
{
	struct tm *loc_time;
	time_t tmp = time;
	unsigned long arj_time;

	loc_time = gmtime(&tmp);

	arj_time = ((unsigned long) loc_time->tm_sec / 2) |
						 ((unsigned long) loc_time->tm_min << 5) |
						 ((unsigned long) loc_time->tm_hour << 11) |
						 ((unsigned long) loc_time->tm_mday << 16) |
						 (((unsigned long) loc_time->tm_mon + 1) << 21) |
						 (((unsigned long) loc_time->tm_year - 80) << 25);

	return arj_time;
}

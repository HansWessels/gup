/*
 * Archive manager.
 *
 *     Win32 support functions.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:12:08 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 221 $
 * $Log$
 * Revision 1.2  2000/09/03 13:12:08  wout
 * Changes for compilation with Borland C++ Builder.
 *
 * Revision 1.1  2000/07/30 14:27:17  wout
 * Added Win32 support.
 */

#include "gup.h"

#if (OS == OS_WIN32)

#include "arc_util.h"
#include "gup_err.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "os.h"
#include "w32_err.h"

#define PATH_CHAR '\\'

/*
 * char *get_name(const char *path)
 *
 * Get a pointer to the filename in the path of file.
 *
 * Parameters:
 *
 * path		- full path of file.
 *
 * Result: pointer to filename.
 */

char *get_name(const char *path)
{
	char *ptr;

	if ((ptr = strrchr((char *) path, PATH_CHAR)) == NULL)
		return (char *) path;
	else
		return ptr + 1;
}

/*
 * void copy_filename(char *dest, const char *src)
 *
 * Copy a filename. Remove any trailing slashes.
 *
 * Parameters:
 *
 * dest	- destination string.
 * src	- source string.
 */

void copy_filename(char *dest, const char *src)
{
	while (*src)
		*dest++ = *src++;

	if (dest[-1] == PATH_CHAR)
		dest[-1] = 0;
	else
		*dest = 0;
}

/*
 * gup_result copy_file(const char *src, const char *dest)
 *
 * Copy a file. If the destination exists, it will be overwritten.
 *
 * Parameters:
 *
 * src			- name of source file
 * dest			- name of destination file
 *
 * Result: GUP_OK if succesfull
 */

gup_result copy_file(const char *src, const char *dest)
{
	if (CopyFileA(src, dest, FALSE) == FALSE)
		return gup_conv_win32_err(GetLastError());
	else
		return GUP_OK;
}

/*
 * gup_result move_file(const char *src, const char *dest)
 *
 * Move a file. If the destination exists, it will be overwritten.
 *
 * Parameters:
 *
 * src			- name of source file
 * dest			- name of destination file
 *
 * Result: GUP_OK if succesfull
 */

gup_result move_file(const char *src, const char *dest)
{
	if (DeleteFileA(dest) == FALSE)
	{
		DWORD error;

		if ((error = GetLastError()) != ERROR_FILE_NOT_FOUND)
			return gup_conv_win32_err(error);
	}

	if (MoveFileA(src, dest) == FALSE)
		return gup_conv_win32_err(GetLastError());

	return GUP_OK;
}

/*****************************************************************************
 *																			 *
 * Time / time conversion functions.										 *
 * 																			 *
 *****************************************************************************/

/*
 * ostime gup_time(void)
 *
 * Get the current time.
 *
 * Result: time in OS specfic format.
 */

ostime gup_time(void)
{
	ostime tmp;

	GetSystemTimeAsFileTime(&tmp.time);

	return tmp;
}

/*
 * ostime time_unix_to_os(uint32 unix_time)
 *
 * Convert a time in 32 bit UNIX format to a time in OS specific
 * format.
 *
 * Parameters:
 *
 * time		- time in UNIX format.
 *
 * Result: time in OS specific format.
 */

ostime time_unix_to_os(uint32 unix_time)
{
	ostime tmp;
	uint32 dos_time;

	/*
	 * !!! This function should be changed to convert the UNIX time
	 * directly to the Win32 time.
	 */

	dos_time = time_unix_to_dos(unix_time);
	DosDateTimeToFileTime((WORD)(dos_time >> 16), (WORD) dos_time, &tmp.time);

	return tmp;
}

/*
 * uint32 time_os_to_unix(ostime time)
 *
 * Convert a time in OS specific format to a time in 32 bit UNIX
 * format.
 *
 * Parameters:
 *
 * time		- time in OS specific format.
 *
 * Result: time in UNIX format.
 */

uint32 time_os_to_unix(ostime time)
{
	WORD tmp_time, tmp_date;

	/*
	 * !!! This function should be changed to convert the Win32 time
	 * directly to the UNIX time.
	 */

	FileTimeToDosDateTime(&time.time, &tmp_date, &tmp_time);

	return time_dos_to_unix(((uint32) tmp_date) << 16 | (uint32) tmp_time);
}

/*****************************************************************************
 *																			 *
 * File mode conversion functions.											 *
 * 																			 *
 *****************************************************************************/

/*
 * osmode mode_unix_to_os(uint16 mode)
 *
 * Convert a 16 bit UNIX file mode to a file mode in OS specific
 * format.
 *
 * Parameters:
 *
 * mode	- mode in UNIX format.
 *
 * Result: mode in OS specific format.
 */

osmode mode_unix_to_os(uint16 mode)
{
	osmode tmp;

	tmp.mode = FILE_ATTRIBUTE_ARCHIVE;	/* By default archive bit is set. */

	if (!(mode & 0x80))					/* Read only file. */
		tmp.mode |= FILE_ATTRIBUTE_READONLY;
	if ((mode & 0xF000) == 0x4000)		/* Directory. */
		tmp.mode |= FILE_ATTRIBUTE_DIRECTORY;

	return tmp;
}

/*
 * uint16 mode_os_to_unix(osmode mode)
 *
 * Convert a file mode in OS specific format to a 16 bit UNIX file
 * mode.
 *
 * Parameters:
 *
 * mode	- mode in OS specific format.
 *
 * Result: mode in UNIX format.
 */

uint16 mode_os_to_unix(osmode mode)
{
	uint16 tmp;

	tmp = 0x180;						/* Default: read and write only for owner. */

	if (mode.mode & FILE_ATTRIBUTE_READONLY)	/* Read only, clear write bits. */
		tmp &= ~0x80;
	if (mode.mode & FILE_ATTRIBUTE_DIRECTORY)	/* Directory. */
		tmp |= 0x4000 | 0x40;			/* Set type and execute bit. */

	return tmp;
}

/*
 * osmode mode_dos_to_os(uint8 mode)
 *
 * Convert a 8 bit DOS file mode to a file mode in OS specific
 * format.
 *
 * Parameters:
 *
 * mode	- mode in DOS format.
 *
 * Result: mode in OS specific format.
 */

osmode mode_dos_to_os(uint8 mode)
{
	osmode tmp;

	tmp.mode = (DWORD) mode;

	return tmp;
}

/*
 * uint8 mode_os_to_dos(osmode mode)
 *
 * Convert a file mode in OS specific format to a 8 bit DOS file
 * mode.
 *
 * Parameters:
 *
 * mode	- mode in OS specific format.
 *
 * Result: mode in DOS format.
 */

uint8 mode_os_to_dos(osmode mode)
{
	return (uint8)(mode.mode & 0x7F);
}

/*
 * ftype gup_file_type(const osstat *stat)
 *
 * Determine the GUP file type from the file status.
 *
 * Parameters:
 *
 * stat	- file status.
 *
 * Result: file type.
 */

ftype gup_file_type(const osstat *stat)
{
	if ((stat->file_mode.mode & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return DIR_TYPE;

	return BINARY_TYPE;
}

/*****************************************************************************
 *																			 *
 * File status functions.													 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result gup_stat(const char *name, osstat *st)
 *
 * Get the status of a file. If the file is a symbolic link,
 * the status of the file the link points to is returned.
 *
 * Parameters:
 *
 * name		- name of the file.
 * st		- contains on return the file status.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_stat(const char *name, osstat *st)
{
	HANDLE handle;
	WIN32_FIND_DATAA data;

	if ((handle = FindFirstFileA(name, &data)) == INVALID_HANDLE_VALUE)
		return gup_conv_win32_err(GetLastError());

	FindClose(handle);

	st->length = (unsigned long) data.nFileSizeLow;	/* File length. */

	st->file_mode.mode = data.dwFileAttributes;	/* File attributes. */
	st->ctime.time = data.ftCreationTime;	/* Creation time. */
	st->atime.time = data.ftLastAccessTime;	/* Last access time. */
	st->mtime.time = data.ftLastWriteTime;	/* Modification time. */

	return GUP_OK;
}

/*
 * gup_result gup_lstat(const char *name, osstat *st)
 *
 * Get the status of a file. If the file is a symbolic link,
 * the file status of the link is returned.
 *
 * Parameters:
 *
 * name		- name of the file.
 * st		- contains on return the file status.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_lstat(const char *name, osstat *st)
{
	return gup_stat(name, st);
}

#if 0
/*
 * gup_result gup_fstat(int handle, osstat *st)
 *
 * Get the status of a file.
 *
 * Parameters:
 *
 * handle	- handle of the file.
 * st		- contains on return the file status.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_fstat(int handle, osstat *st)
{
	struct stat s;

	if (fstat(handle, &s) != 0)
		return gup_conv_err(errno);

	st->device = s.st_dev;				/* Device. */
	st->inode = s.st_ino;				/* I node. */
	st->link_cnt = s.st_nlink;			/* Number of links. */
	st->length = s.st_size;				/* File length. */
	st->file_mode.mode = s.st_mode;		/* File attributes. */
	st->mtime.time = s.st_mtime;		/* Modification time. */
	st->atime.time = s.st_atime;		/* UNIX last access time. */
	st->ctime.time = s.st_ctime;		/* UNIX attribute change time. */
	st->uid = s.st_uid;					/* UNIX user id. */
	st->gid = s.st_gid;					/* UNIX group id. */

	return GUP_OK;
}
#endif

/*
 * gup_result gup_set_stat(const char *filename, const osstat *st)
 *
 * Change the status of a file.
 *
 * Parameters:
 *
 * filename	- name of the file.
 * st		- new status of the file.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_set_stat(const char *filename, const osstat *st)
{
	HANDLE handle;

	if (!(st->file_mode.mode & FILE_ATTRIBUTE_DIRECTORY))
	{
		if ((handle = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE,
								 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
								 NULL)) != INVALID_HANDLE_VALUE)
		{
			BOOL result;

			if ((result = SetFileAttributesA(filename, (st->file_mode.mode & 0x7F))) == TRUE)
				result = SetFileTime(handle, &st->ctime.time, &st->atime.time,
									 &st->mtime.time);
			CloseHandle(handle);

			if (result == FALSE)
				return gup_conv_win32_err(GetLastError());
		}
		else
			return gup_conv_win32_err(GetLastError());
	}

	return GUP_OK;
}

/*
 * int gup_is_dir(const osstat *stat)
 *
 * Test if a file is a directory.
 *
 * Parameters:
 *
 * stat	- file status of file.
 *
 * Result: 0 if the file is not a directory, 1 if the file is a directory.
 */

int gup_is_dir(const osstat *stat)
{
	return ((stat->file_mode.mode & FILE_ATTRIBUTE_DIRECTORY) != 0) ? 1 : 0;
}

/*
 * int gup_is_symlink(const osstat *stat)
 *
 * Test if a file is a symbolic link.
 *
 * Parameters:
 *
 * stat	- file status of file.
 *
 * Result: 0 if the file is not a symbolic link, 1 if the file is a symbolic
 *		   link.
 */

int gup_is_symlink(const osstat *stat)
{
	(void) stat;

	return 0;
}

/*****************************************************************************
 *																			 *
 * Other file functions.													 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result gup_mkdir(const char *dirname, osmode mode)
 *
 * Create a directory.
 *
 * Parameters:
 *
 * dirname	- name of the directory.
 * mode		- mode of the directory.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_mkdir(const char *dirname, osmode mode)
{
	(void) mode;

#if defined(HAVE__MKDIR)
	if (_mkdir(dirname) == -1)
		return gup_conv_err(errno);
	else
		return GUP_OK;
#elif defined(HAVE_MKDIR) 
	if (mkdir(dirname) == -1)
		return gup_conv_err(errno);
	else
		return GUP_OK;
#else
#error "b0rk b0rk b0rk"
#endif
}

/*
 * gup_result gup_mkdir(const char *dirname)
 *
 * Create a directory.
 *
 * Parameters:
 *
 * dirname	- name of the directory.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_mkdir(const char *dirname)
{
	if (mkdir(dirname) == -1)
		return gup_conv_err(errno);
	else
		return GUP_OK;
}

/*
 * gup_result gup_symlink(const char *oldname, const char *newname)
 *
 * Create a symbolic link to 'oldname' with name 'newname'. Not
 * supported on Win32.
 *
 * Parameters:
 *
 * oldname	- name of the file.
 * newmode	- name of the link.
 *
 * Result: GUP_INTERNAL.
 */

gup_result gup_symlink(const char *oldname, const char *newname)
{
	(void) oldname;
	(void) newname;

	return GUP_OK;
}

/*
 * gup_result gup_readlink(const char *filename, char **linkname)
 *
 * Read a symbolic link. Not supported on Win32.
 *
 * Parameters:
 *
 * filename	- name of the link.
 * linkname	- contains on return the name of the file the link
 *			  points to.
 *
 * Result: GUP_INTERNAL.
 */

gup_result gup_readlink(const char *filename, char **linkname)
{
	(void) filename;
	(void) linkname;

	return GUP_INTERNAL;
}

#endif // OS

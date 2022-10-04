/*
 * Archive manager.
 *
 *     UNIX support functions.
 *
 * $Author: wout $
 * $Date: 2000-07-30 16:40:41 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 158 $
 * $Log$
 * Revision 1.3  2000/07/30 14:40:41  wout
 * Added gup_mkdir function that does not need a mode input.
 * Improved DOS file attribute to UNIX mode converion.
 *
 * Revision 1.2  1998/12/28 14:58:10  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.1  1997/12/24 22:54:50  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>

#include "gup.h"
#include "gup_err.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "os.h"

#define PATH_CHAR '/'

#ifdef O_BINARY
#define OPEN_RD_FLAGS	(O_RDONLY | O_BINARY)
#define OPEN_WR_FLAGS	(O_WRONLY | O_CREAT | O_TRUNC | O_BINARY)
#else
#define OPEN_RD_FLAGS	O_RDONLY
#define OPEN_WR_FLAGS	(O_WRONLY | O_CREAT | O_TRUNC)
#endif

#define FMODE	(S_IWUSR | S_IWGRP | S_IWOTH | S_IRUSR | S_IRGRP | S_IROTH)

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
	const char *ptr;

	if ((ptr = strrchr(path, PATH_CHAR)) == NULL)
		return const_cast<char *>(path);
	else
		return const_cast<char *>(ptr + 1);
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
 *		   GUP_WRITE_ERROR if the disk is full
 */

gup_result copy_file(const char *src, const char *dest)
{
	int src_file, dest_file;
	gup_result result = GUP_OK;
	unsigned long tmp_buf_len;
	unsigned char *tmp_buf;

	/*
	 * Allocate temporary buffer.
	 */

	tmp_buf_len = 65536L;
	if((tmp_buf = (unsigned char *) malloc(tmp_buf_len)) == NULL)
		return GUP_NOMEM;

	/*
	 * Copy the file.
	 */

	if ((src_file = open(src, O_RDONLY)) < 0)
		result = gup_conv_err(errno);
	else
	{
		if ((dest_file = open(dest, O_WRONLY | O_CREAT | O_TRUNC, FMODE)) < 0)
			result = gup_conv_err(errno);
		else
		{
			long blk_size, l;

			do
			{
				if ((blk_size = read(src_file, tmp_buf, tmp_buf_len)) >= 0)
				{
					if ((l = write(dest_file, tmp_buf, (size_t)blk_size)) != blk_size)
						result = (l == -1) ? gup_conv_err(errno) : GUP_WRITE_ERROR;
				}
				else
					result = gup_conv_err(errno);
			} while (((unsigned long) blk_size == tmp_buf_len) && (result == GUP_OK));

			close(dest_file);
		}

		close(src_file);
	}

	free(tmp_buf);

	return result;
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
 *		   GUP_WRITE_ERROR if the disk is full
 */

gup_result move_file(const char *src, const char *dest)
{
	gup_result result;
	struct stat st_dest;
	struct stat st_src;
	char *src_dir, *dest_dir, *p;

	/*
	 * Find the names of the directories the files are in.
	 */

	src_dir = new char [strlen(src) + 1];
	dest_dir = new char [strlen(dest) + 1];

	strcpy(src_dir, src);
	strcpy(dest_dir, dest);

	if ((p = strrchr(src_dir, PATH_CHAR)) == NULL)
		strcpy(src_dir, ".");
	else
		*p = 0;

	if ((p = strrchr(dest_dir, PATH_CHAR)) == NULL)
		strcpy(dest_dir, ".");
	else
		*p = 0;

	/*
	 * Determine if the source directory and the destination directory
	 * are on the same device. If true, use 'rename', otherwise copy
	 * the file.
	 */

	if (stat(dest_dir, &st_dest) || stat(src_dir, &st_src))
		result = gup_conv_err(errno);
	else
	{
		if (st_dest.st_dev == st_src.st_dev)
		{
			/*
			 * The archive and the temporary archive are on the same disk.
			 * Use move.
			 */

			unlink(dest);				/* !!! Check if the error code is access denied. If true, return this error code. */
			if (rename(src, dest) != 0)
				result = gup_conv_err(errno);
			else
				result = GUP_OK;
		}
		else
		{
			/*
			 * The archive and the temporary archive are on a different
			 * disk. Use copy.
			 */

			if ((result = copy_file(src, dest)) == GUP_OK)
				unlink(src);
		}
	}

	delete[] src_dir;
	delete[] dest_dir;

	return result;
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

	tmp.time = time(NULL);

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

	tmp.time = unix_time;

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
	return time.time;
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

	tmp.mode = (mode_t) mode;

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
	return (uint16) mode.mode;
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

	tmp.mode = S_IRUSR | S_IWUSR;		/* Default only read and write
										   permission for the owner. */

	if (mode & 0x1)						/* Read only, clear write bit. */
		tmp.mode &= ~S_IWUSR;
	if (mode & 0x10)					/* Directory. */
		tmp.mode |= S_IFDIR | S_IXUSR;

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
	uint8 tmp = 0x20;					/* By default archive bit is set. */

	if (!(mode.mode & S_IWUSR))			/* Read only file. */
		tmp |= 0x1;
	if ((mode.mode & S_IFMT) == S_IFDIR)	/* Directory. */
		tmp |= 0x10;

	return tmp;
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
	if ((stat->file_mode.mode & S_IFMT) == S_IFDIR)
		return DIR_TYPE;
	if ((stat->file_mode.mode & S_IFMT) == S_IFLNK)
		return SYMLINK_TYPE;

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
	struct stat s;

	if (stat(name, &s) != 0)
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
	struct stat s;

	if (lstat(name, &s) != 0)
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
	struct utimbuf dtim;

	if (!S_ISLNK(st->file_mode.mode))
	{
		dtim.actime = st->atime.time;
		dtim.modtime = st->mtime.time;

		if (utime(filename, &dtim))
			return gup_conv_err(errno);

		if (chmod(filename, st->file_mode.mode))
			return gup_conv_err(errno);

		if (chown(filename, st->uid, st->gid))
		{
			if (errno != EPERM)			/* Ignore permission errors. */
				return gup_conv_err(errno);
		}
	}
#ifdef HAVE_LCHOWN
	else
	{
		if (lchown(filename, st->uid, st->gid))
		{
			if (errno != EPERM)			/* Ignore permission errors. */
				return gup_conv_err(errno);
		}
	}
#endif

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
	return ((stat->file_mode.mode & S_IFMT) == S_IFDIR) ? 1 : 0;
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
	return ((stat->file_mode.mode & S_IFMT) == S_IFLNK) ? 1 : 0;
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
	if (mkdir(dirname, mode.mode) == -1)
		return gup_conv_err(errno);
	else
		return GUP_OK;
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
	mode_t mode = (S_IWUSR | S_IWGRP | S_IWOTH |
				   S_IRUSR | S_IRGRP | S_IROTH |
				   S_IXUSR | S_IXGRP | S_IXOTH |
				   S_IFDIR);

	if (mkdir(dirname, mode) == -1)
		return gup_conv_err(errno);
	else
		return GUP_OK;
}

/*
 * gup_result gup_symlink(const char *oldname, const char *newname)
 *
 * Create a symbolic link to 'oldname' with name 'newname'.
 *
 * Parameters:
 *
 * oldname	- name of the file.
 * newmode	- name of the link.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_symlink(const char *oldname, const char *newname)
{
	if (symlink(oldname, newname) == -1)
		return gup_conv_err(errno);
	else
		return GUP_OK;
}

/*
 * gup_result gup_readlink(const char *filename, char **linkname)
 *
 * Read a symbolic link.
 *
 * Parameters:
 *
 * filename	- name of the link.
 * linkname	- contains on return the name of the file the link
 *			  points to.
 *
 * Result: GUP_OK if no error.
 */

gup_result gup_readlink(const char *filename, char **linkname)
{
	unsigned long size = 100;

	while(1)
	{
		long len;

		*linkname = new char[size];
		if ((len = readlink(filename, *linkname, size)) == -1)
		{
			delete[] *linkname;
			*linkname = NULL;

			return gup_conv_err(errno);
		}

		if (len < (long)size)
		{
			(*linkname)[len] = 0;
			return GUP_OK;
		}

		delete[] *linkname;
		size *= 2;
	}
}

/*
 * Archive manager.
 *
 *     Win32 specific declarations.
 *
 * $Author: wout $
 * $Date: 2000-07-30 16:27:17 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 157 $
 * $Log$
 * Revision 1.1  2000/07/30 14:26:11  wout
 * Added Win32 support.
 *
 */

typedef struct
{
	FILETIME time;
} ostime;

typedef struct
{
	DWORD mode;
} osmode;

typedef struct
{
	/*
	 * Note that device and length are currently not used
	 * by the archive manager, either because these variables are not
	 * stored in a header in an archive or because the archive manager
	 * stores this information somewhere else (length).
	 */

	unsigned long length;				/* File length. */

	osmode file_mode;					/* File attributes. */
	ostime ctime;						/* Creation time. */
	ostime atime;						/* Last access time. */
	ostime mtime;						/* Modification time. */
} osstat;

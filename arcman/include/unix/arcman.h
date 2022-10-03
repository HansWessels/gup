/*
 * Archive manager.
 *
 *     UNIX specific declarations.
 *
 * $Author: klarenw $
 * $Date: 1998-12-28 15:58:43 +0100 (Mon, 28 Dec 1998) $
 * $Revision: 137 $
 * $Log$
 * Revision 1.1  1998/12/28 14:58:43  klarenw
 * *** empty log message ***
 *
 */

typedef struct
{
	time_t time;
} ostime;

typedef struct
{
	mode_t mode;
} osmode;

typedef struct
{
	/*
	 * Note that device, inode, link_cnt and length are currently not used
	 * by the archive manager, either because these variables are not
	 * stored in a header in an archive or because the archive manager
	 * stores this information somewhere else (length).
	 */

	dev_t device;						/* Device. */
	ino_t inode;						/* I node. */
	int link_cnt;						/* Number of links. */

	unsigned long length;				/* File length. */

	osmode file_mode;					/* File attributes. */

	ostime mtime;						/* Modification time. */
	ostime atime;						/* UNIX last access time. */
	ostime ctime;						/* UNIX attribute change time. */

	uid_t uid;							/* UNIX user id. */
	gid_t gid;							/* UNIX group id. */
} osstat;

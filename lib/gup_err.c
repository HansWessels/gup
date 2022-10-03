/*
 * Support library.
 *
 *     gup_err - Convert C error codes to GUP error codes.
 *
 * $Author: wout $
 * $Date: 2000-07-30 17:53:10 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 181 $
 * $Log$
 * Revision 1.2  2000/07/30 15:53:10  wout
 * Print message when an unknown C error code is passed to
 * gup_conv_err.
 *
 * Revision 1.1  1998/12/27 20:51:16  klarenw
 * Added pattern matching functions and error code conversion functions.
 * Fixed bugs in file buffering functions and strlwr functions.
 */

#include <errno.h>
#include <stdio.h>

#include "gup_err.h"

/*
 * gup_result gup_conv_err(int error)
 *
 * Convert a C error code to a GUP error code. Note that if error
 * is 0, GUP_UNKNOWN is returned. Call this function only when
 * there is an error.
 *
 * Parameters:
 *
 * error	- C error code <> 0 (from errno).
 *
 * Result: GUP error code.
 */

gup_result gup_conv_err(int error)
{
	switch(error)
	{
	case EPERM:							/* Operation not permitted */
		return GUP_PERM;
#if EACCES != EPERM						/* Some compilers define these to be the same. */
	case EACCES:						/* Permission denied */
		return GUP_ACCESS;
#endif
	case ENOENT:						/* No such file or directory */
		return GUP_NOENT;
	case EIO:							/* I/O error */
		return GUP_IO;
#ifdef ENXIO
	case ENXIO:							/* No such device or address */
		return GUP_NXIO;
#endif
#ifdef ENOEXEC
	case ENOEXEC:						/* Exec format error */
		return GUP_NOEXEC;
#endif
	case EBADF:							/* Bad file number */
		return GUP_BADF;
	case ENOMEM:						/* Out of memory */
		return GUP_NOMEM;
#ifdef EFAULT
	case EFAULT:						/* Bad address */
		return GUP_FAULT;
#endif
#ifdef EBUSY
	case EBUSY:							/* Device or resource busy */
		return GUP_BUSY;
#endif
	case EEXIST:						/* File exists */
		return GUP_EXIST;
#ifdef EXDEV
	case EXDEV:							/* Cross-device link */
		return GUP_XDEV;
#endif
#ifdef ENODEV
	case ENODEV:						/* No such device */
		return GUP_NODEV;
#endif
#ifdef ENOTDIR
	case ENOTDIR:						/* Not a directory */
		return GUP_NOTDIR;
#endif
#ifdef EISDIR
	case EISDIR:						/* Is a directory */
		return GUP_ISDIR;
#endif
	case EINVAL:						/* Invalid argument */
		return GUP_INVAL;
#ifdef ENFILE
	case ENFILE:						/* File table overflow */
		return GUP_NFILE;
#endif
	case EMFILE:						/* Too many open files */
		return GUP_MFILE;
#ifdef ETXTBSY
	case ETXTBSY:						/* Text file busy */
		return GUP_TXTBSY;
#endif
#ifdef EFBIG
	case EFBIG:							/* File too large */
		return GUP_FBIG;
#endif
	case ENOSPC:						/* No space left on device */
		return GUP_NOSPC;
#ifdef ESPIPE
	case ESPIPE:						/* Illegal seek */
		return GUP_SPIPE;
#endif
	case EROFS:							/* Read-only file system */
		return GUP_ROFS;
#ifdef EMLINK
	case EMLINK:						/* Too many links */
		return GUP_MLINK;
#endif
#ifdef EPIPE
	case EPIPE:							/* Broken pipe */
		return GUP_PIPE;
#endif
#ifdef ENAMETOOLONG
	case ENAMETOOLONG:					/* File name too long */
		return GUP_NAMETOOLONG;
#endif
#if defined(ENOTEMPTY) && ENOTEMPTY != EEXIST	/* Some compilers define these to be the same. */
	case ENOTEMPTY:						/* Directory not empty */
		return GUP_NOTEMPTY;
#endif
#if defined(ELOOP) && (ELOOP != EMLINK)	/* Some compilers define these to be the same. */
	case ELOOP:							/* Too many symbolic links encountered */
		return GUP_LOOP;
#endif
#ifdef ENOSTR
	case ENOSTR:						/* Device not a stream */
		return GUP_NOSTR;
#endif
#ifdef ELIBEXEC
	case ELIBEXEC:						/* Cannot exec a shared library directly */
		return GUP_NOEXEC;
#endif
	default:							/* Unknown errors. */
#if 1
		printf("Unknown C error code %d. Please report.\r\n", error);
#endif
		return GUP_UNKNOWN;
	}
}

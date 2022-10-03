/*
 * Include files.
 *
 *     GUP result / error codes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:37:40 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 205 $
 * $Log$
 * Revision 1.9  2000/09/03 12:37:40  wout
 * Added new result and error codes.
 *
 * Revision 1.8  2000/07/30 15:47:20  wout
 * Updated comment.
 *
 * Revision 1.7  1998/12/27 20:48:08  klarenw
 * Removed settings.h. New names for error codes.
 */

#ifndef __GUP_ERRORS_H__
#define __GUP_ERRORS_H__

typedef enum
{
	/*
	 * General result codes.
	 */

	GUP_OK = 0,							/* No error. */
	GUP_INTERNAL,						/* Internal error. */
	GUP_UNKNOWN,						/* Unknown error, probably a C error code
										   not recognized by 'gup_conv_err'. */

	/*
	 * File I/O errors.
	 */

	GUP_READ_ERROR,						/* can't read file */
	GUP_WRITE_ERROR,					/* file write error */
	GUP_EOF,							/* Unexpected end of file. */
	GUP_CANNOT_CREATE_DIR,				/* Result code of builddir. !!! Should be removed. */
	GUP_TMPNAME,						/* Unable to create temporary file name. */

	/*
	 * C error codes.
	 */

	GUP_PERM,							/* Operation not permitted. */
	GUP_ACCESS,							/* Access denied. */
	GUP_NOENT,							/* File or directory not found. */
	GUP_IO,								/* General I/O error. */
	GUP_NXIO,							/* No such device or address. */
	GUP_NOEXEC,							/* Executable format error. */
	GUP_BADF,							/* Bad file handle. */
	GUP_NOMEM,							/* Out of memory. */
	GUP_FAULT,							/* Bad (memory block) address. */
	GUP_BUSY,							/* Device or resource busy. */
	GUP_EXIST,							/* File already exists. */
	GUP_XDEV,							/* Cross device link. */
	GUP_NODEV,							/* No such device. */
	GUP_NOTDIR,							/* Not a directory. */
	GUP_ISDIR,							/* Is a directory. */
	GUP_INVAL,							/* Invalid parameter. */
	GUP_NFILE,							/* File table overflow. */
	GUP_MFILE,							/* Too many open files. */
	GUP_TXTBSY,							/* Text file busy. */
	GUP_FBIG,							/* File too large. */
	GUP_NOSPC,							/* No space left on device. */
	GUP_SPIPE,							/* Illegal seek. */
	GUP_ROFS,							/* Read only file system. */
	GUP_MLINK,							/* Too many links. */
	GUP_PIPE,							/* Broken pipe. */
	GUP_NAMETOOLONG,					/* File name too long. */
	GUP_NOTEMPTY,						/* Directory is not empty. */
	GUP_LOOP,							/* Too many symbolic links. */
	GUP_NOSTR,							/* Device is not a stream. */
	
	/*
	 * Resultcodes of decode.c
	 */

	GUP_BAD_HUFF_TABLE1,				/* Error in first huffman table. */
	GUP_BAD_HUFF_TABLE2,				/* Error in second huffman table. */
	GUP_BAD_HUFF_TABLE3,				/* Error in third huffman table. */

	/*
	 * Warnings of init_encode
	 */

	GUP_NO_JM,							/* "Warning: Insufficient memory for -jm mode!" */
	GUP_NO_FASTMODE,					/* "Warning: Insufficient memory for fast mode!" */
	GUP_NO_MEDMODE,						/* "Warning: Insufficient memory for medium mode!" */
	GUP_NO_LINK,						/* "Warning: Insufficient memory for linking!" */
	GUP_NO_ZEEF34,						/* "Warning: Insufficient memory for sieve mode!" */
	GUP_SMALL_DICTIONARY,				/* "Warning: Using a smaller dictionary!" */

	/*
	 * Errors in archives.
	 */

	GUP_NO_ARCHIVE,						/* File is not a valid archive. */
	GUP_BROKEN,							/* Archive broken. */
	GUP_SEVERE,							/* Archive severly damaged. */
	GUP_CRC_FAULT,						/* CRC error in file. */

	/*
	 * Errors in headers.
	 */

	GUP_HDR_CRC_FAULT,					/* CRC error in a header. */
	GUP_HDR_UNKNOWN_METHOD,				/* Unknown method in header. */
	GUP_HDR_UNKNOWN_OS,					/* Unknown OS id in header. */
	GUP_HDR_POSITION,					/* Header not found at expected position. */
	GUP_HDR_SIZE_OVF,					/* Header size overflow. */
	GUP_HDR_ERROR,						/* Error in header. */

	/*
	 * Last of the errors
	 */

	GUP_LAST_ERROR,

	/*
	 * Result codes. These are not error codes.
	 */

	GUP_END_VOLUME = 0x2000,			/* Result code: End of volume reached. */
	GUP_END_ARCHIVE						/* Result code: End of archive reached. */
} gup_result;

#ifdef __cplusplus
extern "C"
{
#endif

gup_result gup_conv_err(int error);		/* Convert C error code to GUP error code. */

#ifdef __cplusplus
}
#endif

#endif

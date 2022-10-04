/*
 * Support library.
 *
 *     gup_io - Simple file buffering.
 *
 * $Author: wout $
 * $Date: 2000-07-30 17:55:03 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 182 $
 * $Log$
 * Revision 1.4  2000/07/30 15:55:03  wout
 * Check bounds in gup_io_set_current. Added function gup_io_set_position.
 *
 * Revision 1.3  1998/12/27 20:51:17  klarenw
 * Added pattern matching functions and error code conversion functions.
 * Fixed bugs in file buffering functions and strlwr functions.
 *
 * Revision 1.2  1998/03/26 20:03:15  klarenw
 * Added mmap support for reading.
 *
 * Revision 1.1  1998/03/17 18:34:00  klarenw
 * First version of filebuffering.
 */

/*
 * simple arj file buffering by Mr Ni!
 *
 * 25-12-1997
 * HWS: added routines for decompression engine
 *
 * 1997-11-08
 * HWS: added NEVER_USE(x) macro to surpress the warning:
 *      never used parameter x in functions
 */

/*
 * Usage of the start, current and end elements of the buf_handle_t
 * structure:
 *
 * start	- points always to the start of the buffer.
 * current	- points to the current read or write position in the buffer.
 * end		- reading: points one position after the last byte read from
 *			  the file.
 *			  writing: points to the end of the buffer (start + buf_size).
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "gup.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif

#include "compr_io.h"
#include "gup_err.h"
#include "gup_io.h"
#include "gup_io_porting.h"

#define FSF_WRITE		0x01			/* File is opened for writing flag. */
#define FSF_MMAPPED		0x02			/* File is memory mapped. */

typedef struct
{
	buf_fhandle_t xfile_buffer;
	int handle;					/* Filehandle. */
	int flags;
	long pos;					/* filepos of start of buffer */
	int eof;					/* End of file flag. Only used for reading. */
	size_t buf_size;			/* buffer size */
	uint8 *last_ptr;			/* pointer to last byte in buffer. */
#ifdef HAVE_MMAP
	void *mmap_addr;
	size_t mmap_len;
#endif
} file_struct;

/*****************************************************************************
 *																			 *
 * Internal functions.														 *
 * 																			 *
 *****************************************************************************/

/*
 * gup_result gup_io_flush(buf_fhandle_t *file)
 *
 * Write all data in the file buffer to disk.
 *
 * Parameters:
 *
 * file	- file descriptor.
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_flush(buf_fhandle_t *file)
{
	file_struct *com = (void *) file;
	long count, real_count;

	/*
	 * Determine the number of bytes in the buffer.
	 */

	if (com->last_ptr > file->current)
		count = com->last_ptr - file->start;
	else
		count = file->current - file->start;
	TRACE_ME_EX("flush count: %ld", count);

	if (count == 0)
		return GUP_OK;

	/*
	 * Write buffer to disk.
	 */

	do
	{
		TRACE_ME_EX("flushing buffer: %lu bytes", count);
		real_count = write(com->handle, file->start, (size_t) count);

		if (real_count == -1)
		{
#ifdef EINTR
			if (errno == EINTR)			/* Interrupted by signal, try again. */
				continue;
#endif
			return gup_conv_err(errno);	/* Error. Return error code. */
		}

		com->pos += real_count;
		count -= real_count;
	} while ((count > 0) && (real_count != 0));

	if (count > 0)
		return GUP_WRITE_ERROR;			/* Error, buffer was not flushed completely. */

	/*
	 * Update variables.
	 */

	file->current = com->last_ptr = file->start;

	return GUP_OK;
}

/*****************************************************************************
 *																			 *
 * High level functions.													 *
 * 																			 *
 *****************************************************************************/

/*
 * buf_fhandle_t *gup_io_open(const char *name, unsigned char *buf_start,
 *							  unsigned long buf_size, int omode,
 *							  gup_result *result)
 *
 * Open a file. Note that a file can only be opened as read only or
 * write only.
 *
 * Parameters:
 *
 * name			- name of the file to open.
 * buf_start	- buffer to use for buffering the data.
 * buf_size		- size of the buffer.
 * omode		- open mode.
 * result		- pointer to location where the error code is stored.
 *
 * Result: pointer to a buf_handle_t structure if succesfull, or NULL
 *         if an error occured.
 */

buf_fhandle_t *gup_io_open(const char *name, unsigned char *buf_start,
						   unsigned long buf_size, int omode,
						   gup_result *result)
{
	TRACE_ME_EX("name: %s, bufsize: %lu", name, buf_size);
	file_struct *com;
	long res;

	if ((com = malloc(sizeof(file_struct))) == NULL)
	{
		*result = GUP_NOMEM;
		return NULL;
	}

	com->xfile_buffer.start = buf_start;
	com->xfile_buffer.current = com->last_ptr = buf_start;
	com->buf_size = buf_size;
	com->pos = 0;
	com->eof = FALSE;

	if (omode)
	{
		/*
		 * Open file for writing.
		 */

		if ((res = open(name, OPEN_WR_FLAGS, FMODE)) == -1)
		{
			free(com);
			*result = gup_conv_err(errno);
			return NULL;
		}

		com->xfile_buffer.end = buf_start + buf_size;

		com->handle = (int) res;
		com->flags = FSF_WRITE;
	}
	else
	{
		/*
		 * Open file for reading.
		 */

		if ((res = open(name, OPEN_RD_FLAGS)) == -1)
		{
			free(com);
			*result = gup_conv_err(errno);
			return NULL;
		}

		com->xfile_buffer.end = buf_start;

		com->handle = (int) res;
		com->flags = 0;
#ifdef HAVE_MMAP
		{
			struct stat st;

			if (fstat(com->handle, &st) != -1)
			{
				com->mmap_len = st.st_size;

				if ((long) (com->mmap_addr = mmap(NULL, com->mmap_len, PROT_READ | PROT_WRITE, MAP_PRIVATE, com->handle, (off_t) 0)) != -1)
				{
					com->xfile_buffer.start = com->mmap_addr;
					com->xfile_buffer.current = com->last_ptr = com->mmap_addr;
					com->buf_size = com->mmap_len;
					com->flags |= FSF_MMAPPED;
					com->xfile_buffer.end = (uint8 *) com->mmap_addr + com->mmap_len;
					com->eof = TRUE;	/* The end of the buffer is equal to the end of the file. */
				}
			}
		}
#endif
	}

	*result = GUP_OK;

	return &com->xfile_buffer;
}

/*
 * gup_result gup_io_close(buf_fhandle_t *file)
 *
 * Close a file.
 *
 * file	- file_descriptor.
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_close(buf_fhandle_t *file)
{
	TRACE_ME();
	file_struct *com = (void *) file;
	gup_result result = GUP_OK;

	if (com->flags & FSF_WRITE)
	{
		/*
		 * File was opened for writing. If necessary flush the buffer.
		 */

		if (((file->current - file->start) > 0) || ((com->last_ptr - file->start) > 0))
		{
			if ((result = gup_io_flush(file)) != GUP_OK)
			{
				close(com->handle);
				free(com);
				return result;
			}
		}

		if (close(com->handle) == -1)
			result = gup_conv_err(errno);
	}
	else
	{
		/*
		 * File was opened for reading.
		 */

#ifdef HAVE_MMAP
		if (com->flags & FSF_MMAPPED)
			munmap(com->mmap_addr, com->mmap_len);
#endif
		if (close(com->handle) == -1)
			result = gup_conv_err(errno);
	}

	free(com);

	return result;
}

/*
 * gup_result gup_io_seek(buf_fhandle_t *file, long offset, int seekmode,
 *						  long *new_pos)
 *
 * Seek to a position in the file.
 *
 * Parameters:
 *
 * file		- file descriptor.
 * offset	- new position.
 * seekmode	- seek mode.
 * new_pos	- file position after seek.
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_seek(buf_fhandle_t *file, long offset, int seekmode,
					   long *new_pos)
{
	TRACE_ME_EX("seek offset: %ld, mode: %d", offset, seekmode);
	file_struct *com = (void *) file;

	if (com->flags & FSF_WRITE)
	{
		if (seekmode == SEEK_CUR)
		{
			offset += com->pos + (file->current - file->start);
			seekmode = SEEK_SET;
		}

		if (file->current > com->last_ptr)
			com->last_ptr = file->current;

		if ((seekmode != SEEK_END) && (offset >= com->pos) &&
			(offset <= com->pos + com->last_ptr - file->start))
		{
			file->current = file->start + offset - com->pos;

			*new_pos = offset;

			return GUP_OK;
		}
		else
		{
			gup_result result;

			if ((result = gup_io_flush(file)) != GUP_OK)
				return result;

			if ((com->pos = lseek(com->handle, offset, seekmode)) == -1)
				return gup_conv_err(errno);

			*new_pos = com->pos;

			return GUP_OK;
		}
	}
	else
	{
#ifdef HAVE_MMAP
		if (com->flags & FSF_MMAPPED)
		{
			/*
			 * File is memory mapped. Calculate the new position.
			 * If the new position is valid, set the current
			 * position to the new position.
			 */

			size_t pos;

			switch(seekmode)
			{
			case SEEK_SET:
				pos = offset;
				break;
			case SEEK_CUR:
				pos = (file->current - file->start) + offset;
				break;
			case SEEK_END:
				pos = com->mmap_len + offset;
				break;
			default:
				return GUP_INVAL;		/* Invalid parameter. */
			}

			if (pos > com->mmap_len)
				return GUP_INVAL;

			file->current = file->start + pos;

			*new_pos = pos;

			return GUP_OK;
		}
		else
#endif
		{
			if (seekmode == SEEK_CUR)
			{
				offset += com->pos + (file->current - file->start);
				seekmode = SEEK_SET;
			}

			if ((seekmode == SEEK_END) || (offset < com->pos) ||
				(offset > (com->pos + (file->end - file->start))))
			{
				/*
				 * New position is not within the file buffer. Seek
				 * to the desired position and fill the buffer.
				 */

				if ((com->pos = lseek(com->handle, offset, seekmode)) == -1)
					return gup_conv_err(errno);
				file->end = file->current = file->start;

				*new_pos = com->pos;

				return gup_io_fill(file);
			}
			else
			{
				/*
				 * The new file position is within the buffer. Adjust
				 * 'current' to the new position.
				 */

				file->current = file->start + offset - com->pos;

				*new_pos = offset;

				return GUP_OK;
			}
		}
	}
}

/*
 * gup_result gup_io_tell(buf_fhandle_t *file, long *fpos)
 *
 * Return the current position in the file.
 *
 * Parameters:
 *
 * file	- file descriptor.
 * fpos	- current position
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_tell(buf_fhandle_t *file, long *fpos)
{
	file_struct *com = (void *) file;

	*fpos = com->pos + (file->current - file->start);

	TRACE_ME_EX("offset: %ld", *fpos);
	return GUP_OK;
}

/*
 * gup_result gup_io_write(buf_fhandle_t *file, const void *buffer, unsigned long count,
 *						   unsigned long *real_count)
 *
 * Write data to a file.
 *
 * Parameters:
 *
 * file			- file descriptor.
 * buffer		- buffer containing the data to write.
 * count		- number of data bytes to write.
 * real_count	- number of bytes actually written.
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_write(buf_fhandle_t *file, const void *buffer, unsigned long count,
						unsigned long *real_count)
{
	TRACE_ME_EX("count: %lu bytes", count);
	unsigned long bytes_left, cnt;
	char *buf = (char *) buffer;
	file_struct *com = (void *) file;

	if (!(com->flags & FSF_WRITE))
		return GUP_INTERNAL;

	cnt = count;

	while (cnt > 0)
	{
		bytes_left = file->end - file->current;

		if (cnt < bytes_left)
		{
			memcpy(file->current, buf, cnt);
			file->current += cnt;
			cnt -= cnt;
		}
		else
		{
			gup_result result;

			if (bytes_left > 0)
			{
				memcpy(file->current, buf, bytes_left);
				file->current += bytes_left;
				buf += bytes_left;
				cnt -= bytes_left;
			}

			if ((result = gup_io_flush(file)) != GUP_OK)
				return result;
		}
	}

	*real_count = count - cnt;
	TRACE_ME_EX("real count written: %lu", *real_count);

	return GUP_OK;
}

/*
 * gup_result gup_io_read(buf_fhandle_t *file, void *buffer, unsigned long count,
 *						  unsigned long *real_count)
 *
 * Read data from a file.
 *
 * Parameters:
 *
 * file			- file descriptor.
 * buffer		- buffer where read data should be copied to.
 * count		- number of data bytes to read.
 * real_count	- number of bytes actually read.
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_read(buf_fhandle_t *file, void *buffer, unsigned long count,
					   unsigned long *real_count)
{
	TRACE_ME_EX("count: %lu bytes", count);
	unsigned long bytes_left, cnt;
	char *buf = (char *) buffer;
	file_struct *com = (void *) file;

	if (com->flags & FSF_WRITE)
		return GUP_INTERNAL;

	cnt = count;

	while (cnt > 0)
	{
		bytes_left = file->end - file->current;

		if (cnt < bytes_left)
		{
			memcpy(buf, file->current, cnt);
			file->current += cnt;
			cnt -= cnt;
		}
		else
		{
			gup_result result;

			if (bytes_left > 0)
			{
				memcpy(buf, file->current, bytes_left);
				file->current += bytes_left;
				buf += bytes_left;
				cnt -= bytes_left;
			}
			else if (com->eof)
				break;

			if ((result = gup_io_fill(file)) != GUP_OK)
				return result;
		}
	}

	*real_count = count - cnt;
	TRACE_ME_EX("real count read: %lu", *real_count);

	return GUP_OK;
}

/*****************************************************************************
 *																			 *
 * Low level functions.														 *
 * 																			 *
 * The low level functions make it possible to read data from or write data	 *
 * to the file buffer directly using the pointers start, current and end in	 *
 * the buf_handle_t structure. This feature is mainly intended for the		 *
 * compression engine, which can work in a more optimal way using this		 *
 * feature. Note that this feature should only be used to read or write		 *
 * data at the current file position.										 *
 *																			 *
 * Always use gup_io_seek to seek in the file when writing, especially after a	 *
 * previous seek backwards. A gup_io_write_announce might have flushed the 	 *
 * buffer in the mean time. Simply adding the offset seeked backward to		 *
 * the current pointer to seek to the original position results in garbage	 *
 * being written to the file the next time the buffer is flushed.			 *
 *																			 *
 *****************************************************************************/

/*
 * gup_result gup_io_write_announce(buf_fhandle_t *file, unsigned long count)
 *
 * Make sure there at least 'count' bytes free in the file buffer. If
 * necessary the file buffer is flushed.
 *
 * Parameters:
 *
 * file		- file descriptor.
 * count	- number of bytes.
 *
 * Error code, GUP_OK if no error.
 */

gup_result gup_io_write_announce(buf_fhandle_t *file, unsigned long count)
{
	TRACE_ME_EX("announce count: %lu bytes (buffer allocated size: %lu)", count, (unsigned long)(file->end - file->start));
	if (count >= (unsigned long) (file->end - file->current))
	{
		gup_result result;
		file_struct *com = (void *) file;
		long pos = com->pos + (file->current - file->start);

		/*
		 * There is not enough room in the buffer. Flush it.
		 */

		if (!(com->flags & FSF_WRITE))
			return GUP_INTERNAL;

		if ((result = gup_io_flush(file)) != GUP_OK)
			return result;

		/*
		 * In case 'current' was lower than 'last_ptr' (after a
		 * seek backwards), seek to the position in the file
		 * corresponding with the old value of 'current'. The
		 * seek is only done when really necessary to prevent
		 * errors when the file is a stream and seeks should
		 * not be performed.
		 */

		if (com->pos != pos)
		{
			if ((com->pos = lseek(com->handle, pos, SEEK_SET)) == -1)
				return gup_conv_err(errno);
		}
	}

	return GUP_OK;
}

/*
 * gup_result gup_io_fill(buf_fhandle_t *file)
 *
 * Fill the file buffer.
 *
 * Parameters:
 *
 * file	- file descriptor.
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_fill(buf_fhandle_t *file)
{
	TRACE_ME();
	file_struct *com = (void *) file;
	long count, real_count;

	if (com->flags & FSF_WRITE)
		return GUP_INTERNAL;

#ifdef HAVE_MMAP
	if (com->flags & FSF_MMAPPED)
		return GUP_OK;
#endif

	count = file->end - file->current;	/* Number of bytes left in buffer. */
	if (count > 0)
		memmove(file->start, file->current, (size_t) count);	/* Move to start of buffer. */
	com->pos += file->current - file->start;	/* New position in file of start of buffer. */
	file->current = file->start;		/* Update current position. */

	file->end = file->start + count;	/* Pointer to first free byte in buffer. */
	count = com->buf_size - count;		/* Number of free bytes in buffer. */

	do
	{
		real_count = read(com->handle, file->end, (size_t) count);

		if (real_count == -1)
		{
#ifdef EINTR
			if (errno == EINTR)			/* Interrupted by signal, try again. */
				continue;
#endif
			return gup_conv_err(errno);	/* Error. Return error code. */
		}

		file->end = file->end + real_count;	/* Pointer to end of buffer. */
		count -= real_count;
	} while ((count > 0) && (real_count != 0));

	if (real_count == 0)				/* Check if end of file has been reached. */
		com->eof = TRUE;
	else
		com->eof = FALSE;

	return GUP_OK;
}

/*
 * gup_result gup_io_reload(buf_fhandle_t *file)
 *
 * Fill the file buffer from the current file position till the end.
 *
 * This function is used together with a write-mode buffer in dump_archive & friends.
 * A special case where we must refetch the packed data, load it into memory for
 * further processing.
 *
 * Parameters:
 *
 * file	- file descriptor.
 *
 * Result: Error code, GUP_OK if no error.
 */

gup_result gup_io_reload(buf_fhandle_t *file, uint8_t *dstbuf, unsigned long dstbufsize, unsigned long *actual_bytes_read)
{
	TRACE_ME();
	file_struct *com = (void *) file;
	unsigned long count;
	long real_count;
	gup_result result;

	// we're **supposed** to be in write mode!
	if (!(com->flags & FSF_WRITE))
		return GUP_INTERNAL;

	if ((result = gup_io_flush(file)) != GUP_OK)
		return result;
		
	count = 0;
	do
	{
		real_count = read(com->handle, dstbuf, dstbufsize);

		if (real_count == -1)
		{
#ifdef EINTR
			if (errno == EINTR)			/* Interrupted by signal, try again. */
				continue;
#endif
			return gup_conv_err(errno);	/* Error. Return error code. */
		}

		dstbuf += real_count;	/* Pointer to end of buffer. */
		count += real_count;
		dstbufsize -= real_count;
	} while ((dstbufsize > 0) && (real_count != 0));

	if (actual_bytes_read)
		*actual_bytes_read = count;
	
	return GUP_OK;
}

/*
 * uint8 *gup_io_get_current(buf_fhandle_t *file, unsigned long *bytes_left)
 *
 * Get a pointer to the current position in the file buffer and return the
 * number of bytes that can be written into the buffer or can be read from
 * the buffer.
 *
 * Parameters:
 *
 * file			- pointer to file buffer.
 * bytes_left	- pointer to storage for the number of bytes left in the
 * 				  buffer.
 *
 * Result: pointer to current position in the buffer.
 */

uint8 *gup_io_get_current(buf_fhandle_t *file, unsigned long *bytes_left)
{
	TRACE_ME();
	*bytes_left = file->end - file->current;

	return file->current;
}

/*
 * void gup_io_set_current(buf_fhandle_t *file, uint8 *new_pos)
 *
 * Set the pointer in the file buffer to a new position.
 *
 * Parameters:
 *
 * file		- pointer to file buffer.
 * new_pos	- new position in file buffer.
 */

void gup_io_set_current(buf_fhandle_t *file, uint8 *new_pos)
{
	TRACE_ME();
	if ((new_pos >= file->start) && (new_pos < file->end))
		file->current = new_pos;
}

/*
 * void gup_io_set_position(buf_fhandle_t *file, long position)
 *
 * Set the current position in the file to the given value. Any
 * bytes after this position that are in the buffer are discarded.
 * If the position is not within the part of the file that is
 * currently in the buffer, the function does not change the
 * position.
 *
 * This function is intended to fix a feature of the compression
 * engine (it puts a few bytes to much in the buffer). It should
 * only be used for this purpose.
 *
 * Parameters:
 *
 * file		- pointer to file buffer.
 * position	- new position in the file.
 */

void gup_io_set_position(buf_fhandle_t *file, long position)
{
	TRACE_ME_EX("position: %ld", position);
	file_struct *com = (void *) file;

	if (com->flags & FSF_WRITE)
	{
		if ((position >= com->pos) &&
			(position <= com->pos + (file->current - file->start)))
		{
			file->current = file->start + (position - com->pos);
			com->last_ptr = file->current;
		}
	}
}

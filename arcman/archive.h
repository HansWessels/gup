/*
 * Archive manager.
 *
 *     Base archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:51:57 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 212 $
 * $Log$
 * Revision 1.6  2000/09/03 12:51:57  wout
 * Removed is_last_volume() and 'last_volume'. Added skip_compressed_data().
 *
 * Revision 1.5  1998/12/28 14:58:08  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.4  1998/03/26 19:59:24  klarenw
 * Updated to last version of the compression engine. Cleaner interface.
 *
 * Revision 1.3  1998/03/17 18:13:52  klarenw
 * Adapted to the new filebuffering scheme used by encode() and
 * decode().
 *
 * Revision 1.2  1998/01/03 19:24:41  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:47  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#ifndef __ARCMAN_ARCHIVE_H__
#define __ARCMAN_ARCHIVE_H__

#include "gup_err.h"
#include "compress.h"
#include "options.h"


typedef struct
{
	void (*print_progress)(unsigned long delta_size, void* pp_propagator);
	void* pp_propagator;
	void (*init_message)(gup_result message, void* im_propagator);
	void* im_propagator;
} GUPMSG;

class volume
{
  private:
	char *volume_name;					/* Name of the volume. */
	char *tmp_volume_name;				/* Temporary name of volume. */

  public:
	volume *next;						/* Pointer to next volume. */

	volume(const char *name, const char *tmp_name);	/* Constructor. */
	~volume(void);						/* Destructor. */

	inline const char *name(void) { return volume_name; }
	inline const char *tmp_name(void) { return (tmp_volume_name) ?
										tmp_volume_name : volume_name; }
};

class volume_list
{
  private:
	volume *first_ptr;					/* Pointer to first volume in list. */
	volume *cur_ptr;					/* Used when walking through the list. */
	volume *last_ptr;					/* Pointer to last volume in list. */

  public:
	volume_list(void);					/* Constructor. */
	~volume_list(void);					/* Destructor. */

	volume *add(const char *name, const char *tmp_name);	/* Add a volume to the list. */
	void clear(void);					/* Clear the entire list. */
	volume *first(void);				/* Get the first volume in the list. */
	volume *next(void);					/* Get the next volume in the list. */
};

class archive
{
 protected:
	struct
	{
		packstruct pack_str;
		decode_struct unpack_str;
	} st;

	buf_fhandle_t *file;				/* Filehandle of the opened volume. */
	char *archive_name;					/* Filename of archive (allocated with new). */
	volume_list volumes;				/* List of volumes in the archive. */
	volume *cur_volume;

	/*
	 * Internal flags.
	 */

	unsigned int opened : 1;			/* Archive opened flag. */
	unsigned int rw : 1;				/* Read/write flag (read = 0, write = 1). */
	unsigned int handle_valid : 1;		/* If 1, 'handle' is a valid file handle. */
	unsigned int use_temp_files : 1;	/* If set use temporary files to create the archive. */

	/*
	 * Copies of commandline options which are relevant to the member
	 * functions of these classes.
	 */

	unsigned int opt_repair : 1;		/* If set, ignore errors while reading headers. */
	unsigned int opt_no_write : 1;		/* If set, do not write depacked data to file. */

	/*
	 * Variables used during reading.
	 */

	unsigned char *file_buffer;

	/*
	 * The following functions create or destroy the file descriptor
	 * for the current volume.
	 */

	virtual gup_result open_volume(void);	/* Open current volume. */
	virtual gup_result close_volume(void);	/* Close current volume. */

 public:
	archive(void);
	virtual ~archive(void);

	/*
	 * Member functions for opening and closing the archive.
	 * 'create_archive' opens the archive for writing,
	 * 'open_archive' opens the archive for reading.
	 */

	virtual gup_result create_archive(const char *name, OPTIONS *options,
									  unsigned long first_volume_size, int use_temp,
									  GUPMSG *msgfunc);
	virtual gup_result create_next_volume(unsigned long volume_size) = 0;
	virtual gup_result open_archive(const char *name, OPTIONS *options,
									GUPMSG *msgfunc);
	virtual gup_result open_next_volume(void) = 0;
	virtual gup_result close_archive(int ok);
	virtual gup_result close_curr_volume(void) = 0;

	/*
	 * Functions for reading and writing the main header of an
	 * archive.
	 * The first form of 'write_main_header' writes an existing
	 * main header to the archive. The second form creates a new
	 * main header (using 'init_main_header') and writes it to the
	 * archive.
	 */

	virtual gup_result write_main_header(const mainheader *header) = 0;
	virtual gup_result write_main_header(const char *comment);
	virtual mainheader *read_main_header(gup_result &result) = 0;
	virtual mainheader *init_main_header(const char *comment) = 0;

	/*
	 * Functions for reading and writing file headers.
	 */

	virtual gup_result write_file_header(const fileheader *header) = 0;
	virtual gup_result write_file_trailer(const fileheader *header) = 0;
	virtual fileheader *read_file_header(gup_result &result) = 0;
	virtual gup_result read_file_trailer(fileheader *header) = 0;
	virtual fileheader *init_file_header(const char *filename,
										 const char *comment,
										 const osstat *stat) = 0;

	/*
	 * Compression and decompression functions.
	 */

	virtual gup_result decode(fileheader *header, int outfile);
	virtual gup_result skip_compressed_data(fileheader *header) = 0;
	virtual gup_result encode(fileheader *header, int infile);

	/*
	 * CRC functions.
	 */

	virtual uint32 init_crc(void) = 0;
	virtual uint32 calc_crc(uint8 *buf, long len, uint32 crc_in) = 0;
	virtual uint32 post_process_crc(uint32 crc);

	/*
	 * Multiple volume support.
	 */

	virtual unsigned long get_mv_bytes_left(void);
	virtual int mv_break(void);

	/*
	 * File I/O functions.
	 */

	virtual gup_result seek(long offset, int whence);
	virtual gup_result tell(long &offset);
	virtual gup_result read(void *buf, unsigned long len, unsigned long &real_len);
	virtual gup_result write(void *buf, unsigned long len, unsigned long &real_len);
	virtual gup_result getb(unsigned char &ch);

	/*
	 * GUP I/O virtualization functions.
	 */

	virtual buf_fhandle_t *gup_io_open(const char *name, unsigned char *buf_start, unsigned long buf_size, int omode, gup_result *result);
	virtual gup_result gup_io_close(buf_fhandle_t *file);
	// Seek to a position in the file.
	virtual gup_result gup_io_seek(buf_fhandle_t *file, long offset, int seekmode, long *new_pos);
	// Return the current position in the file.
	virtual gup_result gup_io_tell(buf_fhandle_t *file, long *fpos);
	// Write data to a file.
	virtual gup_result gup_io_write(buf_fhandle_t *file, const void *buffer, unsigned long count, unsigned long *real_count);
	// Read data from a file.
	virtual gup_result gup_io_read(buf_fhandle_t *file, void *buffer, unsigned long count, unsigned long *real_count);
	// Make sure there at least 'count' bytes free in the file buffer. If necessary the file buffer is flushed.
	virtual gup_result gup_io_write_announce(buf_fhandle_t *file, unsigned long count);
	// Fill the file buffer.
	virtual gup_result gup_io_fill(buf_fhandle_t *file);
	// Get a pointer to the current position in the file buffer and return the number of bytes that can be written into the buffer or can be read from the buffer.
	virtual uint8 *gup_io_get_current(buf_fhandle_t *file, unsigned long *bytes_left);
	// Set the pointer in the file buffer to a new position.
	virtual void gup_io_set_current(buf_fhandle_t *file, uint8 *new_pos);
	// Set the current position in the file to the given value. Any bytes after this position that are in the buffer are discarded.
	virtual void gup_io_set_position(buf_fhandle_t *file, long position);
	 
	/*
	 * Archive control function.
	 */

	virtual gup_result arcctl(int function, ... ) = 0;
};

typedef enum { AT_UNKNOWN, AT_ARJ, AT_LHA, AT_GZIP, AT_BINDUMP, AT_ASMDUMP, AT_CDUMP } archive_type;

archive *new_archive(archive_type type);
archive_type get_arc_type(const char *filename);



typedef struct
{
	int handle;							/* File handle of source file. */
	archive* arc;
	uint32 crc;
} READ_CRC_STRUCT;

typedef struct
{
	int handle;							/* File handle of destination file. */
	archive* arc;
	uint32 crc;
} WRITE_CRC_STRUCT;

gup_result buf_write_crc_test(long count, void* buffer, void* propagator);

#endif

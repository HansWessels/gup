/*
 * Archive manager.
 *
 *     *DUMP archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:00:35 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 214 $
 * $Log$
 * Revision 1.7  2000/09/03 13:00:35  wout
 * Added variable 'last_volume' to the arj_archive class.
 *
 * Revision 1.6  2000/07/30 14:56:53  wout
 * Changed the prototype of find_header.
 *
 * Revision 1.5  1998/12/28 14:58:08  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.4  1998/03/26 19:59:25  klarenw
 * Updated to last version of the compression engine. Cleaner interface.
 *
 * Revision 1.3  1998/03/17 18:13:53  klarenw
 * Adapted to the new filebuffering scheme used by encode() and
 * decode().
 *
 * Revision 1.2  1998/01/03 19:24:42  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:48  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#include "gup.h"

#if ENABLE_DUMP_OUTPUT_MODES

class dump_archive : public archive
{
  private:

	/*
	 * Variables used during writing.
	 */

	long header_pos;					/* Position in file of the last
										   written header. */
	dump_mainheader *cur_main_hdr;		/* Copy of the main header of the
										   current archive. */
	/*
	 * Variables used during reading.
	 */

	/*
	 * Private functions.
	 */

	gup_result write_end_of_volume(int mv);
	gup_result find_header(int first, uint16 &arj_header_id,
						   uint16 &arj_header_size);
  public:
	dump_archive(void);
	virtual ~dump_archive(void);

	/*
	 * Functions for opening and closing the archive.
	 */

	virtual gup_result create_archive(const char *name, OPTIONS *options,
									  unsigned long first_volume_size,
									  int use_temp, GUPMSG *msgfunc);
	virtual gup_result create_next_volume(unsigned long volume_size);
	virtual gup_result open_archive(const char *name, OPTIONS *options,
									GUPMSG *msgfunc);
	virtual gup_result open_next_volume(void);
	virtual gup_result close_archive(int ok);
	virtual gup_result close_curr_volume(void);

	/*
	 * Functions for reading and writing the main header.
	 */

	virtual gup_result write_main_header(const mainheader *header);
	virtual mainheader *read_main_header(gup_result &result);
	virtual mainheader *init_main_header(const char *comment);

	/*
	 * Functions for reading and writing file headers.
	 */

	virtual gup_result write_file_header(const fileheader *header);
	virtual gup_result write_file_trailer(const fileheader *header);
	virtual fileheader *read_file_header(gup_result &result);
	virtual gup_result read_file_trailer(fileheader *header);
	virtual fileheader *init_file_header(const char *filename,
										 const char *comment,
										 const osstat *stat);

	/*
	 * CRC functions.
	 */

	virtual uint32 init_crc(void);
	virtual uint32 calc_crc(uint8 *buf, long len, uint32 crc_in);
	virtual uint32 post_process_crc(uint32 crc);

	/*
	 * Multiple volume support.
	 */

	virtual unsigned long get_mv_bytes_left(void);
	virtual int mv_break(void);

	/*
	 * Compression and decompression functions.
	 */

	virtual gup_result skip_compressed_data(fileheader *header);
	virtual gup_result encode(fileheader *header, int infile);

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

	virtual gup_result arcctl(int function, ... );
};




class bindump_archive : public dump_archive
{
  private:
  public:
	bindump_archive(void);
	virtual ~bindump_archive(void);

	/*
	 * Functions for opening and closing the archive.
	 */
};


class cdump_archive : public dump_archive
{
  private:
  public:
	cdump_archive(void);
	virtual ~cdump_archive(void);

	/*
	 * Functions for opening and closing the archive.
	 */
};


class asmdump_archive : public dump_archive
{
  private:
  public:
	asmdump_archive(void);
	virtual ~asmdump_archive(void);

	/*
	 * Functions for opening and closing the archive.
	 */
};

#endif

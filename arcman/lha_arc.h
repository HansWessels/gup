/*
 * Archive manager.
 *
 *     LHA archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:09:40 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 218 $
 * $Log$
 * Revision 1.2  2000/09/03 13:09:40  wout
 * Removed variable 'cur_main_hdr' from class lha_archive. Added private
 * function write_ext_headers() to class lha_archive.
 *
 * Revision 1.1  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 */

class lha_archive : public archive
{
  private:
	/*
	 * Variables used during writing.
	 */

	long header_pos;					/* Position in file of the last
										   written header. */

	/*
	 * Other variables.
	 */

	int lha_hdr_level;					/* Header level used to create headers. */

	/*
	 * Private functions.
	 */

	gup_result write_end_of_volume(int mv);
	gup_result find_header(int first, int &hdr_type, int &hdr_len);
	gup_result write_ext_headers(const lha_fileheader *file_hdr,
								 const char *name_ptr, const char *path_ptr,
								 uint8 *&ptr);
  public:
	lha_archive(void);
	virtual ~lha_archive(void);

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
	 * Compression and decompression functions.
	 */

	virtual gup_result skip_compressed_data(fileheader *header);

	/*
	 * CRC functions.
	 */

	virtual uint32 init_crc(void);
	virtual uint32 calc_crc(uint8 *buf, long len, uint32 crc_in);

	/*
	 * Archive control function.
	 */

	virtual gup_result arcctl(int function, ... );
};

/*
 * Archive manager.
 *
 *     GZIP archive class.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:12:46 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 222 $
 * $Log$
 * Revision 1.1  2000/09/03 13:12:46  wout
 * GZIP support.
 *
 */

class gzip_archive : public archive
{
  private:
	/*
	 * Private functions.
	 */

	gup_result write_end_of_volume(int mv);
	gup_result find_header(int &hdr_len);
	gup_result write_ext_headers(const gzip_fileheader *file_hdr,
								 uint8 *&ptr);
  public:
	gzip_archive(void);
	virtual ~gzip_archive(void);

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

	virtual gup_result decode(fileheader *header, int outfile);
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

/*
 * Archive manager.
 *
 *     GZIP main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:12:46 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 222 $
 * $Log$
 * Revision 1.1  2000/09/03 13:12:46  wout
 * GZIP support.
 *
 */

/*
 * GZIP flags.
 */

#define GZ_ASCII		0x01			/* File is probably an ASCII file. */
#define GZ_CONT			0x02			/* Continuation of a multi part GZIp file. */
#define GZ_EXTRA_FIELD	0x04			/* Extra field is present. */
#define GZ_ORIG_FNAME	0x08			/* Original file name is present. */
#define GZ_COMMENT		0x10			/* File comment is present. */
#define GZ_ENCRYPTED	0x20			/* File is encrypted. */

/*
 * Main header class.
 */

class gzip_mainheader : public mainheader
{
  public:
	gzip_mainheader(const char *comment);	/* Constructor. */
	gzip_mainheader(const gzip_mainheader&);	/* Copy constructor. */
	~gzip_mainheader(void);				/* Destructor. */
};

/*
 * File header class.
 */

class gzip_fileheader : public fileheader
{
  private:
	void init(const char *comment);
	void update_file_attributes(void);
	void update_ext_hdrs(void);

	uint8 flags;						/* Flags. */

	uint32 orig_time_stamp;				/* Modification time as read from the
										   archive. Always in UNIX format. */
	uint8 extra_flags;
	uint16 part_number;					/* Multi part volume part number. */

 public:
	gzip_fileheader(const char *filename, const char *comment);
	gzip_fileheader(const char *filename, const char *comment, const osstat *stat);
	~gzip_fileheader(void);

	void set_file_stat(const osstat *stat);

	int get_header_len(void) const;
	int has_crc(void) const;

	friend class gzip_archive;
};

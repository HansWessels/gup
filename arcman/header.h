/*
 * Archive manager.
 *
 *     Base main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:04:12 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 216 $
 * $Log$
 * Revision 1.6  2000/09/03 13:04:12  wout
 * Removed list with compression modes.
 *
 * Revision 1.5  2000/08/27 10:27:16  hans
 * Updated list with compression modes.
 *
 * Revision 1.4  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.3  1998/03/26 19:59:25  klarenw
 * Updated to last version of the compression engine. Cleaner interface.
 *
 * Revision 1.2  1998/01/03 19:24:43  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:50  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

/*
 * Extended header class.
 */

class extended_header
{
  private:
	uint32 hdr_type;
	unsigned long hdr_len;
	uint8 *hdr_data;
  public:
	extended_header *next;

	extended_header(void);
	~extended_header(void);

	void set_header_data(uint32 type, const uint8 *data, unsigned long len);
	void get_header_data(uint32 &type, const uint8 *&data, unsigned long &len) const;
};

/*
 * Main header class.
 */

typedef enum { MHDR_ARJ, MHDR_LHA, MHDR_GZIP } mhdr_type;

class mainheader
{
  private:
	char *comment;						/* Archive comment. NULL if the
										   archive has no comment. */
  protected:
	mhdr_type hdr_type;					/* Header type. */

  public:
	mainheader(const char *comment);	/* Constructor. */
	mainheader(const mainheader&);		/* Copy constructor. */
	virtual ~mainheader(void) = 0;		/* Destructor. */

	/*
	 * Functions for reading and writing 'comment' and 'hdr_type'.
	 * (hdr_type can only be read.)
	 */

	void set_comment(const char *comment);
	inline const char *get_comment(void) const { return comment; }

	inline mhdr_type get_mhdr_type(void) const { return hdr_type; }
};

/*
 * File header class.
 */

typedef enum { FHDR_ARJ, FHDR_LHA, FHDR_GZIP } fhdr_type;

typedef enum { BINARY_TYPE = 0,			/* ARJ, LHARC */
			   TEXT_TYPE = 1,			/* ARJ */
			   COMMENT_TYPE = 2,		/* ARJ */
			   DIR_TYPE = 3,			/* ARJ */
			   LABEL_TYPE = 4,			/* ARJ */
			   SYMLINK_TYPE = 5,		/* LHARC */
			   HARDLINK_TYPE = 6
			 } ftype;

class fileheader
{
  private:
	void init(const char *filename, const char *comment);

	char *comment;						/* File comment. NULL if the
										   archive has no comment. */
	char *filename;						/* Name of the file. */
	char *linkname;						/* Name of the file this file is a link
										   to (only if this file is a symbolic
										   link). */
	extended_header *ext_hdr_list;		/* Chained list of extended headers. */

  protected:
	void add_ext_hdr(uint32 ext_hdr_type, uint8 *ptr, unsigned long ext_hdr_size);
	void del_ext_hdr(uint32 ext_hdr_type);
	const extended_header *first_ext_hdr(uint32 &type, const uint8 *&data,
										 unsigned long &len) const;
	const extended_header *next_ext_hdr(const extended_header *current, uint32 &type,
										const uint8 *&data, unsigned long &len) const;

	fhdr_type hdr_type;					/* Header type. */
	osstat stat;						/* File status. */

  public:
	fileheader(const char *filename, const char *comment);
	fileheader(const char *filename, const char *comment, const osstat *stat);
	virtual ~fileheader(void) = 0;

	/*
	 * Functions for reading and writing 'filename', 'linkname', 'comment',
	 * 'stat' and 'hdr_type'. (hdr_type can only be read.)
	 */

	virtual void set_filename(const char *name);
	inline const char *get_filename(void) const { return filename; }

	void set_linkname(const char *name);
	inline const char *get_linkname(void) const { return linkname; }

	void set_comment(const char *comment);
	inline const char *get_comment(void) const { return comment; }

	virtual void set_file_stat(const osstat *stat);
	inline const osstat *get_file_stat(void) const { return &stat; }

	inline fhdr_type get_fhdr_type(void) const { return hdr_type; }

	/*
	 * Other member functions.
	 */

	virtual int get_header_len(void) const = 0;
	virtual int has_crc(void) const = 0;
	virtual void mv_set_segment_cnt(int count);
	virtual int mv_is_continuation(void);

	/*
	 * Public variables.
	 */

	int host_os;						/* Operating system used to pack the
										   file. */
	ftype file_type;					/* File type. */
	int method;							/* Packmode. */
	unsigned long compsize;				/* Compressed size of this chunk. */
	unsigned long origsize;				/* Original size of this chunk. */
	unsigned long totalsize;			/* Total size of this file. */
	unsigned long offset;				/* Offset in original file of this
										   fragment (multiple volume archive). */
	uint32 file_crc;					/* CRC of the file. If the archive
										   type does not use all 32 bits,
										   these bits are 0. */
	uint32 real_crc;					/* CRC of the file as calculated
										   during decompression. When
										   compressing it is equal to
										   'file_crc' */

	friend class archive;
};

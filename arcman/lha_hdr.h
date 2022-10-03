/*
 * Archive manager.
 *
 *     LHA main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:10:43 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 219 $
 * $Log$
 * Revision 1.3  2000/09/03 13:10:43  wout
 * Changed type of orig_time_stamp to uint32.
 *
 * Revision 1.2  2000/07/30 15:08:45  wout
 * Added update_file_attributes.
 *
 * Revision 1.1  1998/12/28 14:58:10  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 */

/*
 * Main header class.
 */

class lha_mainheader : public mainheader
{
  public:
	lha_mainheader(const char *comment);	/* Constructor. */
	lha_mainheader(const lha_mainheader&);	/* Copy constructor. */
	~lha_mainheader(void);				/* Destructor. */
};

/*
 * File header class.
 */

class lha_fileheader : public fileheader
{
  private:
	void init(uint8 hdr_level);
	void update_file_attributes(void);
	void update_ext_hdrs(void);

	unsigned int flg_has_crc : 1;		/* 1 if the header contains a file CRC. */

	uint32 orig_time_stamp;				/* Modification time as read from the
										   archive. In level 0 and 1 headers
										   this is always in MSDOS format.
										   In level 2 headers it is in UNIX
										   format. */

	/*
	 * Variables specific for level 0 headers.
	 */

	uint8 msdos_file_mode;				/* MS-DOS file mode as stored in level 0
										   headers. */
	uint8 minor_version;
	uint16 unix_file_mode;				/* UNIX file mode. */
	uint16 unix_uid;					/* UNIX user ID. */
	uint16 unix_gid;					/* UNIX group ID. */
	uint32 unix_time_stamp;				/* UNIX time stamp. */

	uint8 lha_hdr_level;				/* Header level. */

 public:
	lha_fileheader(const char *filename, const char *comment);
	lha_fileheader(const char *filename, const char *comment, uint8 hdr_level);
	lha_fileheader(const char *filename, const char *comment, const osstat *stat);
	lha_fileheader(const char *filename, const char *comment, const osstat *stat, uint8 hdr_level);
	~lha_fileheader(void);

	void set_file_stat(const osstat *stat);

	int get_header_len(void) const;
	int has_crc(void) const;

	friend class lha_archive;
};

/*
 * Archive manager.
 *
 *     ARJ main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:03:31 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 215 $
 * $Log$
 * Revision 1.5  2000/09/03 13:01:59  wout
 * Better multiple volume support.
 *
 * Revision 1.4  2000/07/30 14:58:35  wout
 * Added update_file_attributes and update_ext_headers.
 *
 * Revision 1.3  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.2  1998/01/03 19:24:43  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:49  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

#ifndef __ARJ_HDR_H__
#define __ARJ_HDR_H__

/*
 * ARJ flags.
 */

#define GARBLE_FLAG 		0x01
#define VOLUME_FLAG 		0x04		/* Indicates this volume is not the last. */
#define EXTFILE_FLAG		0x08		/* File is split over two volumes. */
#define PATHSYM_FLAG		0x10		/* '/' is used as seperator instead of '\' */
#define BACKUP_FLAG 		0x20

/*
 * Main header class.
 */

class arj_mainheader : public mainheader
{
  protected:
	char *filename;						/* Filename of the archive. */
	uint16 fspecpos_in_fname;			/* Position of the fspec in the name. */

  public:
	arj_mainheader(const char *comment);	/* Constructor. */
	arj_mainheader(const arj_mainheader&);	/* Copy constructor. */
	~arj_mainheader(void);				/* Destructor. */

	/*
	 * Functions for reading and writing the filename from the
	 * main header.
	 */

	void set_filename(const char *filename);
	void set_filename(const char *filename, uint16 fspecpos);
	inline const char *get_filename(void) const { return filename; }
	inline const char *get_filename(uint16 *fspecpos) const
		{ *fspecpos = fspecpos_in_fname; return filename; }

	/*
	 * Variables.
	 */

	uint8 arj_nbr;						/* Archiver version used to pack. */
	uint8 arj_x_nbr;					/* Minimum version of archiver to depack. */
	uint8 host_os;						/* Operating system. */
	uint8 flags;						/* Flags. */

	uint8 arj_sec_nbr;					/* Security version. */
	unsigned long arj_secenv_len;
	unsigned long arj_secenv_fpos;

	ostime ctime;						/* Creation time of archive in OS format. */
	ostime mtime;						/* Modification time of archive in OS format. */

	unsigned long arc_size;				/* Length of the archive. */
};

/*
 * File header class.
 */

class arj_fileheader : public fileheader
{
  protected:
	void init(const char *name);
	void update_file_attributes(void);
	void update_ext_hdrs(void);

	uint16 fspecpos_in_fname;			/* Position of the fspec in the name. */

	uint16 orig_file_mode;				/* File attributes as read from the
										   archive. */
	uint32 orig_time_stamp;				/* Modification time as read from the
										   archive. */

  public:
	arj_fileheader(const char *filename, const char *comment);
	arj_fileheader(const char *filename, const char *comment, const osstat *stat);
	~arj_fileheader(void);

	/*
	 * Functions for reading and writing 'filename' from the
	 * file header.
	 */

	void set_filename(const char *name);
	void set_filename(const char *name, uint16 fspecpos);
	inline const char *get_filename(uint16 &fspecpos) const
		{ fspecpos = fspecpos_in_fname; return fileheader::get_filename(); }

	/*
	 * Other member functions.
	 */

	void set_file_stat(const osstat *stat);
	int get_header_len(void) const;
	int has_crc(void) const;
	void mv_set_segment_cnt(int count);
	int mv_is_continuation(void);

	/*
	 * Variables.
	 */

	uint8 arj_nbr;						/* Archiver version used to pack. */
	uint8 arj_x_nbr;					/* Minimum version of archiver to
										   depack. */
	uint8 flags;						/* Flags. */
	uint16 host_data;

	friend class arj_archive;
};

#endif

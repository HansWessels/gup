/*
 * Archive manager.
 *
 *     Declaration of OS dependent support functions.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:11:14 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 220 $
 * $Log$
 * Revision 1.4  2000/09/03 13:11:14  wout
 * Added GZIP support.
 *
 * Revision 1.3  2000/07/30 15:06:51  wout
 * Added gup_mkdir, updated prototype of lha_conv_from_os_name.
 *
 * Revision 1.2  1998/12/28 14:58:10  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.1  1997/12/24 22:54:50  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */

extern char *get_name(const char *path);
extern void copy_filename(char *dest, const char *src);
extern gup_result copy_file(const char *src, const char *dest);
extern gup_result move_file(const char *src, const char *dest);

extern ostime gup_time(void);
extern ostime time_unix_to_os(uint32 unix_time);
extern uint32 time_os_to_unix(ostime time);

extern osmode mode_unix_to_os(uint16 mode);
extern uint16 mode_os_to_unix(osmode mode);
extern osmode mode_dos_to_os(uint8 mode);
extern uint8 mode_os_to_dos(osmode mode);

extern ftype gup_file_type(const osstat *stat);

extern gup_result gup_stat(const char *name, osstat *st);
extern gup_result gup_lstat(const char *name, osstat *st);
extern gup_result gup_fstat(int handle, osstat *st);
extern gup_result gup_set_stat(const char *filename, const osstat *st);
extern int gup_is_dir(const osstat *stat);
extern int gup_is_symlink(const osstat *stat);

extern gup_result gup_mkdir(const char *dirname, osmode mode);
extern gup_result gup_mkdir(const char *dirname);
extern gup_result gup_symlink(const char *oldname, const char *newname);
extern gup_result gup_readlink(const char *filename, char **linkname);

/*
 * These functions and defines should be somewhere else.
 */

extern uint32 arj_conv_from_os_time(ostime time);
extern ostime arj_conv_to_os_time(uint32 time);
extern char *arj_conv_to_os_name(const char *arj_name, uint16 &fspec_pos, int pathsym_flag);
extern char *arj_conv_from_os_name(const char *os_name, uint16 &fspec_pos, int pathsym_flag);

extern uint32 lha_conv_from_os_time(ostime time, int hdr_type);
extern ostime lha_conv_to_os_time(uint32 time, int hdr_type);
extern char *lha_conv_to_os_name(const char *lha_name, unsigned long name_length,
								 const char *lha_path, unsigned long path_length,
								 int host_os);
extern void lha_conv_from_os_name(const char *os_name, int host_os, int hdr_level,
								  int is_dir, char *&lha_name, char *&lha_path);

extern uint32 gzip_conv_from_os_time(ostime time);
extern ostime gzip_conv_to_os_time(uint32 time);
extern char *gzip_conv_to_os_name(const char *gzip_name);
extern char *gzip_conv_from_os_name(const char *os_name);

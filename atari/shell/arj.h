#ifndef __ARJ_H
#define __ARJ_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PATH_CHAR		'/'

#if (OS == OS_ATARI)
#define COMPATIBLE_OS_NUMBER(os_num)	((os_num == OS_MSDOS) || \
										 (os_num == OS_ATARI) || \
										 (os_num == OS_UNIX))
#endif

/*
 * Type definitions.
 */

typedef enum
{
	ABSOLUTELY_NOTHING, LEAST, AVARAGE, LOTS_OF_IT
} verbosetype;

typedef struct
{
	long max_entries;					/* Current maximum number of entries. */
	long n_entries;						/* Current number of entries in directory. */
	struct arc_entry *entries;			/* Entries in directory. */
} ARC_DIRECTORY;

typedef struct arc_entry
{
	char *name;							/* Filename. */
	int selected;						/* File selected flag. Used by shell. */

	uint8 os_type;						/* Operating system. */
	uint8 arj_flags;					/* ARJ flags. */
	ftype file_type;					/* File type. */
	int method;							/* Compression method. */
	unsigned long compsize;				/* Compressed size. */
	unsigned long origsize;				/* Original size. */

	osstat stat;						/* File status. */

	/*
	 * The following field is only used if the entry is
	 * a directory.
	 */

	ARC_DIRECTORY dir;					/* Entries in directory. */
} ARC_ENTRY;

typedef struct
{
	long nfiles;						/* Total number of files in archive.*/
	long torigsize;						/* Total uncompressed size of files in archive. */
	long tcompsize;						/* Total compressed size of files in archive. */
} ARCHIVE_INFO;

extern void free_dir_tree(ARC_DIRECTORY *dir);
extern int get_arj_directory(const char *file, ARCHIVE_INFO *arc_info,
							 ARC_DIRECTORY *dir, OPTIONS *opts);
extern void decompress_files(const char *file, const char *path,
							 ARC_DIRECTORY *files);
extern gup_result create_empty_archive(const char *filename, OPTIONS *opts);

#ifdef __cplusplus
}
#endif

#endif	/* __ARJ_H */

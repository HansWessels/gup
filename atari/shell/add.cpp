#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "gup.h"
#include "gup_err.h"
#include "compress.h"
#include "options.h"
#include "header.h"
#include "archive.h"

#include "alert.h"
#include "arj.h"
#include "add.h"
#include "message.h"
#include "os_dep.h"
#include "util.h"

extern int opt_repair;
extern int opt_rep_severe;
extern int opt_verbose_level;

#if 0
template<class T> inline T min(T x, T y)	{ return (x < y) ? x : y; }
template<class T> inline T max(T x, T y)	{ return (x > y) ? x : y; }

static void print_progress(unsigned long)
{
}

static void init_message(gup_result warning)
{
	switch(warning)
	{
	case GUP_NO_JM:
		print("Warning: Insufficient memory for -jm mode!\n");
		break;
	case GUP_NO_FASTMODE:
		print("Warning: Insufficient memory for fast mode!\n");
		break;
	case GUP_NO_MEDMODE:
		print("Warning: Insufficient memory for medium mode!\n");
		break;
	case GUP_NO_LINK:
		print("Warning: Insufficient memory for linking!\n");
		break;
	case GUP_NO_ZEEF34:
		print("Warning: Insufficient memory for sieve mode!\n");
		break;
	case GUP_SMALL_DICTIONARY:
		print("Warning: Using a smaller dictionary!\n");
		break;
	default:
		break;
	}
}

/*
 * gup_result copy_part(int dest_file, FILE *src_file, unsigned long length,
 *						struct packstruct *pack_str)
 *
 * Copy a part from 'src_file' to 'dest_file'.
 *
 * Parameters:
 *
 * dest_file	- file handle of destination file
 * src_file		- file handle of source file
 * length			- length of block to copy
 * pack_str		- pointer to pack struct
 *
 * Result: GUP_OK if no error
 */

gup_result copy_part(int dest_file, FILE *src_file, long length,
					 struct packstruct *pack_str)
{
	register size_t l;
	register long result;
	register long unsigned int tmp_buf_len;
	register byte *tmp_buf;
	register gup_result r = GUP_OK;

	/*
	 * Get size and length of the part of the encode buffer that we
	 * can use for our own purposes.
	 */

	if (pack_str->mode == 0)
	{
		tmp_buf_len = 65536L;
		if((tmp_buf = (byte *) malloc(tmp_buf_len)) == NULL)
			return GUP_NOMEM;
	}
	else
		tmp_buf = get_buf(&tmp_buf_len, pack_str);

	/*
	 * Copy the part of the file.
	 */

	while ((length > 0) && (r == GUP_OK))
	{
		l = min((size_t) tmp_buf_len, (size_t) length);

		if ((result = fread(tmp_buf, 1, l, src_file)) != l)
			r = (result == -1) ? gup_conv_err(errno) : GUP_READ_ERROR;
		else
		{
			if ((result = write(dest_file, tmp_buf, l)) != (long) l)
				r = (result == -1) ? gup_conv_err(errno) : GUP_WRITE_ERROR;
			length -= l;
		}
	}

	if (pack_str->mode == 0)
		free(tmp_buf);

	return r;
}

/*
 * ARC_ENTRY *do_update(fileheader *header, const char *arc_path, ARC_DIRECTORY *files)
 *
 * Check if a file in an archive should be updated.
 *
 * Parameters:
 *
 * header	- header of file in archive which should be checked.
 * arc_path	- Path prefix in archive.
 * files	- tree of files on disk, which should be added to the archive
 * 			  (or updated).
 *
 * Result: NULL pointer if the file should not be updated, otherwise pointer
 * 		   to the ARC_ENTRY node in 'files' of the corresponding file on disk.
 */

static ARC_ENTRY *do_update(fileheader *header, const char *arc_path,
							ARC_DIRECTORY *files)
{
	char *slash_ptr, *name_ptr;
	ARC_DIRECTORY *cur_dir;
	int i, found;
	unsigned long l;

	if ((l = strlen(arc_path)) != 0)
	{
		if (strncmp(header->filename, arc_path, l))
			return NULL;
		if ((arc_path[l - 1] != PATH_CHAR) && (header->filename[l] != PATH_CHAR))
			return NULL;
	}

	name_ptr = &header->filename[l];
	if (*name_ptr == PATH_CHAR)
		name_ptr++;
	cur_dir = files;

	do
	{
		found = FALSE;

		if ((slash_ptr = strchr(name_ptr, PATH_CHAR)) != NULL)
			*slash_ptr = 0;

		for (i = 0; i < cur_dir->n_entries; i++)
		{
			ARC_ENTRY *entry = &cur_dir->entries[i];

			if (!strcmp(entry->name, name_ptr))
			{
					/*
					 * The file or a parent directory is selected,
					 * return TRUE.
					 */

				if (slash_ptr == NULL)
					return entry;

				if (entry->file_type == DIR_TYPE)
				{
					found = TRUE;
					cur_dir = &entry->dir;
				}
			}
		}

		if (slash_ptr != NULL)
		{
			*slash_ptr = PATH_CHAR;
			name_ptr = slash_ptr + 1;
		}
	}
	while (slash_ptr && found);

	return NULL;
}

/*
 * arj_result pack_file(int tmp_file, const char *name, const char *arj_name,
 *										 ARC_ENTRY *entry, const char *comment,
 *										 struct packstruct *pack_str, unsigned long *crc_table,
 *										 int updating)
 *
 * Add one file to an archive at the cuurent position in the archive.
 *
 * Parameters:
 *
 * tmp_file		- File handle of the archive to add the file to.
 * name			- Name of the file on disk.
 * arj_name		- Name of the file in the archive.
 * entry		- Information about the file to pack.
 * comment		- Comment string.
 * pack_str		- Parameter structure for encode().
 * crc_table	- Pointer to CRC table. Needed by some called functions.
 * updating		- Flag indicating if the file is being updated or is added.
 *
 * Result: ARJ_OK if no error.
 */

arj_result pack_file(int tmp_file, const char *name, const char *arj_name,
					 ARC_ENTRY *entry, const char *comment,
					 struct packstruct *pack_str, unsigned long *crc_table,
					 int updating)
{
	long header_pos = lseek(tmp_file, 0, SEEK_CUR);
	ARJ_HEADER header;
	arj_result result;

	init_header(&header);

	strncpy(header.filename, arj_name, FNAME_MAX - 1);
	header.filename[FNAME_MAX - 1] = 0;
	strncpy(header.comment, comment, COMMENT_MAX - 1);
	header.comment[COMMENT_MAX - 1] = 0;

	if ((result = write_header(tmp_file, &header, crc_table)) == ARJ_OK)
	{
		int src_file;

		update_filename(name);
		print("%s %s\n", (updating) ? "Updating" : "Adding", arj_name);

		if ((src_file = open(name, O_RDONLY)) < 0)
			result = ARJ_C_ERROR;
		else
		{
			pack_str->bytes_packed = 0;
			pack_str->crc = CRC_MASK;
			pack_str->rbuf_current = pack_str->buffer;
			pack_str->source_file = src_file;

			if (entry->origsize)
			{
				if ((result = encode(pack_str)) == ARJ_OK)
				{
					pack_str->buf_write(pack_str);
					pack_str->crc = ~pack_str->crc;

					if (lseek(tmp_file, header_pos, SEEK_SET) == -1)
						result = ARJ_C_ERROR;
					else
					{
						/*
						 * Update header structure.
						 */

						header.sdata.file.method = pack_str->mode;
						header.sdata.file.time_stamp = convert_to_arj_time(entry->time_stamp);
						header.sdata.file.compsize = pack_str->packed_size;
						header.sdata.file.origsize = entry->origsize;
						header.sdata.file.file_crc = pack_str->crc;
						header.sdata.file.file_mode = entry->file_mode;

						if ((result = write_header(tmp_file, &header, crc_table)) == ARJ_OK)
						{
							if (lseek(tmp_file, header.sdata.file.compsize, SEEK_CUR) == -1)
								result = ARJ_C_ERROR;
							else
							{
								/*
								 * Update entry in 'files'.
								 */

								entry->selected = TRUE;
								entry->os_type = header.host_os;
								entry->arj_flags = header.arj_flags;
								entry->file_type = header.file_type;
								entry->method = header.sdata.file.method;
								entry->compsize = header.sdata.file.compsize;
							}
						}
					}
				}
			}

			close(src_file);
		}
	}

	return result;
}

/*
 * arj_result pack_unpacked_files(int tmp_file, const char *path,
 *															 const char *arj_path,
 *															 ARC_DIRECTORY *files,
 *															 struct packstruct *pack_str,
 *															 unsigned long *crc_table)
 *
 * Pack all files in the tree 'files', which have not yet been packed
 * ('selected' field is FALSE). This function calls itself to pack
 * files in subdirectories.
 *
 * Parameters:
 *
 * tmp_file		- File handle of archive.
 * path			- Path on disk of files to pack.
 * arj_path		- Path in archive of files to pack.
 * files		- Tree with files to pack.
 * pack_str		- Parameter structure for encode().
 * crc_table	- Pointer to CRC table. Needed by some called functions.
 *
 * Result: ARJ_OK if no error.
 */

arj_result pack_unpacked_files(int tmp_file, const char *path,
							   const char *arj_path,
							   ARC_DIRECTORY *files,
							   struct packstruct *pack_str,
							   unsigned long *crc_table)
{
	long i;
	arj_result result = ARJ_OK;

	for (i = 0; ((i < files->n_entries) && (result == ARJ_OK)); i++)
	{
		char *name, *arj_name;
		ARC_ENTRY *entry = &files->entries[i];

		if (!entry->selected || (entry->file_type == DIR_TYPE))
		{
			if ((name = make_path(path, entry->name, 0)) == NULL)
				result = ARJ_OUT_OF_MEM;
			else
			{
				if ((arj_name = make_path(arj_path, entry->name, 0)) == NULL)
					result = ARJ_OUT_OF_MEM;
				else
				{
					if (entry->file_type == DIR_TYPE)
						result = pack_unpacked_files(tmp_file, name, arj_name, &entry->dir,
																			 pack_str, crc_table);
					else
						result = pack_file(tmp_file, name, arj_name, entry, "", pack_str,
														 crc_table, FALSE);

					free(arj_name);
				}

				free(name);
			}
		}
	}

	return result;
}

gup_result do_compress(archive *src, archive *dest, const char *arc_name,
					   const char *path, const char *arc_path,
					   ARC_DIRECTORY *files);
{
	gup_result result;
	mainheader *mheader;
	fileheader *fheader;
	int next_vol;

	next_vol = 0;

	print("Updating archive <%s>\n", arj_file);

	/*
	 * Read main header.
	 */

	mheader = src->read_main_header(result);

	/*
	 * Write the main header to the temporary filename.
	 */

	if (result == GUP_OK)
		result = dest->write_main_header(mheader);

	/*
	 * Scan the archive for files.
	 */

	while (result == GUP_OK)
	{
		if ((fheader = src->read_file_header(result)) != NULL)
		{
			switch(result)
			{
			case GUP_HDR_POSITION:
				warning("<%s> is broken here.%s\n",
						arc_name, (opt_repair ?
						"\nScanning for valid fileheader(s)..." : ""));
				break;
			default:
				break;
			}

			if (next_vol & VOLUME_FLAG)
			{
				warning("Discontinued split file! No other files should follow "
						"this one in the archive! "
						"Consider this file damaged!\n");

				if (!opt_repair)
					result = GUP_BROKEN;
			}

			if (result != GUP_BROKEN)
			{
				ARC_ENTRY *entry;

				next_vol |= (header->flags & VOLUME_FLAG);

				if ((entry = do_update(header, arc_path, files)) != NULL)
				{
					char *name_ptr, *src_name;

					name_ptr = &header.filename[strlen(arj_path)];
					if (*name_ptr == PATH_CHAR)
						name_ptr++;

					if ((src_name = make_path(path, name_ptr, 0)) == NULL)
						result = ARJ_OUT_OF_MEM;
					else
					{
						result = pack_file(tmp_file, src_name, header.filename, entry,
															 header.comment, pack_str, crc_table,
															 TRUE);
						free(src_name);
					}

					fseek(arj_handle, next_pos, SEEK_SET);
				}
				else
				{
					if ((result = write_header(tmp_file, &header, crc_table)) == ARJ_OK)
						result = copy_part(tmp_file, arj_handle, header.sdata.file.compsize, pack_str);
				}
			}
			
			delete fheader;
		}
		else
		{
			if (result == GUP_EOF)
				result = GUP_BROKEN;

			if ((opt_rep_severe) && (result == GUP_END_ARCHIVE))
			{
// !!!				if (opt_verbose_level >= AVERAGE)
					warning("Possible end of archive located.\n");
// !!!				if (opt_verbose_level > AVERAGE)
					warning("Option <-jr1> is switched on so we continue "
							"searching. This will however result in a \'broken\'"
							"message at the end of each archive no matter what!\n");
				result = GUP_OK;
			}
		}
	}

	if (result == GUP_END_ARCHIVE)
		result = GUP_OK;

	/*
	 * Add all files which have not yet been written to
	 * the archive.
	 */

	if (result == GUP_OK)
		result = pack_unpacked_files(tmp_file, path, arj_path, files, pack_str, crc_table);

	delete mheader;

	if ((result == ARJ_END_ARCHIVE) && next_vol)
		result = ARJ_NEXT_VOL;

	switch(result)
	{
	case ARJ_NEXT_VOL:
	case ARJ_END_ARCHIVE:
		result = ARJ_OK;
		break;
	default:
		break;
	}

	return result;
}

/*
 * int compress_files(const char *file, const char *path,
 *										const char *arj_path, ARC_DIRECTORY *files)
 *
 * Compress the files in 'files' and add them in archive
 * 'file'.
 *
 * Parameters:
 *
 * file		- filename of archive
 * path		- pathname of files to pack
 * arj_path	- path in archive to add files to
 * files	- files to pack
 *
 * Result: 0 - archive succesfully updated, the files in
 *			   'files', which have been succesfully compressed,
 *			   are marked by setting the selected flag
 *		   1 - error, archive not changed
 */

int compress_files(const char *file, const char *path,
				   const char *arc_path, ARC_DIRECTORY *files,
				   OPTIONS *options)
{
	clock_t start_time;
	float total_time;
	archive *src, *dest;
	gup_result result, cls_result;
	ARJMSG msgfunc;
	archive_type type;

	start_time = clock();

	msgfunc.print_progress = print_progress;
	msgfunc.pp_propagator = NULL;
	msgfunc.init_message = init_message;
	msgfunc.im_propagator = NULL;

	if ((type = get_arc_type(file)) == AT_UNKNOWN)
		type = AT_ARJ;

	src = new_archive(type);
	dest = new_archive(type);

	if ((result	= src->open_archive(file, opts, &msgfunc)) == GUP_OK)
	{
		if ((result = dest->create_archive(file, opts, opts->mv_size,
										   1, &msgfunc)) == GUP_OK)
		{
			result = do_compress(src, dest, file, path, arc_path, files);

			update_filename("");			/* Make filename in dialog box empty. */

			if ((cls_result = dest->close_archive((result == GUP_OK) ? 1 : 0)) != GUP_OK)
			{
				if (result == GUP_OK)
					result = cls_result;
			}
		}

		src->close_archive(1);
	}

	delete dest;
	delete src;

	/*
	 * Display time used and make filename in dialog box empty.
	 */

	total_time = (float) (clock() - start_time) / (float) CLK_TCK;
	print("Total time: %.2f seconds\n", total_time);

	return (result != GUP_OK) ? 1 : 0;
}
#else
int compress_files(const char *file, const char *path,
				   const char *arc_path, ARC_DIRECTORY *files,
				   OPTIONS *options)
{
}
#endif

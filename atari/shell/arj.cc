#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "gup.h"
#include "gup_err.h"
#include "options.h"
#include "compress.h"
#include "header.h"
#include "archive.h"
#include "arj.h"
#include "message.h"
#include "util.h"

/*
 * Options.
 */

int opt_rep_severe = FALSE;
int ask_next_vol = TRUE;
int opt_wait_for_arj = TRUE;
int opt_only_test = FALSE;
int opt_b_mark_ok = FALSE;
int opt_rq_passw = FALSE;
int opt_orig_password = FALSE;
int opt_b_m_only = FALSE;
int opt_any_attrib = TRUE;
int opt_x_any_attr = FALSE;
int opt_dirtype_too = TRUE;
int opt_do_labels = FALSE;
int opt_label_drive = FALSE;
int opt_verbose_level = AVARAGE;
int opt_ask_createdir = FALSE;
int opt_no_strictchk = TRUE;
int opt_ask_append = FALSE;
int opt_only_new = FALSE;
int opt_extractunique = FALSE;
int opt_freshen = FALSE;
int opt_ask_each_file = FALSE;
int opt_ask_overwrite = FALSE;
int opt_update = FALSE;
int opt_show_comments = FALSE;
int opt_show_banner = FALSE;
int opt_ign_r_tbit = FALSE;
int opt_showonlyfspec = FALSE;
int opt_no_crc_checking = FALSE;

/*
 * Global variables.
 */

int backups_found = FALSE;
mode_t dir_fmask = 0755;				/* File mask for creating directories. */
int adorable_width = 20;

static void print_progress(unsigned long delta, void *prop)
{
	(void) delta;
	(void) prop;
}

static void init_message(gup_result message, void *im_propagator)
{
	(void) message;
	(void) im_propagator;
}

/*
 * arj_result add_dir_entry(ARC_DIRECTORY *dir, ARJ_HEADER *header)
 *
 * Add a file to a directory tree.
 *
 * Parameters:
 *
 * dir		- direcory tree to add file to.
 * header	- header with the info about file.
 */

static gup_result add_dir_entry(ARC_DIRECTORY *dir, fileheader *header)
{
	ARC_ENTRY *entry;
	const char *slash_ptr, *name_ptr;
	ARC_DIRECTORY *cur_dir;

	name_ptr = header->get_filename();
	cur_dir = dir;

	do
	{
		int do_add;
		int i;
		long len;

		slash_ptr = strchr(name_ptr, PATH_CHAR);

		do_add = TRUE;

		len = (slash_ptr == NULL) ? strlen(name_ptr) : slash_ptr - name_ptr;

		for (i = 0; i < cur_dir->n_entries; i++)
		{
			if ((strncmp(cur_dir->entries[i].name, name_ptr, len) == 0) &&
				(cur_dir->entries[i].name[len] == 0))
			{
				entry = &cur_dir->entries[i];

				if (entry->file_type == DIR_TYPE)
				{
					cur_dir = &cur_dir->entries[i].dir;
					do_add = FALSE;
					break;
				}
				else if (((entry->file_type == BINARY_TYPE) ||
						  (entry->file_type == TEXT_TYPE) ||
						  (entry->file_type == SYM_LINK)) &&
						 (header->file_type == entry->file_type))
				{
					entry->origsize += header->origsize;
					do_add = FALSE;
					break;
				}
			}
		}

		if (do_add)
		{
			if (cur_dir->max_entries == 0)
			{
				if ((cur_dir->entries = (ARC_ENTRY *) malloc(sizeof(ARC_ENTRY) * 20)) == NULL)
					 return GUP_NOMEM;

				cur_dir->max_entries = 20;
			}

			if (cur_dir->n_entries >= cur_dir->max_entries)
			{
				ARC_ENTRY *new_list;

				if ((new_list = (ARC_ENTRY *) realloc(cur_dir->entries, sizeof(ARC_ENTRY) *
													  (cur_dir->max_entries + 20))) == NULL)
					 return GUP_NOMEM;

				cur_dir->entries = new_list;
				cur_dir->max_entries += 20;
			}

			entry = &cur_dir->entries[cur_dir->n_entries];

			if ((entry->name = strdup(name_ptr)) == NULL)
				return GUP_NOMEM;

			entry->selected = FALSE;
			entry->os_type = header->host_os;

			if (slash_ptr)
			{
				/*
				 * Add directory entry.
				 */

				entry->file_type = DIR_TYPE;
				entry->arj_flags = 0;
				entry->method = 0;
				entry->compsize = 0;
				entry->origsize = 0;
				entry->stat = *header->get_file_stat();
			}
			else
			{
				/*
				 * Add file entry.
				 */

				entry->file_type = header->file_type;
				entry->arj_flags = header->flags;
				entry->method = header->method;
				entry->compsize = header->compsize;
				entry->origsize = header->origsize;
				entry->stat = *header->get_file_stat();
			}

			entry->dir.max_entries = 0;
			entry->dir.n_entries = 0;
			entry->dir.entries = NULL;

			cur_dir->n_entries++;
			if (slash_ptr)
				cur_dir = &entry->dir;
		}

		if (slash_ptr)
			name_ptr = slash_ptr + 1;
	} while (slash_ptr);

	return GUP_OK;
}

/*
 * void free_dir_tree(ARC_DIRECTORY *dir)
 *
 * Free a directory tree.
 *
 * Parameters:
 *
 * dir	- pointer to directory tree.
 */

void free_dir_tree(ARC_DIRECTORY *dir)
{
	register int i;

	for (i = 0; i < dir->n_entries; i++)
	{
		free(dir->entries[i].name);
		if (dir->entries[i].file_type == DIR_TYPE)
			free_dir_tree(&dir->entries[i].dir);
	}
}

/*
 * int get_arj_directory(const char *filename, ARCHIVE_INFO *arc_info,
 *						 ARC_DIRECTORY *dir)
 *
 * Make a list of all files in the archive. The archive filename
 * has got to have a suffix '.arj' or '.axx', where xx is a two
 * digit number.
 *
 * Parameters:
 *
 * file		- file name of the archive.
 * arc_info	- information about archive.
 * dir		- list of files in archive.
 *
 * Result: 0 if no error, 1 otherwise.
 */

int get_arj_directory(const char *filename, ARCHIVE_INFO *arc_info,
					  ARC_DIRECTORY *dir, OPTIONS *opts)
{
	ARJMSG msgfunc;
	gup_result result;
	archive *archive;

	arc_info->nfiles = 0;
	arc_info->torigsize = 0;
	arc_info->tcompsize = 0;

	dir->max_entries = 0;
	dir->n_entries = 0;
	dir->entries = NULL;

	msgfunc.print_progress = print_progress;
	msgfunc.pp_propagator = NULL;
	msgfunc.init_message = init_message;
	msgfunc.im_propagator = NULL;

	archive = new_archive(get_arc_type(filename));

	if ((result = archive->open_archive(filename, opts, &msgfunc)) == GUP_OK)
	{
		mainheader *main_hdr;
		fileheader *file_hdr;
		int next_vol = 0;

		main_hdr = archive->read_main_header(result);
		if (result == GUP_EOF)
			result = GUP_BROKEN;

		/*
		 * Scan the archive for files.
		 */

		while (result == GUP_OK)
		{
			file_hdr = archive->read_file_header(result);
			if (result == GUP_EOF)
				result = GUP_BROKEN;

			if (result == GUP_OK)
			{
				if (next_vol & VOLUME_FLAG)
					result = GUP_BROKEN;	/* Error, end of archive expected. */
				else
				{
					next_vol |= (file_hdr->flags & VOLUME_FLAG);

					if ((result = archive->seek(file_hdr->compsize, SEEK_CUR)) == GUP_OK)
						if ((result = archive->read_file_trailer(file_hdr)) == GUP_OK)
					{
						result = add_dir_entry(dir, file_hdr);

						arc_info->torigsize += file_hdr->origsize;
						arc_info->tcompsize += file_hdr->compsize;
						++arc_info->nfiles;
					}
				}
			}

			delete file_hdr;

			if ((result == GUP_END_ARCHIVE) && (next_vol || !archive->is_last_volume()))
			{
				if (opts->mv_mode)
				{
					result = archive->close_curr_volume();
#if 0
					if ((is_removable(arc_name) && ask_nextvol) ||
						opt_wait_for_arj)
					{
						/* Ask user to change disk. */
					}
#endif
					if (result == GUP_OK)
						result = archive->open_next_volume();
				}
				else
				{
#if 0
					if (opt_multivol)
						/* Display message multiple volume not supported
							   for this extension. */
					else
						/* Display message this is a multiple volume
							   archive. Use multiple volume option. */
#endif
				}
			}
		}

		delete main_hdr;
	}

	archive->close_archive(1);
	delete archive;

	return 0;
}

gup_result create_empty_archive(const char *filename, OPTIONS *opts)
{
	archive *archive;
	gup_result result, cls_result;
	ARJMSG msgfunc;

	msgfunc.print_progress = print_progress;
	msgfunc.pp_propagator = NULL;
	msgfunc.init_message = init_message;
	msgfunc.im_propagator = NULL;

	archive = new_archive(get_arc_type(filename));

	result = archive->create_archive(filename, opts, opts->mv_size, 0, &msgfunc);

	if (result == GUP_OK)
	{
		result = archive->write_main_header((const char *) NULL);

		if ((cls_result = archive->close_archive((result == GUP_OK) ? 1 : 0)) != GUP_OK)
		{
			if (result == GUP_OK)
				result = cls_result;
		}
	}

	delete archive;

	return result;
}

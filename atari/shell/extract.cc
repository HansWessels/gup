
#ifdef __GNUC__
#define alloca __builtin_alloca
#else
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#else
#ifdef _AIX
#pragma alloca
#else
char *alloca();
#endif
#endif
#endif

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
#include "message.h"
#include "os_dep.h"
#include "util.h"

#if 0
/*
 * Options.
 */

extern int opt_repair;
extern int opt_rep_severe;
extern int opt_multivol;
extern int ask_next_vol;
extern int opt_wait_for_arj;
extern int opt_only_test;
extern int opt_b_mark_ok;
extern int opt_rq_passw;
extern int opt_orig_password;
extern int opt_b_m_only;
extern int opt_any_attrib;
extern int opt_x_any_attr;
extern int opt_dirtype_too;
extern int opt_do_labels;
extern int opt_do_labels;
extern int opt_label_drive;
extern int opt_verbose_level;
extern int opt_ask_createdir;
extern int opt_no_strictchk;
extern int opt_ask_append;
extern int opt_only_new;
extern int opt_extractunique;
extern int opt_freshen;
extern int opt_ask_each_file;
extern int opt_ask_overwrite;
extern int opt_update;
extern int opt_show_comments;
extern int opt_show_banner;
extern int opt_ign_r_tbit;
extern int opt_showonlyfspec;
extern int opt_no_crc_checking;

/*
 * Global variables.
 */

extern int backups_found;
extern mode_t dir_fmask;			/* File mask for creating directories. */
extern int adorable_width;

arj_result arj_write(int handle, void *buffer, unsigned long len)
{
	if (write(handle, buffer, len) != len)
		return ARJ_WRITE_ERROR;
	else
		return ARJ_OK;
}

static void init_progress(void)
{
}

static void finish_progress(void)
{
}

static void display_comment(const char *comment)
{
}

static void display_banner(const char *comment)
{
}

/*
 * arj_result builddir(const char *name)
 *
 * Create directory 'name' and if necessary all non existing
 * parent directories.
 *
 * Parameters:
 *
 * name		- name of directory to create, without a terminating
 *					path seperator.
 */

arj_result builddir(const char *name)
{
	register char *filename, *curpthpos;
	register char ch;
	register int has_to_ask = TRUE;

	filename = (char *) alloca(strlen(name) + 1);
	strcpy(filename, name);

	curpthpos = filename;

	do
	{
		struct stat data;

		/*
		 * Find the next path seperator in 'filename'. If found, make
		 * it 0, to get a part of the path name of the directory to
		 * create.
		 */

		if ((curpthpos = strchr(curpthpos + 1, PATH_CHAR)) != NULL)
		{
			ch = *curpthpos;
			*curpthpos = 0;
		}

		if (stat(filename, &data))
		{
			/*
			 * Directory does not exist. Create the directory.
			 */

			if (has_to_ask && opt_ask_createdir)
			{
				if (request(YES_OR_NO, YES, name, "Create directory <%s>",
								adorable_filename(name)) == NO)
					return ARJ_SKIPPED;

				has_to_ask = FALSE;			 				/* Ask only once per dir. */
			}

			if (mkdir(filename, dir_fmask))
			{
				warning("Write-protected media, disk full, or incorrectly targetpath -> cannot "
								"create directory <%s>\n", filename);
				return ARJ_CANNOT_MAKE_DIR;
			}
		}

		if (curpthpos)
			*curpthpos = ch;									/* Restore path seperator. */
	} while (curpthpos);							 		/* still a dir to go! */

	return ARJ_OK;
}

#if 0

/*
 * int make_volume_label(char *filename)
 *
 * Create a volume label.
 *
 * Parameters:
 */

int make_volume_label(char *filename)
{
	int res;

	/*
	 * it's quite shitty to create a label on the Atari ST: method used here
	 * as mentioned in ProfiBuch ST/e/TT
	 */

	if (((Sversion() >> 8) & 0xFF) < 0x15)
	{
		int h = (int) Fcreate(filename, 0);

		Fclose(h);
		Fdelete(filename);
	}

	res = (int)Fcreate(filename, FA_VOLUME);		/* create label */
	Fclose(res);

	return (res >= 0) ? ARJ_OK : ARJ_WARNING;
}

#endif

/*
 * arj_result check_flags(ARJ_HEADER *header)
 *
 * Check the flags of a file in the archive to check if the
 * file can/should be extracted.
 *
 * Parameters:
 *
 * header		- pointer to header of file.
 *
 * Result: ARJ_OK       - if the file should be extracted
 *         ARJ_WARNING  - skip the file and print a warning
 *         ARJ_SKIPPED  - skip the file and print a warning if
 *                        the warning level is greater than AVARAGE
 */

arj_result check_flags(ARJ_HEADER *header)
{
	if ((header->file_type == DIR_TYPE) || (header->file_type == LABEL_TYPE))
		return ARJ_OK;

	if (header->arj_x_nbr > ARJ_X_VERSION)
	{
		/*
		 * Check if the file was compressed with russian ARJZ. If
		 * true change method to mode 8 (64k dictionary).
		 */

	  if((header->arj_x_nbr == 50) || (header->arj_x_nbr == 51))
	    header->sdata.file.method = EXTRA_METHOD2;
	  else
	  {
		  print("Unknown version: %d, ", header->arj_x_nbr);
		  return ARJ_WARNING;
		}
	}

	if ((header->arj_flags & EXTFILE_FLAG) != 0)
		print("Split archive: start depacking at fileposition <%ld>\n", header->sdata.file.extfile_pos);

	if ((header->arj_flags & GARBLE_FLAG) &&
			!(opt_rq_passw || opt_orig_password))
	{
		print("File is password encrypted, ");
		return ARJ_SKIPPED;
	}

	/*
	 * Check if the packing method is known.
	 */

	if ( /* header->sdata.file.method < 0 || */
		((header->sdata.file.method > MAXMETHOD) &&
		((header->sdata.file.method != EXTRA_METHOD1) &&
		(header->sdata.file.method!=EXTRA_METHOD2))) ||
		((header->sdata.file.method == 4) && (header->arj_nbr == 1)))
	{
		print("Unknown method: %d, ", header->sdata.file.method);
		return ARJ_WARNING;
	}

	/*
	 * Check if file type is known.
	 */

	if ((header->file_type != BINARY_TYPE) && (header->file_type != TEXT_TYPE))
	{
		print("Unknown file type: %d, ", header->file_type);
		return ARJ_WARNING;
	}

	/*
	 * Check BACKUP_FLAG
	 */

	if (header->arj_flags & BACKUP_FLAG)
	{
		if (!opt_b_mark_ok && !opt_b_m_only)
		{
			backups_found = TRUE;
			return ARJ_SKIPPED;							/* no option -jg specified */
		}
	}
	else if (opt_b_m_only)
		return ARJ_SKIPPED;								/* option -jg1 specified */

	/* check the attributes */

	if (!opt_any_attrib && !opt_x_any_attr && is_hidden(header))
			return ARJ_SKIPPED;

	return ARJ_OK;
}

/*
 * arj_result extract(FILE *arcfile, char *filename, ARJ_HEADER *header)
 *
 * Extract a file from an archive.
 *
 * Parameters:
 *
 * arcfile	- file handle of archive
 * filename	- file name of file to write. If the 'extractunique'
 *            switch is used, this string is changed. The array
 *            this pointer points to should be at least four
 *            bytes longer than the file name.
 * header		- header of the file to extract
 *
 * Result: ARJ_OK        - the file has been extracted
 *         ARJ_WARNING   - skip the file and print a warning
 *         ARJ_SKIPPED   - skip the file and print a warning if
 *                         the warning level is greater than AVARAGE
 *         ARJ_SKIPPED2  - skip the file, do not print a warning
 *         ARJ_CRC_FAULT - the file is extracted, but a CRC error
 *                         occured
 *         ARJ_BAD_HUFF_TABLE - error in huffman table
 */

arj_result extract(FILE *arcfile, char *filename, ARJ_HEADER *header,
									 unsigned long *crc_table,
									 struct decode_struct *dec_str)
{
	int exist_status = FALSE;
	arj_result result;

	if ((result = check_flags(header)) != ARJ_OK)
		return result;

	if (!opt_only_test)
	{
		struct stat file_info;

#if 0
		if (check_disk_free_space(filename))
			return ARJ_SKIPPED;
#endif

		if (header->file_type == DIR_TYPE)
		{
			/*
			 * File is a directory. If 'opt_dirtype' is TRUE,
			 * create the directory.
			 */

			if (!opt_dirtype_too)
				return ARJ_SKIPPED;

			if ((result = builddir(filename)) != ARJ_OK)
			{
				if (result == ARJ_SKIPPED)
					return result;
				else
					return ARJ_WARNING;
			}

			return result;
		}
		else if (header->file_type == LABEL_TYPE)
		{
			/*
			 * If file is a label and 'opt_do_labels' is TRUE create a
			 * disk label.
			 */

#if 0
			if (opt_do_labels && opt_label_drive)
			{
				/* change target for label */

				char *p = get_fname_pos(filename);

				*filename = command.opts.label_drive + 'A';
				filename[1] = ':';
				filename[2] = '\\';
				strcpy(filename + 3, p);
			}

			if (opt_do_labels)
				result = make_volume_label(filename);
			return result;
#else
			return ARJ_SKIPPED;
#endif
		}

		/*
		 * File not a label or a directory. Create directories if
		 * necessary.
		 */

		{
			char *dirname, *ptr;

			dirname = (char *) alloca(strlen(filename) + 1);
			strcpy(dirname, filename);

			if ((ptr = strrchr(dirname, PATH_CHAR)))
			{
			  *ptr = 0;
			  
				if ((result = builddir(dirname)) != ARJ_OK)
				{
					if (result == ARJ_SKIPPED)
						return result;
					else
						return ARJ_WARNING;
				}
			}
		}

		exist_status = !stat(filename, &file_info);

		/*
		 * always take fresh block for EXTFILE ARJ-block, since depacking must
		 * restart too. Remind TEXT-mode where EXTFILE block can PARTLY
		 * overlap previous depacked block legally! (That's why we use a
		 * seperate block!)
		 */

		if ((header->arj_flags & EXTFILE_FLAG))
		{
			if (exist_status)
			{
				/*
				 * Check if a file with the time stamp allready exists.
				 */

				if (file_info.st_mtime != convert_arj_time(header->sdata.file.time_stamp))
				{
					if (opt_verbose_level >= AVARAGE)
						warning("File <%s> is not of same time & date.%s\n",
										adorable_filename(filename),
										(opt_repair ? "" :
										" -> Skipping multiple volume part <%s>."),
										get_fname_pos(filename));

					if (!opt_repair)
						return ARJ_SKIPPED2;		/* skip without -jr switch */
				}

				/*
				 * Binary file. Check if the existing file has the
				 * correct size.
				 */

				if ((file_info.st_size == header->sdata.file.extfile_pos) ||
						((file_info.st_size > header->sdata.file.extfile_pos) &&
						(opt_no_strictchk || opt_repair)))
				{
					/*
					 * This is in fact what arj itself does too!
					 * all allright!
					 */
				}
				else
				{
					/*
					 * Display an error message if too small. Display
					 * a warning if too big.
					 */

					if (opt_verbose_level >= AVARAGE)
						warning("File <%s> on disk is too %s\n",
										adorable_filename(filename),
										((file_info.st_size > header->sdata.file.extfile_pos) ?
										"large" : "small"));

					if (opt_verbose_level > AVARAGE)
						warning("Use option <%s> if you want to depack this one.\n",
										((file_info.st_size > header->sdata.file.extfile_pos) ?
										"-~ns" : "-jt> or <-jt1"));

					if (opt_repair)
						header->sdata.file.extfile_pos = file_info.st_size;
					else
						return ARJ_SKIPPED2;
				}
			}
			else
			{
				warning("Maybe split file <%s> skipped because first part "
								"doesn't exist ... or access to file denied...\n",
								adorable_filename(filename));

				if (!opt_repair)
					return ARJ_SKIPPED2;
			}

			/*
			 * We should depack this file. Ask first
			 */

			if (opt_ask_append)
			{
				if (request(YES_OR_NO, YES, filename, "%s <%s>%c",
						"Append", adorable_filename(filename), ' ') == NO)
					return ARJ_SKIPPED;
			}
		}
		else if (exist_status)
		{
			int do_it = FALSE;

			/*
			 * File not continued from other archive and file already
			 * exists.
			 */

			if (opt_only_new)
				return ARJ_SKIPPED;

			print("File <%s> exists.\n", adorable_filename(filename));

			if (opt_extractunique)
			{
				/*
				 * Create unique file names. Replace the extension of the
				 * filename with a three digit number.
				 */

				char *s = (char *) get_fname_pos(filename);
				char *p = strrchr(s, '.');
				int i = 0;

				if (!p)
				{
					strcat(s, ".");
					p = s + strlen(s);
				}
				else
					p++;

				/*
				 * Check if there is a file with the new file name. If true
				 * increase the number and try again.
				 */

				do
					sprintf(p, "%03d", i++);
				while ((i <= 999) &&
								(exist_status = !stat(filename, &file_info)));

				if (exist_status)
				{
					p[-1] = 0;

					warning("You specified option <-jo> but I can't create any "
									"unique filename for base <%s> since they seem all "
									"to exist.\n", adorable_filename(filename));
					p[-1] = '.';

					return ARJ_SKIPPED;
				}

				do_it = TRUE;
			}
			else if (opt_ask_overwrite)
				do_it = TRUE;	/* do it anyway! */
			else
				do_it = (convert_arj_time(header->sdata.file.time_stamp) > file_info.st_mtime);

			if (!opt_extractunique && opt_ask_overwrite && do_it)
			{
				/*
				 * Ask the user if he wants to overwrite the existing file.
				 */
				 
				if (request(YES_OR_NO_FILE, NO, filename, "%s <%s>%c",
						((opt_update || opt_freshen) ? "Update" : "Overwrite"),
						adorable_filename(filename), ' ') == YES)
					return ARJ_SKIPPED;
			}
			else
			{
				if (do_it && opt_ask_each_file)
				{
					if (request(YES_OR_NO, YES, filename, "%s <%s>%c",
							"Extract", adorable_filename(filename), ' ') == NO)
						return ARJ_SKIPPED;
				}
				if (!do_it)
					return ARJ_SKIPPED;
			}
		}
		else
		{
			/*
			 * File does not exist.
			 */

			if (opt_freshen)
			{
				if (opt_verbose_level > AVARAGE)
					warning("File <%s> not at target. %sto extract this one.\n",
									adorable_filename(filename),
									"Disable \'FRESHEN\' ");
				return ARJ_SKIPPED;
			}

			if (opt_ask_each_file)
			{
				if (request(YES_OR_NO, YES, filename, "%s <%s>%c",
										"Extract", adorable_filename(filename), ' ') == NO)
					return ARJ_SKIPPED;
			}
		}
	}

	if (opt_show_comments && (strlen(header->comment) != 0))
	{
		/*
		 * Display comment.
		 */
		 
		display_comment(header->comment);
	}

	if ((header->file_type == BINARY_TYPE) || (header->file_type == TEXT_TYPE))
	{
		int ext_or_ovr = 0;
		int outfile;
		int mode;

		dec_str->crc = CRC_MASK;						/* Initiate CRC. */

		/* directories have been created successfully */

		if (!opt_only_test)
		{
			mode = 0666 & ~umask(0);

			if ((header->arj_flags & EXTFILE_FLAG))
			{
				if (!exist_status)
					outfile = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode);
				else
				{
					/* reset ReadOnly bit! */
					make_writeable(filename);
					outfile = open(filename, O_WRONLY, mode);
				}
			}
			else
			{
				if (exist_status)
				{
					if (is_readonly(filename))
					{
						if (opt_ign_r_tbit)
							make_writeable(filename);		/* Reset the read only bit. */
						else
						{
							warning("Can't write file <%s> since existing file is "
											"WRITE-PROTECTED.\n",
											adorable_filename(filename));
							return ARJ_SKIPPED;
						}
					}
					ext_or_ovr = 1;
				}
				outfile = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode);
			}

			if (outfile < 0)
			{
				warning("Can't open <%s>.\n", adorable_filename(filename));
				return ARJ_WARNING;
			}

			if (header->arj_flags & EXTFILE_FLAG)
			{
				if (lseek(outfile, header->sdata.file.extfile_pos, SEEK_SET) != header->sdata.file.extfile_pos)
					lseek(outfile, 0L, SEEK_END);
			}

			if (!COMPATIBLE_OS_NUMBER(header->host_os) && (header->file_type == BINARY_TYPE))
				warning("Warning! Binary file from a different OS.\n");
		}

		print("%s <%s>\v%2d",
					(opt_only_test ? "Testing" :
					(ext_or_ovr ? ((opt_update || opt_freshen) ?
					"Updating" : "Overwriting") : "Extracting")),
					(opt_showonlyfspec ? get_fname_pos(filename) :
					adorable_filename(filename)),
					(opt_showonlyfspec ?
					(int) (15 + 20) :/* max. filenamelength assumed 20 */
					(int) (adorable_width + 15)));

#if 0
		if (header->arj_flags & GARBLE_FLAG)
		{
			/* prepare for garble routine */
			command.password = opt_orig_password;
			/* converted password for old ARJ versions */
			command.password_length = command.opts.orig_password_length;
			command.bytes_to_garble = header->sdata.file.compsize;
		}
#endif

		init_progress();
		{
			long next_pos = ftell(arcfile) + header->sdata.file.compsize;

			if (header->sdata.file.origsize)
			{
#if 0
				if (command.bytes_to_garble != 0)
					garble();
#endif

#ifdef DEBUG
				print("\ncurrent pos = %ld, next_pos = %ld, (newfpos) = %ld,\n"
					  "method = %d, crc = $%08lx, filecrc = $%08lx\n",
					  (long) buf_tell(), (long) next_pos, (long) command.ul_newfpos,
					  (int) command.method, (long) command.crc ^ CRC_MASK, (long) command.file_crc);
#endif

				/*
				 * Extract the file from the archive.
				 */


				if ((result = decode(arcfile, outfile, header->sdata.file.origsize,
															header->sdata.file.method,
															dec_str)) != ARJ_OK)
				{
					close(outfile);
					return result;
				}
			}

			fseek(arcfile, next_pos, SEEK_SET);
		}
		finish_progress();

		if (!opt_only_test)
		{
			if ((header->file_type == BINARY_TYPE) || (header->file_type == TEXT_TYPE))
				close(outfile);

			set_file_info(filename, header);
		}

#ifdef DEBUG
		print("\ncurrent pos = %ld, (newfpos) = %ld,\n"
			  "method = %d, crc = $%08lx, filecrc = $%08lx\n",
			  (long) buf_tell(), (long) command.ul_newfpos,
			  (int) command.method, (long) command.crc ^ CRC_MASK, (long) command.file_crc);
#endif

		if (opt_no_crc_checking)
			print("Done\n");
		else if ((dec_str->crc ^ CRC_MASK) == header->sdata.file.file_crc)
			print("CRC OK\n");
		else
		{
			print("CRC error!\n");
			return ARJ_CRC_FAULT;
		}
	}

	return ARJ_OK;
}

/*
 * int do_extract(ARC_DIRECTORY *dir, ARJ_HEADER *header)
 *
 * Check if a file in an archive should be extracted.
 *
 * Parameters:
 *
 * dir		- directory tree of the files in the archive.
 * header	- header with the info about the file in the archive.
 *
 * Result: FALSE if the file should not be extracted, TRUE
 *         otherwise.
 */

static int do_extract(ARC_DIRECTORY *dir, ARJ_HEADER *header)
{
	char *slash_ptr, *name_ptr;
	ARC_DIRECTORY *cur_dir;
	int i, found;

	name_ptr = header->filename;
	cur_dir = dir;

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
				if (entry->selected)
				{
					/*
					 * The file or a parent directory is selected,
					 * return TRUE.
					 */

					if (slash_ptr != NULL)
					{
						if (entry->file_type == DIR_TYPE)
						{
							*slash_ptr = PATH_CHAR;
							return TRUE;
						}
					}
					else
						return TRUE;
				}

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

	return FALSE;
}

/*
 * arj_result do_decompress(const char *arj_file, const char *path
 *							ARC_DIRECTORY *files)
 */

arj_result do_decompress(const char *arj_file, const char *path,
						 ARC_DIRECTORY *files, unsigned long *crc_table,
						 struct decode_struct *dec_str)
{
	FILE *file;
	arj_result result;
	ARJ_HEADER header;
	int next_vol;
	int broken = FALSE;
	unsigned long crc_error_count = 0;

	next_vol = 0;

	print("Processing archive <%s>\n", arj_file);

	if ((file = fopen(arj_file, "rb")) == NULL)
		return ARJ_C_ERROR;

	/*
	 * Find main header.
	 */

	if ((result = find_header(file, TRUE, FALSE, crc_table)) != ARJ_OK)
	{
		if (result == ARJ_EOF)
			result = ARJ_NO_ARCHIVE;
	}
	else
	{
		if (read_header(file, TRUE, &header) != ARJ_OK)
		{
			warning("This archive is severely damaged."
							" There's very little chance retrieving something "
							"from this one.\n");/* Only end of archive mark found. */
			result = ARJ_SEVERE;
		}
		else
		{
			if (header.file_type != COMMENT_TYPE)
			{
				broken = TRUE;
				warning("<%s> is damaged: no valid main ARJ-header.%s\n",
								arj_file, (opt_repair ?
								"\nScanning for valid fileheader(s)..." : ""));

				if (!opt_repair)
					result = ARJ_BROKEN;
			}
			else
			{
				if (header.arj_flags & VOLUME_FLAG)
					next_vol = 1;
			}
		}
	}

	if ((result == ARJ_OK) && !broken && opt_show_banner &&
			(strlen(header.comment) != 0))
		display_banner(header.comment);

	/*
	 * Scan the archive for files.
	 */

	while (result == ARJ_OK)
	{
		long fpos = ftell(file);

		if (!broken)
		{
			if ((result = find_header(file, FALSE, FALSE, crc_table)) != ARJ_OK)
			{
				if (result == ARJ_EOF)
					result = ARJ_BROKEN;
			}
			else
			{
				long new_fpos = ftell(file);

				if (new_fpos != fpos)
				{
					/* previous targetfile bad! jumped back! */
					warning("<%s> is broken here.%s\n",
									arj_file, (opt_repair ?
									"\nScanning for valid fileheader(s)..." : ""));

					if (!opt_repair)
						result = ARJ_BROKEN;
				}

				if (result == ARJ_OK)
				{
					if ((result = read_header(file, FALSE, &header)) != ARJ_OK)
					{
						if (opt_rep_severe)
						{
							if (opt_verbose_level >= AVARAGE)
								warning("Possible end of archive located.\n");
							if (opt_verbose_level > AVARAGE)
								warning("Option <-jr1> is switched on so we continue "
												"searching. This will however result in a \'broken\'"
												"message at the end of each archive no matter what!\n");
							result = ARJ_OK;
						}
					}
				}
			}
		}

		broken = FALSE;

		if (result == ARJ_OK)
		{
			if (next_vol & VOLUME_FLAG)
			{
				warning("Discontinued split file! No other files should follow "
								"this one in the archive! "
								"Consider this file damaged!\n");

				if (!opt_repair)
					result = ARJ_BROKEN;
			}

			if (result == ARJ_OK)
			{
				long next_pos = ftell(file) + header.sdata.file.compsize;

				next_vol |= (header.arj_flags & VOLUME_FLAG);
				if (do_extract(files, &header))
				{
					char *outfile;

					if ((outfile = make_path(path, header.filename, 4)))
					{
						update_filename(outfile);

						switch(result = extract(file, outfile, &header, crc_table, dec_str))
						{
						case ARJ_SKIPPED:
							if (opt_verbose_level > AVARAGE)
								warning("Skipped <%s>\n", adorable_filename(outfile));
						case ARJ_SKIPPED2:
							fseek(file, next_pos, SEEK_SET);
							result = ARJ_OK;			/* Continue with next file. */
							break;
						case ARJ_WARNING:
							warning("Skipped <%s>\n", adorable_filename(outfile));
							fseek(file, next_pos, SEEK_SET);
							result = ARJ_OK;			/* Continue with next file. */
							break;
						case ARJ_CRC_FAULT:
							crc_error_count++;
							result = ARJ_OK;			/* Continue with next file. */
							break;
						default:
							break;
						}

						free(outfile);
					}
					else
						result = ARJ_C_ERROR;
				}
				else
					fseek(file, next_pos, SEEK_SET);
			}
		}
	}

	fclose(file);

	if ((result == ARJ_END_ARCHIVE) && next_vol)
		result = ARJ_NEXT_VOL;

	/* Display error messages if necessary. */

	switch(result)
	{
	case ARJ_OK:
	case ARJ_NEXT_VOL:
	case ARJ_END_ARCHIVE:
		break;
	case ARJ_DISK_FULL:
		aprintf(1, "[1][Disk full.][ OK ]");
		break;
	case ARJ_C_ERROR:
		xform_error(errno);
		break;
	default:
		aprintf(1, "[1][Error.][ OK ]");
		break;
	}

	return result;
}

/*
 * void decompress_files(const char *file, const char *path,
 *						ARC_DIRECTORY *files)
 *
 * Extract all files in 'files' of which the 'selected'
 * flag is set.
 *
 * Parameters:
 *
 * file		- filename of archive
 * path		- pathname of files to extract
 * files	- all files in archive 'file'
 */

void decompress_files(const char *file, const char *path,
					  ARC_DIRECTORY *files)
{
	register char *arc_name, *suffix_ptr;
	register int arj_suff;
	register unsigned long *crc_table;
	register clock_t start_time;
	float total_time;
	struct decode_struct dec_str;

	errno = ENOERR;

	if ((crc_table = make_arj_crc_table()) == NULL)
	{
		/* Display error message out of memory. */
		return;
	}

	if ((arc_name = strdup(file)) == NULL)	/* Copy filename. */
	{
		free(crc_table);
		/* Display error message out of memory. */
		return;
	}

	if (init_decode(&dec_str))
	{
		free(arc_name);
		free(crc_table);
		/* Display error message out of memory. */
		return;
	}

	dec_str.crc_table = crc_table;
	dec_str.buf_crc = arj_crc;
	dec_str.write = arj_write;

	arj_suff = get_arj_suffix_cnt(arc_name, &suffix_ptr);

	start_time = clock();

	do
	{
		arj_result result;

		result = do_decompress(arc_name, path, files, crc_table, &dec_str);

		if ((result == ARJ_NEXT_VOL) && opt_multivol && arj_suff)
		{
			sprintf(suffix_ptr, "%02d", arj_suff++);
#if 0
			if ((is_removable(arc_name) && ask_nextvol) ||
				opt_wait_for_arj)
			{
				/* Ask user to change disk. */
			}
#endif
		}
		else if (result == ARJ_NEXT_VOL)
		{
#if 0
			if (opt_multivol)
				/* Display message multiple volume not supported
				   for this extension. */
				else
				/* Display message this is a multiple volume
				   archive. Use multiple volume option. */
#endif
				break;													/* Leave this loop! */
		}
		else
			break;														/* All split archives have been processed. */
	} while ((arj_suff > 0) && (arj_suff < 100) && exist(arc_name));

	/*
	 * Display time used and make filename in dialog box empty.
	 */

	total_time = (float) (clock() - start_time) / (float) CLK_TCK;
	print("Total time: %.2f seconds\n", total_time);

	update_filename("");									/* Make filename in dialog box empty. */

	/*
	 * Free used memory.
	 */

	free_decode(&dec_str);								/* Free buffer. */
	free(arc_name);												/* Free copy of filename. */
	free(crc_table);											/* Free CRC table. */
}
#else
void decompress_files(const char *file, const char *path,
					  ARC_DIRECTORY *files)
{
}
#endif

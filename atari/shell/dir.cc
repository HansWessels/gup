#include <aesbind.h>
#include <dirent.h>
#include <errno.h>
#include <new.h>
#include <osbind.h>
#include <stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "gup.h"
#include "gup_err.h"
#include "compress.h"
#include "options.h"
#include "header.h"
#include "archive.h"
#include "os.h"

#include "cgl_dial.h"

#include "alert.h"
#include "arj.h"
#include "slider.h"
#include "arj_slid.h"
#include "cgl_dial.h"
#include "dir.h"
#include "popup.h"
#include "main.h"
#include "resource.h"
#include "slider.h"
#include "util.h"

void *operator new(size_t size, void *ptr, size_t new_len)
{
	return realloc(ptr, new_len * size);
}

template<class T> inline T min(T x, T y)	{ return (x < y) ? x : y; }
template<class T> inline T max(T x, T y)	{ return (x > y) ? x : y; }

#define CONST_PART	35					/* Length of constant part of directory item. */
#define MAX_LENGTH	(SL_MAX_ITM_LEN - CONST_PART)	/* Maximum length of filename. */

static int sort_by_type(const void *e1, const void *e2)
{
	mode_t mode1, mode2;

	mode1 = mode_os_to_unix(((DIR_ENTRY *) e1)->stat.file_mode);
	mode2 = mode_os_to_unix(((DIR_ENTRY *) e2)->stat.file_mode);

	if (((mode1 & S_IFMT) == S_IFDIR) &&
		((mode2 & S_IFMT) != S_IFDIR))
		return -1;
	else if (((mode2 & S_IFMT) == S_IFDIR) &&
			 ((mode1 & S_IFMT) != S_IFDIR))
		return 1;
	else
		return 0;
}

static int sort_by_name(const void *e1, const void *e2)
{
	int h;

	if ((h = sort_by_type(e1, e2)) != 0)
		return h;
	else
		return strcmp(((DIR_ENTRY *) e1)->name, ((DIR_ENTRY *) e2)->name);
}

/*
 * Implementation of directory_listbox class.
 */

directory_listbox::directory_listbox(void)
{
	path = NULL;
	entries = NULL;
	n_entries = 0;
	max_entries = 0;
}

directory_listbox::~directory_listbox(void)
{
	free_memory();
}

void directory_listbox::free_memory(void)
{
	if (path != NULL)
		delete path;

	if (entries != NULL)
	{
		int i;

		for (i = 0; i < n_entries; i++)
			delete entries[i].name;

		delete entries;
	}
}

/*
 * FILL_UNTIL fills a string until a counter reaches a certain value.
 */

inline void FILL_UNTIL(char *&dest, int &i, int value)
{
	while (i < (value))
	{
		*dest++ = ' ';
		i++;
	}
}

void directory_listbox::get_line(int line, char *string)
{
	DIR_ENTRY *entry = &entries[line];
	int i;
	char *d = string, *p, tmp[16];
	struct tm *time;
	time_t t;

	/*
	 * Put the file type in the string.
	 */

	*d++ = ' ';

	switch(mode_os_to_unix(entry->stat.file_mode) & S_IFMT)
	{
	case S_IFCHR :
		*d++ = 'c';
		break;
	case S_IFDIR :
		*d++ = '\007';
		break;
	case S_IFBLK :
		*d++ = 'b';
		break;
	case S_IFREG :
		*d++ = ' ';
		break;
	case S_IFIFO :
		*d++ = 'p';
		break;
	case S_IMEM :
		*d++ = 'm';
		break;
	case S_IFLNK :
		*d++ = 'l';
		break;
	default :
		*d++ = '?';
		break;
	}

	*d++ = ' ';

	/*
	 * Copy the filename in the string.
	 */

	i = 0;
	p = entry->name;

	while ((*p) && (i < MAX_LENGTH))
	{
		*d++ = *p++;
		i++;
	}

	FILL_UNTIL(d, i, min(max_namelength, MAX_LENGTH));

	/*
	 * Put the file length in the string.
	 */

	if ((mode_os_to_unix(entry->stat.file_mode) & S_IFMT) == S_IFREG)
	{
		int l;

		sprintf(tmp, "%ld", entry->stat.length);
		l = strlen(tmp);
		p = tmp;
		i = 0;
		FILL_UNTIL(d, i, 10 - l);
		while (*p)
			*d++ = *p++;
	}
	else
	{
		i = 0;
		FILL_UNTIL(d, i, 10);
	}

	*d++ = ' ';
	*d++ = ' ';

	/*
	 * Put the time in the string.
	 */

	t = time_os_to_unix(entry->stat.mtime);
 	time = localtime(&t);

	*d++ = time->tm_hour / 10 + '0';
	*d++ = time->tm_hour % 10 + '0';
	*d++ = ':';
	*d++ = time->tm_min / 10 + '0';
	*d++ = time->tm_min % 10 + '0';
	*d++ = ':';
	*d++ = time->tm_sec / 10 + '0';
	*d++ = time->tm_sec % 10 + '0';

	*d++ = ' ';
	*d++ = ' ';

	/*
	 * Put the date in the string.
	 */

	*d++ = time->tm_mday / 10 + '0';
	*d++ = time->tm_mday % 10 + '0';
	*d++ = '-';
	*d++ = (time->tm_mon + 1) / 10 + '0';
	*d++ = (time->tm_mon + 1) % 10 + '0';
	*d++ = '-';
	*d++ = (time->tm_year % 100) / 10 + '0';
	*d++ = time->tm_year % 10 + '0';

	*d++ = ' ';
	*d++ = ' ';

	*d++ = 0;
}

int directory_listbox::is_selected(int line)
{
	return entries[line].selected;
}

void directory_listbox::handle_open(int line)
{
	if ((mode_os_to_unix(entries[line].stat.file_mode) & S_IFMT) == S_IFDIR)
	{
		char *new_path;

		if ((new_path = make_path(path, entries[line].name, 0)))
		{
			set_path(new_path, TRUE);
			free(new_path);
		}
		else
			xform_error(ENOMEM);
	}
	else if (get_arc_type(entries[line].name) != AT_UNKNOWN)
	{
		char *name;

		if ((name = make_path(path, entries[line].name, 0)))
		{
			((main_window *) dialog)->set_archive(name, TRUE);
			free(name);
		}
		else
			xform_error(ENOMEM);
	}
}

void directory_listbox::handle_select(int line)
{
	if (entries[line].selected)
	{
		entries[line].selected = FALSE;
		n_selected--;
		if (n_selected == 0)
			((main_window *) dialog)->update_compress_button();
	}
	else
	{
		entries[line].selected = TRUE;
		n_selected++;
		if (n_selected == 1)
			((main_window *) dialog)->update_compress_button();
	}

	invert_line(line);
}

void directory_listbox::set_path(const char *new_path, int redraw)
{
	DIR *dir;
	struct dirent *entry;

	free_memory();

	path = new char[strlen(new_path) + 1];
	strcpy(path, new_path);

	max_entries = 50;
	entries = new DIR_ENTRY[max_entries];
	n_entries = 0;												/* Number of entries in directory. */
	n_selected = 0;												/* Number of selected entries. */
	max_namelength = 12;									/* Default filename length. */

	errno = ENOERR;

	if ((dir = opendir(path)) != NULL)
	{

		/*
		 * Read the contents of the directory.
		 */

		while(((entry = readdir(dir)) != NULL) && (errno == ENOERR))
		{
			if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
			{
				int l;

				if (n_entries == max_entries)
				{
					max_entries += 50;
					entries = new (entries, max_entries) DIR_ENTRY;
				}

				l = strlen(entry->d_name);
				max_namelength = max(max_namelength, l);

				entries[n_entries].name = new char[l + 1];
				strcpy(entries[n_entries].name, entry->d_name);
				entries[n_entries].selected = FALSE;
				n_entries++;
			}
		}

		closedir(dir);

		/*
		 * Read the file information of the files in the directory.
		 */

		if (errno == ENOERR)
		{
			DIR_ENTRY *entry;
			struct stat data;
			char cur_path[256];
			int i;

			if (getcwd(cur_path, 256) != NULL)
			{
				chdir(new_path);
				((main_window *) dialog)->set_drive(Dgetdrv());

				for (i = 0; (i < n_entries) && (errno == ENOERR); i++)
				{
					entry = &entries[i];

					if (!stat(entry->name, &data))
					{
						entry->stat.device = data.st_dev;
						entry->stat.inode = data.st_ino;
						entry->stat.link_cnt = data.st_nlink;
						entry->stat.length = data.st_size;
						entry->stat.file_mode = mode_unix_to_os(data.st_mode);
						entry->stat.mtime = time_unix_to_os(data.st_mtime);
						entry->stat.atime = time_unix_to_os(data.st_atime);
						entry->stat.ctime = time_unix_to_os(data.st_ctime);
						entry->stat.uid = data.st_uid;
						entry->stat.gid = data.st_gid;
					}
				}

				chdir(cur_path);

				qsort(entries, n_entries, sizeof(DIR_ENTRY), sort_by_name);
			}
		}
	}

	strcpy(((TEDINFO *) dialog->get_tree()[MDCURDIR].ob_spec)->te_ptext, path);

	init_vslider(n_entries, 0, redraw);
	init_hslider(max_namelength + CONST_PART, 0, redraw);

	if (redraw)
	{
		dialog->draw_object(MDCURDIR, 1);
		dialog->draw_object(DIRTEXT, 1);
	}

	((main_window *) dialog)->update_compress_button();

	if (errno != ENOERR)
		xform_error(errno);
}

void directory_listbox::refresh(void)
{
	const char *path;

	if ((path = strdup(get_path())))
	{
		set_path(path, TRUE);
		free((char *) path);
	}
	else
		xform_error(ENOMEM);
}

static int add_directory(ARC_DIRECTORY *arc_dir, const char *path)
{
	ARC_ENTRY *arc_entry;
	DIR *dir;
	struct dirent *entry;

	errno = ENOERR;

	if ((dir = opendir(path)) != NULL)
	{
		/*
		 * Read the contents of the directory.
		 */

		while(((entry = readdir(dir)) != NULL) && (errno == ENOERR))
		{
			if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
			{
				int l;

				if (arc_dir->max_entries == 0)
				{
					arc_dir->entries = new ARC_ENTRY[20];
					arc_dir->max_entries = 20;
				}

				if (arc_dir->n_entries == arc_dir->max_entries)
				{
					arc_dir->max_entries += 50;
					arc_dir->entries = new (arc_dir->entries, arc_dir->max_entries) ARC_ENTRY;
				}

				l = strlen(entry->d_name);

				arc_entry = &arc_dir->entries[arc_dir->n_entries];
				arc_entry->name = new char[l + 1];
				strcpy(arc_entry->name, entry->d_name);
				arc_dir->n_entries++;
			}
		}

		closedir(dir);

		/*
		 * Read the file information of the files in the directory.
		 */

		if (errno == ENOERR)
		{
			struct stat data;
			char cur_path[256];
			int i;

			if (getcwd(cur_path, 256) != NULL)
			{
				chdir(path);

				for (i = 0; (i < arc_dir->n_entries) && (errno == ENOERR); i++)
				{
					arc_entry = &arc_dir->entries[i];

					if (!stat(arc_entry->name, &data))
					{
						arc_entry->selected = FALSE;
						arc_entry->os_type = OS;
						arc_entry->arj_flags = PATHSYM_FLAG;
						arc_entry->file_type = ((data.st_mode & S_IFMT) == S_IFDIR) ? DIR_TYPE :
											   (((data.st_mode & S_IFMT) == S_IFLNK) ? SYM_LINK : BINARY_TYPE);
						arc_entry->method = 0;
						arc_entry->compsize = 0;
						arc_entry->origsize = data.st_size;

						arc_entry->stat.device = data.st_dev;
						arc_entry->stat.inode = data.st_ino;
						arc_entry->stat.link_cnt = data.st_nlink;
						arc_entry->stat.length = data.st_size;
						arc_entry->stat.file_mode = mode_unix_to_os(data.st_mode);
						arc_entry->stat.mtime = time_unix_to_os(data.st_mtime);
						arc_entry->stat.atime = time_unix_to_os(data.st_atime);
						arc_entry->stat.ctime = time_unix_to_os(data.st_ctime);
						arc_entry->stat.uid = data.st_uid;
						arc_entry->stat.gid = data.st_gid;

						arc_entry->dir.max_entries = 0;
						arc_entry->dir.n_entries = 0;
						arc_entry->dir.entries = NULL;
					}
				}

				chdir(cur_path);

				for (i = 0; (i < arc_dir->n_entries) && (errno == ENOERR); i++)
				{
					arc_entry = &arc_dir->entries[i];
					if (arc_entry->file_type == DIR_TYPE)
					{
						char *new_path;

						if ((new_path = make_path(path, arc_entry->name, 0)) != NULL)
						{
							int result;

							result = add_directory(&arc_entry->dir, new_path);
							free(new_path);

							if (result != 0)
								return result;
						}
						else
							return 1;
					}
				}

				qsort(arc_dir->entries, arc_dir->n_entries, sizeof(ARC_ENTRY),
							sort_by_name);
			}
		}
	}

	return (errno == ENOERR) ? 0 :  1;
}

static int add_file(ARC_DIRECTORY *dir, DIR_ENTRY *dir_entry, const char *path)
{
	ARC_ENTRY *entry;

	if (dir->max_entries == 0)
	{
		dir->entries = new ARC_ENTRY[20];
		dir->max_entries = 20;
	}

	if (dir->n_entries >= dir->max_entries)
	{
		dir->max_entries += 20;
		dir->entries = new (dir->entries, dir->max_entries) ARC_ENTRY;
	}

	entry = &dir->entries[dir->n_entries];

	entry->name = new char[strlen(dir_entry->name) + 1];
	strcpy(entry->name, dir_entry->name);

	entry->selected = FALSE;
	entry->os_type = OS;

	entry->arj_flags = PATHSYM_FLAG;
	entry->file_type = ((mode_os_to_unix(dir_entry->stat.file_mode) & S_IFMT) == S_IFDIR) ? DIR_TYPE : BINARY_TYPE;	/* !!! */
	entry->method = 0;
	entry->compsize = 0;
	entry->origsize = dir_entry->stat.length;
	entry->stat = dir_entry->stat;

	entry->dir.max_entries = 0;
	entry->dir.n_entries = 0;
	entry->dir.entries = NULL;

	if (entry->file_type == DIR_TYPE)
	{
		char *new_path;

		if ((new_path = make_path(path, entry->name, 0)) != NULL)
		{
			int result;

			result = add_directory(&entry->dir, new_path);
			free(new_path);

			if (result != 0)
				return result;
		}
		else
			return 1;
	}

	dir->n_entries++;

	return 0;
}

/*
 * int directory_listbox::get_selection(ARC_DIRECTORY *dir)
 *
 * Build a tree of all selected files in the directory slider.
 *
 * Result: 0 no error, 1 error.
 */

int directory_listbox::get_selection(ARC_DIRECTORY *dir)
{
	DIR_ENTRY *entry;
	int i;
	mode_t mode;

	dir->max_entries = 0;
	dir->n_entries = 0;
	dir->entries = NULL;

	for (i = 0; i < n_entries; i++)
	{
		entry = &entries[i];
		mode = mode_os_to_unix(entry->stat.file_mode);

		if ((((mode & S_IFMT) == S_IFREG) ||
			((mode & S_IFMT) == S_IFDIR)) && (entry->selected))
		{
			if (add_file(dir, entry, path))
				return 1;
		}
	}

	return 0;
}

/*
 * void directory_listbox::close(void)
 *
 * Close the current directory in the listbox (i.e. go one level
 * up. The current directory should always be an absolute path.
 * This function can handle Atari (MS-DOS) and Unix-style absolute
 * paths.
 */

void directory_listbox::close(void)
{
	register char *new_path, *last_slash;

	new_path = new char[strlen(path) + 1];
	strcpy(new_path, path);

	if ((last_slash = strrchr(new_path, '/')) != NULL)
	{
		if ((new_path[0] == '/') && new_path[1])
		{
			if (last_slash == new_path)
				new_path[1] = 0;
			else
				*last_slash = 0;
			set_path(new_path, TRUE);
		}
		else if (new_path[0] && (new_path[1] == ':') &&
						 (new_path[2] == '/') && new_path[3])
		{
			if (last_slash == new_path + 2)
				new_path[3] = 0;
			else
				*last_slash = 0;
			set_path(new_path, TRUE);
		}
	}

	delete new_path;
}

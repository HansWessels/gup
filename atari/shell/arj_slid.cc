#include <aesbind.h>
#include <errno.h>
#include <new.h>
#include <stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gup.h"
#include "gup_err.h"
#include "options.h"
#include "compress.h"
#include "header.h"
#include "archive.h"

#include "cgl_dial.h"
#include "cgl_fsel.h"

#include "alert.h"
#include "arj.h"
#include "slider.h"
#include "arj_slid.h"
#include "add.h"
#include "dir.h"
#include "popup.h"
#include "main.h"
#include "message.h"
#include "resource.h"
#include "slider.h"
#include "util.h"

template<class T> inline T min(T x, T y)	{ return (x < y) ? x : y; }
template<class T> inline T max(T x, T y)	{ return (x > y) ? x : y; }

#define CONST_PART	15					/* Length of constant part of directory item. */
#define MAX_LENGTH	(SL_MAX_ITM_LEN - CONST_PART)	/* Maximum length of filename. */

static int sort_by_type(const void *e1, const void *e2)
{
	if (((((ARC_ENTRY *) e1)->file_type) == DIR_TYPE) &&
		((((ARC_ENTRY *) e2)->file_type) != DIR_TYPE))
		return -1;
	else if (((((ARC_ENTRY *) e2)->file_type) == DIR_TYPE) &&
			 ((((ARC_ENTRY *) e1)->file_type) != DIR_TYPE))
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
		return strcmp(((ARC_ENTRY *) e1)->name, ((ARC_ENTRY *) e2)->name);
}

/*
 * Implementation of archive_listbox class.
 */

archive_listbox::archive_listbox(void)
{
	name = NULL;
	arj_path = NULL;
	open = FALSE;
}

archive_listbox::~archive_listbox(void)
{
	free_memory();
}

void archive_listbox::free_memory(void)
{
	if (name != NULL)
	{
		delete[] name;
		name = NULL;
	}

	if (arj_path != NULL)
	{
		delete arj_path;
		arj_path = NULL;
	}

	if (open)
	{
		free_dir_tree(&dir);
		open = FALSE;
	}
}

void archive_listbox::set_curdir(register ARC_DIRECTORY *dir, int redraw)
{
#if 0
	register int i, width, n;

	n_selected = 0;												/* Number of selected objects is 0. */

	if (dir)
	{
		cur_dir = dir;
		max_namelength = 12;		/* Default filename length. */

		for (i = 0; i < dir->n_entries; i++)
		{
			ARC_ENTRY *entry = &dir->entries[i];

			max_namelength = max(max_namelength, (int) strlen(entry->name));
			entry->selected = FALSE;
		}

		qsort(dir->entries, dir->n_entries, sizeof(ARC_ENTRY), sort_by_name);

		width = max_namelength + CONST_PART;
		n = dir->n_entries;
	}
	else
	{
		width = 0;
		n = 0;
	}

	strcpy(((TEDINFO *) (dialog->get_tree())[MDARJDIR].ob_spec)->te_ptext, arj_path);

	init_vslider(n, 0, redraw);
	init_hslider(width, 0, redraw);

	if (redraw)
	{
		dialog->draw_object(ARJTEXT, 1);
		dialog->draw_object(MDARJDIR, 1);
	}

	((main_window *) dialog)->update_extract_button();
#endif
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

void archive_listbox::get_line(int line, char *string) const
{
	register ARC_ENTRY *entry = &cur_dir->entries[line];
	int i;
	register char *p;
	char *d = string, t[16];

	*string = 0;
#if 0
	/*
	 * Put the file type in the string.
	 */

	*d++ = ' ';

	switch(entry->file_type)
	{
	case BINARY_TYPE :
	case TEXT_TYPE :
		*d++ = ' ';
		break;
	case DIR_TYPE :
		*d++ = '\007';
		break;
	case COMMENT_TYPE :
	case LABEL_TYPE :
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

	if ((entry->file_type == BINARY_TYPE) || (entry->file_type == TEXT_TYPE))
	{
		int l;

		sprintf(t, "%ld", entry->origsize);
		l = strlen(t);
		p = t;
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
#endif
}

int archive_listbox::is_selected(int line) const
{
	return cur_dir->entries[line].selected;
}

void archive_listbox::handle_open(int line)
{
	ARC_ENTRY *entry;

	entry = &cur_dir->entries[line];

	if (entry->file_type == DIR_TYPE)
	{
		char *new_path;

		if (strlen(arj_path) == 0)
		{
			if ((new_path = (char *) malloc(strlen(entry->name) + 1)))
				strcpy(new_path, entry->name);
		}
		else
			new_path = make_path(arj_path, entry->name, 0);

		if (new_path)
		{
			free(arj_path);
			arj_path = new_path;
			set_curdir(&entry->dir, TRUE);
		}
		else
			xform_error(ENOMEM);
	}
}

void archive_listbox::handle_select(int line)
{
	if (cur_dir->entries[line].selected)
	{
		cur_dir->entries[line].selected = FALSE;
		n_selected--;
		if (n_selected == 0)
			((main_window *) dialog)->update_extract_button();
	}
	else
	{
		cur_dir->entries[line].selected = TRUE;
		n_selected++;
		if (n_selected == 1)
			((main_window *) dialog)->update_extract_button();
	}

	invert_line(line);
}

void archive_listbox::set_arc_name(const char *filename, int redraw)
{
#if 0
	char *archive = "";
	free_memory();

	if ((arj_path = (char *) malloc(strlen("") + 1)))
	{
		strcpy(arj_path, "");

		if (filename)
		{
			name = new char[strlen(filename) + 1];
			strcpy(name, filename);

			if (!get_arj_directory(name, &info, &dir, &gup_options))
			{
				archive = name;
				open = TRUE;
				set_curdir(&dir, redraw);
			}
			else
				set_curdir(NULL, redraw);
		}
		else
			set_curdir(NULL, redraw);
	}
	else
		set_curdir(NULL, redraw);

	strcpy(((TEDINFO *) dialog->get_tree()[MDARCHIV].ob_spec)->te_ptext, name);
	if (redraw)
		dialog->draw_object(MDARCHIV, 1);
#endif
}

void archive_listbox::compress(const char *path, ARC_DIRECTORY *dir)
{
	message_window msg_window;
	int exit;

	compress_files(name, path, arj_path, dir, &gup_options);
	exit = msg_window.execute(0);
	msg_window.change_object(exit, NORMAL);
}

void archive_listbox::decompress(const char *path)
{
	message_window msg_window;
	int exit;

	decompress_files(name, path, &dir);
	exit = msg_window.execute(0);
	msg_window.change_object(exit, NORMAL);
}

void archive_listbox::close(void)
{
	char *slash_ptr, *name_ptr;
	ARC_DIRECTORY *cur_dir;

	if (open && *arj_path)
	{
		name_ptr = arj_path;
		slash_ptr = strchr(name_ptr, PATH_CHAR);
		cur_dir = &dir;

		while (slash_ptr)
		{
			int i;

			*slash_ptr = 0;

			for (i = 0; i < cur_dir->n_entries; i++)
			{
				if ((strcmp(cur_dir->entries[i].name, name_ptr) == 0) &&
					(cur_dir->entries[i].file_type == DIR_TYPE))
				{
					cur_dir = &cur_dir->entries[i].dir;
					break;
				}
			}

			*slash_ptr = PATH_CHAR;
			name_ptr = slash_ptr + 1;
			slash_ptr = strchr(name_ptr, PATH_CHAR);
		}

		if ((slash_ptr = strrchr(arj_path, PATH_CHAR)) != NULL)
			*slash_ptr = 0;
		else
			*arj_path = 0;

		set_curdir(cur_dir, TRUE);
	}
	else
	{
		/*
		 * No archive or top level in archive. Call a fileselector to let
		 * the user select another archive.
		 */

		const char *file;

		if ((file = fselector(((main_window *) dialog)->get_path(), "*.ARJ",
													"", "Select archive", 1)) != NULL)
			set_arc_name(file, 1);
	}
}

#include <aesbind.h>
#include <errno.h>
#include <new.h>
#include <osbind.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "gup.h"
#include "gup_err.h"
#include "compress.h"
#include "options.h"
#include "header.h"
#include "archive.h"

#include "cgl_dial.h"

#include "alert.h"
#include "arj.h"
#include "slider.h"
#include "arj_slid.h"
#include "cgl_appl.h"
#include "cgl_dial.h"
#include "dir.h"
#include "popup.h"
#include "main.h"
#include "resource.h"
#include "slider.h"
#include "util.h"
#include "sh_opts.h"

#define EVNT_FLAGS	(MU_MESAG | MU_BUTTON | MU_KEYBD)

/*
 * Application class.
 */

class arj_app : public APPLICATION
{
 protected:
	int handle_menu(int item);

 public:
	arj_app(const char *prg_name);
};

long _stksize = 65536;			/* Stacksize is 64 Kbytes. */

char msg_resnfnd[] = "[1][Unable to find resource file.|"
										 "Resource file niet gevonden.|"
										 "Impossible de trouver le|fichier resource.|"
										 "Resource Datei nicht gefunden.][ OK ]";

static main_window *mwindow;

OPTIONS gup_options;

/********************************************************************
 *																																	*
 * Utility functions.																								*
 *																																	*
 ********************************************************************/

/*
 * void get_curdir(char *path, int maxlen)
 *
 * Get the current directory. If the current directory has the form
 *
 *		/dev/?/xxxx
 *
 * where ? is the current drive, convert the current directory to the
 * form
 *
 *		?:/xxxx
 *
 * Also unnecessary trailling slashes are stripped.
 */

void get_curdir(char *path, int maxlen)
{
	const char dev[] = "/dev/";
	register char *end;

	getcwd(path, maxlen);									/* Get current directory. */

	/*
	 * Convert path if necessary.
	 */

	if ((strncmp(path, dev, strlen(dev)) == 0) && (path[5] != 0) &&
			(path[6] == '/'))
	{
		register char *s, *d;

		path[0] = path[5];
		path[1] = ':';
		d = &path[2];
		s = &path[6];

		while (*s)
			*d++ = *s++;
		*d++ = 0;
	}

	/*
	 * Strip unnecessary trailling slashes.
	 */

	end = path + strlen(path) - 1;
	if ((*end == '/') && (strchr(path, '/') != end))
		*end = 0;
}

/*
 * void arj_sh_new_handler(void)
 *
 * New handler. This function is called by 'new' when there is no
 * free memory. Display an error message and terminate the
 * program.
 */

void arj_sh_new_handler(void)
{
	form_alert(1, "[1][Out of memory.][ OK ]");
	exit_cgl();
	exit(0);
}

/********************************************************************
 *																																	*
 * Implementation of drive_popup class.															*
 *																																	*
 ********************************************************************/

drive_popup::drive_popup(void) : popup_menu(MDDRVACT, MDDRVIND, -1)
{
	register long drvmap, mask;
	register int i, cur_drv, cur_item = 0;

	cur_drv = Dgetdrv();
	drvmap = Dsetdrv(cur_drv);
	mask = 1;
	ndrives = 0;

	for (i = 0; i < 32; i++)
	{
		if (drvmap & mask)
		{
			register char *str;

			str = new char [5];

			drv_list[ndrives] = str;
			drives[ndrives] = i;

			*str++ = ' ';
			*str++ = 'a' + i;
			*str++ = ':';
			*str++ = ' ';
			*str++ = 0;

			if (cur_drv == i)
				cur_item = i;

			ndrives++;
		}

		mask <<= 1;
	}

	set_items(ndrives, drv_list, cur_item);
	((TEDINFO *)(maindialog[MDDRVIND].ob_spec))->te_ptext = drv_list[cur_item];
}

drive_popup::~drive_popup(void)
{
	register int i;

	for (i = 0; i < ndrives; i++)
		delete drv_list[i];
}

void drive_popup::new_selected(int item, char *value)
{
	char path[4];

	((TEDINFO *)(maindialog[MDDRVIND].ob_spec))->te_ptext = value;
	dialog->draw_object(MDDRVIND, 1);

	path[0] = drives[item] + 'a';
	path[1] = ':';
	path[2] = '/';
	path[3] = 0;

	((main_window *) dialog)->set_path(path);
}

void drive_popup::set_drive(int new_drive)
{
	register int i;

	if (drives[cur_selected] != new_drive)
	{
		for (i = 0; i < ndrives; i++)
		{
			if (drives[i] == new_drive)
			{
				cur_selected = i;
				((TEDINFO *)(maindialog[MDDRVIND].ob_spec))->te_ptext = drv_list[i];
				dialog->draw_object(MDDRVIND, 1);
			}
		}
	}
}

/********************************************************************
 *																																	*
 * Implementation of main_window class.															*
 *																																	*
 ********************************************************************/

void main_window::handle_button(int button)
{
	ARC_DIRECTORY dir;

	switch(button)
	{
	case DIRCLOSE:												/* Close button directory list box. */
		dir_listbox.close();
		break;

	case ARJCLOSE:												/* Close button archive list box. */
		arj_listbox.close();
		break;

	case BTCOMP:													/* Compress button. */
		if (!dir_listbox.get_selection(&dir))
		{
			arj_listbox.compress(dir_listbox.get_path(), &dir);
			arj_listbox.set_arc_name(arj_listbox.get_arc_name(), TRUE);
		}
		free_dir_tree(&dir);
		break;

	case BTEXTRAC:												/* Extract button. */
		arj_listbox.decompress(dir_listbox.get_path());
		dir_listbox.refresh();
		break;
	}

	cgl_dialog::handle_button(button);
}

void main_window::cw_closed(void)
{
	exit_cgl();
	rsrc_free();
}

void main_window::update_extract_button(void)
{
	if (arj_listbox.archive() && (arj_listbox.number_selected() > 0))
		change_object(BTEXTRAC, NORMAL);
	else
		change_object(BTEXTRAC, DISABLED);
}

void main_window::update_compress_button(void)
{
	if (arj_listbox.archive() && (dir_listbox.number_selected() > 0))
		change_object(BTCOMP, NORMAL);
	else
		change_object(BTCOMP, DISABLED);
}

main_window::main_window(void) : cgl_dialog(maindialog)
{
	char path[256];

	/*
	 * Add special objects to the main window.
	 */

	add_object(&dir_listbox);
	add_object(&arj_listbox);
	add_object(&drive);

	/*
	 * Initiate the directory list box.
	 */

	dir_listbox.init_listbox(SL_HV, DIRTEXT, DIRLEFT, DIRRIGHT, DIRHSLID,
													 DIRHSPAR, DIRUP, DIRDOWN, DIRVSLID, DIRVSPAR);
	get_curdir(path, 256);
	dir_listbox.set_path(path, FALSE);

	/*
	 * Initiate the archive list box.
	 */

	arj_listbox.init_listbox(SL_HV, ARJTEXT, ARJLEFT, ARJRIGHT, ARJHSLID,
							 ARJHSPAR, ARJUP, ARJDOWN, ARJVSLID, ARJVSPAR);
	arj_listbox.set_arc_name(NULL, FALSE);

	/*
	 * Show the dialog box.
	 */

	nmopen(0, -1, -1, NULL);
}

/********************************************************************
 *																	*
 * Implementation of ARJ application class.							*
 *																	*
 ********************************************************************/

arj_app::arj_app(const char *prg_name) : APPLICATION(prg_name, FALSE)
{
}

int arj_app::handle_menu(int item)
{
	switch (item)
	{
	case MNABOUT:						/* About menu item. */
		cgld_do_dialog(infobox, 0);
		break;

	case MNNEW:							/* New archive menu item. */
		{
			int exit, stop = FALSE;
			cgl_dialog dialog(newarchive, CW_DIALOG);
			char *filename = ((TEDINFO *) newarchive[NANAME].ob_spec)->te_ptext;

			filename[0] = 0;
			dialog.mopen();

			do
			{
				if ((exit = dialog.execute(NANAME)) == NAOK)
				{
					if (get_arc_type(filename) == AT_UNKNOWN)
					{
						aprintf(1, "[1]['%s'|is not a valid archive name.][ OK ]",
													filename);
					}
					else
						stop = TRUE;
				}
				dialog.change_object(exit, NORMAL);
			} while ((exit == NAOK) && (!stop));

			dialog.mclose();

			if (exit == NAOK)
			{
				char *fullname;

				if ((fullname = make_path(mwindow->get_path(), filename, 0)) != NULL)
				{
					if (!exist(fullname))
					{
						gup_result result;

						if ((result = create_empty_archive(fullname, &gup_options)) != GUP_OK)
							show_gup_error(result);
						else
							mwindow->dir_listbox.refresh();
					}

					mwindow->arj_listbox.set_arc_name(fullname, TRUE);
					free(fullname);
				}
				else
					xform_error(ENOMEM);
			}
		}
		break;

	case MNPACKNG:						/* Change options. */
		edit_pack_options();
		break;

	case MNQUIT:						/* Quit menu item. */
		return TRUE;
		break;
	}

	return FALSE;
}

/********************************************************************
 *																	*
 * Main program.													*
 *																	*
 ********************************************************************/

int main(int argc, char *argv[])
{
	arj_app application("ARJ-Shell");

	(void) argc;
	(void) argv;

	if (!application)
		return 0;						/* Something went wrong, exit. */

	set_new_handler(arj_sh_new_handler);

	if (!rsc_init())
	{
		cgld_setdialmode(CGLD_WINDOW);	/* Dialog boxes in window. */
		application.set_menu_bar(menu);	/* Set the menu bar. */
		mwindow = new main_window;		/* Create the main window. */
		init_options();					/* Set default options. */
		graf_mouse(ARROW, NULL);
		application.run();				/* Run the application. */

		rsrc_free();
	}
	else
		form_alert(1, msg_resnfnd);

	return 0;
}

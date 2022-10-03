
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

#include "cgl_dial.h"
#include "cgl_evnt.h"
#include "cgl_util.h"
#include "popup.h"

#define FALSE		0
#define TRUE		1

template<class T> inline T min(T x, T y)	{ return (x < y) ? x : y; }
template<class T> inline T max(T x, T y)	{ return (x > y) ? x : y; }

extern GRECT cgl_desk;

/********************************************************************
 *																																	*
 * Constructor and destructor for the 'popup_menu' class.						*
 *																																	*
 ********************************************************************/

popup_menu::popup_menu(int act, int ind, int cyc)
{
	nitems = 0;
	items = NULL;
	cur_selected = 0;

	activator = act;
	indicator = ind;
	cycle = cyc;
}

popup_menu::~popup_menu(void)
{
}

/********************************************************************
 *																																	*
 * Private member functions of the 'popup_menu' class.							*
 *																																	*
 ********************************************************************/

int popup_menu::do_menu(int x, int y)
{
	OBJECT *tree = new OBJECT[nitems + 1];
	size_t maxlen = 0;
	int i, new_item = -1;
	GRECT box;
	char *seperator;
	CGL_SCRNBUF *buf;

	/*
	 * Build popup menu.
	 */

	for (i = 0; i < nitems; i++)
	{
		if (items[i] != NULL)
			maxlen = max(strlen(items[i]), maxlen);
	}

	seperator = (char *) alloca(maxlen + 1);

	for (i = 0; i < maxlen; i++)
		seperator[i] = '-';
	seperator[maxlen] = 0;

	tree[0].ob_next = -1;
	tree[0].ob_head = -1;
	tree[0].ob_tail = -1;
	tree[0].ob_type = G_BOX;
	tree[0].ob_flags = FL3DIND;
	tree[0].ob_state = NORMAL;
	tree[0].ob_spec = WHITE | (IP_HOLLOW << 4) | (BLACK << 12) | 0xFF0000;
	tree[0].ob_x = 0;
	tree[0].ob_y = 0;
	tree[0].ob_width = maxlen;
	tree[0].ob_height = nitems;

	rsrc_obfix(tree, 0);

	for(i = 1; i <= nitems; i++)
	{
		tree[i].ob_next = -1;
		tree[i].ob_head = -1;
		tree[i].ob_tail = -1;
		tree[i].ob_type = G_STRING;
		tree[i].ob_flags = 0;
		tree[i].ob_state = (items[i-1] == NULL) ? DISABLED : ((i == cur_selected + 1) ?  SELECTED : NORMAL);
		tree[i].ob_spec = (items[i-1] == NULL) ? (long) seperator : (long) items[i-1];
		tree[i].ob_x = 0;
		tree[i].ob_y = i - 1;
		tree[i].ob_width = maxlen;
		tree[i].ob_height = 1;

		objc_add(tree, 0, i);
		rsrc_obfix(tree, i);
	}

	tree[nitems].ob_flags |= LASTOB;
	tree[0].ob_x = x;
	tree[0].ob_y = y - cur_selected * tree[1].ob_height;

	if ((tree->ob_x + tree->ob_width + 3) > (cgl_desk.g_x + cgl_desk.g_w))
		tree->ob_x = cgl_desk.g_x + cgl_desk.g_w - tree->ob_width - 3;
	if ((tree->ob_y + tree->ob_height + 3) > (cgl_desk.g_y + cgl_desk.g_h))
		tree->ob_y = cgl_desk.g_y + cgl_desk.g_h - tree->ob_height - 3;

	if ((tree->ob_x - 3) < cgl_desk.g_x)
		tree->ob_x = cgl_desk.g_x + 3;
	if ((tree->ob_y - 3) <= cgl_desk.g_y)
		tree->ob_y = cgl_desk.g_y + 4;

	cgl_update(BEG_MCTRL);

	cgl_objrect(tree, 0, &box);

	box.g_x -= 3;
	box.g_y -= 3;
	box.g_w += 6;
	box.g_h += 6;

	if ((buf = cgl_save_area(&box)) != NULL)
	{
		int item = cur_selected + 1, stop = FALSE;
		int exit_mstate;

		exit_mstate = (cgl_button_state() & 1) ? 0 : 1;
		objc_draw(tree, 0, MAX_DEPTH, box.g_x, box.g_y, box.g_w, box.g_h);

		do
		{
			int mx, my, oitem, dummy;

			oitem = item;

			stop = cgl_mouse_event(exit_mstate, &mx, &my, &dummy);

			if (((item = objc_find(tree, 0, MAX_DEPTH, mx, my)) >= 0) &&
					(tree[item].ob_state & DISABLED))
				item = -1;

			if (item != oitem)
			{
				if (oitem > 0)
					objc_change(tree, oitem, 0, box.g_x, box.g_y, box.g_w, box.g_h, NORMAL, TRUE);
				if (item > 0)
					objc_change(tree, item, 0, box.g_x, box.g_y, box.g_w, box.g_h, SELECTED, TRUE);
			}
		} while (stop == FALSE);

		cgl_restore_area(buf);							/* Restore screen below the popup menu. */

		while (cgl_button_state() & 1);			/* Wait until the mouse button is released. */

		if (item > 0)
			new_item = item - 1;
	}

	cgl_update(END_MCTRL);

	delete tree;

	return new_item;
}

int popup_menu::handle_select(int button, int x, int y)
{
	button &= 0x7FFF;

/*	if (button == activator)
	else*/ if (button == indicator)
	{
		/*
		 * Indicator object. If the user clicks on this object, show
		 * the popup menu at the position of indicator.
		 */

		if (nitems > 0)
		{
			int new_sel, x, y;

			objc_offset(dialog->get_tree(), indicator, &x, &y);

			if ((new_sel = do_menu(x, y)) >= 0)
			{
				cur_selected = new_sel;
				new_selected(cur_selected, items[cur_selected]);
			}

			return TRUE;
		}
	}
	else if (button == cycle)
	{
		/*
		 * Cycle button. Increase the currently selected item and
		 * call the callback function 'new_selected'.
		 */

		if (nitems != 0)
		{
			cur_selected = (cur_selected + 1) % nitems;
			new_selected(cur_selected, items[cur_selected]);
		}

		return TRUE;
	}

	return FALSE;
}

/********************************************************************
 *																																	*
 * Public member functions of the 'popup_menu' class.								*
 *																																	*
 ********************************************************************/

void popup_menu::set_items(int n, char *itms[], int initial)
{
	nitems = n;
	items = itms;
	cur_selected = initial;
}

void popup_menu::new_selected(int item, char *value)
{
}


#include <ctype.h>
#include <osbind.h>
#include <stdlib.h>
#include <string.h>

#include "cgl_dial.h"
#include "cgl_evnt.h"
#include "cgl_util.h"
#include "cglscode.h"
#include "internal.h"

static cgl_dial_pos_mode cgld_posmode = CGLD_CENTERED;	/* Position mode of dialog boxes. */
static cgl_dial_mode cgld_dialmode = CGLD_WINDOW;	/* Dialog mode. */

unsigned char cgld_emode_specs[CGLD_EMODECOUNT] =
{
	0,																		/* (0) */
	0,																		/* (1) */
	0,																		/* (2) */
	0,																		/* (3) */
	0,																		/* (4) */
	__XD_IS_ELEMENT,			/* XD_DRAGBOX(5): button rechtsboven waarmee dialoogbox verplaatst kan worden. */
	__XD_IS_ELEMENT |
	__XD_IS_SELTEXT,			/* XD_ROUNDRB(6): ronde radiobutton. */
	__XD_IS_ELEMENT |
	__XD_IS_SELTEXT,			/* XD_RECTBUT(7): rechthoekige button. */
	__XD_IS_ELEMENT |
	__XD_IS_SELTEXT,			/* XD_BUTTON(8): gewone button (met toetsbediening! I_A). */
	__XD_IS_ELEMENT,			/* XD_RBUTPAR(9): Rechthoek met titel. */
	__XD_IS_ELEMENT,			/* XD_RBUTTITLE(10): IA: Underlined title */
	__XD_IS_ELEMENT |
	__XD_IS_SELTEXT,			/* XD_RECTBUTTRI(11): IA: rectangle button: tri-state! */
	__XD_IS_ELEMENT |
	__XD_IS_SELTEXT,			/* XD_CYCLBUT(12): IA: cycling button. used with pop-ups mostly. */
	__XD_IS_NOTYETDEF,										/* (13) */
	__XD_IS_NOTYETDEF,										/* (14) */
	__XD_IS_NOTYETDEF,										/* (15) */
	__XD_IS_NOTYETDEF,										/* (16) */
	__XD_IS_NOTYETDEF,										/* (17) */
	__XD_IS_NOTYETDEF,										/* (18) */
	__XD_IS_NOTYETDEF,										/* (19) */
	__XD_IS_NOTYETDEF,										/* (20) */
	__XD_IS_NOTYETDEF,										/* (21) */
	__XD_IS_NOTYETDEF,										/* (22) */
	__XD_IS_NOTYETDEF,										/* (23) */
	__XD_IS_NOTYETDEF,										/* (24) */
	__XD_IS_NOTYETDEF,										/* (25) */
	__XD_IS_NOTYETDEF,										/* (26) */
	__XD_IS_NOTYETDEF,										/* (27) */
	__XD_IS_NOTYETDEF,										/* (28) */
	__XD_IS_NOTYETDEF,										/* (29) */
	__XD_IS_NOTYETDEF,										/* (30) */
	__XD_IS_NOTYETDEF,										/* (31) */
	__XD_IS_NOTYETDEF,										/* (32) */
	__XD_IS_NOTYETDEF,										/* (33) */
	__XD_IS_NOTYETDEF,										/* (34) */
	__XD_IS_NOTYETDEF,										/* (35) */
	__XD_IS_NOTYETDEF,										/* (36) */
	__XD_IS_NOTYETDEF,										/* (37) */
	__XD_IS_NOTYETDEF,										/* (38) */
	__XD_IS_NOTYETDEF,										/* (39) */
	__XD_IS_NOTYETDEF,										/* (40) */
	__XD_IS_NOTYETDEF,										/* (41) */
	__XD_IS_NOTYETDEF,										/* (42) */
	__XD_IS_NOTYETDEF,										/* (43) */
	__XD_IS_NOTYETDEF,										/* (44) */
	__XD_IS_NOTYETDEF,										/* (45) */
	__XD_IS_NOTYETDEF,										/* (46) */
	__XD_IS_NOTYETDEF,										/* (47) */
	__XD_IS_NOTYETDEF,										/* (48) */
	__XD_IS_NOTYETDEF,										/* (49) */
	__XD_IS_NOTYETDEF,										/* (50) */
	__XD_IS_NOTYETDEF,										/* (51) */
	__XD_IS_SPECIALKEY,			/* XD_UP(52): Codes voor buttons met bediening cursortoetsen */
	__XD_IS_SPECIALKEY,			/* XD_DOWN(53) */
	__XD_IS_SPECIALKEY,			/* XD_LEFT(54) */
	__XD_IS_SPECIALKEY,			/* XD_RIGHT(55) */
	__XD_IS_SPECIALKEY,			/* XD_SUP(56) */
	__XD_IS_SPECIALKEY,			/* XD_SDOWN(57) */
	__XD_IS_SPECIALKEY,			/* XD_SLEFT(58) */
	__XD_IS_SPECIALKEY,			/* XD_SRIGHT(59) */
	__XD_IS_SPECIALKEY,			/* XD_CUP(60) */
	__XD_IS_SPECIALKEY,			/* XD_CDOWN(61) */
	__XD_IS_SPECIALKEY,			/* XD_CLEFT(62) */
	__XD_IS_SPECIALKEY			/* XD_CRIGHT(63) */
};

static int ckeytab[] =
{
	CURUP,																/* CGLD_UP */
	CURDOWN,
	CURLEFT,
	CURRIGHT,
	SHFT_CURUP,
	SHFT_CURDOWN,
	SHFT_CURLEFT,
	SHFT_CURRIGHT,
	CTL_CURUP,
	CTL_CURDOWN,
	CTL_CURLEFT,
	CTL_CURRIGHT
};

/*
 * Inline functions to check if an object is an extended object known
 * by the library.
 */

static inline int cgld_is_xtndbutton(int emode)		\
	{ return ((cgld_emode_specs[emode] & __XD_IS_SELTEXT) != 0); }
static inline int cgld_is_xtndspecialkey(int emode)	\
	{ return ((cgld_emode_specs[emode] & __XD_IS_SPECIALKEY) != 0); }

/********************************************************************
 *																																	*
 * Utility functions.																								*
 *																																	*
 ********************************************************************/

/*
 * static void calc_border(OBJECT *tree, int *xl, int *xr, int *yu,
 *												 int *yl)
 *
 * Calculate the border around a dialog box.
 */

static void calc_border(OBJECT *tree, int *xl, int *xr, int *yu, int *yl)
{
	int old_x, old_y, x, y, w, h;

	old_x = tree->ob_x;
	old_y = tree->ob_y;

	form_center(tree, &x, &y, &w, &h);

	*xl = tree->ob_x - x;
	*xr = w - tree->ob_width - *xl;
	*yu = tree->ob_y - y;
	*yl = h - tree->ob_height - *yu;

	tree->ob_x = old_x;
	tree->ob_y = old_y;
}

/*
 * static int cgld_abs_curx(OBJECT *tree, int object, int curx)
 *
 * Calculate the position of the cursor in the template string
 * from the position of the cursor in the text string.
 */

static int cgld_abs_curx(OBJECT *tree, int object, int curx)
{
	char *tmplt = tree[object].ob_spec.tedinfo->te_ptmplt, *s, *h;

	if ((s = strchr(tmplt, '_')) != NULL)
	{
		while ((curx > 0) && *s)
		{
			if (*s++ == '_')
				curx--;
		}

		if ((h = strchr(s, '_')) != NULL)
			s = h;

		return (int) (s - tmplt);
	}
	else
		return 0;
}

static int cgld_movebutton(OBJECT *tree)
{
	OBJECT *c_obj;
	int cur = 0;

	for (;;)
	{
		c_obj = &tree[cur];

		if (((c_obj->ob_type >> 8) & 0xFF) == CGLD_DRAGBOX)
			return cur;

		if (c_obj->ob_flags & LASTOB)
			return -1;

		cur++;
	}
}

/*
 * static void str_delete(char *s, int pos)
 *
 * Delete a character from a string.
 */

static void str_delete(char *s, int pos)
{
	char *h = &s[pos], ch;

	if (*h)
	{
		do
		{
			ch = h[1];
			*h++ = ch;
		}
		while (ch);
	}
}

/*
 * static void str_insert(char *s, int pos, int ch, int curlen,
 *												int maxlen)
 *
 * Insert a character in a string.
 */

static void str_insert(char *s, int pos, int ch, int curlen, int maxlen)
{
	int i, m;

	if (pos < maxlen)
	{
		m = curlen + ((curlen < maxlen) ? 1 : -1);
		for (i = m; i > pos; i--)
			s[i] = s[i - 1];
		s[pos] = (char) ch;
	}
}

/*
 * static int cgld_rel_curx(OBJECT *tree, int edit_obj, int curx)
 *
 * Calculate the position of the cursor in the text string from
 * the position of the cursor in the template string.
 */

static int cgld_rel_curx(OBJECT *tree, int edit_obj, int curx)
{
	int x = 0, i;
	char *tmplt = tree[edit_obj].ob_spec.tedinfo->te_ptmplt;

	for (i = 0; i < curx; i++)
	{
		if (tmplt[i] == '_')
			x++;
	}

	return x;
}

/*
 * static int cgld_chk_key(char *valid, int pos, int key)
 *
 * Check if a key is allowed on a given position in a string.
 */

static int cgld_chk_key(char *valid, int pos, int key)
{
	char cvalid = valid[pos];
	int ch = key & 0xFF, cch = key & 0xDF;

	if (!(key & (CGL_SCANCODE | CGL_CTRL | CGL_ALT)))
	{
		if (cvalid == 'X')
			return ch;

		if (ch < 0x80)
		{
			switch (cvalid)
			{
			case 'N':
			case 'n':
			case '9':
				if ((ch >= '0') && (ch <= '9'))
					return ch;
				if (cvalid == '9')
					break;
			case 'A':
			case 'a':
				if (ch == ' ')
					return ch;
				if ((cch >= 'A') && (cch <= 'Z'))
					return (cvalid & 0x20) ? ch : cch;
				break;
			case 'F':
			case 'P':
			case 'p':
				if ((cch >= 'A') && (cch <= 'Z'))
					return cch;
				if ((ch == '_') || ((ch >= '0') && (ch <= '9')) || (ch == 'œ') || (ch == ':'))
					return ch;
				if ((cvalid != 'p') && ((ch == '?') || (ch == '*')))
					return ch;
				if ((cvalid != 'F') && ((ch == '\\') || (ch == '.')))
					return ch;
				break;
			}
		}
	}

	return 0;
}

/*
 * static int cgld_chk_skip(OBJECT *tree, int edit_obj, int key)
 *
 * Check if a key skips to a new position in the current edit
 * object.
 */

static int cgld_chk_skip(OBJECT *tree, int edit_obj, int key)
{
	char *s = tree[edit_obj].ob_spec.tedinfo->te_ptmplt, *h;

	if (key & (CGL_SCANCODE | CGL_CTRL | CGL_ALT))
		return 0;

	if (((h = strchr(s, key & 0xFF)) != NULL) && ((h = strchr(h, '_')) != NULL))
		return cgld_rel_curx(tree, edit_obj, (int) (h - s));

	return 0;
}

/*

 * static int cgld_selectable(OBJECT *tree, int object)
 *
 * Check if an object is selectable.
 */

static int cgld_selectable(OBJECT *tree, int object)
{
	int parent;
	OBJECT *o = &tree[object];

	if ((o->ob_flags & HIDETREE) || (o->ob_state & DISABLED))
		return FALSE;

	parent = cgl_obj_parent(tree, object);

	while (parent > 0)
	{
		o = tree + parent;

		if ((o->ob_flags & HIDETREE) || (o->ob_state & DISABLED))
			return FALSE;

		parent = cgl_obj_parent(tree, parent);
	}

	return TRUE;
}

/*
 * int cgld_find_obj(OBJECT *tree, int start, int which)
 *
 * Find the next or previous editable object or the default
 * object.
 */

int cgld_find_obj(OBJECT *tree, int start, int which)
{
	int obj, flag, theflag, inc;

	obj = 0;
	flag = EDITABLE;
	inc = 1;

	switch (which)
	{
	case FMD_BACKWARD:
		inc = -1;
	case FMD_FORWARD:
		obj = start + inc;
		break;
	case FMD_DEFLT:
		flag = DEFAULT;
		break;
	}

	while (obj >= 0)
	{
		theflag = tree[obj].ob_flags;
		if ((theflag & flag) && cgld_selectable(tree, obj))
			return obj;
		if (theflag & LASTOB)
			obj = -1;
		else
			obj += inc;
	}

	return start;
}

/*
 * void cgl_dialog::handle_rbutton(int parent, int object)
 *
 * Handle the selection of a radio button in a dialog box.
 */

void cgl_dialog::handle_rbutton(int parent, int object)
{
	int i, prvstate, newstate;

	i = dial_tree[parent].ob_head;

	/*
	 * If the dialog box is in a window switch the mouse and the
	 * text cursor off. The mouse is switched of to increase the
	 * speed.
	 */

	if (dial_mode == CGLD_WINDOW)
	{
		cgl_mouse_off();
		cursor_off();
	}

	/*
	 * Set all radio buttons in the parent to the correct state.
	 */
 
	do
	{
		if (dial_tree[i].ob_flags & RBUTTON)
		{
			prvstate = dial_tree[i].ob_state;
			newstate = (i == object) ? prvstate | SELECTED : prvstate & ~SELECTED;
			if (newstate != prvstate)
				change_object(i, newstate);
		}

		i = dial_tree[i].ob_next;
	}
	while (i != parent);

	/*
	 * If the dialog box is in a window switch the mouse and the
	 * text cursor on.
	 */

	if (dial_mode == CGLD_WINDOW)
	{
		cursor_on();
		cgl_mouse_on();
	}
}

/********************************************************************
 *																																	*
 * Constructors and destructors for the cgl_dialog class.						*
 *																																	*
 ********************************************************************/

cgl_dialog::cgl_dialog(OBJECT *tree, int type) : cwindow(type)
{
	dial_tree = tree;
	objects = NULL;

	curs_mfdb.fd_addr = NULL;
	curs_mfdb.fd_w = 1;
	curs_mfdb.fd_h = cgl_regular_font.fnt_chh + 4;
	curs_mfdb.fd_wdwidth = 1;
	curs_mfdb.fd_stand = 0;
	curs_mfdb.fd_nplanes = cgl_nplanes;
	curs_mfdb.fd_r1 = 0;
	curs_mfdb.fd_r2 = 0;
	curs_mfdb.fd_r3 = 0;

	dl_flags.visible = FALSE;

	set_keys();														/* Get keyboard short cuts. */
	dmb = cgld_movebutton(tree);					/* Get index of dialog move button. */
}

cgl_dialog::~cgl_dialog(void)
{
	if (dl_flags.visible)
	{
		if (win_type == CW_DIALOG)
			mclose();
		else
			nmclose();
	}

	if (curs_mfdb.fd_addr != NULL)
		free(curs_mfdb.fd_addr);
}

/********************************************************************
 *																																	*
 * Window event handlers of the cgl_dialog class.										*
 *																																	*
 ********************************************************************/

/*
 * int cgl_dialog::cw_hndlkey(int key, int kstate)
 *
 * Handle a keyboard event in a dialog box.
 */

int cgl_dialog::cw_hndlkey(int key, int kstate)
{
	register int next_obj, kr;
	register int key_handled = TRUE;

	cgl_update(BEG_UPDATE);

	if ((next_obj = find_key(key)) >= 0)
		dl_flags.cont = form_button(next_obj, 1, &next_obj);
	else
	{
		dl_flags.cont = form_keybd(0, key, &next_obj, &kr);
		if (kr)
			key_handled = edit_char(kr);
	}

	if (dl_flags.cont)
	{
		if (next_obj != 0)
			edit_init(next_obj, -1);
		cgl_update(END_UPDATE);
	}
	else
	{
		cgl_update(END_UPDATE);
		handle_button(next_obj);
		exit_obj = next_obj;
	}

	return key_handled;
}

/*
 * void cgl_dialog::cw_hndlbutton(int x, int y, int n, int bstate, int kstate)
 *
 * Handle a button event in a dialog box.
 */

void cgl_dialog::cw_hndlbutton(int x, int y, int n, int bstate, int kstate)
{
	register int next_obj, cmode = -1;
	register cgl_object *object;

	if ((next_obj = objc_find(dial_tree, ROOT, MAX_DEPTH, x, y)) != -1)
	{
		register int handled = FALSE;

		/*
		 * The user clicked on a valid object.
		 */

		cgl_update(BEG_UPDATE);

		if ((dl_flags.cont = form_button(next_obj, n, &next_obj)) != FALSE)
			cmode = x;

		/*
		 * Check if an object is selected. If true, handle the
		 * selection of the object.
		 */

		object = objects;

		while(object && !handled)
		{
			if (!(handled = object->handle_select(next_obj, x, y)))
				object = object->next;
		}

		if (!handled)
		{
			/*
			 * If the move button is selected, let the user drag the
			 * dialog box to a new position.
			 */

			if ((next_obj & 0x7FFF) == dmb)
			{
				/*
				 * The user clicked on the move button. Let the user move
				 * the dialog box and make continue TRUE, to prevent the
				 * dialog from terminating.
				 */

				int nx, ny;

				graf_dragbox(dial_size.g_w, dial_size.g_h, dial_size.g_x,
										 dial_size.g_y, cgl_desk.g_x, cgl_desk.g_y,
										 cgl_desk.g_w, cgl_desk.g_h, &nx, &ny);

				restore();
				set_position(nx, ny);
				save();
				doredraw(2, &dial_size, ROOT, MAX_DEPTH);

				dl_flags.cont = TRUE;
				next_obj = 0;
			}

			if (dl_flags.cont)
			{
				/*
				 * Continue is TRUE. If 'next_obj' is not zero, the user wants
				 * to edit another editable field. Move the text cursor to this
				 * field.
				 */

				if (next_obj != 0)
					edit_init(next_obj, cmode);
				cgl_update(END_UPDATE);
			}
			else
			{
				/*
				 * Continue is FALSE. The user pressed a exit button in the
				 * dialog. Call the call back function and set 'exit_obj' to
				 * 'next_obj'.
				 */

				cgl_update(END_UPDATE);
				handle_button(next_obj);
				exit_obj = next_obj;
			}
		}
		else
		{
			/*
			 * The user has selected an object in the object list.
			 */

			dl_flags.cont = TRUE;							/* Do not exit the dialog. */
			cgl_update(END_UPDATE);
		}
	}
	else if (win_type == CW_DIALOG)
	{
		/*
		 * The user did not click on a valid object and the dialog is
		 * modal, beep.
		 */

		Bconout(2, 7);
	}
}

/*
 * void cgl_dialog::cw_draw(GRECT *area)
 *
 * Draw the dialog box window.
 *
 * Parameters:
 *
 * mode	- redraw mode:
 *				0 - draw both the dialog and the cursor
 *				1 - only draw given objects
 *				2 - draw given objects and cursor
 *				3 - only draw the cursor
 *				4 - restore the area underneath the cursor
 * area	- area to draw.
 * args	- arguments depending on mode.
 */

void cgl_dialog::cw_draw(int mode, GRECT *area, va_list args)
{
	GRECT cursor;
	int start, depth, draw_cur, draw_dlg;

	switch(mode)
	{
	case 0:																/* Draw everything. */
		start = ROOT;												/* Start object. */
		depth = MAX_DEPTH;									/* Depth. */
		draw_dlg = TRUE;										/* Draw dialog. */
		draw_cur = (edit_object > 0);				/* If cursor visible, draw cursor. */
		break;

	case 1:																/* Only draw given objects. */
	case 2:																/* Draw given objects and cursor. */
		start = va_arg(args, int);					/* Get start object. */
		depth = va_arg(args, int);					/* Get depth. */
		draw_dlg = TRUE;
		draw_cur = ((edit_object > 0) && (mode != 1));
		break;

	case 3:																/* Only draw the cursor. */
		draw_dlg = FALSE;
		draw_cur = TRUE;
		break;

	case 4:																/* Restore the area underneath the cursor. */
		if (curs_mfdb.fd_addr != NULL)
		{
			GRECT cursor, in;
			MFDB dmfdb;
			int pxy[8];

			calc_cursor(&cursor);
			dmfdb.fd_addr = NULL;

			if (cgl_rcintersect(area, &cursor, &in))
			{
				cgl_clip_on(&cursor);

				pxy[0] = in.g_x - cursor.g_x;
				pxy[1] = in.g_y - cursor.g_y;
				pxy[2] = pxy[0] + in.g_w - 1;
				pxy[3] = pxy[1] + in.g_h - 1;
				pxy[4] = in.g_x;
				pxy[5] = in.g_y;
				pxy[6] = in.g_x + in.g_w - 1;
				pxy[7] = in.g_y + in.g_h - 1;

				vro_cpyfm(cgl_vdi_handle, S_ONLY, pxy, &curs_mfdb,
									&dmfdb);

				cgl_clip_off();
			}
		}
		return;

	default:
		return;
	}

	/*
	 * Draw the dialog if requested.
	 */

	if (draw_dlg)
		objc_draw(dial_tree, start, depth, area->g_x, area->g_y, area->g_w, area->g_h);

	/*
	 * Draw the cursor if requested.
	 */

	if (draw_cur)
	{
		calc_cursor(&cursor);
		if (cgl_rcintersect(area, &cursor, &cursor))
			redraw_cursor(&cursor);
	}
}

/*
 * void cgl_dialog::cw_moved(GRECT *newpos)
 *
 * Handle the WM_MOVED message for a dialog window.
 */

void cgl_dialog::cw_moved(GRECT *newpos)
{
	GRECT work;

	cw_set(WF_CURRXYWH, newpos);
	cw_get(WF_WORKXYWH, &work);
	set_position(work.g_x, work.g_y);
}

/********************************************************************
 *																																	*
 * Private member functions of the 'cgl_dialog' class.							*
 *																																	*
 ********************************************************************/

/*
 * void cgl_dialog::set_position(int x, int y)
 *
 * Set the x and y position of a dialog box.
 */

void cgl_dialog::set_position(int x, int y)
{
	int dx, dy;

	dx = x - dial_size.g_x;
	dy = y - dial_size.g_y;

	dial_tree->ob_x += dx;
	dial_tree->ob_y += dy;

	dial_size.g_x = x;
	dial_size.g_y = y;
}

/*
 * void cgl_dialog::calc_cursor(GRECT *cursor)
 *
 * Calculate the size and position of the cursor.
 */

void cgl_dialog::calc_cursor(GRECT *cursor)
{
	objc_offset(dial_tree, edit_object, &cursor->g_x, &cursor->g_y);

	cursor->g_x += cgld_abs_curx(dial_tree, edit_object, cursor_x) * cgl_regular_font.fnt_chw;
	cursor->g_y -= 2;
	cursor->g_w = 1;
	cursor->g_h = cgl_regular_font.fnt_chh + 4;
}

/*
 * void cgl_dialog::redraw_cursor(GRECT *area)
 *
 * Redraw a part of the cursor. The area under the cursor is
 * saved in a buffer.
 */

void cgl_dialog::redraw_cursor(GRECT *area)
{
	if (curs_mfdb.fd_addr == NULL)
		curs_mfdb.fd_addr = malloc((long) curs_mfdb.fd_wdwidth *
															 (long) curs_mfdb.fd_h *
															 (long) cgl_nplanes * 2L);

	if (curs_mfdb.fd_addr != NULL)
	{
		GRECT cursor, r;
		MFDB smfdb;
		int pxy[8];

		calc_cursor(&cursor);
		smfdb.fd_addr = NULL;

		if (cgl_rcintersect(area, &cursor, &r))
		{
			/* Save area below cursor. */
			
			pxy[0] = r.g_x;
			pxy[1] = r.g_y;
			pxy[2] = r.g_x + r.g_w - 1;
			pxy[3] = r.g_y + r.g_h - 1;
			pxy[4] = r.g_x - cursor.g_x;
			pxy[5] = r.g_y - cursor.g_y;
			pxy[6] = pxy[4] + r.g_w - 1;
			pxy[7] = pxy[5] + r.g_h - 1;

			vro_cpyfm(cgl_vdi_handle, S_ONLY, pxy, &smfdb,
								&curs_mfdb);

			/* Draw cursor. */
			
			cgl_clip_on(&r);

			vswr_mode(cgl_vdi_handle, MD_REPLACE);
			vsl_color(cgl_vdi_handle, 1);
			vsl_ends(cgl_vdi_handle, 0, 0);
			vsl_type(cgl_vdi_handle, 1);
			vsl_width(cgl_vdi_handle, 1);

			pxy[0] = pxy[2] = cursor.g_x;
			pxy[1] = cursor.g_y;
			pxy[3] = cursor.g_y + cursor.g_h - 1;

			v_pline(cgl_vdi_handle, 2, pxy);

			cgl_clip_off();
		}
	}
}

/*
 * void cgl_dialog::cursor_on(void)
 *
 * Show the text cursor.
 */

void cgl_dialog::cursor_on(void)
{
	curs_cnt -= 1;

	if (curs_cnt == 0)
		doredraw(3, &dial_size, 0, 0);
}

/*
 * void cgl_dialog::cursor_off(void)
 *
 * Hide the cursor.
 */

void cgl_dialog::cursor_off(void)
{
	curs_cnt += 1;

	if (curs_cnt == 1)
		doredraw(4, &dial_size, 0, 0);
}

/*
 * void cgl_dialog::doredraw(int mode, GRECT *area, int start, int depth)
 *
 * Redraw a given part of the dialog box.
 */

void cgl_dialog::doredraw(int mode, GRECT *area, int start, int depth)
{
	if (dial_mode != CGLD_WINDOW)
	{
		cgl_mouse_off();
		cw_draw(mode, area, start, depth);
		cgl_mouse_on();
	}
	else
		cw_redraw(mode, area, start, depth);
}

/*
 * void cgl_dialog::set_keys(void)
 *
 * Determine which keys are associated with the buttons in
 * the object tree.
 */

void cgl_dialog::set_keys(void)
{
	OBJECT *c_obj;
	int etype, i = 0, cur = 0, ch;
	char *h;

	for (;;)
	{
		c_obj = &dial_tree[cur];

		etype = (c_obj->ob_type >> 8) & 0xFF;

		if (cgld_is_xtndbutton(etype))
		{
			/*
			 * Find single '#'.
			 */

			for (h = (char *) cgl_get_obspec(c_obj);
					(h = strchr(h, '#')) && (h[1] == '#');
					h += 2);

			if (h)
			{
				ch = toupper((int) h[1]);

				if (((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9')))
					kinfo[i].key = CGL_ALT | ch;
				else
					kinfo[i].key = 0;

				kinfo[i].object = cur;
				i++;
			}
		}

		if (cgld_is_xtndspecialkey(etype))
		{
			kinfo[i].key = ckeytab[etype - CGLD_UP];
			kinfo[i].object = cur;
			i++;
		}

		if ((c_obj->ob_flags & LASTOB) || (i == CGLD_MAXKEYS))
		{
			nkeys = i;
			return;
		}

		cur++;
	}
}

/*
 * int cgl_dialog::find_key(int key)
 *
 * Find the button which is associated with a given key.
 */

int cgl_dialog::find_key(int key)
{
	int i;

	for (i = 0; i < nkeys; i++)
	{
		if ((kinfo[i].key == key) && cgld_selectable(dial_tree, kinfo[i].object))
			return kinfo[i].object;
	}

	return -1;
}

/*
 * void cgl_dialog::edit_init(int object, int curx)
 *
 * Start editing an object in a dialog box.
 */

void cgl_dialog::edit_init(int object, int curx)
{
	int x, dummy, maxlen;

	if ((object > 0) && cgld_selectable(dial_tree, object))
	{
		maxlen = (int) strlen(dial_tree[object].ob_spec.tedinfo->te_ptext);

		if (curx >= 0)
		{
			objc_offset(dial_tree, object, &x, &dummy);
			x = (curx - x + cgl_regular_font.fnt_chw / 2) / cgl_regular_font.fnt_chw;
			if ((x = cgld_rel_curx(dial_tree, object, x)) > maxlen)
				x = maxlen;
		}

		cgl_update(BEG_UPDATE);

		if (edit_object != object)
		{
			edit_end();
			edit_object = object;
			cursor_x = (curx == -1) ? maxlen : x;
			cursor_on();
		}
		else if ((curx >= 0) && (x != cursor_x))
		{
			cursor_off();
			cursor_x = x;
			cursor_on();
		}

		cgl_update(END_UPDATE);
	}
}

/*
 * int cgl_dialog::edit_char(int key)
 *
 * Edit an object in a dialog box.
 */

int cgl_dialog::edit_char(int key)
{
	int edit_obj, oldpos, newpos, curlen, maxlen, pos, ch;
	TEDINFO *tedinfo;
	GRECT clip;
	int result = TRUE;

	if ((edit_obj = edit_object) <= 0)
		return FALSE;

	tedinfo = dial_tree[edit_obj].ob_spec.tedinfo;

	oldpos = newpos = cursor_x;
	curlen = (int) strlen(tedinfo->te_ptext);
	maxlen = (int) strlen(tedinfo->te_pvalid);

	objc_offset(dial_tree, edit_obj, &clip.g_x, &clip.g_y);
	clip.g_h = cgl_regular_font.fnt_chh;

	if (!(key & (CGL_SCANCODE | CGL_CTRL | CGL_ALT)))
		key &= 0xFF;

	switch (key)
	{
	case SHFT_CURLEFT:
		newpos = 0;
		goto setcursor;
	case SHFT_CURRIGHT:
		newpos = curlen;
		goto setcursor;
	case CURLEFT:
		if (oldpos > 0)
			newpos = oldpos - 1;
		goto setcursor;
	case CURRIGHT:
		if (oldpos < curlen)
			newpos = oldpos + 1;

	  setcursor:
		if (oldpos != newpos)
		{
			cgl_update(BEG_UPDATE);
			cgl_mouse_off();

			cursor_off();
			cursor_x = newpos;
			cursor_on();

			cgl_mouse_on();
			cgl_update(END_UPDATE);
		}
		break;
	case BACKSPC:
	case DELETE:
	case ESCAPE:
		if (((key == BACKSPC) && (oldpos > 0)) ||
			((key == DELETE) && (oldpos < curlen)) ||
			((key == ESCAPE) && (*tedinfo->te_ptext != 0)))
		{
			int s, e;

			cgl_update(BEG_UPDATE);
			cgl_mouse_off();
			cursor_off();

			if (key == ESCAPE)
			{
				*tedinfo->te_ptext = 0;
				cursor_x = 0;
			}
			else
			{
				if (key == BACKSPC)
					cursor_x--;
				str_delete(tedinfo->te_ptext, cursor_x);
			}

			s = cgld_abs_curx(dial_tree, edit_obj, cursor_x);
			e = cgld_abs_curx(dial_tree, edit_obj, curlen);

			clip.g_x += s * cgl_regular_font.fnt_chw;
			clip.g_w = (e - s) * cgl_regular_font.fnt_chw;

			doredraw(1, &clip, edit_obj, 1);

			cursor_on();
			cgl_mouse_on();
			cgl_update(END_UPDATE);
		}
		break;
	default:
		pos = oldpos - ((oldpos == maxlen) ? 1 : 0);

		if (((ch = cgld_chk_key(tedinfo->te_pvalid, pos, key)) != 0) ||
				((pos = cgld_chk_skip(dial_tree, edit_obj, key)) > 0))
		{
			int s, e;

			cgl_update(BEG_UPDATE);
			cgl_mouse_off();
			cursor_off();

			if (ch != 0)
			{
				cursor_x = pos + 1;
				str_insert(tedinfo->te_ptext, pos, ch, curlen, maxlen);

				s = cgld_abs_curx(dial_tree, edit_obj, pos);
				e = cgld_abs_curx(dial_tree, edit_obj, min(curlen + 1, maxlen));
			}
			else
			{
				int i;
				char *str = tedinfo->te_ptext;

				cursor_x = pos;

				for (i = oldpos; i < pos; i++)
					str[i] = ' ';
				str[pos] = 0;

				s = cgld_abs_curx(dial_tree, edit_obj, oldpos);
				e = cgld_abs_curx(dial_tree, edit_obj, pos + 1);
			}

			clip.g_x += s * cgl_regular_font.fnt_chw;
			clip.g_w = (e - s) * cgl_regular_font.fnt_chw;

			doredraw(1, &clip, edit_obj, 1);

			cursor_on();
			cgl_mouse_on();
			cgl_update(END_UPDATE);
		}
		else
			result = FALSE;
		break;
	}

	return result;
}

/*
 * void cgl_dialog::edit_end(void)
 *
 * Stop editing an object in a dialog box.
 */

void cgl_dialog::edit_end(void)
{
	if (edit_object > 0)
	{
		cursor_off();
		edit_object = -1;
		cursor_x = 0;
	}
}

/*
 * int cgl_dialog::form_button(int object, int clicks, int *result)
 *
 * Form button function for cgl_dialog class.
 */

int cgl_dialog::form_button(int object, int clicks, int *result)
{
	int flags = dial_tree[object].ob_flags, parent, oldstate, dummy;

	if (cgld_selectable(dial_tree, object) &&
		((flags & SELECTABLE) || (flags & TOUCHEXIT)))
	{
		oldstate = dial_tree[object].ob_state;

		cgl_update(BEG_MCTRL);

		if (flags & RBUTTON)
		{
			if (((parent = cgl_obj_parent(dial_tree, object)) >= 0) && !(oldstate & SELECTED))
				handle_rbutton(parent, object);
		}
		else if (flags & SELECTABLE)
		{
			cglevent events;
			int evflags, newstate, state;

			/* I_A changed to fit tristate-buttons! */
			if (!cgl_is_tristate(dial_tree + object))
				newstate = (oldstate & SELECTED) ? oldstate & ~SELECTED : oldstate | SELECTED;
			else
			{
				/* switch tri-state button! */
				newstate = cgl_get_tristate(oldstate);
				switch (newstate)
				{
				case TRISTATE_0:
					newstate = cgl_set_tristate(oldstate, TRISTATE_1);
					break;
				case TRISTATE_1:
					newstate = cgl_set_tristate(oldstate, TRISTATE_2);
					break;
				case TRISTATE_2:
					newstate = cgl_set_tristate(oldstate, TRISTATE_0);
					break;
				}
			}

			events.ev_mflags = MU_BUTTON | MU_TIMER;
			events.ev_mbclicks = 1;
			events.ev_mbmask = 1;
			events.ev_mbstate = 0;
			events.ev_mm1flags = 1;
			events.ev_mm2flags = 0;
			events.ev_mtcount = 0;
			cgl_objrect(dial_tree, object, &events.ev_mm1);

			change_object(object, newstate);

			cgl_local_button_event = TRUE;

			do
			{
				evflags = cgl_xmulti(&events);

				if (evflags & MU_M1)
				{
					if (events.ev_mm1flags == 1)
					{
						events.ev_mm1flags = 0;
						state = oldstate;
					}
					else
					{
						events.ev_mm1flags = 1;
						state = newstate;
					}
					change_object(object, state);
				}

				events.ev_mflags = MU_BUTTON | MU_M1;
				events.ev_mtcount = 0;
			}
			while (!(evflags & MU_BUTTON));

			cgl_local_button_event = FALSE;
		}

		cgl_update(END_MCTRL);

		if (flags & TOUCHEXIT)
		{
			*result = object | ((clicks > 1) ? 0x8000 : 0);
			return FALSE;
		}

		if ((flags & EXIT) && (dial_tree[object].ob_state != oldstate))
		{
			*result = object;
			return FALSE;
		}

		evnt_button(1, 1, 0, &dummy, &dummy, &dummy, &dummy);
	}

	*result = (flags & EDITABLE) ? object : 0;

	return TRUE;
}

/*
 * int cgl_dialog::form_keybd(int kobnext, int kchar, int *knxtobject,
 *														int *knxtchar)
 *
 * Form keyboard function for cgl_dialog class.
 */

int cgl_dialog::form_keybd(int kobnext, int kchar, int *knxtobject,
													 int *knxtchar)
{
	int i, mode = FMD_FORWARD;

	*knxtobject = kobnext;
	*knxtchar = 0;

	if (!(kchar & (CGL_SCANCODE | CGL_CTRL | CGL_ALT)))
		kchar &= 0xFF;

	switch (kchar)
	{
	case CURUP:
		mode = FMD_BACKWARD;
	case CURDOWN:
	case TAB:
		if ((i = cgld_find_obj(dial_tree, edit_object, mode)) > 0)
		{
			*knxtobject = i;
			return TRUE;
		}
		break;
	case RETURN:
		if ((i = cgld_find_obj(dial_tree, 0, FMD_DEFLT)) > 0)
		{
			change_object(i, SELECTED);
			*knxtobject = i;
			return FALSE;
		}
		break;
	default:
		*knxtchar = kchar;
		return TRUE;
	}

	return TRUE;
}

/*
 * void cgl_dialog::save(void)
 *
 * Save the screen under a dialog box.
 */

void cgl_dialog::save(void)
{
	MFDB source;
	int pxy[8];

	source.fd_addr = NULL;

	cgl_rect2pxy(&dial_size, pxy);

	pxy[4] = 0;
	pxy[5] = 0;
	pxy[6] = dial_size.g_w - 1;
	pxy[7] = dial_size.g_h - 1;

	cgl_mouse_off();
	vro_cpyfm(cgl_vdi_handle, 3, pxy, &source, &dial_mfdb);
	cgl_mouse_on();
}

/*
 * void cgl_dialog::restore(void)
 *
 * Restore the screen under a dialog box.
 */

void cgl_dialog::restore(void)
{
	MFDB dest;
	int pxy[8];

	dest.fd_addr = NULL;

	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = dial_size.g_w - 1;
	pxy[3] = dial_size.g_h - 1;

	cgl_rect2pxy(&dial_size, &pxy[4]);

	cgl_mouse_off();
	vro_cpyfm(cgl_vdi_handle, 3, pxy, &dial_mfdb, &dest);
	cgl_mouse_on();
}

/*
 * void cgl_dialog::limit_to_desk(GRECT *r)
 *
 * Make sure a dialog box is entirely on the screen.
 */

void cgl_dialog::limit_to_desk(GRECT *r)
{
	if (dial_size.g_x < r->g_x)
		dial_size.g_x = r->g_x;
	if (dial_size.g_y < r->g_y)
		dial_size.g_y = r->g_y;
	if ((dial_size.g_x + dial_size.g_w) > (r->g_x + r->g_w))
		dial_size.g_x = r->g_x + r->g_w - dial_size.g_w;
	if ((dial_size.g_y + dial_size.g_h) > (r->g_y + r->g_h))
		dial_size.g_y = r->g_y + r->g_h - dial_size.g_h;
}

/*
 * void cgl_dialog::calcpos(cgl_dialog *prev, int pmode)
 *
 * Calculate the position of a dialog box using the current
 * position mode and the size of the dialog box.
 */

void cgl_dialog::calcpos(cgl_dialog *prev, cgl_dial_pos_mode pmode)
{
	int dummy, xl, xr, yu, yl;

	if ((pmode == CGLD_CENTERED) && (prev == NULL))
		form_center(dial_tree, &dial_size.g_x, &dial_size.g_y, &dial_size.g_w,
								&dial_size.g_h);
	else
	{
		calc_border(dial_tree, &xl, &xr, &yu, &yl);

		dial_size.g_w = dial_tree->ob_width + xl + xr;
		dial_size.g_h = dial_tree->ob_height + yu + yl;

		switch (pmode)
		{
		case CGLD_CENTERED:
			dial_size.g_x = prev->dial_size.g_x + (prev->dial_size.g_w - dial_size.g_w) / 2;
			dial_size.g_y = prev->dial_size.g_y + (prev->dial_size.g_h - dial_size.g_h) / 2;
			break;

		case CGLD_MOUSE:
			graf_mkstate(&dial_size.g_x, &dial_size.g_y, &dummy, &dummy);
			dial_size.g_x -= dial_size.g_w / 2;
			dial_size.g_y -= dial_size.g_h / 2;
			break;

		case CGLD_CURRPOS:
			dial_size.g_x = dial_tree->ob_x - xl;
			dial_size.g_y = dial_tree->ob_y - yu;
			break;
		}

		limit_to_desk(&cgl_desk);

		dial_tree->ob_x = dial_size.g_x + xl;
		dial_tree->ob_y = dial_size.g_y + yu;
	}
}

/********************************************************************
 *																																	*
 * Public member functions of the 'cgl_dialog' class.								*
 *																																	*
 ********************************************************************/

/*
 * void cgl_dialog::draw_object(int start, int depth)
 *
 * Draw objects in a dialog box. If the dialog box is not visible
 * the function will not draw the objects.
 */

void cgl_dialog::draw_object(int start, int depth)
{
	if (!dl_flags.visible)
		return;

	cgl_update(BEG_UPDATE);

	cursor_off();
	doredraw(1, &dial_size, start, depth);
	cursor_on();

	cgl_update(END_UPDATE);
}

/*
 * void cgl_dialog::change_object(int object, int newstate)
 *
 * Change the state of an object in a dialog box. If the dialog box
 * is visible the object will be redrawn.
 */

void cgl_dialog::change_object(int object, int newstate)
{
	if (dial_mode != CGLD_WINDOW)
		objc_change(dial_tree, object, 0, dial_size.g_x, dial_size.g_y,
					dial_size.g_w, dial_size.g_h, newstate, (int) dl_flags.visible);
	else
	{
		dial_tree[object].ob_state = newstate;

		if (dl_flags.visible)
			draw_object(object, MAX_DEPTH);
	}
}

/*
 * void cgl_dialog::add_object(cgl_object *object)
 *
 * Add an object to the dialog box.
 */

void cgl_dialog::add_object(cgl_object *object)
{
	object->dialog = this;
	object->next = objects;
	objects = object;
}

/********************************************************************
 *																																	*
 * Event handlers for the cgl_dialog class.													*
 *																																	*
 ********************************************************************/

/*
 * void cgl_dialog::handle_button(int object)
 *
 * This function is called by the library when the user has
 * selected an object with an set EXIT or TOUCHEXIT flag.
 */

void cgl_dialog::handle_button(int object)
{
	change_object(object & 0x7FFF, dial_tree[object & 0x7FFF].ob_state & ~SELECTED);
}

/********************************************************************
 *																																	*
 * Functions for modal dialog boxes.																*
 *																																	*
 ********************************************************************/

#define CGLD_NMWDFLAGS	(NAME | CLOSER | MOVER)

/*
 * CGL_ERROR cgl_dialog::nmopen(int start, int x, int y,
 *															const char *title)
 * CGL_ERROR cgl_dialog::nmopen(int start, int x, int y,
 *															GRECT *xywh, int zoom, const char *title)
 *
 * Create a non-modal dailogbox.
 *
 * Parameters:
 *
 * start	- First edit object (as in form_do)
 * x			- x position where dialogbox should appear. If -1 the
 *					library will calculate the position itself.
 * y			- y position where dialogbox should appear. If -1 the
 *					library will calculate the position itself.
 * xywh		- Optional pointer to a GRECT structure. If this pointer
 *					is not NULL and zoom is not 0, the library will draw
 *					a zoombox from the rectangle in xywh to the window.
 * zoom		- see xywh
 * title	- Optional title. If not NULL this string is used as the
 *					title of the window. Otherwise the program name is used.
 *
 * Result: see cgl_appl.h.
 */

CGL_ERROR cgl_dialog::nmopen(int start, int x, int y, const char *title)
{
	return nmopen(start, x, y, NULL, 0, title);
}

CGL_ERROR cgl_dialog::nmopen(int start, int x, int y, GRECT *xywh, int zoom,
														 const char *title)
{
	CGL_ERROR error;
	GRECT wsize;

	if (win_type != CW_NMDIALOG)
		return CGL_DIALTYPE;								/* Dialog is not a non modal dialog. */

	cgl_update(BEG_UPDATE);

	dial_mode = CGLD_WINDOW;
	edit_object = -1;
	cursor_x = 0;
	curs_cnt = 1;

	if (dmb >= 0)
		dial_tree[dmb].ob_flags |= HIDETREE;

	calcpos(NULL, cgld_posmode);
	cw_calc(WC_BORDER, CGLD_NMWDFLAGS, &dial_size, &wsize);

	if ((x != -1) && (y != -1))
	{
		int dx, dy;

		dx = x - wsize.g_x;

		dial_size.g_x += dx;
		dial_tree->ob_x += dx;
		wsize.g_x = x;

		dy = y - wsize.g_y;

		dial_size.g_y += dy;
		dial_tree->ob_y += dy;
		wsize.g_y = y;
	}

	if (wsize.g_x < cgl_desk.g_x)
	{
		int d = cgl_desk.g_x - wsize.g_x;

		dial_size.g_x += d;
		dial_tree->ob_x += d;
		wsize.g_x = cgl_desk.g_x;
	}

	if (wsize.g_y < cgl_desk.g_y)
	{
		int d = cgl_desk.g_y - wsize.g_y;

		dial_size.g_y += d;
		dial_tree->ob_y += d;
		wsize.g_y = cgl_desk.g_y;
	}

	cw_set(WF_NAME, (title == NULL) ? cgl_app->prg_name : title);

	if ((error = cw_create(CGLD_NMWDFLAGS, &wsize)) == CGL_OK)
	{
		dl_flags.visible = TRUE;

		start = (start == 0) ? cgld_find_obj(dial_tree, 0, FMD_FORWARD) : start;
		edit_init(start, -1);

		if (zoom && xywh)
			graf_growbox(xywh->g_x, xywh->g_y, xywh->g_w, xywh->g_h,
									 wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h);
	}

	cgl_update(END_UPDATE);

	return error;
}

/*
 * void cgl_dialog::nmclose(void)
 * void cgl_dialog::nmclose(GRECT *xywh, int zoom)
 *
 * Close a non-modal dialog.
 */

void cgl_dialog::nmclose(void)
{
	nmclose(NULL, 0);
}

void cgl_dialog::nmclose(GRECT *xywh, int zoom)

{
	if (win_type != CW_NMDIALOG)
		return;															/* Wrong dialog type. */

	cgl_update(BEG_UPDATE);

	cw_destroy();

	dl_flags.visible = FALSE;

	if (zoom && xywh)
		graf_shrinkbox(xywh->g_x, xywh->g_y, xywh->g_w, xywh->g_h,
									 dial_size.g_x, dial_size.g_y, dial_size.g_w,
									 dial_size.g_h);

	cgl_update(END_UPDATE);
}

/********************************************************************
 *																																	*
 * Functions for modal dialog boxes.																*
 *																																	*
 ********************************************************************/

#define CGLD_MWDFLAGS	(NAME | MOVER)

/*
 * void cgl_dialog::mopen(void)
 * void cgl_dialog::mopen(GRECT *xywh, int zoom)
 *
 * Open a modal dialog box.
 *
 * Parameters:
 *
 * tree		- object tree,
 */

void cgl_dialog::mopen(void)
{
	mopen(NULL, FALSE);
}

void cgl_dialog::mopen(GRECT *xywh, int zoom)
{
	cgl_dial_mode dialmode = cgld_dialmode;

	edit_object = -1;
	cursor_x = 0;
	curs_cnt = 1;

	cgl_update(BEG_UPDATE);

	calcpos(cgl_dialogs, cgld_posmode);

	if (dialmode == CGLD_WINDOW)
	{
		if ((cgl_dialogs == NULL) || (cgl_dialogs->dial_mode == CGLD_WINDOW))
		{
			GRECT wsize;
			int error;

			cw_calc(WC_BORDER, CGLD_MWDFLAGS, &dial_size, &wsize);

			if (wsize.g_x < cgl_desk.g_x)
			{
				int d = cgl_desk.g_x - wsize.g_x;

				dial_size.g_x += d;
				dial_tree->ob_x += d;
				wsize.g_x = cgl_desk.g_x;
			}

			if (wsize.g_y < cgl_desk.g_y)
			{
				int d = cgl_desk.g_y - wsize.g_y;

				dial_size.g_y += d;
				dial_tree->ob_y += d;
				wsize.g_y = cgl_desk.g_y;
			}

			if (zoom && xywh)
			{
				graf_growbox(xywh->g_x, xywh->g_y, xywh->g_w, xywh->g_h,
										 wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h);
				zoom = FALSE;
			}

			cw_set(WF_NAME, cgl_app->prg_name);

			if ((error = cw_create(CGLD_MWDFLAGS, &wsize)) == CGL_OK)
			{
				if (cgl_dialogs == NULL)
					cgl_app->disable_menu();

				cgl_update(END_UPDATE);
			}
			else
				dialmode = CGLD_BUFFERED;
		}
		else
			dialmode = CGLD_BUFFERED;
	}

	if (zoom && xywh)
	{
		graf_growbox(xywh->g_x, xywh->g_y, xywh->g_w, xywh->g_h,
								 dial_size.g_x, dial_size.g_y, dial_size.g_w,
								 dial_size.g_h);
		zoom = FALSE;
	}

	if (dialmode == CGLD_BUFFERED)
	{
		long scr_size;

		scr_size = cgl_initmfdb(&dial_size, &dial_mfdb);

		if ((dial_mfdb.fd_addr = malloc(scr_size)) == NULL)
			dialmode = CGLD_NORMAL;
	}

	dial_mode = dialmode;

	if (dmb >= 0)
	{
		if (dialmode != CGLD_BUFFERED)
			dial_tree[dmb].ob_flags |= HIDETREE;
		else
			dial_tree[dmb].ob_flags &= ~HIDETREE;
	}

	if (dialmode != CGLD_WINDOW)
	{
		if (dialmode == CGLD_NORMAL)
		{
			if (cgl_dialogs != NULL)
				calcpos(cgl_dialogs, CGLD_CENTERED);

			if ((cgl_dialogs == NULL) || (cgl_dialogs->dial_mode == CGLD_WINDOW))
				form_dial(FMD_START, 0, 0, 0, 0, dial_size.g_x, dial_size.g_y,
									dial_size.g_w, dial_size.g_h);
		}
		else
			save();

		draw_object(ROOT, MAX_DEPTH);
	}
	else
	{
		cglevent events;
		int flags;

		events.ev_mflags = MU_TIMER | MU_MESAG;
		events.ev_mbclicks = 0;
		events.ev_mbmask = 0;
		events.ev_mbstate = 0;
		events.ev_mm1flags = 0;
		events.ev_mm2flags = 0;
		events.ev_mtcount = 0;

		while ((flags = cgl_xmulti(&events)) != MU_TIMER);
	}

	prev = cgl_dialogs;
	cgl_dialogs = this;
	dl_flags.visible = TRUE;
}

int cgl_dialog::execute(int start)
{
	int event;
	cglevent events;

	if (win_type != CW_DIALOG)
		return 0;														/* Not a modal dialog box. */

	dl_flags.cont = TRUE;									/* Reset continue flag. */
	exit_obj = 0;													/* Reset exit object. */

	start = (start == 0) ? cgld_find_obj(dial_tree, 0, FMD_FORWARD) : start;

	edit_init(start, -1);

	if (dial_mode != CGLD_WINDOW)
	{
		events.ev_mflags = MU_KEYBD | MU_BUTTON;
		cgl_update(BEG_MCTRL);
	}
	else
		events.ev_mflags = MU_KEYBD | MU_BUTTON | MU_MESAG;

	events.ev_mbclicks = 2;
	events.ev_mbmask = 1;
	events.ev_mbstate = 1;
	events.ev_mm1flags = 0;
	events.ev_mm2flags = 0;

	events.ev_mtcount = 0;

	while (!cgl_app->quit_appl && dl_flags.cont)
		event = cgl_xmulti(&events);

	if (dial_mode != CGLD_WINDOW)
		cgl_update(END_MCTRL);

	edit_end();

	return exit_obj;
}

/*
 * void mclose(void)
 * void mclose(GRECT *xywh, int zoom)
 *
 * Close a modal dialog box.
 */

void cgl_dialog::mclose(void)
{
	mclose(NULL, FALSE);
}

void cgl_dialog::mclose(GRECT *xywh, int zoom)
{
	if (win_type != CW_DIALOG)
		return;															/* Wrong dialog type. */

	if (dl_flags.visible)
	{
		if (cgl_dialogs != this)
			return;
		cgl_dialogs = prev;

		switch(dial_mode)
		{
		case CGLD_WINDOW :
			cgl_update(BEG_UPDATE);

			if (prev == NULL)
				cgl_app->enable_menu();

			cw_destroy();

			if (prev != NULL)
				prev->cw_set(WF_TOP);
			break;

		case CGLD_BUFFERED :
			restore();
			free(dial_mfdb.fd_addr);
			break;

		case CGLD_NORMAL :
			if ((prev != NULL) && (prev->dial_mode != CGLD_WINDOW))
				prev->draw_object(ROOT, MAX_DEPTH);
			else
				form_dial(FMD_FINISH, 0, 0, 0, 0, dial_size.g_x,
									dial_size.g_y, dial_size.g_w, dial_size.g_h);
			break;
		}

		if (zoom && xywh)
		{
			graf_shrinkbox(xywh->g_x, xywh->g_y, xywh->g_w, xywh->g_h,
										 dial_size.g_x, dial_size.g_y, dial_size.g_w,
										 dial_size.g_h);
		}

		cgl_update(END_UPDATE);
	}

	dl_flags.visible = FALSE;
}

/********************************************************************
 *																																	*
 * Implementation of the 'cgl_object' class.												*
 *																																	*
 ********************************************************************/

cgl_object::cgl_object(void)
{
	next = NULL;
	dialog = NULL;
}

cgl_object::~cgl_object(void)
{
}

int cgl_object::handle_select(int object, int x, int y)
{
	return FALSE;
}

/********************************************************************
 *																																	*
 * Configuration functions.																					*
 *																																	*
 ********************************************************************/

/*
 * cgl_dial_pos_mode cgld_setposmode(cgl_dial_pos_mode new)
 *
 * Set the position where a dialog box will appear. See also
 * 'cgl_dialog::calcpos'.
 */

cgl_dial_pos_mode cgld_setposmode(cgl_dial_pos_mode newmode)
{
	cgl_dial_pos_mode old;

	old = cgld_posmode;

	if (/*(newmode >= CGLD_CENTERED) &&*/ (newmode <= CGLD_CURRPOS))
		cgld_posmode = newmode;

	return old;
}

/*
 * cgl_dial_mode xd_setdialmode(cgl_dial_mode new)
 *
 * Set the dialog mode.
 */

cgl_dial_mode cgld_setdialmode(cgl_dial_mode newmode)
{
	cgl_dial_mode old;

	old = cgld_dialmode;

	if (/*(newmode >= CGLD_NORMAL) && */(newmode <= CGLD_WINDOW))
		cgld_dialmode = newmode;

	return old;
}

/*
 * int cgld_do_dialog(OBJECT *tree, int start)
 *
 * Simple interface to the 'cgl_dialog' class.
 */

int cgld_do_dialog(OBJECT *tree, int start)
{
	int exit;
	cgl_dialog dialog(tree, CW_DIALOG);

	dialog.mopen();
	exit = dialog.execute(start);
	dialog.mclose();

	return exit;
}

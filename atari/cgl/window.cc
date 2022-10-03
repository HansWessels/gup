
#include <osbind.h>
#include <stdlib.h>

#include "cgl_dial.h"
#include "cgl_evnt.h"
#include "cgl_util.h"
#include "cgl_wind.h"
#include "internal.h"

static cwindow *windows = NULL;
static cwindow *desktop = NULL;

static int all_iconified = FALSE;

/********************************************************************
 *																																	*
 * Utility functions for the menu bar of a window.									*
 *																																	*
 ********************************************************************/

/*
 * static int cw_tree_size(OBJECT *menu)
 *
 * Calculate the number of objects in a menu tree.
 *
 * Parameters:
 *
 * menu	- pointer to the menu tree
 *
 * Result: number of objects.
 */

static int cw_tree_size(OBJECT *menu)
{
	register int i = 0;

	if (menu == NULL)
		return 0;
	else
	{
		while ((menu[i].ob_flags & LASTOB) == 0)
			i++;
		i++;

		return i;
	}
}

/*
 * static void cw_find_objects(OBJECT *menu, int &bar, int &boxes)
 *
 * Find the object numbers of the object containing all menu titles
 * and the object containing all menu boxes.
 *
 * Parameters:
 *
 * menu		- pointer to menu tree
 * bar		- object index of the object containing all titles
 * boxes	- object index of the object containing all boxes
 */

static void cw_find_objects(OBJECT *menu, int &bar, int &boxes)
{
	bar = menu->ob_head;
	boxes = menu->ob_tail;
}

/*
 * static void cw_menu_change(OBJECT *menu, int item, int select, GRECT *box)
 *
 * Select or deselect a menu item in a pull down menu.
 *
 * Parameters:
 *
 * menu		- menu tree,
 * item 	- menu item,
 * select	- 1 = select, 0 = deselect,
 * box		- clipping rectangle.
 */

static void cw_menu_change(OBJECT *menu, int item, int select, GRECT *box)
{
	register int newstate = menu[item].ob_state;

	newstate = (select == TRUE) ? newstate | SELECTED : newstate & ~SELECTED;
	objc_change(menu, item, 0, box->g_x, box->g_y, box->g_w, box->g_h, newstate, 1);
}

/********************************************************************
 *																																	*
 * Member functions of the 'cwindow' class.													*
 *																																	*
 ********************************************************************/

/*
 * Constructor for the 'cwindow' class.
 */

cwindow::cwindow(int type)
{
	/*
	 * Initiate variabels of the class.
	 */

	win_type = type;
	win_menu = NULL;
	win_state = CW_INIT;

	win_title = NULL;
	win_info = NULL;
	win_hsl_size = 1000;
	win_hsl_pos = 1;
	win_vsl_size = 1000;
	win_vsl_pos = 1;

	cw_flags.iconified = 0;
	cw_flags.tmp_closed = 0;

	/*
	 * Add the window to the window list.
	 */

	if (windows != NULL)
		windows->prev_win = this;
	prev_win = NULL;
	next_win = windows;
	windows = this;
}

/*
 * Destructor for 'cwindow' class.
 */

cwindow::~cwindow(void)
{
	/*
	 * Close the window if it is still open.
	 */

	if (win_state == CW_CREATED)
		cw_destroy();

	/*
	 * Remove the window from the window list.
	 */

	if (prev_win == NULL)
		windows = next_win;
	else
		prev_win->next_win = next_win;

	if (next_win != NULL)
		next_win->prev_win = prev_win;

	/*
	 * Free allocated space of the menu bar.
	 */

	if (win_menu != NULL)
		delete win_menu;
}

/*
 * CGL_ERROR cwindow:cw_create(int flags, GRECT *msize, GRECT *size)
 *
 * Create the window.
 *
 * Parameters:
 *
 * flags	- window flags
 * msize	- maximum size of window
 * size		- initial size of window
 *
 * Result:	CGL_OK				- No error 
 *					CGL_NMWINDOWS	- No more windows
 */

CGL_ERROR cwindow::cw_create(int flags, GRECT *size)
{
	if (win_state != CW_INIT)
		return CGL_OK;

	win_flags = flags | SMALLER;
	win_size = *size;

	return cw_do_open();
}

/*
 * void cwindow::cw_destroy(void)
 *
 * Destroy the window.
 */

void cwindow::cw_destroy(void)
{
	register cwindow *old_tw, *new_tw;

	if (win_state == CW_CREATED)
	{
		old_tw = cw_get_top();

		wind_close(win_handle);
		wind_delete(win_handle);
		win_state = CW_INIT;

		cw_flags.iconified = 0;							/* Reset iconified flag. */
		cw_flags.tmp_closed = 0;						/* Reset temporary closed flag. */

		new_tw = cw_get_top();

		if (old_tw != new_tw)
		{
			if (new_tw != NULL)
				new_tw->cw_top();
			else if (desktop != NULL)
				desktop->cw_top();
			else
				cgl_app->all_windows_closed();
		}
	}
}

/*
 * void cwindow::cw_set(int field, int w1, int w2, int w3, int w4)
 * void cwindow::cw_set(int field, int w)
 * void cwindow::cw_set(int field, GRECT *w)
 *
 * Change parameters of a window. If this function is called when
 * the window has not yet been opened with 'cw_create', only the
 * window title, info string and the slider positions and sizes
 * can be set.
 */

void cwindow::cw_set(int field, int w1, int w2, int w3, int w4)
{
	switch (field)
	{
	case WF_NAME:
		win_title = (const char *) w1;
		break;

	case WF_INFO:
		win_info = (const char *) w1;
		break;

	case WF_CURRXYWH:
	case WF_ICONIFY:
	case WF_UNICONIFY:
		win_size.g_x = w1;
		win_size.g_y = w2;
		win_size.g_w = w3;
		win_size.g_h = w4;

		if (win_state != CW_INIT)
		{
			wind_set(win_handle, field, win_size.g_x,
							 win_size.g_y, win_size.g_w, win_size.g_h);
			wind_get(win_handle, WF_WORKXYWH, &win_work.g_x,
							 &win_work.g_y, &win_work.g_w, &win_work.g_h);
			cw_set_barpos();
		}
		return;

	case WF_HSLIDE:
		win_hsl_pos = w1;
		break;

	case WF_VSLIDE:
		win_vsl_pos = w1;
		break;

	case WF_TOP:
		cw_set_top();
		return;

	case WF_HSLSIZE:
		win_hsl_size = w1;
		break;

	case WF_VSLSIZE:
		win_vsl_size = w1;
		break;
	}

	if (win_state != CW_INIT)
		wind_set(win_handle, field, w1, w2, w3, w4);
}

/*
 * void cwindow::cw_get(int field, int *w1, int *w2, int *w3, int *w4)
 * void cwindow::cw_get(int field, int *w)
 * void cwindow::cw_get(int field, GRECT *w)
 *
 * Get window parameters. If this function is called when the window
 * has not yet been opened with 'cw_create', the results will be
 * undefined.
 */

void cwindow::cw_get(int field, int *w1, int *w2, int *w3, int *w4)
{
	if (win_state != CW_INIT)
	{
		switch (field)
		{
		case WF_WORKXYWH:
			*w1 = win_work.g_x;
			*w3 = win_work.g_w;

			if ((win_menu != NULL) && !cw_flags.iconified)
			{
				int height;

				height = win_menu[win_bar].ob_height + 1;

				*w2 = win_work.g_y + height;
				*w4 = win_work.g_h - height;
			}
			else
			{
				*w2 = win_work.g_y;
				*w4 = win_work.g_h;
			}
			break;

		case WF_CURRXYWH:
			*w1 = win_size.g_x;
			*w2 = win_size.g_y;
			*w3 = win_size.g_w;
			*w4 = win_size.g_h;
			break;

		default:
			wind_get(win_handle, field, w1, w2, w3, w4);
			break;
		}
	}
}

/*
 * void cwindow::cw_redraw(int mode, GRECT *area, ...)
 *
 * Redraw a part of the window. This function walks through the
 * rectangle list and redraws the menu bar and calls the windows
 * draw function to redraw the user area. 
 *
 * Parameters:
 *
 * mode	- passed to the draw function of the window. If mode is 0
 *				the window should just be redrawn. Other values can be
 *				used by the user for specifing special redraw actions,
 *				for example show a cursor or remove a cursor.
 * area	- area to redraw.
 */

void cwindow::cw_redraw(int mode, GRECT *area, ...)
{
	GRECT menu_rect, user_rect, r, in;
	register va_list args;

	if (win_state == CW_INIT)
		return;

	va_start(args, area);

//	cgl_update(BEG_UPDATE);								/* Claim screen. */
//	cgl_mouse_off();											/* Mouse off. */

	if ((win_menu != NULL) && !cw_flags.iconified)
	{
		/*
		 * A menu bar is present. Get its geometry.
		 */

		cgl_objrect(win_menu, win_bar, &menu_rect);
		menu_rect.g_h += 1;
	}

	cw_get(WF_WORKXYWH, &user_rect);			/* Get user area. */
	cw_get(WF_FIRSTXYWH, &r);							/* Get first rectangle. */

	while ((r.g_w != 0) && (r.g_h != 0))
	{
		if (cgl_clip_desk(&r))
		{
			if (cgl_rcintersect(&r, area, &r))
			{
				if (cw_flags.iconified)					/* Check if window is iconified. */
				{
					int pxy[4];

					cgl_clip_on(&r);
					cgl_rect2pxy(&r, pxy);
					vsf_color(cgl_vdi_handle, 0);
					vsf_interior(cgl_vdi_handle, FIS_SOLID);
					vsf_perimeter(cgl_vdi_handle, 0);
					vr_recfl(cgl_vdi_handle, pxy);
					cgl_clip_off();
				}
				else
				{
					if (win_menu && cgl_rcintersect(&r, &menu_rect, &in))
						cw_draw_menu(win_bar, &in);	/* Redraw menu bar if present. */

					if (cgl_rcintersect(&r, &user_rect, &in))
						cw_draw(mode, &in, args);		/* Redraw user area. */
				}
			}
		}

		cw_get(WF_NEXTXYWH, &r);						/* Get next rectangle. */
	}

//	cgl_mouse_on();												/* Mouse on. */
//	cgl_update(END_UPDATE);								/* Release screen. */

	va_end(args);
}

/*
 * void cwindow::cw_scroll(int mode, GRECT *area, int pixels, int hv, ...)
 *
 * Scroll a part of the window. This function walks through the
 * rectangle list and scrolls every rectangle. Areas which should
 * be redrawn after scrolling, are redrawn by calling the cw_draw
 * member function. 
 *
 * Parameters:
 *
 * mode		- passed to the draw function of the window. If mode is 0
 *					the window should just be redrawn. Other values can be
 *					used by the user for specifing special redraw actions,
 *					for example show a cursor or remove a cursor.
 * area		- area to scroll.
 * pixels	- number of pixels to scroll the area.
 * hv			- scroll horizontal (0) or vertical (1).
 */

void cwindow::cw_scroll(int mode, GRECT *area, int pixels, int hv, ...)
{
	register int wx, wy;
	GRECT work, r, src, dest, in;
	register va_list args;

	if (win_state == CW_INIT)
		return;

	cw_get(WF_WORKXYWH, &work);

	if (!cgl_rcintersect(&work, area, &work))
		return;

	va_start(args, hv);

	wx = work.g_x;
	wy = work.g_y;

	cgl_update(BEG_UPDATE);
	cgl_mouse_off();

	cw_get(WF_FIRSTXYWH, &r);

	while ((r.g_w != 0) && (r.g_h != 0))
	{
		if (cgl_rcintersect(&r, &work, &in) == TRUE)
		{
			GRECT area;

			src = in;
			dest = in;

			/*
			 * Determine the source and destination area's.
			 */

			if (hv == 1)
			{
				/*
				 * Scrolling vertical.
				 */

				if (pixels >= 0)
				{
					dest.g_y += pixels;
					dest.g_h -= pixels;
					src.g_h -= pixels;
				}
				else
				{
					src.g_y -= pixels;
					dest.g_h += pixels;
					src.g_h += pixels;
				}
			}
			else
			{
				/*
				 * Scrolling horizontal.
				 */

				if (pixels >= 0)
				{
					dest.g_x += pixels;
					dest.g_w -= pixels;
					src.g_w -= pixels;
				}
				else
				{
					src.g_x -= pixels;
					dest.g_w += pixels;
					src.g_w += pixels;
				}
			}

			/*
			 * Scroll the screen.
			 */

			if ((src.g_h > 0) && (src.g_w > 0))
			{
				cgl_clip_on(&in);
				cgl_move_screen(&dest, &src);
				cgl_clip_off();
			}

			/*
			 * Calculate the area to redraw.
			 */

			if (pixels >= 0)
			{
				area.g_x = in.g_x;
				area.g_y = in.g_y;
			}
			else
			{
				area.g_x = (hv == 0) ? max(dest.g_x + dest.g_w, in.g_x) : in.g_x;
				area.g_y = (hv == 1) ? max(dest.g_y + dest.g_h, in.g_y) : in.g_y;
			}

			area.g_w = (hv == 0) ? min(abs(pixels), in.g_w) : in.g_w;
			area.g_h = (hv == 1) ? min(abs(pixels), in.g_h) : in.g_h;

			/*
			 * Redraw the area,
			 */

			cw_draw(mode, &area, args);
		}

		cw_get(WF_NEXTXYWH, &r);
	}

	cgl_mouse_on();
	cgl_update(END_UPDATE);

	va_end(args);
}

/*
 * void cwindow::cw_send_redraw(GRECT *area)
 *
 * Send a redraw message to the window.
 *
 * Parameters:
 *
 * area	- area to redraw.
 */

void cwindow::cw_send_redraw(GRECT *area)
{
	int message[8];

	message[0] = WM_REDRAW;
	message[1] = cgl_apid;
	message[2] = 0;
	message[3] = win_handle;
	message[4] = area->g_x;
	message[5] = area->g_y;
	message[6] = area->g_w;
	message[7] = area->g_h;

	appl_write(cgl_apid, 16, message);
}

/*
 * void cwindow::cw_calc(int w_ctype, int w_flags, GRECT *input,
 *											 GRECT *output)
 *
 * Replacement wind_calc() for window. If the window has a menu bar
 * this function takes this into acount.
 *
 * Parameters:
 *
 * w_ctype	- calculation type (WC_BORDER or WC_WORK)
 * w_flags	- flags of the window
 * input		- input rectangle
 * output		- output rectangle
 */

void cwindow::cw_calc(int w_ctype, int w_flags, GRECT *input,
											GRECT *output)
{
	register int height;

	wind_calc(w_ctype, w_flags, input->g_x, input->g_y, input->g_w,
						input->g_h, &output->g_x, &output->g_y, &output->g_w,
						&output->g_h);

	if (win_menu != NULL)
	{
		height = win_menu[win_bar].ob_height + 1;

		if (w_ctype == WC_WORK)
		{
			output->g_y += height;
			output->g_h -= height;
		}
		else
		{
			output->g_y -= height;
			output->g_h += height;
		}
	}
}

/*
 * CGL_ERROR cwindow::cw_do_open(void)
 *
 * Open the window. Called by 'cw_reopen' and 'cw_create'.
 */

CGL_ERROR cwindow::cw_do_open(void)
{
	if (win_state != CW_INIT)
		return CGL_OK;

	if ((win_handle = wind_create(win_flags, cgl_desk.g_x, cgl_desk.g_y,
																cgl_desk.g_w, cgl_desk.g_h)) < 0)
		return CGL_NMWINDOWS;
	else
	{
		win_state = CW_CREATED;

		/*
		 * Set defaults if present.
		 */

		if (cw_flags.iconified)
			wind_set(win_handle, WF_ICONIFY, win_size.g_x, win_size.g_y,
							 win_size.g_w, win_size.g_h);

		if ((win_flags & NAME) && win_title)
			wind_set(win_handle, WF_NAME, (int) win_title, 0, 0, 0);

		if ((win_flags & INFO) && win_info)
			wind_set(win_handle, WF_NAME, (int) win_info, 0, 0, 0);

		if (win_flags & VSLIDE)
		{
			wind_set(win_handle, WF_VSLIDE, win_vsl_pos, 0, 0, 0);
			wind_set(win_handle, WF_VSLSIZE, win_vsl_size, 0, 0, 0);
		}

		if (win_flags & HSLIDE)
		{
			wind_set(win_handle, WF_HSLIDE, win_hsl_pos, 0, 0, 0);
			wind_set(win_handle, WF_HSLSIZE, win_hsl_size, 0, 0, 0);
		}

		/*
		 * Really open the window.
		 */

		wind_open(win_handle, win_size.g_x, win_size.g_y, win_size.g_w,
						  win_size.g_h);

		wind_get(win_handle, WF_WORKXYWH, &win_work.g_x,
						 &win_work.g_y, &win_work.g_w, &win_work.g_h);

		cw_set_barpos();										/* Update the position of the menu bar. */
		cw_top();

		return CGL_OK;
	}
}

/*
 * void cwindow::cw_set_top(void)
 *
 * Make the window the top window.
 */

void cwindow::cw_set_top(void)
{
	if (win_state == CW_CREATED)
	{
		wind_set(win_handle, WF_TOP, win_handle, 0, 0, 0);

		if (this != windows)
		{
			if (next_win != NULL)
				next_win->prev_win = prev_win;
			prev_win->next_win = next_win;
			windows->prev_win = this;
			prev_win = NULL;
			next_win = windows;
			windows = this;

			cw_top();
		}
	}
}

/*
 * void cwindow::cw_iconify(GRECT *r)
 *
 * Iconify the window.
 *
 * Parameters:
 *
 * r	- size and position of the icon.
 */

void cwindow::cw_iconify(GRECT *r)
{
	if (!cw_flags.iconified)
	{
		orig_win_size = win_size;						/* Store current window size. */
		cw_set(WF_ICONIFY, r);							/* Iconify window. */
		cw_flags.iconified = 1;
	}
}

/*
 * void cwindow::cw_uniconify(void)
 *
 * Uniconify the window.
 */

void cwindow::cw_uniconify(void)
{
	if (cw_flags.iconified)
	{
		if (all_iconified)
		{
			cwindow *p = windows;

			/*
			 * Reopen all windows on their original position and in
			 * the order they were in when closing.
			 */

			if (p)
			{
				while(p->next_win)							/* First find the last window. */
					p = p->next_win;

				while (p)
				{
					p->cw_reopen();
					p = p->prev_win;
				}
			}

			/*
			 * Enable the menu bar and reset the 'all_iconified' flag.
			 */

			cgl_app->enable_menu();
			all_iconified = FALSE;
		}

		cw_set(WF_UNICONIFY, &orig_win_size);	/* Uniconify window. */
		cw_flags.iconified = 0;
	}
}

/*
 * void cwindow::cw_tmp_close(void)
 *
 * Temporary close a window. The window can be reopend on it's
 * original position using the 'cw_reopen' function.
 */

void cwindow::cw_tmp_close(void)
{
	wind_close(win_handle);
	wind_delete(win_handle);
	win_state = CW_INIT;
	cw_flags.tmp_closed = 1;
}

/*
 * void cwindow::cw_reopen(void)
 *
 * Reopen a window closed with 'cw_tmp_close'.
 */

void cwindow::cw_reopen(void)
{
	if (cw_flags.tmp_closed)
	{
		if (cw_do_open() == CGL_OK)
			cw_flags.tmp_closed = 0;
	}
}

/********************************************************************
 *																																	*
 * Functions for the menu bar of the window.												*
 *																																	*
 ********************************************************************/

/*
 * int cwindow::cw_menu_bar(OBJECT *menu)
 *
 * Set the menu bar of a window. If menu is NULL, the menu bar will
 * be removed. This function can only be called if the window is
 * not yet created.
 *
 * Parameters:
 *
 * menu	- pointer to menu tree.
 */

int cwindow::cw_menu_bar(OBJECT *menu)
{
	if (win_state != CW_INIT)
		return CGL_NINIT;
	else
	{
		if (win_menu != NULL)
		{
			delete win_menu;
			win_menu = NULL;
		}

		if (menu != NULL)
		{
			win_menu = new OBJECT[cw_tree_size(menu)];
			memcpy(win_menu, menu, cw_tree_size(menu) * sizeof(OBJECT));
			cw_find_objects(menu, win_bar, win_mparent);
		}

		return CGL_OK;
	}
}

/*
 * void cwindow::cw_menu_icheck(int item, int check)
 *
 * Put a check mark in front of a menu item.
 *
 * Parameters:
 *
 * item		- menu item to mark or unmark,
 * check	- 0 unmark, 1 mark.
 */

void cwindow::cw_menu_icheck(int item, int check)
{
	if (win_menu != NULL)
	{
		if (check == 0)
			win_menu[item].ob_state &= ~CHECKED;
		else
			win_menu[item].ob_state |= CHECKED;
	}
}

/*
 * void cwindow::cw_menu_ienable(int item, int enable)
 *
 * Enable or disable a menu item.
 *
 * Parameters:
 *
 * item		- menu item,
 * enable	- 0 disable, 1 enable.
 */

void cwindow::cw_menu_ienable(int item, int enable)
{
	if (win_menu != NULL)
	{
		if (enable == 0)
			win_menu[item].ob_state |= DISABLED;
		else
			win_menu[item].ob_state &= ~DISABLED;
	}
}

/*
 * void cwindow::cw_menu_text(int item, const char *text)
 *
 * Change the text of a menu item.
 *
 * Parameters:
 *
 * item		- menu item,
 * text		- new text for menu item.
 */

void cwindow::cw_menu_text(int item, const char *text)
{
	if (win_menu != NULL)
		win_menu[item].ob_spec.free_string = (char *) text;
}

/*
 * void cwindow::cw_menu_tnormal(int item, int normal)
 *
 * Display a menu item inverse or normal.
 *
 * Parameters:
 *
 * item		- menu item,
 * normal	- 0 inverse, 1 normal.
 */

void cwindow::cw_menu_tnormal(int item, int normal)
{
	if (win_menu != NULL)
	{
		GRECT r;

		if (normal == 0)
			win_menu[item].ob_state |= SELECTED;
		else
			win_menu[item].ob_state &= ~SELECTED;

		cw_bar_rect(&r);
		cw_redraw_menu(item, &r);
	}
}

/*
 * void cwindow::cw_set_barpos(void)
 *
 * If the window has a menu bar, set the position of the menu
 * bar to the current position of the window.
 */

void cwindow::cw_set_barpos(void)
{
	if (win_menu != NULL)
	{
		win_menu->ob_x = win_work.g_x;
		win_menu->ob_y = win_work.g_y;
		win_menu[win_bar].ob_width = win_work.g_w;
	}
}

/*
 * void cwindow::cw_bar_rect(WINDOW *w, GRECT *r)
 *
 * Get the rectangle enclosing the menu bar of the window.
 *
 * Parameters:
 *
 * r	- resulting rectangle.
 */

void cwindow::cw_bar_rect(GRECT *r)
{
	cgl_objrect(win_menu, win_bar, r);
	r->g_h += 1;
}

/*
 * void cwindow::cw_draw_menu(int object, GRECT *r)
 *
 * Draw the menu bar of a window.
 *
 * Parameters:
 *
 * object	- start object to redraw
 * r			- area to redraw
 */

void cwindow::cw_draw_menu(int object, GRECT *r)
{
	GRECT r1, in;
	int pxy[4];

	cgl_objrect(win_menu, object, &r1);
	if (object == win_bar)
		r1.g_h += 1;

	if (cgl_rcintersect(r, &r1, &in) == TRUE)
	{
		/*
		 * Start and end coordinates of the line below the menu bar.
		 */

		pxy[0] = win_work.g_x;
		pxy[1] = pxy[3] = win_work.g_y + r1.g_h - 1;
		pxy[2] = win_work.g_x + win_work.g_w - 1;

		/*
		 * Redraw the menu bar and the line below the menu bar.
		 */

		objc_draw(win_menu, win_bar, MAX_DEPTH, in.g_x, in.g_y, in.g_w, in.g_h);

		vswr_mode(cgl_vdi_handle, MD_REPLACE);

		vsl_color(cgl_vdi_handle, 1);
		vsl_ends(cgl_vdi_handle, 0, 0);
		vsl_type(cgl_vdi_handle, 1);
		vsl_width(cgl_vdi_handle, 1);

		cgl_clip_on(&in);
		v_pline(cgl_vdi_handle, 2, pxy);
		cgl_clip_off();
	}
}

/*
 * void cwindow::cw_redraw_menu(int object, GRECT *r)
 *
 * Redraw the menu bar of a window.
 *
 * Parameters:
 *
 * object	- start object to redraw
 * r			- area to redraw
 */

void cwindow::cw_redraw_menu(int object, GRECT *r)
{
	GRECT r1, r2;

	if ((win_menu != NULL) && !cw_flags.iconified)
	{
		cgl_objrect(win_menu, object, &r1);
		if (object == win_bar)
			r1.g_h += 1;

		if (cgl_rcintersect(r, &r1, &r1) == TRUE)
		{
			cgl_update(BEG_UPDATE);
			cgl_mouse_off();

			cw_get(WF_FIRSTXYWH, &r2);

			while ((r2.g_w != 0) && (r2.g_h != 0))
			{
				cw_draw_menu(object, &r2);
				cw_get(WF_NEXTXYWH, &r2);
			}

			cgl_mouse_on();
			cgl_update(END_UPDATE);
		}
	}
}

/*
 * int cwindow::cw_do_menu(int x, int y)
 *
 * Handle mouse clicks in the menu bar of a window.
 *
 * Parameters:
 *
 * x	- x coordinate of mouse event
 * y	- y coordinate of mouse event
 *
 * Result: TRUE if the mouse event occured in the menu bar, FALSE
 *				 if it occured outside the menu bar.
 */

int cwindow::cw_do_menu(int x, int y)
{
	int item, p, exit_mstate;
	GRECT r, box;
	register int stop, i, c, title, otitle;

	if (win_menu == NULL)
		return FALSE;

	cw_bar_rect(&r);

	if (cgl_inrect(x, y, &r) == FALSE)
		return FALSE;

	p = win_menu[win_bar].ob_head;

	exit_mstate = (cgl_button_state() & 1) ? 0 : 1;

	if (((title = objc_find(win_menu, p, MAX_DEPTH, x, y)) >= 0) && (win_menu[title].ob_type == G_TITLE))
	{
		cgl_update(BEG_UPDATE);
		cgl_update(BEG_MCTRL);

		item = -1;
		stop = FALSE;

		do
		{
			CGL_SCRNBUF *buf;

			win_menu[title].ob_state |= SELECTED;
			cw_redraw_menu(title, &r);

			i = win_menu[p].ob_head;
			c = 0;

			/* Zoek welke titel geselekteerd is */

			while (i != title)
			{
				i = win_menu[i].ob_next;
				c++;
			}

			i = win_menu[win_mparent].ob_head;

			/* Zoek de bijbehorende box */

			while (c > 0)
			{
				i = win_menu[i].ob_next;
				c--;
			}

			cgl_objrect(win_menu, i, &box);

			box.g_x -= 1;
			box.g_y -= 1;
			box.g_w += 2;
			box.g_h += 2;

			otitle = title;

			if ((buf = cgl_save_area(&box)) == NULL)
				stop = TRUE;
			else
			{
				objc_draw(win_menu, i, MAX_DEPTH, box.g_x, box.g_y, box.g_w, box.g_h);

				do
				{
					int mx, my, dummy;
					register int oitem;

					oitem = item;

					stop = cgl_mouse_event(exit_mstate, &mx, &my, &dummy);

					if ((title = objc_find(win_menu, p, MAX_DEPTH, mx, my)) < 0)
					{
						title = otitle;

						if (((item = objc_find(win_menu, i, MAX_DEPTH, mx, my)) >= 0) &&
							(win_menu[item].ob_state & DISABLED))
							item = -1;
					}
					else
					{
						item = -1;
						if (exit_mstate != 0)
							stop = FALSE;
					}

					if (item != oitem)
					{
						if (oitem >= 0)
							cw_menu_change(win_menu, oitem, FALSE, &box);
						if (item >= 0)
							cw_menu_change(win_menu, item, TRUE, &box);
					}
				}
				while ((title == otitle) && (stop == FALSE));

				if (item >= 0)
					win_menu[item].ob_state &= ~SELECTED;

				cgl_restore_area(buf);
			}

			if (item < 0)
			{
				win_menu[otitle].ob_state &= ~SELECTED;
				cw_redraw_menu(otitle, &r);
			}
		}
		while (stop == FALSE);

		/* Wacht tot muisknop wordt losgelaten. */

		while (cgl_button_state() & 1);

		cgl_update(END_MCTRL);
		cgl_update(END_UPDATE);

		if (item >= 0)
		{
			cw_hndlmenu(item);								/* Call menu event handler. */
			if (cw_exist(this))								/* Check if this window still exists. */
				cw_menu_tnormal(title, 1);			/* Change title to normal state. */
		}
	}

	return TRUE;
}

/********************************************************************
 *																																	*
 * Member functions for event handling.															*
 *																																	*
 ********************************************************************/

/*
 * int cwindow::cw_hndlkey(int scancode, int keystate)
 *
 * Handle a keyboard event in the window.
 */

int cwindow::cw_hndlkey(int scancode, int keystate)
{
	return FALSE;
}

/*
 * void cwindow::cw_hndlbutton(int x, int y, int n, int bstate, int kstate)
 *
 * Handle a mouse event in the window.
 */

void cwindow::cw_hndlbutton(int x, int y, int n, int bstate, int kstate)
{
}

/*
 * void cwindow::cw_draw(GRECT *area)
 *
 * This function is called by the library, when a part of the
 * window has to be redrawn for some reason.
 */

void cwindow::cw_draw(int mode, GRECT *area, va_list args)
{
}

/*
 * void cwindow::cw_topped(void)
 *
 * Handle the WM_TOPPED event for the window.
 */

void cwindow::cw_topped(void)
{
	cw_set(WF_TOP);
}

/*
 * void cwindow::cw_newtop(void)
 *
 * Handle the WM_NEWTOP event for the window.
 */

void cwindow::cw_newtop(void)
{
}

/*
 * void cwindow::cw_closed(void)
 *
 * Handle the WM_CLOSED event for the window.
 */

void cwindow::cw_closed(void)
{
	delete this;
}

/*
 * void cwindow::cw_fulled(void)
 *
 * Handle the WM_FULLED event for the window.
 */

void cwindow::cw_fulled(void)
{
}

/*
 * void cwindow::cw_arrowed(int arrows)
 *
 * Handle the WM_ARROWED event for the window.
 */

void cwindow::cw_arrowed(int arrows)
{
}

/*
 * void cwindow::cw_hslider(int newpos)
 *
 * Handle the WM_HSLID for the window.
 */

void cwindow::cw_hslider(int newpos)
{
}

/*
 * void cwindow::cw_vslider(int newpos)
 *
 * Handle the WM_VSLID for the window.
 */

void cwindow::cw_vslider(int newpos)
{
}

/*
 * void cwindow::cw_sized(GRECT *newsize)
 *
 * Handle the WM_SIZED event for the window.
 */

void cwindow::cw_sized(GRECT *newsize)
{
	cw_set(WF_CURRXYWH, newsize);
}

/*
 * void cwindow::cw_moved(GRECT *newpos)
 *
 * Handle the WM_MOVED event for the window.
 */

void cwindow::cw_moved(GRECT *newpos)
{
	cw_set(WF_CURRXYWH, newpos);
}

/*
 * void cwindow::cw_hndlmenu(int item)
 *
 * Handle the selection of a menu item in the menu bar of the
 * window.
 */

void cwindow::cw_hndlmenu(int item)
{
}

/*
 * void cwindow::cw_top(void)
 *
 * This function is called by the library when the window becomes
 * the top window of the application.
 */

void cwindow::cw_top(void)
{
}

/********************************************************************
 *																																	*
 * Other library functions.																					*
 *																																	*
 ********************************************************************/

/*
 * cwindow *cw_get_top(void)
 *
 * Find the topmost window of the application.
 *
 * Result: pointer to the topmost window or NULL if there is no
 *         window open.
 */

cwindow *cw_get_top(void)
{
	int handle, dummy;
	register cwindow *w;

	wind_get(0, WF_TOP, &handle, &dummy, &dummy, &dummy);

	if ((w = cw_hfind(handle)) != NULL)
		return w;

	w = windows;

	while (w)
	{
		if (w->win_state == CW_CREATED)
			return w;
		w = w->next_win;
	}

	return desktop;
}

/*
 * cwindow *cw_find(int x, int y)
 *
 * Find the window at position (x, y)
 *
 * Parameters:
 *
 * x	- x coordinate
 * y	- y coordinate
 *
 * Result: pointer to window or NULL if no window found.
 */

cwindow *cw_find(int x, int y)
{
	return cw_hfind(wind_find(x, y));
}

/*
 * cwindow *cw_hfind(int handle)
 *
 * Find the window with handle 'handle'.
 *
 * Parameters:
 *
 * handle	- hanlde of the window to find.
 *
 * Result: pointer to the window or NULL if the window does not
 *				 exist.
 */

cwindow *cw_hfind(int handle)
{
	register cwindow *w = windows;

	while (w != NULL)
	{
		if (w->win_handle == handle)
			return w;
		w = w->next_win;
	}

	return (handle == 0) ? desktop : NULL;
}

/*
 * void cw_closeall(void)
 *
 * Close all open windows of the application.
 */

void cw_closeall(void)
{
	register cwindow *w = windows, *next;

	while(w)
	{
		next = w->next_win;
		delete w;
		w = next;
	}

/*	cw_close_desk();*/
}

int cw_exist(cwindow *w)
{
	register cwindow *p = windows;

	while (p)
	{
		if (p == w)
			return TRUE;
		p = p->next_win;
	}

	return FALSE;
}

/********************************************************************
 *																																	*
 * Event dispatchers.																								*
 *																																	*
 ********************************************************************/

/*
 * int cw_hndlbutton(int x, int y, int n, int bstate, int kstate)
 * int cw_hndlkey(int scancode, int keystate)
 * int cw_hndlmessage(int *message)
 *
 * Event handlers for the windows managed by the library.
 * Received events are dispatched to the window for which the
 * event occured.
 */

int cw_hndlbutton(int x, int y, int n, int bstate, int kstate)
{
	register cwindow *w;

	if (all_iconified)
		return FALSE;

	w = cw_find(x, y);

	if (cgl_dialogs)
	{
		if (w != (cwindow *) cgl_dialogs)
		{
			cgl_dialogs->cw_uniconify();
			cgl_dialogs->cw_set(WF_TOP);			/* Make dialog box top window. */
			Bconout(2, 7);
		}
		else if (!w->cw_is_iconified())
			((cwindow *) cgl_dialogs)->cw_hndlbutton(x, y, n, bstate, kstate);
		return TRUE;
	}
	else
	{
		if ((w != NULL) && !w->cw_is_iconified())
		{
			if (w->cw_do_menu(x, y) == FALSE)
			{
				w->cw_hndlbutton(x, y, n, bstate, kstate);
				return TRUE;
			}
			else
				return TRUE;
		}
		else
			return FALSE;
	}
}

int cw_hndlkey(int scancode, int keystate)
{
	if (all_iconified)
		return FALSE;

	if (cgl_dialogs)
	{
		cgl_dialogs->cw_uniconify();
		cgl_dialogs->cw_set(WF_TOP);				/* Make dialog box top window. */
		((cwindow *) cgl_dialogs)->cw_hndlkey(scancode, keystate);
		return TRUE;
	}
	else
	{
		register cwindow *w = cw_get_top();

		if ((w != NULL) && !w->cw_is_iconified())
			return w->cw_hndlkey(scancode, keystate);
		else
			return FALSE;
	}
}

int cw_hndlmessage(int *message)
{
	register cwindow *w;

	if (((message[0] < WM_REDRAW) || (message[0] > WM_TOOLBAR)) &&
			(message[0] != AP_DRAGDROP))
		return FALSE;

	if ((w = cw_hfind(message[3])) == NULL)
		return FALSE;

	switch (message[0])
	{
	case WM_REDRAW:
		w->cw_redraw(0, (GRECT *) &message[4]);
		break;

	case WM_TOPPED:
		if (cgl_dialogs && (w != (cwindow *) cgl_dialogs) && !all_iconified)
		{
			cgl_dialogs->cw_uniconify();			/* Uniconify dialog box if necessary. */
			cgl_dialogs->cw_set(WF_TOP);			/* Make dialog box top window. */
			Bconout(2, 7);
		}
		else
		{
			if (w->cw_is_iconified())
				w->cw_set(WF_TOP);
			else
				w->cw_topped();
		}
		break;

	case WM_CLOSED:
		if (cgl_dialogs && (w != (cwindow *) cgl_dialogs))
		{
			cgl_dialogs->cw_uniconify();			/* Uniconify dialog box if necessary. */
			cgl_dialogs->cw_set(WF_TOP);
			Bconout(2, 7);
		}
		else
			w->cw_closed();
		break;

	case WM_FULLED:
		w->cw_fulled();
		break;

	case WM_ARROWED:
		w->cw_arrowed(message[4]);
		break;

	case WM_HSLID:
		w->cw_hslider(message[4]);
		break;

	case WM_VSLID:
		w->cw_vslider(message[4]);
		break;

	case WM_SIZED:
		w->cw_sized((GRECT *) &message[4]);
		break;

	case WM_MOVED:
		if (w->cw_is_iconified())
			w->cw_set(WF_CURRXYWH, (GRECT *) &message[4]);
		else
			w->cw_moved((GRECT *) &message[4]);
		break;

	case WM_NEWTOP:
		if (cgl_dialogs && (w != (cwindow *) cgl_dialogs) && !all_iconified)
		{
			cgl_dialogs->cw_uniconify();			/* Uniconify dialog box if necessary. */
			cgl_dialogs->cw_set(WF_TOP);			/* Make dialog box top window. */
			Bconout(2, 7);
		}
		else if (!w->cw_is_iconified())
			w->cw_newtop();
		break;

	case WM_ICONIFY:
		w->cw_iconify((GRECT *) &message[4]);
		break;

	case WM_UNICONIFY:
		w->cw_uniconify();
		break;

	case WM_ALLICONIFY:
		{
			register cwindow *p;

			/*
			 * Close all windows, except the one which received the
			 * WM_ALLICONIFY message.
			 */

			p = windows;

			while (p)
			{
				if (w != p)
					p->cw_tmp_close();
				p = p->next_win;
			}

			/*
			 * Iconify the window, disable the menu bar and set the
			 * 'all_iconified' flag.
			 */

			w->cw_iconify((GRECT *) &message[4]);
			cgl_app->disable_menu();
			all_iconified = TRUE;
		}
		break;

	case AP_DRAGDROP:
		break;

	default :
		return FALSE;
	}

	return TRUE;
}


#ifndef __CGL_WIND_H
#define __CGL_WIND_H

#include <stdarg.h>

#include "cgl_appl.h"

/*
 * Window types. User defined types should start from 256.
 */

#define CW_NOT_DEF			0								/* Window has no specific type. */
#define CW_DIALOG				1								/* Modal dialog box. */
#define CW_NMDIALOG			2								/* Non modal dialog box. */
#define CW_ICON					3								/* Icon window. */

typedef enum { CW_INIT, CW_CREATED } CW_STATE;

class cwindow
{
 protected:
	class cwindow *prev_win;							/* Previous window pointer. */
	class cwindow *next_win;							/* Next window pointer. */
	int win_type;													/* Window type. */
	CW_STATE win_state;										/* Window status. */
	int win_handle;

	int win_flags;
	const char *win_title;								/* Window title. */
	const char *win_info;									/* Window info string. */
	int win_hsl_size;											/* Horizontal slider size. */
	int win_hsl_pos;											/* Horizontal slider position. */
	int win_vsl_size;											/* Vertical slider size. */
	int win_vsl_pos;											/* Vertical slider position. */

	OBJECT *win_menu;											/* Menu bar. */
	int win_bar;													/* Object index of bar. */
	int win_mparent;											/* Object index of menu parent. */

	GRECT win_size;												/* Size of window. */
	GRECT win_work;												/* Size of work area in window. */
	GRECT orig_win_size;									/* Size of window before iconify. */

	struct
	{
		int iconified : 1;									/* Window is iconified. */
		int tmp_closed : 1;									/* Window has been temporary closed. */
	} cw_flags;

	CGL_ERROR cw_do_open(void);						/* Open window. Called by 'cw_reopen'
																					 and 'cw_create'. */
	void cw_set_top(void);

	void cw_set_barpos(void);
	void cw_bar_rect(GRECT *r);
	void cw_draw_menu(int object, GRECT *r);
	void cw_redraw_menu(int object, GRECT *r);
	int cw_do_menu(int x, int y);

 public:
  	/*
  	 * Constructors and destructors.
  	 */

	cwindow(int type);
	virtual ~cwindow(void);

	/*
	 * Window operations.
	 */

	CGL_ERROR cw_create(int flags, GRECT *size);
	void cw_destroy(void);

	void cw_set(int field, int w1, int w2, int w3, int w4);
	void cw_set(int field) { cw_set(field, 0, 0, 0, 0); }
	void cw_set(int field, int w) { cw_set(field, w, 0, 0, 0); }
	void cw_set(int field, const GRECT *w) { cw_set(field, w->g_x, w->g_y, w->g_w, w->g_h); }
	void cw_set(int field, const void *w) { cw_set(field, (int) w, 0, 0, 0); }

	void cw_get(int field, int *w1, int *w2, int *w3, int *w4);
	void cw_get(int field, int *w) { int dummy; cw_get(field, w, &dummy, &dummy, &dummy); }
	void cw_get(int field, GRECT *w) { cw_get(field, &w->g_x, &w->g_y, &w->g_w, &w->g_h); }

	void cw_redraw(int mode, GRECT *area, ...);
	void cw_scroll(int mode, GRECT *area, int pixels, int hv, ...);
	void cw_send_redraw(GRECT *area);
	void cw_calc(int w_ctype, int w_flags, GRECT *input, GRECT *output);

	void cw_iconify(GRECT *r);						/* Iconify window. */
	void cw_uniconify(void);							/* Uniconify window. */

	void cw_tmp_close(void);							/* Temporary close window. */
	void cw_reopen(void);									/* Reopen window. */

	/*
	 * Enquiry functions.
	 */

/*	int cw_get_handle() { return xw_handle; }*/
	int cw_get_type(void) { return win_type; }
	int cw_is_iconified(void) { return cw_flags.iconified; }

	/*
	 * Functions for the menu bar.
	 */

	int cw_menu_bar(OBJECT *menu);
	void cw_menu_icheck(int item, int check);
	void cw_menu_ienable(int item, int enable);
	void cw_menu_text(int item, const char *text);
	void cw_menu_tnormal(int item, int normal);

	/*
	 * Event handlers.
	 */

	virtual int cw_hndlkey(int scancode, int keystate);
	virtual void cw_hndlbutton(int x, int y, int n, int bstate, int kstate);
	virtual void cw_draw(int mode, GRECT *area, va_list args);
	virtual void cw_topped(void);
	virtual void cw_newtop(void);
	virtual void cw_closed(void);
	virtual void cw_fulled(void);
	virtual void cw_arrowed(int arrows);
	virtual void cw_hslider(int newpos);
	virtual void cw_vslider(int newpos);
	virtual void cw_sized(GRECT *newsize);
	virtual void cw_moved(GRECT *newpos);
	virtual void cw_hndlmenu(int item);

	virtual void cw_top(void);

	/*
	 * Friends.
	 */

	friend cwindow *cw_get_top(void);
	friend cwindow *cw_hfind(int handle);
	friend int cw_hndlmessage(int *message);
	friend int cw_hndlbutton(int x, int y, int n, int bstate, int kstate);
	friend void cw_closeall(void);
	friend int cw_exist(cwindow *w);
};

/*
 * Utility functions for windows.
 */

extern cwindow *cw_find(int x, int y);
extern cwindow *cw_hfind(int handle);
extern cwindow *cw_get_top(void);
extern int cw_exist(cwindow *w);
extern cwindow *cw_first(void);
extern cwindow *cw_next(void);
extern cwindow *cw_last(void);
extern cwindow *cw_prev(void);

extern void cw_cycle(void);

#endif /* __CGL_WIND_H */

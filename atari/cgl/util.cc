
#include <aesbind.h>
#include <stdlib.h>
#include <vdibind.h>

#include "cgl_util.h"
#include "internal.h"

static int cgl_upd_ucnt = 0,						/* Counter for xw_update */
					 cgl_upd_mcnt = 0,						/* Counter for xw_update */
					 cgl_msoff_cnt = 0;						/* Counter for xd_mouse_on/xd_mouse_off */

/********************************************************************
 *																																	*
 * Utilities for the GEM OBJECT structure.													*
 *																																	*
 ********************************************************************/

/*
 * void cgl_objrect(OBJECT *tree, int object, GRECT *r)
 *
 * This function determines the rectangle enclosing an object.
 *
 * Parameters:
 *
 * tree		- object tree,
 * object	- index of object in tree,
 * r			- resulting rectangle.
 */

void cgl_objrect(OBJECT *tree, int object, GRECT *r)
{
	OBJECT *obj = &tree[object];

	objc_offset(tree, object, &r->g_x, &r->g_y);
	r->g_w = obj->ob_width;
	r->g_h = obj->ob_height;
}

/*
 * int cgl_obj_parent(OBJECT *tree, int object)
 *
 * Get the parent object of an object.
 */

int cgl_obj_parent(OBJECT *tree, int object)
{
	int i = tree[object].ob_next, j;

	while (i >= 0)
	{
		if ((j = tree[i].ob_head) >= 0)
		{
			do
			{
				if (j == object)
					return i;
				j = tree[j].ob_next;
			}
			while (j != i);
		}
		i = tree[i].ob_next;
	}

	return -1;
}

/*
 * int cgl_get_rbutton(OBJECT *tree, int rb_parent)
 *
 * Find the selected button in a set of radiobuttons.
 */

int cgl_get_rbutton(OBJECT *tree, int rb_parent)
{
	int i = tree[rb_parent].ob_head;
	OBJECT *obj;

	while ((i > 0) && (i != rb_parent))
	{
		obj = &tree[i];

		if ((obj->ob_state & SELECTED) && (obj->ob_flags & RBUTTON))
			return i;

		i = obj->ob_next;
	}

	return -1;
}

/*
 * void cgl_set_rbutton(OBJECT *tree, int rb_parent, int object)
 *
 * Select a button in a set of radio buttons. All other buttons
 * are deselected.
 */

void cgl_set_rbutton(OBJECT *tree, int rb_parent, int object)
{
	int i = tree[rb_parent].ob_head;
	OBJECT *obj;

	while ((i > 0) && (i != rb_parent))
	{
		obj = &tree[i];

		if (obj->ob_flags & RBUTTON)
		{
			if (i == object)
				obj->ob_state |= SELECTED;
			else
				obj->ob_state &= ~SELECTED;
		}

		i = obj->ob_next;
	}
}

/*
 * int cgl_get_tristate(int ob_state)
 *
 * Get the current state of a tristate button.
 */

int cgl_get_tristate(int ob_state)
{
	return ob_state & TRISTATE_MASK;
}

/*
 * int cgl_set_tristate(int ob_state, int state)
 *
 * Set the current state of a tristate button.
 */

int cgl_set_tristate(int ob_state, int state)
{
	return (ob_state & ~TRISTATE_MASK) | state;
}

/*
 * int cgl_is_tristate(OBJECT *object)
 *
 * Check if an object is a tristate button.
 */

int cgl_is_tristate(OBJECT *object)
{
	return ((object->ob_type >> 8) & 0xFF) == CGLD_RECTBUTTRI;
}

/*
 * long cgl_get_obspec(OBJECT *object)
 *
 * Get the obspec of an object. This function works for both USERDEF
 * and XUSERDEF structures.
 */

long cgl_get_obspec(OBJECT *object)
{
	if ((object->ob_type & 0xFF) == G_USERDEF)
	{
		USERBLK *userblk = object->ob_spec.userblk;

		if (IS_XUSER(userblk))
			return ((XUSERBLK *)userblk)->ob_spec;
		else
			return userblk->ub_parm;
	}
	else
		return object->ob_spec.index;
}

/*
 * void cgl_set_obspec(OBJECT *object, long obspec)
 *
 * Set the obspec of an object. This function works for both
 * USERDEF and XUSERDEF objects.
 */

void cgl_set_obspec(OBJECT *object, long obspec)
{
	if ((object->ob_type & 0xFF) == G_USERDEF)
	{
		USERBLK *userblk = object->ob_spec.userblk;

		if (IS_XUSER(userblk))
			((XUSERBLK *)userblk)->ob_spec = obspec;
		else
			userblk->ub_parm = obspec;
	}
	else
		object->ob_spec.index = obspec;
}

/*
 * void cgl_userdef(OBJECT *object, USERBLK *userblk, int (*code) (PARMBLK *parmblock))
 *
 * Make an object user-defined using the USERDEF structure.
 */

void cgl_userdef(OBJECT *object, USERBLK *userblk,
								 int (*code) (PARMBLK *parmblock))
{
	object->ob_type = (object->ob_type & 0xFF00) | G_USERDEF;
	userblk->ub_code = code;
	userblk->ub_parm = object->ob_spec.index;
	object->ob_spec.userblk = userblk;
}

/*
 * void cgl_xuserdef(OBJECT *object, XUSERBLK *userblk, int (*code) (PARMBLK *parmblock))
 *
 * Make an object user-defined using the XUSERDEF structure.
 */

void cgl_xuserdef(OBJECT *object, XUSERBLK *userblk,
									int (*code) (PARMBLK *parmblock))
{
	userblk->ub_code = code;
	userblk->ub_parm = userblk;
	userblk->ob_type = object->ob_type;
	userblk->ob_flags = object->ob_flags;
	userblk->ob_spec = object->ob_spec.index;

	object->ob_type = (object->ob_type & 0xFF00) | G_USERDEF;
	object->ob_flags &= ~(AES3D_1 | AES3D_2);
	object->ob_spec.userblk = (USERBLK *)userblk;
}

/********************************************************************
 *																																	*
 * Mouse and screen utilities.																			*
 *																																	*
 ********************************************************************/

/*
 * int cgl_update(int mode)
 *
 * Replacement function for wind_update.
 */

int cgl_update(int mode)
{
	switch (mode)
	{
	case BEG_UPDATE:
		if (++cgl_upd_ucnt != 1)
			return 1;
		break;
	case END_UPDATE:
		if (--cgl_upd_ucnt != 0)
			return 1;
		break;
	case BEG_MCTRL:
		if (++cgl_upd_mcnt != 1)
			return 1;
		break;
	case END_MCTRL:
		if (--cgl_upd_mcnt != 0)
			return 1;
		break;
	}

	return wind_update(mode);
}

/*
 * void cgl_mouse_off(void)
 *
 * Hide the mouse pointer.
 */

void cgl_mouse_off(void)
{
	if (cgl_msoff_cnt == 0)
		graf_mouse(M_OFF, NULL);
	cgl_msoff_cnt++;
}

/*
 * void cgl_mouse_on(void)
 *
 * Show the mouse pointer.
 */

void cgl_mouse_on(void)
{
	if (cgl_msoff_cnt == 1)
		graf_mouse(M_ON, NULL);
	cgl_msoff_cnt--;
}

/*
 * void cgl_reset_mouse_update(void)
 *
 * Make sure that the mouse is visible and the screen is not
 * locked (wind_update). This function is only for internal
 * use of the library.
 */

void cgl_reset_mouse_update(void)
{
	if (cgl_upd_ucnt > 0)
	{
		cgl_upd_ucnt = 0;
		wind_update(END_UPDATE);
	}

	if (cgl_upd_mcnt > 0)
	{
		cgl_upd_mcnt = 0;
		wind_update(END_MCTRL);
	}

	if (cgl_msoff_cnt > 0)
	{
		cgl_msoff_cnt = 0;
		graf_mouse(M_ON, NULL);
	}
}

/********************************************************************
 *																																	*
 * Utility functions for rectangles.																*
 *																																	*
 ********************************************************************/

/*
 * void cgl_rect2pxy(GRECT *r, int *pxy)
 * 
 * Convert a rectangle to a pxy array.
 *
 * Parameters:
 *
 * r		- pointer to rectangle,
 * pxy	- pointer to pxy array.
 */

void cgl_rect2pxy(GRECT *r, int *pxy)
{
	pxy[0] = r->g_x;
	pxy[1] = r->g_y;
	pxy[2] = r->g_x + r->g_w - 1;
	pxy[3] = r->g_y + r->g_h - 1;
}

/*
 * int cgl_rcintersect(GRECT *r1, GRECT *r2, GRECT *dest)
 *
 * Calculate the intersection of two rectangles.
 *
 * Parameters:
 *
 * r1	- first rectangle,
 * r2	- second rectangle,
 * dest	- intersection.
 *
 * Result: TRUE if 'r1' and 'r2' intersect, FALSE otherwise.
 */

int cgl_rcintersect(GRECT *r1, GRECT *r2, GRECT *dest)
{
	int xmin, xmax, ymin, ymax, h1, h2;

	xmin = max(r1->g_x, r2->g_x);
	ymin = max(r1->g_y, r2->g_y);

	h1 = r1->g_x + r1->g_w - 1;
	h2 = r2->g_x + r2->g_w - 1;
	xmax = min(h1, h2);

	h1 = r1->g_y + r1->g_h - 1;
	h2 = r2->g_y + r2->g_h - 1;
	ymax = min(h1, h2);

	dest->g_x = xmin;
	dest->g_y = ymin;
	dest->g_w = xmax - xmin + 1;
	dest->g_h = ymax - ymin + 1;

	if ((dest->g_w <= 0) || (dest->g_h <= 0))
		return FALSE;

	return TRUE;
}

/*
 * int cgl_clip_desk(GRECT *r)
 *
 * Calculate the intersection of rectangle 'r' and the desktop.
 * The result is placed in 'r'.
 *
 * Parameters:
 *
 * r	- pointer to rectangle.
 *
 * Result: TRUE if the rectangle intersects with the desktop,
 * 				 FALSE otherwise.
 */

int cgl_clip_desk(GRECT *r)
{
	return cgl_rcintersect(r, &cgl_desk, r);
}

/*
 * int cgl_inrect(int x, int y, GRECT *r)
 *
 * Check if a point is within a rectangle.
 *
 * Parmeters:
 *
 * x	- x coordinate of point,
 * y	- y coordinate of point,
 * r	- rectangle.
 *
 * Result: TRUE if the point is within the rectangle, FALSE
 *				 otherwise.
 */

int cgl_inrect(int x, int y, GRECT *r)
{
	if ((x >= r->g_x) && (x < (r->g_x + r->g_w)) && (y >= r->g_y) && (y < (r->g_y + r->g_h)))
		return TRUE;
	else
		return FALSE;
}

/********************************************************************
 *																																	*
 * Utility functions for the VDI workstation used by the library.		*
 *																																	*
 ********************************************************************/

/*
 * void cgl_clip_on(GRECT *r)
 *
 * Enable clipping.
 *
 * Parameters:
 *
 * r	- clipping rectangle.
 */

void cgl_clip_on(GRECT *r)
{
	int pxy[4];

	cgl_rect2pxy(r, pxy);
	vs_clip(cgl_vdi_handle, 1, pxy);
}

/*
 * void cgl_clip_off(void)
 *
 * Disable clipping.
 */

void cgl_clip_off(void)
{
	int pxy[4];

	vs_clip(cgl_vdi_handle, 0, pxy);
}

/*
 * size_t cgl_initmfdb(GRECT *r, MFDB *mfdb)
 *
 * Initialize a MFDB structure and calculate the amount of memory
 * needed for the given rectangle.
 *
 * Parameters:
 *
 * r		- rectangle,
 * mfdb	- MFDB structure.
 *
 * Result: amount of memory needed for 'rectangle'.
 */

size_t cgl_initmfdb(GRECT *r, MFDB *mfdb)
{
	size_t size;

	mfdb->fd_w = (r->g_w + 16) & 0xFFF0;
	mfdb->fd_h = r->g_h;
	mfdb->fd_wdwidth = mfdb->fd_w / 16;
	mfdb->fd_stand = 0;
	mfdb->fd_nplanes = cgl_nplanes;

	size = ((size_t) (mfdb->fd_w) * (size_t) r->g_h * (size_t) cgl_nplanes) / 8L;

	return size;
}

/*
 * CGL_SCRNBUF *cgl_save_area(GRECT *area)
 *
 * Save a part of the screen in a buffer. The memory for the
 * buffer is allocated with malloc.
 *
 * Parameters:
 *
 * area	- area to save.
 *
 * Result: a pointer to the screen buffer, or NULL if there is
 *				 not enough memory.
 */

CGL_SCRNBUF *cgl_save_area(GRECT *area)
{
	CGL_SCRNBUF *result;
	size_t mem;
	MFDB bmfdb;

	mem = cgl_initmfdb(area, &bmfdb);

	if ((result = (CGL_SCRNBUF *) malloc(sizeof(CGL_SCRNBUF) + mem)) != NULL)
	{
		bmfdb.fd_addr = result->buf;
		result->area = *area;
		result->bmfdb = bmfdb;

		if ((result->draw = cgl_rcintersect(&result->area, &cgl_desk, &result->area)) == TRUE)
		{
			int pxy[8];
			MFDB smfdb;

			cgl_rect2pxy(&result->area, pxy);
			pxy[4] = 0;
			pxy[5] = 0;
			pxy[6] = result->area.g_w - 1;
			pxy[7] = result->area.g_h - 1;

			smfdb.fd_addr = NULL;

			cgl_mouse_off();
			vro_cpyfm(cgl_vdi_handle, 3, pxy, &smfdb, &bmfdb);
			cgl_mouse_on();
		}
	}

	return result;
}

/*
 * void cgl_restore_area(CGL_SCRNBUF *buf)
 *
 * Restore a part of the screen from a buffer. The memeory
 * allocated for the buffer is freed.
 *
 * Parameters:
 *
 * buf	- pointer to screen buffer.
 */

void cgl_restore_area(CGL_SCRNBUF *buf)
{
	if (buf != NULL)
	{
		if (buf->draw == TRUE)
		{
			int pxy[8];
			MFDB smfdb;

			pxy[0] = 0;
			pxy[1] = 0;
			pxy[2] = buf->area.g_w - 1;
			pxy[3] = buf->area.g_h - 1;
			cgl_rect2pxy(&buf->area, &pxy[4]);

			smfdb.fd_addr = NULL;

			cgl_mouse_off();
			vro_cpyfm(cgl_vdi_handle, 3, pxy, &buf->bmfdb, &smfdb);
			cgl_mouse_on();
		}

		free(buf);
	}
}

/*
 * void cgl_move_screen(GRECT *dest, GRECT *src)
 *
 * Move a part from the screen from the source area to the
 * destination area.
 *
 * Parameters:
 *
 * dest	- destination area.
 * src	- source area.
 */

void cgl_move_screen(GRECT *dest, GRECT *src)
{
	MFDB mfdb;
	int pxy[8];

	mfdb.fd_addr = NULL;
	cgl_rect2pxy(src, pxy);
	cgl_rect2pxy(dest, &pxy[4]);

	vro_cpyfm(cgl_vdi_handle, S_ONLY, pxy, &mfdb, &mfdb);
}


#ifndef __CGL_UTIL_H
#define __CGL_UTIL_H

#include <aesbind.h>
#include <vdibind.h>

typedef struct
{
	int draw;															/* Was area saved? */
	GRECT area;														/* Coordinates of saved area. */
	MFDB bmfdb;														/* MFDB for buffer. */
	unsigned char buf[0];									/* Start of screen buffer. */
} CGL_SCRNBUF;

/*
 * Utility functions for objects.
 */

extern void cgl_objrect(OBJECT *tree, int object, GRECT *r);
extern int cgl_obj_parent(OBJECT *tree, int object);
extern int cgl_get_rbutton(OBJECT *tree, int rb_parent);
extern void cgl_set_rbutton(OBJECT *tree, int rb_parent, int object);
extern int cgl_get_tristate(int ob_state);
extern int cgl_set_tristate(int ob_state, int state);
extern int cgl_is_tristate(OBJECT *object);
extern long cgl_get_obspec(OBJECT *object);
extern void cgl_set_obspec(OBJECT *object, long obspec);
extern void cgl_userdef(OBJECT *object, USERBLK *userblk,
												int (*code) (PARMBLK *parmblock));

/*
 * Utility functions for mouse and screen locking.
 */

extern int cgl_update(int mode);
extern void cgl_mouse_off(void);
extern void cgl_mouse_on(void);

/*
 * Utility functions for rectangles.
 */

extern void cgl_rect2pxy(GRECT *r, int *pxy);
extern int cgl_rcintersect(GRECT *r1, GRECT *r2, GRECT *dest);
extern int cgl_inrect(int x, int y, GRECT *r);

/*
 * VDI utility functions.
 */

extern void cgl_clip_on(GRECT *r);
extern void cgl_clip_off(void);
extern size_t cgl_initmfdb(GRECT *r, MFDB *mfdb);
extern CGL_SCRNBUF *cgl_save_area(GRECT *area);
extern void cgl_restore_area(CGL_SCRNBUF *buf);
extern void cgl_move_screen(GRECT *dest, GRECT *src);

#endif /* __CGL_UTIL_H */

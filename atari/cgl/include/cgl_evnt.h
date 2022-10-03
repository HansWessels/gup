
#ifndef __CGL_EVNT_H
#define __CGL_EVNT_H

#include <aesbind.h>

class cglevent
{
 public:
	int ev_mflags;
	int ev_mbclicks, ev_mbmask, ev_mbstate;
	int ev_mm1flags;
	GRECT ev_mm1;
	int ev_mm2flags;
	GRECT ev_mm2;
	unsigned long ev_mtcount;

	int ev_mwhich;
	int ev_mmox, ev_mmoy;
	int ev_mmobutton;
	int ev_mmokstate;
	int ev_mkreturn;
	int ev_mbreturn;

	int ev_mmgpbuf[8];

	int ev_keycode;
};

/*
 * Functions for events.
 */

extern int cgl_keycode(int scancode, int kstate);
extern int cgl_xmulti(cglevent *events);
extern int cgl_button_state(void);
extern int cgl_mouse_event(int mstate, int *x, int *y, int *kstate);

#endif /* __CGL_EVNT_H */

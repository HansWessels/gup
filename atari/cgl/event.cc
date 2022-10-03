
#include <ctype.h>
#include <osbind.h>
#include <stddef.h>

#include "cgl_appl.h"
#include "cgl_evnt.h"
#include "cglscode.h"
#include "internal.h"

/*
 * New AES messages.
 */

#define AP_TERM		50
#define AP_TFAIL	51

int cgl_local_button_event = FALSE;

/********************************************************************
 *																																	*
 * Event dispatchers.																								*
 *																																	*
 ********************************************************************/

/*
 * void cgl_dispatch_message(int *msg)
 *
 * Send a message event to the right destination.
 *
 * Parameters:
 *
 * msg		- pointer to message.
 */

void cgl_dispatch_message(int *msg)
{
	OBJECT *old_menu;

	switch(msg[0])
	{
	case MN_SELECTED:
		old_menu = cgl_app->menu;

		if (((cgl_app->menu[msg[4]].ob_state & DISABLED) == 0) &&
				((cgl_app->menu[msg[3]].ob_state & DISABLED) == 0) &&
				(cgl_dialogs == NULL))
		{
			if (cgl_app->handle_menu(msg[4]))
				cgl_app->quit_appl = TRUE;
		}

		if (cgl_app->menu == old_menu)			/* Check if the menu bar has been changed. */
			menu_tnormal(old_menu, msg[3], 1);
		break;

	case AP_TERM:
		if (cgl_app->terminate())
			cgl_app->quit_appl = TRUE;
		else
		{
			static int ap_tfail[8] = { AP_TFAIL, 0 };
			shel_write(10, 0, 0, (char *) ap_tfail, NULL);
		}
		break;

	default:
		cw_hndlmessage(msg);
		break;
	}
}

/*
 * void cgl_dispatch_keyboard_event(int *msg)
 *
 * Send a keyboard event to the right destination.
 *
 * Parameters:
 *
 * scancode		- code of key.
 * kstate			- state of SHIFT, CONTROL and ALTERNATE keys.
 */

void cgl_dispatch_keyboard_event(int scancode, int kstate)
{
	cw_hndlkey(scancode, kstate);
}

/********************************************************************
 *																																	*
 * Utility functions.																								*
 *																																	*
 ********************************************************************/

/*
 * int cgl_keycode(int scancode, int kstate)
 *
 * Convert a VDI scancode to an own scancode.
 *
 * Parameters:
 *
 * scancode	- scancode of key
 * kstate	- state of SHIFT, CONTROL and ALTERNATE keys.
 *
 * Result: Own scancode.
 */

int cgl_keycode(int scancode, int kstate)
{
	int keycode, nkstate, scan;

	/* Zet key state om in eigen formaat */

	nkstate = (kstate & 3) ? CGL_SHIFT : 0;
	nkstate = nkstate | ((kstate & 0xC) << 7);

	/* Bepaal scancode */

	scan = ((unsigned int) scancode & 0xFF00) >> 8;

	/* Controleer of de scancode hoort bij een ASCII teken */

	if ((scan < 59) || (scan == 74) || (scan == 78) || (scan == 83) ||
		(scan == 96) || ((scan >= 99) && (scan <= 114)) || (scan >= 117))
	{
		if (scan >= 120)
			scan -= 118;
		if ((keycode = scancode & 0xFF) == 0)
			keycode = toupper((int) ((unsigned char) (((char *) ((_KEYTAB *) Keytbl((void *) -1, (void *) -1, (void *) -1))->unshift)[scan])));
		keycode |= nkstate;
	}
	else
	{
		nkstate |= CGL_SCANCODE;
		keycode = nkstate | scan;
	}

	return keycode;
}

/* Vervanging van evnt_multi, die eigen keycode terug levert. */

int cgl_xmulti(cglevent *events)
{
	int r;
	static int level = 0;
	int old_mtcount, old_mflags;

	level++;
	old_mtcount = events->ev_mtcount;
	old_mflags = events->ev_mflags;

	/* Check if the time out time is shorter than the minimum time.
	   If true set to the minimum time. */

	if (events->ev_mtcount < cgl_min_timer)
		events->ev_mtcount = cgl_min_timer;

	/* No message events when a dialog is opened and the dialog is
	   not in a window or when the application is waiting for a
	   mouse event. */

	if ((cgl_dialogs && (cgl_dialogs->get_dial_mode() != CGLD_WINDOW)) ||
			cgl_local_button_event)
		events->ev_mflags &= ~MU_MESAG;

	events->ev_mwhich = evnt_multi(events->ev_mflags, events->ev_mbclicks,
		events->ev_mbmask, events->ev_mbstate, events->ev_mm1flags,
		events->ev_mm1.g_x, events->ev_mm1.g_y, events->ev_mm1.g_w,
		events->ev_mm1.g_h, events->ev_mm2flags, events->ev_mm2.g_x,
		events->ev_mm2.g_y, events->ev_mm2.g_w, events->ev_mm2.g_h, events->ev_mmgpbuf,
		events->ev_mtcount,
		&events->ev_mmox, &events->ev_mmoy, &events->ev_mmobutton,
		&events->ev_mmokstate, &events->ev_mkreturn, &events->ev_mbreturn);

	if ((r = events->ev_mwhich) & MU_KEYBD)
	{
		events->ev_keycode = cgl_keycode(events->ev_mkreturn, events->ev_mmokstate);
		cgl_dispatch_keyboard_event(events->ev_keycode,events->ev_mmokstate);
	}

	if (r & MU_MESAG)
		cgl_dispatch_message(events->ev_mmgpbuf);
		
	if ((r & MU_BUTTON) && !cgl_local_button_event)
		cw_hndlbutton(events->ev_mmox, events->ev_mmoy,
								  events->ev_mbreturn, events->ev_mmobutton,
								  events->ev_mmokstate);

	events->ev_mflags = old_mflags;
	events->ev_mtcount = old_mtcount;
	events->ev_mwhich = r;

	return r;
}

/*
 * int cgl_button_state(void)
 *
 * Get the current state of the mouse buttons.
 *
 * Result: current state of the mouse buttons.
 */

int cgl_button_state(void)
{
	int dummy, mstate;

	graf_mkstate(&dummy, &dummy, &mstate, &dummy);
	return mstate;
}

/*
 * int cgl_mouse_event(int mstate, int *x, int *y, int *kstate)
 *
 * Wait for a mouse event.
 *
 * Parameters:
 *
 * mstate	- state of the mouse buttons which should trigger the
 *			  event,
 * x		- x coordinate of mouse,
 * y		- y coordinate of mouse,
 * kstate	- state of SHIFT, CONTROL and ALTERNATE keys.
 *
 * Result : TRUE if the event occured, FALSE if the event did not
 *			occur.
 */

int cgl_mouse_event(int mstate, int *x, int *y, int *kstate)
{
	cglevent events;
	int flags;

	cgl_local_button_event = TRUE;

	events.ev_mflags = MU_TIMER | MU_BUTTON;
	events.ev_mbclicks = 2;
	events.ev_mbmask = 1;
	events.ev_mbstate = mstate;
	events.ev_mm1flags = 0;
	events.ev_mm2flags = 0;
	events.ev_mtcount = 0;

	flags = cgl_xmulti(&events);

	*x = events.ev_mmox;
	*y = events.ev_mmoy;
	*kstate = events.ev_mmokstate;

	cgl_local_button_event = FALSE;

	return (flags & MU_BUTTON) ? TRUE : FALSE;
}

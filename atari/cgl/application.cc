
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "cgl_appl.h"
#include "cgl_dial.h"
#include "cgl_evnt.h"
#include "cgl_wind.h"

#define EVNT_FLAGS	(MU_MESAG | MU_BUTTON | MU_KEYBD)

int cgl_vdi_handle;											/* VDI handle. */
int cgl_ncolors;												/* Number of colors. */
int cgl_nplanes;												/* Number of planes. */
int cgl_fonts_loaded;										/* Fonts loaded flags. */
CGL_FONT cgl_regular_font;							/* Info of regular font. */
CGL_FONT cgl_small_font;								/* Info of small font. */

int cgl_apid;														/* AES application id. */
int cgl_aes4_0;													/* AES 4.0 flag (MultiTOS extensions). */
int cgl_draw_3d;												/* 3D buttons. */
int cgl_min_timer;											/* Minimum time passed to event_multi(). */
GRECT cgl_desk;													/* Size of desktop. */

APPLICATION *cgl_app = NULL;

cgl_dialog *cgl_dialogs = NULL;					/* List of modal dialog boxes. */

/*
 * Constructors and destructors for the APPLICATION class.
 */

APPLICATION::APPLICATION(const char *prgname, int load_fonts)
{
	cgl_app = this;
	quit_appl = FALSE;
	menu = NULL;

	if ((cgl_apid = appl_init()) < 0)
	{
		status = CGL_AES;
		return;
	}
	else
	{
		int dummy, i, work_in[11], work_out[57];

		prg_name = prgname;
		cgl_aes4_0 = (_global[0] >= 0x400);
		cgl_draw_3d = (_global[0] >= 0x330);
		cgl_min_timer = 20;									/* Minimum time passed to event_multi(). */

		if (cgl_aes4_0)
		{
			char str[128];

			strcpy(str, "  ");
			strcat(str, prg_name);
			menu_register(cgl_apid, str);			/* Set the menu name to the user supllied program name. */
	
			shel_write(9, 1, 0, NULL, NULL);	/* Tell the AES that we know the AP_TERM message. */
		}

		wind_get(0, WF_WORKXYWH, &cgl_desk.g_x, &cgl_desk.g_y, &cgl_desk.g_w, &cgl_desk.g_h);
		cgl_vdi_handle = graf_handle(&dummy, &dummy, &dummy, &dummy);

		for (i = 0; i < 10; i++)
			work_in[i] = 1;
		work_in[10] = 2;

		v_opnvwk(work_in, &cgl_vdi_handle, work_out);

		if (cgl_vdi_handle == 0)
		{
			appl_exit();
			status = CGL_VDI;
			return;
		}
		else
		{
			int pix_height;

			cgl_ncolors = work_out[13];
			pix_height = work_out[4];

			vq_extnd(cgl_vdi_handle, 1, work_out);
			cgl_nplanes = work_out[4];

			if (load_fonts && vq_vgdos())
			{
				nfonts = vst_load_fonts(cgl_vdi_handle, 0);
				cgl_fonts_loaded = TRUE;
			}
			else
			{
				nfonts = 0;
				cgl_fonts_loaded = FALSE;
			}

			if (cgl_aes4_0)
			{
				appl_getinfo(0, &cgl_regular_font.fnt_height,
										 &cgl_regular_font.fnt_id,
										 &cgl_regular_font.fnt_type, &dummy);
				appl_getinfo(1, &cgl_small_font.fnt_height,
										 &cgl_small_font.fnt_id,
										 &cgl_small_font.fnt_type, &dummy);
			}
			else
			{
				vqt_attributes(cgl_vdi_handle, work_out);

				cgl_regular_font.fnt_id = 1;
				cgl_regular_font.fnt_type = 0;
				cgl_regular_font.fnt_height = (work_out[7] <= 7) ? 9 : 10;

				cgl_small_font.fnt_id = 1;
				cgl_small_font.fnt_type = 0;
				cgl_small_font.fnt_height = 8;
			}

			cgl_small_font.fnt_id = vst_font(cgl_vdi_handle,
																			 cgl_small_font.fnt_id);
			cgl_small_font.fnt_height = vst_point(cgl_vdi_handle,
																						cgl_small_font.fnt_height,
																						&dummy, &dummy, &dummy,
																						&cgl_small_font.fnt_chh);
			vqt_width(cgl_vdi_handle, ' ', &cgl_small_font.fnt_chw, &dummy,
								&dummy);

			cgl_regular_font.fnt_id = vst_font(cgl_vdi_handle,
																				 cgl_regular_font.fnt_id);
			cgl_regular_font.fnt_height = vst_point(cgl_vdi_handle,
																							cgl_regular_font.fnt_height,
																							&dummy, &dummy, &dummy,
																							&cgl_regular_font.fnt_chh);
			vqt_width(cgl_vdi_handle, ' ', &cgl_regular_font.fnt_chw, &dummy,
								&dummy);

			status = CGL_OK;
		}
	}
}

void exit_cgl(void)
{
	if (cgl_app)
		cgl_app->~APPLICATION();
	exit(0);
}

APPLICATION::~APPLICATION(void)
{
	CGLOBJDATA *h = cgl_objdata, *next;

	/*
	 * Delete all opened modal dialog boxes.
	 */

	while (cgl_dialogs)
		delete cgl_dialogs;

	/*
	 * Free the memory used by USERDEF and XUSERDEF structures.
	 */

	while (h != NULL)
	{
		next = h->next;
		free(h);
		h = next;
	}

	/*
	 * Delete all other windows.
	 */

	cw_closeall();

	if (menu)
		menu_bar(menu, 0);									/* Remove application's menu bar. */

	cgl_reset_mouse_update();							/* Make sure mouse is visible and screen not locked. */

	if (cgl_fonts_loaded)
		vst_unload_fonts(cgl_vdi_handle, 0);

	v_clsvwk(cgl_vdi_handle);
	appl_exit();

	exit(0);
}

/*
 * Member functions of the APPLICATION class.
 */

void APPLICATION::run(void)
{
	int event;
	cglevent events;

	events.ev_mflags = EVNT_FLAGS;
	events.ev_mbclicks = 2;
	events.ev_mbmask = 1;
	events.ev_mbstate = 1;
	events.ev_mm1flags = 0;
	events.ev_mm2flags = 0;
	events.ev_mtcount = 0;

	while (!quit_appl)
		event = cgl_xmulti(&events);
}

void APPLICATION::set_menu_bar(OBJECT *new_menu)
{
	if (menu)
		menu_bar(menu, 0);									/* Remove current menu bar. */
	menu_bar(new_menu, 1);
	menu = new_menu;
	mn_disable_cnt = 0;
}

void APPLICATION::disable_menu(void)
{
	if ((menu != NULL) && (mn_disable_cnt == 0))
	{
		register int obj, title_parent;

		/*
		 * Disable all menu titles, except the first one.
		 */

		title_parent = menu[menu->ob_head].ob_head;

		obj = menu[title_parent].ob_head;		/* Index of first menu title. */
		menu[obj].ob_state &= ~SELECTED;		/* Make sure the menu title is deselected. */
		obj = menu[obj].ob_next;						/* Next title. */

		while (obj != title_parent)
		{
			menu[obj].ob_state = (menu[obj].ob_state | DISABLED) & ~SELECTED;
			obj = menu[obj].ob_next;
		}

		/*
		 * Disable the first menu item in the first drop down menu.
		 */

		menu[menu[menu[menu->ob_tail].ob_head].ob_head].ob_state |= DISABLED;

		menu_bar(menu, 1);
	}

	mn_disable_cnt++;
}

void APPLICATION::enable_menu(void)
{
	if ((menu != NULL) && (mn_disable_cnt == 1))
	{
		register int obj, title_parent;

		/*
		 * Enable all menu titles.
		 */

		title_parent = menu[menu->ob_head].ob_head;
		obj = menu[menu[title_parent].ob_head].ob_next;	/* Skip the first one. */

		while (obj != title_parent)
		{
			menu[obj].ob_state &= ~DISABLED;
			obj = menu[obj].ob_next;
		}

		/*
		 * Enable the first menu item in the first drop down menu.
		 */

		menu[menu[menu[menu->ob_tail].ob_head].ob_head].ob_state &= ~DISABLED;

		menu_bar(menu, 1);
	}

	if (mn_disable_cnt > 0)
		mn_disable_cnt--;
}

/*
 * Event handlers for the application class.
 */

int APPLICATION::terminate(void)
{
	return TRUE;
}

void APPLICATION::all_windows_closed(void)
{
}

int APPLICATION::handle_menu(int item)
{
	return FALSE;
}

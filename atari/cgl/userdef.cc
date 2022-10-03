
#include <aesbind.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vdibind.h>

#include "cgl_dial.h"
#include "cgl_util.h"
#include "internal.h"

static short rb_data8[] = { 0x07E0, 0x1818, 0x23C4, 0x4422, 0x23C4,
														0x1818, 0x07E0, 0x0000 };
static short rbs_data8[] = { 0x07E0, 0x1818, 0x23C4, 0x47E2, 0x23C4,
							 							 0x1818, 0x07E0, 0x0000 };
static short rb_mask8[] = { 0xF81F, 0xE007, 0xC003, 0x8001, 0xC003,
														0xE007, 0xF81F, 0xFFFF };
static short rb_data16[] = { 0x0000, 0x03C0, 0x0C30, 0x1008, 0x2184,
														 0x2664, 0x4422, 0x4812, 0x4812, 0x4422,
														 0x2664, 0x2184, 0x1008, 0x0C30, 0x03C0,
														 0x0000 };
static short rbs_data16[] = { 0x0000, 0x03C0, 0x0C30, 0x1008, 0x2184,
														  0x27E4, 0x47E2, 0x4FF2, 0x4FF2, 0x47E2,
														  0x27E4, 0x2184, 0x1008, 0x0C30, 0x03C0,
														  0x0000 };
static short rb_mask16[] = { 0xFFFF, 0xFC3F, 0xF00F, 0xE007, 0xC003,
														 0xC003, 0x8001, 0x8001, 0x8001, 0x8001,
														 0xC003, 0xC003, 0xE007, 0xF00F, 0xFC3F,
														 0xFFFF };

CGLOBJDATA *cgl_objdata = NULL;

static int cgl_bg_col = 8;
static int cgl_ind_col = 8;
static int cgl_act_col = 8;

/*
 * Inline function to check which kind of 3D object an object is.
 */

static inline int GET_3D(int flags)	\
	{ return (flags & (AES3D_1 | AES3D_2)); }
static inline int IS_ACT(int flags)	\
	{ return (GET_3D(flags) == (AES3D_1 | AES3D_2)); }
static inline int IS_IND(int flags)	\
	{ return (GET_3D(flags) == AES3D_1); }
static inline int IS_BG(int flags)		\
	{ return (GET_3D(flags) == AES3D_2); }

static inline int cgld_is_xtndelement(int emode)	\
	{ return ((cgld_emode_specs[emode] & __XD_IS_ELEMENT) != 0); }

/********************************************************************
 *																																	*
 * Functions for drawing user-defined objects.											*
 *																																	*
 ********************************************************************/

/*
 * static int get_3d_color(int flags)
 *
 * Determine the background color of an object.
 *
 * Parameters:
 *
 * flags	- The object flags of the object.
 *
 * Result: Color of the object.
 */

static int get_3d_color(int flags)
{
	int color;

	if (IS_ACT(flags))
		color = cgl_act_col;
	else if (IS_IND(flags))
		color = cgl_ind_col;
	else if (IS_BG(flags))
		color = cgl_bg_col;
	else
		color = 0;

	return (color >= cgl_ncolors) ? 0 : color;
}

/*
 * static int cgld_bborder(OBJECT *tree, int object)
 *
 * Calculate the thickness of the border of an object.
 */

static int cgld_bborder(OBJECT *tree, int object)
{
	if (tree[object].ob_flags & DEFAULT)
		return 3;
	else if (tree[object].ob_flags & EXIT)
		return 2;
	else
		return 1;
}

static long cgld_strlen(char *s)
{
	long l;

	l = strlen(s);

	if (strchr(s, '#') != NULL)
		l--;

	return l;
}

#define prt_text(s, x, y, flags)  prt_xtndtext(s, x, y, flags, 0)

static void prt_xtndtext(char *s, int x, int y, int flags, int attrib)
{
	char tmp[80], *h, *p = NULL;/* <- buffer is enlarged! */

	if (flags & DISABLED)
		attrib ^= 2;

	vst_effects(cgl_vdi_handle, attrib);

	/* I_A enhanced: now you can place '#' in text itself! */

	h = strcpy(tmp, s);

	while ((h = strchr(h, '#')) != NULL)
	{
		strcpy(h, h + 1);
		if (*h != '#')
		{
			/* remember location of single '#' in text! */
			p = h--;
		}
		/* else: double '#': make it one! */
		h++;
	}

	v_gtext(cgl_vdi_handle, x, y, tmp);

	if (p)
	{
		/* do underline some character! */
		int xtnd[8];
		char c = *p;

		*p = 0;

		/* request position of this character! */

		vqt_extent(cgl_vdi_handle, tmp, xtnd);
		vst_effects(cgl_vdi_handle, attrib ^ 8);	/* XOR due to text-style extensions! */
		tmp[0] = c;
		tmp[1] = 0;
		v_gtext(cgl_vdi_handle, x + (xtnd[2] - xtnd[0]), y, tmp);
	}
}


static void draw_rect(int x, int y, int w, int h)
{
	int pxy[10];

	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = x + w - 1;
	pxy[3] = y;
	pxy[4] = x + w - 1;
	pxy[5] = y + h - 1;
	pxy[6] = x;
	pxy[7] = y + h - 1;
	pxy[8] = x;
	pxy[9] = y;

	v_pline(cgl_vdi_handle, 5, pxy);
}

static void draw_frame(GRECT *frame, int start, int end)
{
	int i, s, e;

	if (start > end)
	{
		s = end;
		e = start;
	}
	else
	{
		s = start;
		e = end;
	}

	for (i = s; i <= e; i++)
		draw_rect(frame->g_x + i, frame->g_y + i, frame->g_w - 2 * i,
				  frame->g_h - 2 * i);
}

static void set_linedef(int color)
{
	vsl_color(cgl_vdi_handle, color);
	vsl_ends(cgl_vdi_handle, 0, 0);
	vsl_type(cgl_vdi_handle, 1);
	vsl_width(cgl_vdi_handle, 1);
}

static void set_textdef(void)
{
	int dummy;

	vst_font(cgl_vdi_handle, cgl_regular_font.fnt_id);
	vst_rotation(cgl_vdi_handle, 0);
	vst_alignment(cgl_vdi_handle, 0, 5, &dummy, &dummy);
	vst_point(cgl_vdi_handle, cgl_regular_font.fnt_height, &dummy, &dummy, &dummy, &dummy);
	vst_color(cgl_vdi_handle, 1);
}

static void set_filldef(int color)
{
	vsf_color(cgl_vdi_handle, color);
	vsf_interior(cgl_vdi_handle, FIS_SOLID);
	vsf_perimeter(cgl_vdi_handle, 0);
}

static void clr_object(GRECT *r, int color)
{
	int pxy[4];

	cgl_rect2pxy(r, pxy);
	set_filldef(color);
	vr_recfl(cgl_vdi_handle, pxy);
}

static int ub_drag(PARMBLK *pb)
{
	int object = pb->pb_obj, pxy[4], border;
	GRECT frame, clip;
	OBJECT *tree = pb->pb_tree;
	union { bfobspec obspec; long index; } obspec;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	frame.g_x = pb->pb_x;
	frame.g_y = pb->pb_y;
	frame.g_w = pb->pb_w;
	frame.g_h = pb->pb_h;

	vswr_mode(cgl_vdi_handle, MD_REPLACE);

	cgl_clip_on(&clip);

	clr_object(&frame, (cgl_draw_3d) ? get_3d_color(tree[object].ob_flags) : 0);

	obspec.index = pb->pb_parm;

	set_linedef(obspec.obspec.framecol);
	draw_frame(&frame, 0, obspec.obspec.framesize - 1);

	if(cgl_draw_3d)
	{	
		pxy[0] = frame.g_x - 1;
		pxy[1] = frame.g_y + obspec.obspec.framesize;
		pxy[2] = frame.g_x - 1;
		pxy[3] = frame.g_y + frame.g_h;

		vsl_color(cgl_vdi_handle, 0);
		v_pline(cgl_vdi_handle, 2, pxy);

		pxy[0] = frame.g_x - 1;
		pxy[1] = frame.g_y + frame.g_h;
		pxy[2] = frame.g_x + frame.g_w - obspec.obspec.framesize - 1;
		pxy[3] = frame.g_y + frame.g_h;

		vsl_color(cgl_vdi_handle, 9);
		v_pline(cgl_vdi_handle, 2, pxy);

		border = 0;			/* schuine lijn alleen in binnenzijde */
	}
	else
	{
		if (tree[object].ob_state == OUTLINED)
		{
			vsl_color(cgl_vdi_handle, 1);
			draw_frame(&frame, -3, -3);
			vsl_color(cgl_vdi_handle, 0);
			draw_frame(&frame, -1, -2);
			border = 3;
		}
		else
			border = 0;
	}

	vsl_color(cgl_vdi_handle, 1);

	pxy[0] = frame.g_x - border;
	pxy[1] = frame.g_y - border;
	pxy[2] = frame.g_x + frame.g_w - 1 + border;
	pxy[3] = frame.g_y + frame.g_h - 1 + border;

	v_pline(cgl_vdi_handle, 2, pxy);

	cgl_clip_off();

	return 0;
}

static int ub_roundrb(PARMBLK *pb)
{
	int x = pb->pb_x, y = pb->pb_y, pxy[8];
	static int ci[2] = {1, 0};
	void *data;
	MFDB smfdb, dmfdb;
	GRECT clip;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	if (cgl_regular_font.fnt_chh < 16)
	{
		smfdb.fd_addr = rb_mask8;
		data = (pb->pb_currstate & SELECTED) ? rbs_data8 : rb_data8;
		smfdb.fd_h = 8;
		pxy[3] = 7;
	}
	else
	{
		smfdb.fd_addr = rb_mask16;
		data = (pb->pb_currstate & SELECTED) ? rbs_data16 : rb_data16;
		smfdb.fd_h = 16;
		pxy[3] = 15;
	}

	smfdb.fd_stand = 0;
	smfdb.fd_nplanes = 1;
	smfdb.fd_w = 16;
	smfdb.fd_wdwidth = 1;

	dmfdb.fd_addr = NULL;

	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = 15;
	pxy[4] = x;
	pxy[5] = y;
	pxy[6] = x + pxy[2];
	pxy[7] = y + pxy[3];

	vrt_cpyfm(cgl_vdi_handle, MD_ERASE, pxy, &smfdb, &dmfdb, ci);
	smfdb.fd_addr = data;
	vrt_cpyfm(cgl_vdi_handle, MD_TRANS, pxy, &smfdb, &dmfdb, ci);

	vswr_mode(cgl_vdi_handle, (IS_BG(pb->pb_tree[pb->pb_obj].ob_flags) &&
						   cgl_draw_3d) ? MD_TRANS : MD_REPLACE);
	set_textdef();
	prt_text((char *) pb->pb_parm, x + (5 * cgl_regular_font.fnt_chw) / 2, y,
			 pb->pb_currstate);

	cgl_clip_off();

	return 0;
}

static int ub_rectbut(PARMBLK *pb)
{
	int x = pb->pb_x, y = pb->pb_y, pxy[4];
	GRECT clip;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	pxy[0] = x + 2;				/* <- fixed I_A: effects TRISTATE_1 too! */
	pxy[1] = y + 2;				/* <- fixed I_A */
	pxy[2] = x + cgl_regular_font.fnt_chw * 2 - 2;
	pxy[3] = y + cgl_regular_font.fnt_chh - 2;

	vswr_mode(cgl_vdi_handle, MD_REPLACE);
	set_filldef(0);
	vr_recfl(cgl_vdi_handle, pxy);
	set_linedef(1);
	draw_rect(x + 1, y + 1, 2 * cgl_regular_font.fnt_chw - 1, cgl_regular_font.fnt_chh - 1);

	if (pb->pb_currstate & SELECTED)
	{
		pxy[0]--;
		pxy[1]--;
		pxy[2]++;
		pxy[3]++;
		v_pline(cgl_vdi_handle, 2, pxy);
		pxy[0] = pxy[2];
		pxy[2] = x + 1;
		v_pline(cgl_vdi_handle, 2, pxy);
	}

	vswr_mode(cgl_vdi_handle, (IS_BG(pb->pb_tree[pb->pb_obj].ob_flags) &&
						   cgl_draw_3d) ? MD_TRANS : MD_REPLACE);
	set_textdef();
	prt_text((char *) pb->pb_parm, x + 3 * cgl_regular_font.fnt_chw, y,
			 pb->pb_currstate);

	cgl_clip_off();

	return 0;
}

static int ub_rectbuttri(PARMBLK *pb)
{
	int x = pb->pb_x, y = pb->pb_y, pxy[4];
	int state;
	GRECT clip;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	pxy[0] = x + 2;				/* <- fixed I_A: effects TRISTATE_1 too! */
	pxy[1] = y + 2;				/* <- fixed I_A */
	pxy[2] = x + cgl_regular_font.fnt_chw * 2 - 2;
	pxy[3] = y + cgl_regular_font.fnt_chh - 2;

	vswr_mode(cgl_vdi_handle, MD_REPLACE);
	set_filldef(0);
	vr_recfl(cgl_vdi_handle, pxy);
	set_linedef(1);
	draw_rect(x + 1, y + 1, 2 * cgl_regular_font.fnt_chw - 1, cgl_regular_font.fnt_chh - 1);

	/* handle tri-state */

	state = pb->pb_tree[pb->pb_obj].ob_state;

	switch (cgl_get_tristate(state))
	{
	case TRISTATE_0:
		break;
	case TRISTATE_1:
		vsf_interior(cgl_vdi_handle, FIS_PATTERN);
		vsf_style(cgl_vdi_handle, 4);
		vsf_color(cgl_vdi_handle, 1);
		v_bar(cgl_vdi_handle, pxy);
		break;
	case TRISTATE_2:
		pxy[0]--;
		pxy[1]--;
		pxy[2]++;
		pxy[3]++;
		v_pline(cgl_vdi_handle, 2, pxy);
		pxy[0] = pxy[2];
		pxy[2] = x + 1;
		v_pline(cgl_vdi_handle, 2, pxy);
		break;
	}

	vswr_mode(cgl_vdi_handle, (IS_BG(pb->pb_tree[pb->pb_obj].ob_flags) &&
						   cgl_draw_3d) ? MD_TRANS : MD_REPLACE);
	set_textdef();
	prt_text((char *) pb->pb_parm, x + 3 * cgl_regular_font.fnt_chw, y, state);

	cgl_clip_off();

	return 0;
}

static int ub_cyclebut(PARMBLK *pb)
{
	int x = pb->pb_x, y = pb->pb_y, w = pb->pb_w, h = pb->pb_h;
	GRECT clip;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	vswr_mode(cgl_vdi_handle, MD_REPLACE);
	clr_object(&clip, 0);
	set_linedef(1);
	draw_rect(x, y, w, h);

	cgl_clip_off();

	return 0;
}

static int ub_button(PARMBLK *pb)
{
	int border, object = pb->pb_obj, pxy[8], flags, offset = 0;
	register int x, y, org_ob_type;
	register int button_3d, act_ind_sel = FALSE;
	register char *string;
	GRECT frame, clip;
	register OBJECT *tree = pb->pb_tree;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	border = cgld_bborder(tree, object);

	if (IS_XUSER(tree[object].ob_spec.userblk))
	{
		flags = ((XUSERBLK *)(pb->pb_parm))->ob_flags;
		string = (char *)((XUSERBLK *)(pb->pb_parm))->ob_spec;
		org_ob_type = ((XUSERBLK *)(pb->pb_parm))->ob_type & 0xFF;
	}
	else
	{
		flags = tree[object].ob_flags;
		string = (char *)pb->pb_parm;
		org_ob_type = G_BUTTON;
	}

	frame.g_x = pb->pb_x + border;
	frame.g_y = pb->pb_y + border;
	frame.g_w = pb->pb_w - 2 * border;
	frame.g_h = pb->pb_h - 2 * border;

	button_3d = ( (IS_ACT(flags) || IS_IND(flags)) && cgl_draw_3d );

	if(((pb->pb_currstate & SELECTED) == (pb->pb_prevstate & SELECTED)) ||
		 button_3d )
	{
		vswr_mode(cgl_vdi_handle, MD_REPLACE);
		set_linedef(1);

		if (org_ob_type != G_STRING)
		{
			if (button_3d)
			{
				if (pb->pb_currstate & SELECTED)
					act_ind_sel = TRUE;
				else if (IS_ACT(flags))
					offset = -1;

				clr_object(&frame, (IS_IND(flags) && act_ind_sel) 
									? 9 : get_3d_color(flags));

				pxy[0] = frame.g_x;
				pxy[1] = frame.g_y + frame.g_h - 2;
				pxy[2] = frame.g_x;
				pxy[3] = frame.g_y;
				pxy[4] = frame.g_x + frame.g_w - 2;
				pxy[5] = frame.g_y;

				vsl_color(cgl_vdi_handle, (act_ind_sel) ? 1 : 0);
				v_pline(cgl_vdi_handle, 3, pxy);

				pxy[0] = frame.g_x + 1;
				pxy[1] = frame.g_y + frame.g_h - 1;
				pxy[2] = frame.g_x + frame.g_w - 1;
				pxy[3] = frame.g_y + frame.g_h - 1;
				pxy[4] = frame.g_x + frame.g_w - 1;
				pxy[5] = frame.g_y + 1;

				if (act_ind_sel && IS_IND(flags))
					vsl_color(cgl_vdi_handle, (cgl_nplanes < 4) ? 0 : 8);
				else
					vsl_color(cgl_vdi_handle, (act_ind_sel) ? 0 : 9);
				v_pline(cgl_vdi_handle, 3, pxy);
			}
			else
				clr_object(&frame, 0);

			vsl_color(cgl_vdi_handle, 1);
			draw_frame(&frame, -1, -border);
		}

		if (org_ob_type != G_STRING)
		{
			x = pb->pb_x + (pb->pb_w - (int) cgld_strlen(string) * cgl_regular_font.fnt_chw) / 2 + offset;
			y = pb->pb_y + (pb->pb_h - cgl_regular_font.fnt_chh) / 2 + offset;
		}
		else
		{
			x = pb->pb_x;
			y = pb->pb_y;
		}

		vswr_mode(cgl_vdi_handle, MD_TRANS);
		set_textdef();
		if (act_ind_sel && IS_IND(flags))
			vst_color(cgl_vdi_handle, 0);
		prt_text(string, x, y, pb->pb_currstate);
	}
	else
		goto invert;

	if ((pb->pb_currstate & SELECTED) && !button_3d)
	{
	  invert:
		vswr_mode(cgl_vdi_handle, MD_XOR);
		set_filldef(1);
		cgl_rect2pxy(&frame, pxy);
		vr_recfl(cgl_vdi_handle, pxy);
	}

	cgl_clip_off();

	return 0;
}

static int ub_rbutpar(PARMBLK *pb)
{
	int x = pb->pb_x, y = pb->pb_y, w = pb->pb_w, h = pb->pb_h;
	int d = cgl_regular_font.fnt_chh / 2, pxy[12], ext[8];
	GRECT clip, frame;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	frame.g_x = pb->pb_x;
	frame.g_y = pb->pb_y;
	frame.g_w = pb->pb_w;
	frame.g_h = pb->pb_h;

	cgl_clip_on(&clip);

	vswr_mode(cgl_vdi_handle, MD_REPLACE);
	set_linedef(1);
	set_textdef();

	if (!(IS_BG(pb->pb_tree[pb->pb_obj].ob_flags) && cgl_draw_3d))
		clr_object(&frame, 0);

	vqt_extent(cgl_vdi_handle, (char *) pb->pb_parm, ext);

	pxy[0] = x + cgl_regular_font.fnt_chw - 2;
	pxy[1] = y + d;
	pxy[2] = x;
	pxy[3] = pxy[1];
	pxy[4] = x;
	pxy[5] = y + h - 1;
	pxy[6] = x + w - 1;
	pxy[7] = pxy[5];
	pxy[8] = pxy[6];
	pxy[9] = pxy[1];
	pxy[10] = x + cgl_regular_font.fnt_chw + ext[2] - ext[0] + 1;
	pxy[11] = pxy[1];

	v_pline(cgl_vdi_handle, 6, pxy);
	if (IS_BG(pb->pb_tree[pb->pb_obj].ob_flags) && cgl_draw_3d)
		vswr_mode(cgl_vdi_handle, MD_TRANS);
	prt_text((char *) pb->pb_parm, x + cgl_regular_font.fnt_chw, y, pb->pb_currstate);

	cgl_clip_off();

	return 0;
}

static int ub_title(PARMBLK *pb)
{
	int x = pb->pb_x, y = pb->pb_y, w = pb->pb_w, h = pb->pb_h;
	int pxy[4];
	GRECT clip;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	pxy[0] = x;
	pxy[1] = y + h - 1;
	pxy[2] = x + w - 1;
	pxy[3] = pxy[1];

	vswr_mode(cgl_vdi_handle, MD_REPLACE);
	set_linedef(1);
	v_pline(cgl_vdi_handle, 2, pxy);

	if (IS_BG(pb->pb_tree[pb->pb_obj].ob_flags) && cgl_draw_3d)
		vswr_mode(cgl_vdi_handle, MD_TRANS);

	set_textdef();
	prt_text((char *) pb->pb_parm, x, y + (h - cgl_regular_font.fnt_chh - 1) / 2,
			 pb->pb_currstate);

	cgl_clip_off();

	return 0;
}

static int ub_unknown(PARMBLK *pb)
{
	int x = pb->pb_x, y = pb->pb_y, w = pb->pb_w, h = pb->pb_h;
	GRECT clip;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	vswr_mode(cgl_vdi_handle, MD_REPLACE);
	clr_object((GRECT *) & pb->pb_x, 0);
	set_linedef(1);
	draw_rect(x, y, w, h);

	cgl_clip_off();

	return 0;
}

/********************************************************************
 *																	*
 * Functions for installing the user-defined objects in an object	*
 * tree.															*
 *																	*
 ********************************************************************/

/*
 * static void cgld_translate(OBJECT *tree, int parent, int offset)
 *
 * Translate all childs of an object.
 */

static void cgld_translate(OBJECT *tree, int parent, int offset)
{
	int i = tree[parent].ob_head;

	while ((i >= 0) && (i != parent))
	{
		tree[i].ob_y += offset;
		i = tree[i].ob_next;
	}
}

/*
 * Function for counting user-defined objects.
 *
 * Parameters:
 *
 * tree		- Pointer to the object tree.
 * n		- Pointer to a variable in which the number of
 *			  user-defined objects is placed.
 * nx		- Pointer to a variable in which the number of
 *			  extended user-defined objects is placed.
 *
 * Result	: Total number of user-defined objects (*n + *nx).
 */

static int cnt_user(OBJECT *tree, int *n, int *nx)
{
	OBJECT *object = tree;
	int etype;

	*n = 0;
	*nx = 0;

	for (;;)
	{
		etype = (object->ob_type >> 8) & 0xFF;

		if (cgld_is_xtndelement(etype) && ((object->ob_type & 0xFF) != G_USERDEF))
		{
			switch(etype)
			{
			case CGLD_BUTTON :
				if ((GET_3D(object->ob_flags) && !IS_BG(object->ob_flags)) ||
						((object->ob_type & 0xFF) == G_STRING))
					(*nx)++;
				else
					(*n)++;
				break;
			default :
				(*n)++;
				break;
			}
		}

		if (object->ob_flags & LASTOB)
			return (*n + *nx);

		object++;
	}
}

/*
 * void cgld_set_userobjects(OBJECT *tree)
 *
 * Make all objects in an object tree with an extended type known by
 * the library user-defined.
 */

void cgld_set_userobjects(OBJECT *tree)
{
	int etype, n, nx, d, object = 0;
	OBJECT *c_obj;
	USERBLK *c_ub;
	CGLOBJDATA *data;
	int (*c_code) (PARMBLK *parmblock);
	int xuserblk;

	if (cnt_user(tree, &n, &nx) == 0)
		return;

	if ((data = (CGLOBJDATA *) malloc(sizeof(CGLOBJDATA) + sizeof(USERBLK) * (long) n +
									 sizeof(XUSERBLK) * (long) nx)) == NULL)
		return;

	data->next = cgl_objdata;
	cgl_objdata = data;
	c_ub = (USERBLK *) &data[1];

	for (;;)
	{
		c_obj = tree + object;
		xuserblk = FALSE;
		etype = (c_obj->ob_type >> 8) & 0xFF;

		if (cgld_is_xtndelement(etype) && ((c_obj->ob_type & 0xFF) != G_USERDEF))
		{
			switch (etype)
			{
			case CGLD_DRAGBOX :
				c_code = ub_drag;
				break;
			case CGLD_ROUNDRB :
				c_code = ub_roundrb;
				break;
			case CGLD_RECTBUT :
				c_code = ub_rectbut;
				break;
			case CGLD_RECTBUTTRI :
				c_code = ub_rectbuttri;
				break;
			case CGLD_CYCLBUT :
				c_code = ub_cyclebut;
				break;
			case CGLD_BUTTON :
				if ((c_obj->ob_type & 0xFF) == G_STRING)
				{
					xuserblk = TRUE;
					d = 0;
				}
				else
				{
					d = cgld_bborder(tree, object);

					if (GET_3D(c_obj->ob_flags) && !IS_BG(c_obj->ob_flags))
					{
						xuserblk = TRUE;
						if (cgl_draw_3d)
							d += 2;
					}
				}

				c_obj->ob_x -= d;
				c_obj->ob_y -= d;
				c_obj->ob_width += 2 * d;
				c_obj->ob_height += 2 * d;

				c_code = ub_button;
				break;
			case CGLD_RBUTPAR :
				d = cgl_regular_font.fnt_chh / 2;
				c_obj->ob_y -= d;
				c_obj->ob_height += d;
				cgld_translate(tree, object, d);
				c_code = ub_rbutpar;
				break;
			case CGLD_TITLE :
				c_code = ub_title;
				c_obj->ob_height += 1;
				break;
			default :
				/* yet unknown userdef! */
				c_code = ub_unknown;
				break;
			}

			if (xuserblk)
			{
				XUSERBLK *c_xub = (XUSERBLK *)c_ub;

				cgl_xuserdef(c_obj,  c_xub, c_code);
				c_ub = (USERBLK *)(c_xub + 1);
			}
			else
			{
				cgl_userdef(c_obj, c_ub, c_code);
				c_ub++;
			}
		}

		if (c_obj->ob_flags & LASTOB)
			return;

		object++;
	}
}

/*
 * int cgld_gaddr(int type, int index, void *addr)
 *
 * Replacement rsrc_gaddr function.
 */

int cgld_gaddr(int type, int index, void *addr)
{
	int result;

	if (((result = rsrc_gaddr(type, index, addr)) != 0) && (type == R_TREE))
		cgld_set_userobjects(*(OBJECT **) addr);

	return result;
}

void cgld_fixtree(OBJECT *tree)
{
	int i = 0;

	for (;;)
	{
		rsrc_obfix(tree, i);
		if (tree[i].ob_flags & LASTOB)
			break;
		i++;
	}

	cgld_set_userobjects(tree);
}

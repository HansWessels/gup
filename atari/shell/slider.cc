
#include <stddef.h>
#include <osbind.h>
#include <vdibind.h>

#include "cgl_dial.h"
#include "cgl_evnt.h"
#include "cgl_util.h"
#include "slider.h"

#define AES3D_1			0x200
#define AES3D_2			0x400

#define FIS_HOLLOW		0
#define FIS_SOLID		1
#define FIS_PATTERN		2
#define FIS_HATCH		3
#define FIS_USER		4

#define FALSE			0
#define TRUE			1

template<class T> inline T min(T x, T y)	{ return (x < y) ? x : y; }
template<class T> inline T max(T x, T y)	{ return (x > y) ? x : y; }

/*
 * Internal variabels the Xdialog library.
 */

typedef struct
{
	int fnt_id;
	int fnt_type;
	int fnt_height;
	int fnt_chw;
	int fnt_chh;
} CGL_FONT;

extern int cgl_vdi_handle;
extern int cgl_nplanes;
extern int cgl_draw_3d;
extern int cgl_ncolors;

extern CGL_FONT cgl_regular_font;

static int cgld_bg_col = 8;
static int cgld_ind_col = 8;
static int cgld_act_col = 8;

/********************************************************************
 *																	*
 * Utility functions.												*
 *																	*
 ********************************************************************/

static void clr_object(GRECT *r, int color)
{
	int pxy[4];

	cgl_rect2pxy(r, pxy);
	vsf_color(cgl_vdi_handle, color);
	vsf_interior(cgl_vdi_handle, FIS_SOLID);
	vsf_perimeter(cgl_vdi_handle, 0);
	vr_recfl(cgl_vdi_handle, pxy);
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

inline int GET_3D(int flags)	\
	{ return (flags & (AES3D_1 | AES3D_2)); }
inline int IS_ACT(int flags)	\
	{ return (GET_3D(flags) == (AES3D_1 | AES3D_2)); }
inline int IS_IND(int flags)	\
	{ return (GET_3D(flags) == AES3D_1); }
inline int IS_BG(int flags)		\
	{ return (GET_3D(flags) == AES3D_2); }

static int get_3d_color(int flags)
{
	register int color;

	if (IS_ACT(flags))
		color = cgld_act_col;
	else if (IS_IND(flags))
		color = cgld_ind_col;
	else if (IS_BG(flags))
		color = cgld_bg_col;
	else
		color = 0;

	return (color >= cgl_ncolors) ? 0 : color;
}

static int ub_listbox(PARMBLK *pb)
{
	((SL_USERBLK *)(pb->pb_parm))->lb->ub_listbox(pb);
}

static void object_offset(OBJECT *tree, int object, int *x, int *y)
{
	*x = 0;
	*y = 0;

	do
	{
		*x += tree[object].ob_x;
		*y += tree[object].ob_y;
	} while((object = cgl_obj_parent(tree, object)) >= 0);
}

/********************************************************************
 *																	*
 * Constructor and destructor for the 'listbox' class.				*
 *																	*
 ********************************************************************/

listbox::listbox(void)
{
	type = SL_DISABLED;
	dialog = NULL;

	nlines = 0;
	cur_line = 0;
	ncolumns = 0;
	cur_column = 0;
}

listbox::~listbox(void)
{
	register OBJECT *parent = &(dialog->get_tree())[object_parent];

	/*
	 * Restore original size and position of the list box.
	 */

	parent->ob_x++;
	parent->ob_y++;
	parent->ob_width--;
	parent->ob_height--;
}

/********************************************************************
 *																	*
 * Private member functions of the 'listbox' class.					*
 *																	*
 ********************************************************************/

/*
 * void listbox::draw_line(int line, int firstcol, int lastcol)
 *
 * Draw the line given by 'line' in the list box, starting
 * in column 'firstcol' and ending in column 'lastcol'.
 *
 * Parameters:
 *
 * line			- line to draw.
 * firstcol	- first column to draw.
 * lastcol	- last column to draw.
 */

void listbox::draw_line(int line, int firstcol, int lastcol)
{
	register int x, y, l, last_column;
	static char str[SL_MAX_ITM_LEN + 1];
	register OBJECT *tree = dialog->get_tree();

	get_line(line, str);

	last_column = cur_column + min((tree[object_parent].ob_width - 1) / cgl_regular_font.fnt_chw, lastcol + 1);

	if ((l = strlen(str)) < last_column)
	{
		register int i;

		for (i = l; i < last_column; i++)
			str[i] = ' ';
	}

	str[last_column] = 0;

	object_offset(tree, object_parent, &x, &y);
	y += (line - cur_line) * cgl_regular_font.fnt_chh + 1;
	x += firstcol * cgl_regular_font.fnt_chw + 1;

	vswr_mode(cgl_vdi_handle, ((IS_ACT(userblk.ob_flags) || IS_IND(userblk.ob_flags))
							&& cgl_draw_3d) ? MD_TRANS : MD_REPLACE);

	v_gtext(cgl_vdi_handle, x, y, &str[cur_column + firstcol]);

	if (is_selected(line))
	{
		int pxy[4];

		pxy[0] = x;
		pxy[1] = y;
		pxy[2] = x + cgl_regular_font.fnt_chw * (lastcol - firstcol + 1) - 1;
		pxy[3] = y + cgl_regular_font.fnt_chh - 1;

		vswr_mode(cgl_vdi_handle, MD_XOR);
		vr_recfl(cgl_vdi_handle, pxy);
	}
}

/*
 * void listbox::scroll(scr_type type)
 *
 * Scroll the text in the listbox in the direction given by 'type'.
 */

void listbox::scroll(scr_type type)
{
	GRECT r;
	register int pixels, hv;

	cgl_objrect(dialog->get_tree(), object_parent, &r);

	r.g_x++;
	r.g_y++;
	r.g_w--;
	r.g_h--;

	if ((type == SCR_UP) || (type == SCR_DOWN))
	{
		hv = 1;
		pixels = (type == SCR_UP) ? cgl_regular_font.fnt_chh : -cgl_regular_font.fnt_chh;
	}
	else
	{
		hv = 0;
		pixels = (type == SCR_LEFT) ? cgl_regular_font.fnt_chw : -cgl_regular_font.fnt_chw;
	}

	dialog->cw_scroll(1, &r, pixels, hv, object_parent, 1);
}

void listbox::varrows(int button)
{
	register int redraw, first = 1;
	register int mstate;

	cgl_update(BEG_UPDATE);

	dialog->change_object(button, SELECTED);

	do
	{
		redraw = 0;

		if (button == up_arrow)
		{
			if (cur_line > 0)
			{
				cur_line--;
				redraw = 1;
				set_vslider(TRUE);
			}
		}
		else
		{
			if (cur_line < (nlines - lines))
			{
				cur_line++;
				redraw = 1;
				set_vslider(TRUE);
			}
		}

		if (redraw == 1)
			scroll((button == up_arrow) ? SCR_UP : SCR_DOWN);

		mstate = cgl_button_state() & 1;

		if ((first == 1) && mstate)
		{
			evnt_timer(250);
			first = 0;
		}
	}
	while (mstate);

	dialog->change_object(button, NORMAL);

	cgl_update(END_UPDATE);
}

void listbox::harrows(int button)
{
	register int redraw, first = 1;
	register int mstate;

	cgl_update(BEG_UPDATE);

	dialog->change_object(button, SELECTED);

	do
	{
		redraw = 0;

		if (button == left_arrow)
		{
			if (cur_column > 0)
			{
				cur_column--;
				redraw = 1;
				set_hslider(TRUE);
			}
		}
		else
		{
			if (cur_column < (ncolumns - columns))
			{
				cur_column++;
				redraw = 1;
				set_hslider(TRUE);
			}
		}

		if (redraw == 1)
			scroll((button == left_arrow) ? SCR_LEFT : SCR_RIGHT);

		mstate = cgl_button_state() & 1;

		if ((first == 1) && mstate)
		{
			evnt_timer(250);
			first = 0;
		}
	}
	while (mstate);

	dialog->change_object(button, NORMAL);

	cgl_update(END_UPDATE);
}

/*
 * void listbox::set_vslider(int draw)
 *
 * Set the position and size of the vertical slider bar.
 */

void listbox::set_vslider(int draw)
{
	register int sh, s;
	register OBJECT *tree = dialog->get_tree();

	cur_line = ((nlines < lines) || (cur_line < 0)) ? 0 : min(cur_line, nlines - lines);

	if (nlines > lines)
	{
		sh = (int) (((long) lines * (long) tree[vert_sparent].ob_height) / (long) nlines);
		if (sh < cgl_regular_font.fnt_chh)
			sh = cgl_regular_font.fnt_chh;
	}
	else
		sh = tree[vert_sparent].ob_height;

	tree[vert_slider].ob_height = sh;

	s = nlines - lines;
	tree[vert_slider].ob_y = (s > 0) ? (int) (((long) (tree[vert_sparent].ob_height - sh) * (long) cur_line) / (long) s) : 0;

	if (draw)
		dialog->draw_object(vert_sparent, MAX_DEPTH);
}

/*
 * void listbox::set_hslider(int draw)
 *
 * Set the position and size of the horizontal slider bar.
 */

void listbox::set_hslider(int draw)
{
	register int sh, s;
	register OBJECT *tree = dialog->get_tree();

	cur_column = ((ncolumns < columns) || (cur_column < 0)) ? 0 : min(cur_column, ncolumns - columns);

	if (ncolumns > columns)
	{
		sh = (int) (((long) columns * (long) tree[horz_sparent].ob_width) / (long) ncolumns);
		if (sh < cgl_regular_font.fnt_chw)
			sh = cgl_regular_font.fnt_chw;
	}
	else
		sh = tree[horz_sparent].ob_width;

	tree[horz_slider].ob_width = sh;

	s = ncolumns - columns;
	tree[horz_slider].ob_x = (s > 0) ? (int) (((long) (tree[horz_sparent].ob_width - sh) * (long) cur_column) / (long) s) : 0;

	if (draw)
		dialog->draw_object(horz_sparent, MAX_DEPTH);
}

void listbox::vbar(void)
{
	int my, oy, dummy;
	register int old, max;

	graf_mkstate(&dummy, &my, &dummy, &dummy);
	objc_offset(dialog->get_tree(), vert_slider, &dummy, &oy);

	cgl_update(BEG_UPDATE);

	do
	{
		old = cur_line;

		if (my < oy)
		{
			cur_line -= lines;
			if (cur_line < 0)
				cur_line = 0;
		}
		else
		{
			cur_line += lines;
			max = nlines - lines;
			if (cur_line > max)
				cur_line = max;
		}

		if (cur_line != old)
		{
			set_vslider(TRUE);
			dialog->draw_object(object_parent, 1);
		}
	}
	while (cgl_button_state() & 0x1);

	cgl_update(END_UPDATE);
}

void listbox::hbar(void)
{
	int mx, ox, dummy;
	register int old, max;

	graf_mkstate(&mx, &dummy, &dummy, &dummy);
	objc_offset(dialog->get_tree(), horz_slider, &ox, &dummy);

	cgl_update(BEG_UPDATE);

	do
	{
		old = cur_column;

		if (mx < ox)
		{
			cur_column -= columns;
			if (cur_column < 0)
				cur_column = 0;
		}
		else
		{
			cur_column += columns;
			max = ncolumns - columns;
			if (cur_column > max)
				cur_column = max;
		}

		if (cur_column != old)
		{
			set_hslider(TRUE);
			dialog->draw_object(object_parent, 1);
		}
	}
	while (cgl_button_state() & 0x1);

	cgl_update(END_UPDATE);
}

void listbox::vslider(void)
{
	register int newpos;
	register long l;

	cgl_update(BEG_MCTRL);
	newpos = graf_slidebox(dialog->get_tree(), vert_sparent, vert_slider, 1);
	cgl_update(END_MCTRL);

	l = (long) (nlines - lines);
	cur_line = (int) (((long) newpos * l + 500L) / 1000L);
	set_vslider(TRUE);
	dialog->draw_object(object_parent, 1);
}

void listbox::hslider(void)
{
	register int newpos;
	register long c;

	cgl_update(BEG_MCTRL);
	newpos = graf_slidebox(dialog->get_tree(), horz_sparent, horz_slider, 0);
	cgl_update(END_MCTRL);

	c = (long) (ncolumns - columns);
	cur_column = (int) (((long) newpos * c + 500L) / 1000L);
	set_hslider(TRUE);
	dialog->draw_object(object_parent, 1);
}

/*
 * int listbox::handle_select(int button, int x, int y)
 *
 * Check if the button is associated with this listbox. Handle the
 * button if TRUE and return 1, otherwise return 0.
 */

int listbox::handle_select(int button, int x, int y)
{
	register int dc;

	dc = button & 0x8000;			/* Double click flag. */
	button &= 0x7FFF;

	if ((type == SL_HORZ) || (type == SL_HV))
	{
		if ((button == left_arrow) || (button == right_arrow))
		{
			harrows(button);
			return 1;
		}
		else if (button == horz_slider)
		{
			hslider();
			return 1;
		}
		else if (button == horz_sparent)
		{
			hbar();
			return 1;
		}
	}

	if ((type == SL_VERT) || (type == SL_HV))
	{
		if ((button == up_arrow) || (button == down_arrow))
		{
			varrows(button);
			return 1;
		}
		else if (button == vert_slider)
		{
			vslider();
			return 1;
		}
		else if (button == vert_sparent)
		{
			vbar();
			return 1;
		}
	}

	if (button == object_parent)
	{
		register int line = find_line(y);;

		if (line >= 0)
		{
			if (dc)
			{
				cgl_update(END_UPDATE);
				handle_open(line);
				cgl_update(BEG_UPDATE);
			}
			else
			{
				int mx, my, kstate, dummy, min_y;
				register int new_line, state, max_y;
				register OBJECT *tree = dialog->get_tree();

				object_offset(tree, object_parent, &dummy, &min_y);
				min_y++;
				max_y = min_y + tree[object_parent].ob_height - 2;

				handle_select(line);
				state = is_selected(line);

				cgl_update(BEG_UPDATE);

				while (!cgl_mouse_event(0, &mx, &my, &kstate))
				{
					register int i, new_state, inc;

					if (my < min_y)
					{
						if (cur_line > 0)
						{
							cur_line--;
							set_vslider(TRUE);
							scroll(SCR_UP);
						}
						new_line = cur_line;
					}
					else if (my > max_y)
					{
						if (cur_line < (nlines - lines))
						{
							cur_line++;
							set_vslider(TRUE);
							scroll(SCR_DOWN);
						}
						new_line = min(nlines - 1, cur_line + lines - 1);
					}
					else
					{
						if ((new_line = find_line(my)) < 0)
							new_line = min(nlines - 1, cur_line + lines - 1);
					}

					if (new_line != line)
					{
						inc = (new_line > line) ? 1 : -1;
						i = line;

						do
						{
							i += inc;
							new_state = is_selected(i);
							if (new_state != state)
								handle_select(i);
						} while (i != new_line);

						line = new_line;
					}
				}

				cgl_update(END_UPDATE);
			}
		}

		return 1;
	}

	return 0;
}

/*
 * int listbox::find_line(int y)
 *
 * Return the line number which is on the y coordinate 'y', or
 * -1, if there is no line on that coordinate.
 */

int listbox::find_line(int y)
{
	register int line;
	int sl_y, dummy;

	object_offset(dialog->get_tree(), object_parent, &dummy, &sl_y);
	sl_y++;
	line = cur_line + (y - sl_y) / cgl_regular_font.fnt_chh;

	if ((line < 0) || (line >= nlines))
		return -1;
	else
		return line;
}

/********************************************************************
 *																	*
 * Public member functions of the 'listbox' class.					*
 *																	*
 ********************************************************************/

void listbox::init_listbox(slider_type sltype, int parent, int larrow,
						   int rarrow, int slider, int sparent)
{
	if (sltype == SL_HORZ)
		init_listbox(sltype, parent, larrow, rarrow, slider, sparent,
					 0, 0, 0, 0);
	else
		init_listbox(sltype, parent, 0, 0, 0, 0, larrow, rarrow, slider,
					 sparent);
}

/*
 * void listbox::init_listbox(slider_type sltype, int parent, int larrow,
 *							  int rarrow, int hslider, int hsparent,
 *							  int uarrow, int darrow, int vslider,
 *							  int vsparent)
 *
 * Initiate the listbox.
 */

void listbox::init_listbox(slider_type sltype, int parent, int larrow,
						   int rarrow, int hslider, int hsparent,
						   int uarrow, int darrow, int vslider,
						   int vsparent)
{
	register OBJECT *sltree = dialog->get_tree();
	register OBJECT *object = &sltree[parent];

	/*
	 * Convert the parent to a user-defined object.
	 */

	userblk.ub_code = ::ub_listbox;
	userblk.ub_parm = &userblk;
	userblk.ob_type = object->ob_type;
	userblk.ob_flags = object->ob_flags;
	userblk.ob_spec = object->ob_spec;
	userblk.lb = this;

	object->ob_type = (object->ob_type & 0xFF00) | G_USERDEF;
	object->ob_flags &= ~(AES3D_1 | AES3D_2);
	object->ob_spec = (long) &userblk;

	type = sltype;				/* Slider type. */
	object_parent = parent;		/* Parent object of strings. */

	if ((type == SL_VERT) || (type == SL_HV))
	{
		register int x;

		x = sltree[parent].ob_x + sltree[parent].ob_width + 1;

		sltree[uarrow].ob_x = x;
		sltree[darrow].ob_x = x;

		sltree[vsparent].ob_x = x;
		sltree[vsparent].ob_y = sltree[uarrow].ob_y + sltree[uarrow].ob_height + 1;
		sltree[vsparent].ob_height = sltree[darrow].ob_y - sltree[vsparent].ob_y - 1;
	}

	if ((type == SL_HORZ) || (type == SL_HV))
	{
		register int y;

		y = sltree[parent].ob_y + sltree[parent].ob_height + 1;

		sltree[larrow].ob_y = y;
		sltree[rarrow].ob_y = y;

		sltree[hsparent].ob_y = y;
		sltree[hsparent].ob_x = sltree[larrow].ob_x + sltree[larrow].ob_width + 1;
		sltree[hsparent].ob_width = sltree[rarrow].ob_x - sltree[hsparent].ob_x - 1;
	}

	sltree[parent].ob_x--;
	sltree[parent].ob_y--;
	sltree[parent].ob_width++;
	sltree[parent].ob_height++;

	/*
	 * Calculate number of lines and columns.
	 */

	lines = sltree[object_parent].ob_height / cgl_regular_font.fnt_chh;
	columns = sltree[object_parent].ob_width / cgl_regular_font.fnt_chw;

	/*
	 * Vertical slider.
	 */

	up_arrow = uarrow;			/* Up arrow. */
	down_arrow = darrow;		/* Down arrow. */
	vert_slider = vslider;		/* Vertical slider. */
	vert_sparent = vsparent;	/* Vertical slider parent. */

	/*
	 * Horizontal slider.
	 */

	left_arrow = larrow;		/* Left arrow. */
	right_arrow = rarrow;		/* Right arrow. */
	horz_slider = hslider;		/* Horizontal slider. */
	horz_sparent = hsparent;	/* Horizontal slider parent. */
}

/*
 * void listbox::init_vslider(int nl, int cl, int draw)
 *
 * Set the number of lines and the current line of the
 * vertical slider.
 */

void listbox::init_vslider(int nl, int cl, int draw)
{
	nlines = nl;
	cur_line = cl;

	if ((type == SL_VERT) || (type == SL_HV))
		set_vslider(draw);
}

/*
 * void listbox::init_hslider(int nc, int cc, int draw)
 *
 * Set the number of columns and the current column of the
 * horizontal slider.
 */

void listbox::init_hslider(int nc, int cc, int draw)
{
	ncolumns = nc;
	cur_column = cc;

	if ((type == SL_HORZ) || (type == SL_HV))
		set_hslider(draw);
}

/*
 * int listbox::ub_listbox(PARMBLK *pb)
 *
 * This function draws the list box. 
 */

int listbox::ub_listbox(PARMBLK *pb)
{
	register int i, firstline, lastline, firstcol, lastcol;
	int dummy, button_3d;
	GRECT frame, clip, area;

	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;

	cgl_clip_on(&clip);

	frame.g_x = pb->pb_x;
	frame.g_y = pb->pb_y;
	frame.g_w = pb->pb_w;
	frame.g_h = pb->pb_h;

	button_3d = ((IS_ACT(userblk.ob_flags) || IS_IND(userblk.ob_flags)) && cgl_draw_3d);

	vswr_mode(cgl_vdi_handle, MD_REPLACE);

	if (button_3d)
		clr_object(&frame, (IS_IND(userblk.ob_flags)) ? 9 : get_3d_color(userblk.ob_flags));

	vsl_ends(cgl_vdi_handle, 0, 0);
	vsl_type(cgl_vdi_handle, 1);
	vsl_width(cgl_vdi_handle, 1);
	vsl_color(cgl_vdi_handle, 1);

	draw_rect(frame.g_x, frame.g_y, frame.g_w, frame.g_h);

	area.g_x = pb->pb_x + 1;
	area.g_y = pb->pb_y + 1;
	area.g_w = pb->pb_w - 1;
	area.g_h = pb->pb_h - 1;

	if (cgl_rcintersect(&area, &clip, &area))
	{
		vst_font(cgl_vdi_handle, cgl_regular_font.fnt_id);
		vst_rotation(cgl_vdi_handle, 0);
		vst_alignment(cgl_vdi_handle, 0, 5, &dummy, &dummy);
		vst_point(cgl_vdi_handle, cgl_regular_font.fnt_height, &dummy, &dummy, &dummy, &dummy);
		vst_color(cgl_vdi_handle, 1);
		vst_effects(cgl_vdi_handle, 0);

		vsf_color(cgl_vdi_handle, 1);
		vsf_interior(cgl_vdi_handle, FIS_SOLID);
		vsf_perimeter(cgl_vdi_handle, 0);

		firstline = (area.g_y - pb->pb_y - 1) / cgl_regular_font.fnt_chh;
		lastline = min((area.g_y + area.g_h - pb->pb_y - 2) / cgl_regular_font.fnt_chh, min(lines, nlines) - 1);
	
		firstcol = (area.g_x - pb->pb_x - 1) / cgl_regular_font.fnt_chw;
		lastcol = (area.g_x + area.g_w - pb->pb_x - 2) / cgl_regular_font.fnt_chw;

		for (i = firstline; i <= lastline; i++)
			draw_line(cur_line + i, firstcol, lastcol);

		if ((nlines < lines) && !button_3d)
		{
			frame.g_x = pb->pb_x + 1;
			frame.g_y = pb->pb_y + nlines * cgl_regular_font.fnt_chh + 1;
			frame.g_w = pb->pb_w - 1;
			frame.g_h = pb->pb_h - nlines * cgl_regular_font.fnt_chh - 1;

			if (cgl_rcintersect(&area, &frame, &frame))
			{
				vswr_mode(cgl_vdi_handle, MD_REPLACE);
				clr_object(&frame, 0);
			}
		}
	}

	cgl_clip_off();

	return 0;
}

/*
 * void listbox::invert_line(int line)
 *
 * Invert a line in the listbox.
 */

void listbox::invert_line(int line)
{
	register int wx, wy;
	GRECT work, r, in;
	register OBJECT *tree = dialog->get_tree();

	vsf_color(cgl_vdi_handle, 1);
	vsf_interior(cgl_vdi_handle, FIS_SOLID);
	vsf_perimeter(cgl_vdi_handle, 0);

	vswr_mode(cgl_vdi_handle, MD_XOR);

	objc_offset(tree, object_parent, &work.g_x, &work.g_y);
	work.g_x++;
	work.g_y += (line - cur_line) * cgl_regular_font.fnt_chh + 1;
	work.g_w = tree[object_parent].ob_width - 1;
	work.g_h = cgl_regular_font.fnt_chh;

	wx = work.g_x;
	wy = work.g_y;

	if ((line < cur_line) || (line >= (cur_line + lines)) || (line >= nlines))
		return;					/* Return if the line is not visible. */

	if (dialog->get_dial_mode() == CGLD_WINDOW)
	{
		/*
		 * Dialog is in a window. Invert the line using the
		 * window rectangle list.
		 */

		cgl_update(BEG_UPDATE);
		cgl_mouse_off();

		dialog->cw_get(WF_FIRSTXYWH, &r);

		while ((r.g_w != 0) && (r.g_h != 0))
		{
			if (cgl_rcintersect(&r, &work, &in) == TRUE)
			{
				int pxy[4];

				cgl_clip_on(&in);

				pxy[0] = in.g_x;
				pxy[1] = in.g_y;
				pxy[2] = in.g_x + in.g_w - 1;
				pxy[3] = in.g_y + in.g_h - 1;

				vr_recfl(cgl_vdi_handle, pxy);

				cgl_clip_off();
			}

			dialog->cw_get(WF_NEXTXYWH, &r);
		}

		cgl_mouse_on();
		cgl_update(END_UPDATE);
	}
	else
	{
		/*
		 * Dialog is not in a window. Invert the line.
		 */
	}
}

/*
 * void listbox::get_line(int line, char *string)
 *
 * This function is called when line 'line' in the listbox is
 * printed.
 */

void listbox::get_line(int line, char *string)
{
	*string = 0;
}

/*
 * int listbox::is_selected(int line)
 *
 * This function is called when line 'line' in the listbox is
 * printed.
 */

int listbox::is_selected(int line)
{
	return FALSE;
}

/*
 * void listbox::handle_open(int line)
 *
 * This function is called when a user double-clicks on an item
 * in the listbox.
 */

void listbox::handle_open(int line)
{
}

/*
 * void listbox::handle_select(int line)
 *
 * This function is called when a user single-clicks on an item
 * in the listbox.
 */

void listbox::handle_select(int line)
{
}

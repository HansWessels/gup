
#ifndef __SLIDER_H
#define __SLIDER_H

#define SL_MAX_ITM_LEN	511

typedef enum { SL_DISABLED, SL_HORZ, SL_VERT, SL_HV } slider_type;

typedef struct sl_userblk
{
	int (*ub_code)(PARMBLK *parmblock);
	struct sl_userblk *ub_parm;						/* Pointer to itself. */
	int ob_type;													/* Original object type. */
	int ob_flags;													/* Original object flags. */
	long ob_spec;													/* Original object specifier. */
	class listbox *lb;										/* Pointer to listbox. */
} SL_USERBLK;

class listbox : public cgl_object
{
 protected:
	typedef enum { SCR_LEFT, SCR_RIGHT, SCR_UP, SCR_DOWN } scr_type;

	slider_type type;											/* Slider type. */
	SL_USERBLK userblk;										/* USERBLK structure. */
	int object_parent;										/* Parent object of strings. */

	/*
	 * Variabels of vertical slider.
	 */

	int up_arrow;													/* Up arrow. */
	int down_arrow;												/* Down arrow. */
	int vert_slider;											/* Vertical slider. */
	int vert_sparent;											/* Vertical slider parent. */
	int lines;														/* Number of visible lines. */
	int nlines;														/* Total number of lines. */
	int cur_line;													/* Current line (0 - (nlines - lines)). */

	/*
	 * Variabels of horizontal slider.
	 */

	int left_arrow;												/* Left arrow. */
	int right_arrow;											/* Right arrow. */
	int horz_slider;											/* Horizontal slider. */
	int horz_sparent;											/* Horizontal slider parent. */
	int columns;													/* Number of visible columns. */
	int ncolumns;													/* Total number of columns. */
	int cur_column;												/* Current line (0 - (ncolumns - columns)). */

	void draw_line(int line, int firstcol, int lastcol);
	void scroll(scr_type type);

	void varrows(int button);
	void harrows(int button);
	void vslider(void);
	void hslider(void);
	void vbar(void);
	void hbar(void);

	virtual int handle_select(int button, int x, int y);

	void set_vslider(int draw);
	void set_hslider(int draw);

	int find_line(int y);

 public:
	/*
	 * Constructors and destructors.
	 */
  	 
	listbox(void);
	virtual ~listbox(void);

	/*
	 * Public member functions.
	 */

	void init_listbox(slider_type type, int parent, int larrow,
										int rarrow, int slider, int sparent);
	void init_listbox(slider_type type, int parent, int larrow,
										int rarrow, int hslider, int hsparent,
										int uarrow, int darrow, int vslider, int vsparent);
	void init_vslider(int nlines, int cur_line, int draw);
	void init_hslider(int ncolumns, int cur_column, int draw);

	int ub_listbox(PARMBLK *pb);
	void invert_line(int line);

	virtual void get_line(int line, char *string);
	virtual int is_selected(int line);
	virtual void handle_open(int line);
	virtual void handle_select(int line);
};

#endif	/* __SLIDER_H */

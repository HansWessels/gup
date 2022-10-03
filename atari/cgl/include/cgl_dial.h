
#ifndef __CGL_DIAL_H
#define __CGL_DIAL_H

#include "cgl_wind.h"

#define CGLD_MAXKEYS	48

/*
 * Constants for position mode.
 */

typedef enum { CGLD_CENTERED, CGLD_MOUSE, CGLD_CURRPOS } cgl_dial_pos_mode;

/*
 * Constants for dialog mode.
 */

typedef enum { CGLD_NORMAL, CGLD_BUFFERED, CGLD_WINDOW } cgl_dial_mode;

/*
 * Object class.
 */

class cgl_object
{
 protected:
	cgl_object *next;											/* Pointer to next object. */
	cgl_dialog *dialog;										/* Pointer to dialog box. */

 public:
	cgl_object(void);											/* Constructor. */
	virtual ~cgl_object(void);						/* Destructor. */

	inline cgl_dialog *get_dialog(void) { return dialog; }

	virtual int handle_select(int object, int x, int y);

	friend class cgl_dialog;
};

/*
 * Dialog class.
 */

class cgl_dialog : public cwindow
{
 protected:
	/*
	 * Private typedefs
	 */

	typedef struct
	{
		int key;
		int object;
	} KINFO;

	/*
	 * General variables.
	 */

	OBJECT *dial_tree;										/* Pointer to object tree. */
	cgl_dial_mode dial_mode;							/* Dialog mode. */
	GRECT dial_size;											/* Size of dialog box. */
	int edit_object;											/* Current edit object. */
	int cursor_x;													/* x position in edit_object. */
	int curs_cnt;													/* Cursor counter. */
	MFDB curs_mfdb;												/* Used to save the area under the cursor. */

	int exit_obj;													/* If continue is FALSE, this variable
																					 contains the index of the object,
																					 which caused this dialog to exit. */

	int nkeys;														/* Number of keyboard short cuts for this dialog box. */
	KINFO kinfo[CGLD_MAXKEYS];						/* Keyboard short cuts. */

	cgl_object *objects;									/* List of objects coupled to this dialog. */

	struct
	{
		int visible : 1;										/* Dialog visible flag. */
		int cont : 1;												/* Continue flag. */
	} dl_flags;

	/*
	 * Variables for modal dialog boxes.
	 */

	MFDB dial_mfdb;												/* Screen buffer. */
	cgl_dialog *prev;											/* Pointer to previous dialog. */
	int dmb;															/* Index of dialog move buttton. */

	/*
	 * Private member functions.
	 */

	void set_position(int x, int y);			/* Set the position of the dialog. */

	void doredraw(int mode, GRECT *area, int start, int depth);

	void calc_cursor(GRECT *cursor);			/* Calculate size and position of cursor. */
	void redraw_cursor(GRECT *area);			/* Redraw the text cursor. */
	void cursor_on(void);									/* Switch the text cursor on. */
	void cursor_off(void);								/* Switch the text cursor off. */

	void set_keys(void);									/* Set keyboard short cuts. */
	int find_key(int key);								/* Find keyboard short cut. */

	void edit_init(int object, int curx);	/* Start editing an object. */
	int edit_char(int key);								/* Edit an object. */
	void edit_end(void);									/* Stop editing an object. */

	int form_button(int object, int clicks, int *result);
	int form_keybd(int kobnext, int kchar, int *knxtobject, int *knxtchar);

	void handle_rbutton(int parent, int object);	/* Handle selection of a radio button. */

	void save(void);											/* Save area below the dialog box. */
	void restore(void);										/* Restore the area below the dialog box. */

	void limit_to_desk(GRECT *r);					/* Limit the position of the dialog to the desktop. */
	void calcpos(cgl_dialog *prev, cgl_dial_pos_mode pmode);	/* Calculate the initial position of the dialog box. */

	virtual void cw_draw(int mode, GRECT *area, va_list args);	/* Redraw event handler. */
	inline void cw_draw(int mode, GRECT *area, ...) { va_list args; va_start(args, area); cw_draw(mode, area, args); va_end(args); }
	virtual void cw_moved(GRECT *newpos);	/* Moved event handler. */
	virtual int cw_hndlkey(int scancode, int keystate);
	virtual void cw_hndlbutton(int x, int y, int n, int bstate, int kstate);

 public:
	/*
	 * Constructors and destructors.
	 */

	cgl_dialog(OBJECT *tree, int type = CW_NMDIALOG);
	virtual ~cgl_dialog(void);

	/*
	 * Member functions.
	 */

	inline int get_dial_mode(void) { return dial_mode; }
	inline OBJECT *get_tree(void) { return dial_tree; }
	void draw_object(int start, int depth);
	void change_object(int object, int newstate);
	void add_object(cgl_object *object);

	/*
	 * Functions for non modal dialog boxes.
	 */

	CGL_ERROR nmopen(int start, int x, int y, const char *title);
	CGL_ERROR nmopen(int start, int x, int y, GRECT *xywh, int zoom,
									 const char *title);
	void nmclose(void);
	void nmclose(GRECT *xywh, int zoom);

	/*
	 * Functions for modal dialog boxes.
	 */

	void mopen(void);
	void mopen(GRECT *xywh, int zoom);
	int execute(int start);
	void mclose(void);
	void mclose(GRECT *xywh, int zoom);

	/*
	 * Event handlers.
	 */

	virtual void handle_button(int object);	/* Called when the user has selected an
																						 exit object. */
};

/*
 * Simple interface for modal dialog boxes.
 */

extern int cgld_do_dialog(OBJECT *tree, int start);

/*
 * Configuration functions.
 */

extern cgl_dial_pos_mode cgld_setposmode(cgl_dial_pos_mode newmode);
extern cgl_dial_mode cgld_setdialmode(cgl_dial_mode newmode);

extern int cgld_gaddr(int type, int index, void *addr);
extern void cgld_fixtree(OBJECT *tree);
extern void cgld_set_userobjects(OBJECT *tree);

#endif /* __CGL_DIAL_H */

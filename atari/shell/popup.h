
#ifndef __POPUP_H
#define __POPUP_H

class popup_menu : public cgl_object
{
 protected:
	int nitems;														/* Number of items in popup menu. */
	char **items;													/* Items in popup menu. */
	int cur_selected;											/* Currently selected item in popup menu. */

	int activator;												/* Object that activates the popup menu. */
	int indicator;												/* Object that shows the current selection. */
	int cycle;														/* Object that cycles through the items. */

	virtual int handle_select(int button, int x, int y);
	int do_menu(int x, int y);

 public:
	/*
	 * Constructors and destructors.
	 */
  	 
	popup_menu(int activator, int indicator, int cycle);
	virtual ~popup_menu(void);

	/*
	 * Public member functions.
	 */

	void set_items(int nitems, char *items[], int initial);

	virtual void new_selected(int item, char *value);
};

#endif	/* __POPUP_H */

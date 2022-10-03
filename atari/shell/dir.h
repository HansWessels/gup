#ifndef __DIR_H
#define __DIR_H

#include <stat.h>

#include "slider.h"

typedef struct
{
	char *name;
	int selected;
	osstat stat;
} DIR_ENTRY;

class directory_listbox : public listbox
{
 protected:
	char *path;							/* Current directory. */
	DIR_ENTRY *entries;					/* Files in directory. */
	int n_entries;						/* Number of files. */
	int n_selected;						/* Number of selected items. */
	int max_entries;					/* Maximum number of files in 'entries'. */
	int max_namelength;					/* Maximum length of filename. */

	void free_memory(void);				/* Free all allocated memory. */

	/*
	 * Functions called by the 'listbox' class functions.
	 */

	void get_line(int line, char *string);
	int is_selected(int line);
	void handle_open(int line);
	void handle_select(int line);

 public:
	directory_listbox(void);
	~directory_listbox(void);

	void set_path(const char *new_path, int redraw);
	inline const char *get_path(void) { return path; }
	inline int number_selected(void) { return n_selected; }

	void refresh(void);

	int get_selection(ARC_DIRECTORY *dir);

	void close(void);					/* Close the current directory. */
};

#endif	/* __DIR_H */

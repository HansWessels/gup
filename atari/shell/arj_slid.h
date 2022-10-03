#ifndef __ARJ_SLIDER_H
#define __ARJ_SLIDER_H

class archive_listbox : public listbox
{
 protected:
	char *name;							/* Current archive. */
	int open;							/* Indicates if an archive is opened. */
	ARCHIVE_INFO info;					/* Information about archive. */
	ARC_DIRECTORY dir;					/* Directory tree of archive. */
	char *arj_path;						/* Current directory in archive. */
	ARC_DIRECTORY *cur_dir;				/* Pointer to current directory in 'dir'. */
	int max_namelength;					/* Maximum filename length in current directory. */
	int n_selected;						/* Number of selected items. */

	void free_memory(void);				/* Free all allocated memory. */
	void set_curdir(ARC_DIRECTORY *dir, int redraw);	/* Set 'cur_dir'. */

	/*
	 * Functions called by the 'listbox' class functions.
	 */

	void get_line(int line, char *string) const;
	int is_selected(int line) const;
	void handle_open(int line);
	void handle_select(int line);

 public:
	archive_listbox(void);
	~archive_listbox(void);

	void set_arc_name(const char *filename, int redraw);
	inline const char *get_arc_name(void) const { return name; }
	inline int archive(void) const { return open; }
	inline int number_selected(void) const { return n_selected; }

	void compress(const char *path, ARC_DIRECTORY *dir);
	void decompress(const char *path);

	void close(void);					/* Close the current directory. */
};

#endif	/* __ARJ_SLIDER_H */

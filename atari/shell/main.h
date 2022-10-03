
#define FALSE	0
#define TRUE	1

extern OPTIONS gup_options;

class drive_popup : public popup_menu
{
 protected:
	char *drv_list[32];										/* List with drives. */
	int drives[32];												/* List with drive numbers. */
	int ndrives;													/* Number of drives. */

	virtual void new_selected(int item, char *value);

 public:
	drive_popup(void);
	virtual ~drive_popup(void);

	void set_drive(int new_drive);
};

class main_window : cgl_dialog
{
 protected:
	directory_listbox dir_listbox;				/* List box with current directory. */
	archive_listbox arj_listbox;					/* List box with archive contents. */
	drive_popup drive;										/* Popup menu for changing the current drive. */

	void handle_button(int object);
	void cw_closed(void);

 public:
	main_window(void);

	inline const char *get_path(void) { return dir_listbox.get_path(); }
	inline void set_archive(const char *filename, int redraw)
		{ arj_listbox.set_arc_name(filename, redraw); }
	inline void set_drive(int drv) { drive.set_drive(drv); }
	inline void set_path(const char *path) { dir_listbox.set_path(path, TRUE); }

	void update_extract_button(void);
	void update_compress_button(void);

	friend class arj_app;
};

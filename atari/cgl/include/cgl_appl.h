
#ifndef __CGL_APPL_H
#define __CGL_APPL_H

#include <aesbind.h>
#include <vdibind.h>

typedef enum
{
	CGL_OK,																/* No error. */
	CGL_AES,															/* Error while initiating AES. */
	CGL_VDI,															/* Error while initiating VDI. */
	CGL_NSMEM,														/* Not enough memory. */
	CGL_NMWINDOWS,												/* No more windows. */
	CGL_NINIT,														/* Window is already created. */
	CGL_DIALTYPE,													/* Function not allowed for this dialog type. */
	CGL_UNKNOWN														/* Unknown error. */
} CGL_ERROR;

class APPLICATION
{
 protected:
	int nfonts;														/* Number of loaded VDI fonts. */
	CGL_ERROR status;											/* Application status. */
	const char *prg_name;
	int quit_appl;												/* Quit application flag. */

	OBJECT *menu;													/* Application menu bar. */
	int mn_disable_cnt;										/* Menu bar disable count. */

 public:
	APPLICATION(const char *prgname, int load_fonts = 1);
	virtual ~APPLICATION(void) __attribute__ ((noreturn));

	void run(void);												/* Main event loop of the application. */
	void set_menu_bar(OBJECT *tree);			/* Set the menu of the application. */
	void disable_menu(void);							/* Disable the menu bar. */
	void enable_menu(void);								/* Enable the menu bar. */

	/*
	 * Event handlers for application.
	 */

	virtual int terminate(void);					/* Called when the OS requests all
																					 applications to terminate. Should
																					 return TRUE if the application can
																					 terminate or FALSE if not. */
	virtual void all_windows_closed(void);/* This function is called when the
																					 last open window of the application
																					 has been closed. */
	virtual int handle_menu(int item);		/* Called when an item in the applications
																					 menu is selected. */

	/*
	 * Operators.
	 */

	int operator!() const { return (status != CGL_OK); }

	/*
	 * Friends.
	 */

	friend void cgl_dispatch_message(int *msg);
	friend class cgl_dialog;
};

extern void exit_cgl(void) __attribute__ ((noreturn));

#endif /* __CGL_APPL_H */

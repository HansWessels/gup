
#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <aesbind.h>

#include "cgl_dial.h"
#include "slider.h"

#define MSG_MAX_LINES		50

/*
 * Defines for request().
 */

#define NO	0
#define YES	1

enum request_type
{
	YES_OR_NO_FILE,										/* ask for diff. fname if no */
	YES_OR_NO,												/* yes or no */
	PRESS_KEY,												/* any key */
	NOPE,															/* do nothing */
	CONTINUE_YNQA,										/* while displaying pages of text: yes, no, quit, all */
	YNQA,															/* yes, no, quit, all */
	PASSWORD													/* ask for password */
};

class msg_listbox : public listbox
{
 protected:
	char buffer[MSG_MAX_LINES][81];
	int lines;												/* Number of lines used in 'buffer'. */
	int cur_oline;										/* Current output line. */
	int cur_ocolumn;									/* Current output column. */
	int first_line;										/* First line in buffer. */

	void get_line(int line, char *string);
 public:
	msg_listbox(void);
	~msg_listbox(void);

	friend void print(const char *s, ... );
	friend void warning(const char *s, ... );
};

class message_window : public cgl_dialog
{
 protected:
	msg_listbox msg_lb;
 public:
	message_window(void);
	~message_window(void);

	friend void update_filename(const char *filename);
};

extern void print(const char *s, ... );
extern void warning(const char *s, ... );
extern int request(enum request_type rq, int deflt, const char *fnm,
									 const char *fmt,...);
extern const char *adorable_filename(const char *filename);

extern void update_filename(const char *filename);

extern void alert(const char *format, ...);

extern const char *arj_err_str(gup_result error);

#endif	/* __MESSAGE_H */

#include <aesbind.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "gup.h"
#include "gup_err.h"

#include "cgl_dial.h"
#include "message.h"
#include "resource.h"
#include "slider.h"

static msg_listbox *cur_msg = NULL;
static message_window *cur_win = NULL;

/*
 * Member functions of the msg_listbox class.
 */

void msg_listbox::get_line(int line, char *string)
{
	if ((line = line + first_line) >= MSG_MAX_LINES)
		line -= MSG_MAX_LINES;

	strcpy(string, buffer[line]);
}

/*
 * Constructor and destructor for the msg_listbox class.
 */

msg_listbox::msg_listbox(void)
{
	/*
	 * Initiate variables.
	 */

	lines = 0;
	first_line = 0;
	cur_oline = 0;
	cur_ocolumn = 0;

	cur_msg = this;
}

msg_listbox::~msg_listbox(void)
{
	cur_msg = NULL;
}

/*
 * Constructor and destructor for the message_window class.
 */

message_window::message_window(void) : cgl_dialog(progressdialog, CW_DIALOG)
{
	add_object(&msg_lb);

	msg_lb.init_listbox(SL_HV, MSGTEXT, MSGLEFT, MSGRIGHT, MSGHSLID,
											MSGHSPAR, MSGUP, MSGDOWN, MSGVSLID, MSGVSPAR);
	msg_lb.init_vslider(0, 0, 0);
	msg_lb.init_hslider(80, 0, 0);

	mopen();

	cur_win = this;
}

message_window::~message_window(void)
{
	mclose();
	cur_win = NULL;
}

/*
 * Functions for displaying messages in the current message
 * window.
 */

void print(const char *s, ... )
{
	char buf[512], *p;
	va_list parms;
	int line;

	if (cur_msg == NULL)
		return;

	va_start(parms, s);
	vsprintf(buf, s, parms);

	p = buf;

	while(*p)
	{
		if ((*p == '\n') || (cur_msg->cur_ocolumn == 80))
		{
			cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn++] = 0;
			cur_msg->cur_ocolumn = 0;

			if (++(cur_msg->cur_oline) >= MSG_MAX_LINES)
				cur_msg->cur_oline = 0;

			if (++(cur_msg->lines) > MSG_MAX_LINES)
			{
				cur_msg->lines = MSG_MAX_LINES;
				if (++(cur_msg->first_line) >= MSG_MAX_LINES)
					cur_msg->first_line = 0;
			}

			if (*p == '\n')
				p++;
		}
		else if (*p == '\v')
		{
			int pos;

			p++;
			pos = (int) (p[0] - '0') * 10 + (int) p[1] - '0';
			p += 2;

			while (cur_msg->cur_ocolumn < pos)
				cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn++] = ' ';
		}
		else
			cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn++] = *p++;
	}

	cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn] = 0;

	if ((line = cur_msg->cur_oline - cur_msg->first_line) <= 0)
		line += MSG_MAX_LINES;

	cur_msg->init_vslider(cur_msg->lines, line, 1);
	cur_msg->dialog->draw_object(MSGTEXT, 1);

	va_end(parms);
}

/*
 * void warning(char *str, ...)
 *
 * Display an warning.
 *
 * Parameters:
 *
 * str	- warning to display.
 * ...	- optional other parameters.
 */

void warning(const char *s, ... )
{
	char buf[512], *p;
	va_list parms;
	int line;

	if (cur_msg == NULL)
		return;

	va_start(parms, s);
	vsprintf(buf, s, parms);

	p = buf;

	while(*p)
	{
		if ((*p == '\n') || (cur_msg->cur_ocolumn == 80))
		{
			cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn++] = 0;
			cur_msg->cur_ocolumn = 0;

			if (++(cur_msg->cur_oline) >= MSG_MAX_LINES)
				cur_msg->cur_oline = 0;

			if (++(cur_msg->lines) > MSG_MAX_LINES)
			{
				cur_msg->lines = MSG_MAX_LINES;
				if (++(cur_msg->first_line) >= MSG_MAX_LINES)
					cur_msg->first_line = 0;
			}

			if (*p == '\n')
				p++;
		}
		else if (*p == '\v')
		{
			int pos;

			p++;
			pos = (int) (p[0] - '0') * 10 + (int) p[1] - '0';
			p += 2;

			while (cur_msg->cur_ocolumn < pos)
				cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn++] = ' ';
		}
		else
			cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn++] = *p++;
	}

	cur_msg->buffer[cur_msg->cur_oline][cur_msg->cur_ocolumn] = 0;

	if ((line = cur_msg->cur_oline - cur_msg->first_line) <= 0)
		line += MSG_MAX_LINES;

	cur_msg->init_vslider(cur_msg->lines, line, 1);
	cur_msg->dialog->draw_object(MSGTEXT, 1);

	va_end(parms);
}

int request(enum request_type rq, int deflt, const char *fnm,
						const char *fmt,...)
{
	(void) rq;
	(void) fnm;
	(void) fmt;

	return deflt;
}

const char *adorable_filename(const char *filename)
{
	return filename;
}

/*
 * void update_file_name(const char *filename)
 *
 * Update the filename currently displayed in the message
 * window.
 */

void update_filename(const char *filename)
{
	int maxlen = strlen(((TEDINFO *) progressdialog[MCURFILE].ob_spec)->te_pvalid);

	if (cur_win == NULL)
		return;

	strncpy(((TEDINFO *) progressdialog[MCURFILE].ob_spec)->te_ptext, filename, maxlen);
	cur_win->draw_object(MCURFILE, 1);
}

/*
 * void alert(const char *format, ...)
 *
 * In a GUI, display a messagebox displaying the message 'format'. On a
 * text screen, just display an error message.
 *
 * Parameters:
 *
 * format	- Message to display.
 */

void alert(const char *format, ...)
{
	char tmpbuf[512], msg_str[512], *s, *d;
	int line_length;
	va_list parms;
	const int max_len = 34;

	va_start(parms, format);
	vsprintf(tmpbuf, format, parms);

	strcpy(msg_str, "[1][");
	d = msg_str + strlen(msg_str);
	s = strtok(tmpbuf, " \t");
	line_length = 0;

	while (s)
	{
		int l = strlen(s);

		if (l > (max_len - 5))
		{
			l = max_len;
			s[max_len] = 0;
		}

		if ((line_length + l) > max_len)
		{
			*d++ = '|';
			line_length = 0;
		}
		else if (line_length != 0)
			*d++ = ' ';

		while (*s)
			*d++ = *s++;
		line_length += l;

		s = strtok(NULL, " \t");
	}

	strcat(msg_str, "][ OK ]");

	form_alert(1, msg_str);

	va_end(parms);
}

const char *arj_err_str(gup_result error)
{
	switch(error)
	{
	case GUP_OK:
		return "No error";
	case GUP_NOENT:
		return "File not found";
	case GUP_EOF:
		return "Unexpected end of file";
	case GUP_NO_ARCHIVE:
		return "Not a valid archive";
	case GUP_BROKEN:
	case GUP_SEVERE:
		return "Archive is broken";
	case GUP_CRC_FAULT:
		return "CRC error";
	case GUP_NOSPC:
		return "Disk full";
	case GUP_BAD_HUFF_TABLE1:
	case GUP_BAD_HUFF_TABLE2:
	case GUP_BAD_HUFF_TABLE3:
		return "Bad huffman table";
	case GUP_NOMEM:
		return "Out of memory";
	case GUP_READ_ERROR:
		return "Error while reading file";
	case GUP_WRITE_ERROR:
		return "Error while writing file";
	default:
		return "Unknown error";
	}
}

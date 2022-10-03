#include <aesbind.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "gup_err.h"
#include "alert.h"
#include "arjshell.h"

#pragma implementation

int vaprintf(int def, const char *string, va_list argpoint)
{
	char s[256];

	vsprintf(s, string, argpoint);
	return form_alert(def, s);
}

int aprintf(int def, char *string,...)
{
	va_list argpoint;
	int button;

	va_start(argpoint, string);
	button = vaprintf(def, string, argpoint);
	va_end(argpoint);

	return button;
}

int alert_printf(int def, int message,...)
{
	va_list argpoint;
	int button;
	char *string;

	rsrc_gaddr(R_STRING, message, &string);

	va_start(argpoint, message);
	button = vaprintf(def, string, argpoint);
	va_end(argpoint);

	return button;
}

void xform_error(int error)
{
	int message;

	switch (error)
	{
	case ENOENT:
		message = MENOENT;
		break;
	case EPATH:
		message = MEPATH;
		break;
	case ENOMEM:
		message = MENOMEM;
		break;
	case ELOCKED:
		message = MELOCKED;
		break;
	case EPLFMT:
		message = MEPLFMT;
		break;
	default:
		message = -1;
		break;
	}

	if (message == -1)
		alert_printf(1, MERROR, error);
	else
		alert_printf(1, message);
}

void show_gup_error(gup_result error)
{
	switch(error)
	{
	case GUP_NOMEM:
		xform_error(ENOMEM);
		break;
	case GUP_WRITE_ERROR:
		aprintf(1, "[1][Error while writing file.|Disk full?][ OK ]");
		break;
	default:
		aprintf(1, "[1][Error %d.][ OK ]", (int) error);
		break;
	}
}

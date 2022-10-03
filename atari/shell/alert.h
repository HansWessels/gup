
#pragma interface

#include <stdarg.h>

extern int vaprintf(int def, const char *string, va_list argpoint);
extern int aprintf(int def, char *string,...);
extern int alert_printf(int def, int message,...);
extern void xform_error(int error);
extern void show_gup_error(gup_result error);

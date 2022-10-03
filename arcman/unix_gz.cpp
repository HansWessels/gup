/*
 * Archive manager.
 *
 *     GZIP UNIX support functions.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:12:46 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 222 $
 * $Log$
 * Revision 1.1  2000/09/03 13:12:46  wout
 * GZIP support.
 *
 */

#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "gup.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "support.h"
#include "gup_err.h"
#include "arc_util.h"
#include "os.h"

uint32 gzip_conv_from_os_time(ostime time)
{
	return time.time;
}

ostime gzip_conv_to_os_time(uint32 gzip_time)
{
	ostime time;

	time.time = gzip_time;

	return time;
}

/*
 * char *gzip_conv_to_os_name(const char *gzip_name)
 *
 * Convert a file name read from a file header to OS format. The file name is
 * returned in a string allocated with new. Trailing slashes are removed
 * from the file name.
 *
 * Parameters:
 *
 * gzip_name		- filename
 *
 * Result: file name in OS format.
 */

char *gzip_conv_to_os_name(const char *gzip_name)
{
	char *os_name;

	os_name = new char[strlen(gzip_name) + 1];
	strcpy(os_name, gzip_name);

	return os_name;
}

/*
 * void gzip_conv_from_os_name(const char *os_name)
 *
 * Convert a file name in OS format to gzip format.
 *
 * os_name		- name of file in OS format.
 *
 * Result: file name in gzip format.
 */

char *gzip_conv_from_os_name(const char *os_name)
{
	const char *name_ptr;
	char *gzip_name;

	name_ptr = get_name(os_name);

	gzip_name = new char[strlen(name_ptr) + 1];
	strcpy(gzip_name, name_ptr);

	return gzip_name;
}

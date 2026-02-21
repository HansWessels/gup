
#include "gup.h"

//#include <malloc.h> //niet meer nodig onder linux en MAC OS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "gup_err.h"
#include "arj.h"
#include "options.h"
#include "compress.h"
#include "arcman.h"
#include "utils.h"

/*
 * gup_result builddir(const char *name)
 *
 * Check if all parent directories of the file 'name' exist. If
 * a parent directory does not exist, it is created.
 *
 * Parameters:
 *
 * name		- name of the file.
 */

gup_result builddir(const char *name)
{
	char *filename, *curpthpos;
	char ch;

	filename = (char *) alloca(strlen(name) + 1);
	strcpy(filename, name);

	curpthpos = strchr(filename + 1, DELIM);

	while (curpthpos)					/* still a dir to go! */
	{
		struct stat data;

		ch = *curpthpos;
		*curpthpos = 0;

		if (stat(filename, &data) != 0)
		{
			gup_result result;

			/*
			 * Directory does not exist. Create the directory.
			 */

			if ((result = gup_mkdir(filename)) != GUP_OK)
			{
				printf("create directory <%s>: ", name);
				display_error(result);

				return result;
			}
		}

		*curpthpos = ch;
		curpthpos = strchr(curpthpos + 1, DELIM);
	}

	return GUP_OK;
}

/*
 * char *host_os_name( int os )
 *
 * Returns string for os number as defined in arj.h
 * or string "UnknownOS" if unkown os
 */

const char *host_os_names[] = HOST_OS_NAMES;

const char *host_os_name(int os)
{

	if (os >= MAX_OS)
		return "UnknownOS";
	return host_os_names[os];
}

/* Print progress indicator */

unsigned long progress_current;
unsigned long progress_total;

/*
// Pretty Name Print
// Prints the filename in a TOS-crew approved manner.
*/
char *pnprint(int width, const char *name, char *buf)
{
	int len = (int) strlen(name);

	if (len <= width)
	{
		strcpy(buf, name);
	}
	else if (width <= 0)
	{
		*buf = 0;
	}
	else if (width <= 15)
	{
		strcpy(buf, name + len - width);
	}
	else if (width <= 25)				/* dit is een heel erg willekeurige getal */
	{
		sprintf(buf, "...%s", name + len + 3 - width);
	}
	else
	{
		int first = (width - 5) / 3;

		sprintf(buf, "%.*s ... %s", first, name,
				name + len - width + 5 + first);
	}
	return buf;
}

void init_progress(unsigned long size)
{
	progress_current = 0;
	progress_total = (size / 100) + 1;	/* +1 om division by zero te voorkomen */
}

void print_progress(unsigned long delta, void *propagator)
{
	NEVER_USE(propagator);
	printf("%3i%%\b\b\b\b", (int) ((progress_current) / progress_total));
	progress_current += delta;
	FLUSH();
}

void init_progress_size(unsigned long size)
{
	NEVER_USE(size);
	progress_current = 0;
}

void print_progress_size(unsigned long delta, void *propagator)
{
	NEVER_USE(propagator);
	progress_current += delta;
	if (progress_current < 1000)
	{
		printf("%3i \b\b\b\b", (int) progress_current);
	}
	else if (progress_current < 10240L)
	{
		printf("%1ik%02i\b\b\b\b", (int) (progress_current >> 10),
			   (int) (100 * (progress_current & 0x3ff) >> 10));
	}
	else if (progress_current < 102400L)
	{
		printf("%2ik%1i\b\b\b\b", (int) (progress_current >> 10),
			   (int) (10 * (progress_current & 0x3ff) >> 10));
	}
	else if (progress_current < 1024000L)
	{
		printf("%3ik\b\b\b\b", (int) (progress_current >> 10));
	}
	else if (progress_current < (1024L * 10240L))
	{
		printf("%1iM%02i\b\b\b\b", (int) (progress_current >> 20),
			   (int) (100 * ((progress_current >> 10) & 0x3ff) >> 10));
	}
	else if (progress_current < (1024L * 102400L))
	{
		printf("%2iM%1i\b\b\b\b", (int) (progress_current >> 20),
			   (int) (10 * ((progress_current >> 10) & 0x3ff) >> 10));
	}
	else if (progress_current < (1024L * 1024000L))
	{
		printf("%3iM\b\b\b\b", (int) (progress_current >> 20));
	}
	else						/* if(progress_current<(1024L*1024*10240L)) */
	{
		printf("%1iG%02i\b\b\b\b", (int) (progress_current >> 30),
			   (int) (100 * ((progress_current >> 20) & 0x3ff) >> 10));
	}
	FLUSH();
}

const char *arj_error_disc[] = {
	/*
	 * General result codes.
	 */

	"No error",
	"Internal error.",
	"Unknown error, probably a C error code not recognized by 'gup_conv_err'.",

	/*
	 * File I/O errors.
	 */

	"Can't read file.",
	"File write error.",
	"Unexpected end of file.",
	"Can not create directory.",
	"Unable to create temporary file name.",

	/*
	 * C error codes.
	 */

	"Operation not permitted.",
	"Access denied.",
	"File or directory not found.",
	"General I/O error.",
	"No such device or address.",
	"Executable format error.",
	"Bad file handle.",
	"Out of memory.",
	"Bad (memory block) address.",
	"Device or resource busy.",
	"File already exists.",
	"Cross device link.",
	"No such device.",
	"Not a directory.",
	"Is a directory.",
	"Invalid internal parameter.",
	"File table overflow.",
	"Too many open files.",
	"Text file busy.",
	"File too large.",
	"No space left on device.",
	"Illegal seek.",
	"Read only file system.",
	"Too many links.",
	"Broken pipe.",
	"File name too long.",
	"Directory is not empty.",
	"Too many symbolic links.",
	"Device is not a stream.",

	/*
	 * Resultcodes of decode.c
	 */

	"Error in first huffman table.",
	"Error in second huffman table.",
	"Error in third huffman table.",

	/*
	 * Warnings of init_encode
	 */

	"Warning: Insufficient memory for -jm mode!",
	"Warning: Insufficient memory for fast mode!",
	"Warning: Insufficient memory for medium mode!",
	"Warning: Insufficient memory for linking!",
	"Warning: Insufficient memory for sieve mode!",
	"Warning: Using a smaller dictionary!",

	/*
	 * Errors in archives.
	 */

	"File is not a valid archive.",
	"Archive broken.",
	"Archive severly damaged.",
	"CRC error in file.",

	/*
	 * Errors in headers.
	 */

	"CRC error in a header.",
	"Unknown method in header.",
	"Unknown OS id in header.",
	"Header not found at expected position.",
	"Header is too long.",
	"Error in header.",

	/*
	 * Last of the errors
	 */
};


void display_error(gup_result err)
{								/* displays arj_result errors */
	if (err >= GUP_LAST_ERROR)
	{
		printf("Unkown error!\n");
	}
	else
	{
		printf("%s\n", arj_error_disc[err]);
	}
}

void init_message(gup_result message, void *propagator)
{
	NEVER_USE(propagator);
	display_error(message);
}


#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

/*
 * char *make_path(const char *path, const char *name, int extra)
 *
 * Join 'path' and 'name' to a full pathname of a file.
 * The resulting string is allocated with 'malloc'.
 *
 * path  - path
 * name  - filename to append to path
 * extra - extra length to allocate
 *
 * Result: NULL if not enough memory, otherwise pointer to
 *         allocated string.
 */

char *make_path(const char *path, const char *name, int extra)
{
	char *fullname, *ptr;

	if ((fullname = malloc(strlen(path) + strlen(name) + 2 + extra)))
	{
		if (path[0] == 0)
			ptr = fullname;
		else
		{
			strcpy(fullname, path);
			ptr = fullname + strlen(fullname) - 1;
			if (*ptr++ != '/')
				*ptr++ = '/';
		}

		strcpy(ptr, name);
	}

	return fullname;
}

/*
 * const char *get_fname_pos(const char *pathname)
 *
 * Return a pointer to the file name part of a full path name
 * of a file.
 *
 * Parameters:
 *
 * pathname	- full path name of file
 *
 * Result: pointer to file name of file.
 */

const char *get_fname_pos(const char *pathname)
{
	const char *ptr = strrchr(pathname, '/');

	return (ptr) ? ptr + 1 : pathname;
}

/*
 * int exist(const char *filename)
 *
 * Check if a file exists.
 *
 * Parameters:
 *
 * filename	- name of the file.
 *
 * Result: 0 if file does not exist, 1 if file exists.
 */

int exist(const char *filename)
{
	int handle;

	if ((handle = open(filename, O_RDONLY)) >= 0)
	{
		close(handle);
		return 1;										/* File exists. */
	}
	else
		return 0;										/* File does not exist. */
}

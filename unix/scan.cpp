/* directory traversal routs
 */

#include "gup.h"

#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "arj.h"
#include "gup_err.h"
#include "options.h"
#include "compr_io.h"
#include "compress.h"
#include "arcman.h"
#include "scan.h"
#include "support.h"

/* #define TEST
*/

extern gup_result pack_file(archive *archive, const char *the_name, const osstat *stat, OPTIONS *opts);

typedef struct dstack
{
	char *name;
	osstat stat;
	struct dstack *next;
} DSTACK;

/* for first only names of type ([nowildcards]/)*[wildcardthing]
 * expect here only unixfied names e.g. '/'
 */
/* TODO: filter out opts->arj_name
 */

gup_result traverse_dirs(DSTACK *dir, const char *pattern,
						 gup_result (*dothing) (archive *archive,
												const char *the_name,
												const osstat *stat,
												OPTIONS *opts),
						 archive *archive, OPTIONS *opts)
{
	DSTACK *cur;

	/* this may be a formality....*/

	for( cur = dir; cur->next != NULL; cur = cur->next );

	/* cur is last in list */

	while (dir != NULL)
	{
		long dirl = strlen(dir->name);
		DIR *dirp;
		gup_result result = GUP_OK;

		if( dirl == 0 )
		{
			puts("traversedirs: illegal dirname (length 0)");
			return GUP_INTERNAL;
		}

		if ((dirp = opendir(dir->name)) == NULL)
		{
			perror("traversedirs (opendir)");
			return gup_conv_err(errno);
		}

		while(result == GUP_OK)				  /* read entries from dir */
		{
			struct dirent *entp;
			DSTACK *new_dir;

			if ((entp = readdir(dirp)) == NULL)
			{
				if (errno != 0)
					perror("traversedirs (readdir)");
				break;
			}

			if ((strcmp(entp->d_name, ".") == 0) || (strcmp(entp->d_name, "..") == 0))
				continue;

			/* skip volume labels too */
			new_dir = (DSTACK *) malloc(sizeof(DSTACK));
			if (new_dir == NULL)
			{
				printf("Couldn't alloc new DSTACK\n");
				closedir(dirp);
				return GUP_NOMEM;
			}
			new_dir->name = (char *) malloc(dirl + strlen(entp->d_name) + 2);
			if (new_dir->name == NULL)
			{
				puts("Couldn't alloc new fname");
				closedir(dirp);
				return GUP_NOMEM;
			}

			/* fill in struct */

			strcpy(new_dir->name, dir->name);

			if(dir->name[dirl-1] != DELIM)
			{
				strcat(new_dir->name, DELIM_STR);
				strcpy(new_dir->name+dirl+1,entp->d_name);
			}
			else
				strcpy(new_dir->name+dirl, entp->d_name);

			if ((result = gup_lstat(new_dir->name, &new_dir->stat)) == GUP_OK)
			{
				new_dir->next = NULL;

				if (gup_is_dir(&new_dir->stat))
				{
					cur->next = new_dir;
					cur = new_dir;

					result = dothing(archive, new_dir->name, &new_dir->stat, opts);
				}
				else
				{
					if (pattern)
					{
						if (match_pattern(new_dir->name, pattern))
							result = dothing(archive, new_dir->name, &new_dir->stat, opts);
					}
					else
						result = dothing(archive, new_dir->name, &new_dir->stat, opts);

					free(new_dir->name);
					free(new_dir);
				}
			}
		}

		closedir(dirp);
		dir = dir->next;
	}

	return GUP_OK;
}

/* is_regexp checks if expr regexpish
 */

int is_regexp(char *expr)
{
	return (strpbrk(expr, "?*[]") != 0);
}

/* split commandline arg in a directory part and a wildcard part
 * *dir and *wild are pointers into arg!!!
 * *dir contains also *wild. to seperate *(wild-1) = 0
 */

void split_arg(char *arg, char **dir, char **wild)
{
	char *h;

	if ((h = strrchr(arg, DELIM)) != NULL)
	{
		*dir = arg;

		if (is_regexp(h + 1))
			*wild = h + 1;
		else
			*wild = NULL;
	}
	else
	{
		if (is_regexp(arg))
		{
			*dir = NULL;
			*wild = arg;
		}
		else
		{
			*dir = arg;
			*wild = NULL;
		}
	}
}

gup_result pack_arg(archive *archive, char *arg, OPTIONS *opts)
{
	DSTACK new_dir;
	const char *dir, *wild;
	gup_result ret;
	osstat stat;

	split_arg(arg, &dir, &wild);

	if (wild)
	{
		if (dir)						  /* if there's something in front of wildcard */
			*(wild - 1) = 0;
	}

	if (!dir || (*dir == 0))
		dir = ".";

	if ((ret = gup_lstat(dir, &stat)) != GUP_OK)
		return ret;

	if (!gup_is_dir(&stat))
	{
		if (wild)
			*(wild - 1) = DELIM;

		return pack_file(archive, dir, &stat, opts);
	}

	ret = pack_file(archive, dir, &stat, opts);

	if (ret == GUP_OK)
	{
		/* now we got a dir part and a regular expression to match with */

		new_dir.stat = stat;
		new_dir.next = NULL;

		if ((new_dir.name = strdup(dir)) == NULL)
		{
			printf("Error while processing: \'%s\'.\nOut of memory\n", arg);
			return GUP_NOMEM;
		}

		ret = traverse_dirs(&new_dir, wild, pack_file, archive, opts);

		free(new_dir.name);
	}

	return ret;
}

#ifdef TEST

void pack_file(char *path)
{
	printf("Packing file (dothing): %s\n", path);
}

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
	{
		pack_arg(argv[i]);
	}
	return 0;
}
#endif

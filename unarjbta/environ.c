/*
 * environ.c what's left of the original Jung implementation
 */

#include "unarj.h"
#include <string.h>
#include <stdlib.h>

void case_path(char *name)
{
  (void) name;
}

void default_case_path(char *name)
{
  (void) name;
}

int file_exists(char *name)
{
  FILE *fd;

  if ((fd = fopen(name, "rb")) == NULL)
  {
    return 0;
  }
  fclose(fd);
  return 1;
}

void get_mode_str(char *str, uint mode)
{
  strcpy(str, "---W");
  if (mode & FA_ARCH)
  {
    str[0] = 'A';
  }
  if (mode & FA_SYSTEM)
  {
    str[1] = 'S';
  }
  if (mode & FA_HIDDEN)
  {
    str[2] = 'H';
  }
  if (mode & FA_RDONLY)
  {
    str[3] = 'R';
  }
}

int set_ftime_mode(char *name, ulong tstamp, uint attribute, uint  host)
{
  (void) name;
  (void) tstamp;
  (void) attribute;
  (void) host;
  return 0;
}

FILE *file_open(char *name, char *mode)
{
  return fopen(name, mode);
}

/* end ENVIRON.C */

/********************************************************************
 *                                                                  *
 * Arj directory scanner. Copyright (c) 1994 W. Klaren.             *
 *                                                                  *
 ********************************************************************
 *
 * $Author: hwessels $
 * $Date: 1998-01-03 23:43:16 +0100 (Sat, 03 Jan 1998) $
 * $Revision: 88 $
 * $Log$
 * Revision 1.2  1998/01/03 22:43:16  hwessels
 * Sources van Hans, 03-01-1997.
 *
 * Revision 1.1  1996/11/20 20:55:36  hans
 * Sources from Hans, 16-11-1996.
 *
 ********************************************************************/


#include "port.h"

#if 0
/* Unix file attributen */

#define S_IFMT  0170000
#define S_IFREG 0100000   /* Regular file */
#define S_IFDIR 0040000   /* Directory */
#define S_IFCHR 0020000   /* BIOS special file */
#define S_IFIFO 0120000   /* FIFO */
#define S_IMEM  0140000   /* memory region or process */
#define S_IFLNK 0160000   /* symbolic link */

#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001
#endif

#ifndef FA_NORMAL
#define FA_NORMAL      _A_NORMAL
#endif

#ifdef __GNUC__
#define _A_RDONLY	FA_RDONLY
#define _A_SUBDIR	FA_DIR
#endif

#ifndef FA_READONLY
#define FA_READONLY    _A_RDONLY
#endif

#ifndef FA_SUBDIR
#define FA_SUBDIR   _A_SUBDIR
#endif

#ifndef FA_HIDDEN
#define FA_HIDDEN   _A_HIDDEN
#endif

#ifndef FA_SYSTEM
#define FA_SYSTEM _A_SYSTEM
#endif

#ifndef FA_VOLID
#define FA_VOLID _A_VOLID
#endif

#ifndef FA_ARCH
#define FA_ARCH _A_ARCH
#endif


typedef struct
{
  char *path;
  union
  {
    long handle;
    struct
    {
      int first;      /* 1 = eerste file lezen, 0 = huidige lezen. */
#ifndef __WATCOMC__
      DTA *old_dta;
      DTA dta;
#else
    struct find_t dta;
#endif
    } gdata;
  } data;
} XDIR;

extern int scan_dir(const char *file, packstruct *com);

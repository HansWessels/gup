
#ifndef __PORTARJ__
#define __PORTARJ__

#include "gup.h"
#ifdef __ATARI__
#ifdef __PUREC__
#include <tos.h>
#include <ext.h>
#elif __GNUC__
#include <osbind.h>

typedef struct
{
  unsigned int   mode;
  long           index;
  unsigned int   dev;
  unsigned int   reserved1;
  unsigned int   nlink;
  unsigned int   uid;
  unsigned int   gid;
  long           size;
  long           blksize;
  long           nblocks;
  int            mtime;
  int            mdate;
  int            atime;
  int            adate;
  int            ctime;
  int            cdate;
  int            attr;
  int            reserved2;
  long           reserved3[2];
} XATTR;

#define DTA			_DTA
#define DISKINFO	_DISKINFO
#define DOSTIME		_DOSTIME
#define getch()

#endif
#else
/* DOS stuff */
#include <sys\stat.h>
#include <sys\types.h>

#include <conio.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#endif

#ifdef __ATARI__
#define FLUSH()
#else

#define FLUSH() fflush(stdout) /* Flush standard out */

typedef struct
{
  uint8 d_reserved[21];
  uint8 d_attrib;
  uint16 d_time;
  uint16 d_date;
  long d_length;
  char d_fname[14];
} DTA;

typedef struct
{
  long b_free;
  long b_total;
  long b_secsiz;
  long b_clsiz;
} DISKINFO;


typedef struct
{
  unsigned int   mode;
  long           index;
  unsigned int   dev;
  unsigned int   reserved1;
  unsigned int   nlink;
  unsigned int   uid;
  unsigned int   gid;
  long           size;
  long           blksize;
  long           nblocks;
  int            mtime;
  int            mdate;
  int            atime;
  int            adate;
  int            ctime;
  int            cdate;
  int            attr;
  int            reserved2;
  long           reserved3[2];
} XATTR;

typedef struct
{
  uint16 time;
  uint16 date;
} DOSTIME;


int Dgetdrv(void);

long Dfree(DISKINFO *buf, int drv);

long Fwrite(int handle, long count, void *buf);

long Fread(int handle, long count, void *buf);

int Fclose(int handle);

long Fseek(long offset, int handle, uint16 seekmode);

int Fcreate(const char *filename, uint16 attribs);

uint16 Tgetdate(void) ;

uint16 Tgettime(void);

int Fopen(const char *fname, int mode);

int Fattrib(const char *fname, int wflag, unsigned int attrib);

void Fdatime(DOSTIME *timeptr, int handle, int wflag);

#endif

#ifndef FO_READ
#define FO_READ (O_RDONLY | SH_DENYWR)
#endif

#endif
 
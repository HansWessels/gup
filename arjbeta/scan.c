/********************************************************************
 *                                                                  *
 * Arj directory scanner. Copyright (c) 1994 W. Klaren.             *
 *                                                                  *
 ********************************************************************
 *
 * $Author: hans $
 * $Date: 2000-07-30 16:15:21 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 155 $
 * $Log$
 * Revision 1.4  2000/07/30 14:15:21  hans
 * Updated to GUP 0.0.4.
 *
 * Revision 1.3  1998/01/03 22:43:16  hwessels
 * Sources van Hans, 03-01-1997.
 *
 * Revision 1.1  1996/11/20 20:55:24  hans
 * Sources from Hans, 16-11-1996.
 *
 * 01-11-1997 HWS:
 * -adapted source for file buffering routines
 * -fixed 0 byte length bug in multiple volume code
 * -fixed mode 0 bug in multiple volume code
 *
 ********************************************************************/

#include "port.h"

#include "gup.h"
#include "arjbeta.h"
#include "arj_outp.h"
#include "scan.h"
#include "file_buf.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif


#define ERROR -1
#define ENSMEM  -39   /* Out of memory. */
#define EFILNF  -33     /* File not found. */
#define ENMFIL  -49     /* No more files. */

#ifdef __GNUC__
/* file types */
#define S_IFMT  0170000     /* mask to select file type */
#define S_IFCHR 0020000     /* BIOS special file */
#define S_IFDIR 0040000     /* directory file */
#define S_IFREG 0100000     /* regular file */
#define S_IFIFO 0120000     /* FIFO */
#define S_IMEM  0140000     /* memory region or process */
#define S_IFLNK 0160000     /* symbolic link */

/* special bits: setuid, setgid, sticky bit */
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

/* file access modes for user, group, and other*/
#define S_IRUSR         0400
#define S_IWUSR         0200
#define S_IXUSR         0100
#define S_IRGRP         0040
#define S_IWGRP         0020
#define S_IXGRP         0010
#define S_IROTH         0004
#define S_IWOTH         0002
#define S_IXOTH         0001

#include <fcntl.h>

#ifndef FO_READ
#define FO_READ (O_RDONLY | O_DENYW)
#endif

#define SH_DENYWR		O_DENYW
#endif

typedef struct copydata
{
  int result;
  XDIR *dir;
  char *path;
  char *name;
  struct copydata *prev;
} COPYDATA;

/*
 * Get the path of a filename. The path is returned in a malloced
 * string.
 */

char *get_path(const char *path, int *error)
{
  char *backsl;
  long l;

  if ((backsl = strrchr(path, '\\')) == NULL)
    backsl = (char *) path;

  if (((l = backsl - (char *) path) == 2) && (path[1] == ':'))
    l++;

  if ((backsl = malloc(l + 1)) == NULL)
  {
    *error = ENSMEM;
    return NULL;
  }

  strncpy(backsl, path, l);
  backsl[l] = 0;

  *error = 0;
  return backsl;
}

/*
 * Return a pointer to the filename.
 */

char *get_name(const char *path)
{
  char *h;

  if ((h = strrchr(path, '\\')) == NULL)
    return (char *) path;

  else
    return h + 1;
}

/*
 * Join a path and a filename to a new filename (malloced).
 */

char *make_path(const char *path, const char *name, int *error)
{
  char *p;

  if ((p = malloc(strlen(path) + strlen(name) + 2L)) != NULL)
  {
    long l;

    *error = 0;

    strcpy(p, path);
    l = strlen(p);
    if (l && (p[l - 1] != '\\'))
      p[l++] = '\\';
    strcpy(p + l, name);
  }
  else
    *error = ENSMEM;

  return p;
}

/*
 * Split a filename in name and extension.
 */

static void split_name(char *name, char *ext, const char *fname)
{
  char *s, *d, *e;

  if ((e = strrchr(fname, '\\')) == NULL)
    e = (char *) fname;

  if ((e = strchr(e, '.')) == NULL)
  {
    strcpy(name, fname);
    *ext = 0;
  }
  else
  {
    s = (char *) fname;
    d = name;

    while (s != e)
      *d++ = *s++;
    *d = 0;

    strcpy(ext, e + 1);
  }
}

/*
 * Functions for wildcard matching.
 */

static int cmp_part(const char *name, const char *wildcard)
{
  int i = -1, j = -1;

  do
  {
    j++;
    i++;
    switch (wildcard[i])
    {
    case '?':
      if (name[j] == 0)
        return FALSE;
      break;
    case '*':
      if (wildcard[i + 1] == 0)
        return TRUE;
      else
      {
        i++;
        while (name[j] != 0)
        {
          if (cmp_part(name + j, wildcard + i) == TRUE)
            return TRUE;
          j++;
        }
        return FALSE;
      }
    default:
      if (name[j] != wildcard[i])
        return FALSE;
      break;
    }
  }
  while (wildcard[i] != 0);
  return TRUE;
}

int cmp_wildcard(const char *fname, const char *wildcard)
{
#ifdef _MINT_
  return cmp_part(fname,wildcard);
#else
  char name[10], ext[4], wname[10], wext[4];

  split_name(name, ext, fname);
  split_name(wname, wext, wildcard);

  strlwr(name);
  strlwr(ext);
  strlwr(wname);
  strlwr(wext);

  if (cmp_part(name, wname) == FALSE)
    return FALSE;
  return cmp_part(ext, wext);
#endif
}

/*
 * Check if a name contains wildcard characters.
 */

int is_wildcard(const char *name)
{
  if (strchr(name, '?') || strchr(name, '*'))
    return TRUE;
  else
    return FALSE;
}

#ifndef __WATCOMC__

/* Convert a DTA structure to a XATTR structure. The index, dev,
   rdev, blksize and nblocks fields in attrib are not set. They
   are not necessary anyway on TOS. */

static void dta_to_xattr(DTA *dta_ptr, XATTR *attrib)
{
  attrib->mode = 0777;
#ifdef __GNUC__
  if (dta_ptr->dta_attribute & FA_SUBDIR)
#else
  if (dta_ptr->d_attrib & FA_SUBDIR)
#endif
    attrib->mode |= S_IFDIR;
  else
    attrib->mode |= S_IFREG;
#ifdef __GNUC__
  if (dta_ptr->dta_attribute & FA_READONLY)
#else
  if (dta_ptr->d_attrib & FA_READONLY)
#endif
    attrib->mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
#ifdef __GNUC__
  attrib->size = dta_ptr->dta_size;
#else
  attrib->size = dta_ptr->d_length;
#endif
  attrib->uid = 0;
  attrib->gid = 0;
#ifdef __GNUC__
  attrib->mtime = attrib->atime = attrib->ctime = dta_ptr->dta_time;
  attrib->mdate = attrib->adate = attrib->cdate = dta_ptr->dta_date;
  attrib->attr = (int) dta_ptr->dta_attribute & 0xFF;
#else
  attrib->mtime = attrib->atime = attrib->ctime = dta_ptr->d_time;
  attrib->mdate = attrib->adate = attrib->cdate = dta_ptr->d_date;
  attrib->attr = (int) dta_ptr->d_attrib & 0xFF;
#endif
}

#else

/* Convert a DTA structure to a XATTR structure. The index, dev,
   rdev, blksize and nblocks fields in attrib are not set. They
   are not necessary anyway on TOS. */

static void dta_to_xattr(struct find_t *dta, XATTR *attrib)
{
  attrib->mode = 0777;
  if (dta->attrib & FA_SUBDIR)
    attrib->mode |= S_IFDIR;
  else
    attrib->mode |= S_IFREG;
  if (dta->attrib & FA_READONLY)
    attrib->mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
  attrib->size = dta->size;
  attrib->uid = 0;
  attrib->gid = 0;
  attrib->mtime = attrib->atime = attrib->ctime = dta->wr_time;
  attrib->mdate = attrib->adate = attrib->cdate = dta->wr_date;
  attrib->attr = (int) dta->attrib & 0xFF;
}

#endif

XDIR *x_opendir(const char *path, int *error)
{
#ifdef _MINT_
  XDIR *dir;

  if ((dir = malloc(sizeof(XDIR))) == NULL)
    *error = ENSMEM;
  else
  {
    dir->path = (char *) path;
    if (((dir->data.handle = gemdos(0x128, path, 0)) & 0xFF000000L) == 0xFF000000L)
    {
      *error = (int) dir->data.handle;
      free(dir);
      dir = NULL;
    }
  }
  return dir;
#else
  XDIR *dir;

  if ((dir = malloc(sizeof(XDIR))) == NULL)
    *error = ENSMEM;
  else
  {
    dir->path = (char *) path;
    dir->data.gdata.first = 1;
#ifndef __WATCOMC__
    dir->data.gdata.old_dta = Fgetdta();
    Fsetdta(&dir->data.gdata.dta);
#endif
    *error = 0;
  }
  return dir;
#endif
}

long x_xreaddir(XDIR *dir, char *buffer, int len, XATTR *attrib)
{
#ifdef _MINT_
  long error;
  int h;
  char *str;

  if ((str = malloc(len + 4)) != NULL)
  {
    if ((error = gemdos(0x129, len, dir->data.handle, str)) == 0)
    {
      strcpy(buffer, &str[4]);
    }
    free(str);

    if (error == 0)
    {
      char *name;

      if ((name = x_makepath(dir->path, buffer, &h)) != NULL)
      {
        error = x_attr(0, name, attrib);
        free(name);
      }
      else
      {
        error = (long) h;
      }
      return error;
    }
  }
  else
  {
    error = ENSMEM;
  }
  return error;

#else
  int error;

  if (dir->data.gdata.first != 0)
  {
    char *fspec;

    if ((fspec = make_path(dir->path, "*.*", &error)) != NULL)
    {
#ifndef __WATCOMC__
      error = Fsfirst(fspec, 0x37);
#else
      error = _dos_findfirst(fspec, 0x37, &dir->data.gdata.dta);
#endif
      free(fspec);
      dir->data.gdata.first = 0;
    }
  }
  else
  {
#ifndef __WATCOMC__
    error = Fsnext();
#else
      error = _dos_findnext(&dir->data.gdata.dta);
#endif
  }

#ifndef __WATCOMC__
  if (error == 0)
  {
#ifdef __GNUC__
    if (strlen(dir->data.gdata.dta.dta_name) + 1 > len)
      error = ERROR;
    else
    {
      strncpy(buffer, dir->data.gdata.dta.dta_name, len);
      dta_to_xattr(&dir->data.gdata.dta, attrib);
    }
#else
    if (strlen(dir->data.gdata.dta.d_fname) + 1 > len)
      error = ERROR;
    else
    {
      strncpy(buffer, dir->data.gdata.dta.d_fname, len);
      dta_to_xattr(&dir->data.gdata.dta, attrib);
    }
#endif
  }
#else
  if (error == 0)
  {
    if (strlen(dir->data.gdata.dta.name) + 1 > len)
      error = ERROR;
    else
    {
      strncpy(buffer, dir->data.gdata.dta.name, len);
      dta_to_xattr(&dir->data.gdata.dta, attrib);
    }
  }
#endif
  return (long) error;

#endif
}

long x_rewinddir(XDIR *dir)
{
#ifdef _MINT_
  return gemdos(0x12A, dir->data.handle);
#else
  dir->data.gdata.first = 1;
  return 0L;
#endif
}

long x_closedir(XDIR *dir)
{
#ifdef _MINT_
  long error;

  error = gemdos(0x12B, dir->data.handle);
  free(dir);
  return error;
#else
#ifndef __WATCOMC__
   Fsetdta( dir->data.gdata.old_dta);
#else
      _dos_findclose(&dir->data.gdata.dta);
#endif
  free(dir);
  return 0L;
#endif
}

#pragma warn -par

long x_attr(int flag, const char *name, XATTR *xattr)
{
#ifdef _MINT_
  return (int) gemdos(0x12C, flag, name, xattr);
#else
#ifndef __WATCOMC__
  DTA *olddta, dta;
#else
  struct find_t dta;
#endif
    int result;

#ifndef __WATCOMC__
  olddta = Fgetdta();
  Fsetdta( &dta);

  if ((result = Fsfirst(name, 0x37)) == 0)
  {
    dta_to_xattr(&dta, xattr);
  }
  Fsetdta( olddta);
#else
  if ((result = _dos_findfirst(name, 0x37, &dta)) == 0)
  {
    dta_to_xattr(&dta, xattr);
  }
  _dos_findclose(&dta);
#endif

  return result;
#endif
}

/*
 * Stack emulation.
 */

static int push(COPYDATA **stack, const char *path)
{
  COPYDATA *new;
  int error = 0;

  if ((new = malloc(sizeof(COPYDATA))) == NULL)
  {
    error = ENSMEM;
  }
  else
  {
    new->path = (char *) path;
    new->result = 0;
    if ((new->dir = x_opendir(path, &error)) != NULL)
    {
      new->prev = *stack;
      *stack = new;
    }
    else
    {
      free(new);
    }
  }
  return error;
}

static int pull(COPYDATA **stack, int *result)
{
  COPYDATA *top = *stack;

  x_closedir(top->dir);
  *result = top->result;
  *stack = top->prev;
  free(top);

  return (*stack == NULL) ? TRUE : FALSE;
}

static int stk_readdir(COPYDATA *stack, char *name, XATTR *attr, int *eod)
{
  int error;

  while (((error = (int) x_xreaddir(stack->dir, name, 256, attr)) == 0)
       && ((strcmp("..", name) == 0) || (strcmp(".", name) == 0)));

  if ((error == ENMFIL) || (error == EFILNF))
  {
    error = 0;
    *eod = TRUE;
  }
  else
  {
    *eod = FALSE;
  }

  return error;
}

void pack_file(const char *the_path, packstruct *com)
{
  long source_file;
  const char *the_name;
  
  if(xcommand.path)
  {
    the_name=the_path;
  }
  else
  {
    the_name=get_name(the_path);
  }
  printf("     %s\r", pnprint(xcommand.columns-5, the_name, xcommand.strbuf));

  if ((source_file = Fopen(the_path, FO_READ)) > 0)
  {
    long origsize;
    long file_pos;
    long len;
    int mode = com->mode;
    int attrib;
    DOSTIME tijd;
    
    xcommand.rc.handle=(int)source_file;
    origsize = Fseek(0, (int) source_file, 2);
    Fseek(0, (int)source_file, 0);
    attrib=Fattrib(the_path, 0, 0);     /* Get attributes */
    Fdatime(&tijd, (int) source_file, 0);  /* Get date and time */
    if(the_name[1]==':')
    { /*- als drive spec in de naam staat, gooi die dan weg */
      the_name+=3;
    }
    len = xcommand.headerlen(the_name, com);
    if (com->mv_mode)
    {
      int extend_file=0;
      /*
        hier geven we mee aan of de file extended moet zijn =0x08
        continued on next volume=0x04
      */
      do
      {
        unsigned long virtual_start=Fseek(0, (int) source_file, 1);
        if (com->mv_bytes_left<=(len+1))
        { /* we willen minstens 1 byte kunnen storen */
          /* multiple volume break! */
          xcommand.close_and_create_volume(com);
        }
        file_pos = buf_seek_write(0, 1, com->bw_buf);
        com->mv_bytes_left-=len; /* len bytes gebruikt */
        if(buf_write_announce(len, com->bw_buf)!=GUP_OK)
        {
          printf("\n\nDiskfull!!!\n");
          buf_close_write(com->bw_buf);
          exit(-1);
        }
        com->bw_buf->current+=len;
        xcommand.rc.crc=xcommand.init_crc();
        mode = com->mode;
        if (origsize)
        {
          if((origsize+(origsize>>6)+1024)<com->mv_bytes_left)
          { /*- past altijd, zet mv mode uit */
            gup_result err;
            com->mv_mode=0;
            init_progres(origsize);
            if((err=encode(com))!=GUP_OK)
            {
              display_error(err);
              exit(-1);
            }
            com->mv_mode=1; /* en weer aan... */
            com->mv_bytes_left-=com->packed_size;
          }
          else
          {
            gup_result err;
            init_progres(origsize);
            if((err=encode(com))!=GUP_OK)
            {
              display_error(err);
              exit(-1);
            }
          }
          if ((com->packed_size > com->bytes_packed) || (com->bytes_packed==0))
          { /* whoops! */
            com->mv_bytes_left+=com->packed_size;
            Fseek(virtual_start, (int)source_file, 0);
            xcommand.rc.crc=xcommand.init_crc();
            buf_seek_write(file_pos + len, 0, com->bw_buf);
            mode = 0;
            init_progres(origsize);
            {
              int oldmode=com->mode;
              gup_result err;
              com->mode=0;
              if((err=encode(com))!=GUP_OK)
              {
                display_error(err);
                exit(-1);
              }
              com->mode=oldmode;
            }
          }
          origsize-=com->bytes_packed;
          if(origsize && com->mv_next)
          {
            /* multiple volume break! */
            gup_result err;
            Fseek(virtual_start, (int) source_file, 0);
            xcommand.rc.crc=xcommand.init_crc();
            if((err=re_crc(com->bytes_packed, com))!=GUP_OK)
            {
              display_error(err);
              exit(-1);
            }
            Fseek(virtual_start+com->bytes_packed, (int)source_file, 0);
            printf("%3i%%\n", (int)((com->packed_size * 100) / com->bytes_packed));
            buf_seek_write(file_pos, 0, com->bw_buf);
            xcommand.make_header(the_name, com->bytes_packed, com->packed_size, virtual_start,
                                 mode, attrib, tijd.date, tijd.time,
                                 extend_file+4, com);
            buf_seek_write(file_pos + len + com->packed_size, 0, com->bw_buf);
            xcommand.close_and_create_volume(com);
            if(!extend_file)
            {
              extend_file=8;
              len+=4; /* aantal bytes nodig om file offset aan te geven */
            }
          }
          else
          {
            printf("%3i%%\n",(int)((com->packed_size * 100) / com->bytes_packed));
            Fclose((int) source_file);
            buf_seek_write(file_pos, 0, com->bw_buf);
            xcommand.make_header(the_name, com->bytes_packed, com->packed_size, virtual_start,
                             mode, attrib, tijd.date, tijd.time, extend_file, com);
            buf_seek_write(file_pos + len + com->packed_size, 0, com->bw_buf);
          }
        }
        else
        { /* file length is 0 */
          printf("100%%\n");
          Fclose((int) source_file);
          buf_seek_write(file_pos, 0, com->bw_buf);
          xcommand.make_header(the_name, 0, 0, 0, 0 /* mode */, 
                               attrib, tijd.date, tijd.time, extend_file, com);
          buf_seek_write(file_pos + len, 0, com->bw_buf);
          origsize=0;
        }
      }
      while(origsize);
    }
    else
    {
      file_pos = buf_seek_write(0, 1, com->bw_buf);
      if(buf_write_announce(len, com->bw_buf) != GUP_OK)
      {
        printf("\n\nDiskfull!!!\n");
        buf_close_write(com->bw_buf);
        exit(-1);
      }
      com->bw_buf->current += len;
      xcommand.rc.crc=xcommand.init_crc();

      if (origsize)
      {
        mode = com->mode;
        init_progres(origsize);
        {
          gup_result err;
          if((err=encode(com))!=GUP_OK)
          {
            display_error(err);
            exit(-1);
          }
        }
        if((com->packed_size>origsize) && (com->mode != GZIP))  /* whoops! */
        {
          Fseek(0, (int)source_file, 0);
          xcommand.rc.crc=xcommand.init_crc();
          buf_seek_write(file_pos + len, 0, com->bw_buf);
          mode = 0;
          {
            gup_result err;
            int tmp_mode= com->mode;
            com->mode=0;
            if((err=encode(com))!=GUP_OK)
            {
              display_error(err);
              exit(-1);
            }
            com->mode=tmp_mode;
          }
        }
        printf("%3i%%\n", (int)((com->packed_size * 100) / origsize));
      }
      else
      {
        printf("100%%\n");
        com->packed_size=0;
        mode = 0;
      }
      Fclose((int) source_file);
      if(com->mode == GZIP)
      { /* patch end of file stuff */

        if((com->bw_buf->end-com->bw_buf->current)<8)
        { /* flush buf to make space */
          gup_result res;
          res=com->buf_write_announce(8, com->bw_buf, com->bw_propagator);
          if(res!=GUP_OK)
          {
            display_error(res);
            exit(-1);
          }
        }

        {
          uint32 long crc = ~xcommand.rc.crc;
          uint8 *p = com->bw_buf->current;
          *p++=(uint8)(crc&0xff);
          crc>>=8;
          *p++=(uint8)(crc&0xff);
          crc>>=8;
          *p++=(uint8)(crc&0xff);
          crc>>=8;
          *p++=(uint8)(crc&0xff);
          crc=origsize;
          *p++=(uint8)(crc&0xff);
          crc>>=8;
          *p++=(uint8)(crc&0xff);
          crc>>=8;
          *p++=(uint8)(crc&0xff);
          crc>>=8;
          *p++=(uint8)(crc&0xff);
          com->bw_buf->current=p;
        }
      }
      buf_seek_write(file_pos, 0, com->bw_buf);
      xcommand.make_header(the_name, origsize, com->packed_size, 0, mode, 
                       attrib, tijd.date, tijd.time, 0, com);
      buf_seek_write(file_pos + len + com->packed_size, 0, com->bw_buf);
    }
  }
  else
  {
    printf("Source file open error\n");
  }
}

/*
 * Scan the specified path recursivly for files. Call the packer
 * for every file.
 */

int scan_dir(const char *file, packstruct *com)
{
  COPYDATA *stack = NULL;
  int ready = FALSE, eod = FALSE;
  int error, dummy;
  char name[256], *path, *filename;
  XATTR attr;

  filename = get_name(file);

  if (is_wildcard(filename))
  {
    if ((path = get_path(file, &error)) == NULL)
    {
      return error;
    }
  }
  else
  {
    if ((strlen(file) < 4) &&
        (cmp_wildcard(file, "?:") || cmp_wildcard(file, "?:\\")))
    {
      if ((path = strdup(file)) == NULL)
      {
        return ENSMEM;
      }
      filename = "*.*";
    }
    else
    {
      if ((error = (int) x_attr(0, file, &attr)) != 0)
      {
        return error;
      }

      if ((attr.mode & S_IFMT) == S_IFDIR)
      {
        if ((path = strdup(file)) == NULL)
        {
          return ENSMEM;
        }
        filename = "*.*";
      }
      else
      {
        pack_file(file, com);
        return 0;
      }
    }
  }

  if ((error = push(&stack, path)) != 0)
  {
    return error;
  }
  else
  {
    do
    {
      if (((error = stk_readdir(stack, name, &attr, &eod)) == 0) && (eod == FALSE))
      {
        if ((attr.mode & S_IFMT) == S_IFDIR)
        {
          if ((stack->name = make_path(stack->path, name, &error)) != NULL)
          {
            if ((error = push(&stack, stack->name)) != 0)
            {
              free(stack->name);
            }
          }
        }

        if ((attr.mode & S_IFMT) == S_IFREG)
        {
          if (cmp_wildcard(name, filename))
          {
            char *the_name;

            if ((the_name = make_path(stack->path, name, &error)) != NULL)
            {
              pack_file(the_name, com);
              free(the_name);
            }
          }
        }
      }

      if ((eod == TRUE) || (error != 0))
      {
        if ((ready = pull(&stack, &dummy)) == FALSE)
        {
          free(stack->name);
        }
      }
    }
    while (ready == FALSE);
  }

  free(path);

  return error;
}
 
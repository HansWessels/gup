/*
 * simple arj file buffering by Mr Ni!
 *
 * 25-12-1997
 * HWS: added routines for decompression engine
 *
 * 1997-11-08
 * HWS: added NEVER_USE(x) macro to surpress the warning: 
 *      never used parameter x in functions
 * 
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <tos.h>
#include "gup.h"
#include "gup_err.h"
#include "compr_io.h"
#include "file_buf.h"
#include "arjbeta.h"
#include "arj_outp.h"

 
typedef struct
{
  buf_fhandle_t xfile_buffer;
  int handle;     /* TOS filehandle */
  long pos;       /* filepos of start of buffer */
  int flushed;    /* has buffer been flushed on a seek back? 0=NOPE */
} pack_file_struct;

typedef struct
{
  buf_fhandle_t xfile_buffer;
  int handle;     /* TOS filehandle */
  long pos;       /* filepos of start of buffer */
  long buf_size;  /* buffer size */
} depack_file_struct;

static pack_file_struct pack_file;
static depack_file_struct depack_file;

/* buffer for compression */
buf_fhandle_t* buf_open_write(const char* naam /* file naam */
                             ,char* buf_start  /* start of file buffer, (must be !=NULL) */
                             ,long buf_size    /* size of file buffer (must be >0) */
                             ,int add_mode     /* =1: open for append, else create file */
                             )

{
  long res;
  if(add_mode==1)
  {
    if((res=Fopen(naam, FO_RW))<0)
    { /* error */
      if(res==-33)
      { /* didn't exist, create */
        if ((res = Fcreate(naam, 0)) < 0)
        {
          return NULL;
        }
      }
      else
      {
        return NULL;
      }
    }
  }
  else
  {
    if ((res = Fcreate(naam, 0)) < 0)
    {
      return NULL;
    }
  }
  pack_file.xfile_buffer.start=buf_start;
  pack_file.xfile_buffer.current=buf_start;
  pack_file.xfile_buffer.end=buf_start+buf_size;
  pack_file.handle=(int)res;
  pack_file.pos=0;
  pack_file.flushed=0;
  return &pack_file.xfile_buffer;
}

/* file close for packer */
gup_result buf_close_write(buf_fhandle_t* buf)
{
  pack_file_struct *com=(void*)buf;
  if((buf->current - buf->start)>0)
  { /* flush it! */
    long count=buf->current - buf->start;
    if (Fwrite(com->handle, count, buf->start) < count)
    {
      return GUP_WRITE_ERROR;
    }
    buf->current=buf->start;
  }
  {
    int res=Fclose(com->handle);
    if(res!=0)
    {
      printf("Error on Fclose!\n");
      return GUP_WRITE_ERROR;
    }
  }
  return GUP_OK;
}

/* buf_write announce */
gup_result buf_write_announce(long count, buf_fhandle_t* buf)
{
  if(count>=(buf->end-buf->current))
  { /* flush it! */
    pack_file_struct *com=(void*)buf;
    count=buf->current-buf->start;
    if (Fwrite(com->handle, count, buf->start) < count)
    {
      return GUP_WRITE_ERROR;
    }
    com->pos+=count;
    buf->current=buf->start;
  }
  return GUP_OK;
}

/* file seek */
/*
 * Waarschuwing: deze fileseek fucntie is erg gekoppeld aan
 * arjbeta.c en scan.c. Deze functie werkt alleen goed met deze 
 * functies!
 */
long buf_seek_write(long offset, int seekmode, buf_fhandle_t* buf)
{
  /*
   * GUP_SEEK_SET    0            Dateianfang
   * GUP_SEEK_CUR    1            aktuelle Position
   * GUP_SEEK_END    2            Dateiende
   */
  pack_file_struct *com=(void*)buf;
  if((seekmode==ARJ_SEEK_CUR) && (offset==ARJ_SEEK_SET))
  { /* get current pos */
    return com->pos+(buf->current - buf->start);
  }
  else
  {
    if((com->flushed==0) && (seekmode==ARJ_SEEK_SET) && (offset>=com->pos) 
       && (offset<=(com->pos+buf->end - buf->start)))
    { /* it's in range */
      buf->current = buf->start+offset-com->pos;
      return offset;
    }
    else
    { /* geen opti gevallen, strond aan de knikker */
      long old_pos=com->pos;
      if((buf->current - buf->start)>0)
      { /* flush it! */
        long count=buf->current - buf->start;
        if (Fwrite(com->handle, count, buf->start) < count)
        {
          display_error(GUP_WRITE_ERROR);
          exit(-1);
        }
        buf->current=buf->start;
      }
      com->pos = Fseek(offset, com->handle, seekmode);
      if(com->pos>=old_pos)
      {
        com->flushed=0;
      }
      else
      {
        com->flushed=1;
      }
      return com->pos;
    }
  }
}

/* speciale functie voor add mode */
long buf_seek_and_fill(long count, long offset, buf_fhandle_t* buf)
{
  pack_file_struct *com=(void*)buf;
  if((buf->current - buf->start)>0)
  { /* flush it! */
    long count=buf->current - buf->start;
    if (Fwrite(com->handle, count, buf->start) < count)
    {
      display_error(GUP_WRITE_ERROR);
      exit(-1);
    }
    buf->current=buf->start;
  }
  Fseek(offset, com->handle, 0);
  Fread(com->handle, count, buf->start);
  com->pos = Fseek(offset, com->handle, 0);
  return com->pos;
}



/* file open, for decompression */
buf_fhandle_t* buf_open_read(const char* naam, char* buf_start, long buf_size)
{
  long res;
  if((res=Fopen(naam, FO_READ))<0)
  { /* error */
    return NULL;
  }
  depack_file.xfile_buffer.start=buf_start;
  depack_file.xfile_buffer.current=buf_start;
  depack_file.xfile_buffer.end=buf_start;
  depack_file.handle=(int)res;
  depack_file.pos=0;
  depack_file.buf_size=buf_size;
  return &depack_file.xfile_buffer;
}

/* file seek, for decompression, fills buffer too */
long buf_seek_read(long offset, int seekmode, buf_fhandle_t* buf)
{
  /*
   * ARJ_SEEK_SET    0            Dateianfang
   * ARJ_SEEK_CUR    1            aktuelle Position
   * ARJ_SEEK_END    2            Dateiende
   */
  depack_file_struct *com=(void *)buf;
  if(seekmode==ARJ_SEEK_END)
  {
    long count;
    com->pos=Fseek(offset, com->handle, seekmode);
    buf->current=buf->start;
    count=Fread(com->handle, com->buf_size, buf->start);
    if(count<0)
    {
      printf("ARJ READ ERROR!\n");
      exit(-1);
    }
    else
    {
      buf->end=buf->start+count;
    }
    return com->pos;
  }
  else
  {
    long curpos=com->pos+(buf->current-buf->start);
    if(seekmode==1)
    {
      offset+=curpos;
    }
    if((offset<com->pos) || (offset>(com->pos+(buf->end-buf->start))))
    { /* seek before, or after */
      long count;
      com->pos=Fseek(offset, com->handle, 0);
      buf->current=buf->start;
      count=Fread(com->handle, com->buf_size, buf->start);
      if(count<0)
      {
        printf("ARJ READ ERROR!\n");
        exit(-1);
      }
      else
      {
        buf->end=buf->start+count;
      }
      return com->pos;
    }
    else
    {
      buf->current = buf->start+offset-com->pos;
      return offset;
    }
  }
}

/* buf fill, for decompression */
gup_result buf_fill(buf_fhandle_t* buf)
{
  depack_file_struct *com=(void *)buf;
  long count=buf->end-buf->current;

  com->pos+=buf->start-buf->current;
  memmove(buf->start, buf->current, count);
  count=com->buf_size-count;
  buf->end=buf->start+count;
  count=Fread(com->handle, count, buf->start);
  if(count<0)
  {
    return GUP_READ_ERROR;
  }
  buf->end=buf->start+count;
  buf->current=buf->start;
  return GUP_OK;
}

unsigned long buf_tell(buf_fhandle_t* buf)
{
  depack_file_struct* com= (void*)buf;
  return com->pos+(buf->current-buf->start);
}

uint8 bufget_byte(buf_fhandle_t* buf)
{
  if(buf->current>=buf->end)
  {
    buf_fill(buf);
  }
  return *buf->current++;
}

void buf_close_read(buf_fhandle_t* buf)
{
  depack_file_struct* com= (void*)buf;
  Fclose(com->handle);
}
 
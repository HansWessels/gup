/*
  Genereert de ARJ file headers
  1997-11-08
  HWS: added NEVER_USE(x) macro to surpress the warning: 
       never used parameter x in functions
  
*/


/***************************************************************************

 Structure of archive main header (low order byte first):

  2  header id (comment and local file) = 0x60, 0xEA
  2  basic header size (from 'first_hdr_size' thru 'comment' below)
       = first_hdr_size + strlen(filename) + 1 + strlen(comment) + 1
       = 0 if end of archive

  1  first_hdr_size (size up to 'extra data')
  1  archiver version number
  1  minimum archiver version to extract
  1  host OS   (0 = MSDOS, 1 = PRIMOS, 2 = UNIX, 3 = AMIGA, 4 = MACDOS)
               (5 = OS/2, 6 = APPLE GS, 7 = ATARI ST, 8 = NEXT)
               (9 = VAX VMS)
  1  arj flags (0x01 = GARBLED_FLAG, 0x02 = OLD_SECURED_FLAG)
               (0x04 = VOLUME_FLAG,  0x08 = EXTFILE_FLAG)
               (0x10 = PATHSYM_FLAG, 0x20 = BACKUP_FLAG)
               (0x40 = SECURED_FLAG)
  1  arj security version (2 = current)
  1  file type            (2 = comment header)
  1  ?                   ]
  4  date time stamp created
  4  date time stamp modified
  4  archive size up to the end of archive marker
  4  file position of security envelope data
  2  entryname position in filename
  2  length in bytes of trailing security data
  2  host data
  ?  extra data

  ?  archive filename (null-terminated)
  ?  archive comment  (null-terminated)

  4  basic header CRC

  2  1st extended header size (0 if none)
  ?  1st extended header
  4  1st extended header's CRC
  ...


 Structure of archive file header (low order byte first):

  2  header id (comment and local file) = 0x60, 0xEA
  2  basic header size (from 'first_hdr_size' thru 'comment' below)
       = first_hdr_size + strlen(filename) + 1 + strlen(comment) + 1
       = 0 if end of archive

  1  first_hdr_size (size up to 'extra data')
  1  archiver version number
  1  minimum archiver version to extract
  1  host OS   (0 = MSDOS, 1 = PRIMOS, 2 = UNIX, 3 = AMIGA, 4 = MACDOS)
               (5 = OS/2, 6 = APPLE GS, 7 = ATARI ST, 8 = NEXT)
               (9 = VAX VMS)
  1  arj flags (0x01 = GARBLED_FLAG, 0x02 = NOT USED)
               (0x04 = VOLUME_FLAG,  0x08 = EXTFILE_FLAG)
               (0x10 = PATHSYM_FLAG, 0x20 = BACKUP_FLAG)
               (0x40 = NOT USED)
  1  method    (0 = stored, 1 = compressed most ... 4 compressed fastest)
  1  file type (0 = binary, 1 = text, 2 = comment header, 3 = directory)
               (4 = label)
  1  garble password modifier
  4  date time stamp modified
  4  compressed size
  4  original size
  4  original file's CRC
  2  entryname position in filename
  2  file access mode                   Hier staan dus de flags in een woord
  2  host data
  ?  extra data
     4 bytes for extended file position

  ?  filename (null-terminated)
  ?  comment  (null-terminated)

  4  basic header CRC

  2  1st extended header size (0 if none)
  ?  1st extended header
  4  1st extended header's CRC
  ...
  ?  compressed file

     Time stamp format:

      31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16
     |<---- year-1980 --->|<- month ->|<--- day ---->|

      15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
     |<--- hour --->|<---- minute --->|<- second/2 ->|

*/


#include <time.h>
#include <string.h>
#include "gup.h"
#include "encode.h"
#include "port.h"
#include "file_buf.h"
#include "arj_hdr.h"
#include "arjbeta.h"

#if 0
#include "crc.h"
#include "scan.h"
#include "arj_outp.h"
#endif

long arj_headerlen(const char *naam, packstruct *com)
{
  NEVER_USE(com);
  return 4 + 30 + strlen(naam) + 2 + 4 + 2;
}


unsigned long arj_make_main_header(char *naam, uint8 flags, packstruct *com)
{ /* return waarde is de header size */

  long dt;
  long dtc;
  long len;
  { /* get filenaam */
    char *q;

    if ((q = strrchr(naam, '\\')) != NULL)
    {
      naam = q + 1;
    }
  }
  {
    len= 42+strlen(naam);
    if(len>=(com->bw_buf->end-com->bw_buf->current))
    { /* flush buf to make space */
      gup_result res;
      res=com->buf_write_announce(len, com->bw_buf, com->bw_propagator);
      if(res!=GUP_OK)
      {
        return res;
      }
    }
  }

  {
    uint8 *base = com->bw_buf->current;
    uint8 *p = base;

    *p++ = 0x60;                         /* header id                          */
    *p++ = 0xea;
    *p++ = 0;                            /* basic header size                  */
    *p++ = 0;
    *p++ = 30;                           /* 1st header size                    */
    *p++ = 6;                            /* archiver version number            */
    *p++ = 1;                            /* minimum archiver version to extract */
    *p++ = 0;                            /* HOST OS is MS WORST                */
    *p++ = flags;                        /* arj flags                          */
    *p++ = 0;                            /* security version                   */
    *p++ = 2;                            /* file type                          */
    dt = Tgetdate();
    dt <<= 16;
    dt += Tgettime();
    dtc = dt;
    *p++ = (uint8)dtc;                          /* reserved (extra garble addition dus) */
    *p++ = (uint8)dtc;                          /* time stamp                         */
    dtc >>= 8;
    *p++ = (uint8)dtc;
    dtc >>= 8;
    *p++ = (uint8)dtc;
    dtc >>= 8;
    *p++ = (uint8)dtc;
    *p++ = (uint8)dt;                           /* last changed time stamp            */
    dt >>= 8;
    *p++ = (uint8)dt;
    dt >>= 8;
    *p++ = (uint8)dt;
    dt >>= 8;
    *p++ = (uint8)dt;
    *p++ = 0;                            /* archive size                       */
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;                            /* security enveloppe file position   */
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;                            /* file spec position in file name    */
    *p++ = 0;
    *p++ = 0;                            /* length in bytes of security enveloppe data */
    *p++ = 0;
    *p++ = 0;                            /* not used                           */
    *p++ = 0;
    /* tot hier is 1st header size */
    strcpy(p, naam);                     /* filenaam of archive                */
    p = strend(p);
    *p++ = 0;                            /* str end */
    *p++ = 0;                            /* archive commend */
    {
      long length;
      unsigned long crc;

      length = p - base - 4;
      crc = -1;
      crc=~xcommand.rc.crc_func(base + 4, length, crc, xcommand.rc.crc_table);
      base[2] = length;
      length >>= 8;
      base[3] = length;
      *p++ = (uint8)crc;
      crc >>= 8;
      *p++ = (uint8)crc;
      crc >>= 8;
      *p++ = (uint8)crc;
      crc >>= 8;
      *p++ = (uint8)crc;
    }
    *p++ = 0;                            /* 1st extended header size */
    *p++ = 0;
    com->bw_buf->current= p;
    return p - base;
  }
}





gup_result arj_make_header(const char *naam, long origsize, long packedsize, long filestart,
                           int mode, int attrib, uint16 datum, uint16 tijd, 
                           uint8 flag, packstruct *com)
{
  int entry_pos = (int)strlen(naam);
  char *q = strend((char *)naam);
  long len;
  unsigned long filecrc = ~xcommand.rc.crc;

  while ((entry_pos) && (*q != '/') && (*q != '\\'))
  {
    entry_pos--;
    q--;
  }
  if ((*q == '/') || (*q == '\\'))
  {
    entry_pos++;
  }

  {
    len= 4 + 30 + strlen(naam) + 2 + 4 + 2;
    if (flag & 8)
    {
      len+=4; /* ext header pos */
    }
    if(len>=(com->bw_buf->end-com->bw_buf->current))
    { /* flush buf to make space */
      gup_result res;
      res=com->buf_write_announce(len, com->bw_buf, com->bw_propagator);
      if(res!=GUP_OK)
      {
        return res;
      }
    }
  }

  {
    uint8 *base = com->bw_buf->current;
    uint8 *p = base;
    *p++ = 0x60;                         /* header id */
    *p++ = 0xea;
    *p++ = 0;                            /* basic header size */
    *p++ = 0;
    if (flag & 8)
    {
      *p++ = 34;                         /* 1st header size includes ext filepos */
    }
    else
    {
      *p++ = 30;                         /* 1st header size */
    }
    *p++ = 6;                            /* archiver version number */
    *p++ = 1;                            /* minimum archiver version to extract */
    *p++ = 0;                            /* HOST OS is MS WORST */
    *p++ = (flag | 0x10);                /* arj flags, arj.exe zet altijd path sym flag, in het path wordt '\' dan '/', dat doen we dus */
    *p++ = mode;                         /* method */
    *p++ = 0;                            /* file type */
    *p++ = 0xe3;                         /* reserved (extra garble addition dus) */
    *p++ = (uint8)tijd;                         /* time stamp */
    tijd >>= 8;
    *p++ = (uint8)tijd;
    *p++ = (uint8)datum;
    datum >>= 8;
    *p++ = (uint8)datum;
    *p++ = (uint8)packedsize;                   /* compressed size */
    packedsize >>= 8;
    *p++ = (uint8)packedsize;
    packedsize >>= 8;
    *p++ = (uint8)packedsize;
    packedsize >>= 8;
    *p++ = (uint8)packedsize;
    *p++ = (uint8)origsize;                     /* original size */
    origsize >>= 8;
    *p++ = (uint8)origsize;
    origsize >>= 8;
    *p++ = (uint8)origsize;
    origsize >>= 8;
    *p++ = (uint8)origsize;
    *p++ = (uint8)filecrc;                      /* file crc */
    filecrc >>= 8;
    *p++ = (uint8)filecrc;
    filecrc >>= 8;
    *p++ = (uint8)filecrc;
    filecrc >>= 8;
    *p++ = (uint8)filecrc;
    *p++ = (uint8)entry_pos;                    /* file spec position in filename */
    entry_pos >>= 8;
    *p++ = (uint8)entry_pos;
    *p++ = (uint8)attrib;                       /* file attributes */
    attrib >>= 8;
    *p++ = (uint8)attrib;
    *p++ = 0;                            /* host data */
    *p++ = 0;
    if (flag & 8)
    {
      /* extended file */
      *p++ = (uint8)filestart;
      filestart >>= 8;
      *p++ = (uint8)filestart;
      filestart >>= 8;
      *p++ = (uint8)filestart;
      filestart >>= 8;
      *p++ = (uint8)filestart;
    }
    /* tot hier is 1st header size */
    strcpy(p, naam);                     /* file naam */
    while(*p!=0)
    {
      if(*p=='\\')
      {
        *p='/';
      }
      p++;
    }
    *p++ = 0;
    *p++ = 0;                            /* comment */
    {
      long length;
      unsigned long crc = -1;

      length = p - base - 4;
      crc = ~xcommand.rc.crc_func(base + 4, length, crc, xcommand.rc.crc_table);
      base[2] = length;
      length >>= 8;
      base[3] = length;
      *p++ = (uint8)crc;
      crc >>= 8;
      *p++ = (uint8)crc;
      crc >>= 8;
      *p++ = (uint8)crc;
      crc >>= 8;
      *p++ = (uint8)crc;
    }
    *p++ = 0;                            /* 1st extended header size */
    *p++ = 0;
    com->bw_buf->current=p;
  }
  return GUP_OK;
}





void arj_close_and_create_volume(packstruct *com)
{
  unsigned long filepos = buf_seek_write(0, 1, com->bw_buf);
  char *q;

  buf_seek_write(xcommand.main_header, 0, com->bw_buf);
  arj_make_main_header(xcommand.arj_naam, 4, com);  /* 4 is V flag zetten            */
  buf_seek_write(filepos, 0, com->bw_buf);
  com->buf_write_announce(4, com->bw_buf, NULL);
  *com->bw_buf->current++ = 0x60;
  *com->bw_buf->current++ = 0xea;
  *com->bw_buf->current++ = 0;
  *com->bw_buf->current++ = 0;
  buf_close_write(com->bw_buf);

  /* maak het volgende volume */
  xcommand.mv_nr++;
  com->mv_next = 0;
  if (xcommand.mv_size)
  {
    unsigned long free;
    DISKINFO di;

    do
    {
      com->mv_bytes_left=xcommand.mv_size;
      Dfree(&di, xcommand.mv_drive);
      /* diskfreeval = di.b_free * di.b_secsiz * di.b_clsiz; */
      free = (di.b_free * di.b_secsiz * di.b_clsiz);
      if (com->mv_bytes_left > free)
      {
        printf("Not enough space on disk in drive %c for next volume!\n", 'A' - 1 + xcommand.mv_drive);
        printf("\nPlease insert disk for next volume in drive %c.\n"
               "Then press any key to continue packing.\n", 'A' - 1 + xcommand.mv_drive);
        getch();
        free = 0;
      }
    }
    while (free == 0);
    com->mv_bytes_left -= 4;        /* eind header reserveren */
  }
  else
  {
    DISKINFO di;

    do
    {
      printf("\nPlease insert disk for next volume in drive %c.\n"
             "Then press any key to continue packing.\n", 'A' - 1 + xcommand.mv_drive);
      getch();
      Dfree(&di, xcommand.mv_drive);
      /* diskfreeval = di.b_free * di.b_secsiz * di.b_clsiz; */
      com->mv_bytes_left =
        di.b_free * di.b_secsiz * di.b_clsiz;
      if (com->mv_bytes_left < 8192)
      {
        printf("Not enough space on disk in drive %c for next volume!\n", 'A' - 1 + xcommand.mv_drive);
      }
    }
    while (com->mv_bytes_left < 8192);
    com->mv_bytes_left -= 4;        /* eind header reserveren */
  }
  q = xcommand.arj_naam;
  if ((q = strchr(q, '.')) == 0)
  {
    q = strend(xcommand.arj_naam);
  }
  #if 0
    sprintf(q, ".a%02i", xcommand.mv_nr); /* normal multiple volume */
  #else
    sprintf(q, ".%03x", xcommand.mv_nr); /* big multiple volume */
  #endif
  printf("Creating archive: %s\n", xcommand.arj_naam);
  {
    buf_fhandle_t* res;
    if ((res = buf_open_write(xcommand.arj_naam, com->buffer_start, 
                        com->buffer_size, 0))==NULL)
    {
      printf("ARJ file open error\n");
      exit(-1);
    }
    else
    {
      unsigned long hs;
      com->bw_buf=res;
      xcommand.main_header = buf_seek_write(0, 1, com->bw_buf);
      hs = arj_make_main_header(xcommand.arj_naam, 0, com);
      if (com->mv_bytes_left < hs)
      {
        printf("Error, not ennough space for mainheader!\n");
        exit(-1);
      }
      com->mv_bytes_left -= hs;
    }
  }
}

gup_result arj_close_archive(packstruct *com)
{ /* sluit archive af */
  gup_result res;
  if((res=com->buf_write_announce(4, com->bw_buf, NULL))==GUP_OK)
  {
    *com->bw_buf->current++ = 0x60;
    *com->bw_buf->current++ = 0xea;
    *com->bw_buf->current++ = 0;
    *com->bw_buf->current++ = 0;
    buf_close_write(com->bw_buf);
  }
  return res;
}
 
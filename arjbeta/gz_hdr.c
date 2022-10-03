/*
  Genereert de Gzip file headers
  1999-06-20
  HWS: added NEVER_USE(x) macro to surpress the warning: 
       never used parameter x in functions
  
*/


/***************************************************************************

 There is no main header

 Structure of archive file header (low order byte first):
 See also RFC 1952

  2  header id = 0x1f, 0x8b
  1  CM, Compression Method = 8 (deflate)
  1  FLG, Flag,
        1 = FTEXT, file could be ascii text
        2 = FHCRC, header crc present
        4 = FEXTRA, optional extra fields are present
        8 = FNAME, original filename is present
       16 = FCOMMENT, file comment is present
  4  MTIME, modification time, unix timestamp
  1  XFL, eXtra flags, for deflate: 2 = best compression, 4 = fast compression
  1  OS
        0 FAT filesystem
        1 Amiga
        2 VMS
        3 Unix
        4 VM/CMS
        5 Atari TOS
        6 HPFS
        7 Macintosh
        8 Z-system
        9 CP/M
       10 TOPS-20
       11 NTFS
       12 QDOS
       13 Acorn RISCOS
      255 Unknown
  optional, FLG.FEXTRA is set
    2 XLEN, extra len bytes
    XLEN, extra bytes
  optional, FLG.FNAME is set
    ? file name, zero terminated
  optional, FLG.FCOMMENT is set
    ? file comment, zero terminated
  optional, FLG.FHCRC is set, last to bytes of the CRC 32 of the complete fileheader
  ? compressed data
  4 CRC32 according to ISO 3309
  4 ISIZE, size of uncompressed data modulo 2^32
   
*/


#include <time.h>
#include <string.h>
#include "gup.h"
#include "encode.h"
#include "port.h"
#include "file_buf.h"
#include "gz_hdr.h"
#include "arjbeta.h"

#define strend(s)   (s + strlen(s))
#define GZ_ID1 0x1f
#define GZ_ID2 0x8b
#define GZ_CM_DEFLATE 8
#define GZ_FLG_FTEXT 1
#define GZ_FLG_FHCRC 2
#define GZ_FLG_FEXTRA 4
#define GZ_FLG_FNAME 8
#define GZ_FLG_FCOMMENT 16
#define GZ_XFL_DEFLATE_BEST 2
#define GZ_XFL_DEFLATE_FAST 4
#define GZ_OS_FAT 0
#define GZ_OS_AMIGA 1
#define GZ_OS_VMS 2
#define GZ_OS_UNIX 3
#define GZ_OS_VM_CMS 4
#define GZ_OS_ATARI 5
#define GZ_OS_HPFS 6
#define GZ_OS_MACINTOSH 7
#define GZ_OS_Z_SYSTEM 8
#define GZ_OS_CPM 9
#define GZ_OS_TOPS20 10
#define GZ_OS_NTFS 11
#define GZ_OS_QDOS 12
#define GZ_OS_ACORN_RISCOS 13
#define GZ_OS_UNKNOWN 255


long gz_headerlen(const char *naam, packstruct *com)
{
  int naam_len = 0;
  char tmp;

  NEVER_USE(com);
  
  while((tmp=naam[naam_len++]) != 0);
  {
    if((tmp=='/') || (tmp=='\\'))
    {
      naam_len=0;
    }
  }

  return 1 + 1 + 1 + 1 + 4 + 1 + 1 + naam_len;
}

unsigned long gz_make_main_header(char *naam, uint8 flags, packstruct *com)
{ /* return waarde is de header size */
  NEVER_USE(naam);
  NEVER_USE(flags);
  NEVER_USE(com);
  return 0;
}

gup_result gz_make_header(const char *naam, long origsize, long packedsize, long filestart,
                          int mode, int attrib, uint16 datum, uint16 tijd, 
                          uint8 flag, packstruct *com)
{
  long len;
  NEVER_USE(packedsize);
  NEVER_USE(mode);
  NEVER_USE(attrib);
  NEVER_USE(flag);
  NEVER_USE(datum);
  NEVER_USE(tijd);
  NEVER_USE(filestart);
  NEVER_USE(origsize);

  {
    len = gz_headerlen(naam, com);
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

    *p++ = GZ_ID1;                       /* ID1 */
    *p++ = GZ_ID2;                       /* ID2 */
    *p++ = GZ_CM_DEFLATE;                /* CM */
    *p++ = GZ_FLG_FNAME;                 /* FLG */
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;                            /* time, kuch */
    *p++ = GZ_XFL_DEFLATE_BEST;          /* XFL */
    *p++ = GZ_OS_ATARI;                  /* OS */
    {
      char tmp;
      const char *naam_pos=naam;
      int naam_len=0;
  
      while((tmp=naam[naam_len++]) != 0);
      {
        if((tmp=='/') || (tmp=='\\'))
        {
          naam_pos += naam_len;
          naam_len=0;
        }
      }
      strcpy(p, naam_pos);                     /* file naam */
      p+=naam_len;
    }
    com->bw_buf->current=p;
  }
  return GUP_OK;
}

gup_result gz_close_archive(packstruct *com)
{ /* sluit archive af */
  buf_close_write(com->bw_buf);
  return GUP_OK;
}
 
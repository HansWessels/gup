/*
  Genereert de LHA file headers
  1997-11-08
  HWS: added NEVER_USE(x) macro to surpress the warning: 
       never used parameter x in functions
  
*/

/*-

If this file contains incorrect information or some information is still
missing, please contact Christian_Grunenberg@S2.maus.de (Email). Anyway,
please distribute this file into any network.

  A. General archive structure:

     (SFX-Loader)
      ? loader (max. 4k)

     (Archive comment)
      5 '-com-'
      ? comment (max. 1k)
      1 \0

     File-headers (see below)
      ...

  B. Header Formats
     (integers are little-endian; i.e. low-order bytes come first)

-----------------------------------------------------------------------------
        level-0                 level-1                 level-2
-----------------------------------------------------------------------------
      1 header size           1 header size           2 total header size
      1 header sum            1 header sum
      5 method ID             5 method ID             5 method ID
B     4 packed size           4 skip size             4 packed size
      4 original size         4 original size         4 original size
A     2 time (MS-DOS)         2 time (MS-DOS)         4 time (UNIX style)
      2 date (MS-DOS)         2 date (MS-DOS)
S     1 MS-DOS attribute      1 fixed (0x20)          1 reserved (now 0x20)
      1 level (0x00)          1 level (0x01)          1 level (0x02)
I     1 name length           1 name length
      ? pathname              ? filename
C     (2 file crc)            2 file crc              2 file crc
      (1 OS ID ('U'/'K'))     1 OS ID                 1 OS ID
      (1 minor version)       . ........
      (4 last modified stamp)
      (2 Unix mode)
      (2 Unix uid)
      (2 Unix gid)
                              2 next header size      2 next header size
    *************************************************************************
     22 + ? no crc           27 + ?                  26
     24 + ? with crc
     36 + ? Unix/OS68k extension
-----------------------------------------------------------------------------
EX-                           1 ext-type              1 ext-type
TEN-                          . ........              . ........
SION                          2 next-header size      2 next-header size
-----------------------------------------------------------------------------

    Level-0 header:
      Same as LHarc and LArc.  Directory delimiter character is '\'.

    Level-1 header:
      Current default header is level 1

    Level-2 header:
      Future versions will use this header.

    b. Method IDs

      -lhd-   LHarc directory method

      -lh0-   no compression

      -lh1-   4k sliding dictionary(max 60 bytes) + dynamic Huffman
              + fixed encoding of position

      -lh2-   8k sliding dictionary(max 256 bytes) + dynamic Huffman

      -lh3-   8k sliding dictionary(max 256 bytes) + static Huffman

      -lh4-   4k sliding dictionary(max 256 bytes) + static Huffman
              + improved encoding of position and trees

      -lh5-   8k sliding dictionary(max 256 bytes) + static Huffman
              + improved encoding of position and trees

      -lh6-   8k sliding dictionary(max 256 bytes) + static Huffman
              + improved encoding of position and trees

      -lzs-   2k sliding dictionary(max 17 bytes)

      -lz4-   no compression

      -lz5-   4k sliding dictionary(max 17 bytes)

      -afx-   same as -lz5-

    c. OS ID

        MS-DOS  'M'
        OS/2    '2'
        OS9     '9'
        OS/68K  'K'
        OS/386  '3'
        HUMAN   'H'
        UNIX    'U'
        CP/M    'C'
        FLEX    'F'
        Mac     'm'
        Runser  'R'
        TownOS  'T'
        XOSK    'X'
        Amiga   'A'
        Atari   'a'

  C. Extension

    a. OS independent (0x00 - 0x3f)

        common header
         1 0x00
         2 header crc
       ( 1 information )
         2 next-header size

        filename header
         1 0x01
         ? filename
         2 next-header size

        dirname header
         1 0x02
         ? dirname (separated and terminated by 0xff)
         2 next-header size

        encryption header
         1 0x20
         1 flag (!=0 -> encryption)
         2 next-header size

         Encryption technique:
         ---------------------
         It is based on eXclusive ORing a variable-length key with the data;
         the same technique will recover the original data.
         For anyone wishing to add this feature to another archiver, the
         following fragment of C code demonstrates the technique used:

         char *key;            * ptr to start of variable-length key *
         char *keyptr;         * work ptr; NULL if no key *
          *
          * crypt - encryption/decryption routine
          *
         int crypt(int c)      * c is the CHARACTER to encode *
         {
            if (keyptr) {             * key supplied ? *
                if (!*keyptr)         * at end of key ? *
                    keyptr = key;     * yes - reinitialise *
                return c^*keyptr++;   * XOR it *
            }
            return c;                 * no key, no change *
         }

         Note that encryption takes place IMMEDIATELY before writing the
         archive (i.e. after compression), and decryption takes place
         IMMEDIATELY after reading the archive (i.e. before decompression).

        comment header
         1 0x3f
         ? comments
         2 next-header size

    b. OS dependent (0x40 - 0x7f)

        MS-DOS attribute header
         1 0x40
         2 file attribute
         2 next-header size

        Unix-Permission
         1 0x50
         2 permission
         2 next-header size

        Unix gid and uid
         1 0x51
         2 gid
         2 uid
         2 next-header size

        Unix group name
         1 0x52
         ? group name
         2 next-header size

        Unix user name
         1 0x53
         ? user name
         2 next-header size

        Unix last modified time
         1 0x54
         4 last modified time (Unix style)
         2 next-header size

Note: Archived files are no longer sorted alphabetically.

*/

#include "gup.h"
#include "encode.h"
#include "port.h"
#include "lha_hdr.h"
#include "arjbeta.h"
#include "file_buf.h"

unsigned long lha_make_main_header(char *naam, uint8 flags, packstruct *com)
{ /* return waarde is de header size */
  NEVER_USE(naam);
  NEVER_USE(flags);
  NEVER_USE(com);
  return 0;
}

long lha_headerlen(const char *naam, packstruct *com)
{
  NEVER_USE(com);
  return 25 + strlen(naam);
}


gup_result lha_make_header(const char *naam, long origsize, long packedsize, long filestart,
                     int mode, int attrib, uint16 datum, uint16 tijd, 
                     uint8 flag, packstruct *com)
{
  uint16 filecrc = xcommand.rc.crc;
  int len;

  NEVER_USE(flag);
  NEVER_USE(filestart);
  {
    len= 25+(int) strlen(naam);
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
    
    *p++=(char)(23+strlen(naam));   /* header size */
    *p++=0;                         /* header sum */
    switch(mode)
    {
    case STORE:
      strcpy(p, "-lh0-");           /* stored */
      break;
    case LHA_LH6_:
      strcpy(p, "-lh6-");           /* lh6'ed */
      break;
    case LHA_LH5_:
    default:
      strcpy(p, "-lh5-");           /* lh5'ed */
      break;
    }
    p+=5;
    *p++ = (uint8)packedsize;              /* compressed size */
    packedsize >>= 8;
    *p++ = (uint8)packedsize;
    packedsize >>= 8;
    *p++ = (uint8)packedsize;
    packedsize >>= 8;
    *p++ = (uint8)packedsize;
    *p++ = (uint8)origsize;                /* original size */
    origsize >>= 8;
    *p++ = (uint8)origsize;
    origsize >>= 8;
    *p++ = (uint8)origsize;
    origsize >>= 8;
    *p++ = (uint8)origsize;
    *p++ = (uint8)tijd;                    /* time stamp */
    tijd >>= 8;
    *p++ = (uint8)tijd;
    *p++ = (uint8)datum;
    datum >>= 8;
    *p++ = (uint8)datum;
    *p++ = attrib;                  /* file attributes */
    *p++= 0;                        /* level */
    *p++=(char)1+strlen(naam);      /* lengte naam */
    *p++='\\';
    strcpy(p, naam);                /* file naam */
    while(*p!=0)
    {
      if(*p=='\\')
      {
        *p='/';
      }
      p++;
    }
    *p++ = (uint8)filecrc;                 /* file crc */
    filecrc >>= 8;
    *p++ = (uint8)filecrc;
    {
      uint8 som=0;
      int i= (int)(23 + strlen(naam));
      uint8 *q=base+2;
      do
      {
        som+=*q++;
      }
      while(--i>0);
      base[1]=som;       /* header sum */
    }
    com->bw_buf->current= p;
  }
  return GUP_OK;
}


gup_result lha_close_archive(packstruct *com)
{ /* sluit archive af */
  if (com->buf_write_announce(1, com->bw_buf, NULL) != GUP_OK)
  {
    buf_close_write(com->bw_buf);
    return GUP_WRITE_ERROR;
  }
  *com->bw_buf->current=0;
  buf_close_write(com->bw_buf);
  return GUP_OK;
}
 
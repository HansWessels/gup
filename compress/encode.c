/********************************************************************
 *                                                                  *
 * Arj compression engine. Copyright (c) 1994 H. Wessels.           *
 *                                                                  *
 ********************************************************************
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:41:37 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 208 $
 * $Log$
 * Revision 1.25  2000/09/03 12:41:37  wout
 * Changed uint to unsigned int.
 *
 * Revision 1.24  2000/08/27 10:31:34  hans
 * Added LZS and LZ5 support.
 *
 * Revision 1.23  2000/08/05 16:03:37  hans
 * Removed warnings.
 *
 * Revision 1.22  2000/07/30 15:31:41  wout
 * Removed bug in flush_bitbuf.
 *
 * Revision 1.21  2000/07/28 20:26:46  hans
 * Some changes to prevent warnings from the compiler.
 *
 * Revision 1.20  2000/07/27 19:48:44  hwessels
 * Fixed bug in huffman code.
 *
 * Revision 1.19  2000/07/22 14:58:43  hwessels
 * *** empty log message ***
 *
 * Revision 1.18  2000/07/22 14:52:05  hwessels
 * *** empty log message ***
 *
 * Revision 1.17  2000/07/16 17:11:21  hwessels
 * Updated to GUP 0.0.4
 *
 * Revision 1.16  1998/12/27 20:43:14  klarenw
 * Updated makefiles. Changes for new error codes. Changes for LH6 mode.
 * settings.h is no longer used.
 *
 * Revision 1.15  1998/05/26 19:47:25  hwessels
 * Sources van Hans, 26-05-98.
 *
 * Revision 1.14  1998/03/21 14:26:33  hwessels
 * Sources van Hans, 21-03-1998.
 *
 * Revision 1.13  1998/01/03 22:45:57  hwessels
 * Sources van Hans, 03-01-98.
 *
 *
 * 22-12-1997 HWS
 * -fixed aligment of structs in the malloced buffer
 *
 * 01-11-1997 HWS:
 * -adapted source for file buffering routines
 * -changed length calculations for normal and mv mode
 * -re_crc uses now the buffer provided by getbuf
 *
 ********************************************************************/

/*-
  ARJ mode 1 packing
  (c) 1994 Mr Ni! (the Great) of the TOS-crew

  Originele tekst, ondanks een crash van de PC editor, met duizendmaal
  dank aan BUGABOO voor het weer vinden van deze tekst in het geheugen
  en het op harddisk pleuren.

  Ja, het codeer schema voor mode 1 is wat ingewikkelder dan dat voor
  mode 4.

  Huffman tabel generatie:
  code(0)=0, #0=0 Begin voorwaarden
  code(n+1)=(code(n)+#n)*2 met #n het aantal karakters met lengte n

  Structuur:
    huffman blok

    16 bits = blocksize #Huffman elementen in komend blok, 1-65535 (no typo)

    c_len coderings blok:
        lengte van de pointers die c_len coderen, er zijn 19 pointers:
            0          = c_len = 0
            1 + 4 bits = de volgende 3-18 karakters hebben lengte 0
            2 + 9 bits = de volgende 20-531 karakters hebben lengte 0
                         Waar de hell is 19 gebleven?? foutje?????
            3          = c_len = 1
            :
            n          = c_len = n-2
            :
            18         = c_len = 16

        5 bits = n, aantal te lezen pointerlengtes 0-19
            special case: n=0, er komt een 5bit waarde en dat is de enige
            c_len die zal voorkomen, hiermee kan de huffman tabel gemaakt
            worden, ptr_len=0, ptr_table is gevuld met deze 5 bit waarde
        n*[
            3 bits: 0-6 = lengte voor ptr[n]
                    7+[extra] = lengte is 7+1 voor iedere gezette bit die volgt
                                met een maximum van 13 bits, dan wordt er gestopt
                                maar helaas, een pointerlengte van 20 kan niet
                                voorkomen.
                                Denk dus aan de afsluitende 0-bit.
            Na 3 pointer lengtes:
            2 bits  = 0-3 aantal pointers dat op nul kan worden gezet, deze
                      tellen dus niet mee in het aantal te lezen pointers
                      Dit is dus de codering voor de eerste drie echte lengtes
                      en die zullen inderdaad vaak 0 zijn
           ]
        De resterende pointer lengtes worden op nul gezet. Maak nu een huffman
        tabel met de verkregen lengtes en gebruik de huffman codering om
        c_len door te geven
    EINDE c_len coderings blok

    c_len blok:
        Nu komt het blok met de lengte voor iedere karakter, gecodeerd met de
        zojuist gecodeerde pointers
        lengte van de pointers die c_len coderen, er zijn 19 pointers:
            0          = c_len = 0
            1 + 4 bits = de volgende 3-18 karakters hebben lengte 0
                         19=18+een maal 0
            2 + 9 bits = de volgende 20-532 karakters hebben lengte 0
            3          = c_len = 1
            :
            n          = c_len = n-2
            :
            18         = c_len = 16
        9 bits = n, aantal karakter lengtes dat eraankomt 0-510
            0 = special case, 9 bits geven het enige karakter aan dat voorkomt
                c_len van dit karakter is 0, en de hele tabel kan worden gevuld
                met dit karakter, maak huffman tabel kan dus worden overgeslagen
        n*[
            huffamn code van ptr, verwerk volgens bovenstaande tabel
          ]
        De overblijvende karakterlengtes zijn alle 0, nu kunnen we de
        huffman tabel voor de karakters maken.
    EINDE c_len blok

    ptr_len blok:
        Als laatste komen de pointerlengtes.
        er zijn 17 verschillende pointers:
        0           = 0
        1           = 1
        2+ 1 bit    = 2, 3
        3+ 2 bits   = 4 t/m 7
        :
        n+ n-1 bits = 2**(n-1) t/m 2**n -1
        :
        16+ 15 bits = 32768 t/m 65535, de sliding dictionary gaat maar t/m 26623
                      26623 = 79*337, geen logica voor mij... ofwel 26k-1
        5 bits = n, aantal pointers dat nu komt
            n=0 is special case, er is maar 1 pointerlengte, 5 bits geven deze
            pointer
        n*[
            3 bits = len 0-6
                7 [+extra] = len = 7+1 voor elke volgende bit die 1 is, max 13
                             bits, max ptr len is echter 16
          ]
        rest van de pointers is nul, met deze pointer lengtes kan de huffman
        tabel voor de pointers gemaakt worden
    EINDE ptr_len blok

    blocksize*[
                huffman code karakter+[ptr]
              ]
  Q.E.D. (Quite Easily  explaineD) ;-)
*/


/*-
 * ARJ mode 4 packing
 *
 * (c) 1993 Mr Ni! (the Great) of the TOS-crew
 *
 * codeer schema: [len (literal 8 bits | ptr_code)]
 *
 * len codering: w bits:
 *
 * 0 8               :  literal
 *
 * 1 10x             :  1 -   2
 *
 * 2 110xx           :  3 -   6
 *
 * 3 1110xxx         :  7 -  14
 *
 * 4 11110xxxx       : 15 -  30
 *
 * 5 111110xxxxx     : 31 -  62
 *
 * 6 1111110xxxxxx   : 63 - 126
 *
 * 7 1111111xxxxxxx  :127 - 254
 *
 * Hierbij moet nog twee worden opgeteld, dus minimale lengte 3, maximaal
 * 256!!!
 *
 * ptr codering: w bits
 *
 * 9  0xxxxxxxxx            0 -   511
 *
 * 10 10xxxxxxxxxx        512 -  1535
 *
 * 11 110xxxxxxxxxxx     1536 -  3583
 *
 * 12 1110xxxxxxxxxxxx   3584 -  7679
 *
 * 13 1111xxxxxxxxxxxxx  7680 - 15871
 *
 * codering dus van 0 - 15871
 */

/*-
 * Methode om bloklengte te berekenen:
 * neem een 4k blok
 * bereken #header bits
 * bereken #message bits
 * count = 1
 * while(;;)
 * {
 *   neem het volgende 4k blok, bereken #message bits volgens huidige huffman code
 *   als abs(#messagebits/count-newmessage bits)<#headerbits
 *     voeg 4k blok toe aan te packen blok en bereken #messagebits en #headerbits
 *     count++
 *   anders
 *     break
 * }
 * alternatief:
 * count=1
 * while(;;)
 * {
 *   count++
 *   neem een count * 4k blok
 *   bereken #messagebits
 *   bereken #headerbits
 *   bekijk het laatste 4k blok apart
 *   bereken #nieuwmessagebits
 *   bereken #nieuwheaderbits
 *   stop zodra (#messagebits+#headerbits)/count>(#nieuw messagebits+nieuwheaderbits)
 * }
*/

#undef NDEBUG
#if 01
#define NDEBUG /* no debugging */
#endif

#if 0
  /* log literal en pointer len combi's */
  #define LOG_LITERAL(lit)  printf("Literal: %02X\n", lit);
  #define LOG_LITERAL_RUN(len)  printf("Literal run: %u\n", len);
  #define LOG_PTR_LEN(len, ptr) printf("Len: %u, ptr: %u\n",len, ptr);
  #define LOG_bit(bit) printf("bit = %i\n",bit);
#else
  #define LOG_LITERAL(lit) /* */
  #define LOG_LITERAL_RUN(len) /* */
  #define LOG_PTR_LEN(len, ptr) /* */
  #define LOG_bit(bit) /* */
#endif


/*
 * position of print progress update
 * When PP_AFTER is defined then print progress is updated when bytes
 * actually are stored.
 * else print progress is updated when bytes are being read to be processed
 *
 * default is undefined, in -jm mode a 64k huffman buffer is used and a large
 * part of the file can be buffered (compressed) in that huffman buffer before
 * the percentage counter is updated
 */
#undef PP_AFTER

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "gup.h"
#include "compress.h"
#include "evaluatr.h"
#include "encode.h"



/* eerst ff wat definities */

void init_bitbuffer(packstruct *com);
gup_result flush_n1_bitbuf(packstruct *com);
gup_result flush_bitbuf(packstruct *com);
gup_result store(packstruct *com);
gup_result announce(unsigned long bytes, packstruct *com);     /* kondigt aantal bytes in huffblok aan */
gup_result compress_chars(packstruct *com); /* maakt huffman tabellen */
gup_result compress_m4(packstruct *com);
gup_result compress_n1(packstruct *com);
gup_result compress_lzs(packstruct *com);
gup_result compress_lz5(packstruct *com);
unsigned long count_bits(unsigned long* header_size, unsigned long* message_size,
                         unsigned long* packed_bytes, int charct, uint16 entries,
                         uint8* charlen, uint8* ptrlen, uint16* charfreq, uint16* ptrfreq, packstruct *com);
unsigned long count_m4_bits(unsigned long* packed_bytes,
                            packstruct *com,
                            uint16 entries, c_codetype *chars,
                            pointer_type *ptrs);
unsigned long count_n1_bits(unsigned long *packed_bytes,  /* aantal bytes dat gepacked wordt */
                            uint16 entries, /* aantal character die moeten worden gepacked */
                            c_codetype * p, /* pointer naar de karakters     */
                            pointer_type * q);

void make_hufftable(uint8* len, uint16* tabel, uint16* freq, uint16 totaalfreq, int nchar, int max_hufflen, packstruct *com); /* maakt huffman tabel */
void make_huffmancodes(uint16* table, uint8* len, int nchar); /* maakt de huffman codes */

/*
 * ALIGN is a macro to align elements of a certain size in a malloced
 * array. The start of the array is guaranteed aligned with every possible 
 * object. So an object is aligned when it's position relative to the start
 * of the array is a multiple of it's size. Instead of the modulo operator 
 * the & operator is used. This assumes that the alignment of an object 
 * is the smalest power of two of its size; ie: sizeof(object)==3 -> alignment
 * is 1. sizeof(object)==6 ->alignment is 2.
 */
#define ALIGN(start,ptr,size) ptr+=((size)-((ptr)-(start)))&((size)-1)

#ifndef ALIGN_BUFP
  /*
   * x=packstruct pointer.
   * Function to allign buffer_pointer pointers, needed in some optimisations of
   * storebits, only align when in the packstruct use_align!=0 
   */
  #define ALIGN_BUFP(x) /* */
#endif

#ifndef NDEBUG
FILE* redir = stderr;
#define ARJ_Assert(expr)   \
  ((void)((expr)||(fprintf(redir, \
  "\nAssertion failed: %s, file %s, line %d\n",\
   #expr, __FILE__, __LINE__ ),\
   ((int (*)(void))abort)())))
#else
#define ARJ_Assert( expr )
#endif

#ifndef NOT_USE_STD_free_encode
void free_encode(packstruct *com)
{
  if (com->bufbase)
  {
    com->gfree(com->bufbase, com->gf_propagator);
    com->bufbase=NULL;
  }
}
#endif

uint8 *get_buf(unsigned long *buflen, packstruct *com) /* geeft begin adres en lengte van buffer, result is NULL als er geen buffer is */
{
  if(com->bufbase!=NULL)
  {
    *buflen=com->buffer_start-com->dictionary;
    return com->dictionary;
  }
  else
  {
    return NULL;
  }
}


#ifndef NOT_USE_STD_init_encode

/*
  Alignement problemen bij de Alpha...
  Deze functie malloct een memory blok en zet daar pointers in naar diverse
  typen. Probleem is dat hier een hard coded alignment werd gebruikt op 
  word en longword boundaries. De macro ALIGN zorgt ervoor dat de alignment 
  machine onafhankelijk gefixed wordt. (Hopelijk.)
*/

gup_result init_encode(packstruct *com)
{
  void (*i_fastlog)(packstruct *com);
  com->compress=compress_chars;
  com->max_match = 256;          /* grootte van max_match voor Junk & LHA */
  com->use_align=1;              /* use align macro */
  com->flush_bitbuf=flush_bitbuf; /* use flush_bitbuf() by default */
  switch(com->mode)
  {
    case ARJ_MODE_1:
    case ARJ_MODE_2:
    case ARJ_MODE_3:
    default:
      com->n_ptr=ARJ_NPT;
      com->m_ptr_bit=ARJ_PBIT;
      com->maxptr= MAX_PTR;
      i_fastlog=init_fast_log;
      break;
    case ARJ_MODE_4:
      com->maxptr= MAX_M4_PTR;
      com->compress=compress_m4;
      i_fastlog=init_m4_fast_log;
      break;
    case NI_MODE_1:
      com->maxptr= MAX_M4_PTR;
      com->compress=compress_n1;
      i_fastlog=init_n1_fast_log;
      com->flush_bitbuf=flush_n1_bitbuf; /* use flush_n1_bitbuf() */
      break;
    case GNU_ARJ_MODE_7:
      com->n_ptr=ARJ_NPT;
      com->m_ptr_bit=ARJ_PBIT;
      i_fastlog=init_fast_log;
      com->maxptr= 65534UL;
      /*
      // Voor mode 7 moet dit 65535 zijn, maar op deze positie 65535 wordt de 
      // nieuwe node geinsert, daarom met deze 1 kleiner worden genomen 
      // Als we eerst matchen, dan deleten, en dan pas inserten kunnen we deze
      // positie ook gebruiken...
      */
      break;
    case LHA_LZS_:
      com->maxptr= MAX_LHA_LZS_PTR;
      com->compress=compress_lzs;
      com->max_match = 17;          /* grootte van max_match voor LHA_LZx_ */
      i_fastlog=init_lzs_fast_log;
      com->use_align=0;             /* do NOT use align macro */
      break;
    case LHA_LZ5_:
    case LHA_AFX_:
      com->maxptr= MAX_LHA_LZ5_PTR;
      com->compress=compress_lz5;
      com->max_match = 18;          /* grootte van max_match voor LHA_LZx_ */
      i_fastlog=init_lz5_fast_log;
      com->use_align=0;             /* do NOT use align macro */
      break;
    case LHA_LH4_:
      com->n_ptr=LHA_NPT;
      com->m_ptr_bit=LHA_PBIT;
      com->maxptr=4095;
      i_fastlog=init_fast_log;
      break;
    case 8:
    case LHA_LH5_:
      com->n_ptr=LHA_NPT;
      com->m_ptr_bit=LHA_PBIT;
      com->maxptr=8191;
      i_fastlog=init_fast_log;
      break;
    case LHA_LH6_:
      com->n_ptr=ARJ_NPT;
      com->m_ptr_bit=ARJ_PBIT;
      i_fastlog=init_fast_log;
      com->maxptr= 32767UL;
      break;
  }
  if (com->mode > 0)
  {
    int jm=com->jm;
    for (;;)
    {
      unsigned long memneed = 0;
      long buf;
      int zeef = 0;
      int link = 0;

      memneed += (DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL) * sizeof (uint8);  /* sliding dictionary + 4 voor terugkijken */
      ALIGN(0, memneed, sizeof(node_struct)); /* align on node struct */
      memneed += sizeof (node_struct) * TREE_SIZE; /* sld tree        */
      ALIGN(0, memneed, sizeof(node_type)); /* align on node_type */
      memneed += sizeof (node_type) * HASH_SIZE;  /* normal root */

      memneed += FASTLOGBUF;           /* grootte fastlog buffer */

      ALIGN(0, memneed, sizeof(unsigned long)); /* unsigned long is used for writing bitbuffer into huffman buffer */
      memneed += 4UL*BIG_HUFFSIZE;
      ALIGN(0, memneed, sizeof(uint8));
      memneed += sizeof (uint8) * (NC + NC); /* karakter lengte */
      ALIGN(0, memneed, sizeof(uint16));
      memneed += sizeof (uint16) * (NC);  /* huffman codes van de karakters */
      ALIGN(0, memneed, sizeof(uint8));
      memneed += sizeof (uint8) * (MAX_NPT + MAX_NPT); /* pointer lengte */
      ALIGN(0, memneed, sizeof(uint16));
      memneed += sizeof (uint16) * (MAX_NPT); /* huffman codes van de pointers */
      ALIGN(0, memneed, sizeof(uint8));
      memneed += sizeof (uint8) * (NCPT + NCPT); /* pointer lengte */
      ALIGN(0, memneed, sizeof(uint16));
      memneed += sizeof (uint16) * (NCPT);  /* huffman codes van de pointers */
      if (jm)
      {
        buf = HUFFBUFSIZE;
      }
      else
      {
        buf = HUFFSIZE;
      }
      ALIGN(0, memneed, sizeof(c_codetype));
      memneed += buf * sizeof (c_codetype);
      ALIGN(0, memneed, sizeof(pointer_type));
      memneed += buf * sizeof (pointer_type);
      if (com->speed<2)
      { /* linking */
        link = 1;
        ALIGN(0, memneed, sizeof(node_type));
        memneed += sizeof (node_type) * HASH2_SIZE; /* rle root */
        ALIGN(0, memneed, sizeof(hist_struct)); /* history is a array of hist_struct */
        memneed += sizeof (history) * HISTSIZE;     /* match history buffer */
        ALIGN(0, memneed, sizeof(node_type));
        memneed += TREE_SIZE * sizeof (node_type);  /* buffer voor link */
        if (com->speed==0)
        { /* zeef 34 */
          zeef = 1;
          ALIGN(0, memneed, sizeof(uint8));
          memneed += buf * 5;
        }
      }
      com->bufbase = com->gmalloc(memneed, com->gm_propagator);
      if (com->bufbase == NULL)
      {
        return GUP_NOMEM;
      }
      else
      {
        uint8 *cp = com->bufbase;
        uint8 *base = cp;

        /* de copy buffer, die kan worden opgevraagd met de functie get_buf(); start hier met com->dictionary */

        com->dictionary = (void *)(cp + DICTIONARY_OFFSET);  /* DICTIONARY_OFFSET extra om terug te kunnen kijken */
        cp += (DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL) * sizeof (uint8); /* sliding dictionary + 4 voor terugkijken */
        ALIGN(base, cp, sizeof(node_type));
#ifndef INDEX_STRUCT
        com->root.big = (void *)cp;
#else
        com->root.big = ((int32 *)cp)-((int32*)base);
#endif
        cp += sizeof (node_type) * HASH_SIZE; /* normal root */
        if (link)
        {
          ALIGN(base, cp, sizeof(node_type));
#ifndef INDEX_STRUCT
          com->root2.big = (void *)cp;
#else
          com->root2.big = ((int32 *)cp)-((int32*)base);
#endif
          cp += sizeof (node_type) * HASH2_SIZE;  /* rle root */
          ALIGN(base, cp, sizeof(hist_struct)); /* history is a array of hist_structs */
          com->hist=(void *)cp;          /* match history buffer */
          cp += sizeof (history) * HISTSIZE;
          ALIGN(base, cp, sizeof(node_type));
#ifndef INDEX_STRUCT
          com->link.big = (void *)(cp);
#else
          com->link.big = ((int32 *)cp)-((int32*)base);
#endif
          cp += TREE_SIZE * sizeof (node_type); /* buffer voor link */
        }
        else
        {
#ifndef INDEX_STRUCT
          com->link.big = NULL;
#else
          com->link.big = 0;
#endif
        }
        ALIGN(base, cp, sizeof(unsigned long)); /* align on unsigned long for output buffer */
        com->buffer_start=(void*)cp;

        /* de copy buffer loopt tot hier, com->buffer_start is het eind adres */
        com->buffer_size=4UL*BIG_HUFFSIZE;
        cp += com->buffer_size;
        ALIGN(base, cp, sizeof(node_struct));
#ifndef INDEX_STRUCT
        com->tree.big = (void *)cp;
#else
        com->tree.big = ((int32 *)cp)-((int32*)base);
#endif
        memset(cp, 0, sizeof (node_struct) * TREE_SIZE); /* wis geheugen */
        cp += sizeof (node_struct) * TREE_SIZE;  /* sld tree          */
        ALIGN(base, cp, sizeof(uint8));
        com->charlen = (void *)cp;
        cp += sizeof (uint8) * (NC + NC);  /* karakter lengte */
        ALIGN(base, cp, sizeof(uint16));
        com->char2huffman = (void *)cp;
        cp += sizeof (uint16) * (NC);  /* huffman codes van de karakters */
        ALIGN(base, cp, sizeof(uint8));
        com->ptrlen = (void *)cp;
        cp += sizeof (uint8) * (MAX_NPT + MAX_NPT);  /* pointer lengte */
        ALIGN(base, cp, sizeof(uint16));
        com->ptr2huffman = (void *)cp;
        cp += sizeof (uint16) * (MAX_NPT); /* huffman codes van de pointers */
        ALIGN(base, cp, sizeof(uint8));
        com->ptrlen1 = (void *)cp;
        cp += sizeof (uint8) * (NCPT + NCPT);  /* pointer lengte */
        ALIGN(base, cp, sizeof(uint16));
        com->ptr2huffman1 = (void *)cp;
        cp += sizeof (uint16) * (NCPT);/* huffman codes van de pointers */
        ALIGN(base, cp, sizeof(c_codetype));
        com->chars = (void *)(cp);
        cp += buf * sizeof (c_codetype);
        com->hufbufsize = (uint16)(buf - 4UL);/* voor doorschot en pointerswap */
        ALIGN(base, cp, sizeof(pointer_type));
        com->pointers = (void *)(cp);
        cp += buf * sizeof (pointer_type);
        if (zeef)
        {
          ALIGN(base, cp, sizeof(uint8));
          com->matchstring = cp;
          cp += buf * 4UL;
          com->backmatch = cp;
          cp += buf;
        }
        else
        {
          com->matchstring = NULL;
        }

        ALIGN(base, cp, sizeof(uint8));
        com->fast_log = cp;
        cp += FASTLOGBUF;
        (*i_fastlog)(com);
        com->tree_size=TREE_SIZE;  /* gewone 64k tree */
        com->small_code=0;       /* geen small code dus */
        ARJ_Assert((com->bufbase+memneed)==cp); /* al het geheugen gebruiken, nix meer en nix minder */
        NEVER_USE(cp); /* shut up some compilers */
        return GUP_OK; /* succes */
      }
    }
  }
  else
  {
    unsigned long memneed = 65536UL;
    com->bufbase = com->gmalloc(memneed, com->gm_propagator);
    com->dictionary=NULL;

    if (com->bufbase == NULL)
    {
      return GUP_NOMEM;
    }
    else
    {
      uint8 *cp = com->bufbase;
      com->buffer_start=(void*)cp;
      com->buffer_size=65536UL;
      cp += com->buffer_size;
      NEVER_USE(cp); /* shut up some compilers */
    }
  }
  return GUP_OK; /* succes */
}

#endif


#ifndef NOT_USE_STD_store

gup_result store(packstruct *com)
{
  if(com->mv_mode)
  {
    signed long bytes_read;
    do
    {
      unsigned long bytes_left=com->bw_buf->end - com->bw_buf->current;
      if (bytes_left == 0)
      {
        gup_result res;
        res=com->buf_write_announce(com->bw_buf->current-com->bw_buf->start, com->bw_buf, com->bw_propagator);
        if(res!=GUP_OK)
        {
          return res;
        }
        bytes_left=com->bw_buf->end - com->bw_buf->current;
      }
      if(bytes_left>com->mv_bytes_left)
      {
        bytes_left=com->mv_bytes_left;
      }
      if((bytes_read=com->buf_read_crc(bytes_left, com->bw_buf->current, com->brc_propagator))<0)
      {
        return GUP_READ_ERROR; /* ("read error"); */
      }
      if(bytes_read!=0)
      {
        #ifndef PP_AFTER
        com->print_progres(bytes_read, com->pp_propagator);
        #endif
        com->bw_buf->current += bytes_read;
        com->bytes_packed += bytes_read;
        if((com->mv_bytes_left -= bytes_read)==0)
        {
          com->mv_next=1;
          break;
        }
      }
    }
    while (bytes_read!=0);
  }
  else
  {
    signed long bytes_read;
    do
    {
      unsigned long bytes_left=com->bw_buf->end - com->bw_buf->current;
      if (bytes_left == 0)
      {
        gup_result res;
        res=com->buf_write_announce(com->bw_buf->current-com->bw_buf->start, com->bw_buf, com->bw_propagator);
        if(res!=GUP_OK)
        {
          return res;
        }
        bytes_left=com->bw_buf->end - com->bw_buf->current;
      }
      if((bytes_read=com->buf_read_crc(bytes_left, com->bw_buf->current, 
                                       com->brc_propagator))<0)
      {
        return GUP_READ_ERROR; /* ("read error"); */
      }
      if(bytes_read!=0)
      {
        #ifndef PP_AFTER
        com->print_progres(bytes_read, com->pp_propagator);
        #endif
        com->bw_buf->current += bytes_read;
        com->bytes_packed += bytes_read;
      }
    }
    while (bytes_read!=0);
  }
  com->packed_size=com->bytes_packed;
  return GUP_OK; /* succes */
}

#endif

#ifndef NOT_USE_STD_encode

gup_result encode(packstruct *com)
{
	TRACE_ME();
  gup_result res;
  com->bytes_packed=0;
  if((com->mode==STORE) || (com->mode==LHA_LH0_))
  {
    res=store(com);
  }
  else
  {
    com->rbuf_current=com->bw_buf->current;
    com->rbuf_tail=com->bw_buf->end;
    com->mv_bits_left=0;
    if(com->small_code==0)
    {
      res=encode_big(com);
    }
    else
    { /* this should not happen */
      res=GUP_INTERNAL;
    }
    com->bw_buf->current=com->rbuf_current;
  }
  return res;
}

#endif


#ifndef NOT_USE_STD_flush_bitbuf

gup_result flush_bitbuf(packstruct *com)
{
  if (com->bits_in_bitbuf>0)
  {
    int bytes_extra=(com->bits_in_bitbuf+7)>>3;
    if ((com->rbuf_current+bytes_extra) >= com->rbuf_tail)
    {
      gup_result res;
      com->bw_buf->current=com->rbuf_current;
      res=com->buf_write_announce(bytes_extra, com->bw_buf, com->bw_propagator);
      if(res!=GUP_OK)
      {
        return res;
      }
      com->rbuf_current=com->bw_buf->current;
      com->rbuf_tail=com->bw_buf->end;
      ALIGN_BUFP(com);
    }
    if(bytes_extra>0)
    {
      *com->rbuf_current++ = (uint8) (com->bitbuf >> 24);
      bytes_extra--;
    }
    if(bytes_extra>0)
    {
      *com->rbuf_current++ = (uint8) (com->bitbuf >> 16);
      bytes_extra--;
    }
    if(bytes_extra>0)
    {
      *com->rbuf_current++ = (uint8) (com->bitbuf >> 8);
      bytes_extra--;
    }
    if(bytes_extra>0)
    {
      *com->rbuf_current++ = (uint8) (com->bitbuf);
    }
  }
  return GUP_OK;
}

#endif

#ifndef NOT_USE_STD_flush_n1_bitbuf

gup_result flush_n1_bitbuf(packstruct *com)
{
  if (com->bits_in_bitbuf>0)
  {
  	 com->bitbuf=com->bitbuf<<(8-com->bits_in_bitbuf);
    *com->command_byte_ptr=(uint8)com->bitbuf;
    com->bits_in_bitbuf=0;
  }
  return GUP_OK;
}

#endif

#ifndef NOT_USE_STD_announce

gup_result announce(unsigned long bytes, packstruct *com)
{
  if((com->rbuf_current+bytes)>=com->rbuf_tail)
  {
    gup_result res;
    com->bw_buf->current=com->rbuf_current;
    res=com->buf_write_announce(bytes, com->bw_buf, com->bw_propagator);
    if(res!=GUP_OK)
    {
      return res;
    }
    com->rbuf_current=com->bw_buf->current;
    com->rbuf_tail=com->bw_buf->end;
    ALIGN_BUFP(com);
  }
  return GUP_OK;
}

#endif

#ifndef ST_BITS

/*
  in de generieke versie moet
    if (bit_count)
    {
  zitten, omdat anders er een shift over 32 gedaan kan worden.
  als een long 32 bits is is dit door ansi c niet gedefinieerd
  bekende processor dit een long niet over 32 bits kan schuiven is de
  80x86 serie van intel....
*/

/* unsigned long val, int bit_count */
#define ST_BITS(val_0, bit_count_0)                        \
{                                                          \
  unsigned long val=val_0;                                 \
  int16 bit_count=(int16)bit_count_0;                      \
  if (bit_count!=0)                                        \
  {                                                        \
    com->bits_in_bitbuf += bit_count;                      \
    if (com->bits_in_bitbuf >= 32)                         \
    {                                                      \
      com->bits_in_bitbuf -= 32;                           \
      com->bitbuf += val >> com->bits_in_bitbuf;           \
      *com->rbuf_current++ = (uint8) (com->bitbuf >> 24);  \
      *com->rbuf_current++ = (uint8) (com->bitbuf >> 16);  \
      *com->rbuf_current++ = (uint8) (com->bitbuf >> 8);   \
      *com->rbuf_current++ = (uint8) (com->bitbuf);        \
      if(com->bits_in_bitbuf!=0)                           \
      {                                                    \
        com->bitbuf = val << (32 - com->bits_in_bitbuf);   \
      }                                                    \
      else                                                 \
      {                                                    \
        com->bitbuf=0;                                     \
      }                                                    \
    }                                                      \
    else                                                   \
    {                                                      \
      com->bitbuf += val << (32 - com->bits_in_bitbuf);    \
    }                                                      \
  }                                                        \
}

#endif


#ifndef NOT_USE_STD_compress_chars

gup_result compress_chars(packstruct *com)
{
  uint16 entriesextra = 0;
  uint16 charfreq1[NC + NC + X_CHARS];
  uint16 *charfreq = charfreq1 + X_CHARS;/* frequentie tabel voor karakters, charfreq[-1 .. -X_CHARS] bestaan */
  uint16 freq1[MAX_NPT + MAX_NPT + X_CHARS];
  uint16 *freq = freq1 + X_CHARS;        /* frequentie tabel pointers          */
  uint16 pointer_count;
  uint16 entries;
  uint16 charct;

  { /*- aantal entries berekenen */
    uint16 lentries;

    lentries = (uint16) (com->charp - com->chars);
    #if !defined(NDEBUG) || 0
    { /*- for debugging use */
      long i = lentries;
      c_codetype *cp = com->chars;

      while (--i!=0)
      {
        c_codetype tmp = *cp++;
        if(tmp<0)
        {
          fprintf(redir, "te klein!");
        }
        if(tmp>(NLIT-MIN_MATCH+com->max_match))
        {
          fprintf(redir, "te groot!");
        }
        ARJ_Assert(tmp>=0);
        ARJ_Assert(tmp<=(NLIT-MIN_MATCH+com->max_match));
      }
    }
    #endif
    if (lentries > com->hufbufsize)
    {
      lentries = com->hufbufsize;
    }
    if (com->jm && (lentries > HUFFSTART))
    { /*- aantal entries dynamisch berekenen */
      int count = (int)(HUFFSTART / HUFFDELTA);
      c_codetype *cp = com->chars;
      pointer_type *pp = com->pointers;
      uint16 delta;
      uint16 dentries = HUFFSTART;

      delta = dentries;
      /* character frequentie op nul zetten */
      memset(charfreq, 0, NC * sizeof (*charfreq));
      /* zet pointer_count op nul */
      memset(freq, 0, MAX_NPT * sizeof (*freq));
      while (dentries < lentries)
      {
        { /*- character frequentie tellen */
          uint16 i = delta;

          do
          {
            c_codetype tmp = *cp++;

            ARJ_Assert(cp - 1 >= com->chars);
            ARJ_Assert(cp - 1 < com->charp);
            ARJ_Assert(tmp >= 0);
            ARJ_Assert(tmp < NC + NC);
            ARJ_Assert(tmp < NC);
            charfreq[tmp]++;
            if (tmp > (NLIT - 1))
            {
              freq[LOG(*pp++)]++;
            }
          }
          while (--i!=0);
        }
        /* hier kunnen we charct niet gebruiken omdat de bovenkant van charfreq schoon moet blijven */
        make_hufftable(com->charlen, com->char2huffman, charfreq, dentries, NC, MAX_HUFFLEN, com);
        make_hufftable(com->ptrlen, com->ptr2huffman, freq, (uint16)(pp - com->pointers), com->n_ptr, MAX_HUFFLEN, com);
        /*
         * Karakter frequenties zijn bekend, karakter huffman tabel is
         * berekend. Pointer frequenties zijn bekend, pointer huffman
         * tabel is berekend.
        */
        {
          unsigned long h_bits, m_bits, m_size;
          unsigned long newsize = 0;
          { /*- bepalen tot welke wat het hoogste gebruikte character is */
            uint16 *p = charfreq + NC;
            while (!*--p)
            {
              ;
            }
            ARJ_Assert(p >= charfreq);
            charct = (uint16)(p + 1 - charfreq);
          }
          count_bits(&h_bits, &m_bits, &m_size, charct, dentries, com->charlen,
                     com->ptrlen, charfreq, freq, com);
          { /*- nu alleen newsize nog berekenen */
            c_codetype *p = cp;
            pointer_type *q = pp;
            uint8 *r = com->charlen+charct;
            uint8 maxlen = 0;
            uint16 i = charct;

            do
            {
              if (*--r > maxlen)
              {
                maxlen = *r;
              }
            }
            while (--i!=0);
            maxlen++;
            r+=NC;
            i=NC;
            do
            {
              if (*--r == 0)
              {
                *r = maxlen;
              }
            }
            while (--i!=0);
            maxlen = 0;
            i = com->n_ptr;
            r = com->ptrlen+com->n_ptr;
            do
            {
              if (*--r > maxlen)
              {
                maxlen = *r;
              }
            }
            while (--i!=0);
            maxlen++;
            i = com->n_ptr;
            r += com->n_ptr;
            do
            {
              if (*--r == 0)
              {
                *r = maxlen;
              }
            }
            while (--i!=0);
            i = HUFFDELTA;
            if (dentries + i > lentries)
            {
              i = lentries - dentries;
              ARJ_Assert(((long)lentries - (long)dentries) >= 0);
            }
            do
            {
              c_codetype tmp = *p++;
#if 0
              ARJ_Assert((p - 1) >= com->chars);
              ARJ_Assert((p - 1) < com->charp);
              ARJ_Assert(tmp >= 0);
              ARJ_Assert(tmp < NC + NC);
              ARJ_Assert(tmp < NC);
#endif
              newsize += com->charlen[tmp];
              if (tmp > (NLIT - 1))
              {
                int lenq;

                {
                  lenq = LOG(*q++);
                  newsize += com->ptrlen[lenq];
                  if (lenq > 0)
                  {
                    newsize += lenq - 1;
                  }
                }
              }
            }
            while (--i!=0);
          }
          if ((unsigned long)labs((long)(m_bits / count - newsize)) < h_bits)
          {
            count++;
            delta = HUFFDELTA;
            dentries += HUFFDELTA;
            if (dentries > lentries)
            {
              dentries = lentries;
              #if 0
                fprintf(redir, "%X", count);
              #endif
              break;
            }
          }
          else
          {
            if ((lentries - dentries) < HUFFDELTA)
            {
              dentries = lentries;
            }
            #if 0
              fprintf(redir, "%X", count);
            #endif
            break;
          }
        }
      }
      lentries = dentries;
    }
    entries = (uint16) lentries;
  }
  /* character frequentie op nul zetten */
  memset(charfreq, 0, NC * sizeof (*charfreq));
  /* zet pointer count op nul */
  memset(freq, 0, MAX_NPT * sizeof (*freq));
  { /*- character frequentie tellen */
    uint16 i = entries;
    c_codetype *p = com->chars;
    pointer_type *q = com->pointers;

    do
    {
      c_codetype tmp = *p++;

      charfreq[tmp]++;
      if (tmp > (NLIT - 1))
      {
        freq[LOG(*q++)]++;
      }
    }
    while (--i!=0);
    pointer_count = (uint16) (q - com->pointers);
  }
  make_hufftable(com->charlen, com->char2huffman, charfreq, entries, NC, MAX_HUFFLEN, com);
  make_hufftable(com->ptrlen, com->ptr2huffman, freq, pointer_count, com->n_ptr, MAX_HUFFLEN, com);
  /*
   * Karakter frequenties zijn bekend, karakter huffman tabel is berekend.
   * Pointer frequenties zijn bekend, pointer huffman tabel is berekend.
  */
  if (com->matchstring != NULL)
  {
    uint16 old_entries;
    ARJ_Assert(com->backmatch!=NULL);
    com->backmatch[pointer_count]=0; /* om te voorkomen dat hij een backmatch over de huffmangrens vindt */
    
    do
    {
      /*
      //  Nu filteren van matches van lengte 3 en 4
      //  Probeer zo veel mogelijk characters bij de volgende match aan te
      //  hangen (backmatch lengte) en converteer de overgebleven characters 
      //  naar literals. Dit werkt ook voor matches langer dan 4, zolang de
      //  overgebleven backmatchlengte maar kleiner of gelijk aan 4 is.
      //  Als dit allemaal niet lukt wordt er gekeken of het zinnig is om
      //  matches van drie en vier in z'n geheel naar literals om te zetten.
      */
      /* c_len is bekend */
      old_entries = entries;
      { /*- bepaal de maximum lengte */
        uint8 maxlen = 0;
        uint8 *p = com->charlen; /* van achter naar voor werken, pointers hebben meestal de kleinste frequentie */
        uint16 i = NC;
        while (i--!=0)
        {
          if (p[i] > maxlen)
          {
            maxlen = p[i];
          }
        }
        maxlen++;
        i = NC;
        while (i--!=0)
        {
          if (p[i] == 0)
          {
            p[i] = maxlen;
          }
        }
      }
      {
        pointer_type *q = com->pointers;
        uint8 *mstp = com->matchstring;
        uint8* bp=com->backmatch+1;
        c_codetype *p = com->chars;
        {
          /* char-length for element 'MIN_MATCH': */
          int8 len3 = com->charlen[MIN_MATCH + (NLIT - MIN_MATCH)]; 
          /* char-length for element 'MIN_MATCH+1': */
          int8 len4 = com->charlen[(MIN_MATCH + 1) + (NLIT - MIN_MATCH)];
          uint16 i = entries-entriesextra;
          
          do
          {
            c_codetype kar = *p++;
            
            if (kar > (NLIT-1))
            {
              int bml;
              
              if((bml=*bp++)>0)
              {
                /*
                //  Kijken of we van twee opeenvolgende matches
                //  1 tot 4 literals en een grotere match kunnen maken
                */
                int ckar = kar-bml-NLIT+MIN_MATCH; /* aantal mogelijke literals */
                
                ARJ_Assert(*p>(NLIT-1));
                ARJ_Assert((bml+*p-NLIT+MIN_MATCH)<=com->max_match);
                ARJ_Assert(ckar>0);
                
                if((ckar < 5) && (entries < (MAX_ENTRIES-ckar))) /* entries moet kleiner MAX_ENTRIES blijven */
                { /* conversie mogelijk */
                  int8 lengte1;
                  int8 lenq = LOG(*q);
                  lengte1 = com->ptrlen[lenq];
                  if (lenq > 0)
                  {
                    lengte1 += lenq - 1;
                  }
                  lengte1 += com->charlen[*p];
                  lengte1 += com->charlen[kar];
                  lengte1 -= com->charlen[(*p)+bml];
                  lengte1 -= com->charlen[*mstp];
                  
                  if(ckar == 1)
                  {
                    if(lengte1>=0)
                    { /* conversie, 1 extra char */
                      if(bml==2)
                      { /* lengte 3 geconverteerd, 2 extra len */
                        mstp[7]=mstp[5];
                        mstp[6]=mstp[4];
                        mstp[5]=mstp[2];
                        mstp[4]=mstp[1];
                      }
                      else if(bml==3)
                      { /* lengte 4 geconverteerd, 3 extra len */
                        mstp[7]=mstp[4];
                        mstp[6]=mstp[3];
                        mstp[5]=mstp[2];
                        mstp[4]=mstp[1];
                      }
                      /*
                      //  bij de overige gevallen is zelfs bij maximale backmatch
                      //  een lengte groter dan 4 gegarandeerd ->
                      //  geen dubbele conversie mogelijk
                      */
                      charfreq[*mstp]++;
                      charfreq[kar]--;
                      charfreq[*p]--;
                      freq[lenq]--;
                      pointer_count--;
                      p[-1] = -1;
                      *p+=bml;
                      charfreq[*p]++;
                      bp[-1]=0;
                      bp[-2]=0;
                    }
                  }
                  else
                  {
                    lengte1 -= com->charlen[mstp[1]];
                    if(ckar == 2)
                    {
                      if(lengte1>=0)
                      { /* conversie, 2 extra chars */
                        if(bml==1)
                        { /* lengte 3 geconverteerd, 1 extra len */
                          mstp[7]=mstp[6];
                          mstp[6]=mstp[5];
                          mstp[5]=mstp[4];
                          mstp[4]=mstp[2];
                        }
                        else if(bml==2)
                        { /* lengte 4 geconverteerd, 2 extra len */
                          mstp[7]=mstp[5];
                          mstp[6]=mstp[4];
                          mstp[5]=mstp[3];
                          mstp[4]=mstp[2];
                        }
                        else if(bml==3)
                        { /* lengte 5 geconverteerd, zorg dat er geen dubbele conversie kan gebeuren */
                          if((p[1]>(NLIT-1)) && ((*p-*bp-NLIT+MIN_MATCH)==1))
                          { /* er is een gevaarlijke volgende match aanwezig */
                            *bp-=1; /* fix, nu kan er geen lengte vier conversie komen */
                          }
                        }
                        charfreq[mstp[1]]++;
                        charfreq[*mstp]++;
                        charfreq[kar]--;
                        charfreq[*p]--;
                        freq[lenq]--;
                        pointer_count--;
                        p[-1] = -2;
                        *p+=bml;
                        charfreq[*p]++;
                        entries+=1;
                        bp[-1]=0;
                        bp[-2]=0;
                      }
                    }
                    else
                    {
                      lengte1 -= com->charlen[mstp[2]];
                      if(ckar == 3)
                      {
                        if(lengte1>=0)
                        { /* conversie, 3 extra chars */
                          if(bml==1)
                          { /* lengte 4 geconverteerd, 1 extra len */
                            mstp[7]=mstp[6];
                            mstp[6]=mstp[5];
                            mstp[5]=mstp[4];
                            mstp[4]=mstp[3];
                          }
                          else if(bml<=3)
                          { /* zorg dat er geen dubbele conversie kan gebeuren */
                            if(p[1]>(NLIT-1))
                            {
                              int tmp =*p-*bp+bml-NLIT+MIN_MATCH; /* minimum size waartoe volgende match tot gereduceert kan worden */
                              if(tmp<=4)
                              { /* er is een gevaarlijke volgende match aanwezig */
                                *bp-=(uint8)(5-tmp); /* fix, nu kan er geen lengte vier conversie komen */
                              }
                            }
                          }
                          charfreq[mstp[2]]++;
                          charfreq[mstp[1]]++;
                          charfreq[*mstp]++;
                          charfreq[kar]--;
                          charfreq[*p]--;
                          freq[lenq]--;
                          pointer_count--;
                          p[-1] = -3;
                          *p+=bml;
                          charfreq[*p]++;
                          entries+=2;
                          bp[-1]=0;
                          bp[-2]=0;
                        }
                      }
                      else
                      { /* ckar == 4 */
                        lengte1 -= com->charlen[mstp[3]];
                        if(lengte1>=0)
                        { /* conversie, 4 extra chars */
                          if (*p==(MIN_MATCH + (NLIT - MIN_MATCH)))
                          { /* lengte 3 kan fout gaan met zeef34 */
                            bp[-1]--; /* fix bp */
                          }
                          else
                          {
                            if(bml<=3)
                            { /* zorg dat er geen dubbele conversie kan gebeuren */
                              if(p[1]>(NLIT-1))
                              {
                                int tmp =*p-*bp+bml-NLIT+MIN_MATCH; /* minimum size waartoe volgende match tot gereduceert kan worden */
                                if(tmp<=4)
                                { /* er is een gevaarlijke volgende match aanwezig */
                                  *bp-=(uint8)(5-tmp); /* fix, nu kan er geen lengte vier conversie komen */
                                }
                              }
                            }
                            charfreq[mstp[3]]++;
                            charfreq[mstp[2]]++;
                            charfreq[mstp[1]]++;
                            charfreq[*mstp]++;
                            charfreq[kar]--;
                            charfreq[*p]--;
                            freq[lenq]--;
                            pointer_count--;
                            p[-1] = -4;
                            *p+=bml;
                            charfreq[*p]++;
                            entries+=3;
                            bp[-1]=0;
                            bp[-2]=0;
                          }
                        }
                      }
                    }
                  }
                }
              }
              else if (kar < ((MIN_MATCH + 2) + (NLIT - MIN_MATCH)))
              { /* check for (MIN_MATCH) or (MIN_MATCH+1) matches only! */
                int8 lengte1;
                int8 lenq;
                
                lenq = LOG(*q);
                lengte1 = com->ptrlen[lenq];
                
                if (lenq > 0)
                {
                  lengte1 += lenq - 1;
                }
                lengte1 -= com->charlen[*mstp];
                lengte1 -= com->charlen[mstp[1]];
                lengte1 -= com->charlen[mstp[2]];
                if(kar == (MIN_MATCH + (NLIT - MIN_MATCH)))
                { /*- match van drie */
                  if(((lengte1+len3) >= 0)
                  && (entries <= (MAX_ENTRIES - MIN_MATCH)))  /* entries moet kleiner MAX_ENTRIES blijven */
                  {
                    charfreq[*mstp]++;
                    charfreq[mstp[1]]++;
                    charfreq[mstp[2]]++;
                    charfreq[kar]--;
                    p[-1] = -3;
                    entries += 2;
                    freq[lenq]--;
                    bp[-2]=0; /* hier geen backmatch meer mogelijk */
                    pointer_count--;
                  }
                }
                else if(((lengte1+len4) >= com->charlen[mstp[3]])
                     && (entries <= (MAX_ENTRIES - (MIN_MATCH + 1)))) /* entries moet kleiner MAX_ENTRIES blijven */
                { /*- match van 4 */
                  charfreq[*mstp]++;
                  charfreq[mstp[1]]++;
                  charfreq[mstp[2]]++;
                  charfreq[mstp[3]]++;
                  charfreq[kar]--;
                  p[-1] = -4;
                  entries += 3;
                  freq[lenq]--;
                  bp[-2]=0; /* hier geen backmatch meer mogelijk */
                  pointer_count--;
                }
              }
              mstp += 4;
              q++;
            }
            else
            {
              if (kar < 0)
              {
                mstp += 4;
                q++;
                bp++;
              }
            }
          }
          while (--i!=0);
        }
      }
      #if 0
      if(entries > MAX_ENTRIES-4)
      { /* huffblocksize research */
        printf("!");
      }
      #endif
      #if !defined(NDEBUG) || 0
      { /*- for debugging use */
        long lentries = (uint16) (com->charp - com->chars);
        {
          long i = lentries;
          c_codetype *cp = com->chars;
          while (--i!=0)
          {
            c_codetype tmp = *cp++;
            ARJ_Assert(tmp>=-4);
            ARJ_Assert(tmp<=(c_codetype)(NLIT-MIN_MATCH+com->max_match));
            if(tmp<-4)
            {
              fprintf(redir, "te klein!");
            }
            if(tmp>(int)(NLIT-MIN_MATCH+com->max_match))
            {
              fprintf(redir, "te groot!");
            }
          }
        }
      }
      #endif
      entriesextra += entries - old_entries;
      /* recalc charlen */
      make_hufftable(com->charlen, com->char2huffman, charfreq, entries, NC, MAX_HUFFLEN, com);
      /* recalc ptrlen */
      make_hufftable(com->ptrlen, com->ptr2huffman, freq, pointer_count, com->n_ptr, MAX_HUFFLEN, com);
    }
    while (com->jm && (entries != old_entries));
    {
      /* 
      //  Code die backmatch stringlengtes optimaliseert.
      //  Bij een backmatch zijn er twee opeenvolgende matches, waarbij 
      //  de laatste match een backmatch waarde groter dan nul heeft.
      //  Deze laatste macth kunnen we groter laten worden tenkoste van de 
      //  grootte van de match die ervoor ligt...
      */
      int redo; /* geeft aan dat er een conversie heeft plaatsgevonden -> nog een iteratie */
      do
      {
        redo=0;
        { /*- bepaal de maximum lengte */
          uint8 maxlen = 0;
          uint8 *p = com->charlen+NC;
          uint16 i = NC;
          do
          {
            if (*--p > maxlen)
            {
              maxlen = *p;
            }
          }
          while (--i!=0);
          maxlen++;
          i = NC-NLIT;
          p +=NC;
          do
          {
            if (*--p == 0)
            {
              *p = maxlen;
            }
          }
          while (--i!=0);
          /* voorkomen dat we een match kleiner dan drie maken */
          *--p=32;
          *--p=32;
          *--p=32;
        }
        {
          uint8* bp=com->backmatch;
          c_codetype *p = com->chars;
          uint16 i = entries-entriesextra;
          do
          {
            c_codetype kar = *p++;
            if (kar > (NLIT-1))
            {
              uint8 len=*bp++;
              if(len>0)
              {
                c_codetype kar_1;
                if((kar_1=p[-2])<0)
                {
                  bp[-1]=0;
                }
                else
                {
                  uint8 offset=1;
                  uint8 optlen=com->charlen[kar_1]+com->charlen[kar];
                  do
                  {
                    if((com->charlen[kar_1-offset]+com->charlen[kar+offset])<optlen)
                    {
                      redo=1;
                      charfreq[kar]--;
                      charfreq[kar_1]--;
                      bp[-1]-=offset;
                      p[-2]-=offset;
                      p[-1]+=offset;
                      kar+=offset;
                      kar_1-=offset;
                      optlen=com->charlen[kar_1]+com->charlen[kar];
                      charfreq[kar]++;
                      charfreq[kar_1]++;
                      offset=1;
                    }
                    else
                    {
                      offset++;
                    }
                  }
                  while(--len!=0);
                }
              }
            }
            else
            {
              if(kar<0)
              {
                bp++;
              }
            }
          }
          while (--i!=0);
        }
        #if !defined(NDEBUG) || 0
          { /*- for debugging use */
            long lentries = (uint16) (com->charp - com->chars);
            {
              long i = lentries;
              c_codetype *cp = com->chars;
              while (--i!=0)
              {
                c_codetype tmp = *cp++;
                ARJ_Assert(tmp>=-4);
                ARJ_Assert(tmp<=(c_codetype)(NLIT-MIN_MATCH+com->max_match));
                if(tmp<-4)
                {
                  fprintf(redir, "te klein!");
                }
                if(tmp>(int)(NLIT-MIN_MATCH+com->max_match))
                {
                  fprintf(redir, "te groot!");
                }
              }
            }
          }
        #endif
        /* recalc charlen, ptr len hoeft niet */
        make_hufftable(com->charlen, com->char2huffman, charfreq, entries, NC, MAX_HUFFLEN, com);
      }
      while(com->jm && (redo));
    }
  }
  /*
   * Karakter frequenties zijn bekend, karakter huffman tabel is berekend.
   * Pointer frequenties zijn bekend, pointer huffman tabel is berekend.
  */
  {
    unsigned long m_size, bits_comming;
    unsigned long h_bits, m_bits;
    {
      /* bepalen tot welke wat het hoogste gebruikte character is */
      charct=NC;
      while (!charfreq[charct - 1])
      {
        charct--;
      }
    }
    bits_comming = count_bits(&h_bits, &m_bits, &m_size, charct, entries, com->charlen,
                              com->ptrlen, charfreq, freq, com);
    /*
     * Nu hebben we de data geoptimaliseerd en kan het de file in, maar past
     * het wel in de file? dat beslist de multiple volume code!
    */
    if(com->mv_mode) 
    {
      unsigned long bits=bits_comming+com->mv_bits_left+7;
      bits >>= 3;                      /* bytes=bits/8 */
      if (bits > com->mv_bytes_left)
      { /*- gedonder!, MV break! */
        uint16 delta_size;
        uint16 the_size = 1;
        
        com->mv_next = 1;
        delta_size=0x8000;
        entries -= entriesextra;
        while(delta_size>=entries)
        {
          delta_size>>=1;
        }
        entries -= delta_size;
        do
        {
          delta_size >>= 1;
          /* frequentie tabel op nul */
          memset(charfreq, 0, NC * sizeof (*charfreq));
          /* pointer frequentie op nul */
          memset(freq, 0, MAX_NPT * sizeof (*freq));
          { /*- character frequentie tellen */
            uint16 i = entries;
            c_codetype *p = com->chars;
            pointer_type *q = com->pointers;
            uint8 *mstp = com->matchstring;

            entriesextra = 0;
            pointer_count = 0;
            do
            {
              c_codetype tmp = *p++;

              if (tmp >= 0)
              {
                charfreq[tmp]++;
                if (tmp > (NLIT-1))
                {
                  mstp += 4;
                  freq[LOG(*q++)]++;
                }
              }
              else
              {
                q++;
                pointer_count--;
                charfreq[*mstp++]++;
                if (tmp < -1)
                {
                  charfreq[*mstp++]++;
                  if(tmp < -2)
                  {
                    charfreq[*mstp++]++;
                    if(tmp < -3)
                    {
                      charfreq[*mstp++]++;
                    }
                  }
                }
                entriesextra-=tmp+1;
                mstp+=4+tmp;
              }
            }
            while (--i!=0);
            pointer_count += (uint16)(q - com->pointers);
          }
          { /*- bepalen tot welke wat het hoogste gebruikte character is */
            uint16 *p = charfreq + NC;

            while (!*--p)
            {
              ;
            }
            charct = (uint16)(p + 1 - charfreq);
          }
          {
            unsigned long h_bits, m_bits;

            make_hufftable(com->ptrlen, com->ptr2huffman, freq, 
                           pointer_count, com->n_ptr, MAX_HUFFLEN, com);
            make_hufftable(com->charlen, com->char2huffman, charfreq, 
                           (uint16)(entries + entriesextra), NC, MAX_HUFFLEN, com);
            bits = count_bits(&h_bits, &m_bits, &m_size, charct, (uint16)(entries + entriesextra),
                              com->charlen, com->ptrlen, charfreq, freq, com) + com->mv_bits_left+7;
            bits >>= 3;                  /* bytes=bits/8 */
          }
          if (bits > com->mv_bytes_left)
          {
            if(delta_size>=entries)
            {
              entries=1;
            }
            else
            {
              entries -= delta_size;
            }
          }
          else
          {
            the_size = entries;
            entries += delta_size;
          }
        }
        while (delta_size);
        if ((the_size == 1) && (bits > com->mv_bytes_left))
        {
          return GUP_OK;
        }
        entries = the_size;
        {
          uint8 *mstp = com->matchstring;

          entriesextra = 0;
          delta_size >>= 1;
          /* frequentie tabel op nul */
          memset(charfreq, 0, NC * sizeof (*charfreq));
          /* pointer frequentie op nul */
          memset(freq, 0, MAX_NPT * sizeof (*freq));
          { /*- character frequentie tellen */
            uint16 i = entries;
            c_codetype *p = com->chars;
            pointer_type *q = com->pointers;

            pointer_count = 0;
            do
            {
              c_codetype tmp = *p++;

              if (tmp >= 0)
              {
                charfreq[tmp]++;
                if (tmp > (NLIT-1))
                {
                  mstp += 4;
                  freq[LOG(*q++)]++;
                }
              }
              else
              {
                q++;
                pointer_count--;
                charfreq[*mstp++]++;
                if (tmp < -1)
                {
                  charfreq[*mstp++]++;
                  if(tmp < -2)
                  {
                    charfreq[*mstp++]++;
                    if(tmp < -3)
                    {
                      charfreq[*mstp++]++;
                    }
                  }
                }
                entriesextra-=tmp+1;
                mstp+=4+tmp;
              }
            }
            while (--i!=0);
            pointer_count += (uint16)(q - com->pointers);
          }
          { /*- bepalen tot wat het hoogste gebruikte character is */
            uint16 *p = charfreq + NC;

            while (!*--p)
            {
              ;
            }
            charct = (uint16)(p + 1 - charfreq);
          }
          {
            unsigned long h_bits, m_bits;

            make_hufftable(com->ptrlen, com->ptr2huffman, freq, pointer_count, com->n_ptr, MAX_HUFFLEN, com);
            entries += entriesextra;
            make_hufftable(com->charlen, com->char2huffman, charfreq, entries, NC, MAX_HUFFLEN, com);
            bits_comming = count_bits(&h_bits, &m_bits, &m_size, charct, entries,
                              com->charlen, com->ptrlen, charfreq, freq, com);
          }
        }
      }
      bits=bits_comming+com->mv_bits_left;
      com->mv_bits_left = (int16)(bits&7);
      com->mv_bytes_left -= bits>>3;
    }
    /*
      we weten nu hoveel bits er aan komen, passen deze nog in de buffer,
      of moeten we het ding flushen?
    */
    {
      gup_result res;
      if((res=announce((bits_comming+7)>>3, com))!=GUP_OK)
      {
        return res;
      }
      bits_comming+=com->bits_rest;
      com->bits_rest=(int16)(bits_comming&7);
      com->packed_size += bits_comming>>3;
    }
    #ifdef PP_AFTER
    com->print_progres(m_size, com->pp_propagator);
    #endif
    com->bytes_packed += m_size; /* alweer een paar bytes gedaan! */
  }
  /*
   * Karakter frequenties zijn bekend, karakter huffman tabel is berekend.
   * Pointer frequenties zijn bekend, pointer huffman tabel is berekend.
   * charct is bekend
  */
  /*-
   * we hebben nu de huffman codes van de karakterset berekend, nu moeten
   * we de lengtes gaan coderen. deze staan in charlen c_len coderings
   * blok: 
   * lengte van de pointers die c_len coderen, er zijn 19 pointers:
   * 0          = c_len = 0 
   * 1 + 4 bits = de volgende 3-18 karakters hebben lengte 0
   * 2 + 9 bits = de volgende 20-531 karakters hebben lengte 0
   * Waar de hell is 19 gebleven? Foutje????? 
   * 3          = c_len = 1 
   * :
   * n          = c_len = n-2 
   * : 
   * 18         = c_len = 16
  */
  if(com->special_header!=NORMAL_HEADER)
  { /* minimale header, alles literal */
    #if 0
    fprintf(redir, "!");
    #endif
    {
      uint16 xentries=entries;
      if(charct>NLIT)
      { /* tel len twee en dries op */
        xentries+=2*charfreq[NLIT];
        if(charct>(NLIT+1))
        {
          xentries+=3*charfreq[NLIT+1];
        }
      }
      ST_BITS(xentries, 16);           /* aantal huffman karakters */
    }
    { /*- special case 1, er is maar een character lengte */
      ST_BITS(0, 5);
      ST_BITS(10, 5); /* charlen is 8! */
    }
    {
      ST_BITS(256, 9); /* alle huffman lengtes zijn 0 */
    }
    /*
     * charlen is overgestuurd, nu weer een ptrlen
    */
    { /*- special case 3, er is maar een pointerlengte */
      ST_BITS(0, com->m_ptr_bit);
      ST_BITS(0, com->m_ptr_bit);
    }
    {
      int i;
      for(i=0;i<NLIT;i++)
      {
        com->char2huffman[i]=(uint16)i;
        com->charlen[i]=8;
      }
    }
    {
      uint16 i=entries-entriesextra;
      c_codetype *r = com->chars;
      do
      {
        c_codetype xkar=*r;
        if(xkar>=NLIT)
        {
          if(xkar>NLIT)
          { /* len is 4 */
            entries+=3;
            entriesextra+=3;
            *r++=-4;
          }
          else
          { /* len is 3 */
            entries+=2;
            entriesextra+=2;
            *r++=-3;
          }
        }
        else
        {
          r++;
        }
      }
      while(--i!=0);
    }
  }
  else
  { /* vanaf hier hebben wij charfreq niet meer nodig! */
    uint16 ptr_count = 0;
    int i;
    /* frequentie tabel op nul zetten voor gebruik pointers */
    memset(charfreq, 0, NCPT * sizeof (*charfreq));
    /* frequentie character lengtes tellen */
    for (i = 0; i < charct; i++)
    {
      if (com->charlen[i])
      {
        ptr_count++;
        charfreq[com->charlen[i] + 2]++;
      }
      else
      { /*- charlen nul krijgt een speciale behandeling */
        int nulct = 1;

        while (!com->charlen[i + nulct])
        {
          nulct++;
        }
        if (nulct < 3)
        {
          charfreq[0] += (uint16)nulct;
          ptr_count += (uint16)nulct;
        }
        else
        {
          if (nulct < 20)
          {
            charfreq[1]++;
            if (nulct == 19)
            {
              charfreq[0]++;
              ptr_count++;
            }
          }
          else
          {
            charfreq[2]++;
          }
          ptr_count++;
        }
        i += nulct - 1;
      }
    }
    make_hufftable(com->ptrlen1, com->ptr2huffman1, charfreq, ptr_count, NCPT, MAX_HUFFLEN, com);
    /*
     * Nu zijn alle ptrs gedefinieerd, stuur ze de ARJ file in
    */
    ST_BITS(entries, 16);           /* aantal huffman karakters */
    {
      /*
       * belangrijk item, wat zijn de gevallen dat er slechts 1
       * pointerlengte overgedragen hoeft te worden? er is maar 1 pointer
       * lengte er is maar 1 karakter (dat kan wel meerdere ptrlens
       * veroorzaken)
      */
      int vp = 0;
      int np = 1;
      uint8 *p = com->charlen;
      int len = *p;

      i = charct;
      do
      {
        int tmp = *p++;

        if (tmp)
        {
          vp++;
          if (tmp != len)
          {
            np = 0;
          }
        }
        else
        {
          np = 0;
        }
      }
      while (--i!=0);
      if ((vp < 2) || np)
      { /*- special case 1, er is maar een character lengte */
        ST_BITS(0, 5);
        ST_BITS(*com->charlen + 2, 5);
      }
      else
      {
        long ptrct = NCPT;
        int extra_add = 0;

        while (!com->ptrlen1[ptrct - 1])
        {
          ptrct--;
        }
        if (com->ptrlen1[3] == 0)
        {
          extra_add = 1;
          if (com->ptrlen1[4] == 0)
          {
            extra_add = 2;
            if (com->ptrlen1[5] == 0)
            {
              extra_add = 3;
            }
          }
        }
        ST_BITS(ptrct, 5);          /* aantal pointers dat er aan komt */
        for (i = 0; i < ptrct; i++)
        {
          if (com->ptrlen1[i] < 7)
          {
            ST_BITS(com->ptrlen1[i], 3);
          }
          else
          {
            int rest = com->ptrlen1[i] - 7;

            ST_BITS(7, 3);
            while (rest!=0)
            {
              rest--;
              ST_BITS(1, 1);
            }
            ST_BITS(0, 1);
          }
          if (i == 2)
          {
            ST_BITS(extra_add, 2);
            i += extra_add;
          }
        }
      }
    }
    /*
     * charlen overgedragen, breng characters
    */
    {
      /*
       * De enige special case voor de characters is dat er maar een
       * character is.
      */
      uint16 vp = 0;
      uint8 *p = com->charlen;

      i = charct;
      do
      {
        if (*p++)
        {
          vp++;
        }
      }
      while (--i!=0);
      if (vp < 2)
      { /*- special case 2, er is maar een karakter lengte */
        c_codetype pos = *com->chars;

        if (pos < 0)
        {
          pos = *com->matchstring;
        }
        ST_BITS(0, 9);
        ST_BITS(pos, 9);
        com->charlen[pos] = 0;
        com->char2huffman[pos] = 0;
      }
      else
      {
        ST_BITS(charct, 9);
        for (i = 0; i < charct; i++)
        {
          if (com->charlen[i])
          {
            ST_BITS(com->ptr2huffman1[com->charlen[i] + 2], com->ptrlen1[com->charlen[i] + 2]);
          }
          else
          {
            int nulct = 1;

            while (!com->charlen[i + nulct])
            {
              nulct++;
            }
            i += nulct - 1;
            if (nulct < 3)
            {
              while (nulct!=0)
              {
                ST_BITS(com->ptr2huffman1[0], com->ptrlen1[0]);
                nulct--;
              }
            }
            else
            {
              if (nulct < 20)
              {
                if (nulct == 19)
                {
                  ST_BITS(com->ptr2huffman1[0], com->ptrlen1[0]);
                  nulct--;
                }
                ST_BITS(com->ptr2huffman1[1], com->ptrlen1[1]);
                ST_BITS(nulct - 3, 4);
              }
              else
              {
                ST_BITS(com->ptr2huffman1[2], com->ptrlen1[2]);
                ST_BITS(nulct - 20, 9);
              }
            }
          }
        }
      }
    }
    /*
     * charlen is overgestuurd, nu weer een ptrlen
    */
    {
      /*
       * wat is de specialcase voor de pointers? 1 er is maar een
       * pointerlengte
      */
      uint16 vp = 0;
      uint16 *p = freq;

      i = com->n_ptr;
      do
      {
        if (*p++)
        {
          vp++;
        }
      }
      while (--i!=0);
      if (vp < 2)
      { /*- special case 3, er is maar een pointerlengte */
        int j;

        if (vp == 1)
        {
          uint16 *p = freq;

          while (*p++ == 0)
          {
            ;
          }
          j = (int)(p - 1 - freq);
        }
        else
        {
          j = 0;
        }
        ST_BITS(0, com->m_ptr_bit);
        ST_BITS(j, com->m_ptr_bit);
        com->ptrlen[j] = 0;
        com->ptr2huffman[j] = 0;
      }
      else
      {
        int ptrct = com->n_ptr;

        while ((ptrct) && (!com->ptrlen[ptrct - 1]))
        {
          ptrct--;
        }
        ST_BITS(ptrct, com->m_ptr_bit);
        for (i = 0; i < ptrct; i++)
        {
          if (com->ptrlen[i] < 7)
          {
            ST_BITS(com->ptrlen[i], 3);
          }
          else
          {
            int rest = com->ptrlen[i] - 7;

            ST_BITS(7, 3);
            while (rest!=0)
            {
              rest--;
              ST_BITS(1, 1);
            }
            ST_BITS(0, 1);
          }
        }
      }
    }
  }
  /*
   * alle codes overgedragen, stuur nu de gecodeerde message
  */
  entries -= entriesextra;
  {
    c_codetype *r = com->chars;
    pointer_type *s = com->pointers;
    uint8 *s3 = com->matchstring;
    uint16 i = entries;

    do
    {
      c_codetype kar = *r++;

      if (kar < 0)
      {
        c_codetype xkar;
        xkar = *s3++;
        ST_BITS(com->char2huffman[xkar], com->charlen[xkar]);
        if(kar< -1)
        {
          xkar = *s3++;
          ST_BITS(com->char2huffman[xkar], com->charlen[xkar]);
          if(kar< -2)
          {
            xkar = *s3++;
            ST_BITS(com->char2huffman[xkar], com->charlen[xkar]);
            if(kar< -3)
            {
              xkar = *s3++;
              ST_BITS(com->char2huffman[xkar], com->charlen[xkar]);
            }
          }
        }
        s3+=4+kar;
        s++;
      }
      else
      {
        ST_BITS(com->char2huffman[kar], com->charlen[kar]);
        if (kar > (NLIT - 1))
        {
          int j = LOG(*s);
  
          ST_BITS(com->ptr2huffman[j], com->ptrlen[j]);
          if (--j > 0)
          {
            ST_BITS(((*s) & (0xffff >> (16 - j))), j);
          }
          s3 += 4;
          s++;
        }
      }
    }
    while(--i!=0);
    {
      long i = (com->charp - com->chars) - entries;
      long ptrctr = s - com->pointers;

      if (com->matchstring != NULL)
      {
        memmove(com->matchstring, s3, i * 4);
        memmove(com->backmatch, com->backmatch+ptrctr, i);
        com->msp -= 4 * (long)ptrctr;
        com->bmp-=ptrctr;
      }
      memmove(com->chars, r, i * sizeof (c_codetype));
      memmove(com->pointers, s, i * sizeof (pointer_type));
      com->ptrp -= ptrctr;
      com->charp -= entries;
    }
  }
  return GUP_OK;
}

#endif


#ifndef NOT_USE_STD_make_hufftable

/*************************    make_hufftable    *************************
 Function:    make_hufftable
 Purpose:     Assign Huffman codes to characters, based on the occurrence
              frequency of each character.  Normally, standard Huffman codes
              are assigned.  However, if this means that the longest Huffman
              code is longer than <max_hufflen> bits, the Huffman codes will
              be re-assigned to make the longest Huffman code not exceed
              <max_hufflen> bits.
 Input:       freq       Occurrence frequency of each character.  freq[i]
                         contains the occurrence frequency for character i.
              totalfreq  Sum of all occurrence frequencies.
              nchar      Number of characters in the character set.
 Output:      table      Huffman code for each character.  table[i] will be
                         the Huffman code for character i.
              len        Length of the Huffman codes for the characters, in
                         bits. len[i] will be the length of table[i].
 Assumptions: 1. Elements 0..N-1 of <freq> are defined by the caller.
              2. Elements -X_CHARS..-1 of <freq> exist and may be
                 modified.
              3. Elements N..2*N-1 of <freq> exist and may be modified.
              4. <totalfreq> is less than 0xFFFF .  Note that this implies that
                 all elements freq[0..N-1] are less than 0xFFFF .
              5. <nchar> is less than or equal to <NC>.
              6. Elements N..2*N-1 of <len> exist and may be modified.
******************************************************************************/

void make_hufftable(uint8 * len,        /* O: lengths of the Huffman codes      */
                    uint16 * table,      /* O: Huffman codes                     */
                    uint16 * freq,       /* I: occurrence frequencies            */
                    uint16 totalfreq,    /* I: sum of all occurrence frequencies */
                    int nchar,         /* I: number of characters in <freq>    */
                    int max_hufflen,   /* I: maximum huffman code length */
                    packstruct * com)
{
  memset(len, 0, nchar);
  if(totalfreq != 0)
  {
    for(;;)
    {
      int up1[X_CHARS + NC + NC + 1];  /* linked list of characters          */
      uint16 xfreq[X_CHARS + NC + NC];   /* modified character frequencies     */
      int *up = (up1 + X_CHARS);

      { /*- link de X_CHARS met elkaar */
        int *p = up1;
        int i = -X_CHARS + 2;

        *p++ = 2 * nchar - 1;          /* terminate up array */
        do
        {
          *p++ = i;
        }
        while(++i < 0);
        *p = -X_CHARS;
      }
      { /*- zet X_CHARS freq op 0xffff */
        int i = HUFF_HIGH_FREQS;
        uint16 tmp = 0xffff;
        uint16 *p = freq;

        do
        {
          *--p = tmp;
        }
        while(--i!=0);
        freq[-X_CHARS] = tmp;
        /* zet freq[] boven nchar op 0xffff */
        i = nchar;
        p = freq + nchar;
        do
        {
          *p++ = tmp;
        }
        while(--i!=0);
      }
      { /*- zet karakters in linked list */
        uint16 *p = freq;                /* current character frequency        */
        int c = 0;                     /* current character                  */

        do
        {
          uint16 currfreq;               /* frequency of the current character <c> */

          if((currfreq = *p++) != 0)
          {
            if(currfreq <= HUFF_MAX_SINGLE_FREQ)
            {
              up[c] = up1[currfreq];
              up1[currfreq] = c;
            }
            else
            {
              int c1 = LOG(currfreq) - 17;  /* 17=maximale LOG waarde +1     */
              int c2;

              for(;;)
              {
                c2 = up[c1];
                if(freq[c2] >= currfreq)
                {
                  up[c1] = c;
                  up[c] = c2;
                  break;
                }
                c1 = up[c2];
                if(freq[c1] >= currfreq)
                {
                  up[c2] = c;
                  up[c] = c1;
                  break;
                }
              }
            }
          }
        }
        while(++c < nchar);
      }
      {
        int mem[NC + NC];
        int *child = mem;              /* left and right child of the pseudo-characters */
        int new_char = nchar;          /* pseudo-character                   */

        {
          int low_p = -X_CHARS + 1;    /* source index for low half          */
          int high_p = nchar;          /* source index for high half         */
          uint16 new_freq;               /* frequency of new character         */

          while((low_p = up[low_p]) < 0)
          {
            ;
          }
          if((new_freq = freq[low_p]) == totalfreq)
          { /*- there is only one character in the array, we're done */
            return;
          }
          *child++ = low_p;            /* define first child of pseudo-char */
          while((low_p = up[low_p]) < 0) /* find next char of low half */
          {
            ;
          }
          for(;;)
          {
            if (freq[low_p] <= freq[high_p])
            {
              *child++ = low_p;        /* 2nd child of pseudo-char */
              freq[new_char++] = new_freq + freq[low_p];  /* frequency of pseudo-char */
              while((low_p = up[low_p]) < 0) /* find next char */
              {
                ;
              }
            }
            else
            {
              *child++ = high_p;       /* 2nd child of pseudo-char */
              freq[new_char++] = new_freq + freq[high_p++]; /* frequency of pseudo-char */
            }
            if((new_freq = freq[low_p]) <= freq[high_p])
            {
              *child++ = low_p;        /* define first child of pseudo-char */
              while((low_p = up[low_p]) < 0) /* find next char */
              {
                ;
              }
            }
            else
            {
              if((new_freq = freq[high_p]) >= totalfreq)
              {
                break;
              }
              else
              {
                *child++ = high_p++;   /* define first child of pseudo-char */
              }
            }
          }
        }
        {
          uint8 *p = len + new_char;
          uint8 tmp;

          p[-1] = 0;                   /* len[new_char]=0 */
          new_char -= nchar;
          do
          {
            tmp = *--p + 1;
            len[*--child] = tmp;
            len[*--child] = tmp;
          }
          while(--new_char > 0);
          if(tmp <= max_hufflen)
          {
            make_huffmancodes(table, len, nchar);
            return;
          }
          else
          {
            if((xfreq + X_CHARS) != freq)
            {
              memcpy(xfreq + X_CHARS, freq, nchar * sizeof (*freq));
              freq = xfreq + X_CHARS;
            }
            freq[*child]++;            /* fix lowest freq */
            do
            {
              child += 2;
            }
            while(*p++ != 0);
            while(*--child >= nchar)
            {
              ;
            }
            freq[*child]--;            /* fix highest freq */
          }
        }
      }
    }
  }
}

#endif

#ifndef NOT_USE_STD_make_huffmancodes

/************************    make_huffmancodes    ************************
 Function:    make_huffmancodes
 Purpose:     Generate Huffman codes, based on the Huffman code lengths of the
              characters.
 Input:       len    Length of the Huffman codes for the characters, in bits.
                     len[i] will be the length of table[i].
              nchar  Number of characters in the character set.
 Output:      table  Huffman code for each character.  table[i] will be the
                     Huffman code for character i.
 Assumptions: For all len[i] with 0<=i<nchar: 0<=len[i]<=MAHUFFLEN.

******************************************************************************/

void make_huffmancodes(uint16 * table,   /* Tabel waarin de huffman codes komen te staan */
                       uint8 * len,     /* lengte van de karakters            */
                       int nchar)      /* aantal karakters                   */
{
  uint16 count[MAX_HUFFLEN + 1];
  uint16 huff[MAX_HUFFLEN + 1];

  { /*- hoeveel van iedere lengte hebben we eigenlijk? */
    int i = nchar;
    uint8 *p = len;

    memset(count, 0, sizeof (count));
    do
    {
      count[*p++]++;
    }
    while(--i!=0);
  }

  { /*- bereken eerste huffmancode van iedere lengte */
    int i = MAX_HUFFLEN - 1;           /* de eerste huffcode is gegarandeerd 0 */
    uint16 *p = huff + 1;                /* huff[0] doet er niet toe           */
    uint16 *q = count + 1;               /* count[0]==0 overslaan dus          */
    uint16 tmp = 0;

    *p++ = tmp;                        /* de huffmancode van len[1] komt met nul garantie */
    do
    {
      tmp += *q++;
      tmp += tmp;
      *p++ = tmp;
    }
    while(--i!=0);
  }
  { /*- assign huffman codes to characters */
    uint16 *p = table;
    uint8 *q = len;

    do
    {
      *p++ = huff[*q++]++;
    }
    while(--nchar > 0);
  }
}

#endif

#ifndef NOT_USE_STD_count_bits

unsigned long count_bits(unsigned long *header_size,  /* komt header size in bits in te staan */
                         unsigned long *message_size, /* komt message size in bits in te staan */
                         unsigned long *packed_bytes, /* aantal bytes dat gepacked wordt */
          /* nu de variabelen die nodig zijn voor de berekening */
                         int charct,   /* hoogste character dat in gebruik is */
                         uint16 entries, /* aantal character die moeten worden gepacked */
                         uint8 * charlen,  /* character lengte tabel          */
                         uint8 * ptrlen,/* pointerlengte tabel                */
                         uint16 * charfreq, /* karakter frequentie tabel       */
                         uint16 * ptrfreq, /* pointer frequentie tabel           */
                         packstruct *com
)
{
  unsigned long header_bits = 0;
  unsigned long message_bits = 0;
  uint16 freq1[NCPT + NCPT + X_CHARS];
  uint16 *freq = freq1 + X_CHARS;        /* frequentie tabel, freq[-1] bestaat */

  com->special_header=NORMAL_HEADER;
  { /*- bereken aantal bytes dat gepacked gaat worden */
    unsigned long packed = entries;    /* alle karakters + pointers          */
    int i;
    uint16 *p = charfreq + NLIT;
    long j = 2;

    for (i = NLIT; i < charct; i++)
    {
      packed += (long)*p++ * j;
      j++;
    }
    *packed_bytes = packed;
  }
  /* aantal gepackte bytes is berekend */
  /*
   * we hebben nu de huffman codes van de karakterset berekend, nu moeten
   * we de lengtes gaan coderen. deze staan in charlen c_len coderings
   * blok: lengte van de pointers die c_len coderen, er zijn 19 pointers.
   */
  {
    uint16 ptr_count = 0;

    /* frequentie tabel op nul zetten voor gebruik pointers */
    int i;

    memset(freq, 0, NCPT * sizeof (*freq));
    /* frequentie character lengtes tellen */
    for (i = 0; i < charct; i++)
    {
      if (charlen[i])
      {
        ptr_count++;
        freq[charlen[i] + 2]++;
      }
      else
      { /*- charlen nul krijgt een speciale behandeling */
        int nulct = 1;

        while (!charlen[i + nulct])
        {
          nulct++;
        }
        if (nulct < 3)
        {
          freq[0] += (uint16)nulct;
          ptr_count += (uint16)nulct;
        }
        else
        {
          if (nulct < 20)
          {
            freq[1]++;
            if (nulct == 19)
            {
              freq[0]++;
              ptr_count++;
            }
          }
          else
          {
            freq[2]++;
          }
          ptr_count++;
        }
        i += nulct - 1;
      }
    }
    make_hufftable(com->ptrlen1, com->ptr2huffman1, freq, ptr_count, NCPT, MAX_HUFFLEN, com);
    /*
     * Nu zijn alle ptrs gedefinieerd, stuur ze de ARJ file in
    */
    header_bits += 16;                 /* aantal huffman karakters */
    {
      /*
       * belangrijk item, wat zijn de gevallen dat er slechts 1
       * pointerlengte overgedragen hoeft te worden? er is maar 1 pointer
       * lengte er is maar 1 karakter (dat kan wel meerdere ptrlens
       * veroorzaken)
      */
      int vp = 0;
      int np = 1;
      uint8 *p = charlen;
      int len = *p;

      i = charct;
      do
      {
        int tmp = *p++;

        if (tmp)
        {
          vp++;
          if (tmp != len)
          {
            np = 0;
          }
        }
        else
        {
          np = 0;
        }
      }
      while (--i!=0);
      if ((vp < 2) || np)
      { /*- special case 1, er is maar een character lengte */
        header_bits += 10;
      }
      else
      {
        int ptrct = NCPT;
        int extra_add = 0;

        while (!com->ptrlen1[ptrct - 1])
        {
          ptrct--;
        }
        if (com->ptrlen1[3] == 0)
        {
          extra_add = 1;
          if (com->ptrlen1[4] == 0)
          {
            extra_add = 2;
            if (com->ptrlen1[5] == 0)
            {
              extra_add = 3;
            }
          }
        }
        header_bits += 5;              /* aantal pointers dat er aan komt */
        for (i = 0; i < ptrct; i++)
        {
          if (com->ptrlen1[i] < 7)
          {
            header_bits += 3;
          }
          else
          {
            header_bits += com->ptrlen1[i] - 7 + 3 + 1;
          }
          if (i == 2)
          {
            header_bits += 2;
            i += extra_add;
          }
        }
      }
    }
    /*
     * charlen overgedragen, breng characters
    */
    {
      /*
       * De enige special case voor de characters is dat er maar een
       * character is.
      */
      uint16 vp = 0;
      uint8 *p = charlen;

      i = charct;
      do
      {
        if (*p++)
        {
          vp++;
        }
      }
      while (--i!=0);
      if (vp < 2)
      { /*- special case 2, er is maar een karakter lengte */
        header_bits += 18;
      }
      else
      {
        uint16 *p = freq;
        uint8 *q = com->ptrlen1;

        header_bits += 9;
        i = NCPT;
        header_bits += 4 * freq[1];
        header_bits += 9 * freq[2];
        do
        {
          header_bits += (unsigned long)*p++ * (unsigned long)*q++;
        }
        while (--i!=0);
        /*
         * stuur de gecodeerde message
        */
        {
          uint8 *q = charlen;
          uint16 *p = charfreq;

          i = charct;
          do
          {
            message_bits += (long)*p++ * (long)*q++;
          }
          while (--i!=0);
        }
      }
    }
    /*
     * charlen is overgestuurd, nu weer een ptrlen
    */
    {
      /*
       * wat is de specialcase voor de pointers? 1 er is maar een
       * pointerlengte
      */
      uint16 vp = 0;
      uint8 *p = ptrlen;

      i = com->n_ptr;
      do
      {
        if (*p++)
        {
          vp++;
        }
      }
      while (--i!=0);
      if (vp < 2)
      { /*- special case 3, er is maar een pointerlengte */
        header_bits += com->m_ptr_bit+com->m_ptr_bit;
        { /*- bereken ruimte ingenomen door pointers */
          int j = 1;
          int i = com->n_ptr - 2;
          uint16 *p = ptrfreq + 2;

          do
          {
            message_bits += (long)*p++ * (long)j++;
          }
          while (--i!=0);
        }
      }
      else
      {
        int ptrct = com->n_ptr;

        while ((ptrct) && (!ptrlen[ptrct - 1]))
        {
          ptrct--;
        }
        header_bits += com->m_ptr_bit;
        for (i = 0; i < ptrct; i++)
        {
          if (ptrlen[i] < 7)
          {
            header_bits += 3;
          }
          else
          {
            header_bits += ptrlen[i] - 7 + 3 + 1;
          }
        }
        { /*- bereken ruimte ingenomen door pointers */
          int j = -1;
          uint8 *q = ptrlen;
          uint16 *p = ptrfreq;

          (*ptrlen)++;
          i = com->n_ptr;
          do
          {
            message_bits += (long)*p++ * (long)(*q++ + j);
            j++;
          }
          while (--i!=0);
          (*ptrlen)--;
        }
      }
    }
  }
  /*
   * als er alleen maar matches van 3 en 4 zijn kan alles naar karakters worden
   * omgezet, door gebruik te maken van een minimum ASCII header 
   */
  if(((message_bits+header_bits)>>3)>(*packed_bytes) && (*packed_bytes <= MAX_ENTRIES))
  { /* is een conversie mogelijk */
    if((com->matchstring!=NULL) && (charct<=(NLIT+2)))
    { /* conversie is mogelijk */
      if((message_bits+header_bits)>((*packed_bytes<<3)+MIN_ASCII_HEADER))
      { /* conversie */
        unsigned long size=MIN_ASCII_HEADER+(*packed_bytes<<3);
        com->special_header=SPECIAL_MIN_ASCII_HEADER;
        message_bits=size-header_bits;
      }
    }
  }
  *header_size = header_bits;
  *message_size = message_bits;
  return header_bits + message_bits;
}

#endif


#ifndef NOT_USE_STD_compress_m4

gup_result compress_m4(packstruct *com)
{
  /*
   * pointer lengte codering:
   *
   * 9  0xxxxxxxxx            0 -   511  10 bits
   *
   * 10 10xxxxxxxxxx        512 -  1535  12 bits
   *
   * 11 110xxxxxxxxxxx     1536 -  3583  14 bits
   *
   * 12 1110xxxxxxxxxxxx   3584 -  7679  16 bits
   *
   * 13 1111xxxxxxxxxxxxx  7680 - 15871  17 bits
   *
   * len codering:
   *
   * 0 0               : literal
   *
   * 1 10x             :  3 -   4   3 bits
   *
   * 2 110xx           :  5 -   8   5 bits
   *
   * 3 1110xxx         :  9 -  16   7 bits
   *
   * 4 11110xxxx       : 17 -  32   9 bits
   *
   * 5 111110xxxxx     : 33 -  64  11 bits
   *
   * 6 1111110xxxxxx   : 65 - 128  13 bits
   *
   * 7 1111111xxxxxxx  :129 - 256  14 bits
  */
  uint16 entries = (uint16) (com->charp - com->chars);
  c_codetype *p = com->chars;
  pointer_type *q = com->pointers;
  if (entries > com->hufbufsize)
  {
    entries = com->hufbufsize;
  }
  if (com->matchstring != NULL)
  {
    ARJ_Assert(com->backmatch!=NULL);
    {
      uint16 pointer_count=0;
      uint16 i=entries;
      c_codetype *p = com->chars;
      do
      {
        if(*p++>(NLIT-1))
        {
          pointer_count++;
        }
      }
      while (--i!=0);
      com->backmatch[pointer_count]=0; /* om te voorkomen dat hij een backmatch over
                                          de huffmangrens vindt */
    }
    {
      /* 
        Code die backmatch stringlengtes optimaliseert.
        Bij een backmatch zijn er twee opeenvolgende matches, waarbij 
        de laatste match een backmatch waarde groter dan nul heeft.
        Deze laatste macth kunnen we groter laten worden tenkoste van de 
        grootte van de match die ervoor ligt...
      */
      uint8 *lenp=com->fast_log+32768L; /* len tabel */
      int redo; /* geeft aan dat er een conversie heeft plaatsgevonden -> nog een iteratie */
      do
      {
        redo=0;
        {
          uint8* bp=com->backmatch;
          c_codetype *p = com->chars;
          uint16 i = entries;
          do
          {
            c_codetype kar = *p++;
            if (kar > (NLIT-1))
            {
              uint8 len=*bp++;
              if(len>0)
              {
                c_codetype kar_1=p[-2];
                uint8 offset=1;
                uint8 optlen=lenp[kar_1]+lenp[kar];
                do
                {
                  if((lenp[kar_1-offset]+lenp[kar+offset])<optlen)
                  {
                    redo=1;
                    bp[-1]-=offset;
                    p[-2]-=offset;
                    p[-1]+=offset;
                    kar+=offset;
                    kar_1-=offset;
                    optlen=lenp[kar_1]+lenp[kar];
                    offset=1;
                  }
                  else
                  {
                    offset++;
                  }
                }
                while(--len!=0);
              }
            }
          }
          while (--i!=0);
        }
        #if 0
          { /*- for debugging use */
            long lentries = (uint16) (com->charp - com->chars);
            {
              long i = lentries;
              c_codetype *cp = com->chars;
              while (--i!=0)
              {
                c_codetype tmp = *cp++;
                ARJ_Assert(tmp>=0);
                ARJ_Assert(tmp<=(NLIT-MIN_MATCH+com->max_match))
                if(tmp<0)
                {
                  fprintf(redir, "te klein!");
                }
                if(tmp>(NLIT-MIN_MATCH+com->max_match))
                {
                  fprintf(redir, "te groot!");
                }
              }
            }
          }
        #endif
      }
      while(com->jm && (redo));
    }
  }
  {
    unsigned long m_size;
    long bits_comming = count_m4_bits(&m_size, com, entries, com->chars, com->pointers);
    if (com->mv_mode)
    {
      unsigned long bits;
      bits=bits_comming+com->mv_bits_left+7;
      bits >>= 3;                      /* bytes=bits/8 */
      if (bits > com->mv_bytes_left)
      { /*- gedonder!, MV break! */
        uint16 delta_size;
        uint16 the_size = 1;

        com->mv_next = 1;
        delta_size=0x8000;
        while(delta_size>=entries)
        {
          delta_size>>=1;
        }
        entries -= delta_size;

        do
        {
          delta_size >>= 1;
          bits = count_m4_bits(&m_size, com, entries, com->chars, com->pointers)+
                 com->mv_bits_left+7;
          bits >>= 3;                    /* bytes=bits/8 */
          if (bits > com->mv_bytes_left)
          {
            if(delta_size>=entries)
            {
              entries=1;
            }
            else
            {
              entries -= delta_size;
            }
          }
          else
          {
            the_size = entries;
            entries += delta_size;
          }
        }
        while (delta_size!=0);
        if ((the_size == 1) && (bits > com->mv_bytes_left))
        {
          return GUP_OK;
        }
        entries = the_size;
        bits_comming = count_m4_bits(&m_size, com, entries, com->chars, com->pointers);
      }
      bits=bits_comming+com->mv_bits_left;
      com->mv_bits_left = (int16)(bits&7);
      bits>>=3;
      com->mv_bytes_left -= bits;
    }
    {
      gup_result res;
      if((res=announce((bits_comming+7)>>3, com))!=GUP_OK)
      {
        return res;
      }
      bits_comming+=com->bits_rest;
      com->bits_rest=(int16)(bits_comming&7);
      com->packed_size += bits_comming>>3;
    }
    #ifdef PP_AFTER
    com->print_progres(m_size, com->pp_propagator);
    #endif
    com->bytes_packed += m_size;
  }
  entries++;
  while (--entries != 0)
  {
    c_codetype kar = *p++;

    if (kar < NLIT)
    { /*- store literal */
      ST_BITS(kar, 9);
      LOG_LITERAL(kar);
    }
    else
    {
      kar += MIN_MATCH - NLIT;
      LOG_PTR_LEN(kar, *q);
      if (kar < 17)
      {
        if (kar < 5)
        {
          ST_BITS(2, 2);
          ST_BITS(kar - 3, 1);
        }
        else
        {
          if (kar < 9)
          {
            ST_BITS(6, 3);
            ST_BITS(kar - 5, 2);
          }
          else
          {
            ST_BITS(14, 4);
            ST_BITS(kar - 9, 3);
          }
        }
      }
      else
      {
        if (kar < 65)
        {
          if (kar < 33)
          {
            ST_BITS(30, 5);
            ST_BITS(kar - 17, 4);
          }
          else
          {
            ST_BITS(62, 6);
            ST_BITS(kar - 33, 5);
          }
        }
        else
        {
          if (kar < 129)
          {
            ST_BITS(126, 7);
            ST_BITS(kar - 65, 6);
          }
          else
          {
            ST_BITS(127, 7);
            ST_BITS(kar - 129, 7);
          }
        }
      }
      kar = *q++;
      if (kar < 1536)
      {
        if (kar < 512)
        {
          ST_BITS(0, 1);
          ST_BITS(kar, 9);
        }
        else
        {
          ST_BITS(2, 2);
          ST_BITS(kar - 512, 10);
        }
      }
      else
      {
        if (kar < 3584)
        {
          ST_BITS(6, 3);
          ST_BITS(kar - 1536, 11);
        }
        else
        {
          if (kar < 7680)
          {
            ST_BITS(14, 4);
            ST_BITS(kar - 3584, 12);
          }
          else
          {
            ST_BITS(15, 4);
            ST_BITS(kar - 7680, 13);
          }
        }
      }
    }
  }
  entries=(uint16)(com->charp-p);
  #if 0
  {
    long i = (com->charp - com->chars) - entries;
    long ptrctr = s - com->pointers;
    if (com->matchstring != NULL)
    {
      memmove(com->matchstring, s3, i * 4);
      memmove(com->backmatch, com->backmatch+ptrctr, i);
      com->msp -= 4 * (long)ptrctr;
      com->bmp-=ptrctr;
    }
    memmove(com->chars, r, i * sizeof (c_codetype));
    memmove(com->pointers, s, i * sizeof (pointer_type));
    com->ptrp -= ptrctr;
    com->charp -= entries;
    ARJ_Assert_ZEEF34();
  }
  #else
  if (com->matchstring != NULL)
  {
    long ptrctr = q - com->pointers;
    long i = (com->charp - com->chars) - entries;
    memmove(com->backmatch, com->backmatch+ptrctr, i);
    com->msp -= 4 * (long)ptrctr;
    com->bmp-=ptrctr;
  }
  com->charp = com->chars+entries;
  com->ptrp = com->pointers+(com->ptrp-q);
  memmove(com->chars, p, entries*sizeof(*p));
  memmove(com->pointers, q, entries*sizeof(*q));
  #endif
  return GUP_OK;
}

#endif

#ifndef NOT_USE_STD_count_m4_bits

unsigned long count_m4_bits(unsigned long *packed_bytes,  /* aantal bytes dat gepacked wordt */
          /* nu de variabelen die nodig zijn voor de berekening */
                            packstruct *com, /* commadstruct */
                            uint16 entries, /* aantal character die moeten worden gepacked */
                            c_codetype * p, /* pointer naar de karakters     */
                            pointer_type * q  /* pointer naar de pointers    */
)
{
  unsigned long bits = 0;
  unsigned long bytes = 0;

  entries++;
  while (--entries != 0)
  {
    c_codetype kar = *p++;

    if (kar < NLIT)
    { /*- store literal */
      bytes++;
      bits += 9;
    }
    else
    {
      uint8 *lenp=com->fast_log+32768L; /* len tabel */
      bytes += kar + MIN_MATCH - NLIT;
      bits+=lenp[kar];
      kar = *q++;
      bits+=LOG(kar);
    }
  }
  *packed_bytes = bytes;
  return bits;
}

#endif


#ifndef NOT_USE_STD_compress_n1

int32 first_bit_set32(uint32 u);
int n1_lit_len(uint32 val);
int n1_len_len(uint32 val);
int n1_ptr_len(uint32 val);
void store_n1_val(uint32 val, packstruct *com);
void store_n1_len_val(uint32 val, packstruct *com);
void store_n1_literal_val(uint32 val, packstruct *com);
void store_n1_ptr_val(int32_t val, packstruct *com);


/*
** Voor de n1 packer zijn meerdere log functies nodig, 
** er wordt voor gekozen om deze niet via een look-up tabel te doen 
** maar via een functie. De compiler kan deze mooi inlinen 
** en maak gebruik van intrinsic log2 gcc:
** int32_t log2_uint32 (uint32_t u)
** {
**   if (u == 0) { return INT32_MIN; }
**
**  return ((int32_t)31 - (int32_t)__builtin_clz(u));
** }
**/

int32 first_bit_set32(uint32 u)
{ /* first set bit at... pos 1 .. 32, 0 means no bits set */
  if (u == 0) { return 0; }
  return ((int32)32 - (int32)__builtin_clz(u));
}

int n1_lit_len(uint32 val)
{ /* bereken de code lengte voor val, 1 <= val <= 2^32 -1 */
	return 2*(first_bit_set32(val+1)-1)+1;
}

int n1_len_len(uint32 val)
{ /* bereken de code lengte voor val, 2 <= val <= 2^32 */
	return 2*(first_bit_set32(val)-1)+1;
}

int n1_ptr_len(uint32 val)
{ /* bereken de code lengte voor val, 0 <= val <= 65536 */
	if(val<256)
	{
		return 9;
	}
	else
	{
		return 17;
	}
}

#define ST_BIT_N1(bit)												\
{ /* store a 1 or a 0 */											\
  int val=bit;				                                 \
  LOG_bit(val);														\
  if(com->bits_in_bitbuf==0)										\
  { /* reserveer plek in bytestream */							\
  	 com->command_byte_ptr=com->rbuf_current++;				\
  	 com->bitbuf=0;													\
  }																		\
  com->bits_in_bitbuf++;											\
  com->bitbuf+=com->bitbuf+val;									\
  if (com->bits_in_bitbuf >= 8)									\
  {																		\
    com->bits_in_bitbuf=0;											\
    *com->command_byte_ptr=(uint8)com->bitbuf;				\
  }																		\
}

void store_n1_val(uint32 val, packstruct *com)
{ /* waarde val >=2 */
	int bits_to_do=first_bit_set32(val)-1;
	uint32 mask=1<<bits_to_do;
	mask>>=1;
	do
	{
		if((val&mask)==0)
		{
			ST_BIT_N1(0);
		}
		else
		{
			ST_BIT_N1(1);
		}
		mask>>=1;
		if(mask==0)
		{
			ST_BIT_N1(0);
		}
		else
		{
			ST_BIT_N1(1);
		}
	}while(mask!=0);
}

void store_n1_len_val(uint32 val, packstruct *com)
{ /* waarde val >=2 */
	store_n1_val(val, com);
	ST_BIT_N1(1);
}

void store_n1_literal_val(uint32 val, packstruct *com)
{ /* waarde val >=1 */
	store_n1_val(val+1, com);
	ST_BIT_N1(0);
}

void store_n1_ptr_val(int32_t val, packstruct *com)
{ /* waarde val >=0 <=65535 */
	val++;
	if(val<=256)
	{
		val=-val;
		*com->rbuf_current++ = (uint8) (val&0xff);
		ST_BIT_N1(0);
	}
	else
	{
		val=-val;
		*com->rbuf_current++ = (uint8) ((val>>8)&0xff);
		ST_BIT_N1(1);
		*com->rbuf_current++ = (uint8) (val&0xff);
	}
}

gup_result compress_n1(packstruct *com)
{
  /*
   * pointer lengte codering:
   * 8 xxxxxxxx0              0 -   255   9 bits
   * 16 xxxxxxxx1xxxxxxxx   256 - 65536  17 bits
   *
   * len codering:
   * 1 x0              :  2 -   3   2 bits
   * 2 x1x0            :  4 -   7   4 bits
   * 3 x1x1x0          :  8 -  15   6 bits
   * 4 x1x1x1x0        : 16 -  31   8 bits
   * 5 x1x1x1x1x0      : 32 -  63  10 bits
   * 6 x1x1x1x1x1x0    : 64 - 127  12 bits
   * 7 x1x1x1x1x1x1x0  :128 - 255  14 bits
   * 8 x1x1x1x1x1x1x1x0:129 - 256  16 bits
   * enz...
   *   
  */
  uint16 entries = (uint16) (com->charp - com->chars);
  c_codetype *p = com->chars;
  pointer_type *q = com->pointers;
  uint8* start;
  if (entries > com->hufbufsize)
  {
    entries = com->hufbufsize;
  }
  if (com->matchstring != NULL)
  {
    ARJ_Assert(com->backmatch!=NULL);
    {
      uint16 pointer_count=0;
      uint16 i=entries;
      c_codetype *p = com->chars;
      do
      {
        if(*p++>(NLIT-1))
        {
          pointer_count++;
        }
      }
      while (--i!=0);
      com->backmatch[pointer_count]=0; /* om te voorkomen dat hij een backmatch over de huffmangrens vindt */
    }
    {
      /* 
        Code die backmatch stringlengtes optimaliseert.
        Bij een backmatch zijn er twee opeenvolgende matches, waarbij 
        de laatste match een backmatch waarde groter dan nul heeft.
        Deze laatste macth kunnen we groter laten worden tenkoste van de 
        grootte van de match die ervoor ligt... We optimaliseren de lengte.
      */
      uint8* bp=com->backmatch;
      c_codetype *p = com->chars;
      uint16 i = entries;
      do
      {
        c_codetype kar = *p++;
        if (kar >= (NLIT))
        { /* gevonden een ptr-len */
          uint8 len=*bp++; /* backmatch lengte */
          if(len>0)
          { /* we kunnen een backmatch doen */
            c_codetype kar_1=p[-2]+MIN_MATCH-NLIT; /* lengte van de vorige match */
            uint8 offset=1;
            uint8 optlen;
            kar+=MIN_MATCH-NLIT; /* matchlengte huidige match */
            optlen=n1_len_len(kar_1)+n1_len_len(kar);
            do
            {
              if((n1_len_len(kar_1-offset)+n1_len_len(kar+offset))<optlen)
              {
                bp[-1]-=offset;
                p[-2]-=offset;
                p[-1]+=offset;
                kar+=offset;
                kar_1-=offset;
                optlen=n1_len_len(kar_1)+n1_len_len(kar);
                offset=1;
              }
              else
              {
                offset++;
              }
            }
            while(--len!=0);
          }
        }
      }
      while (--i!=0);
    }
  }
  {
    unsigned long m_size;
    long bits_comming = count_n1_bits(&m_size, entries, com->chars, com->pointers);
    {
      gup_result res;
      if((res=announce((bits_comming+7)>>3, com))!=GUP_OK)
      {
        return res;
      }
      bits_comming+=com->bits_rest;
      com->bits_rest=(int16)(bits_comming&7);
      com->packed_size += bits_comming>>3;
    }
    #ifdef PP_AFTER
    com->print_progres(m_size, com->pp_propagator);
    #endif
    com->bytes_packed += m_size;
  }
  start=com->rbuf_current;
  {
    uint16 literal_run = 0;
    c_codetype* literal_run_start=p;
    entries++;
    while (--entries != 0)
    {
      c_codetype kar = *p++;
      
      if (kar < NLIT)
      { /*- store literal */
        literal_run++;
      }
      else
      {
        if(literal_run>0)
        {
          store_n1_literal_val(literal_run, com);
          LOG_LITERAL_RUN(literal_run);
          do
          {
	        LOG_LITERAL(*literal_run_start);
            *com->rbuf_current++=*literal_run_start++;
          } while(--literal_run!=0); /* aan het einde van deze loop is literal_run weer 0 */
        }
        kar += MIN_MATCH - NLIT;
        store_n1_len_val(kar, com);
        store_n1_ptr_val(*q++, com);
        LOG_PTR_LEN(kar, q[-1]+1);
        literal_run_start=p;
      }
    }
    if(literal_run>0)
    {
      store_n1_literal_val(literal_run, com);
      LOG_LITERAL_RUN(literal_run);
      do
      {
        LOG_LITERAL(*literal_run_start);
        *com->rbuf_current++=*literal_run_start++;
      } while(--literal_run!=0); /* aan het einde van deze loop is literal_run weer 0 */
    }
  }
  printf("Packed size =%li\n", com->rbuf_current-start);
  entries=(uint16)(com->charp-p);
  if (com->matchstring != NULL)
  {
    long ptrctr = q - com->pointers;
    long i = (com->charp - com->chars) - entries;
    memmove(com->backmatch, com->backmatch+ptrctr, i);
    com->msp -= 4 * (long)ptrctr;
    com->bmp-=ptrctr;
  }
  com->charp = com->chars+entries;
  com->ptrp = com->pointers+(com->ptrp-q);
  memmove(com->chars, p, entries*sizeof(*p));
  memmove(com->pointers, q, entries*sizeof(*q));
  return GUP_OK;
}

#endif

#ifndef NOT_USE_STD_count_n1_bits

unsigned long count_n1_bits(unsigned long *packed_bytes,  /* aantal bytes dat gepacked wordt */
                            uint16 entries, /* aantal character die moeten worden gepacked */
                            c_codetype * p, /* pointer naar de karakters     */
                            pointer_type * q  /* pointer naar de pointers    */
									)
{
  unsigned long bits = 0;
  unsigned long bytes = 0;
  uint16 literal_run = 0;

  entries++;
  while (--entries != 0)
  {
    c_codetype kar = *p++;

    if (kar < NLIT)
    { /* store literal */
    	literal_run++;
    }
    else
    { /* ptr-len paar */
    	if(literal_run>0)
    	{ /* bereken ruimte voor de literals */
    		bytes+=literal_run;
    		bits+=literal_run*8;
    		bits+=n1_lit_len(literal_run);
    		literal_run=0;
    	}
    	kar+=MIN_MATCH-NLIT;
      bytes+=kar;
      bits+=n1_len_len(kar);
      kar=*q++;
      bits+=n1_ptr_len(kar);
    }
  }
  if(literal_run>0)
  { /* bereken ruimte voor de literals */
    bytes+=literal_run;
    bits+=literal_run*8;
    bits+=n1_lit_len(literal_run);
  }
  *packed_bytes = bytes;
  return bits;
}

#endif

#ifndef NOT_USE_STD_init_n1_fast_log

void init_n1_fast_log(packstruct *com)
{
  /*
   * fastlog tabel voor ni mode 1.
   */
  /*
   * pointer lengte codering:
   *  9: xxxxxxxx0             0 -   255 9 bits
   * 16: xxxxxxxx1xxxxxxxx     0 - 65535 17 bits
   */
  uint8 *p = com->fast_log;
  memset(p, 9, 256);
  p+=256;
  memset(p, 17, 65536-256);
}

#endif

#ifndef NOT_USE_STD_compress_lzs

gup_result compress_lzs(packstruct *com)
{ /* LZS and LZ5 compression at: https://github.com/fragglet/lhasa/tree/master/lib */
  uint16 entries = (uint16) (com->charp - com->chars);
  unsigned int i;
  c_codetype *p = com->chars;
  pointer_type *q = com->pointers;
  uint16 dic_size=2047;
  uint16 pos=(uint16)((dic_size-17+com->bytes_packed)&dic_size);
  if(entries==0)
  {
    return GUP_OK;
  }
  {
    unsigned long m_size;
    long bits_comming=(entries<<3) + entries;
    m_size=entries;
    {
      i=entries;
      do
      {
        c_codetype kar=*p++;
        if(kar>=NLIT)
        {
          bits_comming+=7;
          m_size+=kar-NLIT+MIN_MATCH-1;
        }
      }
      while(--i != 0);
    }
    {
      gup_result res;
      if((res=announce((bits_comming+7)>>3, com))!=GUP_OK)
      {
        return res;
      }
      bits_comming+=com->bits_rest;
      com->bits_rest=(int16)(bits_comming&7);
      com->packed_size += bits_comming>>3;
    }
    #ifdef PP_AFTER
    com->print_progres(m_size, com->pp_propagator);
    #endif
    com->bytes_packed += m_size;
  }
	{
      p = com->chars;
		i = entries;
		do
		{
		  c_codetype kar=*p++;
			if(kar<NLIT)
			{ /* literal */
           ST_BITS(0x100+kar, 9);
			  pos++;
			}
			else
			{ /* pointer len */
			  ST_BITS((pos-*q++)&dic_size, 12);
			  ST_BITS(kar-NLIT+1, 4);
           pos+=kar-NLIT+MIN_MATCH;
			}
		}
		while(--i != 0);
	}
  entries=(uint16)(com->charp-p);
  if (com->matchstring != NULL)
  {
    long ptrctr = q - com->pointers;
    long i = (com->charp - com->chars) - entries;
    memmove(com->backmatch, com->backmatch+ptrctr, i);
    com->msp -= 4 * (long)ptrctr;
    com->bmp-=ptrctr;
  }
  com->charp = com->chars+entries;
  com->ptrp = com->pointers+(com->ptrp-q);
  memmove(com->chars, p, entries*sizeof(*p));
  memmove(com->pointers, q, entries*sizeof(*q));
  return GUP_OK;
}
    
#endif

#ifndef NOT_USE_STD_compress_lz5

gup_result compress_lz5(packstruct *com)
{
  uint16 entries = (uint16) (com->charp - com->chars);
  c_codetype *p = com->chars;
  pointer_type *q = com->pointers;
  uint16 dic_size=4095;
  uint16 pos=(uint16)((dic_size-18+com->bytes_packed)&dic_size);
  if (entries > (com->hufbufsize & 0xfff8U))
  {
    entries = com->hufbufsize &0xfff8U;
  }
  else
  {
    if(entries==0)
    {
      return GUP_OK;
    }
  }
  {
    unsigned long m_size;
    long bytes_comming=entries;
    bytes_comming += ((entries+7)>>3);
    m_size=entries;
    {
      c_codetype *p = com->chars;
      uint16 i=entries;
      do
      {
        c_codetype kar=*p++;
        if(kar>=NLIT)
        {
          bytes_comming++;
          m_size+=kar-NLIT+MIN_MATCH-1;
        }
      }
      while(--i!=0);
    }
    {
      if((com->rbuf_current+bytes_comming)>=com->rbuf_tail)
      {
        gup_result res;
        com->bw_buf->current=com->rbuf_current;
        res=com->buf_write_announce(bytes_comming, com->bw_buf, com->bw_propagator);
        if(res!=GUP_OK)
        {
          return res;
        }
        com->rbuf_current=com->bw_buf->current;
        com->rbuf_tail=com->bw_buf->end;
      }
      com->packed_size += bytes_comming;
    }
    #ifdef PP_AFTER
    com->print_progres(m_size, com->pp_propagator);
    #endif
    com->bytes_packed += m_size;
  }
  for(;;)
  {
    uint8 flags=0;
    uint8 *flag_p=com->rbuf_current++;
    c_codetype kar;
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags++;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    if(--entries==0)
    {
      *flag_p=flags;
      break;
    }
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags+=0x02;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    if(--entries==0)
    {
      *flag_p=flags;
      break;
    }
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags+=0x04;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    if(--entries==0)
    {
      *flag_p=flags;
      break;
    }
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags+=0x08;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    if(--entries==0)
    {
      *flag_p=flags;
      break;
    }
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags+=0x10;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    if(--entries==0)
    {
      *flag_p=flags;
      break;
    }
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags+=0x20;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    if(--entries==0)
    {
      *flag_p=flags;
      break;
    }
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags+=0x40;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    if(--entries==0)
    {
      *flag_p=flags;
      break;
    }
    kar=*p++;
    if(kar>=NLIT)
    {
      pointer_type ptr=(pos-*q++)&dic_size;
      pos+=kar-NLIT+MIN_MATCH;
      *com->rbuf_current++ = (uint8)ptr;
      *com->rbuf_current++ = (uint8)(((ptr&0xf00)>>4)|(kar-NLIT));
    }
    else
    {
      flags+=0x80;
      pos++;
      *com->rbuf_current++ = (uint8)kar;
    }
    *flag_p=flags;
    if(--entries==0)
    {
      break;
    }
  }
  entries=(uint16)(com->charp-p);
  if (com->matchstring != NULL)
  {
    long ptrctr = q - com->pointers;
    long i = (com->charp - com->chars) - entries;
    memmove(com->backmatch, com->backmatch+ptrctr, i);
    com->msp -= 4 * (long)ptrctr;
    com->bmp-=ptrctr;
  }
  com->charp = com->chars+entries;
  com->ptrp = com->pointers+(com->ptrp-q);
  memmove(com->chars, p, entries*sizeof(*p));
  memmove(com->pointers, q, entries*sizeof(*q));
  return GUP_OK;
}
    
#endif


#ifndef NOT_USE_STD_re_crc

/*
 * Aanname: origsize > 0
*/
gup_result re_crc(unsigned long origsize, packstruct * com)
{
  unsigned long buflen;
  uint8 *buffer=get_buf(&buflen, com);
  
  if(buffer==NULL)
  {
    gup_result res;
    res=com->buf_write_announce(com->bw_buf->end - com->bw_buf->start, com->bw_buf, com->bw_propagator);
    if(res!=GUP_OK)
    {
      return res;
    }
    buflen=com->bw_buf->end - com->bw_buf->current;
    buffer=com->bw_buf->current;
  }

  for(;;)
  {
    if (buflen > origsize)
    {
      if (com->buf_read_crc(origsize, buffer, com->brc_propagator) < (long)origsize)
      {
        return GUP_READ_ERROR; /* ("Read error"); */
      }
      return GUP_OK; /* succes */
    }
    else
    {
      if (com->buf_read_crc(buflen, buffer, com->brc_propagator) < (long)buflen)
      {
        return GUP_READ_ERROR; /* ("Read error"); */
      }
      origsize-=buflen;
    }
  }
}

#endif

#ifndef NOT_USE_STD_init_fast_log

void init_fast_log(packstruct *com)
{
  uint8 *p = com->fast_log;
  int i;
  *p++ = 0;
  for (i = 0; i < 16; i++)
  {
    memset(p, i + 1, 1UL << i);
    p += 1UL << i;
  }
}

#endif

#ifndef NOT_USE_STD_init_m4_fast_log

void init_m4_fast_log(packstruct *com)
{
  /*
   * fastlog tabel voor mode 4.
   * in de tweede 32k is een len->bitlen tabel.
   */
  /*
   * pointer lengte codering:
   *
   * 9  0xxxxxxxxx            0 -   511  10 bits
   *
   * 10 10xxxxxxxxxx        512 -  1535  12 bits
   *
   * 11 110xxxxxxxxxxx     1536 -  3583  14 bits
   *
   * 12 1110xxxxxxxxxxxx   3584 -  7679  16 bits
   *
   * 13 1111xxxxxxxxxxxxx  7680 - 15871  17 bits
   */
  uint8 *p = com->fast_log;
  memset(p, 10, 512);
  p+=512;
  memset(p, 12, 1024);
  p+=1024;
  memset(p, 14, 2048);
  p+=2048;
  memset(p, 16, 4096);
  p+=4096;
  memset(p, 17, 8192);
  /*
   * 0 8               :  literal     9 bits
   *
   * 1 10x             :  1 -   2     3
   *
   * 2 110xx           :  3 -   6     5
   *
   * 3 1110xxx         :  7 -  14     7
   *
   * 4 11110xxxx       : 15 -  30     9
   *
   * 5 111110xxxxx     : 31 -  62    11
   *
   * 6 1111110xxxxxx   : 63 - 126    13
   *
   * 7 1111111xxxxxxx  :127 - 254    14
   */
  p = com->fast_log+32768L; /* len tabel */
  memset(p, 32, 256); /* voorkom foute conversies */
  p+=256;
  memset(p, 3, 2);
  p+=2;
  memset(p, 5, 4);
  p+=4;
  memset(p, 7, 8);
  p+=8;
  memset(p, 9, 16);
  p+=16;
  memset(p, 11, 32);
  p+=32;
  memset(p, 13, 64);
  p+=64;
  memset(p, 14, 128);
}

#endif

#ifndef NOT_USE_STD_init_lzs_fast_log

void init_lzs_fast_log(packstruct *com)
{
  uint8 *p = com->fast_log;
  memset(p, 4096, 7); /* willekeurige waarde, klein genoeg om geen alarm te slaan */
}

#endif

#ifndef NOT_USE_STD_init_lz5_fast_log

void init_lz5_fast_log(packstruct *com)
{
  uint8 *p = com->fast_log;
  memset(p, 4096, 7); /* willekeurige waarde, klein genoeg om geen alarm te slaan */
}

#endif

 

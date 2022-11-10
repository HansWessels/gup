#include "encode.h"
#include "compress.h"
#include "decode.h"

unsigned long m4_count_bits(unsigned long* packed_bytes,
                            packstruct *com,
                            uint16 entries, c_codetype *chars,
                            pointer_type *ptrs);

#if 0
	/* log literal en pointer len combi's */
	static unsigned long log_pos_counter=0;
	#define LOG_LITERAL(lit)  {printf("%lX Literal: %02X\n", log_pos_counter, lit); log_pos_counter++;}
	#define LOG_PTR_LEN(len, ptr) {printf("%lX Len: %u, ptr: %u\n", log_pos_counter ,len, ptr); log_pos_counter+=len;}
	#define LOG_BIT(bit) // printf("bit = %i\n",bit);
  	#define LOG_RUN(run) printf("Run = %lu\n", run);
	#define LOG_COUNTER_RESET log_pos_counter=0;
	#define LOG_TEXT(string) printf(string);
#else
	#define LOG_LITERAL(lit) /* */
	#define LOG_PTR_LEN(len, ptr) /* */
	#define LOG_BIT(bit) /* */
	#define LOG_RUN(run) /* */
 	#define LOG_COUNTER_RESET
	#define LOG_TEXT(string) /* */
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


gup_result m4_compress(packstruct *com)
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
                  fprintf(assert_redir_fptr, "te klein!");
                }
                if(tmp>(NLIT-MIN_MATCH+com->max_match))
                {
                  fprintf(assert_redir_fptr, "te groot!");
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
    long bits_comming = m4_count_bits(&m_size, com, entries, com->chars, com->pointers);
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
          bits = m4_count_bits(&m_size, com, entries, com->chars, com->pointers)+
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
        bits_comming = m4_count_bits(&m_size, com, entries, com->chars, com->pointers);
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

unsigned long m4_count_bits(unsigned long *packed_bytes,  /* aantal bytes dat gepacked wordt */
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


gup_result m4_close_stream(packstruct *com)
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
  if(com->mv_mode)
  {
    if(com->mv_bits_left!=0)
    {
      com->mv_bytes_left--;
    }
  }
  if(com->bits_rest!=0)
  {
    com->packed_size++; /* corrigeer packed_size */
  }
  return GUP_OK;
}

void m4_init_fast_log(packstruct *com)
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


/********************************************************************
 *                                                                  *
 * ARJ Shell.                                                       *
 *                                                                  *
 ********************************************************************
 *
 * $Author: hans $
 * $Date: 2000-08-27 12:31:34 +0200 (Sun, 27 Aug 2000) $
 * $Revision: 194 $
 * $Log$
 * Revision 1.11  2000/08/27 10:31:34  hans
 * Added LZS and LZ5 support.
 *
 * Revision 1.10  2000/08/05 16:03:37  hans
 * Removed warnings.
 *
 * Revision 1.9  1998/12/27 20:43:14  klarenw
 * Updated makefiles. Changes for new error codes. Changes for LH6 mode.
 * settings.h is no longer used.
 *
 * Revision 1.8  1998/05/26 19:47:24  hwessels
 * Sources van Hans, 26-05-98.
 *
 * Revision 1.7  1998/03/21 14:26:32  hwessels
 * Sources van Hans, 21-03-1998.
 *
 * Revision 1.6  1998/01/03 22:45:55  hwessels
 * Sources van Hans, 03-01-98.
 *
 * Revision 1.5  1997/10/25 14:12:50  hwessels
 * *** empty log message ***
 *
 * Revision 1.4  1997/10/25 12:53:14  hwessels
 * Sources van Hans 25-10-97.
 *
 * Revision 1.1  1996/07/26 18:16:56  wklaren
 * Depacker.
 *
 ********************************************************************/

#include <string.h>
#include <stdio.h>

#include "gup.h"
#include "compress.h"
#include "gup_err.h"
#include "crc.h"
#include "decode.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*-
  ARJ decode routines
  (c) 1995 Mr Ni! (the Great of the TOS-crew)
*/

/*-
  het buff blok zit alsvolgt in elkaar:
      64k: voor cr/lf conversie
      64k: cr/lf conversie en maximum terug copy
    buffstart:
      64k: depack blok
    buffend:
      256 bytes maximum overflow

    totale buffsize is dus 3*64k+256 bytes, dus ... ja!

  een karakter wordt alsvolgt gecodeerd:
  signed short int kar= :
    -256 .. -1 literal, neem de lowerbyte van kar als literal waarde
               dus -256==0 -255==1 ... -1==255
    0,1,2      geen geldige waarde, mag niet voorkomen, komt het toch voor
               dan wordt 0 als een literal beschouwd, 1 en twee als length
               waarden.
    3 .. 258   length waarden, gzip compatible
  de clen tabel zit dus als volgt in elkaar:
    256 bytes: karakter lengte
  karlen:
    259 bytes: length lengte

  de pointerlengte tabel zit op een normale manier in elkaar
*/

#if 0
  /* log literal en pointer len combi's */
	static unsigned long log_pos_counter=0;
	#define LOG_LITERAL(lit)  {printf("%lX Literal: %02X\n", log_pos_counter, lit); log_pos_counter++;}
	#define LOG_PTR_LEN(len, ptr) {printf("%lX Len: %u, ptr: %u\n", log_pos_counter ,len, ptr); log_pos_counter+=len;}
	#define LOG_BIT(bit) /* printf("bit = %i\n",bit); */
  	#define LOG_RUN(run) printf("Run = %lu\n", run);
	#define LOG_COUNTER_RESET log_pos_counter=0;
	#define LOG_TEXT(string) printf(string);
#else
  #define LOG_LITERAL(lit) /* */
  #define LOG_RUN(len) /* */
  #define LOG_PTR_LEN(len, ptr) /* */
  #define LOG_BIT(bit) /* */
  #define LOG_COUNTER_RESET /* */
	#define LOG_TEXT(string) /* */
#endif

#define BITBUFSIZE    (sizeof(unsigned long) * 8)   /* aantal bits in bitbuffer */

gup_result decode_big(decode_struct *com);
gup_result decode_m4(decode_struct *com);
gup_result decode_m0(decode_struct *com);
gup_result decode_n0(decode_struct *com);
gup_result decode_n1(decode_struct *com);
gup_result read_data(decode_struct *com);

/*
 * ALIGN is a macro to align elements of e certain size in a molloced
 * array. The start of the array is guaranteed alignd with every possible 
 * object. So an object is aligned when it's position relative to the start
 * of the array is a multiple of it's size. Instead of the modulo operator 
 * the & operator is used. This assumes that the alignment of an object 
 * is the samlest power of to of its size; ie: sizeof(object)==3 -> alignment
 * is 1. sizeof(object)==6 ->alignment is 2.
 */
#define ALIGN(start,ptr,size) ptr+=((size)-((ptr)-(start)))&((size)-1)

#if 0
  /* 
    Trashbits function for Atari ST pure C,
    due to a terrible and unbeliveble compiler error
    the trashbits macro doesn't work in some cases in
    the mode 4 depacker...
  */
#define TRASHBITS(x) bitbuf=tb(x, &bib, bitbuf, com);

unsigned long int tb(int xbits, int* bib, unsigned long int bitbuf, decode_struct *com)
{
  *bib-=xbits;
  if(*bib<0)
  { /* refill bitbuffer */
    int i;
    unsigned long int newbuf=0; /* BITBUFSIZE bits groot */
    bitbuf<<=(xbits+*bib);        /* gooi bits er uit */
    xbits=-*bib;
    i=(int)sizeof(bitbuf)-2;
    while(--i>=0)
    {
      if(com->rbuf_current>=com->rbuf_tail)
      {
        gup_result res;
        if((res=read_data(com))!=GUP_OK)
        {
          return res;
        }
        if(com->rbuf_current>=com->rbuf_tail)
	{ /* We have some trouble here */
          *bib = INT_MAX;
          newbuf <<= 8*(i+1);
          break;
	}
      }
      newbuf <<= 8;
      newbuf+=*com->rbuf_current++;
      *bib += 8;
    }
    bitbuf+=newbuf;
  }
  bitbuf<<=xbits;
  return bitbuf;
}

#else
#define TRASHBITS(x)    /* trash  bits from bitbuffer */    \
{                                                           \
  int xbits=(x);                                            \
  bib -= xbits;                                             \
  if (bib < 0)                                              \
  { /* refill bitbuffer */                                  \
    int i;                                                  \
    unsigned long int newbuf = 0; /* BITBUFSIZE bits groot */ \
                                                            \
    bitbuf <<= (xbits + bib);     /* gooi bits er uit */    \
    xbits =- bib;                                           \
    i = (int)sizeof(bitbuf) - 2;                            \
    while(--i >= 0)                                         \
    {                                                       \
      if(com->rbuf_current>=com->rbuf_tail)                 \
      {                                                     \
        gup_result res;                                     \
        if((res=read_data(com))!=GUP_OK)                    \
        {                                                   \
          return res;                                       \
        }                                                   \
        if(com->rbuf_current>=com->rbuf_tail)               \
      	{ /* We have some trouble */                        \
          bib = INT_MAX;                                    \
          newbuf <<= 8*(i+1);                               \
          break;                                            \
	      }                                                   \
      }                                                     \
      newbuf <<= 8;                                         \
      newbuf+=*com->rbuf_current++;                         \
      bib += 8;                                             \
    }                                                       \
    bitbuf += newbuf;                                       \
  }                                                         \
  bitbuf <<= xbits;                                         \
}
#endif


#ifndef NOT_USE_STD_read_data

gup_result read_data(decode_struct *com)
{
  gup_result res;
  com->br_buf->current=com->rbuf_current;
  res=com->buf_fill(com->br_buf, com->br_propagator);
  com->rbuf_current=com->br_buf->current;
  com->rbuf_tail=com->br_buf->end;
  return res;
}

#endif

#ifndef NOT_USE_STD_init_decode

gup_result init_decode(decode_struct *com)
{
  uint8 *buf;
  uint8 *start;
  unsigned long memneed=0;
  
  memneed+=(3*65536UL+MAXMATCH)*sizeof(uint8);  /* buffer */
  ALIGN(0, memneed, sizeof(uint16));
  memneed+=65536UL * sizeof(uint16);            /* huff2kar */
  ALIGN(0, memneed, sizeof(uint8));
  memneed+=(CHARS+MAXMATCH+1)*sizeof(uint8);    /* karlen1 */
  ALIGN(0, memneed, sizeof(uint8));
  memneed+=65536UL * sizeof(uint8);            /* huff2ptr */
  ALIGN(0, memneed, sizeof(uint8));
  memneed+=19*sizeof(uint8);                    /* ptrlen */
  if ((buf = com->gmalloc(memneed, com->gm_propagator))== NULL)
  {
    return GUP_NOMEM;
  }
  start=buf;
  com->buffer = buf;
  com->buffstart = buf + 2 * 65536UL;           /* buffer */
  buf += 3 * 65536UL + MAXMATCH;
  ALIGN(start, buf, sizeof(kartype));
  com->huff2kar = (void*)buf;                   /* huf2kar */
  buf += 65536UL * sizeof(kartype);
  ALIGN(start, buf, sizeof(uint8));
  com->karlen = buf + CHARS;                    /* karlen1 */
  buf += CHARS + MAXMATCH + 1;
  ALIGN(start, buf, sizeof(uint8));
  com->huff2ptr = (void*)buf;                   /* huf2ptr */
  buf += 65536UL * sizeof(uint8);
  ALIGN(start, buf, sizeof(uint8));
  com->ptrlen = buf;                            /* ptrlen */
  buf += 19*sizeof(uint8);
  NEVER_USE(buf);
  return GUP_OK;
}

#endif

#ifndef NOT_USE_STD_free_decode

void free_decode(decode_struct *com)
{
  if((com->buffer)!=NULL)
  {
    com->gfree(com->buffer, com->gf_propagator);
  }
}

#endif

#ifndef NOT_USE_STD_decode

gup_result decode(decode_struct *com)
{
  gup_result ret;
  com->rbuf_current=com->br_buf->current;
  com->rbuf_tail=com->br_buf->end;
  switch(com->mode)
  {
  case STORE:
    ret=decode_m0(com);
    break;
  case ARJ_MODE_1:
  case ARJ_MODE_2:
  case ARJ_MODE_3:
  case GNU_ARJ_MODE_7:
  case LHA_LH6_:
  case LHA_LH7_:
    com->n_ptr=ARJ_NPT;
    com->m_ptr_bit=ARJ_PBIT;
    ret=decode_big(com);
    break;
  case ARJ_MODE_4:
    ret=decode_m4(com);
    break;
  case LHA_LH4_:
  case LHA_LH5_:
    com->n_ptr=LHA_NPT;
    com->m_ptr_bit=LHA_PBIT;
    ret=decode_big(com);
    break;
  case NI_MODE_0:
    ret=decode_n0(com);
    break;
  case NI_MODE_1:
    ret=decode_n1(com);
    break;
  default:
    ret=GUP_HDR_UNKNOWN_METHOD;
  }
  com->br_buf->current=com->rbuf_current;
  return ret;
}

#endif

#ifndef NOT_USE_STD_decode_big

gup_result decode_big(decode_struct *com)
{
  /* aanname origsize > 0 */
  int karshlvl;             /* grootte van de karakter shift */
  int ptrshlvl;             /* groote van de pointer shift */
  long origsize = com->origsize; /* original size */
  int bib;                  /* bits in bitbuf */
  unsigned long int bitbuf; /* shift buffer, BITBUFSIZE bits groot */
  uint16 huffcount=0;         /* grootte huffmanblok */
  uint8* buff=com->buffstart;
  uint8* buffend;

  if(origsize>(65536L+MAXMATCH))
  {
    buffend=com->buffstart+65536L;
    origsize-=65536L;
  }
  else
  {
    buffend=com->buffstart+origsize;
    origsize=0;
  }
  bitbuf=0;
  bib=0;
  karshlvl=0;
  ptrshlvl=0;
  { /* init bitbuf */
    int i=(int)sizeof(bitbuf);
    while(--i>=0)
    {
      bitbuf<<=8;
      bitbuf+=*com->rbuf_current++;
      if(com->rbuf_current>com->rbuf_tail)
      {
        gup_result res;
        if((res=read_data(com))!=GUP_OK)
        {
          return res;
        }
      }
      bib+=8;
    }
    bib-=16;
  }

  for(;;)
  { /* decode loop */
    while(huffcount--!=0) /* moet eigenlijk while(--huffcount>=0) zijn dbra compatible dus */
    {
      kartype kar;
      if((kar=com->huff2kar[bitbuf>>karshlvl])>0)
      { /* pointer length combinatie */
        uint16 ptr;
        
        TRASHBITS(com->karlen[kar]);
        ptr=com->huff2ptr[bitbuf>>ptrshlvl];
        TRASHBITS(com->ptrlen[ptr]);
        if(ptr>1)
        {
          int tmp;
          ptr--;
          tmp=ptr;
          ptr=(1<<ptr) + (pointer_type)(bitbuf>>(BITBUFSIZE-ptr));
          TRASHBITS(tmp);
        }
        {
          uint8* q=buff-ptr-1;
          do
          {
              *buff++=*q++;
          } 
          while(--kar>0);
        }
      }
      else
      {
        *buff++=(uint8)kar;
        TRASHBITS(com->karlen[kar]);
      }
      if(buff>=buffend)
      {
        if(origsize==0)
        {
          unsigned long len;
          if((len=(buff-com->buffstart))!=0)
          {
            gup_result err;
            com->print_progres(len, com->pp_propagator);
            if ((err = com->write_crc(len, com->buffstart, com->wc_propagator))!=GUP_OK)
            {
              return err;
            }
          }
          return GUP_OK; /* exit succes */
        }
        else
        {
          {
            gup_result err;
            com->print_progres(65536UL, com->pp_propagator);
            if ((err = com->write_crc(65536UL, com->buffstart, com->wc_propagator))!=GUP_OK)
            {
              return err;
            }
          }
          buff-=65536UL;
          if(com->mode<4)
          { /* copy 26k blok terug plus 256 bytes overflow */
            memmove(com->buffstart-26UL*1024UL, buffend-26UL*1024UL, 26UL*1024UL+MAXMATCH);
          }
          else
          { /* mode == 7 */
            /* copy 64k blok terug met 256 bytes overflow, dit moet eigenlijk een memmove zijn */
            memmove(com->buffstart-64UL*1024UL, com->buffstart, 64UL*1024UL+MAXMATCH);
          }
          if(origsize>(65536L+MAXMATCH))
          {
            origsize-=65536UL;
          }
          else
          {
            buffend=com->buffstart+origsize;
            origsize=0;
          }
        }
      }
    }
    { /* read new huffman codes */
      huffcount=(uint16)(bitbuf>>(BITBUFSIZE-16));
      if(huffcount==0)
      { /* stream end code */
        /* eventueel kan hier de depacked size worden berekend */
        unsigned long len;
        if((len=(buff-com->buffstart))!=0)
        {
          gup_result err;
          com->print_progres(len, com->pp_propagator);
          if ((err = com->write_crc(len, com->buffstart, com->wc_propagator))!=GUP_OK)
          {
            return err;
          }
        }
        return GUP_OK; /* exit succes */
      }
      TRASHBITS(16);
      { /* lees huffman codes voor de karakter lengtes */
        int count=(uint16)(bitbuf>>(BITBUFSIZE-5));
        TRASHBITS(5);
        if(count)
        { /* lees count huffmancodes voor de lengte in */
          uint8 *p=com->ptrlen;
          int i=3;
          while(--i>=0)
          { /* get lengte 0, 2-18 maal 0 en 20+ maal 0 */
            uint8 tmp=(uint8)(bitbuf>>(BITBUFSIZE-3));
            TRASHBITS(3);
            if(tmp==7)
            {
              while(((signed long) bitbuf) <0)
              {
                tmp++;
                TRASHBITS(1); /* ruimte voor optimalisatie: bitbuf+=bitbuf... */
              }
              TRASHBITS(1);
            }
            *p++=tmp;
          }
          /* nu is er de mogelijk heid om karakter lengte 1, 2 en 3 over te slaan */
          i=(int)(-((uint16)(bitbuf>>(BITBUFSIZE-2))));
          *p=0;
          p[1]=0;
          p[2]=0;
          p-=i; /* sla i posities over */
          i+=count-3; /* 3 wegens bovenstaande loop */
          TRASHBITS(2);
          while(--i>=0)
          {
            uint8 tmp=(uint8)(bitbuf>>(BITBUFSIZE-3));
            TRASHBITS(3);
            if(tmp==7)
            {
              while(((signed long) bitbuf) <0)
              {
                tmp++;
                TRASHBITS(1); /* ruimte voor optimalisatie: bitbuf+=bitbuf... */
              }
              TRASHBITS(1);
            }
            *p++=tmp;
          }
          { /* lengtes zijn ingelezen, maak huff2ptr tabel */
            unsigned long clenct[17]; /* er zijn maximaal 17 verschillende karakter lengtes 0 t/m 16 */
            int maxlen;
            unsigned long *q=clenct;
            i=17;
            while(--i>=0)
            {
              *q++=0;
            }
            i=count;
            p=com->ptrlen;
            while(--i>=0)
            {
              clenct[*p++]++;
            }
            q=clenct+17;
            /* zoek maxlen */
            while(*--q==0)
            {
              ;
            }
            maxlen=(int)(q-clenct);
            ptrshlvl=(int)(BITBUFSIZE-maxlen);
            /* bereken start adressen voor verschillende lengtes */
            {
              unsigned long start=0;
              int j=maxlen;
              q=clenct;
              *q++=0; /* we moeten 0 overslaan */
              while(--j>=0)
              {
                unsigned long tmp=*q;
                *q++=start;
                start+=tmp<<j;
              }
              if(start != (1UL<<maxlen))
              {
                return GUP_BAD_HUFF_TABLE1;
              }
            }
            p=com->ptrlen;
            for(i=0;i<count;i++)
            { /* bouw huff2ptr */
              unsigned long j;
              int tmp;
              if((tmp=*p++)!=0)
              {
                j=1<<(maxlen-tmp);
                memset(com->huff2ptr+clenct[tmp], i, j);
                clenct[tmp]+=j;
              }
            }
          }
        }
        else
        { /* er is maar 1 karakter lengte */
          uint8 ptr=(uint8)(bitbuf>>(BITBUFSIZE-5));
          TRASHBITS(5);
          ptrshlvl=(int)(BITBUFSIZE-1); /* we mogen niet over BITBUFSIZE schuiven */
          *com->huff2ptr=ptr;  /* zet dus de pointer voor nul en een */
          com->huff2ptr[1]=ptr;
          com->ptrlen[ptr]=0;
        }
      }
      {
        /*
          karakter lengtes zijn ingelezen, huffman table is gemaakt
          haal nu de karakters op
        */
        int count=(uint16)(bitbuf>>(BITBUFSIZE-9));
        TRASHBITS(9);
        if(count!=0)
        {
          int i=count;
          uint8 * p=com->karlen-CHARS;
          while(--i>=0)
          {
            unsigned int tmp=com->huff2ptr[bitbuf>>ptrshlvl];
            TRASHBITS(com->ptrlen[tmp])
            if(tmp>2) /* echte len */
            {
              *p++=(uint8)(tmp-2);
            }
            else
            {
              if(tmp)
              {
                if(tmp==1)
                {
                  tmp=3+(uint16)(bitbuf>>(BITBUFSIZE-4));
                  TRASHBITS(4);
                }
                else
                {
                  tmp=20+(uint16)(bitbuf>>(BITBUFSIZE-9));
                  TRASHBITS(9);
                }
                memset(p, 0, tmp);
                p+=tmp;
                i-=tmp-1;
              }
              else
              {
                *p++=0;
              }
            }
          }
          memset(p, 0, CHARS+MAXMATCH+1-count); /* clear rest karlen */
          memmove(com->karlen+3, com->karlen, 256); /* fix voor speciale tabel, 256 wegens gnuzip */
          *com->karlen=0;    /* anders foute huffmantabel */
          com->karlen[1]=0;
          com->karlen[2]=0;
          count+=3;
          { /* bouw nu huff2kar */
            unsigned long clenct[17]; /* er zijn maximaal 17 verschillende karakter lengtes 0 t/m 16 */
            int maxlen;
            unsigned long *q=clenct;
            memset(q, 0, 17*sizeof(unsigned long));
            i=count;
            p=com->karlen-CHARS;
            while(--i>=0)
            {
              clenct[*p++]++;
            }
            q=clenct+17;
            /* zoek maxlen */
            while(*--q==0)
            {
              ;
            }
            maxlen=(int)(q-clenct);
            karshlvl=(int)(BITBUFSIZE-maxlen);
            /* bereken start adressen voor verschillende lengtes */
            {
              unsigned long start=0;
              int j=maxlen;
              q=clenct;
              *q++=0; /* 0 weer overslaan */
              while(--j>=0)
              {
                unsigned long tmp=*q;
                *q++=start;
                start+=tmp<<j;
              }
              if(start != (1UL<<maxlen))
              {
                return GUP_BAD_HUFF_TABLE2;
              }
            }
            p=com->karlen-CHARS;
            count-=CHARS;
            {
              kartype i;
              for(i=-CHARS;i<count;i++)
              { /* bouw huff2ptr */
                long j;
                int tmp;
                if((tmp=*p++)!=0)
                {
                  kartype *q=com->huff2kar+clenct[tmp];
                  j=1<<(maxlen-tmp);
                  clenct[tmp]+=j;
                  while(--j>=0)
                  {
                    *q++=i;
                  }
                }
              }
            }
          }
        }
        else
        { /* er is maar een karakter */
          kartype kar=(kartype)(bitbuf>>(BITBUFSIZE-9));
          TRASHBITS(9);
          kar-=CHARS;
          if(kar>=0)
          { /* het karakter is een pointer-len */
            kar+=3;
          }
          *com->huff2kar=kar;
          com->huff2kar[1]=kar;
          karshlvl=(int)(BITBUFSIZE-1); /* we kunnen niet over BITBUFSIZE schuiven */
          com->karlen[kar]=0;
        }
      }
      { /* karakters zijn opgehaald nu nog de pointers */
        int count=(uint16)(bitbuf>>(BITBUFSIZE-com->m_ptr_bit));
        TRASHBITS(com->m_ptr_bit);
        if(count)
        { /* lees count huffmanodes voor de pointers in */
          uint8 *p=com->ptrlen;
          int i=count;
          while(--i>=0)
          {
            uint8 tmp=(uint8)(bitbuf>>(BITBUFSIZE-3));
            TRASHBITS(3);
            if(tmp==7)
            {
              while(((signed long) bitbuf) <0)
              {
                tmp++;
                TRASHBITS(1); /* ruimte voor optimalisatie: bitbuf+=bitbuf... */
              }
              TRASHBITS(1);
            }
            *p++=tmp;
          }
          { /* lengtes zijn ingelezen, maak huff2ptr tabel */
            unsigned long clenct[17]; /* er zijn maximaal 17 verschillende karakter lengtes 0 t/m 16 */
            int maxlen;
            unsigned long *q=clenct;
            i=17;
            while(--i>=0)
            {
              *q++=0;
            }
            i=count;
            p=com->ptrlen;
            while(--i>=0)
            {
              clenct[*p++]++;
            }
            q=clenct+17;
            /* zoek maxlen */
            while(*--q==0)
            {
              ;
            }
            maxlen=(int)(q-clenct);
            ptrshlvl=(int)(BITBUFSIZE-maxlen);
            /* bereken start adressen voor verschillende lengtes */
            {
              unsigned long start=0;
              int j=maxlen;
              q=clenct;
              *q++=0; /* wis 0 */
              while(--j>=0)
              {
                unsigned long tmp=*q;
                *q++=start;
                start+=tmp<<j;
              }
              if(start != (1UL<<maxlen))
              {
                return GUP_BAD_HUFF_TABLE3;
              }
            }
            p=com->ptrlen;
            for(i=0;i<count;i++)
            { /* bouw huff2ptr */
              unsigned long j;
              int tmp;
              if((tmp=*p++)!=0)
              {
                j=1<<(maxlen-tmp);
                memset(com->huff2ptr+clenct[tmp], i, j);
                clenct[tmp]+=j;
              }
            }
          }
        }
        else
        { /* er is maar 1 pointer */
          uint8 ptr=(uint8)(bitbuf>>(BITBUFSIZE-com->m_ptr_bit));
          TRASHBITS(com->m_ptr_bit);
          ptrshlvl=(int)(BITBUFSIZE-1); /* we mogen niet over BITBUFSIZE schuiven */
          *com->huff2ptr=ptr;  /* zet dus de pointer voor nul en een */
          com->huff2ptr[1]=ptr;
          com->ptrlen[ptr]=0;
        }
      }
    }
  }
}

#endif

#ifndef NOT_USE_STD_decode_m0

gup_result decode_m0(decode_struct *com)
{ /* Extract a file, which is stored with mode 0, from the archive */
  long origsize = com->origsize; /* original size */
  while(origsize!=0)
  {
    gup_result err;
    long bufsize=com->br_buf->end-com->br_buf->current;
    if(bufsize>origsize)
    {
      bufsize=origsize;
    }
    com->print_progres(bufsize, com->pp_propagator);
    if ((err = com->write_crc(bufsize, com->br_buf->current, com->wc_propagator))!=GUP_OK)
    {
      return err;
    }
    com->br_buf->current+=bufsize;
    if((origsize-=bufsize)>0)
    {
      err=com->buf_fill(com->br_buf, com->br_propagator);
      if(err!=GUP_OK)
      {
        return err;
      }
    }
  }
  return GUP_OK;
}

#endif

/*-
 * ARJ mode 4 packing
 *
 * (c) 1993 Mr Ni! (the Great) of the TOS-crew
 *
 * codeer schema: [len (literal 8 bits | ptr_code)]
 *
 * len codering: w bits:
 *
 * 0 0               :  literal
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

#ifndef NOT_USE_STD_decode_m4

gup_result decode_m4(decode_struct *com)
{
  /* aanname origsize>0 */

  int bib;                  /* bits in bitbuf */
  unsigned long int bitbuf; /* shift buffer, BITBUFSIZE bits groot */
  uint8* buff=com->buffstart;
  uint8* buffend;
  
  if(com->origsize>(65536L+MAXMATCH))
  {
    buffend=com->buffstart+65536L;
    com->origsize-=65536L;
  }
  else
  {
    buffend=com->buffstart+com->origsize;
    com->origsize=0;
  }
  bitbuf=0;
  bib=0;
  { /* init bitbuf */
    int i=(int)sizeof(bitbuf);
    while(--i>=0)
    {
      bitbuf<<=8;
      bitbuf+=*com->rbuf_current++;
      if(com->rbuf_current>com->rbuf_tail)
      {
        gup_result res;
        if((res=read_data(com))!=GUP_OK)
        {
          return res;
        }
      }
      bib+=8;
    }
    bib-=16;
  }

  for(;;)
  { /* decode loop */
    kartype kar;
    if((kar=(kartype)(bitbuf>>(BITBUFSIZE-9)))>255)
    { /* pointer length combinatie */
      uint16 ptr;
      
      if(kar<480)
      {
        if(kar<384)
        {
          TRASHBITS(2);
          kar=3+(kartype)(bitbuf>>(BITBUFSIZE-1));
          TRASHBITS(1);
        }
        else
        {
          if(kar<448)
          {
            TRASHBITS(3);
            kar=5+(kartype)(bitbuf>>(BITBUFSIZE-2));
            TRASHBITS(2);
          }
          else
          {
            TRASHBITS(4);
            kar=9+(kartype)(bitbuf>>(BITBUFSIZE-3));
            TRASHBITS(3);
          }
        }
      }
      else
      {
        if(kar<504)
        {
          if(kar<496)
          {
            TRASHBITS(5);
            kar=17+(kartype)(bitbuf>>(BITBUFSIZE-4));
            TRASHBITS(4);
          }
          else
          {
            TRASHBITS(6);
            kar=33+(kartype)(bitbuf>>(BITBUFSIZE-5));
            TRASHBITS(5);
          }
        }
        else
        {
          if(kar<508)
          {
            TRASHBITS(7);
            kar=65+(kartype)(bitbuf>>(BITBUFSIZE-6));
            TRASHBITS(6);
          }
          else
          {
            TRASHBITS(7);
            kar=129+(kartype)(bitbuf>>(BITBUFSIZE-7));
            TRASHBITS(7);
          }
        }
      }
      if((ptr=(uint16)(bitbuf>>(BITBUFSIZE-4)))<12)
      {
        if(ptr<8)
        {
          ptr=(uint16)(bitbuf>>(BITBUFSIZE-10));
          TRASHBITS(10);
        }
        else
        {
          TRASHBITS(2);
          ptr=512+(uint16)(bitbuf>>(BITBUFSIZE-10));
          TRASHBITS(10);
        }
      }
      else
      {
        if(ptr<14)
        {
          TRASHBITS(3);
          ptr=1536+(uint16)(bitbuf>>(BITBUFSIZE-11));
          TRASHBITS(11);
        }
        else
        {
          TRASHBITS(4);
          if(ptr<15)
          {
            ptr=3584+(uint16)(bitbuf>>(BITBUFSIZE-12));
            TRASHBITS(12);
          }
          else
          {
            ptr=7680+(uint16)(bitbuf>>(BITBUFSIZE-13));
            TRASHBITS(13);
          }
        }
      }
      {
        uint8* q=buff-ptr-1;
        do
        {
          *buff++=*q++;
        } 
        while(--kar>0);
      }
    }
    else
    {
      *buff++=(uint8)kar;
      TRASHBITS(9);
    }
    if(buff>=buffend)
    {
      if(com->origsize==0)
      {
        unsigned long len;
        if((len=(buff-com->buffstart))!=0)
        {
          gup_result err;
          com->print_progres(len, com->pp_propagator);
          if ((err = com->write_crc(len, com->buffstart, com->wc_propagator))!=GUP_OK)
          {
            return err;
          }
        }
        return GUP_OK; /* exit succes */
      }
      else
      {
        {
          gup_result err;
          com->print_progres(65536UL, com->pp_propagator);
          if ((err = com->write_crc(65536UL, com->buffstart, com->wc_propagator))!=GUP_OK)
          {
            return err;
          }
        }
        buff-=65536UL;
        memmove(com->buffstart-16UL*1024UL, buffend-16UL*1024UL, 16UL*1024UL+MAXMATCH);
        if(com->origsize>(65536L+MAXMATCH))
        {
          com->origsize-=65536UL;
        }
        else
        {
          buffend=com->buffstart+com->origsize;
          com->origsize=0;
        }
      }
    }
  }
}

#endif

#ifndef NOT_USE_STD_decode_n0

#define GET_N0_BIT(bit)								\
{ /* get a bit from the data stream */			\
 	if(bits_in_bitbuf==0)							\
 	{ /* fill bitbuf */								\
  		if(com->rbuf_current>com->rbuf_tail)	\
		{													\
			gup_result res;							\
			if((res=read_data(com))!=GUP_OK)		\
			{												\
				return res;								\
			}												\
		}													\
  		bitbuf=*com->rbuf_current++;				\
  		bits_in_bitbuf=8;								\
	}														\
	bit=(bitbuf&0x80)>>7;							\
	bitbuf+=bitbuf;									\
	bits_in_bitbuf--;									\
}

#define DECODE_N0_LEN(val)							\
{ /* get value 2 - 2^32-1 */						\
	int bit;												\
	val=1;												\
	do														\
	{														\
		GET_N0_BIT(bit);								\
		val+=val+bit;									\
		GET_N0_BIT(bit);								\
	} while(bit!=0);									\
}


gup_result decode_n0(decode_struct *com)
{
	uint8* dst=com->buffstart;
	uint8* dstend;
	uint8 bitbuf=0;
	int bits_in_bitbuf=0;
	dstend=com->buffstart+65536L;
	if(com->origsize==0)
	{
		return GUP_OK; /* exit succes? */
	}
	{ /* start met een literal */
		com->origsize--;
  		if(com->rbuf_current > com->rbuf_tail)
		{
			gup_result res;
			if((res=read_data(com))!=GUP_OK)
			{
				return res;
			}
		}
		LOG_LITERAL(*com->rbuf_current);
		*dst++=*com->rbuf_current++;
		if(dst>=dstend)
		{
			gup_result err;
			long bytes=dst-com->buffstart;
			com->print_progres(bytes, com->pp_propagator);
			if ((err = com->write_crc(bytes, com->buffstart, com->wc_propagator))!=GUP_OK)
			{
				return err;
			}
			dst-=bytes;
			memmove(com->buffstart-bytes, com->buffstart, bytes);
		}
	}
	for(;;)
	{
		int bit;
		GET_N0_BIT(bit);
		if(bit==0)
		{ /* literal */
	  		if(com->rbuf_current > com->rbuf_tail)
			{
				gup_result res;
				if((res=read_data(com))!=GUP_OK)
				{
					return res;
				}
			}
			LOG_LITERAL(*com->rbuf_current);
			*dst++=*com->rbuf_current++;
			if(dst>=dstend)
			{
				gup_result err;
				long bytes=dst-com->buffstart;
				com->print_progres(bytes, com->pp_propagator);
				if ((err = com->write_crc(bytes, com->buffstart, com->wc_propagator))!=GUP_OK)
				{
					return err;
				}
				dst-=bytes;
				memmove(com->buffstart-bytes, com->buffstart, bytes);
			}
		}
		else
		{ /* ptr len */
			int32 ptr;
			uint8* src;
			uint8 data;
			int len;
			ptr=-1;
			ptr<<=8;
	  		if(com->rbuf_current > com->rbuf_tail)
			{
				gup_result res;
				if((res=read_data(com))!=GUP_OK)
				{
					return res;
				}
			}
			data=*com->rbuf_current++;
			GET_N0_BIT(bit);
			if(bit==0)
			{
				ptr|=data;
			}
			else
			{ /* 16 bit pointer */
				if(data==0)
				{
					break; /* end of stream */
				}
				ptr|=~data;
				ptr<<=8;
		  		if(com->rbuf_current > com->rbuf_tail)
				{
					gup_result res;
					if((res=read_data(com))!=GUP_OK)
					{
						return res;
					}
				}
				ptr|=*com->rbuf_current++;
			}
			DECODE_N0_LEN(len);
			len++;
			LOG_PTR_LEN(len, -ptr)
			src=dst+ptr;
			do
			{
  				*dst++=*src++;
  				if(dst>=dstend)
  				{
					gup_result err;
					long bytes=dst-com->buffstart;
					com->print_progres(bytes, com->pp_propagator);
  					if ((err = com->write_crc(bytes, com->buffstart, com->wc_propagator))!=GUP_OK)
  					{
  						return err;
  					}
					dst-=bytes;
					src-=bytes;
					memmove(com->buffstart-bytes, com->buffstart, bytes);
				}
			} while(--len!=0);
		}
	}
	{
		unsigned long len;
		if((len=(dst-com->buffstart))!=0)
		{
			gup_result err;
			com->print_progres(len, com->pp_propagator);
			if ((err = com->write_crc(len, com->buffstart, com->wc_propagator))!=GUP_OK)
			{
				return err;
			}
		}
	}
	return GUP_OK; /* exit succes */
}

#endif 

#ifndef NOT_USE_STD_decode_n1

#define GET_N1_BIT(bit)								\
{ /* get a bit from the data stream */			\
 	if(bits_in_bitbuf==0)							\
 	{ /* fill bitbuf */								\
  		if(com->rbuf_current>com->rbuf_tail)	\
		{													\
			gup_result res;							\
			if((res=read_data(com))!=GUP_OK)		\
			{												\
				return res;								\
			}												\
		}													\
  		bitbuf=*com->rbuf_current++;				\
  		bits_in_bitbuf=8;								\
	}														\
	bit=(bitbuf&0x80)>>7;							\
	bitbuf+=bitbuf;									\
	bits_in_bitbuf--;									\
	LOG_BIT(bit);										\
}

#define DECODE_N1_PTR(ptr)							\
{ /* get value -1 - -65536 */						\
	int len=0;											\
	int bit;												\
	int i=4;												\
	do														\
	{														\
		GET_N1_BIT(bit);								\
		len+=len+bit;									\
	} while(--i!=0);									\
	ptr=-1;												\
	if(len>0)											\
	{														\
		ptr+=ptr;										\
		len--;											\
	}														\
	do														\
	{														\
		GET_N1_BIT(bit);								\
		ptr+=ptr+bit;									\
	} while(len-->0);									\
}															\

#define DECODE_N1_LEN(val)							\
{ /* get value 2 - 2^32-1 */						\
	int bit;												\
	val=1;												\
	do														\
	{														\
		GET_N1_BIT(bit);								\
		val+=val+bit;									\
		GET_N1_BIT(bit);								\
	} while(bit!=0);									\
}

#define DECODE_N1_COUNTER(val)					\
{ /* get value 0 - 2^32-1 */						\
	int bit;												\
	int i;												\
	int len=0;											\
	for(i=0; i<5; i++)								\
	{														\
		GET_N1_BIT(bit);								\
		len+=len+bit;									\
	}														\
	if(len==0)											\
	{														\
		GET_N1_BIT(bit);								\
		val=bit;											\
	}														\
	else 													\
	{														\
		val=1;											\
		do													\
		{													\
			GET_N1_BIT(bit);							\
			val+=val+bit;								\
		} while(--len>0);								\
	}														\
}

#define DECODE_N1_RUN(val)							\
{ /* get value 0 - 2^32-1 */						\
	int bit;												\
	GET_N1_BIT(bit);									\
	if(bit==0)											\
	{														\
		val=0;											\
	}														\
	else													\
	{														\
		val=1;											\
		GET_N1_BIT(bit);								\
		if(bit==1)										\
		{													\
			DECODE_N1_LEN(val);						\
		}													\
	}														\
}
		

gup_result decode_n1(decode_struct *com)
{
	uint8* dst=com->buffstart;
	uint8* dstend;
	uint8 bitbuf=0;
	int bits_in_bitbuf=0;
	unsigned long loop;
	long origsize=com->origsize;
	int ptrs=0;
	unsigned long run;
	dstend=com->buffstart+65536L;
	if(origsize==0)
	{
		return GUP_OK; /* exit succes? */
	}
	DECODE_N1_COUNTER(loop);
	LOG_RUN(loop);
	do
	{
		if(origsize<=0)
		{ /* zou niet moeten gebeuren */
			LOG_TEXT("Exit on origsize\n");
			return GUP_OK; /* exit succes? */
		}
		DECODE_N1_RUN(run);
		LOG_RUN(run);
		if(ptrs==0)
		{ /* literal run */
			do
			{
		  		if(com->rbuf_current > com->rbuf_tail)
				{
					gup_result res;
					if((res=read_data(com))!=GUP_OK)
					{
						return res;
					}
				}
				LOG_LITERAL(*com->rbuf_current);
				*dst++=*com->rbuf_current++;
				origsize--;
				if(dst>=dstend)
				{
					gup_result err;
					long bytes=dst-com->buffstart;
					com->print_progres(bytes, com->pp_propagator);
					if ((err = com->write_crc(bytes, com->buffstart, com->wc_propagator))!=GUP_OK)
					{
						return err;
					}
					dst-=bytes;
					memmove(com->buffstart-bytes, com->buffstart, bytes);
				}
			} while(run-->0);
			ptrs=1;
		}
		else
		{ /* ptr len run */
			do
			{
				int32 ptr;
				uint8* src;
				int len;
				DECODE_N1_PTR(ptr);
				DECODE_N1_LEN(len);
				len++;
				LOG_PTR_LEN(len, -ptr)
				src=dst+ptr;
				origsize-=len;
				do
				{
  					*dst++=*src++;
  					if(dst>=dstend)
	  				{
						gup_result err;
						long bytes=dst-com->buffstart;
						com->print_progres(bytes, com->pp_propagator);
  						if ((err = com->write_crc(bytes, com->buffstart, com->wc_propagator))!=GUP_OK)
  						{
	  						return err;
  						}
						dst-=bytes;
						src-=bytes;
						memmove(com->buffstart-bytes, com->buffstart, bytes);
					}
				} while(--len!=0);
			} while(run-->0);
			ptrs=0;
		}
	} while(loop-->0);
	{
		unsigned long len;
		if((len=(dst-com->buffstart))!=0)
		{
			gup_result err;
			com->print_progres(len, com->pp_propagator);
			if ((err = com->write_crc(len, com->buffstart, com->wc_propagator))!=GUP_OK)
			{
				return err;
			}
		}
	}
	return GUP_OK; /* exit succes */
}

#endif 

/*
** arj m4 decoder
**
*/

#include <stdint.h>

typedef uint8_t uint8;         /* unsigned 8 bit */
typedef uint16_t uint16;    /* unsigned 16 bit */
typedef int16_t kartype;   /* signed 16 bit */
typedef uint32_t uint32;

#define BITBUFSIZE (sizeof(bitbuf)*8)

#define TRASHBITS(x)    /* trash  bits from bitbuffer */    \
{                                                           \
  int xbits=(x);                                            \
  bib -= xbits;                                             \
  if (bib < 0)                                              \
  { /* refill bitbuffer */                                  \
    int i;                                                  \
    uint32 newbuf = 0; /* BITBUFSIZE bits groot */           \
                                                            \
    bitbuf <<= (xbits + bib);     /* gooi bits er uit */    \
    xbits =- bib;                                           \
    i = (int)sizeof(bitbuf) - 2;                            \
    while(--i >= 0)                                         \
    {                                                       \
      newbuf <<= 8;                                         \
      newbuf+=*data++;                                      \
      bib += 8;                                             \
    }                                                       \
    bitbuf += newbuf;                                       \
  }                                                         \
  bitbuf <<= xbits;                                         \
}

void decode_m4(unsigned long size, uint8_t *dst, uint8_t *data)
{
  /* aanname origsize>0 */

  int bib;                  /* bits in bitbuf */
  uint32 bitbuf; /* shift buffer, BITBUFSIZE bits groot */
  uint8* buff=dst;
  uint8* buffend;

  buffend=dst+size;
  bitbuf=0;
  bib=0;
  { /* init bitbuf */
    int i=(int)sizeof(bitbuf);
    while(--i>=0)
    {
      bitbuf<<=8;
      bitbuf+=*data++;
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
    	return;
    }
  }
}

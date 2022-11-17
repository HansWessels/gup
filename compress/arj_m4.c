#include "encode.h"
#include "compress.h"
#include "decode.h"

int m4_len_len(match_t match);
int m4_ptr_len(ptr_t ptr);
void m4_store_len(match_t match, packstruct *com);
void m4_store_ptr(ptr_t ptr, packstruct *com);
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

#define BITBUFSIZE    (sizeof(unsigned long) * 8)   /* aantal bits in bitbuffer */

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


unsigned long m4_cost_ptrlen(match_t match, ptr_t ptr)
{
	unsigned long res=1; /* 1 bit voor aan te geven dat het een ptr len is */
	res+=m4_len_len(match);
	res+=m4_ptr_len(ptr);
	return res;
}

unsigned long m4_cost_lit(match_t kar)
{
	NEVER_USE(kar);
	return 9; /* 1 bit om aan te geven dat het een literal is en 8 bits voor de literal */
}

int m4_len_len(match_t match)
{
	if(match<3)
	{
		return 255;
	}
	if(match>128)
	{
		if(match>256)
		{
			return 255;
		}
		else
		{
			return 14;
		}
	}
	return 1+2*(first_bit_set32(match-1)-1);
}

int m4_ptr_len(ptr_t ptr)
{
	if(ptr<512)
	{
		return 10;
	}
	if(ptr>7679)
	{
		if(ptr<=15871)
		{
			return 17;
		}
		else
		{
			return 256; /*error */
		}
	}
	return 10+2*first_bit_set32(((ptr-512)>>10)+1);
}

void m4_store_len(match_t match, packstruct *com)
{
	int len;
	match_t mask;
	match-=1;
	len=first_bit_set32(match)-1;
	mask=1<<len;
	mask--;
	ST_BITS(mask, len);
	if(len!=7)
	{
		ST_BITS(0, 1);
	}
	ST_BITS(match&mask, len);
}

void m4_store_ptr(ptr_t ptr, packstruct *com)
{
	int len;
	ptr_t mask;
	if(ptr<512)
	{
		len=0;
		mask=0;
	}
	else
	{
		len=first_bit_set32(((ptr-512)>>10)+1);
		mask=1<<len;
		mask--;
		ST_BITS(mask, len);
	}
	if(len!=4)
	{
		ST_BITS(0, 1);
	}
	ST_BITS(ptr-(mask<<9), len+9);
}

gup_result m4_compress(packstruct *com)
{
	/*
	** pointer lengte codering:
	** 9  0xxxxxxxxx            0 -   511  10 bits
	** 10 10xxxxxxxxxx        512 -  1535  12 bits
	** 11 110xxxxxxxxxxx     1536 -  3583  14 bits
	** 12 1110xxxxxxxxxxxx   3584 -  7679  16 bits
	** 13 1111xxxxxxxxxxxxx  7680 - 15871  17 bits
	**
	** len codering:
	** 0 0               : literal
	** 1 10x             :  3 -   4   3 bits
	** 2 110xx           :  5 -   8   5 bits
	** 3 1110xxx         :  9 -  16   7 bits
	** 4 11110xxxx       : 17 -  32   9 bits
	** 5 111110xxxxx     : 33 -  64  11 bits
	** 6 1111110xxxxxx   : 65 - 128  13 bits
	** 7 1111111xxxxxxx  :129 - 256  14 bits
	*/
	index_t current_pos = DICTIONARY_START_OFFSET; /* wijst de te packen byte aan */
	unsigned long bytes_to_do=com->origsize;
	void* rbuf_current_store=com->rbuf_current;
	com->rbuf_current=com->compressed_data;
	com->bits_in_bitbuf=0;
	com->bitbuf=0;
	while(bytes_to_do>0)
	{
		match_t match;
		match=com->match_len[current_pos];
		if(match==0)
		{ /* store literal */
			match=com->dictionary[current_pos];
			ST_BITS(match, 9);
			LOG_LITERAL(match);
			bytes_to_do--;
			current_pos++;
		}
		else
		{
			ptr_t ptr;
			ptr=com->ptr_len[current_pos];
         bytes_to_do-=match;
         current_pos+=match;
	      LOG_PTR_LEN(match, ptr);
			m4_store_len(match, com);
			m4_store_ptr(ptr, com);

		}
	}
	if(com->bits_in_bitbuf>0)
	{ /* flush bitbuf */
		int bytes_extra=(com->bits_in_bitbuf+7)>>3;
		if(bytes_extra>0)
		{
			*com->rbuf_current++ = (uint8) (com->bitbuf >> 24);
			bytes_extra--;
			if(bytes_extra>0)
			{
				*com->rbuf_current++ = (uint8) (com->bitbuf >> 16);
				bytes_extra--;
				if(bytes_extra>0)
				{
					*com->rbuf_current++ = (uint8) (com->bitbuf >> 8);
					bytes_extra--;
					if(bytes_extra>0)
					{
						*com->rbuf_current++ = (uint8) (com->bitbuf);
					}
				}
			}
		}
		com->bits_in_bitbuf=0;
	}
	com->packed_size=com->rbuf_current-com->compressed_data;
	com->bytes_packed=com->origsize;
	com->rbuf_current=rbuf_current_store;
	{
		unsigned long bytes_to_do=com->packed_size;
		uint8 *src=com->compressed_data;
		while(bytes_to_do>0)
		{
			unsigned long bytes_comming=65536;
			gup_result res;
			if(bytes_comming>bytes_to_do)
			{
				bytes_comming=bytes_to_do;
			}
			if((res=announce(bytes_comming, com))!=GUP_OK)
			{
				return res;
			}
			bytes_to_do-=bytes_comming;
			while(bytes_comming-->0)
			{
				*com->rbuf_current++=*src++;
			}	
		}
	}
	return GUP_OK;
}

unsigned long m4_count_bits(unsigned long *packed_bytes,  /* aantal bytes dat gepacked wordt */
          /* nu de variabelen die nodig zijn voor de berekening */
                            packstruct *com, /* commandstruct */
                            uint16 entries, /* aantal characters die moeten worden gepacked */
                            c_codetype * p, /* pointer naar de karakters     */
                            pointer_type * q  /* pointer naar de pointers    */
)
{
	NEVER_USE(com);
	unsigned long bits = 0;
	unsigned long bytes = 0;

	entries++;
	while (--entries != 0)
	{
		match_t kar = *p++;

		if (kar < NLIT)
		{ /*- store literal */
			bytes++;
			bits += 9;
		}
		else
		{
			bytes += kar + MIN_MATCH - NLIT;
			bits+=m4_len_len(kar);
			kar = *q++;
			bits+=m4_ptr_len(kar);
		}
	}
	*packed_bytes = bytes;
	return bits;
}

gup_result m4_close_stream(packstruct *com)
{
	NEVER_USE(com);
	return GUP_OK;
	return GUP_OK;
}

#define TRASHBITS(x)		/* trash  bits from bitbuffer */		\
{																				\
	int xbits=(x);															\
	bib -= xbits;															\
	if(bib < 0)																\
	{ /* refill bitbuffer */											\
		int i;																\
		unsigned long int newbuf = 0; /* BITBUFSIZE bits groot */ \
		bitbuf <<= (xbits + bib); /* gooi bits er uit */		\
		xbits =- bib;														\
		i = (int)sizeof(bitbuf) - 2;									\
		while(--i >= 0)													\
		{																		\
			if(com->rbuf_current>=com->rbuf_tail)					\
			{																	\
				gup_result res;											\
				if((res=read_data(com))!=GUP_OK)						\
				{																\
					return res;												\
				}																\
				if(com->rbuf_current>=com->rbuf_tail)				\
				{ /* We have some trouble */							\
					bib = INT_MAX;											\
					newbuf <<= 8*(i+1);									\
					break;													\
				}																\
			}																	\
			newbuf <<= 8;													\
			newbuf+=*com->rbuf_current++;								\
			bib += 8;														\
		}																		\
		bitbuf += newbuf;													\
	}																			\
	bitbuf <<= xbits;														\
}

/*-
** ARJ mode 4 packing
**
** (c) 1993 Mr Ni! (the Great) of the TOS-crew
**
** codeer schema: [len (literal 8 bits | ptr_code)]
**
** len codering: w bits:
** 0 0               :  literal
** 1 10x             :  2 -   3
** 2 110xx           :  4 -   7
** 3 1110xxx         :  8 -  15
** 4 11110xxxx       : 16 -  31
** 5 111110xxxxx     : 32 -  63
** 6 1111110xxxxxx   : 64 - 127
** 7 1111111xxxxxxx  :128 - 255
**
** Hierbij moet nog een worden opgeteld, dus minimale lengte 3, maximaal
** 256!!!
**
** ptr codering: w bits
** 9  0xxxxxxxxx            0 -   511
** 10 10xxxxxxxxxx        512 -  1535
** 11 110xxxxxxxxxxx     1536 -  3583
** 12 1110xxxxxxxxxxxx   3584 -  7679
** 13 1111xxxxxxxxxxxxx  7680 - 15871
**
** codering dus van 0 - 15871
*/

gup_result m4_decode(decode_struct *com)
{
	/* aanname origsize>0 */
	int bib; /* bits in bitbuf */
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
		unsigned long mask=1UL<<(BITBUFSIZE-1);
		if((bitbuf&mask)==0)
		{ /* literal */
			match_t kar;
			kar=(uint8)(bitbuf>>(BITBUFSIZE-9));
			*buff++=kar;
			LOG_LITERAL(kar);
			TRASHBITS(9);
		}
		else
		{ /* pointer length combinatie */
			int i;
			int tb=7;
			ptr_t ptr;
			match_t kar;
			i=1;
			do
			{
				mask>>=1;
				if((bitbuf&mask)==0)
				{
					tb=i+1;
					break;
				}
				i++;
			} while(i<7);
			TRASHBITS(tb);
			kar=(1<<i)+(bitbuf>>(BITBUFSIZE-i))+1;
			TRASHBITS(i);
			tb=4;
			i=0;
			mask=1UL<<(BITBUFSIZE-1);
			do
			{
				if((bitbuf&mask)==0)
				{
					tb=i+1;
					break;
				}
				mask>>=1;
				i++;
			} while(i<4);
			TRASHBITS(tb);
			ptr=(((1<<i)-1)<<9)+(ptr_t)(bitbuf>>(BITBUFSIZE-(i+9)));
			TRASHBITS(i+9);
			{
				uint8* q=buff-ptr-1;
				LOG_PTR_LEN(kar, ptr);
				do
				{
					*buff++=*q++;
				} 
				while(--kar>0);
			}
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
				gup_result err;
				com->print_progres(65536UL, com->pp_propagator);
				if ((err = com->write_crc(65536UL, com->buffstart, com->wc_propagator))!=GUP_OK)
				{
					return err;
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


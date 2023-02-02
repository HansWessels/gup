#include "encode.h"
#include "compress.h"
#include "decode.h"

#define N1_MAX_PTR 130560              /* maximale pointer offset + 1 */
#define N1_MIN_MATCH 2						/* n1 maximum match */
#define N1_MAX_MATCH 65535					/* n1 maximum match */
#define N1_MAX_HIST 0						/* n1 does not use history pointers */

gup_result n1_compress(packstruct *com);
gup_result n1_close_stream(packstruct *com);
unsigned long n1_cost_lit(match_t kar);
unsigned long n1_cost_ptrlen(match_t match, ptr_t ptr, index_t pos, ptr_t *ptr_hist);

int n1_len_len(match_t match);
int n1_ptr_len(ptr_t ptr);
void n1_store_len(match_t match, packstruct *com);
void n1_store_ptr(ptr_t ptr, packstruct *com);

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


unsigned long n1_cost_ptrlen(match_t match, ptr_t ptr, index_t pos, ptr_t *ptr_hist)
{
	NEVER_USE(pos);
	NEVER_USE(ptr_hist);

	unsigned long res=1; /* 1 bit voor aan te geven dat het een ptr len is */
	res+=n1_len_len(match);
	res+=n1_ptr_len(ptr);
	return res;
}

unsigned long n1_cost_lit(match_t kar)
{
	NEVER_USE(kar);
	return 9; /* 1 bit om aan te geven dat het een literal is en 8 bits voor de literal */
}

int n1_len_len(match_t match)
{
	if((match<N1_MIN_MATCH) || (match>N1_MAX_MATCH))
	{
		return 256; /* error */
	}
	if(match>32767)
	{
		return 29;
	}
	return 2*(first_bit_set32(match)-1);
}

int n1_ptr_len(ptr_t ptr)
{
	if(ptr<512)
	{
		return 10;
	}
	if(ptr>65023)
	{
		if(ptr<N1_MAX_PTR)
		{
			return 23;
		}
		else
		{ /* can happen with length 2 matches */
			return 256; /* error */
		}
	}
	return 10+2*first_bit_set32(((ptr-512)>>10)+1);
}

void n1_store_len(match_t match, packstruct *com)
{
	int len;
	match_t mask;
	len=first_bit_set32(match)-1;
	mask=1<<len;
	mask--;
	ST_BITS(mask, len);
	if(len!=15)
	{
		ST_BITS(0, 1);
	}
	ST_BITS(match&mask, len);
}

void n1_store_ptr(ptr_t ptr, packstruct *com)
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
	if(len!=7)
	{
		ST_BITS(0, 1);
	}
	ST_BITS(ptr-(mask<<9), len+9);
}

gup_result n1_compress(packstruct *com)
{
	/*
	** pointer lengte codering:
	** 9  0xxxxxxxxx            0 -   511  10 bits
	** 10 10xxxxxxxxxx        512 -  1535  12 bits
	** 11 110xxxxxxxxxxx     1536 -  3583  14 bits
	** 12 1110xxxxxxxxxxxx   3584 -  7679  16 bits
	** 13 11110xxxxxxxxxxxxx 7680 - 15871  17 bits
	**
	** len codering:
	** 0 0               : literal
	** 1 10x             :  2 -   3   3 bits
	** 2 110xx           :  4 -   7   5 bits
	** 3 1110xxx         :  8 -  15   7 bits
	** 4 11110xxxx       : 16 -  31   9 bits
	** 5 111110xxxxx     : 32 -  63  11 bits
	** 6 1111110xxxxxx   : 63 - 127  13 bits
	** 7 11111110xxxxxxx :128 - 255  15 bits
	** 8 ....
	*/
	index_t current_pos = DICTIONARY_START_OFFSET; /* wijst de te packen byte aan */
	unsigned long bytes_to_do=com->origsize;
	void* rbuf_current_store=com->rbuf_current;
	com->rbuf_current=com->compressed_data;
	com->bits_in_bitbuf=0;
	com->bitbuf=0;
	{ /* send original size */
		ST_BITS((bytes_to_do>>16)&65535, 16);
		ST_BITS(bytes_to_do&65535, 16);
	}
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
			n1_store_len(match, com);
			n1_store_ptr(ptr, com);

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

gup_result n1_close_stream(packstruct *com)
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
					com->gfree(buffer, com->gf_propagator);		\
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

gup_result n1_decode(decode_struct *com)
{
	/* aanname origsize>0 */
	int bib; /* bits in bitbuf */
	unsigned long int bitbuf; /* shift buffer, BITBUFSIZE bits groot */
	unsigned long origsize;
   uint8* buffer=NULL;
	uint8* buffend;
   uint8* buff;
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
   { /* get origsize */
		origsize=bitbuf>>(BITBUFSIZE-16);
		origsize<<=16;
		TRASHBITS(16);
		origsize+=bitbuf>>(BITBUFSIZE-16);
		TRASHBITS(16);
   }
	buffer=com->gmalloc(origsize, com->gm_propagator);
  	if(buffer == NULL)
	{
		return GUP_NOMEM;
	}
	buffend=buffer+origsize;
	buff=buffer;
	com->origsize=0;

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
			int tb=15;
			ptr_t ptr;
			match_t len;
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
			} while(i<15);
			TRASHBITS(tb);
			len=(1<<i)+(bitbuf>>(BITBUFSIZE-i));
			TRASHBITS(i);
			tb=7;
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
			} while(i<7);
			TRASHBITS(tb);
			ptr=(((1<<i)-1)<<9)+(ptr_t)(bitbuf>>(BITBUFSIZE-(i+9)));
			TRASHBITS(i+9);
			{
				uint8* q=buff-ptr-1;
				LOG_PTR_LEN(len, ptr);
				do
				{
					*buff++=*q++;
				} 
				while(--len>0);
			}
		}
		if(buff>=buffend)
		{
			unsigned long len;
			if((len=(buff-buffer))!=0)
			{
				gup_result err;
				com->print_progres(len, com->pp_propagator);
				if ((err = com->write_crc(len, buffer, com->wc_propagator))!=GUP_OK)
				{
					com->gfree(buffer, com->gf_propagator);
					return err;
				}
			}
			com->gfree(buffer, com->gf_propagator);
			return GUP_OK; /* exit succes */
		}
	}
}

gup_result n1_init(packstruct *com)
{
	gup_result res=GUP_OK;
	com->min_match32=N1_MIN_MATCH;
	com->max_match32=N1_MAX_MATCH;
	com->maxptr32=N1_MAX_PTR;
	com->max_hist=N1_MAX_HIST;
	com->compress=n1_compress;
	com->close_packed_stream=n1_close_stream;
	com->cost_ptrlen=n1_cost_ptrlen;
	com->cost_lit=n1_cost_lit;
	res=init_dictionary32(com);
	com->rbuf_current=com->bw_buf->current;
	com->rbuf_tail=com->bw_buf->end;
	com->mv_bits_left=0;
	if(res==GUP_OK)
	{
		res=encode32(com);
		free_dictionary32(com);
	}
	com->bw_buf->current=com->rbuf_current;
	return res;
}

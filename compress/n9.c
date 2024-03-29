#include "encode.h"
#include "compress.h"
#include "decode.h"

#define M7_MAX_PTR  0x10000              /* maximale pointer offset + 1 */
#define M7_MIN_MATCH 3						/* m4 maximum match */
#define M7_MAX_MATCH 256					/* m4 maximum match */
#define M7_MAX_HIST 0						/* m4 does not use history pointers */
#define ERROR_COST 32767               /* high cost for impossible matches or pointers */

#define MAX_HIST M7_MAX_HIST
#undef MIN_MATCH
#define MIN_MATCH M7_MIN_MATCH
#define MAX_PTR32 M7_MAX_PTR
#define MAX_MATCH32 M7_MAX_MATCH
#define N_PTR ARJ_NPT
#define M_PTR_BIT ARJ_PBIT;


#if(RLE32_DEPTH>(MAX_MATCH32-2))
	#undef RLE32_DEPTH
	#define RLE32_DEPTH (MAX_MATCH32-2)
#endif

#define COST_LIT(kar, com) cost_lit(kar, com)
#define COST_PTRLEN(match, ptr, pos, ptr_hist, com) cost_ptrlen(match, ptr, com)
static int cost_lit(match_t kar, packstruct *com);
static int cost_ptrlen(match_t match, ptr_t ptr, packstruct *com);

static gup_result compress(packstruct *com);

static int len_len(match_t match, packstruct *com);
static int ptr_len(ptr_t ptr, packstruct *com);

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

#include "sld32i.c" /* sliding dictionary routines */

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

#include "compress_charsi.c"

static void init_charlen(uint8 *charlen)
{
	int i;
	for(i=0; i<NLIT; i++)
	{ /* init literals */
		charlen[i]=4;
	}
	for(i=0; i<(NC-NLIT); i++)
	{ /* init len */
		charlen[i+NLIT]=1+first_bit_set32(i);
	}
}

static void init_ptrlen(uint8 *ptrlen)
{
	int i;
	for(i=0; i<MAX_NPT; i++)
	{
		ptrlen[i]=4;
	}
}	

static int cost_ptrlen(match_t match, ptr_t ptr, packstruct *com)
{
	int res=1; /* 1 bit voor aan te geven dat het een ptr len is */
	res+=len_len(match, com);
	res+=ptr_len(ptr, com);
	return res;
}

static int cost_lit(match_t kar, packstruct *com)
{
	return com->charlen[kar];
}

static int len_len(match_t match, packstruct *com)
{
	if(match<M7_MIN_MATCH)
	{
		return ERROR_COST;
	}
	if(match>M7_MAX_MATCH)
	{
		return ERROR_COST;
	}
	return com->charlen[match+NLIT-MIN_MATCH];
}

static int ptr_len(ptr_t ptr, packstruct *com)
{
	if(ptr<M7_MAX_PTR)
	{
		int res;
		int j = first_bit_set32(com->ptr_len[ptr]);
		res=com->ptrlen[j];
		j--;
		if(j > 0)
		{
			res+=j;
		}
		return res;
	}
	return ERROR_COST; /* error */
}

#define BLOCK_SIZE (4*1024) /* size of huffman block */

static gup_result compress(packstruct *com)
{
	index_t current_pos = DICTIONARY_START_OFFSET; /* wijst de te packen byte aan */
	unsigned long bytes_to_do=com->origsize;
	void* rbuf_current_store=com->rbuf_current;
	index_t token_aantal=0;
	index_t block_size;
	com->rbuf_current=com->compressed_data;
	com->bits_in_bitbuf=0;
	com->bitbuf=0;
	{
		/* eerst het totaal aantal huffman tokens tellen */
		while(bytes_to_do>0)
		{
			match_t match;
			match=com->match_len[current_pos];
			if(match==0)
			{ /* store literal */
				bytes_to_do--;
				current_pos++;
				token_aantal++;
			}
			else
			{
   	      bytes_to_do-=match;
      	   current_pos+=match;
				token_aantal++;
			}
		}
		block_size=token_aantal/((token_aantal/BLOCK_SIZE)+1);
	}
	current_pos = DICTIONARY_START_OFFSET;
	while(token_aantal>0)
	{
		if(token_aantal<(2*block_size))
		{
			block_size=token_aantal;
		}
		compress_chars32(&current_pos, block_size, com);
		token_aantal-=block_size;
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

gup_result n9_decode(decode_struct *com)
{
	/* aanname origsize>0 */
	int bib; /* bits in bitbuf */
	unsigned long int bitbuf; /* shift buffer, BITBUFSIZE bits groot */
	unsigned long origsize;
   uint8* buffer=NULL;
	uint8* buffend;
   uint8* buff;

	origsize=com->origsize;
	buffer=com->gmalloc(origsize, com->gm_propagator);
  	if(buffer == NULL)
	{
		return GUP_NOMEM;
	}
	buffend=buffer+origsize;
	buff=buffer;
	com->origsize=0;

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

gup_result n9_init(packstruct *com)
{
	gup_result res=GUP_OK;
	res=init_dictionary32(com);
	uint8 charlen[NC+NC];  /* karakter lengte */
	uint16 char2huffman[NC];  /* huffman codes van de karakters */
	uint8 ptrlen[MAX_NPT+MAX_NPT];  /* pointer lengte */
	uint16 ptr2huffman[MAX_NPT]; /* huffman codes van de pointers */
	uint8 ptrlen1[NCPT+NCPT];  /* pointer lengte */
	uint16 ptr2huffman1[NCPT];/* huffman codes van de pointers */
	com->charlen=charlen;  /* karakter lengte */
	com->char2huffman=char2huffman;
	com->ptrlen=ptrlen;
	com->ptr2huffman=ptr2huffman;
	com->ptrlen1=ptrlen1;
	com->ptr2huffman1=ptr2huffman1;
	com->rbuf_current=com->bw_buf->current;
	com->rbuf_tail=com->bw_buf->end;
	com->mv_bits_left=0;
	init_charlen(com->charlen);
	init_ptrlen(com->ptrlen);
	if(res==GUP_OK)
	{
		res=encode32(com);
		free_dictionary32(com);
	}
	com->bw_buf->current=com->rbuf_current;
	return res;
}

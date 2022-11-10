#include "encode.h"
#include "compress.h"
#include "decode.h"

int m4_len_len(match_t match);
int m4_ptr_len(ptr_t ptr);
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

unsigned long m4_cost_ptrlen(match_t match, ptr_t ptr)
{
	unsigned long res=1; /* 1 bit voor aan te geven dat het een ptr len is */
	if(match<3)
	{ /* match < 3 niet mogelijk */
		return -1UL;
	}
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
	if(match<=4)
	{
		return 3;
	}
	else if(match<=8)
	{
		return 5;
	}
	else if(match<=16)
	{
		return 7;
	}
	else if(match<=32)
	{
		return 9;
	}
	else if(match<=64)
	{
		return 11;
	}
	else if(match<=128)
	{
		return 13;
	}
	else if(match<=256)
	{
		return 14;
	}
	return 256; /*error */
}

int m4_ptr_len(ptr_t ptr)
{
	if(ptr<=511)
	{
		return 10;
	}
	else if(ptr<=1535)
	{
		return 12;
	}
	else if(ptr<=3583)
	{
		return 14;
	}
	else if(ptr<=7679)
	{
		return 16;
	}
	else if(ptr<=15871)
	{
		return 17;
	}
	return 256; /*error */
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
	      if(match < 17)
			{
				if(match < 5)
				{
					ST_BITS(2, 2);
					ST_BITS(match-3, 1);
				}
				else
				{
					if(match < 9)
					{
						ST_BITS(6, 3);
						ST_BITS(match - 5, 2);
					}
					else
					{
						ST_BITS(14, 4);
						ST_BITS(match - 9, 3);
					}
				}
			}
			else
			{
				if(match < 65)
				{
					if(match < 33)
					{
						ST_BITS(30, 5);
						ST_BITS(match - 17, 4);
					}
					else
					{
						ST_BITS(62, 6);
						ST_BITS(match - 33, 5);
					}
				}
				else
				{
					if(match < 129)
					{
						ST_BITS(126, 7);
						ST_BITS(match - 65, 6);
					}
					else
					{
						ST_BITS(127, 7);
						ST_BITS(match - 129, 7);
					}
				}
			}
			if(ptr < 1536)
			{
				if(ptr < 512)
				{
					ST_BITS(0, 1);
					ST_BITS(ptr, 9);
				}
				else
				{
					ST_BITS(2, 2);
					ST_BITS(ptr - 512, 10);
				}
			}
			else
			{
				if(ptr < 3584)
				{
					ST_BITS(6, 3);
					ST_BITS(ptr - 1536, 11);
				}
				else
				{
					if(ptr < 7680)
					{
						ST_BITS(14, 4);
						ST_BITS(ptr - 3584, 12);
					}
					else
					{
						ST_BITS(15, 4);
						ST_BITS(ptr - 7680, 13);
					}
				}
			}
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


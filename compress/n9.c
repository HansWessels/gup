#include <stdint.h>
#include "encode.h"
#include "compress.h"
#include "decode.h"

#define M7_MAX_PTR  0x10000              /* maximale pointer offset + 1 */
#define M7_MIN_MATCH 3						/* m4 maximum match */
#define M7_MAX_MATCH 256					/* m4 maximum match */

#define TREE63_MIN_MATCH M7_MIN_MATCH
#define TREE63_MAX_PTR M7_MAX_PTR
#define TREE63_MAX_MATCH M7_MAX_MATCH
#define N_PTR ARJ_NPT
#define N_PTR_BIT ARJ_PBIT

#define MAX_BUCKETS (N_PTR+1)

static gup_result compress(packstruct *com);

static int len_len(len63_t len, packstruct *com);
static int ptr_len(pointer63_t ptr, packstruct *com);
static int32_t ptr_index(pointer63_t ptr);

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

#include "sld63i.c" /* sliding dictionary routines */

static gup_result compress(packstruct *com)
{
	return GUP_OK;
}


gup_result n9_decode(decode_struct *com)
{
	return GUP_OK; /* exit succes */
}

gup_result n9_init(packstruct *com)
{
	gup_result res=GUP_OK;
	init_dictionary63(com);
	uint64_t orig_size;
	uint64_t bytes_to_do;
	if(res!=GUP_OK)
	{
		return res;
	}
	{ /*- dictionary buffer vullen */
		long byte_count;
		if ((byte_count = com->buf_read_crc(com->origsize, com->dictionary63, com->brc_propagator)) < 0)
		{
			return GUP_READ_ERROR; /* ("Read error"); */
		}
		else
		{
			if (com->origsize == 0)
			{
				com->packed_size = 0;
				com->bytes_packed = 0;
				return GUP_OK;
			}
			else if (com->origsize != byte_count)
			{
				return GUP_READ_ERROR; /* ("Read error"); */
			}
			#ifndef PP_AFTER
			com->print_progres(byte_count, com->pp_propagator);
			#endif
		}
		orig_size = (uint64_t)byte_count;
		bytes_to_do = orig_size;
	}
#if 01
    mb63_i mb_pos=0;
    for(uint64_t i=0; i<bytes_to_do; i++)
    {
        mb63_i new_mb_pos=match63(com, i, mb_pos);
        printf("pos=%li\n", i);
        while(mb_pos<new_mb_pos)
        {
            if(com->mb63[mb_pos].len==0)
            {
                if((com->mb63[mb_pos].u.lit>32) && (com->mb63[mb_pos].u.lit<127))
                {
                    printf("literal = %c\n", com->mb63[mb_pos].u.lit);
                }
                else
                {
                    printf("literal = \\%02X\n", com->mb63[mb_pos].u.lit);
                }
            }
            else
            {
                printf("len=%5i, ptr=%5i, ", com->mb63[mb_pos].len, com->mb63[mb_pos].u.ptr);
                for(int j=0; j<com->mb63[mb_pos].len; j++)
                {
                    if((com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]>32) && (com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]<127))
                    {
                        printf("%c", com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]);
                    }
                    else
                    {
                        printf("\\%02X", com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]);
                    }
                }
                printf("\n");
            }
            mb_pos++;
        }
    }
	printf("\n");
#endif
#if 0
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
#endif
    free_dictionary63(com);
	return res;
}

static int32_t ptr_index(pointer63_t ptr)
{
    return first_bit_set32(ptr);
}

static int32_t ptr2bucket(pointer63_t ptr)
{
    return ptr_index(ptr);
}

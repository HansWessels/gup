#include "encode.h"
#include "compress.h"
#include "decode.h"

int n2_len_len(match_t match);
int n2_ptr_len(ptr_t ptr, ptr_t *ptr_hist);
void n2_store_len(match_t match, ptr_t ptr, packstruct *com);
void n2_store_ptr(ptr_t ptr, ptr_t last_ptr, packstruct *com);

#define MATCH_2_CUTTOFF 0x400

#if 0
#define STATISTICS
	#define LIT_STAT_FINE 300 /* tot LIT_RUN_FINE exact bijhouden, daarboven log */
	#define LIT_STAT_COUNT 300
	#define LEN_RUN_FINE 300
	#define LEN_RUN_COUNT 300
	#define LEN_STAT_FINE 300
	#define LEN_STAT_COUNT 300
	#define PTR_STAT_FINE 300
	#define PTR_STAT_COUNT 300
	#define STAT_MAX (300)
	#define LAST_PTR_COUNT 16
	static unsigned long lit_stat_fine[LIT_STAT_FINE]={0};
	static unsigned long lit_stat[LIT_STAT_COUNT]={0};
	static unsigned long len_run_stat_fine[LEN_RUN_FINE]={0};
	static unsigned long len_run_stat[LEN_RUN_COUNT]={0};
	static unsigned long len_stat_fine[LEN_STAT_FINE]={0};
	static unsigned long len_stat[LEN_STAT_COUNT]={0};
	static unsigned long ptr_stat_fine[PTR_STAT_FINE]={0};
	static unsigned long ptr_stat[PTR_STAT_COUNT]={0};
	static unsigned long ptr_copy[LAST_PTR_COUNT]={0};
	static unsigned long total_size=0;
#endif

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


/* unsigned long val, int bit_count */
#define N2_ST_BIT(bit)												\
{ /* store a 1 or a 0 */											\
	int val=bit;			                                 \
	LOG_BIT(val);														\
	if(com->bits_in_bitbuf==0)										\
	{ /* reserveer plek in bytestream */						\
		com->command_byte_ptr=com->rbuf_current++;			\
		com->bitbuf=0;													\
	}																		\
	com->bits_in_bitbuf++;											\
	com->bitbuf+=com->bitbuf+val;									\
	if (com->bits_in_bitbuf >= 8)									\
	{																		\
		com->bits_in_bitbuf=0;										\
		*com->command_byte_ptr=(uint8)com->bitbuf;			\
	}																		\
}


unsigned long n2_cost_ptrlen(match_t match, ptr_t ptr, index_t pos, ptr_t *ptr_hist)
{
	NEVER_USE(pos);
	unsigned long res=1; /* 1 bit voor aan te geven dat het een ptr len is */
	if(match==2)
	{
		if(ptr>=MATCH_2_CUTTOFF)
		{
			res+=256;
		}
	}
	else
	{
		if(ptr>=MATCH_2_CUTTOFF)
		{
			match--;
		}
	}
	res+=n2_len_len(match);
	res+=n2_ptr_len(ptr, ptr_hist);
	return res;
}

unsigned long n2_cost_lit(match_t kar)
{
	NEVER_USE(kar);
	return 9; /* 1 bit om aan te geven dat het een literal is en 8 bits voor de literal */
}

int n2_len_len(match_t match)
{
	if((match<N2_MIN_MATCH) || (match>N2_MAX_MATCH))
	{
		return 256; /* error */
	}
	if(match<5)
	{
		return 2;
	}
	match-=3;
	return 2+2*(first_bit_set32(match)-1);
}

int n2_ptr_len(ptr_t ptr, ptr_t *ptr_hist)
{
	if(ptr==ptr_hist[0])
	{
		return 2;
	}
	return 8+2*(first_bit_set32((ptr>>8)+3)-1);
}

void n2_store_len(match_t match, ptr_t ptr, packstruct *com)
{
	if(ptr>=MATCH_2_CUTTOFF)
	{
		match--;
	}
	if(match==2)
	{
		N2_ST_BIT(0);
		N2_ST_BIT(0);
		return;
	}
	if(match==3)
	{
		N2_ST_BIT(0);
		N2_ST_BIT(1);
		return;
	}
	if(match==4)
	{
		N2_ST_BIT(1);
		N2_ST_BIT(0);
		return;
	}
	N2_ST_BIT(1);
	N2_ST_BIT(1);
	match-=3;
	{
		int len;
		match_t mask;
		len=first_bit_set32(match);
		mask=1<<(len-2);
		do
		{ /* stuur bits */
			if(match&mask)
			{
				N2_ST_BIT(1);
			}
			else
			{
				N2_ST_BIT(0);
			}
			mask>>=1;
			if(mask==0)
			{
				N2_ST_BIT(1);
			}
			else
			{
				N2_ST_BIT(0);
			}
		}
		while(mask!=0);
	}
}

void n2_store_ptr(ptr_t ptr, ptr_t last_ptr, packstruct *com)
{
	if(ptr==last_ptr)
	{ /* pointer reuse, send 1 */
		N2_ST_BIT(1);
		N2_ST_BIT(1);
	}
	else
	{
		int len;
		ptr_t mask;
		uint8 ptr_lsb=(uint8)((~ptr)&0xff);
		ptr>>=8;
		ptr+=3;
		len=first_bit_set32(ptr);
		mask=1<<(len-2);
		do
		{ /* stuur geinverteerde bits */
			if(ptr&mask)
			{
				N2_ST_BIT(0);
			}
			else
			{
				N2_ST_BIT(1);
			}
			mask>>=1;
			if(mask==0)
			{
				N2_ST_BIT(1);
			}
			else
			{
				N2_ST_BIT(0);
			}
		}
		while(mask!=0);
		*com->rbuf_current++=ptr_lsb;
	}
}

gup_result n2_compress(packstruct *com)
{
	index_t current_pos = DICTIONARY_START_OFFSET; /* wijst de te packen byte aan */
	unsigned long bytes_to_do=com->origsize;
	void* rbuf_current_store=com->rbuf_current;
	com->rbuf_current=com->compressed_data;
	com->bits_in_bitbuf=0;

#ifdef STATISTICS
	{
		unsigned long i=bytes_to_do;
		unsigned int lit_run=0;
		unsigned int len_run=0;
		ptr_t last_ptr[LAST_PTR_COUNT]={0};
		index_t current_pos = DICTIONARY_START_OFFSET; /* wijst de te packen byte aan */
		while(i>0)
		{
			match_t match;
			match=com->match_len[current_pos];
			if (match == 0)
			{ /* literal */
				lit_run++;
				/* if(len_run!=0) */
				{
					if(len_run<LEN_RUN_FINE)
					{
						len_run_stat_fine[len_run]++;
					}
					len_run_stat[first_bit_set32(len_run)]++;
					len_run=0;
				}
				total_size+=n2_cost_lit(match);
				i--;
				current_pos++;
			}
			else
			{
				ptr_t ptr;
				int j;
				ptr=com->ptr_len[current_pos];
				len_run++;
				/* if(lit_run!=0) */
				{
					if(lit_run<LIT_STAT_FINE)
					{
						lit_stat_fine[lit_run]++;
					}
					lit_stat[first_bit_set32(lit_run)]++;
					lit_run=0;
				}
				if(match<LEN_STAT_FINE)
				{
					len_stat_fine[match]++;
				}
				len_stat[first_bit_set32(match)]++;
				if(ptr<PTR_STAT_FINE)
				{
					ptr_stat_fine[ptr]++;
				}
				ptr_stat[first_bit_set32(ptr)]++;
				{
					total_size+=n2_cost_ptrlen(match, ptr, i, last_ptr);
				}
	         i-=match;
   	      current_pos+=match;
				{
					ptr_t copy_ptr;
					copy_ptr=ptr;
					for(j=0; j<LAST_PTR_COUNT; j++)
					{
						ptr_t tmp;
						tmp=last_ptr[j];
						if(ptr==last_ptr[j])
						{
							ptr_copy[j]++;
							last_ptr[j]=copy_ptr;
							break;
						}
						last_ptr[j]=copy_ptr;
						copy_ptr=tmp;
					}
				}
			}
		}
	}
#endif

	{ /* send free literal */
		match_t kar;
		kar=com->dictionary[current_pos];
		LOG_LITERAL(kar);
		*com->rbuf_current++=(uint8)kar;
		bytes_to_do--;
		current_pos++;
	}
	{ /* send main message */
		ptr_t last_ptr=0;
		{
			while(bytes_to_do>0)
			{
				match_t match;
				match=com->match_len[current_pos];
				if(match==0)
				{ /* store literal */
					match_t kar;
					N2_ST_BIT(0);
					kar=com->dictionary[current_pos];
					LOG_LITERAL(kar);
					*com->rbuf_current++=(uint8)kar;
					bytes_to_do--;
					current_pos++;
				}
				else
      		{
		      	ptr_t ptr;
					N2_ST_BIT(1);
					ptr=com->ptr_len[current_pos];
      		   n2_store_ptr(ptr, last_ptr, com);
		         n2_store_len(match, ptr, com);
      		   LOG_PTR_LEN(match, ptr);
      		   last_ptr=ptr;
		         bytes_to_do-=match;
      		   current_pos+=match;
      		}
      	}
		}
	}
	{ /* send end of stream token, 15 zero's makes the 16 bit value 0 */
		int i;
		N2_ST_BIT(1); /* pointer is comming */
		for(i=0; i<29; i++)
		{
			N2_ST_BIT(0);
		}
		N2_ST_BIT(1);
	}
	if (com->bits_in_bitbuf>0)
	{
		com->bitbuf=com->bitbuf<<(8-com->bits_in_bitbuf);
		*com->command_byte_ptr=(uint8)com->bitbuf;
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

gup_result n2_close_stream(packstruct *com)
{
	NEVER_USE(com);
	#ifdef STATISTICS
	{
		int i;
		printf("****************************************** Statistics results *****************************************\n");
		printf("Packed size = %lu\n", total_size/8);
		printf("   i    Lit_run log(lit_run)       ptr   log(ptr)  last_ptr   len_run log(len_run)       len   log(len)\n");
		for(i=0; i<STAT_MAX; i++)
		{
			printf("%4i", i);
			if(i<LIT_STAT_FINE)
			{
				printf(" %10lu", lit_stat_fine[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			if(i<LIT_STAT_COUNT)
			{
				printf(" %10lu", lit_stat[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			if(i<PTR_STAT_FINE)
			{
				printf(" %10lu", ptr_stat_fine[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			if(i<PTR_STAT_COUNT)
			{
				printf(" %10lu", ptr_stat[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			if(i<LAST_PTR_COUNT)
			{
				printf(" %10lu", ptr_copy[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			if(i<LEN_RUN_FINE)
			{
				printf(" %10lu", len_run_stat_fine[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			if(i<LEN_RUN_COUNT)
			{
				printf(" %10lu", len_run_stat[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
		
			if(i<LEN_STAT_FINE)
			{
				printf(" %10lu", len_stat_fine[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			if(i<LEN_STAT_COUNT)
			{
				printf(" %10lu", len_stat[i]);
			}
			else
			{
				printf(" %10lu", 0UL);
			}
			printf("\n");
		}
		printf("**************************************** Statistics results end ***************************************\n");
	}
#endif
	return GUP_OK;
}

#define N2_GET_BIT(bit)								\
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

#define N2_GET_LEN(len)								\
{ /* get length from data stream */				\
	int bit;												\
	N2_GET_BIT(bit);									\
	len=bit;												\
	N2_GET_BIT(bit);									\
	len+=len+bit;										\
	if(len<3)											\
	{ /* short len */									\
		len+=2;											\
	}														\
	else													\
	{ /* long len */									\
		len=1;											\
		do													\
		{													\
			N2_GET_BIT(bit);							\
			len+=len+bit;								\
			N2_GET_BIT(bit);							\
		} while(bit==0);								\
		len+=3;											\
	}														\
	if(ptr>=MATCH_2_CUTTOFF)						\
	{														\
		len++;											\
	}														\
}
			


#define N2_GET_PTR(ptr)								\
{ /* get pointer from data stream */			\
	int tmp=-2;											\
	int bit;												\
	do														\
	{														\
		N2_GET_BIT(bit);								\
		tmp+=tmp+bit;									\
		N2_GET_BIT(bit);								\
	} while(bit==0);									\
	if(tmp==-65536)									\
	{ /* eof token */									\
		break;											\
	}														\
	tmp+=3;												\
	if(tmp==0)											\
	{														\
		ptr=last_ptr;									\
	}														\
	else													\
	{														\
  		if(com->rbuf_current>com->rbuf_tail)	\
		{													\
			gup_result res;							\
			if((res=read_data(com))!=GUP_OK)		\
			{												\
				return res;								\
			}												\
		}													\
		tmp<<=8;											\
		tmp|=*com->rbuf_current++;					\
		ptr=~tmp;										\
	}														\
}

gup_result n2_decode(decode_struct *com)
{
   uint8* buffer;
   uint8* buff;
	uint8 bitbuf=0;
	ptr_t last_ptr=0;
	unsigned long origsize;
	int bits_in_bitbuf=0;
	if(com->origsize==0)
	{
		return GUP_OK; /* exit succes? */
	}
	origsize=com->origsize;
	buffer=com->gmalloc(origsize, com->gm_propagator);
  	if(buffer == NULL)
	{
		return GUP_NOMEM;
	}
	buff=buffer;
	com->origsize=0;
	
	{ /* start met een literal */
		origsize--;
		match_t kar;
  		if(com->rbuf_current > com->rbuf_tail)
		{
			gup_result res;
			if((res=read_data(com))!=GUP_OK)
			{
				return res;
			}
		}
		kar=*com->rbuf_current++;
		*buff++=kar;
		LOG_LITERAL(kar);
	}
	for(;;)
	{
		int bit;
		N2_GET_BIT(bit);
		if(bit==0)
		{ /* literal */
			origsize--;
			match_t kar;
	  		if(com->rbuf_current > com->rbuf_tail)
			{
				gup_result res;
				if((res=read_data(com))!=GUP_OK)
				{
					return res;
				}
			}
			kar=*com->rbuf_current++;
			*buff++=kar;
			LOG_LITERAL(kar);
		}
		else
		{ /* ptr len */
			match_t len;
			ptr_t ptr;
			N2_GET_PTR(ptr);
			N2_GET_LEN(len);
			{ /* copy */
				uint8* q=buff-ptr-1;
				LOG_PTR_LEN(len, ptr);
				do
				{
					*buff++=*q++;
				} 
				while(--len>0);
			}
			last_ptr=ptr;
		}
	}
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
	}
	return GUP_OK; /* exit succes */
}


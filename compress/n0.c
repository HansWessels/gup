#include "encode.h"
#include "compress.h"
#include "decode.h"

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


int n0_lit_len(uint32 val);
int n0_len_len(uint32 val);
int n0_ptr_len(uint32 val);
void n0_store_val(uint32 val, packstruct *com);
void n0_store_len_val(uint32 val, packstruct *com);
void n0_store_literal_val(uint32 val, packstruct *com);
void n0_store_ptr_val(int32_t val, packstruct *com);
unsigned long n0_count_bits(unsigned long *packed_bytes,  /* aantal bytes dat gepacked wordt */
                            uint16 entries, /* aantal character die moeten worden gepacked */
                            c_codetype * p, /* pointer naar de karakters     */
                            pointer_type * q);

int n0_len_len(uint32 val)
{ /* bereken de code lengte voor val, 2 <= val <= 2^32 */
	return 2*(first_bit_set32(val-1)-1);
}

int n0_ptr_len(uint32 val)
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

#define N0_ST_BIT(bit)												\
{ /* store a 1 or a 0 */											\
  int val=bit;				                                 \
  LOG_BIT(val);														\
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

void n0_store_val(uint32 val, packstruct *com)
{ /* waarde val >=2 */
	int bits_to_do=first_bit_set32(val)-1;
	uint32 mask=1<<bits_to_do;
	mask>>=1;
	do
	{
		if((val&mask)==0)
		{
			N0_ST_BIT(0);
		}
		else
		{
			N0_ST_BIT(1);
		}
		mask>>=1;
		if(mask==0)
		{
			N0_ST_BIT(1);
		}
		else
		{
			N0_ST_BIT(0);
		}
	}while(mask!=0);
}

void n0_store_len_val(uint32 val, packstruct *com)
{ /* waarde val >=3 */
	n0_store_val(val-1, com);
}

void n0_store_ptr_val(int32_t val, packstruct *com)
{ /* waarde val >=0 <=65535 */
	val++;
	if(val<=256)
	{
		val=-val;
		*com->rbuf_current++ = (uint8) (val&0xff);
		N0_ST_BIT(0);
	}
	else
	{
		val=-val;
		*com->rbuf_current++ = (uint8) ~((val>>8)&0xff);
		N0_ST_BIT(1);
		*com->rbuf_current++ = (uint8) (val&0xff);
	}
}

gup_result n0_compress(packstruct *com)
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
   * enz...
   *   
  */
  int entries = (int) (com->charp - com->chars);
  if (com->matchstring != NULL)
  {
    ARJ_Assert(com->backmatch!=NULL);
    {
      /* 
        Code die backmatch stringlengtes optimaliseert.
        Bij een backmatch zijn er twee opeenvolgende matches, waarbij 
        de laatste match een backmatch waarde groter dan nul heeft.
        Deze laatste macth kunnen we groter laten worden tenkoste van de 
        grootte van de match die ervoor ligt...
        Waarschuwing, zodra je met matchlengtes hebt geschove kin je geen zeef34 code
        meer toepassen. Dus eerst zeef34 conversies en daarna matchlen optimalisatie.
      */
      { /* ronde 1, alleen zeef 34, twee maal achter elkaar zeven is een probeem, van achteren naar voren werken? Of voor naar achter met vlag */
			uint8* bp=com->backmatch;
			c_codetype *p = com->chars;
			pointer_type *q= com->pointers;
			*bp=0; /* een backmatch naar een vorig blok is niet mogelijk */
			int i = entries;
			do
			{
				c_codetype kar = *p++;
				if (kar > (NLIT-1))
				{
					bp++; /* next backmatch */
					q++; /* next pointer */
				}
			} while (--i!=0);
			i = entries;
			do
			{ /* nu staat alles achter aan, terug werken */
				c_codetype kar = *--p;
				if (kar > (NLIT-1))
				{
					uint8 len=*--bp;
					q--;
					if(len>0)
					{
						c_codetype kar_1=p[-1]+MIN_MATCH-NLIT-len;
						if(kar_1<3)
						{ /* conversie naar 1 of 2? */
							if(kar_1==2)
							{ /* conversie naar 2? */
								if((n0_ptr_len(q[-1])+n0_len_len(3)+1)>18) /* pointer lengte is of 9 of 17, de len(3) = 2 dus bij een lange ptr altijd raak, bij een korte nooit raak */
								{ /* conversie naar 2 mogelijk */
									if(bp[-1]>0)
									{ /* extra conversie? */
										c_codetype kar_2=p[-2]+MIN_MATCH-NLIT-bp[-1];
										if(kar_2>2)
										{ /* geen extra conversie */
											p[0]+=len;
											bp[0]=0;
											p[-1]=-2;
										}
										else
										{ /* deze of volgende conversie nemen? */
											if(kar_2==2)
											{ /* conversie twee */
												if(n0_ptr_len(q[-2])>n0_ptr_len(q[-1]))
												{ /* neem de langste pointer */
													p[-1]+=bp[-1];
													bp[-1]=0;
													p[-2]=-2;
												}
												else
												{ /* originele conversie houden */
													p[0]+=len;
													bp[0]=0;
													p[-1]=-2;
												}
											}
											else
											{ /* conversie een */
												p[-1]+=bp[-1];
												bp[-1]=0;
												p[-2]=-1;
											}
										}
									}
									else
									{ /* geen extra conversie */
										p[0]+=len;
										bp[0]=0;
										p[-1]=-2;
									}
								}
							}
							else
							{ /* conversie naar 1 */
								if(bp[-1]>0) 
								{ /* extra conversie? */
									c_codetype kar_2=p[-2]+MIN_MATCH-NLIT-bp[-1];
									if(kar_2==1)
									{
										if(n0_ptr_len(q[-2])>n0_ptr_len(q[-1]))
										{ /* neem de langste pointer */
											p[-1]+=bp[-1];
											bp[-1]=0;
											p[-2]=-1;
										}
										else
										{ /* geen extra conversie */
											p[0]+=len;
											bp[0]=0;
											p[-1]=-1;
										}
									}
									else
									{ /* geen extra conversie */
										p[0]+=len;
										bp[0]=0;
										p[-1]=-1;
									}
								}
								else
								{
									p[0]+=len;
									bp[0]=0;
									p[-1]=-1;
								}
							}
						}
					}
				}
				else if(kar<0)
				{
					q--;
					bp--;
				}
			} while (--i!=0);
      }
      { /* ronde 2, matchlen optimalisatie */
        uint8* bp=com->backmatch;
        c_codetype *p = com->chars;
        pointer_type *q= com->pointers;
        int i = entries;
        do
        {
          c_codetype kar = *p++;
          if (kar > (NLIT-1))
          {
            uint8 len=*bp++;
            if(len>0)
            {
              c_codetype kar_1=p[-2]+MIN_MATCH-NLIT;
              int offset=1;
              int optlen;
              kar+=MIN_MATCH-NLIT;
              optlen=n0_len_len(kar_1)+n0_len_len(kar);
              do
              {
                if(((n0_len_len(kar_1-offset)+n0_len_len(kar+offset))<optlen) && ((kar_1-offset)>2))
                {
                 	bp[-1]-=offset;
                 	p[-2]-=offset;
                 	p[-1]+=offset;
                 	kar+=offset;
                 	kar_1-=offset;
                 	optlen=n0_len_len(kar_1)+n0_len_len(kar);
                 	offset=1;
                }
                else
                {
                  offset++;
                }
              }
              while(--len!=0);
              offset--;
            }
            q++; /* next pointer */
          }
          else if(kar<0)
          { /* is al geconverteerd */
          	q++; /* next pointer */
          	bp++; /* next backmatch */
          }
        } while (--i!=0);
      }
    }
  }
  { /* bereken het aantal bits en bytes van de gecomprimeerde data */
    unsigned long m_size;
    long bits_comming = n0_count_bits(&m_size, entries, com->chars, com->pointers);
    {
      gup_result res;
      long bytes_extra=0;
      if(com->command_byte_ptr!=NULL)
      { /* command byte pointer is gebruikt, is hij nu in gebruik? */
      	if(com->bits_in_bitbuf!=0)
      	{ /* er zitten bits in de bitbuf, we mogen wegschrijven tot de command_byte_ptr */
      		bytes_extra=com->rbuf_current-com->command_byte_ptr;
      		com->rbuf_current=com->command_byte_ptr;
      	}
      }
      else
      { /* bij het eerste blok beginnen we met een literal, 1 bit minder versturen */
      	bits_comming--;
      }
      if((res=announce(bytes_extra+((bits_comming+7)>>3), com))!=GUP_OK)
      {
        return res;
      }
      if(com->command_byte_ptr!=NULL)
      {
      	memcpy(com->rbuf_current, com->command_byte_ptr, bytes_extra);
      	com->command_byte_ptr=com->rbuf_current;
      	com->rbuf_current+=bytes_extra;
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
  { /* send the message... */
    c_codetype *p = com->chars;
    pointer_type *q = com->pointers;
    uint8 *r = com->matchstring;
    if(com->command_byte_ptr==NULL)
    { /* eerste byte is gratis */
      c_codetype kar = *p++;
  		LOG_LITERAL(kar);
  		*com->rbuf_current++=kar;
  		entries--;
    }
    while(entries-- != 0)
    {
      c_codetype kar = *p++;
      
      if (kar < NLIT)
      { /*- store literal */
      	N0_ST_BIT(0);
        	if(kar==-1)
        	{
        		kar=*r++;	
        		r+=3;
        		q++; /* skip pointer */
        	}
        	else if(kar==-2)
        	{
        		kar=*r++;	
     			LOG_LITERAL(kar);
     			*com->rbuf_current++=kar;
      		N0_ST_BIT(0);
        		kar=*r++;	
        		r+=2;
        		q++; /* skip pointer */
        	}
     		LOG_LITERAL(kar);
     		*com->rbuf_current++=kar;
      }
      else
      {
			N0_ST_BIT(1);
         kar += MIN_MATCH - NLIT;
         n0_store_ptr_val(*q++, com);
         n0_store_len_val(kar, com);
         LOG_PTR_LEN(kar, q[-1]+1);
         r+=4; /* skip literals */
      }
    }
    entries=(uint16)(com->charp-p);
    if (com->matchstring != NULL)
    {
      com->msp = com->matchstring;
      com->bmp = com->backmatch;
    }
    com->charp = com->chars;
    com->ptrp = com->pointers;
  }
  return GUP_OK;
}

unsigned long n0_count_bits(unsigned long *packed_bytes,  /* aantal bytes dat gepacked wordt */
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
    { /* store literal */
  		bytes++;
    	bits+=9;
    	if(kar<0)
    	{ /* geconverteerde ptr len, next pointer */
    		if(kar==-2)
    		{ /* dubbele zeef */
  				bytes++;
    			bits+=9;
    		}
    		q++;
    	}
    }
    else
    { /* ptr-len paar */
    	kar+=MIN_MATCH-NLIT;
      bytes+=kar;
      bits+=n0_len_len(kar)+1;
      bits+=n0_ptr_len(*q++);
    }
  }
  *packed_bytes = bytes;
  return bits;
}

void n0_init_fast_log(packstruct *com)
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
  uint32 i;
  for(i=0;i<65536; i++)
  {
    *p++=n0_ptr_len(i);
  }
}

gup_result n0_close_stream(packstruct *com)
{
	long bits_comming=10; /* end of archive marker */
	long bytes_extra=0;
	gup_result res;
	if(com->command_byte_ptr!=NULL)
	{ /* command byte pointer is gebruikt, is hij nu in gebruik? */
		if(com->bits_in_bitbuf!=0)
		{ /* er zitten bits in de bitbuf, we mogen wegschrijven tot de command_byte_ptr */
			bytes_extra=com->rbuf_current-com->command_byte_ptr;
			com->rbuf_current=com->command_byte_ptr;
		}
	}
	if((res=announce(bytes_extra+((bits_comming+7)>>3), com))!=GUP_OK)
	{
		return res;
	}
	if(com->command_byte_ptr!=NULL)
	{
		memcpy(com->rbuf_current, com->command_byte_ptr, bytes_extra);
		com->command_byte_ptr=com->rbuf_current;
		com->rbuf_current+=bytes_extra;
   }
   bits_comming+=com->bits_rest;
   com->bits_rest=(int16)(bits_comming&7);
   com->packed_size += bits_comming>>3;
	{ /* schrijf n0 end of stream marker, een speciaal geformateerde ptr */
		N0_ST_BIT(1); /* pointer comming */
		*com->rbuf_current++ = 0; 
		N0_ST_BIT(1); /* deze combi kan niet voorkomen */
	}
	if (com->bits_in_bitbuf>0)
	{
		com->bitbuf=com->bitbuf<<(8-com->bits_in_bitbuf);
		*com->command_byte_ptr=(uint8)com->bitbuf;
		com->bits_in_bitbuf=0;
	}
	com->command_byte_ptr=NULL;
	if(com->bits_rest!=0)
	{
		com->packed_size++; /* corrigeer packed_size */
	}
	return GUP_OK;
}


#define N0_GET_BIT(bit)								\
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
		N0_GET_BIT(bit);								\
		val+=val+bit;									\
		N0_GET_BIT(bit);								\
	} while(bit==0);									\
}


gup_result n0_decode(decode_struct *com)
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
		N0_GET_BIT(bit);
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
			N0_GET_BIT(bit);
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

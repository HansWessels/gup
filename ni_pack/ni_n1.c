/*
** ni packer mode n1 depacker
** 2022 Hans Wessels
*/

#include <stdint.h>

typedef uint8_t uint8;         /* unsigned 8 bit */
typedef uint16_t uint16;    /* unsigned 16 bit */
typedef int16_t kartype;   /* signed 16 bit */
typedef uint32_t uint32;
typedef int32_t int32;


#define GET_N1_BIT(bit)								\
{ /* get a bit from the data stream */			\
 	if(bits_in_bitbuf==0)							\
 	{ /* fill bitbuf */								\
  		bitbuf=*data++;								\
  		bits_in_bitbuf=8;								\
	}														\
	bit=(bitbuf&0x80)>>7;							\
	bitbuf+=bitbuf;									\
	bits_in_bitbuf--;									\
}

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

#define DECODE_N1_PTR(ptr)							\
{ /* n1 ptr(4) get value -1 - -65536 */		\
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
		

void decode_n1(uint8_t *dst, uint8_t *data)
{
	uint8* dstend;
	uint8 bitbuf=0;
	int bits_in_bitbuf=0;
	unsigned long loop;
	int ptrs=0;
	unsigned long run;
	DECODE_N1_COUNTER(loop);
	do
	{
		DECODE_N1_RUN(run);
		if(ptrs==0)
		{ /* literal run */
			do
			{
				*dst++=*data++;
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
				src=dst+ptr;
				do
				{
  					*dst++=*src++;
				} while(--len!=0);
			} while(run-->0);
			ptrs=0;
		}
	} while(loop-->0);
}

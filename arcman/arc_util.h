/*
 * Archive manager.
 *
 *     Utility functions.
 *
 * $Author: klarenw $
 * $Date: 1998-12-28 15:58:10 +0100 (Mon, 28 Dec 1998) $
 * $Revision: 136 $
 * $Log$
 * Revision 1.1  1998/12/28 14:58:08  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 */

/*
 * Inline functions for writing words and longwords to a buffer in
 * little endian format.
 */

static inline void put_word(uint8 *ptr, uint16 v)
{
	ptr[0] = (uint8)(v);
	ptr[1] = (uint8)((v) >> 8);
}

static inline void put_long(uint8 *ptr, uint32 v)
{
	ptr[0] = (uint8)(v);
	ptr[1] = (uint8)((v) >> 8);
	ptr[2] = (uint8)((v) >> 16);
	ptr[3] = (uint8)((v) >> 24);
}

static inline void put_word_i(uint8 *&ptr, uint16 v)
{
	*ptr++ = (uint8)(v);
	*ptr++ = (uint8)((v) >> 8);
}

static inline void put_long_i(uint8 *&ptr, uint32 v)
{
	*ptr++ = (uint8)(v);
	*ptr++ = (uint8)((v) >> 8);
	*ptr++ = (uint8)((v) >> 16);
	*ptr++ = (uint8)((v) >> 24);
}

/*
 * Inline functions for reading words and longwords from a buffer in
 * little endian format.
 */

static inline uint8 get_byte(uint8 *ptr)
{
	return *ptr;
}

static inline uint16 get_word(uint8 *ptr)
{
	uint16 result;

	result = (uint16) *ptr;
	return (uint16) (result + ((uint16) *(ptr+1) << 8));
}

static inline uint32 get_longword(uint8 *ptr)
{
	uint32 result;

	result = (uint32) *ptr;
	result += ((uint32) *(ptr+1)) << 8;
	result += ((uint32) *(ptr+2)) << 16;
	return (result + (((uint32) *(ptr+3)) << 24));
}

static inline uint8 get_byte_i(uint8 *&ptr)
{
	return *ptr++;
}

static inline uint16 get_word_i(uint8 *&ptr)
{
	uint16 result;

	result = (uint16) *ptr++;
	return (uint16) (result + ((uint16) *ptr++ << 8));
}

static inline uint32 get_longword_i(uint8 *&ptr)
{
	uint32 result;

	result = (uint32) *ptr++;
	result += ((uint32) *ptr++) << 8;
	result += ((uint32) *ptr++) << 16;
	return (result + (((uint32) *ptr++) << 24));
}

/*
 * Time conversion functions.
 */

uint32 time_unix_to_dos(uint32 time);
uint32 time_dos_to_unix(uint32 time);

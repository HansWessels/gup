
#include <stdlib.h>

#include "gup.h"
#include "crc.h"

/************************************************************************
 ****************              ARJ CRC              *********************
 ************************************************************************/

#ifndef NOT_USE_STD_crc32

/*
 * unsigned long crc32(const uint8 *str, long len, unsigned long crc,
 *										 const uint32 *crc_table)
 *
 * Calculate the CRC of a buffer.
 *
 * Parameters:
 *
 * str				- pointer to buffer.
 * len				- length of buffer.
 * crc				- crc start value.
 * crc_table	- pointer to crc table.
 *
 * Result: CRC of buffer.
 */

#define NLIT 256

uint32 crc32(const uint8 * str, long len, uint32 crc, const uint32 * crc_table)
{
  do
  {
    uint8 kar = *str++;

    kar ^= (uint8) crc;
    crc >>= 8;
    crc ^= crc_table[kar];
  }
  while (--len != 0);

  return crc;
}

#endif

#ifndef NOT_USE_STD_arj_init_crc

uint32 arj_init_crc(void)
{
  return (uint32) - 1;
}

#endif

#ifndef NOT_USE_STD_free_crc32_table

void free_crc32_table(uint32 * table)
{
  if (table != NULL)
  {
    free(table);
  }
}

#endif

#ifndef NOT_USE_STD_make_crc32_table

/*
 * unsigned long *make_crc32_table(void)
 *
 * Initialize the CRC table used by the buf_crc() function.
 *
 * Result: NULL if out of memory, pointer to crc table otherwise.
 */

uint32 *make_crc32_table(uint32 * crc_poly_p)
{
  uint32 *crc_table;

  if ((crc_table = malloc(NLIT * sizeof(uint32))) != NULL)
  {
    uint32 poly;
    uint32 kar;

    kar = NLIT - 1;
    poly = *crc_poly_p;

    do
    {
      int i = 7;
      uint32 crc = kar;

      do
      {
        if (crc & 1)
        {
          crc >>= 1;
          crc ^= poly;
        }
        else
        {
          crc >>= 1;
        }
      }
      while (i-- != 0);

      crc_table[kar] = crc;
    }
    while (kar-- != 0);
  }

  return crc_table;
}

#endif


/************************************************************************
 ****************              LHA CRC              *********************
 ************************************************************************/

#ifndef NOT_USE_STD_crc16

/*
 * uint16 crc16(const uint8 *str, long len, uint16 crc, const uint16 *crc_table)
 *
 * Calculate the CRC of a buffer.
 *
 * Parameters:
 *
 * str				- pointer to buffer.
 * len				- length of buffer.
 * crc				- crc start value.
 * crc_table	- pointer to crc table.
 *
 * Result: CRC of buffer.
 */

#define NLIT 256

uint16 crc16(const uint8 * str, long len, uint16 crc,
             const uint16 * crc_table)
{
  do
  {
    uint8 kar = *str++;

    kar ^= (uint8) crc;
    crc >>= 8;
    crc ^= crc_table[kar];
  }
  while (--len != 0);

  return crc;
}

#endif

#ifndef NOT_USE_STD_lha_init_crc

uint16 lha_init_crc(void)
{
  return 0;
}

#endif

#ifndef NOT_USE_STD_free_crc16_table

void free_crc16_table(uint16 * table)
{
  if (table != NULL)
  {
    free(table);
  }
}

#endif

#ifndef NOT_USE_STD_make_crc16_table

/*
 * uint16 *make_crc16_table(uint32 crc_poly)
 *
 * Initialize the CRC table used by the buf_crc() function.
 *
 * Result: NULL if out of memory, pointer to crc table otherwise.
 */

uint16 *make_crc16_table(uint16 * crc_poly_p)
{
  uint16 *crc_table;

  if ((crc_table = malloc(NLIT * sizeof(uint16))) != NULL)
  {
    uint16 poly;
    uint16 kar;

    kar = NLIT - 1;
    poly = *crc_poly_p;

    do
    {
      int i = 7;
      uint16 crc = kar;

      do
      {
        if (crc & 1)
        {
          crc >>= 1;
          crc ^= poly;
        }
        else
        {
          crc >>= 1;
        }
      }
      while (i-- != 0);

      crc_table[kar] = crc;
    }
    while (kar-- != 0);
  }

  return crc_table;
}

#endif

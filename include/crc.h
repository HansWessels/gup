/*
  crc routs
*/

#ifndef __CRC_H__
#define __CRC_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CRC32_POLY 0xEDB88320UL
#define ANSI_CRC16_POLY  0xA001  /* ANSI CRC-16 */
#define CCITT_CRC16_POLY 0x8408  /* CCITT: 0x8408 */

extern uint32 *make_crc32_table(uint32* crc_poly_p);
extern void free_crc32_table(uint32 *table);
extern uint32 arj_init_crc(void);
extern uint32 crc32(const uint8 *str, long len, uint32 crc, const uint32 *crc_table);

extern uint16 *make_crc16_table(uint16* crc_poly_p);
extern void free_crc16_table(uint16 *table);
extern uint16 lha_init_crc(void);
extern uint16 crc16(const uint8 *str, long len, uint16 crc, const uint16 *crc_table);

#ifdef __cplusplus
}

#endif

#endif

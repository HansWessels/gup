/*
** include file for the huffman routines
*/

#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint16_t huffman_t;
typedef uint64_t freq_t;
typedef int symbol_count_t; /* symbol count moet ook negatief kunnen worden */

symbol_count_t get_max_character(freq_t freq[], symbol_count_t symbol_count);
void set_maxlen(symbol_count_t symbol_count, uint8_t len[]); /* zet iedere sybbol len die 0 is op max_len+1 */
void make_hufftable(uint8_t s_len[], huffman_t huff_codes[], const freq_t in_freq[], const symbol_count_t symbol_size,
                    int max_huff_len, int sort_order);

#ifdef __cplusplus
}
#endif

#endif

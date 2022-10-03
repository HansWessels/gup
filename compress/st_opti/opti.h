#ifndef __ST_OPTI_H__
#define __ST_OPTI_H__

#define __ATARI__ /* for atari version */

#define SCREENINIT "\n\033v"

/* Atari Pure C optimalisations */

#ifdef __cplusplus
extern "C"
{
#endif
/* some extra prototypes */
void make_crc32table(uint32 crc_poly, uint32 *table);
void make_crc16table(uint16 crc_poly, uint16 *table);
void make_16bit_crc32table(uint32 crc_poly, uint32 *crc_table);
void make_16bit_crc16table(uint16 crc_poly, uint16 *crc_table);

#if 01
#define NOT_USE_STD_make_crc32_table                 /* build crc table */
#define NOT_USE_STD_free_crc32_table                 /* free table */
#define NOT_USE_STD_crc32                            /* crc routine */
#define NOT_USE_STD_make_crc16_table                 /* build crc table */
#define NOT_USE_STD_free_crc16_table                 /* free table */
#define NOT_USE_STD_crc16                            /* crc routine */
#endif

#if 0
#define NOT_USE_STD_insert2nms
#define NOT_USE_STD_insertnms
#define NOT_USE_STD_insert2nmm
#define NOT_USE_STD_insertnmm
#endif
#if 01
#define NOT_USE_STD_insert
#define NOT_USE_STD_insert2
#define NOT_USE_STD_insertnm
#define NOT_USE_STD_insert2nm
#define NOT_USE_STD_init_dictionary
#define NOT_USE_STD_find_dictionary
#endif
#define NOT_USE_STD_insert_fast
#define NOT_USE_STD_insert2_fast
#define NOT_USE_STD_insertnm_fast
#define NOT_USE_STD_find_dictionary_fast
#if 01
#define NOT_USE_STD_backmatch_big
#endif

#undef INDEX_STRUCT       /* if defined the nodestruct is linked with indices */

#define NOT_USE_STD_init_fast_log                      /* build LOG lookup table */
#define ALIGN_BUFP(x) st_align(x)

void st_align(struct packstruct_t *com);       /* align buffer on word boundary */
void init_fast_log(struct packstruct_t *com);  /* init fastlog prototype */
void init_m4_fast_log(struct packstruct_t *com); /* m4 fastlog */

#define NOT_USE_STD_init_bitbuffer                     /* word wise buffer write */
#define NOT_USE_STD_flush_bitbuf                       /* buffer flushing */

#define ST_BITS(val, count) st_bits(val, count, com)
void st_bits(unsigned long val, int bit_count, struct packstruct_t *com);

#define NOT_USE_STD_make_hufftable                     /* huffman encoder */
#define NOT_USE_STD_make_huffmancodes                  /* huffman encoder */

#ifdef __cplusplus
}
#endif

#endif
  
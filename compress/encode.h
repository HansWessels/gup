/*
 * include file for the compression engine 
 */

#ifndef __ENCODE_H__
#define __ENCODE_H__

#include "gup_err.h"
#include "compress.h"
#include "compr_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BIG_HUFFSIZE (65536UL)           /* memory for ouput file */
#if 01
  #define HUFFSIZE (8192UL+4UL)          /* max size huffman table */
  #define HUFFBUFSIZE (MAX_ENTRIES-EXPANTIE_SLACK + 4UL) /* Grootte huffman buffer in -jm mode */
  #define M4HUFFSIZE (1024UL+4UL)        /* Grootte van huffman buffer in m4 mode */
#else
  #if 0
    #define HUFFSIZE (8192UL+4UL)        /* max size huffman table */
    #define HUFFBUFSIZE (8192UL+4UL)     /* Grootte huffman buffer in -jm mode */
    #define M4HUFFSIZE (8192UL+4UL)      /* Grootte van huffman buffer in m4 mode */
  #else
    #define HUFFSIZE (257L+4UL)            /* max size for testing */
    #define HUFFBUFSIZE (257UL+4UL)        /* Grootte huffman buffer in -jm mode */
    #define M4HUFFSIZE (257UL+4UL)         /* Grootte van huffman buffer in m4 mode */
  #endif
#endif
#define HUFFDELTA 2048                 /* grootte deltablok bij bepaling huffsize */
#define HUFFSTART (2*HUFFDELTA)        /* startgrootte voor bepaling huffsize */

#define NLIT 256                       /* (input) alphabet size */
#define NC (NLIT+MAX_MATCH-MIN_MATCH+1)  /* (output) alphabet size */
#define NCPT 19                        /* aantal c_len coderings pointers */
#define ARJ_NPT 17                     /* arj: max aantal pointers */
#define ARJ_PBIT 5                     /* arj: aantal bits in pointer */
#define LHA_NPT 14                     /* lha: max aantal pointers */
#define LHA_PBIT 4                     /* lha: aantal bits in pointer */
#if LHA_NPT>ARJ_NPT
  #define MAX_NPT  LHA_NPT
#else
  #define MAX_NPT  ARJ_NPT
#endif
#define MIN_ASCII_HEADER (16L+5L+5L+9L+com->m_ptr_bit+com->m_ptr_bit) /* minimum all ascii header */
#define NORMAL_HEADER 0
#define SPECIAL_MIN_ASCII_HEADER 1


/* sld defines */

#define HASH2_SIZE 65536UL
#define HASH_SIZE 65536UL
#define HASH_SIZE32 65536
#define RLE32_DEPTH 32768
#define HASH_SIZE_RLE32 (256U*RLE32_DEPTH)
#define SMALL_HASH2_SIZE 256
#define SMALL_HASH_SIZE 256
#define DICTIONARY_OFFSET 4UL

#if 1
#define DIC_START_SIZE  DIC_SIZE /* Geeft aan in hoeverre de sliding dictionary bij
                                    de start gevuld wordt, moet macht van twee zijn,
                                    DIC_START_SIZE<DIC_SIZE
                                 */
#define DIC_DELTA_SIZE (DIC_SIZE/4) /*- Geeft de grootte van de blokken die in de
                                       sliding dictionary worden nageladen, moet
                                       macht van twee zijn,
                                       DIC_DELTA_SIZE<=(DIC_SIZE/4),
                                       DIC_DELTA_SIZE<=DIC_START_SIZE
                                     */
#else
#define DIC_START_SIZE 8192      /* kleine waarden voor deze variabelen geeft  */
#define DIC_DELTA_SIZE 8192      /* veel output op print_progres() */
#endif


#define HUFF_MAX_SINGLE_FREQ 31        /* Karakter frequenties die direct in tabel verdwijnen, moet 2^n-1 zijn */
#define HUFF_HIGH_FREQS (16-5)         /* 16-LOG(HUFF_MAX_SINGLE_FREQ) */
#define X_CHARS (HUFF_MAX_SINGLE_FREQ+HUFF_HIGH_FREQS) /* Aantal hulp karakters voor de huffman table */
#define MAX_DEFLATE_HUFFLEN 15         /* maximale hufflength deflate is 15 */
#define MAX_HUFFLEN 16                 /* maximale lengte van een huffmancode */
#define MAX_ENTRIES 65534U             /* Maximum aantal entries in huffman blok */
#define EXPANTIE_SLACK (7*256)         /* space subtracted from MAX_ENTRIES in order to allow ZEEF34 expansions */
#define MAX_M4_PTR  15871              /* maximale pointer offset + 1 */
#define MAX_LHA_LZS_PTR 2047           /* max ptr lha_lzs */
#define MAX_LHA_LZ5_PTR 4095           /* max ptr lha_lz5 */
#define MAX_PTR   26623                /* arj dictionary is 26k */
#define PTR_CONV MAX_PTR+1             /* array size voor pointer conversie */
#define MIN_MATCH 3                    /* Minimale match lengte */
#define MAX_MATCH 258         /* maximale match lengte = 258 */
#define MATCH_CONV MAX_MATCH+1         /* array count start bij 0! */
#define DIC_SIZE 2*65536UL
#define DIC_SIZE_MEDIUM 65536UL
#define DIC_SIZE_SMALL 65536UL

/* sld.c definities */

#define TREE_SIZE 65536UL              /* treesize */
#define MAXDELTA  7                    /* maximale afstand tussen twee pointers */

#define M16(x) ((x) & 0xffff)          /* make 16 bits */
#define M17(x) ((x) & 0x1ffffUL)       /* buffer is 128k groot */

#define MAXD_MATCH 9 /* het maximum voor MAXD_MATCH is 16, anders sld.c veranderen */

/*- 
  grootte van buffsize:
  als we een huffmanblok altijd in de buffer kwijt willen is moet buffsize zijn
  64k * 9 bits charsize
  64k * 5 bits pointersize
  64k * 15 bits extra pointer bits
  -------- + som
  64 k * 29 bits = 64k * 4 bytes = 262144
*/

#define FASTLOGBUF 65536UL           /* grootte fastlog buffer */
void init_fast_log(packstruct *com); /* lookuptable for: int LOG(uint16 x){return x?LOG(x/2)+1:0;} */
void init_m4_fast_log(packstruct *com);
void init_n0_fast_log(packstruct *com);
void init_n1_fast_log(packstruct *com);
void init_lzs_fast_log(packstruct *com);
void init_lz5_fast_log(packstruct *com);
#define LOG(x) com->fast_log[x]

int32 first_bit_set32(uint32 u);
gup_result announce(unsigned long bytes, packstruct *com);     /* kondigt aantal bytes in huffblok aan */


gup_result compress_n0(packstruct *com);
gup_result close_n0_stream(packstruct *com);

gup_result compress_n9(packstruct *com);
gup_result close_n9_stream(packstruct *com);

#ifdef __cplusplus
}

#endif

#endif
  

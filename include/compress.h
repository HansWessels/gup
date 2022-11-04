/***************************************************************************
 *                                                                         *
 * Include file voor compress bij gebruik van ARJBETA.C                    *
 *                                                                         *
 * (c) 1994, 1995, 1996 Mr Ni! (the Great)                                 *
 *                                                                         *
 * How to use the encoder in your sources.                                 *
 *                                                                         *
 * The first thing you need is an encode struct. The encode routines are   *
 * fully reentrant. For every pack proces you start a seperate encode      *
 * struct is needed.                                                       *
 *                                                                         *
 * Fill in the following packstruct variables:                             *
 *                                                                         *
 *   int mode: Fill in the pack mode you want to use:                      *
 *             This can be one of the macros defined in this header file.  *
 *             Supported are:                                              *
 *             STORE            store, no compression (arj mode 0, -lh0- ) *
 *             ARJ_MODE_1       arj mode 1..3, 26K dictionary,             *
 *                                  huffman encoding                       *
 *             ARJ_MODE_4       arj mode 4, c.a. 16K dictionary,           *
 *                                  unary encoding                         *
 *             GNU_ARJ_MODE_7   arj mode 7, 64K dictionary,                *
 *                                  huffman encoding, not compatible with  *
 *                                  the original (R. Jung) arj.            *
 *             NI_MODE_x        Ni packer packing methodds are hosted in   *
 *                                 arj archives wits packing modes         *
 *                                 starting at method $10 for mode 0.      *
 *             LHA_LZS_         LHARC mode LZS                             *
 *             LHA_AFX_         LHARC mode afx (same as LZ5)               *
 *             LHA_LZ5_         LHARC mode LZ5                             *
 *             LHA_LH0_         LHARC mode LH0 (store)                     *
 *             LHA_LH4_         LHARC mode LH4                             *
 *             LHA_LH5_         LHARC mode LH5                             *
 *             LHA_LH6_         LHARC mode LH5                             *
 *             GZIP             GZIP deflate                               *
 *                                                                         *
 *   int speed: Fill in the compression speed you want to use:             *
 *             0     normal, best compression                              *
 *             1     fast, less compression more speed                     *
 *             2     fastest, low compression, high speed                  *
 *                                                                         *
 *   int jm:  Use maximum compression within selected mode:                *
 *             0     no maximum compression, faster performance            *
 *             1     maximum compression, iterative packing optimisation   *
 *                                                                         *
 *   void (*init_message)(gup_result warning, void * im_propagator)        *
 *   void* im_propagator:                                                  *
 *            point this to a function for displaying messages from the    *
 *            init_encode() function which has to be called before any     *
 *            packing is to be performed. You _have_ to provide a          *
 *            function reference here as a NULL pointer is illegal!        *
 *            The propagate pointer allows extra parameters to be passed   *
 *            to the init_message function. The pointer is not used by     *
 *            the compression engine itself.                               *
 *            NOTE: this is not verified by the packer so you'd better     *
 *            make sure you've done it right or severe crashes/core        *
 *            dumps will be yours.                                         *
 *                                                                         *
 * Now there is enough information in the packstruct to call the           *
 * function:                                                               *
 *                                                                         *
 *   gup_result init_encode(packstruct *com);                              *
 *                                                                         *
 * This function initializes lots of variables in the packstruct and       *
 * reserves a huge amount of memory needed by the encode engine using      *
 * malloc().                                                               *
 *                                                                         *
 * It is wise to reserve the memory you need for other tasks before you    *
 * call the init_encode() function. The init_encode() function first       *
 * tries to reserve memory for the fastest and best encode method, but     *
 * when there's not enough memory available less memory hungry methods     *
 * are selected. This progressive degradation is announced by messages     *
 * sent to the registered init_message() function. Thus you are able to    *
 * choose the way these messages are displayed.                            *
 *                                                                         *
 * init_encode() will try to allocate about 2 MEGAbytes of heap when       *
 * invoked with mode != 4 or mode != 0. init_encode() will decrease        *
 * memory hunger down to a mere 300 KILObytes of heap when there's less    *
 * heap space available. If even this low amount of heap mem is not        *
 * avialable init_encode() will return an errorcode (ARJ_OUT_OF_MEM).      *
 *                                                                         *
 * When init_encode() is successfull it will return GUP_OK (0) as a        *
 * function result.                                                        *
 *                                                                         *
 * -----                                                                   *
 *                                                                         *
 * Before you can start using the encoder itself you have to fill in the   *
 * following packstruct variables (before/after calling init_encode()):    *
 *                                                                         *
 *   void (*print_progres)(unsigned long blocksize, void* pp_propagator)   *
 *   void* pp_propagator:                                                  *
 *            A reference to a function which displays progress during     *
 *            packing. 'blocksize' contains the number of bytes going to   *
 *            be compressed until the next call to print_progress().       *
 *            The propagagator can be used to pass extra information to    *
 *            the print_progres() function. The pointer is not used by     *
 *            the compression engine itself.                               *
 *                                                                         *
 *   gup_result (*buf_write)(long size, void* data, void* bw_propagator)   *
 *   void* bw_propagator:                                                  *
 *            A reference to the function for writing packed data to a     *
 *            destination file/device/anything                             *
 *            The propagagator can be used to pass extra information to    *
 *            the buf_write() function. The pointer is not used by         *
 *            the compression engine itself.                               *
 *                                                                         *
 *   uint32 (*buf_read_crc)(long size, void *data, void* brc_propagator)   *
 *   void* brc_propagator:                                                 *
 *            A reference to the function to read data to be packed from   *
 *            a file/device/anything (source). If the compression format   *
 *            uses CRC validation of the data, this routine also has to    *
 *            compute the CRC over the read data.                          *
 *            The propagagator can be used to pass extra information to    *
 *            the buf_read_crc() function. The pointer is not used by      *
 *            the compression engine itself.                               *
 *            returns number of bytes read.                                *
 *                                                                         *
 * Some parts of the packstruct have to be set before packing a            *
 * data-stream (file/etc.) every time again, i.e. when compressing         *
 * multiple files consecutively, the following packstruct elements have    *
 * to be set to the proper values before compressing each file in the      *
 * list...                                                                 *
 *                                                                         *
 * For every file to be packed fill in the following variables:            *
 *                                                                         *
 *   int mode:                                                             *
 *            pack mode to be used, 0 (store) or the mode used with        *
 *            initialisation. Change this value at your own risk.          *
 *                                                                         *
 *   int mv_mode:                                                          *
 *            multiple volume mode, 0 is off, 1 is on                      *
 *            NOTE: when the 'mv_mode' variable is set you have to set     *
 *            the following values too:                                    *
 *                                                                         *
 *            unsigned long mv_bytes_left:                                 *
 *                       bytes left untill the multiple-volume break       *
 *                                                                         *
 *            int mv_next:                                                 *
 *                       reset to 0, when a new volume has to be           *
 *                       created. 'mv_next' is set to 1 by the encode()    *
 *                       routine.                                          *
 *                                                                         *
 * You can now call the                                                    *
 *                                                                         *
 *   gup_result encode(packstruct *com);                                   *
 *                                                                         *
 * routine, the data supplied by arj_read() is being compressed and        *
 * written to the target by buf_write().                                   *
 *                                                                         *
 * The encode() routine sets the following packstruct elements as a        *
 * result:                                                                 *
 *                                                                         *
 *   long packed_size:                                                     *
 *            size of the compressed output in bytes (number of bytes      *
 *            processed)                                                   *
 *            NOTE: buf_write() might have written more bytes than this.   *
 *            This nasty behaviour does work with ARJ files as the         *
 *            maximum 'overflow' (= bytes written unneccesarily) is 4      *
 *            bytes: the ARJ archive EOF indicator is exactly 4 bytes!     *
 *            The current implementation uses these routines as follows:   *
 *            1) write archive file header dummy (reserve space by bogus   *
 *               write of correct header size!)                            *
 *            2) call encode() and write compressed data to file           *
 *               including a _possible_ overshoot of 4 bytes!              *
 *            3) seek back to file header and write correct data (size &   *
 *               other info which is only available _after_                *
 *               compression!)                                             *
 *            4) seek forward 'packed_size' bytes and continue with step   *
 *               1) _or_ write 4 byte ARJ EOF mark.                        *
 *            This sequence ensures correct structure of the final         *
 *            archive while intermediate encode() buf_write() results      *
 *            cause overshoot...                                           *
 *                                                                         *
 *   unsigned long bytes_packed:                                           *
 *            number of bytes packed (number of input bytes processed)     *
 *            NOTE: buf_read() might have read more bytes than this        *
 *            number (only in 'multiple volume' mode of compression. The   *
 *            same 'nasty method' remarks as above do apply here ;-)       *
 *                                                                         *
 * In between 2 calls to encode() a part of the encode buffers (in the     *
 * 'com' packstruct) can be used for storing temporary data or as a        *
 * file/data copy buffer. The adres and size of the space available to     *
 * you is supplied by the following function:                              *
 *                                                                         *
 *   uint8 *get_buf(unsigned long *buflen_p, packstruct *com);             *
 *            a NULL pointer is returned when no space was available.      *
 *            Otherwise this function returns a reference to the memory    *
 *            block available for personal use as a function result and    *
 *            the size of the buffer is written to the variable            *
 *            referenced by 'buflen_p'                                     *
 *                                                                         *
 * When you have packed all your data you can free the encode buffers      *
 * using the function:                                                     *
 *                                                                         *
 *   void free_encode(packstruct *com);                                    *
 *            Release all allocated heap memory reserved by                *
 *            init_encode() before.                                        *
 *                                                                         *
 * Special Internals:                                                      *
 *                                                                         *
 * The following entries are for 'hackers' only; use at our own            *
 * risk/peril ;-) These may change as might their behaviour in future      *
 * releases...                                                             *
 *                                                                         *
 *   int max_match:                                                        *
 *            Maximum match size allowed. Must be in the doamin            *
 *            [MIN_MATCH..MAX_MATCH]. May be set/changed anytime!          *
 *            init_encode() does affect this value: R.Jung compliant       *
 *            output requires max_match = [MIN_MATCH..256]!                *
 *                                                                         *
 *   uint8 *buffer:                                                        *
 *            output buffer start position. Used by buf_write(); check     *
 *            the current implementation for proper use.                   *
 *                                                                         *
 *   uint8 *rbuf_tail:                                                     *
 *            pointer past the end of the output buffer. Used by           *
 *            buf_write(); check the current implementation for proper     *
 *            use.                                                         *
 *                                                                         *
 *   uint8 *rbuf_current:                                                  *
 *            pointer to current position within the output buffer. Used   *
 *            by buf_write(); check the current implementation for proper  *
 *            use.                                                         *
 *                                                                         *
 *   gup_result re_crc(unsigned long origsize, packstruct *com);           *
 *            To be used when creating multiple-volume archives: as        *
 *            multiple-volume archives gets more bytes read than can be    *
 *            written to a volume (input buffers are filled to the top!)   *
 *            the CRC calculation done by buf_crc() will provide the       *
 *            _incorrect_ CRC value when a multiple-volume break occurs.   *
 *            To provide the correct CRC value, one should call            *
 *            re_crc().                                                    *
 *            NOTE: this nasty catch has the effect that input streams     *
 *            _cannot_ be compressed to multiple-volume archives as        *
 *            re_crc() _has_ to rewind to the position of the first        *
 *            _source-byte_ which was compressed into the current          *
 *            'volume'; then the data has to be reloaded and the CRC       *
 *            calculated all over again! This implies that the _user_ of   *
 *            the encode() library should keep track of the                *
 *            multiple-volume break positions in terms of 'sourcefile      *
 *            file-position' as the packstruct does _not_ contain any      *
 *            reference to that location (to which re_crc() has to seek    *
 *            back to provide a proper CRC for the current volume)!        *
 *            Multiple-volume construction is there meant for the          *
 *            experienced only (guru level 7 required ;-)                  *
 *                                                                         *
 ***************************************************************************/

#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include "gup.h"
#include "sld32.h"
#include "gup_err.h"
#include "compr_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef int16 c_codetype;   /* type waarin literals en lengths worden opgeslagen */
typedef uint16 pointer_type; /* type waarin de pointers van pointer length in wordt opgeslagen */
struct node_struct_t;

#ifndef INDEX_STRUCT
/*
  struct is linked with pointers
*/
typedef struct node_struct_t* node_type; /* type voor tree nodes */

typedef struct node_struct_t
{
  uint8* key;
  node_type* parent;
  node_type c_left;
  node_type c_right;
} node_struct;

#else
/*
  struct is linked with indices
*/
typedef int32 node_type; /* type voor tree nodes */

typedef struct node_struct_t
{
  int32 key;
  node_type parent;
  node_type c_left;
  node_type c_right;
} node_struct;

#endif


/*
  struct for small systems
*/
#define MS(x) ((x) & (com->andval))

typedef uint16 small_node_type; /* type voor tree nodes */

typedef struct
{
  small_node_type parent;
  small_node_type c_left;
  small_node_type c_right;
  small_node_type link;
} small_node_struct;


/*
  De history array bevat de zoek geschiedenis van een match.
  in deze array zijn alle gevonden matches tot en met max_match 
  opgenomen.
  Hiermee kan een kan een match korter worden gemaakt (door de 
  daaropvolgende match langer te maken) in de hoop een kortere 
  pointer te vinden.
*/

#define HISTSIZE 16          /* aantal history arrays, moet een macht van twee zijn ivm modulo */

typedef struct
{
  c_codetype match;
  pointer_type ptr;
} hist_struct;

typedef hist_struct history[65536 /* MAX_MATCH-MIN_MATCH, 256 rekent makkelijker */];

/* definitions for the pack methods */

#define STORE 0          /* general store */
#define ARJ_MODE_1 1     /* arj mode 1 */
#define ARJ_MODE_2 2     /* arj mode 2 */
#define ARJ_MODE_3 3     /* arj mode 3 */
#define ARJ_MODE_4 4     /* arj mode 4 */
#define GNU_ARJ_MODE_7 7 /* gnu arj mode 7 */
#define NI_MODE_0 0x10   /* ni packer mode 0 */
#define NI_MODE_1 0x11   /* ni packer mode 1 */
#define NI_MODE_2 0x12   /* ni packer mode 2 */
#define NI_MODE_3 0x13   /* ni packer mode 3 */
#define NI_MODE_4 0x14   /* ni packer mode 4 */
#define NI_MODE_5 0x15   /* ni packer mode 5 */
#define NI_MODE_6 0x16   /* ni packer mode 6 */
#define NI_MODE_7 0x17   /* ni packer mode 7 */
#define NI_MODE_8 0x18   /* ni packer mode 8 */
#define NI_MODE_9 0x19   /* ni packer mode 9 */

#define LHA_LHD_ 0x100   /* LHarc directory method */
#define LHA_LZ4_ 0x101   /* no compression */
#define LHA_LZS_ 0x102   /*  2k sliding dictionary(max 17 bytes, min match = 2, not supported by gup) */
#define LHA_LZ5_ 0x103   /*  4k sliding dictionary(max 18 bytes) */
#define LHA_AFX_ 0x104   /* same as -lz5- */
#define LHA_LH0_ 0x105   /* no compression */
#define LHA_LH1_ 0x106   /*  4k sliding dictionary(max 60 bytes) + dynamic Huffman + fixed encoding of position */
#define LHA_LH2_ 0x107   /*  8k sliding dictionary(max 256 bytes) + dynamic Huffman */
#define LHA_LH3_ 0x108   /*  8k sliding dictionary(max 256 bytes) + static Huffman */
#define LHA_LH4_ 0x109   /*  4k sliding dictionary(max 256 bytes) + static Huffman + improved encoding of position and trees */
#define LHA_LH5_ 0x10a   /*  8k sliding dictionary(max 256 bytes) + static Huffman + improved encoding of position and trees */
#define LHA_LH6_ 0x10b   /* 32k sliding dictionary(max 256 bytes) + static Huffman + improved encoding of position and trees */
#define LHA_LH7_ 0x10c   /* 64k sliding dictionary(max 256 bytes) + static Huffman + improved encoding of position and trees */

#define GZIP     0x200   /* GZIP implode method, 32k dictionary, maxmatch = 258 */


#ifndef NOT_USE_STD_packstruct

typedef struct packstruct_t          /* Bij aanpassing van deze struct ook ENCODE.MAC aanpassen */
{
/* encode.c internals */
  uint8 *fast_log;               /* pointer naar logtabel, st_optie gaat er van uit dat 
	                                  dit het eerste element van packstruct is */
/* bit buffer */
  int16 bits_in_bitbuf;          /* # bits in bitbuf */
  unsigned long bitbuf;          /* slaat bits op die in de output file moeten worden gemoved */
/* encode variabelen */
  uint16 n_ptr;                  /* maximum aantal pointers */
  uint16 m_ptr_bit;              /* aantal bits voor een pointer */
  uint16 max_match;              /* maximum match lengte voor gebruikte mode */
  c_codetype* chars;             /* buffer met code karakters */
  c_codetype* charp;             /* pointer naar chars */
  pointer_type* pointers;        /* buffer met pointers */
  pointer_type* ptrp;            /* pointer naar pointers */
  uint8* matchstring;            /* Array waar eerste vier karakters 
                                          van een match in zitten */
  uint8 *msp;                    /* pointer naar matchstring */
  uint8 *backmatch;              /* pointer naar backmatch array */
  uint8 *bmp;                    /* backmatch pointer */
  uint16 special_header;         /* geeft special header aan */

  uint8 *buffer_start;           /* echte start output buffer */
  long buffer_size;              /* size of buffer */
  uint8 *rbuf_start;             /* start output buffer */
  uint8 *rbuf_tail;              /* einde output buffer */
  uint8 *rbuf_current;           /* output buffer */
  uint8 *command_byte_ptr;       /* plek waar commandbyte(s) geschreven moeten worden */

  uint8 *bufbase;                /* base adres van de buffers */
  uint16 hufbufsize;             /* grootte huffman buffers */
/* huffman buffers */
  uint8 *charlen;                /* karakter lengte */
  uint16 *char2huffman;          /* huffman codes van de karakters */
  uint8 *ptrlen;                 /* pointer lengte */
  uint16 *ptr2huffman;           /* huffman codes van de pointers */
  uint8 *ptrlen1;                /* pointer lengte */
  uint16 *ptr2huffman1;          /* huffman codes van de pointers */
/* sliding dictionary variabelen */
  int16 speed;                   /* gewenste pack snelheid */
  union
  {
#ifndef INDEX_STRUCT
    node_type *big;              /* pointer naar link array */
#else
    int32 big;                   /* index offset in array */
#endif
    small_node_type *small;      /* pointer naar link array, small systems */
  }link;
  uint16 rle_size;               /* grootte rle */
  uint16 rle_char;               /* karakter rle */
  uint16 rle_hash;               /* rle hash */
  uint16 last_pos;               /* laatst gedane positie */
  union
  {
#ifndef INDEX_STRUCT
    node_struct *big;            /* sld tree */
#else
    int32 big;                   /* tree offset in array */
#endif
    small_node_struct *small;    /* sld tree small systems */
  }tree;
  uint16 del_pos;                /* wis positie */
  uint8 *dictionary;             /* sliding dictionary */
  uint16 delta_hash;             /* normal hash */
  union
  {
#ifndef INDEX_STRUCT
    node_type *big;              /* normal root */
#else
    int32 big;                   /* root offset in array */
#endif
    small_node_type *small;      /* small root */
  }root;
  int16 hist_index;              /* history index */
  history *hist;                 /* pointer naar history arrays */
  pointer_type best_match_pos;   /* max_match positie in dictionary */
  union
  {
#ifndef INDEX_STRUCT
    node_type *big;              /* rle root */
#else
    int32 big;                   /* root2 offset in array */
#endif
    small_node_type *small;      /* small rle root */
  }root2;
  pointer_type maxptr;           /* maximale afstand binnen de dictionary */
  long tree_size;                /* grootte van sld tree */
  int16 small_code;              /* staat small code aan of uit? */
  uint16 andval;                 /* and value voor small code */
  unsigned long packed_size;     /* file size in bytes */
  int16 bits_rest;               /* number of bits not counted jet */
  int16 use_align;               /* do we use store bits? */
  
  unsigned long bytes_packed;    /* bytes packed in file */
  int16 mode;                    /* pack mode 2B used */
  int16 jm;                      /* -jm mode, 0 is uit, 1 aan  */
  /* multiple volume support variabelen */
  int16 mv_mode;                 /* 0=mv uit */
  int16 mv_next;                 /* 0=geen next volume */
  unsigned long mv_bytes_left;   /* bytes left voor mv break */
  int16 mv_bits_left;            /* tellen we de bits mee die over zijn */
  
  /* some functions we need */
  gup_result (*compress)(struct packstruct_t *com); /* internal use encode */
  
  void (*print_progres)(unsigned long delta_size, void* pp_propagator); /* routine for displaying packing progress */
  void* pp_propagator;
  void (*init_message)(gup_result message, void* im_propagator); /* routine for displaying init messages */
  void* im_propagator;
  void* (*gmalloc)(unsigned long size, void* gm_propagator); /* routine for mallocing encode buffers */
  void* gm_propagator;
  void (*gfree)(void *ptr, void* gf_propagator); /* routine for freeing encode buffers */
  void* gf_propagator;
  gup_result (*buf_write_announce)(long count, buf_fhandle_t* bw_buf, void* bw_propagator); /* routine for writing the arj file */
  buf_fhandle_t* bw_buf;
  void* bw_propagator;
  long (*buf_read_crc)(long count, void *buf, void* brc_propagator); /* read routine for reading the original file */
  void* brc_propagator;
  gup_result (*close_packed_stream)(struct packstruct_t *com); /* output the last bits in bitbuf */
  uint8 *inmem_output; /* buffer, origsize*2+8 bytes in size, after 8 bytes it shares space with the inmem_input buffer */
  uint8 *inmem_output_cur; /* current position in inmem_ouput buffer */
  uint16 *inmem_input; /* buffer for origsize 16 bit words */
  uint16 *inmem_input_cur; /* current position in the inmem_input buffer */
  /* used bij sld32 */
	long origsize; /* size of uncompressed file */
	match_t *match_len; /* match lengte op iedere positie van de file */
	ptr_t *ptr_len; /* bij behorende pointer lengte op iedere positie van de file */
	match_t *backmatch_len; /* back match lengte op iedere positie van de file */
	uint8 *compressed_data; /* de gecomprimeerde data */
	index_t *match_1; /* laatst geziene locatie van een enkele byte -> matchlen=1 */
	index_t *match_2; /* laatst geziene locatie van twee bytes -> matchlen=2 */
	index_t *hash_table; /* 1e locatie voor de hash */
	node_t *tree32; /* dictionary tree */
	cost_t *cost; /* geschatte kosten om tot een bepaalde plek te komen */
	ptr_t maxptr32; /* maximum size of a pointer */
	match_t min_match32; /* minimum match */
	match_t max_match32; /* maximum match */
	match_t match32; /* found match after find dictionary */
	ptr_t ptr32; /* value of the ptr after find_dictionary */
} packstruct;
#endif

gup_result encode(packstruct *com);
gup_result re_crc(unsigned long origsize, packstruct *com);
uint8 *get_buf(unsigned long *buflen, packstruct *com); /* geeft begin adres en lengte van buffer, result is NULL als er geen buffer is */

gup_result init_encode(packstruct *com);
void free_encode(packstruct *com);

/***************************************************************************
 *   int mode: Fill in the pack mode you want to use:                      *
 *             This can be one of the macros defined in this header file.  *
 *             Supported are:                                              *
 *             STORE            store, no compression (arj mode 0, -lh0- ) *
 *             ARJ_MODE_1       arj mode 1..3, 26K dictionary,             *
 *                                  huffman encoding                       *
 *             ARJ_MODE_4       arj mode 4, c.a. 16K dictionary,           *
 *                                  unary encoding                         *
 *             GNU_ARJ_MODE_7   arj mode 7, 64K dictionary,                *
 *                                  huffman encoding, not compatible with  *
 *                                  the original (R. Jung) arj.            *
 *             LHA_LH4_         LHARC mode LH4, warning,this mode is not   *
 *                                  tested, LHA ST does not support -lh4-. *
 *             LHA_LH5_         LHARC mode LH5                             *
 *             GZIP             Gzip implode                               *
 ***************************************************************************/

typedef int16 kartype;

typedef struct
{
  /* internals */
  
  uint8 *buffer;
  uint8 *buffstart;
  kartype *huff2kar;
  uint8 *karlen;
  uint8 *huff2ptr;
  uint8 *ptrlen;         /* moet ff exact bepaald worden */
  
  /* input buffer */
  #if 0  /* not needed yet */
  uint8 *buffer_start;           /* echte start output buffer */
  long buffer_size;              /* size of buffer */
  uint8 *rbuf_start;             /* start output buffer */
  #endif
  
  uint8 *rbuf_tail;              /* einde output buffer */
  uint8 *rbuf_current;           /* output buffer */
  
  uint16 n_ptr;                  /* number of pointers */
  uint16 m_ptr_bit;              /* pointer bits */

  /* things that realy matter */

  int16 mode;                    /* pack mode used */
  long origsize;                 /* orig size */

  void (*print_progres)(unsigned long delta_size, void* pp_propagator);
  void* pp_propagator;
  void* (*gmalloc)(unsigned long size, void* gm_propagator); /* routine for mallocing encode buffers */
  void* gm_propagator;
  void (*gfree)(void *ptr, void* gf_propagator); /* routine for freeing encode buffers */
  void* gf_propagator;
  
  /* function for reading the arj file */
  gup_result (*buf_fill)(buf_fhandle_t* br_buf, void* br_propagator);
  buf_fhandle_t* br_buf;
  void* br_propagator;
  
  /* function for writing the decompressed file */
  gup_result (*write_crc)(long count, void *buf, void* wc_propagator);
  void* wc_propagator;
} decode_struct;

gup_result init_decode(decode_struct *com);
gup_result decode(decode_struct *com);
void free_decode(decode_struct *com);


#ifdef __cplusplus
}

#endif

#endif

 

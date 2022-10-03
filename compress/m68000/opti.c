/*
 * Compression and decompression functions.
 *
 *     Optimized functions for m680x0 systems using the GNU C compiler.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:46:19 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 209 $
 */

#undef  USE_16_BIT_CRC /* Use high speed 16bit crc, 257kB table */


#include <stdlib.h>
#include <stddef.h>
#include <string.h>
/* prototypes */

#include "gup.h"
#include "encode.h"
#include "crc.h"
#include "evaluatr.h"

#ifdef NOT_USE_STD_flush_bitbuf

gup_result flush_bitbuf(packstruct *com)
{
#ifdef TEST_PROTOCOL  
fprintf(f, "s%lu %i\n",val, bit_count);
#endif
  if(com->bits_in_bitbuf>0)
  {
    if ((com->rbuf_current+4) >= com->rbuf_tail)
    {
      gup_result res;
      com->bw_buf->start=com->rbuf_start;
      com->bw_buf->current=com->rbuf_current;
      com->bw_buf->end=com->rbuf_tail;
      res=com->buf_write_announce(4, com->bw_buf, com->bw_propagator);
      com->rbuf_start=com->bw_buf->start;
      com->rbuf_current=com->bw_buf->current;
      com->rbuf_tail=com->bw_buf->end;
      ALIGN_BUFP(com);
      if(res!=GUP_OK)
      {
        return res;
      }
    }
    *((uint32 *) com->rbuf_current)++ = com->bitbuf;
  }
  return GUP_OK;
}

#endif

#ifdef NOT_USE_STD_free_crc32_table

void free_crc32_table(uint32 *table)
{
  if(table!=NULL)
  {
    free(table);
  }
}

#endif

#ifdef NOT_USE_STD_make_crc32_table

#ifdef USE_16_BIT_CRC

/*
 * Ultra high speed crc routs, 16 bits a time 257kB table
 */

uint32 *make_crc32_table(uint32* crc_poly_p)
{
  uint32 *crc_table;
  /*
   * eerste 256 longs zijn de normale crc tabel, 
   * daarna 65536 longs voor Super tabel
   */
  if((crc_table=malloc(257UL*1024UL))==NULL)
  {
    return NULL; /* printf("\n Not enough memory for CRC table!\n") */
  }
  /* memory available, so we can build the table... */
  make_16bit_crc32table(*crc_poly_p, crc_table);
  return crc_table;
}

#else

uint32 *make_crc32_table(uint32* crc_poly_p)
{
  uint32 *crc_table;
  /*
   * eerste 256 longs zijn de normale crc tabel, daarna 256 
   * long voor Chaos tabel 1, dan de BIG Chaos crc tabel 
   */
  if((crc_table=malloc(2UL*((unsigned long)NLIT)*sizeof(unsigned long)+1024))==NULL)
  {
    return NULL; /* printf("\n Not enough memory for CRC table!\n") */
  }
  /* memory available, so we can build the table... */
  make_crc32table(*crc_poly_p, crc_table);
  return crc_table;
}

#endif

#endif

#ifdef NOT_USE_STD_free_crc16_table

void free_crc16_table(uint16 *table)
{
  if(table!=NULL)
  {
    free(table);
  }
}

#endif

#ifdef NOT_USE_STD_make_crc16_table

#ifdef USE_16_BIT_CRC

/*
 * Ultra high speed crc routs, 16 bits a time 257kB table
 */

uint16 *make_crc16_table(uint16* crc_poly_p)
{
  uint16 *crc_table;
  /*
   * eerste 256 words zijn de normale crc tabel, 
   * daarna 65536 words voor Super tabel
   */
  if((crc_table=malloc(257UL*512UL))==NULL)
  {
    return NULL; /* printf("\n Not enough memory for CRC table!\n") */
  }
  /* memory available, so we can build the table... */
  make_16bit_crc16table(*crc_poly_p, crc_table);
  return crc_table;
}

#else

uint16 *make_crc16_table(uint16* crc_poly_p)
{
  uint16 *crc_table;
  /*
   * eerste 256 words zijn de normale crc tabel, 
   * daarna 65536 words voor Super tabel
   */
  if((crc_table=malloc(2UL*((unsigned long)NLIT)*sizeof(uint16)+512))==NULL)
  {
    return NULL; /* printf("\n Not enough memory for CRC table!\n") */
  }
  /* memory available, so we can build the table... */
  make_crc16table(*crc_poly_p, crc_table);
  return crc_table;
}

#endif

#endif

#ifdef NOT_USE_STD_init_fast_log
/* fastlog function is needed in the ST optimized version */
void init_fast_log(packstruct *com)
{
  uint8 *p=com->fast_log;
  int i;
  *p++=0;
  for(i=0;i<16;i++)
  {
    memset(p, i+1, 1UL<<i);
    p+=1UL<<i;
  }
}
#endif


#ifdef NOT_USE_STD_init_dictionary

#define NO_NODE   NULL
#define HASH(x) (((*(x)^(x)[1])<<8) ^ ((x)[1]^(x)[2]))

void init_dictionary(packstruct *com)
{
  /*
   * initialiseert de sliding dictionary initialiseert de
   * slidingdictionary structuren en zet eerste uint8 in dictionary
  */
  /* reset hash table */
  memset(com->root.big, 0, HASH_SIZE * sizeof (node_type));
  if(com->speed<2)
  {
    memset(com->root2.big, 0, HASH_SIZE * sizeof (node_type));
    /* Hier gaan we er dus vanuit dal NULL==0x00000000 */
  }
  if(com->link.big!=NULL)
  {
    memset(com->link.big, 0, com->tree_size * sizeof (node_type));
    /* Hier gaan we er dus vanuit dal NULL==0x00000000 */
  }
  com->last_pos = 2;
  com->del_pos = (uint16) (com->last_pos - com->maxptr - 2);
  if ((com->delta_hash = HASH(com->dictionary)) != 0)
  {
    com->rle_hash = 0;
/*
    insertnm(com->dictionary+2, 2, com->delta_hash);
    void insertnm(key_type nkey, uint16 nnode, uint16 hash)
*/
    {
      node_type tp = com->tree.big + 2;
      tp->c_left=NO_NODE;
      tp->c_right=NO_NODE;
      tp->parent=com->root.big+com->delta_hash;
      tp->key=com->dictionary+2;
      com->root.big[com->delta_hash]=tp;
      com->delta_hash<<=8;
      com->delta_hash+=com->dictionary[2];
    }
  }
  else
  {
    uint8 orig;
    uint8 *p;
    uint8 *new_key=com->dictionary+2;

    p = new_key;
    com->delta_hash = *new_key;
    orig = new_key[com->max_match];
    new_key[com->max_match] = ~com->delta_hash;
    while (*p++ == com->delta_hash)
    {
      ;
    }
    new_key[com->max_match] = orig;
    com->rle_hash = (com->delta_hash << 8) + (int)(p - new_key - 1);
    /*
      insert2nm(new_key+com->rle_size, 2, com->rle_hash);
      void insert2nm(key_type nkey, uint16 nnode, uint16 hash)
    */
    if(com->speed<2)
    {
      node_type tp = com->tree.big + 2;
      tp->c_left=NO_NODE;
      tp->c_right=NO_NODE;
      tp->parent = com->root2.big + com->rle_hash;
      tp->key = new_key+(com->rle_hash & 0xff);
      com->root2.big[com->rle_hash] = tp;
    }
    com->rle_hash--;
  }
}

#endif

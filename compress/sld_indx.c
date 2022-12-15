/*
 * Sliding dictionary code voor ARJ
 * Index linked version
 * 
 * (c) 1998 Hans "Mr Ni! the Great" Wessels
 * 
 */

#undef NDEBUG
#if 01
#define NDEBUG
#endif

#include <string.h>
#include <stddef.h>

#include "gup.h"
#include "encode.h"
#include "evaluatr.h"

#ifdef INDEX_STRUCT

#define STARTMATCH 3 /* Matches van STARTMATCH en kleiner worden niet bekeken op bitafstand */

#define NO_NODE   0

#ifndef NDEBUG

/* debug 'macro's': include lotsa ASSERTs to have'm checked... */
#define DIC_INIT(x) __genece_dic_init(com, (x)+DICTIONARY_OFFSET, "DIC_INIT", #x, __FILE__, __LINE__)
#define NODE_INIT(x) __genece_node_init(com, com->tree.big+(x)*4, "NODE_INIT", #x, __FILE__, __LINE__)
#define ROOT_INIT(x) __genece_root_init(com, com->root.big+(x), "ROOT_INIT", #x, __FILE__, __LINE__)
#define ROOT2_INIT(x) __genece_root2_init(com, com->root2.big+(x), "ROOT2_INIT", #x, __FILE__, __LINE__)

#define KEY_INDEX(x) __genece_key_index(com, x, "KEY_INDEX", #x, __FILE__, __LINE__)
#define PARENT_INDEX(x) __genece_parent_index(com, (x)+1, "PARENT_INDEX", #x, __FILE__, __LINE__)
#define C_LEFT_INDEX(x) __genece_c_left_index(com, (x)+2, "C_LEFT_INDEX", #x, __FILE__, __LINE__)
#define C_RIGHT_INDEX(x) __genece_c_right_index(com, (x)+3, "C_RIGHT_INDEX", #x, __FILE__, __LINE__)
#define LINK_INDEX(x) __genece_link_index(com, com->link.big+(((x)-com->tree.big)>>2), "LINK_INDEX", #x, __FILE__, __LINE__)

#define KEY(x) *__genece_key(com, &base[KEY_INDEX(x)], "KEY", #x, __FILE__, __LINE__)
#define PARENT(x) *__genece_parent(com, &base[PARENT_INDEX(x)], "PARENT", #x, __FILE__, __LINE__)
#define C_LEFT(x) *__genece_c_left(com, &base[C_LEFT_INDEX(x)], "C_LEFT", #x, __FILE__, __LINE__)
#define C_RIGHT(x) *__genece_c_right(com, &base[C_RIGHT_INDEX(x)], "C_RIGHT", #x, __FILE__, __LINE__)
#define LINK(x) *__genece_link(com, &base[LINK_INDEX(x)], "LINK", #x, __FILE__, __LINE__)
#define DIC(x) *__genece_dic(com, &((uint8*)base)[(x)], "DIC", #x, __FILE__, __LINE__)

/* debug functions: assert 'till death... */
#define __GENECE_ASSERT(condition)                                \
  if (!(condition))                                               \
  {                                                               \
    fprintf( stderr                                               \
           , "\nAssertion failed: %s(%s) on condition '%s'\n"     \
             "  [file %s, line %d]\n"                             \
           , macro                                                \
           , msg                                                  \
           , #condition                                           \
           , filename                                             \
           , lineno                                               \
           );                                                     \
    abort();                                                      \
  }

static int32 __genece_dic_init(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= 0);
  __GENECE_ASSERT(x < (DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL));
  NEVER_USE(com);
  return x;
}

static int32 __genece_node_init(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->tree.big);
  __GENECE_ASSERT(x < (com->tree.big+com->tree_size*4));
  return x;
}

static int32 __genece_root_init(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->root.big);
  __GENECE_ASSERT(x < (com->root.big+HASH_SIZE));
  return x;
}

static int32 __genece_root2_init(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->root2.big);
  __GENECE_ASSERT(x < (com->root2.big+HASH2_SIZE));
  return x;
}

static int32 __genece_key_index(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->tree.big);
  __GENECE_ASSERT(x < (com->tree.big+com->tree_size*4));
  __GENECE_ASSERT(((x-com->tree.big)&3)==0);
  return x;
}

static int32 __genece_parent_index(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->tree.big);
  __GENECE_ASSERT(x < (com->tree.big+com->tree_size*4));
  __GENECE_ASSERT(((x-com->tree.big)&3)==1);
  return x;
}

static int32 __genece_c_left_index(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->tree.big);
  __GENECE_ASSERT(x < (com->tree.big+com->tree_size*4));
  __GENECE_ASSERT(((x-com->tree.big)&3)==2);
  return x;
}

static int32 __genece_c_right_index(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->tree.big);
  __GENECE_ASSERT(x < (com->tree.big+com->tree_size*4));
  __GENECE_ASSERT(((x-com->tree.big)&3)==3);
  return x;
}

static int32 __genece_link_index(packstruct *com, int32 x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= com->link.big);
  __GENECE_ASSERT(x < (com->link.big+com->tree_size));
  return x;
}

static int32 *__genece_key(packstruct *com, int32 *x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= (((int32*)com->bufbase)+com->tree.big));
  __GENECE_ASSERT(x < (((int32*)com->bufbase)+(com->tree.big+com->tree_size*4)));
  return x;
}

static int32 *__genece_parent(packstruct *com, int32 *x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= (((int32*)com->bufbase)+com->tree.big));
  __GENECE_ASSERT(x < (((int32*)com->bufbase)+(com->tree.big+com->tree_size*4)));
  return x;
}

static int32 *__genece_c_left(packstruct *com, int32 *x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= (((int32*)com->bufbase)+com->tree.big));
  __GENECE_ASSERT(x < (((int32*)com->bufbase)+(com->tree.big+com->tree_size*4)));
  return x;
}

static int32 *__genece_c_right(packstruct *com, int32 *x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= (((int32*)com->bufbase)+com->tree.big));
  __GENECE_ASSERT(x < (((int32*)com->bufbase)+(com->tree.big+com->tree_size*4)));
  return x;
}

static int32 *__genece_link(packstruct *com, int32 *x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= (((int32*)com->bufbase)+com->link.big));
  __GENECE_ASSERT(x < (((int32*)com->bufbase)+(com->link.big+com->tree_size)));
  return x;
}

static uint8 *__genece_dic(packstruct *com, uint8 *x, const char *macro, const char *msg, const char * filename, int lineno)
{
  __GENECE_ASSERT(x >= (com->bufbase+0));
  __GENECE_ASSERT(x < (com->bufbase+(DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL)));
  return x;
}

#else

#define DIC_INIT(x) (((int32)(x))+DICTIONARY_OFFSET)
#define NODE_INIT(x) (com->tree.big+((int32)(x))*4)
#define ROOT_INIT(x) (com->root.big+((int32)(x)))
#define ROOT2_INIT(x) (com->root2.big+((int32)(x)))

#define KEY_INDEX(x) (x)
#define PARENT_INDEX(x) ((x)+1)
#define C_LEFT_INDEX(x) ((x)+2)
#define C_RIGHT_INDEX(x) ((x)+3)
#define LINK_INDEX(x) (com->link.big+(((x)-com->tree.big)>>2))

#define KEY(x) (base[KEY_INDEX(x)])
#define PARENT(x) (base[PARENT_INDEX(x)])
#define C_LEFT(x) (base[C_LEFT_INDEX(x)])
#define C_RIGHT(x) (base[C_RIGHT_INDEX(x)])
#define LINK(x) (base[LINK_INDEX(x)])
#define DIC(x) (((uint8*)base)[(x)])

#endif

#define HASH(x) (((DIC(x)^DIC((x)+1))<<8) ^ (DIC((x)+1)^DIC((x)+2)))
#define DELTA_HASH(x) ((com->delta_hash<<8) ^ (DIC((x)-1)^DIC(x)))

c_codetype insert2(int32 nkey, node_type pos, node_type hash, uint16 rle_size, packstruct *com);
c_codetype insert(int32 nkey, node_type pos, node_type hash, packstruct *com);
void insert2nm(int32 nkey, node_type pos, node_type hash, packstruct *com);
void insertnm(int32 nkey, node_type pos, node_type hash, packstruct *com);

c_codetype insert2_fast(int32 nkey, uint16 rle_size, packstruct *com);
c_codetype insert_fast(int32 nkey, node_type pos, node_type hash, packstruct *com);
void insertnm_fast(int32 nkey, node_type pos, node_type hash, packstruct *com);


/*
 * struct is linked with int32's
*/

#ifndef NOT_USE_STD_init_dictionary
void init_dictionary(packstruct *com)
{
  /*
   * initialiseert de sliding dictionary initialiseert de
   * slidingdictionary structuren en zet eerste byte in dictionary
  */

  int32* base = (node_type*)com->bufbase;

  /* reset hash table */
  memset(base+com->root.big, 0, HASH_SIZE * sizeof (node_type));
  if(com->speed<2)
  {
    memset(base+com->root2.big, 0, HASH_SIZE * sizeof (node_type));
    memset(base+com->link.big, 0, com->tree_size * sizeof (node_type));
    com->hist_index=0;
  }
  com->last_pos = 2;
  com->del_pos = (uint16) (com->last_pos - com->maxptr - 1);
  if ((com->delta_hash = HASH(DIC_INIT(0))) != 0)
  {
    com->rle_size = 0;
    insertnm(DIC_INIT(2), NODE_INIT(2), ROOT_INIT(com->delta_hash), com);
  }
  else
  {
    uint8 orig;
    int32 new_key=DIC_INIT(2);
    int32 p = new_key;
    com->rle_char = DIC(new_key);
    orig = DIC(new_key+com->max_match);
    DIC(new_key+com->max_match) = ~(uint8)com->rle_char;
    while (DIC(p++) == com->rle_char)
    {
      ;
    }
    com->rle_size = (uint16)(p - new_key - 1);
    DIC(new_key+com->max_match) = orig;
    com->rle_hash = (com->rle_char << 8) + com->rle_size;
    if(com->speed<2)
    {
      insert2nm(new_key+com->rle_size, NODE_INIT(2), ROOT2_INIT(com->rle_hash), com);
    }
    if (DIC(new_key+com->rle_size) != com->rle_char)
    {
      com->rle_size--;
      com->rle_hash--;
    }
  }
}

#endif


#ifndef NOT_USE_STD_find_dictionary

c_codetype find_dictionary(long insertpos, packstruct *com)
{
  /*
   * zoekt maximale match in dictionary bij pos update de
   * slidingdictionary structuren returnt max_match, als geen max_match
   * gevonden dan return 0 best_match_pos is de absolute positie van
   * max_match
  */
  int32* base = (node_type*)com->bufbase;
  uint16 pos=(uint16)insertpos;
  uint16 to_do = pos-com->last_pos;
  int32 new_key = DIC_INIT(insertpos-to_do);
  ARJ_Assert(((pos-com->last_pos)&0xffff)<=MAX_MATCH);
  { /*- wis de nodes */
    int i = to_do;
    uint16 key = com->del_pos;
    do
    {
      node_type tp;

      tp = PARENT(NODE_INIT(key));
      if (tp != NO_NODE)
      {
        if (base[tp] == NODE_INIT(key))
        {
          base[tp] = NO_NODE;
        }
      }
      key++;
    }
    while (--i > 0);
    com->del_pos = key;
  }
  com->last_pos = pos;
  pos -= to_do;
  while (--to_do > 0)
  {
    new_key++;
    pos++;
    if (com->rle_size > 0)
    {
      insert2nm(new_key+com->rle_size, NODE_INIT(pos), ROOT2_INIT(com->rle_hash), com);
      if (DIC(new_key+com->rle_size) != com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        insertnm(new_key, NODE_INIT(pos), ROOT_INIT(com->delta_hash), com);
      }
      else
      {
        uint8 orig;
        int32 p = new_key;
        com->rle_char = DIC(new_key);
        orig = DIC(new_key+com->max_match);
        DIC(new_key+com->max_match) = (uint8)~com->rle_char;
        while (DIC(p++) == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 1);
        DIC(new_key+com->max_match) = orig;
        com->rle_hash = (com->rle_char << 8) + com->rle_size;
        insert2nm(new_key+com->rle_size, NODE_INIT(pos), ROOT2_INIT(com->rle_hash), com);
        if (DIC(new_key+com->rle_size) != com->rle_char)
        {
          com->rle_size--;
          com->rle_hash--;
        }
      }
    }
  }
  pos++;
  new_key++;
  {
    uint8 orig = DIC(new_key+com->max_match);
    c_codetype res;
    if (com->rle_size > 0)
    {
      res=insert2(new_key, NODE_INIT(pos), ROOT2_INIT(com->rle_hash), com->rle_size, com);
      DIC(new_key+com->max_match) = orig;
      if (DIC(new_key+com->rle_size) != com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        res=insert(new_key, NODE_INIT(pos), ROOT_INIT(com->delta_hash), com);
        DIC(new_key+com->max_match) = orig;
      }
      else
      {
        int32 p;
        p = new_key;
        com->rle_char = DIC(new_key);
        DIC(new_key+com->max_match) = (uint8)~com->rle_char;
        while (DIC(p++) == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 1);
        com->rle_hash = (com->rle_char << 8) + com->rle_size;
        res=insert2(new_key, NODE_INIT(pos), ROOT2_INIT(com->rle_hash), com->rle_size, com);
        DIC(new_key+com->max_match) = orig;
        if (DIC(new_key+com->rle_size) != com->rle_char)
        {
          com->rle_size--;
          com->rle_hash--;
        }
      }
    }
    #if !defined(NDEBUG) || 0
      if(res>(com->max_match+2))
      {
        printf("\n%i\n",res);
      }
      ARJ_Assert(res<=(com->max_match+2));
    #endif
    return res;
  }
}

#endif


#ifndef NOT_USE_STD_insert2nm

void insert2nm(int32 nkey, node_type nnode, node_type hash, packstruct *com)
{
  int32* base = (node_type*)com->bufbase;
  node_type tp = nnode;
  node_type left = C_LEFT_INDEX(tp);
  node_type right = C_RIGHT_INDEX(tp);
  PARENT(tp) = hash;
  KEY(tp) = nkey;
  {
    node_type tmp = base[hash];
    base[hash] = tp;
    if(tmp==NO_NODE)
    {
      base[left] = NO_NODE;
      base[right] = NO_NODE;
      return;
    }
    tp = tmp;
  }
  for(;;)
  {
    int32 p = KEY(tp);
    int32 q = nkey;
    {
      if (DIC(p++) == DIC(q++))
      #if MAXD_MATCH>1
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>2
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>3
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>4
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>5
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>6
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>7
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>8
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>9
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>10
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>11
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>12
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>13
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>14
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>15
      if (DIC(p++) == DIC(q++))
      #endif
      { /*- we hebben een 100% match */
        KEY(tp)+=MAXD_MATCH;
        base[left] = C_LEFT(tp);
        if (C_LEFT(tp) != NO_NODE)
        {
          PARENT(C_LEFT(tp)) = left;
        }
        base[right] = C_RIGHT(tp);
        if (C_RIGHT(tp) != NO_NODE)
        {
          PARENT(C_RIGHT(tp)) = right;
        }
        LINK(nnode) = tp;
        PARENT(tp) = LINK_INDEX(nnode);
        return;
      }
    }
    if(DIC(q-1)<DIC(p-1))
    {
      /*
       * de key van nnode is kleiner dan die van tp, tp moet dus rechts
       * van nnode komen tp node wordt dus de linker child van rechts. de
       * nieuwe tp node wordt de linker child van tp
      */
      base[right] = tp;
      PARENT(tp) = right;
      right = C_LEFT_INDEX(tp);
      if((tp = base[right])==NO_NODE)
      {
        base[left]=NO_NODE;
        return;
      }
    }
    else
    {
      base[left] = tp;
      PARENT(tp) = left;
      left = C_RIGHT_INDEX(tp);
      if((tp = base[left])==NO_NODE)
      {
        base[right]=NO_NODE;
        return;
      }
    }
  }
}

#endif


#ifndef NOT_USE_STD_insertnm

void insertnm(int32 nkey, node_type nnode, node_type hash, packstruct *com)
{
  int32* base = (node_type*)com->bufbase;
  node_type tp = nnode;
  node_type left = C_LEFT_INDEX(tp);
  node_type right = C_RIGHT_INDEX(tp);
  PARENT(tp) = hash;
  KEY(tp) = nkey;
  {
    node_type tmp;
    tmp = base[hash];
    base[hash] = tp;
    if(tmp==NO_NODE)
    {
      base[left]=NO_NODE;
      base[right]=NO_NODE;
      return;
    }
    tp=tmp;
  }
  for(;;)
  {
    int32 p = KEY(tp);
    int32 q = nkey;
    {
      if (DIC(p++) == DIC(q++))
      #if MAXD_MATCH>1
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>2
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>3
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>4
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>5
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>6
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>7
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>8
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>9
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>10
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>11
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>12
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>13
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>14
      if (DIC(p++) == DIC(q++))
      #endif
      #if MAXD_MATCH>15
      if (DIC(p++) == DIC(q++))
      #endif
      { /*- we hebben een 100% match */
        base[left] = C_LEFT(tp);
        if (C_LEFT(tp) != NO_NODE)
        {
          PARENT(C_LEFT(tp)) = left;
        }
        base[right] = C_RIGHT(tp);
        if (C_RIGHT(tp) != NO_NODE)
        {
          PARENT(C_RIGHT(tp)) = right;
        }
        LINK(nnode) = tp;
        KEY(tp)+=MAXD_MATCH;
        PARENT(tp) = LINK_INDEX(nnode);
        return;
      }
    }
    if(DIC(q-1)<DIC(p-1))
    {
      /*
       * de key van nnode is kleiner dan die van tp, tp moet dus rechts
       * van nnode komen tp node wordt dus de linker child van rechts. de
       * nieuwe tp node wordt de linker child van tp
      */
      base[right] = tp;
      PARENT(tp) = right;
      right = C_LEFT_INDEX(tp);
      if((tp = base[right])==NO_NODE)
      {
        base[left]=NO_NODE;
        break;
      }
    }
    else
    {
      base[left] = tp;
      PARENT(tp) = left;
      left = C_RIGHT_INDEX(tp);
      if((tp = base[left])==NO_NODE)
      {
        base[right]=NO_NODE;
        break;
      }
    }
  }
}

#endif


#ifndef NOT_USE_STD_insert2

c_codetype insert2(int32 nkey, node_type nnode, node_type hash, uint16 rle_size, packstruct *com)
{
  int32* base = (node_type*)com->bufbase;
  c_codetype best_match;
  c_codetype opt_match;
  uint16 max_match=com->max_match;
  hist_struct *hist;
  node_type tp = nnode;
  node_type left = C_LEFT_INDEX(tp);
  node_type right = C_RIGHT_INDEX(tp);
  PARENT(tp) = hash;
  com->hist_index=((com->hist_index+1) & (HISTSIZE-1));
  hist=com->hist[com->hist_index];
  if (DIC(nkey-3) == DIC(nkey))
  {
    best_match=0;
    opt_match=2;
    com->best_match_pos = 0;
    hist->match=rle_size;
    hist->ptr=0;
    hist++;
  }
  else
  {
    best_match=-4;
    opt_match=-2;
  }
  nkey+=rle_size;
  KEY(tp) = nkey;
  {
    node_type tmp = base[hash];
    base[hash] = tp;
    tp = tmp;
  }
  if(tp==NO_NODE)
  {
    if(best_match<0)
    {
      opt_match=0;
    }
    else
    {
      opt_match=rle_size+2;
    }
    base[left] = NO_NODE;
    base[right] = NO_NODE;
  }
  else
  {
    int32 origkey=nkey;
    max_match-=rle_size;
    for(;;)
    {
      c_codetype match;
      int32 p = KEY(tp);
      DIC(nkey+max_match) = ~DIC(p+max_match);  /* sentinel */
      do
      {
        if (DIC(p++) != DIC(nkey++))
          break;
        if (DIC(p++) != DIC(nkey++))
          break;
        if (DIC(p++) != DIC(nkey++))
          break;
        if (DIC(p++) != DIC(nkey++))
          break;
      }
      while (DIC(p++) == DIC(nkey++));
      nkey--;
      match = (c_codetype)(nkey - origkey);
      if (match > best_match)
      {
        best_match = match;
        {
          uint16 pos = (uint16)(nkey - p);
          hist->match = match+rle_size;
          hist->ptr=pos;
          hist++;
          if ((match -= opt_match) <= 0)
          {
            if (match < 0)
            {
              if (((LOG(pos) - LOG(com->best_match_pos)) < MAXDELTA))
              {
                opt_match = best_match+2;
                com->best_match_pos = pos;
              }
            }
            else
            {
              if (((LOG(pos) - LOG(com->best_match_pos)) < (MAXDELTA * 2)))
              {
                opt_match = best_match+2;
                com->best_match_pos = pos;
              }
            }
          }
          else
          {
            opt_match = best_match+2;
            com->best_match_pos = pos;
          }
        }
        if (best_match >= MAXD_MATCH)
        { /*- we hebben een 100% match */
          base[left] = C_LEFT(tp);
          if (C_LEFT(tp) != NO_NODE)
          {
            PARENT(C_LEFT(tp)) = left;
          }
          base[right] = C_RIGHT(tp);
          if (C_RIGHT(tp) != NO_NODE)
          {
            PARENT(C_RIGHT(tp)) = right;
          }
          {
            if (best_match == max_match)
            { /*- link met de link van deze node */
              LINK(nnode) = LINK(tp);
              if (LINK(tp) != NO_NODE)
              {
                PARENT(LINK(tp)) = LINK_INDEX(nnode);
                LINK(tp) = NO_NODE;
              }
            }
            else
            {
              max_match-=MAXD_MATCH;
              origkey+=MAXD_MATCH;
              best_match-=MAXD_MATCH;
              opt_match-=MAXD_MATCH;
              LINK(nnode) = tp;
              KEY(tp)+=MAXD_MATCH;
              PARENT(tp) = LINK_INDEX(nnode);
              tp = LINK(tp);
              while (tp != NO_NODE)
              {
                {
                  p = KEY(tp);
                  nkey=origkey;
                  DIC(nkey+max_match) = ~DIC(p+max_match);  /* sentinel */
                  do
                  {
                    if (DIC(p++) != DIC(nkey++))
                      break;
                    if (DIC(p++) != DIC(nkey++))
                      break;
                    if (DIC(p++) != DIC(nkey++))
                      break;
                    if (DIC(p++) != DIC(nkey++))
                      break;
                  }
                  while (DIC(p++) == DIC(nkey++));
                  nkey--;
                  match = (c_codetype)(nkey - origkey);
                }
                if (match > best_match)
                {
                  best_match = match;
                  {
                    uint16 pos = (uint16)(nkey - p);
                    hist->match=match+rle_size+MAXD_MATCH;
                    hist->ptr=pos;
                    hist++;
                    if ((match -= opt_match) <= 0)
                    {
                      if (match < 0)
                      {
                        if ((LOG(pos) - LOG(com->best_match_pos)) < MAXDELTA)
                        {
                          opt_match = best_match+2;
                          com->best_match_pos = pos;
                        }
                      }
                      else
                      {
                        if ((LOG(pos) - LOG(com->best_match_pos)) < (MAXDELTA * 2))
                        {
                          opt_match = best_match+2;
                          com->best_match_pos = pos;
                        }
                      }
                    }
                    else
                    {
                      opt_match = best_match+2;
                      com->best_match_pos = pos;
                    }
                  }
                  if (best_match == max_match)
                  {
                    break;
                  }
                }
                tp = LINK(tp);
              }
              opt_match+=MAXD_MATCH;
            }
          }
          #if !defined(NDEBUG) || 0
            if((opt_match+rle_size)>(com->max_match+2))
            {
              printf("\n%i\n",opt_match+rle_size);
            }
            ARJ_Assert((opt_match+rle_size)<=(com->max_match+2));
          #endif
          return opt_match+rle_size;
        }
        else
        {
          if (best_match == max_match)
          { /*- we hebben een 100% match */
            base[left] = C_LEFT(tp);
            if (C_LEFT(tp) != NO_NODE)
            {
              PARENT(C_LEFT(tp)) = left;
            }
            base[right] = C_RIGHT(tp);
            if (C_RIGHT(tp) != NO_NODE)
            {
              PARENT(C_RIGHT(tp)) = right;
            }
            #if !defined(NDEBUG) || 0
              if((opt_match+rle_size)>(com->max_match+2))
              {
                printf("\n%i\n",opt_match+rle_size);
              }
              ARJ_Assert((opt_match+rle_size)<=(com->max_match+2));
            #endif
            return opt_match+rle_size;
          }
        }
      }
      if(DIC(nkey) < DIC(p-1))
      {
        /*
         * de key van nnode is kleiner dan die van tp, tp moet dus rechts
         * van nnode komen tp node wordt dus de linker child van rechts. de
         * nieuwe tp node wordt de linker child van tp
        */
        base[right] = tp;
        PARENT(tp) = right;
        right = C_LEFT_INDEX(tp);
        if((tp = base[right])==NO_NODE)
        {
          base[left]=NO_NODE;
          break;
        }
      }
      else
      {
        base[left] = tp;
        PARENT(tp) = left;
        left = C_RIGHT_INDEX(tp);
        if((tp = base[left])==NO_NODE)
        {
          base[right]=NO_NODE;
          break;
        }
      }
      nkey=origkey;
    }
    opt_match+=rle_size;
  }
  #if !defined(NDEBUG) || 0
    if(opt_match>(com->max_match+2))
    {
      printf("\n%i\n",opt_match);
    }
    ARJ_Assert(opt_match<=(com->max_match+2));
  #endif
  return opt_match;
}

#endif


#ifndef NOT_USE_STD_insert

c_codetype insert(int32 nkey, node_type nnode, node_type hash, packstruct *com)
{
  int32* base = (node_type*)com->bufbase;
  uint16 max_match=com->max_match-1;
  node_type tp = nnode;
  node_type left = C_LEFT_INDEX(tp);
  node_type right = C_RIGHT_INDEX(tp);
  PARENT(tp) = hash;
  KEY(tp) = nkey;
  {
    node_type tmp = base[hash];
    base[hash] = tp;
    tp = tmp;
  }
  if(tp==NO_NODE)
  {
    base[left] = NO_NODE;
    base[right] = NO_NODE;
    return 0;
  }
  else
  {
    c_codetype best_match=0;
    c_codetype opt_match=0;
    int32 origpos=nkey;
    hist_struct *hist;
    com->hist_index=((com->hist_index+1) & (HISTSIZE-1));
    hist=com->hist[com->hist_index];
    for(;;)
    {
      c_codetype match;
      {
        int32 p = KEY(tp);
        if (DIC(p++) != DIC(nkey++))
        {
          if(DIC(--nkey)<DIC(p-1))
          {
            /*
             * de key van nnode is kleiner dan die van tp, tp moet dus rechts
             * van nnode komen tp node wordt dus de linker child van rechts. de
             * nieuwe tp node wordt de linker child van tp
            */
            base[right] = tp;
            PARENT(tp) = right;
            right = C_LEFT_INDEX(tp);
            if((tp = base[right])==NO_NODE)
            {
              base[left]=NO_NODE;
              break;
            }
          }
          else
          {
            base[left] = tp;
            PARENT(tp) = left;
            left = C_RIGHT_INDEX(tp);
            if((tp = base[left])==NO_NODE)
            {
              base[right]=NO_NODE;
              break;
            }
          }
        }
        else
        {
          DIC(nkey+max_match) = ~DIC(p+max_match);  /* sentinel */
          do
          {
            if (DIC(p++) != DIC(nkey++))
              break;
            if (DIC(p++) != DIC(nkey++))
              break;
            if (DIC(p++) != DIC(nkey++))
              break;
            if (DIC(p++) != DIC(nkey++))
              break;
          }
          while (DIC(p++) == DIC(nkey++));
          nkey--;
          match = (c_codetype)(nkey-origpos);
          if (match > best_match)
          {
            best_match = match;
            {
              uint16 pos = (uint16)(nkey - p);
              hist->match=match;
              hist->ptr=pos;
              hist++;
              if ((match -= opt_match) <= 0)
              {
                if (match < 0)
                {
                  if (((LOG(pos) - LOG(com->best_match_pos)) < MAXDELTA))
                  {
                    opt_match = best_match+2;
                    com->best_match_pos = pos;
                  }
                }
                else
                {
                  if (((LOG(pos) - LOG(com->best_match_pos)) < (MAXDELTA * 2)))
                  {
                    opt_match = best_match+2;
                    com->best_match_pos = pos;
                  }
                }
              }
              else
              {
                opt_match = best_match+2;
                com->best_match_pos = pos;
              }
            }
            if (best_match >= (MAXD_MATCH))
            { /*- we hebben een 100% match */
              base[left] = C_LEFT(tp);
              if (C_LEFT(tp) != NO_NODE)
              {
                PARENT(C_LEFT(tp)) = left;
              }
              base[right] = C_RIGHT(tp);
              if (C_RIGHT(tp) != NO_NODE)
              {
                PARENT(C_RIGHT(tp)) = right;
              }
              {
                if (best_match == com->max_match)
                { /* link met de link van deze node */
                  LINK(nnode) = LINK(tp);
                  if (LINK(tp) != NO_NODE)
                  {
                    PARENT(LINK(tp)) = LINK_INDEX(nnode);
                    LINK(tp) = NO_NODE;
                  }
                }
                else
                {
                  max_match-=MAXD_MATCH-1;
                  origpos+=MAXD_MATCH;
                  best_match-=MAXD_MATCH;
                  opt_match-=MAXD_MATCH;
                  LINK(nnode) = tp;
                  KEY(tp)+=MAXD_MATCH;
                  PARENT(tp) = LINK_INDEX(nnode);
                  tp = LINK(tp);
                  while (tp != NO_NODE)
                  {
                    {
                      p=KEY(tp);
                      nkey=origpos;
                      DIC(nkey+max_match) = ~DIC(p+max_match);  /* sentinel */
                      do
                      {
                        if (DIC(p++) != DIC(nkey++))
                          break;
                        if (DIC(p++) != DIC(nkey++))
                          break;
                        if (DIC(p++) != DIC(nkey++))
                          break;
                        if (DIC(p++) != DIC(nkey++))
                          break;
                      }
                      while (DIC(p++) == DIC(nkey++));
                      nkey--;
                      match = (c_codetype)(nkey-origpos);
                    }
                    if (match > best_match)
                    {
                      best_match = match;
                      {
                        uint16 pos = (uint16)(nkey - p);
                        hist->match=match+MAXD_MATCH;
                        hist->ptr=pos;
                        hist++;
                        if ((match -= opt_match) <= 0)
                        {
                          if (match < 0)
                          {
                            if ((LOG(pos) - LOG(com->best_match_pos)) < MAXDELTA)
                            {
                              opt_match = best_match+2;
                              com->best_match_pos = pos;
                            }
                          }
                          else
                          {
                            if ((LOG(pos) - LOG(com->best_match_pos)) < (MAXDELTA * 2))
                            {
                              opt_match = best_match+2;
                              com->best_match_pos = pos;
                            }
                          }
                        }
                        else
                        {
                          opt_match = best_match+2;
                          com->best_match_pos = pos;
                        }
                      }
                      if (best_match == max_match)
                      {
                        break;
                      }
                    }
                    tp = LINK(tp);
                  }
                  opt_match+=MAXD_MATCH;
                }
              }
              #if !defined(NDEBUG) || 0
                if(opt_match>(com->max_match+2))
                {
                  printf("\n%i\n",opt_match);
                }
                ARJ_Assert(opt_match<=(com->max_match+2));
              #endif
              return opt_match;
            }
          }
          if(DIC(nkey)<DIC(p-1))
          {
            /*
             * de key van nnode is kleiner dan die van tp, tp moet dus rechts
             * van nnode komen tp node wordt dus de linker child van rechts. de
             * nieuwe tp node wordt de linker child van tp
            */
            base[right] = tp;
            PARENT(tp) = right;
            right = C_LEFT_INDEX(tp);
            if((tp = base[right])==NO_NODE)
            {
              base[left]=NO_NODE;
              break;
            }
          }
          else
          {
            base[left] = tp;
            PARENT(tp) = left;
            left = C_RIGHT_INDEX(tp);
            if((tp = base[left])==NO_NODE)
            {
              base[right]=NO_NODE;
              break;
            }
          }
          nkey=origpos;
        }
      }
    }
    #if !defined(NDEBUG) || 0
      if(opt_match>(com->max_match+2))
      {
        printf("\n%i\n",opt_match);
      }
      ARJ_Assert(opt_match<=(com->max_match+2));
    #endif
    return opt_match;
  }
}

#endif


/**************************************************************************
 * fast dictionary routines, use first match found, use only rle matches  *
 **************************************************************************/


#ifndef NOT_USE_STD_find_dictionary_fast

c_codetype find_dictionary_fast(long insertpos, packstruct *com)
{
  /*
   * zoekt eerste match in dictionary bij pos update de
   * slidingdictionary structuren returnt max_match, als geen match
   * gevonden dan return 0 best_match_pos is de absolute positie van
   * match
  */
  int32* base = (node_type*)com->bufbase;
  uint16 pos=(uint16)insertpos;
  uint16 to_do = pos-com->last_pos;
  int32 new_key = DIC_INIT(insertpos-to_do);
  ARJ_Assert(((pos-com->last_pos)&0xffff)<=MAX_MATCH);
  { /* wis de nodes */
    /*
     * ik ga er van uit dat de te deleten nodes altijd laatste nodes
     * zijn anders worden gewoon takken verwijderd, dat mag ook
    */
    int i = to_do;
    uint16 key = com->del_pos;
    do
    {
      node_type tp;
      tp = PARENT(NODE_INIT(key));
      if (tp != NO_NODE)
      {
        if (base[tp] == NODE_INIT(key))
        {
          base[tp] = NO_NODE;
        }
      }
      key++;
    }
    while (--i > 0);
    com->del_pos = key;
  }
  com->last_pos = pos;
  pos -= to_do;
  while (--to_do > 0)
  {
    new_key++;
    pos++;
    if (com->rle_size > 0)
    {
      if (DIC(new_key+com->rle_size) != com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        insertnm_fast(new_key, NODE_INIT(pos), ROOT_INIT(com->delta_hash), com);
      }
      else
      {
        uint8 orig;
        int32 p;
        p = new_key;
        com->rle_char = DIC(new_key);
        orig = DIC(new_key+com->max_match);
        DIC(new_key+com->max_match) = (uint8)~com->rle_char;
        while (DIC(p++) == com->rle_char)
        {
          ;
        }
        DIC(new_key+com->max_match) = orig;
        com->rle_size = (uint16)(p - new_key - 1);
        if (DIC(new_key+com->rle_size) != com->rle_char)
        {
          com->rle_size--;
        }
      }
    }
  }
  pos++;
  new_key++;
  {
    uint8 orig = DIC(new_key+com->max_match);
    c_codetype res;
    if (com->rle_size > 0)
    {
      if (DIC(new_key-3) == DIC(new_key))
      {
        com->best_match_pos = 0;
        res=com->rle_size+2;
      }
      else
      {
        res= 0;
      }
      if (DIC(new_key+com->rle_size) != com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        res=insert_fast(new_key, NODE_INIT(pos), ROOT_INIT(com->delta_hash), com);
        DIC(new_key+com->max_match) = orig;
      }
      else
      {
        int32 p;
        p = new_key;
        com->rle_char = DIC(new_key);
        DIC(new_key+com->max_match) = (uint8)~com->rle_char;
        while (DIC(p++) == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 1);
        DIC(new_key+com->max_match) = orig;
        if (DIC(new_key+com->rle_size) != com->rle_char)
        {
          com->rle_size--;
        }
        res=0;
      }
    }
    return res;
  }
}

#endif

#ifndef NOT_USE_STD_insertnm_fast

void insertnm_fast(int32 nkey, node_type nnode, node_type hash, packstruct *com)
{
  int32* base = (node_type*)com->bufbase;
  node_type tp = nnode;
  node_type left = C_LEFT_INDEX(tp);
  node_type right = C_RIGHT_INDEX(tp);
  PARENT(tp) = hash;
  KEY(tp) = nkey;
  {
    node_type tmp;
    tmp = base[hash];
    base[hash] = tp;
    if(tmp==NO_NODE)
    {
      base[left]=NO_NODE;
      base[right]=NO_NODE;
      return;
    }
    tp = tmp;
  }
  {
    uint8 first=DIC(nkey);
    for(;;)
    {
      if(first<DIC(KEY(tp)))
      {
        /*
         * de key van nnode is kleiner dan die van tp, tp moet dus rechts
         * van nnode komen tp node wordt dus de linker child van rechts. de
         * nieuwe tp node wordt de linker child van tp
        */
        base[right] = tp;
        PARENT(tp) = right;
        right = C_LEFT_INDEX(tp);
        if((tp = base[right])==NO_NODE)
        {
          base[left]=NO_NODE;
          break;
        }
      }
      else
      {
        if((first)>(DIC(KEY(tp))))
        {
          base[left] = tp;
          PARENT(tp) = left;
          left = C_RIGHT_INDEX(tp);
          if((tp = base[left])==NO_NODE)
          {
            base[right]=NO_NODE;
            break;
          }
        }
        else
        { /*- we hebben een 100% match */
          base[left] = C_LEFT(tp);
          if (C_LEFT(tp) != NO_NODE)
          {
            PARENT(C_LEFT(tp)) = left;
          }
          base[right] = C_RIGHT(tp);
          if (C_RIGHT(tp) != NO_NODE)
          {
            PARENT(C_RIGHT(tp)) = right;
          }
          return;
        }
      }
    }
  }
}

#endif


#ifndef NOT_USE_STD_insert_fast

c_codetype insert_fast(int32 nkey, node_type nnode, node_type hash, packstruct *com)
{
  int32* base = (node_type*)com->bufbase;
  node_type tp = nnode;
  node_type left = C_LEFT_INDEX(tp);
  node_type right = C_RIGHT_INDEX(tp);
  PARENT(tp) = hash;
  KEY(tp) = nkey;
  {
    node_type tmp = base[hash];
    base[hash] = tp;
    tp = tmp;
  }
  if(tp==NO_NODE)
  {
    base[left] = NO_NODE;
    base[right] = NO_NODE;
    return 0;
  }
  else
  {
    uint8 first=DIC(nkey++);
    for(;;)
    {
      int32 p = KEY(tp);
      if(first != DIC(p++))
      {
        if (first < DIC(p-1))
        {
          /*
           * de key van nnode is kleiner dan die van tp, tp moet dus rechts
           * van nnode komen tp node wordt dus de linker child van rechts. de
           * nieuwe tp node wordt de linker child van tp
          */
          base[right] = tp;
          PARENT(tp) = right;
          right = C_LEFT_INDEX(tp);
          if((tp = base[right])==NO_NODE)
          {
            base[left]=NO_NODE;
            return 0;
          }
        }
        else
        {
          base[left] = tp;
          PARENT(tp) = left;
          left = C_RIGHT_INDEX(tp);
          if((tp = base[left])==NO_NODE)
          {
            base[right]=NO_NODE;
            return 0;
          }
        }
      }
      else
      {
        base[left] = C_LEFT(tp);
        if (C_LEFT(tp) != NO_NODE)
        {
          PARENT(C_LEFT(tp)) = left;
        }
        base[right] = C_RIGHT(tp);
        if (C_RIGHT(tp) != NO_NODE)
        {
          PARENT(C_RIGHT(tp)) = right;
        }
        DIC(nkey+com->max_match-1) = ~DIC(p+com->max_match-1);  /* sentinel */
        {
          int32 q = nkey;
          do
          {
            if (DIC(p++) != DIC(nkey++))
              break;
            if (DIC(p++) != DIC(nkey++))
              break;
            if (DIC(p++) != DIC(nkey++))
              break;
            if (DIC(p++) != DIC(nkey++))
              break;
          }
          while (DIC(p++) == DIC(nkey++));
          com->best_match_pos = nkey-p-1;
          return (c_codetype)(nkey-q+2);
        }
      }
    }
  }
}

#endif


#endif
 

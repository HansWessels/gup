/*
 * Sliding dictionary code voor ARJ
 * Small machine code
 * (c) 1996 Hans "Mr Ni! the Great" Wessels
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

#define STARTMATCH 3                   /* Matches van STARTMATCH en kleiner worden niet bekeken op bitafstand */

#define NO_NODE   0x0000

#define HASH(x) (((*(x)^(x)[1])<<8) ^ ((x)[1]^(x)[2]))
#define DELTA_HASH(x) ((com->delta_hash<<8) ^ ((x)[-1]^*(x)))

c_codetype insert2m(uint16 pos, uint16 hash, uint16 rle_size, packstruct *com);
c_codetype insertm(uint16 pos, uint16 hash, packstruct *com);
void insert2nmm(uint16 pos, uint16 hash, uint16 rle_size, packstruct *com);
void insertnmm(uint16 pos, uint16 hash, packstruct *com);
void init_dictionary_medium(packstruct *com);
c_codetype find_dictionary_medium(uint16 pos, packstruct *com);
c_codetype find_dictionary_medium_fast(uint16 pos, packstruct *com);

void insertnmm_fast(uint16 nnode, uint16 hash, packstruct *com);
c_codetype insertm_fast(uint16 nnode, uint16 hash, packstruct *com);

#ifndef NOT_USE_STD_init_dictionary_medium
void init_dictionary_medium(packstruct *com)
{
  /*
   * initialiseert de sliding dictionary initialiseert de
   * slidingdictionary structuren en zet eerste byte in dictionary
  */
  /* reset hash table */
  memset(com->root.small, 0, HASH_SIZE * sizeof (small_node_type));
  if(com->speed<2)
  {
    memset(com->root2.small, 0, HASH_SIZE * sizeof (small_node_type));
    memset(com->tree.small, 0, com->tree_size * sizeof (small_node_struct));
    com->hist_index=0;
  }
  com->last_pos = 2;
  com->del_pos = (uint16) (com->last_pos - com->maxptr - 1);
  if ((com->delta_hash = HASH(com->dictionary)) != 0)
  {
    com->rle_size = 0;
    insertnmm(2, com->delta_hash, com);
  }
  else
  {
    uint8 orig;
    uint8 *new_key=com->dictionary+2;
    uint8 *p = new_key;
    com->rle_char = *new_key;
    orig = new_key[com->max_match];
    new_key[com->max_match] = (uint8)~com->rle_char;
    while (*p++ == com->rle_char)
    {
      ;
    }
    com->rle_size = (uint16)(p - new_key - 1);
    new_key[com->max_match] = orig;
    com->rle_hash = (com->rle_char << 8) + com->rle_size;
    if(com->speed<2)
    {
      insert2nmm(2, com->rle_hash, com->rle_size, com);
    }
    if(new_key[com->rle_size] != com->rle_char)
    {
      com->rle_size--;
      com->rle_hash--;
    }
  }
}

#endif


#ifndef NOT_USE_STD_find_dictionary_medium

c_codetype find_dictionary_medium(uint16 pos, packstruct *com)
{
  /*
   * zoekt maximale match in dictionary bij pos update de
   * slidingdictionary structuren returnt max_match, als geen max_match
   * gevonden dan return 0 best_match_pos is de absolute positie van
   * max_match
  */
  uint16 to_do = pos - com->last_pos;
  ARJ_Assert(((pos-com->last_pos)&0xffff)<=MAX_MATCH);
  { /*- wis de nodes */
    int i = to_do;
    uint16 key = com->del_pos;
    do
    {
      if(key!=NO_NODE)
      {
        small_node_type tp = com->tree.small[key].parent;
        if (tp!=NO_NODE)
        { /* we moeten eerst de beide roots doen */
          if (com->root.small[tp] == key)
          {
            com->root.small[tp] = NO_NODE;
          }
          else
          {
            if (com->root2.small[tp] == key)
            {
              com->root2.small[tp] = NO_NODE;
            }
            else
            {
              small_node_struct * tpc=com->tree.small+tp;
              if (tpc->c_left == key)
              {
                tpc->c_left = NO_NODE;
              }
              else
              {
                if (tpc->c_right == key)
                {
                  tpc->c_right = NO_NODE;
                }
                else
                {
                  if (tpc->link == key)
                  {
                    tpc->link = NO_NODE;
                  }
                }
              }
            }
          }
        }
      }
      key++;
    }
    while (--i > 0);
    com->del_pos = key;
  }
  com->last_pos = pos;
  pos-=to_do;
  while (--to_do > 0)
  {
    pos++;
    if (com->rle_size > 0)
    {
      if(pos!=NO_NODE)
      {
        insert2nmm(pos, com->rle_hash, com->rle_size, com);
      }
      if((com->dictionary+pos)[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(com->dictionary+pos)) != 0)
      {
        if(pos!=NO_NODE)
        {
          insertnmm(pos, com->delta_hash, com);
        }
      }
      else
      {
        uint8 orig;
        uint8 *p=com->dictionary+pos;
        uint8 *q=p;
        com->rle_char = *p;
        orig = p[com->max_match];
        p[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - q - 1);
        q[com->max_match] = orig;
        com->rle_hash = (com->rle_char << 8) + com->rle_size;
        if(pos!=NO_NODE)
        {
          insert2nmm(pos, com->rle_hash, com->rle_size, com);
        }
        if(q[com->rle_size]!=com->rle_char)
        {
          com->rle_size--;
          com->rle_hash--;
        }
      }
    }
  }
  pos++;
  {
    uint8 *new_keyp=com->dictionary+pos;
    uint8 orig = new_keyp[com->max_match];
    c_codetype res=0;
    if (com->rle_size > 0)
    {
      if(pos!=NO_NODE)
      {
        res=insert2m(pos, com->rle_hash, com->rle_size, com);
      }
      new_keyp[com->max_match]=orig;
      if(new_keyp[com->rle_size]!=com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_keyp)) != 0)
      {
        if(pos!=NO_NODE)
        {
          res=insertm(pos, com->delta_hash, com);
        }
        new_keyp[com->max_match]=orig;
      }
      else
      {
        uint8* p=new_keyp;
        com->rle_char = *p;
        p[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p- new_keyp - 1);
        com->rle_hash = (com->rle_char << 8) + com->rle_size;
        if(pos!=NO_NODE)
        {
          res=insert2m(pos, com->rle_hash, com->rle_size, com);
        }
        new_keyp[com->max_match]=orig;
        if(new_keyp[com->rle_size]!=com->rle_char)
        {
          com->rle_size--;
          com->rle_hash--;
        }
      }
    }
    return res;
  }
}

#endif


#ifndef NOT_USE_STD_insert2nmm

void insert2nmm(uint16 nnode, uint16 hash, uint16 rle_size, packstruct *com)
{
  uint8 *nkey;
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left = nnode;
  small_node_type right = nnode;
  tpc->parent=hash;
  current=com->root2.small[hash];
  com->root2.small[hash]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    return;
  }
  nkey=com->dictionary+nnode+rle_size;
  for(;;)
  {
    uint8* p;
    uint8* q = nkey;
    tpc=tp+current;
    p = com->dictionary+current+rle_size;
    if (*p++ == *q++)
    #if MAXD_MATCH>1
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>2
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>3
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>4
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>5
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>6
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>7
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>8
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>9
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>10
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>11
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>12
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>13
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>14
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>15
    if (*p++ == *q++)
    #endif
    { /*- we hebben een 100% match */
      tp[left].c_right=tpc->c_left;
      if(tpc->c_left!=NO_NODE)
      {
        tp[tpc->c_left].parent=left;
      }
      tp[right].c_left=tpc->c_right;
      if(tpc->c_right!=NO_NODE)
      {
        tp[tpc->c_right].parent=right;
      }
      com->tree.small[nnode].link=current;
      tpc->parent=nnode;
      break;
    }
    if((*--q)<(*--p))
    {
      /*
       * de key van nnode is kleiner dan die van tp, tp moet dus rechts
       * van nnode komen tp node wordt dus de linker child van rechts. de
       * nieuwe tp node wordt de linker child van tp
      */
      tp[right].c_left=current;
      tpc->parent=right;
      right=current;
      if((current=tpc->c_left)==NO_NODE)
      {
        tp[left].c_right=NO_NODE;
        break;
      }
    }
    else
    {
      tp[left].c_right=current;
      tpc->parent=left;
      left=current;
      if((current=tpc->c_right)==NO_NODE)
      {
        tp[right].c_left=NO_NODE;
        break;
      }
    }
  }
  tpc=tp+nnode;
  current=tpc->c_left;
  tpc->c_left=tpc->c_right;
  tpc->c_right=current;
}

#endif


#ifndef NOT_USE_STD_insertnmm

void insertnmm(uint16 nnode, uint16 hash, packstruct *com)
{
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left = nnode;
  small_node_type right = nnode;
  uint8 * nkey;
  tpc->parent=hash;
  current=com->root.small[hash];
  com->root.small[hash]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    return;
  }
  nkey = com->dictionary+nnode;
  for(;;)
  {
    uint8* p;
    uint8* q = nkey;
    tpc=tp+current;
    p = com->dictionary+current;
    if (*p++ == *q++)
    #if MAXD_MATCH>1
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>2
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>3
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>4
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>5
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>6
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>7
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>8
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>9
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>10
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>11
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>12
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>13
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>14
    if (*p++ == *q++)
    #endif
    #if MAXD_MATCH>15
    if (*p++ == *q++)
    #endif
    { /*- we hebben een 100% match */
      tp[left].c_right=tpc->c_left;
      if(tpc->c_left!=NO_NODE)
      {
        tp[tpc->c_left].parent=left;
      }
      tp[right].c_left=tpc->c_right;
      if(tpc->c_right!=NO_NODE)
      {
        tp[tpc->c_right].parent=right;
      }
      com->tree.small[nnode].link=current;
      tpc->parent=nnode;
      break;
    }
    if((*--q)<(*--p))
    {
      /*
       * de key van nnode is kleiner dan die van tp, tp moet dus rechts
       * van nnode komen tp node wordt dus de linker child van rechts. de
       * nieuwe tp node wordt de linker child van tp
      */
      tp[right].c_left=current;
      tpc->parent=right;
      right=current;
      if((current=tpc->c_left)==NO_NODE)
      {
        tp[left].c_right=NO_NODE;
        break;
      }
    }
    else
    {
      tp[left].c_right=current;
      tpc->parent=left;
      left=current;
      if((current=tpc->c_right)==NO_NODE)
      {
        tp[right].c_left=NO_NODE;
        break;
      }
    }
  }
  tpc=tp+nnode;
  current=tpc->c_left;
  tpc->c_left=tpc->c_right;
  tpc->c_right=current;
}

#endif


#ifndef NOT_USE_STD_insert2m

c_codetype insert2m(uint16 nnode, uint16 hash, uint16 rle_size, packstruct *com)
{
  c_codetype best_match;
  c_codetype opt_match;
  uint16 max_match=com->max_match;
  hist_struct *hist;
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left=nnode;
  small_node_type right=nnode;
  uint8* nkey=com->dictionary+nnode;
  tpc->parent=hash;
  com->hist_index=((com->hist_index+1) & (HISTSIZE-1));
  hist=com->hist[com->hist_index];
  if(nkey[-3]==*nkey)
  {
    best_match=0;
    opt_match=2;
    com->best_match_pos=0;
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
  current=com->root2.small[hash];
  com->root2.small[hash]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    if(opt_match<0)
    {
      return 0;
    }
    else
    {
      return rle_size+2;
    }
  }
  else
  {
    uint8 *origkey=nkey;
    max_match-=rle_size;
    for(;;)
    {
      c_codetype match;
      uint8 *p;
      tpc=tp+current;
      p = com->dictionary+current+rle_size;
      nkey[max_match] = ~p[max_match];  /* sentinel */
      do
      {
        if (*p++ != *nkey++)
          break;
        if (*p++ != *nkey++)
          break;
        if (*p++ != *nkey++)
          break;
        if (*p++ != *nkey++)
          break;
      }
      while (*p++ == *nkey++);
      nkey--;
      match = (c_codetype)(nkey - origkey);
      if (match > best_match)
      {
        best_match = match;
        {
          uint16 pos = (uint16)(nkey - p);
          hist->match=match+rle_size;
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
          tp[left].c_right=tpc->c_left;
          if(tpc->c_left!=NO_NODE)
          {
            tp[tpc->c_left].parent=left;
          }
          tp[right].c_left=tpc->c_right;
          if(tpc->c_right!=NO_NODE)
          {
            tp[tpc->c_right].parent=right;
          }
          if (best_match == max_match)
          { /*- link met de link van deze node */
            if(tpc->link!=NO_NODE) 
            {
              com->tree.small[nnode].link=tpc->link;
              tp[tpc->link].parent=nnode;
              tpc->link=NO_NODE;
            }
          }
          else
          {
            max_match-=MAXD_MATCH;
            origkey+=MAXD_MATCH;
            best_match-=MAXD_MATCH;
            opt_match-=MAXD_MATCH;
            rle_size+=MAXD_MATCH;
            com->tree.small[nnode].link = current;
            tpc->parent = nnode;
            current = tpc->link;
            while (current != NO_NODE)
            {
              tpc=tp+current;
              {
                p = com->dictionary+current+rle_size;
                nkey=origkey;
                nkey[max_match] = ~p[max_match];  /* sentinel */
                do
                {
                  if (*p++ != *nkey++)
                    break;
                  if (*p++ != *nkey++)
                    break;
                  if (*p++ != *nkey++)
                    break;
                  if (*p++ != *nkey++)
                    break;
                }
                while (*p++ == *nkey++);
                nkey--;
                match = (c_codetype)(nkey - origkey);
              }
              if (match > best_match)
              {
                best_match = match;
                {
                  uint16 pos = (uint16)(nkey - p);
                  hist->match=match+rle_size;
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
              current = tpc->link;
            }
            tpc=tp+nnode;
            current=tpc->c_left;
            tpc->c_left=tpc->c_right;
            tpc->c_right=current;
            return opt_match+rle_size;
          }
          tpc=tp+nnode;
          current=tpc->c_left;
          tpc->c_left=tpc->c_right;
          tpc->c_right=current;
          return opt_match+rle_size;
        }
      }
      else
      {
        if (best_match == max_match)
        { /*- we hebben een 100% match */
          tp[left].c_right=tpc->c_left;
          if(tpc->c_left!=NO_NODE)
          {
            tp[tpc->c_left].parent=left;
          }
          tp[right].c_left=tpc->c_right;
          if(tpc->c_right!=NO_NODE)
          {
            tp[tpc->c_right].parent=right;
          }
          tpc=tp+nnode;
          current=tpc->c_left;
          tpc->c_left=tpc->c_right;
          tpc->c_right=current;
          return opt_match+rle_size;
        }
      }
      if((*nkey) < (*--p))
      {
        /*
         * de key van nnode is kleiner dan die van tp, tp moet dus rechts
         * van nnode komen tp node wordt dus de linker child van rechts. de
         * nieuwe tp node wordt de linker child van tp
        */
        tp[right].c_left=current;
        tpc->parent=right;
        right=current;
        if((current=tpc->c_left)==NO_NODE)
        {
          tp[left].c_right=NO_NODE;
          break;
        }
      }
      else
      {
        tp[left].c_right=current;
        tpc->parent=left;
        left=current;
        if((current=tpc->c_right)==NO_NODE)
        {
          tp[right].c_left=NO_NODE;
          break;
        }
      }
      nkey=origkey;
    }
    opt_match+=rle_size;
  }
  tpc=tp+nnode;
  current=tpc->c_left;
  tpc->c_left=tpc->c_right;
  tpc->c_right=current;
  return opt_match;
}

#endif


#ifndef NOT_USE_STD_insertm

c_codetype insertm(uint16 nnode, uint16 hash, packstruct *com)
{
  uint16 max_match=com->max_match-1;
  c_codetype opt_match;
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left=nnode;
  small_node_type right=nnode;
  uint8* nkey=com->dictionary+nnode;
  tpc->parent=hash;
  current=com->root.small[hash];
  com->root.small[hash]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    return 0;
  }
  else
  {
    c_codetype best_match=0;
    uint8 *origkey=nkey;
    hist_struct *hist;
    com->hist_index=((com->hist_index+1) & (HISTSIZE-1));
    hist=com->hist[com->hist_index];
    opt_match=0;
    for(;;)
    {
      c_codetype match;
      uint8 *p;
      tpc=tp+current;
      p = com->dictionary+current;
      if(*p++!=*nkey++)
      {
        if((*--nkey)<(*--p))
        {
          /*
           * de key van nnode is kleiner dan die van tp, tp moet dus rechts
           * van nnode komen tp node wordt dus de linker child van rechts. de
           * nieuwe tp node wordt de linker child van tp
          */
          tp[right].c_left=current;
          tpc->parent=right;
          right=current;
          if((current=tpc->c_left)==NO_NODE)
          {
            tp[left].c_right=NO_NODE;
            break;
          }
        }
        else
        {
          tp[left].c_right=current;
          tpc->parent=left;
          left=current;
          if((current=tpc->c_right)==NO_NODE)
          {
            tp[right].c_left=NO_NODE;
            break;
          }
        }
      }
      else
      {
        nkey[max_match] = ~p[max_match];  /* sentinel */
        do
        {
          if (*p++ != *nkey++)
            break;
          if (*p++ != *nkey++)
            break;
          if (*p++ != *nkey++)
            break;
          if (*p++ != *nkey++)
            break;
        }
        while (*p++ == *nkey++);
        nkey--;
        match = (c_codetype)(nkey - origkey);
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
          if (best_match >= MAXD_MATCH)
          { /*- we hebben een 100% match */
            tp[left].c_right=tpc->c_left;
            if(tpc->c_left!=NO_NODE)
            {
              tp[tpc->c_left].parent=left;
            }
            tp[right].c_left=tpc->c_right;
            if(tpc->c_right!=NO_NODE)
            {
              tp[tpc->c_right].parent=right;
            }
            if (best_match == com->max_match)
            { /* link met de link van deze node */
              if(tpc->link!=NO_NODE) 
              {
                com->tree.small[nnode].link=tpc->link;
                tp[tpc->link].parent=nnode;
                tpc->link=NO_NODE;
              }
            }
            else
            {
              max_match-=MAXD_MATCH-1;
              origkey+=MAXD_MATCH;
              best_match-=MAXD_MATCH;
              opt_match-=MAXD_MATCH;
              com->tree.small[nnode].link = current;
              tpc->parent = nnode;
              current = tpc->link;
              while (current != NO_NODE)
              {
                tpc=tp+current;
                {
                  p = com->dictionary+current+MAXD_MATCH;
                  nkey=origkey;
                  nkey[max_match] = ~p[max_match];  /* sentinel */
                  do
                  {
                    if (*p++ != *nkey++)
                      break;
                    if (*p++ != *nkey++)
                      break;
                    if (*p++ != *nkey++)
                      break;
                    if (*p++ != *nkey++)
                      break;
                  }
                  while (*p++ == *nkey++);
                  nkey--;
                  match = (c_codetype)(nkey - origkey);
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
                current = tpc->link;
              }
              opt_match+=MAXD_MATCH;
            }
            tpc=tp+nnode;
            current=tpc->c_left;
            tpc->c_left=tpc->c_right;
            tpc->c_right=current;
            return opt_match;
          }
        }
        if((*nkey)<(*--p))
        {
          /*
           * de key van nnode is kleiner dan die van tp, tp moet dus rechts
           * van nnode komen tp node wordt dus de linker child van rechts. de
           * nieuwe tp node wordt de linker child van tp
          */
          tp[right].c_left=current;
          tpc->parent=right;
          right=current;
          if((current=tpc->c_left)==NO_NODE)
          {
            tp[left].c_right=NO_NODE;
            break;
          }
        }
        else
        {
          tp[left].c_right=current;
          tpc->parent=left;
          left=current;
          if((current=tpc->c_right)==NO_NODE)
          {
            tp[right].c_left=NO_NODE;
            break;
          }
        }
        nkey=origkey;
      }
    }
  }
  tpc=tp+nnode;
  current=tpc->c_left;
  tpc->c_left=tpc->c_right;
  tpc->c_right=current;
  return opt_match;
}
#endif


/**************************************************************************
 * fast dictionary routines, use first match found, use only rle matches  *
 **************************************************************************/


#ifndef NOT_USE_STD_find_dictionary_medium_fast

c_codetype find_dictionary_medium_fast(uint16 pos, packstruct *com)
{
  /*
   * zoekt eerste match in dictionary bij pos update de
   * slidingdictionary structuren returnt max_match, als geen match
   * gevonden dan return 0 best_match_pos is de absolute positie van
   * match
  */
  uint16 to_do = pos - com->last_pos;
  ARJ_Assert(((pos-com->last_pos)&0xffff)<=MAX_MATCH);
  { /*- wis de nodes */
    int i = to_do;
    uint16 key = com->del_pos;
    do
    {
      small_node_type tp;
      if(key!=NO_NODE)
      {
        tp = com->tree.small[key].parent;
        if (tp!=NO_NODE)
        { /* we moeten eerst de beide roots doen */
          if (com->root.small[tp] == key)
          {
            com->root.small[tp] = NO_NODE;
          }
          else
          {
            small_node_struct * tpc=com->tree.small+tp;
            if (tpc->c_left == key)
            {
              tpc->c_left = NO_NODE;
            }
            else
            {
              if (tpc->c_right == key)
              {
                tpc->c_right = NO_NODE;
              }
            }
          }
        }
      }
      key++;
    }
    while (--i > 0);
    com->del_pos = key;
  }
  com->last_pos = pos;
  pos-=to_do;
  while (--to_do > 0)
  {
    pos++;
    if (com->rle_size > 0)
    {
      if((com->dictionary+pos)[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(com->dictionary+pos)) != 0)
      {
        if(pos!=NO_NODE)
        {
          insertnmm_fast(pos, com->delta_hash, com);
        }
      }
      else
      {
        uint8 orig;
        uint8 *p=com->dictionary+pos;
        uint8 *q=p;
        com->rle_char = *p;
        orig = p[com->max_match];
        p[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - q - 1);
        q[com->max_match] = orig;
        if(q[com->rle_size]!=com->rle_char)
        {
          com->rle_size--;
        }
      }
    }
  }
  pos++;
  {
    uint8 *new_keyp=com->dictionary+pos;
    uint8 orig = new_keyp[com->max_match];
    c_codetype res=0;
    if (com->rle_size > 0)
    {
      if(new_keyp[-3]==*new_keyp)
      {
        com->best_match_pos=0;
        res=com->rle_size+2;
      }
      if(new_keyp[com->rle_size]!=com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_keyp)) != 0)
      {
        if(pos!=NO_NODE)
        {
          res=insertm_fast(pos, com->delta_hash, com);
        }
        new_keyp[com->max_match]=orig;
      }
      else
      {
        uint8* p=new_keyp;
        com->rle_char = *p;
        p[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p- new_keyp - 1);
        new_keyp[com->max_match]=orig;
        if(new_keyp[com->rle_size]!=com->rle_char)
        {
          com->rle_size--;
        }
      }
    }
    return res;
  }
}

#endif

#ifndef NOT_USE_STD_insertnmm_fast

void insertnmm_fast(uint16 nnode, uint16 hash, packstruct *com)
{
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left = nnode;
  small_node_type right = nnode;
  uint8 first;
  tpc->parent=hash;
  current=com->root.small[hash];
  com->root.small[hash]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    return;
  }
  first = com->dictionary[nnode];
  for(;;)
  {
    uint8* p;
    tpc=tp+current;
    p = com->dictionary+current;
    if (*p == first)
    { /*- we hebben een 100% match */
      tp[left].c_right=tpc->c_left;
      if(tpc->c_left!=NO_NODE)
      {
        tp[tpc->c_left].parent=left;
      }
      tp[right].c_left=tpc->c_right;
      if(tpc->c_right!=NO_NODE)
      {
        tp[tpc->c_right].parent=right;
      }
      break;
    }
    if(first<(*p))
    {
      /*
       * de key van nnode is kleiner dan die van tp, tp moet dus rechts
       * van nnode komen tp node wordt dus de linker child van rechts. de
       * nieuwe tp node wordt de linker child van tp
      */
      tp[right].c_left=current;
      tpc->parent=right;
      right=current;
      if((current=tpc->c_left)==NO_NODE)
      {
        tp[left].c_right=NO_NODE;
        break;
      }
    }
    else
    {
      tp[left].c_right=current;
      tpc->parent=left;
      left=current;
      if((current=tpc->c_right)==NO_NODE)
      {
        tp[right].c_left=NO_NODE;
        break;
      }
    }
  }
  tpc=tp+nnode;
  current=tpc->c_left;
  tpc->c_left=tpc->c_right;
  tpc->c_right=current;
}

#endif


#ifndef NOT_USE_STD_insertm_fast

c_codetype insertm_fast(uint16 nnode, uint16 hash, packstruct *com)
{
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left=nnode;
  small_node_type right=nnode;
  uint8* nkey=com->dictionary+nnode;
  tpc->parent=hash;
  current=com->root.small[hash];
  com->root.small[hash]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    return 0;
  }
  else
  {
    uint8 first=*nkey++;
    for(;;)
    {
      uint8 *p;
      tpc=tp+current;
      p = com->dictionary+current;
      if(*p++!=first)
      {
        if(first<(*--p))
        {
          /*
           * de key van nnode is kleiner dan die van tp, tp moet dus rechts
           * van nnode komen tp node wordt dus de linker child van rechts. de
           * nieuwe tp node wordt de linker child van tp
          */
          tp[right].c_left=current;
          tpc->parent=right;
          right=current;
          if((current=tpc->c_left)==NO_NODE)
          {
            tp[left].c_right=NO_NODE;
            break;
          }
        }
        else
        {
          tp[left].c_right=current;
          tpc->parent=left;
          left=current;
          if((current=tpc->c_right)==NO_NODE)
          {
            tp[right].c_left=NO_NODE;
            break;
          }
        }
      }
      else
      {
        uint8 *origkey;
        tp[left].c_right=tpc->c_left;
        if(tpc->c_left!=NO_NODE)
        {
          tp[tpc->c_left].parent=left;
        }
        tp[right].c_left=tpc->c_right;
        if(tpc->c_right!=NO_NODE)
        {
          tp[tpc->c_right].parent=right;
        }
        tpc=tp+nnode;
        current=tpc->c_left;
        tpc->c_left=tpc->c_right;
        tpc->c_right=current;
        origkey=nkey;
        nkey[com->max_match-1] = ~p[com->max_match-1];  /* sentinel */
        do
        {
          if (*p++ != *nkey++)
            break;
          if (*p++ != *nkey++)
            break;
          if (*p++ != *nkey++)
            break;
          if (*p++ != *nkey++)
            break;
        }
        while (*p++ == *nkey++);
        com->best_match_pos = (uint16)(nkey-p-1);
        return (c_codetype)(nkey-origkey+2);
      }
    }
  }
  tpc=tp+nnode;
  current=tpc->c_left;
  tpc->c_left=tpc->c_right;
  tpc->c_right=current;
  return 0;
}
#endif
  
 

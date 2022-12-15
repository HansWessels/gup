/*
 * Sliding dictionary code voor ARJ
 * Small machine code
 * (c) 1996 Hans "Mr Ni! the Great" Wessels
 * 
 */

#undef NDEBUG
#if 0
#define NDEBUG
#endif

#include <string.h>
#include <stddef.h>

#include "gup.h"
#include "encode.h"
#include "evaluatr.h"

#define STARTMATCH 3                   /* Matches van STARTMATCH en kleiner worden niet bekeken op bitafstand */

#define NO_NODE   0xffff

#define SMALL_HASH(x)  ((uint8)((*(x)<<6)^((x)[1]<<3)^(x)[2]))
#define SMALL_DELTA_HASH(x) ((uint8)((com->delta_hash<<3) ^ (x)[2]))

c_codetype insert2s(uint16 pos, uint16 rle_size, packstruct *com);
c_codetype inserts(uint16 pos, uint16 hash, packstruct *com);
void insert2nms(uint16 pos, uint16 rle_size, packstruct *com);
void insertnms(uint16 pos, uint16 hash, packstruct *com);
void init_dictionary_small(packstruct *com);
c_codetype find_dictionary_small(uint16 pos, packstruct *com);
c_codetype find_dictionary_small_fast(uint16 pos, packstruct *com);

void insertnms_fast(uint16 nnode, uint16 hash, packstruct *com);
c_codetype inserts_fast(uint16 nnode, uint16 hash, packstruct *com);

#ifndef NOT_USE_STD_init_dictionary_small
void init_dictionary_small(packstruct *com)
{
  /*
   * initialiseert de sliding dictionary initialiseert de
   * slidingdictionary structuren en zet eerste byte in dictionary
  */
  /* reset hash table */
  memset(com->root.small, 0xff, SMALL_HASH_SIZE * sizeof (small_node_type));
  if(com->speed<2)
  {
    memset(com->root2.small, 0xff, SMALL_HASH2_SIZE * sizeof (small_node_type));
    memset(com->tree.small, 0, com->tree_size * sizeof (small_node_struct));
    com->hist_index=0;
  }
  com->last_pos = 0;
  com->del_pos = MS(com->last_pos - com->maxptr - 1);
  {
    uint8 *new_key=com->dictionary;
    if((*new_key==new_key[2]) && (*new_key==new_key[1]))
    {
      uint8 orig;
      uint8 *p = new_key+2;
      com->rle_char = *new_key;
      orig = new_key[com->max_match];
      new_key[com->max_match] = (uint8)~com->rle_char;
      while (*p++ == com->rle_char)
      {
        ;
      }
      com->rle_size = (uint16)(p - new_key - 3);
      new_key[com->max_match] = orig;
      if(com->speed<2)
      {
        insert2nms(0, com->rle_size, com);
      }
      if(new_key[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
      }
    }    
    else
    {
      com->delta_hash=SMALL_HASH(new_key);
      insertnms(0, com->delta_hash, com);
    }
  }
}

#endif


#ifndef NOT_USE_STD_find_dictionary_small

c_codetype find_dictionary_small(uint16 pos, packstruct *com)
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
        for(;;)
        {
          if (tp!=NO_NODE)
          { /* we moeten eerst de beide roots doen */
            if(tp<SMALL_HASH_SIZE)
            {
              if (com->root.small[tp] == key)
              {
                com->root.small[tp] = NO_NODE;
                break;
              }
              if (com->root2.small[tp] == key)
              {
                com->root2.small[tp] = NO_NODE;
                break;
              }
            }
            {
              small_node_struct * tpc=com->tree.small+tp;
              if (tpc->c_left == key)
              {
                tpc->c_left = NO_NODE;
                break;
              }
              if (tpc->c_right == key)
              {
                tpc->c_right = NO_NODE;
                break;
              }
              if (tpc->link == key)
              {
                tpc->link = NO_NODE;
                break;
              }
            }
          }
          break;
        }
      }
      key=MS(key+1);
    }
    while (--i > 0);
    com->del_pos = key;
  }
  com->last_pos = pos;
  pos=MS(pos-to_do);
  while (--to_do > 0)
  {
    pos=MS(pos+1);
    if (com->rle_size > 0)
    {
      if(pos!=NO_NODE)
      {
        insert2nms(pos, com->rle_size, com);
      }
      if((com->dictionary+pos)[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      uint8 *new_key=com->dictionary+pos;
      if((*new_key==new_key[2]) && (*new_key==new_key[1]))
      {
        uint8 orig;
        uint8 *p=new_key+3;
        com->rle_char=*new_key;
        orig = p[com->max_match];
        p[com->max_match] = ~*new_key;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 3);
        new_key[com->max_match] = orig;
        if(pos!=NO_NODE)
        {
          insert2nms(pos, com->rle_size, com);
        }
        if(new_key[com->rle_size]!=com->rle_char)
        {
          com->rle_size--;
        }
      }
      else
      {
        com->delta_hash=SMALL_DELTA_HASH(com->dictionary+pos);
        if(pos!=NO_NODE)
        {
          insertnms(pos, com->delta_hash, com);
        }
      }
    }
  }
  pos=MS(pos+1);
  {
    uint8 *new_keyp=com->dictionary+pos;
    uint8 orig = new_keyp[com->max_match];
    c_codetype res=0;
    if (com->rle_size > 0)
    {
      if(pos!=NO_NODE)
      {
        res=insert2s(pos, com->rle_size, com);
      }
      new_keyp[com->max_match]=orig;
      if(new_keyp[com->rle_size]!=com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if((*new_keyp==new_keyp[2]) && (*new_keyp==new_keyp[1]))
      {
        uint8* p=new_keyp+3;
        com->rle_char = *new_keyp;
        p[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p- new_keyp - 3);
        if(pos!=NO_NODE)
        {
          res=insert2s(pos, com->rle_size, com);
        }
        new_keyp[com->max_match]=orig;
        if(new_keyp[com->rle_size]!=com->rle_char)
        {
          com->rle_size--;
        }
      }
      else
      {
        com->delta_hash = SMALL_DELTA_HASH(new_keyp);
        if(pos!=NO_NODE)
        {
          res=inserts(pos, com->delta_hash, com);
        }
        new_keyp[com->max_match]=orig;
      }
    }
    return res;
  }
}

#endif


#ifndef NOT_USE_STD_insert2nms

void insert2nms(uint16 nnode, uint16 rle_size, packstruct *com)
{
  uint8 *nkey;
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left = nnode;
  small_node_type right = nnode;
  tpc->parent=rle_size;
  current=com->root2.small[rle_size];
  com->root2.small[rle_size]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    return;
  }
  nkey=com->dictionary+nnode+rle_size+1;
  for(;;)
  {
    uint8* p;
    uint8* q = nkey;
    tpc=tp+current;
    p = com->dictionary+current+rle_size+1;
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


#ifndef NOT_USE_STD_insertnms

void insertnms(uint16 nnode, uint16 hash, packstruct *com)
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


#ifndef NOT_USE_STD_insert2s

c_codetype insert2s(uint16 nnode, uint16 rle_size, packstruct *com)
{
  c_codetype best_match;
  c_codetype opt_match;
  hist_struct *hist;
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left=nnode;
  small_node_type right=nnode;
  uint8* nkey=com->dictionary+nnode;
  tpc->parent=rle_size;
  com->hist_index=((com->hist_index+1) & (HISTSIZE-1));
  hist=com->hist[com->hist_index];
  if(nkey[-1]==*nkey)
  {
    best_match=1;
    opt_match=3;
    com->best_match_pos=0;
    hist->match=rle_size;
    hist->ptr=0;
    hist++;
  }
  else
  {
    best_match=0;
    opt_match=-2;
  }
  nkey+=rle_size+1;
  current=com->root2.small[rle_size];
  com->root2.small[rle_size]=nnode;
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
    uint16 max_match=com->max_match-rle_size;
    for(;;)
    {
      c_codetype match;
      uint8 *p;
      tpc=tp+current;
      p = com->dictionary+current+rle_size+1;
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
          hist->match=match+rle_size-1;
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


#ifndef NOT_USE_STD_inserts

c_codetype inserts(uint16 nnode, uint16 hash, packstruct *com)
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
    c_codetype best_match=2;
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


#ifndef NOT_USE_STD_find_dictionary_small_fast

c_codetype find_dictionary_small_fast(uint16 pos, packstruct *com)
{
  /*
   * zoekt eerste match in dictionary bij pos update de
   * slidingdictionary structuren returnt max_match, als geen match
   * gevonden dan return 0 best_match_pos is de absolute positie van
   * match
  */
  uint16 to_do = MS(pos - com->last_pos);
  ARJ_Assert(to_do<=MAX_MATCH);
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
      key=MS(key+1);
    }
    while (--i > 0);
    com->del_pos = key;
  }
  com->last_pos = pos;
  pos-=to_do;
  while (--to_do > 0)
  {
    uint8 *p;
    pos=MS(pos+1);
    p=com->dictionary+pos;
    if (com->rle_size > 0)
    {
      if(p[com->rle_size+2] != com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if((p[0]!=p[1]) || (p[0]!=p[2]))
      {
        com->delta_hash = SMALL_DELTA_HASH(com->dictionary+pos);
        if(pos!=NO_NODE)
        {
          insertnms_fast(pos, com->delta_hash, com);
        }
      }
      else
      {
        uint8 orig;
        uint8 *q=p;
        com->rle_char = *p;
        orig = p[com->max_match];
        p[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - q - 3);
        q[com->max_match] = orig;
        if(q[com->rle_size+2]!=com->rle_char)
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
      if(new_keyp[-1]==*new_keyp)
      {
        com->best_match_pos=0;
        res=com->rle_size+2;
      }
      if(new_keyp[com->rle_size+2]!=com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if((new_keyp[0]!=new_keyp[1]) || (new_keyp[0]!=new_keyp[2]))
      {
        com->delta_hash = SMALL_DELTA_HASH(com->dictionary+pos);
        if(pos!=NO_NODE)
        {
          res=inserts_fast(pos, com->delta_hash, com);
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
        com->rle_size = (uint16)(p- new_keyp - 3);
        new_keyp[com->max_match]=orig;
        if(new_keyp[com->rle_size+2]!=com->rle_char)
        {
          com->rle_size--;
        }
      }
    }
    return res;
  }
}

#endif

#ifndef NOT_USE_STD_insertnms_fast

void insertnms_fast(uint16 nnode, uint16 hash, packstruct *com)
{
  small_node_struct *tp=com->tree.small;
  small_node_struct *tpc=tp+nnode;
  small_node_type current;
  small_node_type left = nnode;
  small_node_type right = nnode;
  uint8 *first;
  tpc->parent=hash;
  current=com->root.small[hash];
  com->root.small[hash]=nnode;
  if(current==NO_NODE)
  {
    tpc->c_right=NO_NODE;
    tpc->c_left=NO_NODE;
    return;
  }
  first = com->dictionary+nnode;
  for(;;)
  {
    uint8* p;
    uint8* q;
    tpc=tp+current;
    p = com->dictionary+current;
    q=first;
    if (*p++ == *q++)
    if (*p++ == *q++)
    if (*p++ == *q++)
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


#ifndef NOT_USE_STD_inserts_fast

c_codetype inserts_fast(uint16 nnode, uint16 hash, packstruct *com)
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
    for(;;)
    {
      uint8 *p;
      uint8 *q=nkey;
      tpc=tp+current;
      p = com->dictionary+current;
      if(*p++==*q++)
      if(*p++==*q++)
      if(*p++==*q++)
      { /* 100% match */
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
        nkey[com->max_match] = (uint8)~p[com->max_match-3];  /* sentinel */
        do
        {
          if (*p++ != *q++)
            break;
          if (*p++ != *q++)
            break;
          if (*p++ != *q++)
            break;
          if (*p++ != *q++)
            break;
        }
        while (*p++ == *q++);
        com->best_match_pos = (uint16)(q-p-1);
        return (c_codetype)(q-nkey-1);
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
  }
  tpc=tp+nnode;
  current=tpc->c_left;
  tpc->c_left=tpc->c_right;
  tpc->c_right=current;
  return 0;
}
#endif
  
 

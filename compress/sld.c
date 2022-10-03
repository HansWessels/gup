/*
 * Sliding dictionary code voor ARJ
 * 
 * (c) 1995 Hans "Mr Ni! the Great" Wessels
 * 
*/

/*
 * Tree for sliding dictionary
 * 
 * (c) Copyright 1995 Hans Wessels
 * 
 * Wat is het idee van deze tree?
 * 
 * De nieuwste nodes zitten altijd boven aan, de beide childs van een node
 * zijn altijd ouder dan de node zelf. Dit heeft tot gevolg dat de leaves
 * van de tree de oudste nodes zijn.
 * 
 * Hierdoor vind je bij het matchen altijd de dichtbijzijndste match. Een
 * tweede voordeel is dat de oudste nodes altijd helemaal onderaan liggen,
 * op deze manier kun je ze makkelijk verwijderen, gewoon van de parent
 * afknippen.
 * 
*/

/*-
  De sliding dictionary en verscheidene HASH functies.

  configuratie:
  arjbeta -m7 -jm
  file: calgary corpus
  alle tijden in seconden
  de packed size was voor alle gevallen: 1024206 bytes
  rle detectie stond voor alle gevallen aan
  indien mogelijk is er een delta hash gebruikt

  HASH functie:                             | Tijd
  ------------------------------------------+-----
  0                                         | 1301
  x[0]                                      | 1074
  x[2]                                      | 1052
  (byte)((x[0]<<6)^(x[1]<<3)^x[2])          |  928
  (x[0]<<8)+x[1]                            |  901
  (x[1]<<8)+x[2]                            |  883
  ((x[1]&31)<<5)^((x[0]&31)<<10)^(x[2]&31)  |  797
  (x[0]<<12)^(x[1]<<6)^x[2]                 |  793
  (x[0]<<14)^(x[1]<<7)^(x[2])               |  828
  ((x[0]^x[1])<<8)^(x[1]^x[2])              |  794 ***
  CRC16 (van LZH)                           |  822

  Conclusie:
  *De hash functie heeft wel degelijk invloed op de pack snelheid.
  *Een hash die alle drie de characters gebruikt heeft de beste
    resultaten.
  *De nu gebruikte HASH functie (gemerkt met ***) is de beste. Deze
    heeft geen aparte rle detectie nodig en is daarom 4 seconden
    sneller dan hier staat aangegeven.
*/

#undef NDEBUG
#if 01
#define NDEBUG
#endif


#include <string.h>
#include <stddef.h>

#include "gup.h"
#include "encode.h"

#ifndef NDEBUG
#include <stdlib.h>
#include <stdio.h>
#define ARJ_Assert(expr)   \
  ((void)((expr)||(fprintf( stderr, \
  "\nAssertion failed: %s, file %s, line %d\n",\
   #expr, __FILE__, __LINE__ ),\
   ((int (*)(void))abort)())))
#else
#define ARJ_Assert( expr )
#endif

#define STARTMATCH 3 /* Matches van STARTMATCH en kleiner worden niet bekeken op bitafstand */

#define NO_NODE   NULL

#define HASH(x) (((*(x)^(x)[1])<<8) ^ ((x)[1]^(x)[2]))
#define DELTA_HASH(x) ((com->delta_hash<<8) ^ ((x)[-1]^*(x)))

/*
 * Als je voor de hash functie hash=((p[0] ^ p[1])<<8) + (p[1] ^ p[2])
 * gebruikt dan is het resultaat dan en alleen dan nul als de hash string
 * drie de zelfde bytes bevat. Voor deze runlength achtige entries zou je
 * speciale code kunnen ontwerpen om langzaam zoeken in linked [runlength]
 * lists te voorkomen. Denk hier bij aan een linked list met pointers naar
 * de eerste afwijkende byte voor de rl match. Iedere byte krijgt zo zijn
 * eigen list. Natuurlijk moet de list tegengesteld doorzocht worden om te
 * kijken of hij wel lang genoeg is voor de nieuwe entry.
*/

c_codetype insert2(uint8* nkey, uint16 pos, uint16 hash, uint16 rle_size, packstruct *com);
c_codetype insert(uint8* nkey, uint16 pos, uint16 hash, packstruct *com);
void insert2nm(uint8* nkey, uint16 pos, uint16 hash, packstruct *com);
void insertnm(uint8* nkey, uint16 pos, uint16 hash, packstruct *com);

c_codetype insert2_fast(uint8* nkey, uint16 rle_size, packstruct *com);
c_codetype insert_fast(uint8* nkey, uint16 pos, uint16 hash, packstruct *com);
void insertnm_fast(uint8* nkey, uint16 pos, uint16 hash, packstruct *com);


#ifndef INDEX_STRUCT
/*
  struct is linked with pointers
*/

#ifndef NOT_USE_STD_init_dictionary
void init_dictionary(packstruct *com)
{
  /*
   * initialiseert de sliding dictionary initialiseert de
   * slidingdictionary structuren en zet eerste byte in dictionary
  */

  /* reset hash table */
  memset(com->root.big, 0, HASH_SIZE * sizeof (node_type));
  /* Hier gaan we er dus vanuit dat NULL==0x00000000 */
  if(com->speed<2)
  {
    memset(com->root2.big, 0, HASH_SIZE * sizeof (node_type));
    /* Hier gaan we er dus vanuit dat NULL==0x00000000 */
    memset(com->link.big, 0, com->tree_size * sizeof (node_type));
    /* Hier gaan we er dus vanuit dat NULL==0x00000000 */
    com->hist_index=0;
  }
  com->last_pos = 2;
  com->del_pos = (uint16) (com->last_pos - com->maxptr - 1);
  if ((com->delta_hash = HASH(com->dictionary)) != 0)
  {
    com->rle_size = 0;
    insertnm(com->dictionary+2, 2, com->delta_hash, com);
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
      insert2nm(new_key+com->rle_size, 2, com->rle_hash, com);
    }
    if (new_key[com->rle_size] != com->rle_char)
    {
      com->rle_size--;
      com->rle_hash--;
    }
  }
}

#endif


#ifndef NOT_USE_STD_find_dictionary

#if 01  /* switch for testing, make false for testing */
/* normal code */
c_codetype find_dictionary(long insertpos, packstruct *com)
{
  /*
   * zoekt maximale match in dictionary bij pos update de
   * slidingdictionary structuren returnt max_match, als geen max_match
   * gevonden dan return 0 best_match_pos is de absolute positie van
   * max_match
  */
  uint16 pos=(uint16)insertpos;
  uint16 to_do = pos-com->last_pos;
  uint8* new_key = com->dictionary+insertpos-to_do;
  ARJ_Assert(((pos-com->last_pos)&0xffff)<=MAX_MATCH);
  { /*- wis de nodes */
    int i = to_do;
    uint16 key = com->del_pos;
    do
    {
      node_type *tp;

      tp = com->tree.big[key].parent;
      if (tp != NO_NODE)
      {
        if (*tp == com->tree.big + key)
        {
          *tp = NO_NODE;
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
      insert2nm(new_key+com->rle_size, pos, com->rle_hash, com);
      if (new_key[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        insertnm(new_key, pos, com->delta_hash, com);
      }
      else
      {
        uint8 orig;
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
        insert2nm(new_key+com->rle_size, pos, com->rle_hash, com);
        if (new_key[com->rle_size] != com->rle_char)
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
    uint8 orig = new_key[com->max_match];
    c_codetype res;
    if (com->rle_size > 0)
    {
      res=insert2(new_key, pos, com->rle_hash, com->rle_size, com);
      new_key[com->max_match] = orig;
      if (new_key[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        res=insert(new_key, pos, com->delta_hash, com);
        new_key[com->max_match] = orig;
      }
      else
      {
        uint8 *p;
        p = new_key;
        com->rle_char = *new_key;
        new_key[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 1);
        com->rle_hash = (com->rle_char << 8) + com->rle_size;
        res=insert2(new_key, pos, com->rle_hash, com->rle_size, com);
        new_key[com->max_match] = orig;
        if (new_key[com->rle_size] != com->rle_char)
        {
          com->rle_size--;
          com->rle_hash--;
        }
      }
    }
    return res;
  }
}

#else

/* testing code */
c_codetype find_dictionary(long insertpos, packstruct *com)
{
  /*
   * zoekt maximale match in dictionary bij pos update de
   * slidingdictionary structuren returnt max_match, als geen max_match
   * gevonden dan return 0 best_match_pos is de absolute positie van
   * max_match
  */
  uint16 pos=(uint16)insertpos;
  uint16 to_do = pos-com->last_pos;
  uint8* new_key = com->dictionary+insertpos-to_do;
  ARJ_Assert(((pos-com->last_pos)&0xffff)<=MAX_MATCH);
  { /*- wis de nodes */
    int i = to_do;
    uint16 key = com->del_pos;
    do
    {
      node_type *tp;

      tp = com->tree.big[key].parent;
      if (tp != NO_NODE)
      {
        if (*tp == com->tree.big + key)
        {
          *tp = NO_NODE;
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
      insert2nm(new_key+com->rle_size, pos, com->rle_hash, com);
      if (new_key[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        insertnm(new_key, pos, com->delta_hash, com);
      }
      else
      {
        uint8 orig;
        uint8 *p = new_key;
        com->rle_char = *new_key;
        orig = new_key[com->max_match];
        new_key[com->max_match] = ~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 1);
        new_key[com->max_match] = orig;
        com->rle_hash = (com->rle_char << 8) + com->rle_size;
        insert2nm(new_key+com->rle_size, pos, com->rle_hash, com);
        if (new_key[com->rle_size] != com->rle_char)
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
    uint8 orig = new_key[com->max_match];
    c_codetype res;
    if (com->rle_size > 0)
    {
      res=insert2(new_key, pos, com->rle_hash, com->rle_size, com);
      new_key[com->max_match] = orig;
      if (new_key[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
        com->rle_hash--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        res=insert(new_key, pos, com->delta_hash, com);
        new_key[com->max_match] = orig;
      }
      else
      {
        uint8 *p;
        p = new_key;
        com->rle_char = *new_key;
        new_key[com->max_match] = ~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 1);
        com->rle_hash = (com->rle_char << 8) + com->rle_size;
        res=insert2(new_key, pos, com->rle_hash, com->rle_size, com);
        new_key[com->max_match] = orig;
        if (new_key[com->rle_size] != com->rle_char)
        {
          com->rle_size--;
          com->rle_hash--;
        }
      }
    }
    if(res>0)
    {
      hist_struct *hist;
      hist=com->hist[com->hist_index];
      hist->match=1;
      hist->ptr=com->best_match_pos;
      return 3;
    }
    return res;
  }
}
#endif

#endif


#ifndef NOT_USE_STD_insert2nm

void insert2nm(uint8* nkey, uint16 nnode, uint16 hash, packstruct *com)
{
  node_type tp = com->tree.big + nnode;
  node_type *left = &(tp->c_left);
  node_type *right = &(tp->c_right);
  tp->parent = com->root2.big + hash;
  tp->key = nkey;
  {
    node_type tmp = com->root2.big[hash];
    com->root2.big[hash] = tp;
    if(tmp==NO_NODE)
    {
      *left=NO_NODE;
      *right=NO_NODE;
      return;
    }
    tp = tmp;
  }
  for(;;)
  {
    uint8 *p = tp->key;
    uint8 *q = nkey;
    {
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
        tp->key+=MAXD_MATCH;
        *left = tp->c_left;
        if (tp->c_left != NO_NODE)
        {
          (tp->c_left)->parent = left;
        }
        *right = tp->c_right;
        if (tp->c_right != NO_NODE)
        {
          (tp->c_right)->parent = right;
        }
        com->link.big[nnode] = tp;
        tp->parent = com->link.big + nnode;
        return;
      }
    }
    if((*--q)<(*--p))
    {
      /*
       * de key van nnode is kleiner dan die van tp, tp moet dus rechts
       * van nnode komen tp node wordt dus de linker child van rechts. de
       * nieuwe tp node wordt de linker child van tp
      */
      *right = tp;
      tp->parent = right;
      right = &(tp->c_left);
      if((tp = *right)==NO_NODE)
      {
        *left=NO_NODE;
        return;
      }
    }
    else
    {
      *left = tp;
      tp->parent = left;
      left = &(tp->c_right);
      if((tp = *left)==NO_NODE)
      {
        *right=NO_NODE;
        return;
      }
    }
  }
}

#endif


#ifndef NOT_USE_STD_insertnm

void insertnm(uint8* nkey, uint16 nnode, uint16 hash, packstruct *com)
{
  node_type tp = com->tree.big + nnode;
  node_type *left = &(tp->c_left);
  node_type *right = &(tp->c_right);
  tp->parent = com->root.big + hash;
  tp->key = nkey;
  {
    node_type tmp;
    tmp = com->root.big[hash];
    com->root.big[hash] = tp;
    if(tmp==NO_NODE)
    {
      *left=NO_NODE;
      *right=NO_NODE;
      return;
    }
    tp=tmp;
  }
  for(;;)
  {
    uint8 *p = tp->key;
    uint8 *q = nkey;
    {
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
        *left = tp->c_left;
        if (tp->c_left != NO_NODE)
        {
          (tp->c_left)->parent = left;
        }
        *right = tp->c_right;
        if (tp->c_right != NO_NODE)
        {
          (tp->c_right)->parent = right;
        }
        com->link.big[nnode] = tp;
        tp->key+=MAXD_MATCH;
        tp->parent = com->link.big + nnode;
        return;
      }
    }
    if((*--q)<(*--p))
    {
      /*
       * de key van nnode is kleiner dan die van tp, tp moet dus rechts
       * van nnode komen tp node wordt dus de linker child van rechts. de
       * nieuwe tp node wordt de linker child van tp
      */
      *right = tp;
      tp->parent = right;
      right = &(tp->c_left);
      if((tp = *right)==NO_NODE)
      {
        *left=NO_NODE;
        break;
      }
    }
    else
    {
      *left = tp;
      tp->parent = left;
      left = &(tp->c_right);
      if((tp = *left)==NO_NODE)
      {
        *right=NO_NODE;
        break;
      }
    }
  }
}

#endif


#ifndef NOT_USE_STD_insert2

c_codetype insert2(uint8* nkey, uint16 nnode, uint16 hash, uint16 rle_size, packstruct *com)
{
  c_codetype best_match;
  c_codetype opt_match;
  uint16 max_match=com->max_match;
  hist_struct *hist;
  node_type tp = com->tree.big + nnode;
  node_type *left = &(tp->c_left);
  node_type *right = &(tp->c_right);
  tp->parent = com->root2.big + hash;
  com->hist_index=((com->hist_index+1) & (HISTSIZE-1));
  hist=com->hist[com->hist_index];
  if (nkey[-3] == *nkey)
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
    #if 0
    /* 
      experimentele code 07-06-1997, optimaliseert backmatching bij
      rle cases.
      Resultaat: Gemiddeld lijkt er een kleine verbetering op te treden
      kost wel meer tijd.
    */
    int i=rle_size;
    node_type tmp;
    node_type *p=com->root2.big+(hash&0xff00U);
    /* hier alle key's afzoeken naar gevulde plaatsen, om hist in te vullen */
    do
    {
      if((tmp=*p++)!=NO_NODE)
      {
        hist->match=rle_size-i;
        hist->ptr=nkey-(tmp->key-rle_size+i+1);
        hist++;
      }
    }
    while(--i>0);
    #endif
    best_match=-4;
    opt_match=-2;
  }
  nkey+=rle_size;
  tp->key = nkey;
  {
    node_type tmp = com->root2.big[hash];
    com->root2.big[hash] = tp;
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
    *left = NO_NODE;
    *right = NO_NODE;
  }
  else
  {
    uint8* origkey=nkey;
    max_match-=rle_size;
    for(;;)
    {
      c_codetype match;
      uint8 *p = tp->key;
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
          *left = tp->c_left;
          if (tp->c_left != NO_NODE)
          {
            (tp->c_left)->parent = left;
          }
          *right = tp->c_right;
          if (tp->c_right != NO_NODE)
          {
            (tp->c_right)->parent = right;
          }
          {
            if (best_match == max_match)
            { /*- link met de link van deze node */
              uint16 tmp = (uint16) (tp - com->tree.big);
              com->link.big[nnode] = com->link.big[tmp];
              if (com->link.big[tmp] != NO_NODE)
              {
                com->link.big[tmp]->parent = com->link.big + nnode;
                com->link.big[tmp] = NO_NODE;
              }
            }
            else
            {
              max_match-=MAXD_MATCH;
              origkey+=MAXD_MATCH;
              best_match-=MAXD_MATCH;
              opt_match-=MAXD_MATCH;
              com->link.big[nnode] = tp;
              tp->key+=MAXD_MATCH;
              tp->parent = com->link.big + nnode;
              tp = com->link.big[tp - com->tree.big];
              while (tp != NO_NODE)
              {
                {
                  p = tp->key;
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
                tp = com->link.big[tp - com->tree.big];
              }
              opt_match+=MAXD_MATCH;
            }
          }
          return opt_match+rle_size;
        }
        else
        {
          if (best_match == max_match)
          { /*- we hebben een 100% match */
            *left = tp->c_left;
            if (tp->c_left != NO_NODE)
            {
              (tp->c_left)->parent = left;
            }
            *right = tp->c_right;
            if (tp->c_right != NO_NODE)
            {
              (tp->c_right)->parent = right;
            }
            return opt_match+rle_size;
          }
        }
      }
      if((*nkey) < (*--p))
      {
        /*
         * de key van nnode is kleiner dan die van tp, tp moet dus rechts
         * van nnode komen tp node wordt dus de linker child van rechts. de
         * nieuwe tp node wordt de linker child van tp
        */
        *right = tp;
        tp->parent = right;
        right = &(tp->c_left);
        if((tp = *right)==NO_NODE)
        {
          *left=NO_NODE;
          break;
        }
      }
      else
      {
        *left = tp;
        tp->parent = left;
        left = &(tp->c_right);
        if((tp = *left)==NO_NODE)
        {
          *right=NO_NODE;
          break;
        }
      }
      nkey=origkey;
    }
    opt_match+=rle_size;
  }
  return opt_match;
}

#endif


#ifndef NOT_USE_STD_insert

c_codetype insert(uint8* nkey, uint16 nnode, uint16 hash, packstruct *com)
{
  uint16 max_match=com->max_match-1;
  node_type tp = com->tree.big + nnode;
  node_type *left = &(tp->c_left);
  node_type *right = &(tp->c_right);
  tp->parent = com->root.big + hash;
  tp->key = nkey;
  {
    node_type tmp = com->root.big[hash];
    com->root.big[hash] = tp;
    tp = tmp;
  }
  if(tp==NO_NODE)
  {
    *left = NO_NODE;
    *right = NO_NODE;
    return 0;
  }
  else
  {
    c_codetype best_match=0;
    c_codetype opt_match=0;
    uint8 * origpos=nkey;
    hist_struct *hist;
    com->hist_index=((com->hist_index+1) & (HISTSIZE-1));
    hist=com->hist[com->hist_index];
    for(;;)
    {
      c_codetype match;
      {
        uint8 *p = tp->key;
        if (*p++ != *nkey++)
        {
          if((*--nkey)<(*--p))
          {
            /*
             * de key van nnode is kleiner dan die van tp, tp moet dus rechts
             * van nnode komen tp node wordt dus de linker child van rechts. de
             * nieuwe tp node wordt de linker child van tp
            */
            *right = tp;
            tp->parent = right;
            right = &(tp->c_left);
            if((tp = *right)==NO_NODE)
            {
              *left=NO_NODE;
              break;
            }
          }
          else
          {
            *left = tp;
            tp->parent = left;
            left = &(tp->c_right);
            if((tp = *left)==NO_NODE)
            {
              *right=NO_NODE;
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
              *left = tp->c_left;
              if (tp->c_left != NO_NODE)
              {
                (tp->c_left)->parent = left;
              }
              *right = tp->c_right;
              if (tp->c_right != NO_NODE)
              {
                (tp->c_right)->parent = right;
              }
              {
                if (best_match == com->max_match)
                { /* link met de link van deze node */
                  uint16 tmp = (uint16) (tp - com->tree.big);
                  com->link.big[nnode] = com->link.big[tmp];
                  if (com->link.big[tmp] != NO_NODE)
                  {
                    com->link.big[tmp]->parent = com->link.big + nnode;
                    com->link.big[tmp] = NO_NODE;
                  }
                }
                else
                {
                  max_match-=MAXD_MATCH-1;
                  origpos+=MAXD_MATCH;
                  best_match-=MAXD_MATCH;
                  opt_match-=MAXD_MATCH;
                  com->link.big[nnode] = tp;
                  tp->key+=MAXD_MATCH;
                  tp->parent = com->link.big + nnode;
                  tp = com->link.big[tp - com->tree.big];
                  while (tp != NO_NODE)
                  {
                    {
                      p=tp->key;
                      nkey=origpos;
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
                    tp = com->link.big[tp - com->tree.big];
                  }
                  opt_match+=MAXD_MATCH;
                }
              }
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
            *right = tp;
            tp->parent = right;
            right = &(tp->c_left);
            if((tp = *right)==NO_NODE)
            {
              *left=NO_NODE;
              break;
            }
          }
          else
          {
            *left = tp;
            tp->parent = left;
            left = &(tp->c_right);
            if((tp = *left)==NO_NODE)
            {
              *right=NO_NODE;
              break;
            }
          }
          nkey=origpos;
        }
      }
    }
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
  uint16 pos=(uint16)insertpos;
  uint16 to_do = pos-com->last_pos;
  uint8* new_key = com->dictionary+insertpos-to_do;
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
      node_type *tp;
      tp = com->tree.big[key].parent;
      if (tp != NO_NODE)
      {
        if (*tp == com->tree.big + key)
        {
          *tp = NO_NODE;
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
      if (new_key[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        insertnm_fast(new_key, pos, com->delta_hash, com);
      }
      else
      {
        uint8 orig;
        uint8 *p;
        p = new_key;
        com->rle_char = *new_key;
        orig = new_key[com->max_match];
        new_key[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        new_key[com->max_match] = orig;
        com->rle_size = (uint16)(p - new_key - 1);
        if (new_key[com->rle_size] != com->rle_char)
        {
          com->rle_size--;
        }
      }
    }
  }
  pos++;
  new_key++;
  {
    uint8 orig = new_key[com->max_match];
    c_codetype res;
    if (com->rle_size > 0)
    {
      if (new_key[-3] == *new_key)
      {
        com->best_match_pos = 0;
        res=com->rle_size+2;
      }
      else
      {
        res= 0;
      }
      if (new_key[com->rle_size] != com->rle_char)
      {
        com->rle_size--;
      }
    }
    else
    {
      if ((com->delta_hash = DELTA_HASH(new_key)) != 0)
      {
        res=insert_fast(new_key, pos, com->delta_hash, com);
        new_key[com->max_match] = orig;
      }
      else
      {
        uint8 *p;
        p = new_key;
        com->rle_char = *new_key;
        new_key[com->max_match] = (uint8)~com->rle_char;
        while (*p++ == com->rle_char)
        {
          ;
        }
        com->rle_size = (uint16)(p - new_key - 1);
        new_key[com->max_match] = orig;
        if (new_key[com->rle_size] != com->rle_char)
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

void insertnm_fast(uint8* nkey, uint16 nnode, uint16 hash, packstruct *com)
{
  node_type tp = com->tree.big + nnode;
  node_type *left = &(tp->c_left);
  node_type *right = &(tp->c_right);
  tp->parent = com->root.big + hash;
  tp->key = nkey;
  {
    node_type tmp;
    tmp = com->root.big[hash];
    com->root.big[hash] = tp;
    if(tmp==NO_NODE)
    {
      *left=NO_NODE;
      *right=NO_NODE;
      return;
    }
    tp = tmp;
  }
  {
    uint8 first=*nkey;
    for(;;)
    {
      if((first)<(*tp->key))
      {
        /*
         * de key van nnode is kleiner dan die van tp, tp moet dus rechts
         * van nnode komen tp node wordt dus de linker child van rechts. de
         * nieuwe tp node wordt de linker child van tp
        */
        *right = tp;
        tp->parent = right;
        right = &(tp->c_left);
        if((tp = *right)==NO_NODE)
        {
          *left=NO_NODE;
          break;
        }
      }
      else
      {
        if((first)>(*tp->key))
        {
          *left = tp;
          tp->parent = left;
          left = &(tp->c_right);
          if((tp = *left)==NO_NODE)
          {
            *right=NO_NODE;
            break;
          }
        }
        else
        { /*- we hebben een 100% match */
          *left = tp->c_left;
          if (tp->c_left != NO_NODE)
          {
            (tp->c_left)->parent = left;
          }
          *right = tp->c_right;
          if (tp->c_right != NO_NODE)
          {
            (tp->c_right)->parent = right;
          }
          return;
        }
      }
    }
  }
}

#endif


#ifndef NOT_USE_STD_insert_fast

c_codetype insert_fast(uint8* nkey, uint16 nnode, uint16 hash, packstruct *com)
{
  node_type tp = com->tree.big + nnode;
  node_type *left = &(tp->c_left);
  node_type *right = &(tp->c_right);
  tp->parent = com->root.big + hash;
  tp->key = nkey;
  {
    node_type tmp = com->root.big[hash];
    com->root.big[hash] = tp;
    tp = tmp;
  }
  if(tp==NO_NODE)
  {
    *left = NO_NODE;
    *right = NO_NODE;
    return 0;
  }
  else
  {
    uint8 first=*nkey++;
    for(;;)
    {
      uint8 *p = tp->key;
      if(first != *p++)
      {
        if (first < *--p)
        {
          /*
           * de key van nnode is kleiner dan die van tp, tp moet dus rechts
           * van nnode komen tp node wordt dus de linker child van rechts. de
           * nieuwe tp node wordt de linker child van tp
          */
          *right = tp;
          tp->parent = right;
          right = &(tp->c_left);
          if((tp = *right)==NO_NODE)
          {
            *left=NO_NODE;
            return 0;
          }
        }
        else
        {
          *left = tp;
          tp->parent = left;
          left = &(tp->c_right);
          if((tp = *left)==NO_NODE)
          {
            *right=NO_NODE;
            return 0;
          }
        }
      }
      else
      {
        *left = tp->c_left;
        if (tp->c_left != NO_NODE)
        {
          (tp->c_left)->parent = left;
        }
        *right = tp->c_right;
        if (tp->c_right != NO_NODE)
        {
          (tp->c_right)->parent = right;
        }
        nkey[com->max_match-1] = (uint8)~p[com->max_match-1];  /* sentinel */
        {
          uint8 *q = nkey;
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
          com->best_match_pos = nkey-p-1;
          return (c_codetype)(nkey-q+2);
        }
      }
    }
  }
}

#endif


#endif
 

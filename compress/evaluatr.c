/*-
 * sliding dictionary evaluator
 * (c) 1996 Mr Ni! (the Great) of the TOS-crew
 */

/*-
 * Slidingdictionarylogie, wat doen we met de sliding dictionary
 *
 * De volgende structuur levert de beste match op. Het grote probleem is
 * sat de slidingdictionary gevoerd moet worden met nieuwe karakters. Dit
 * houdt in dat je op iedere positie maar een keer kunt matchen. Daarom
 * moeten positie-lengte paren en literals worden doorgegeven  zodat er
 * later weer gebruik van kan worden gemaakt.
 *
 * Idee van pointer swapping:
 *   als match 0 en match 1 het zelfde zijn kun je de dichtst bijzijnde
 *   match nemen als na de pointer length een literal komt.
 *   de keuze wordt dus:
 *         pointer length, literal
 *     of: literal, pointer length
 * Gegeneraliseerde pointer swapping:
 *     if (match0+match3 == match1+match4)
 *     {
 *       if((LOG(ptr0)+LOG(ptr3) > LOG(ptr1)+LOG(ptr4))
 *       {
 *         als keuze valt op match0
 *           dan ptrswap=1
 *           anders ptrswap=0
 *       }
 *       else
 *       {
 *         als keuze valt op match1
 *           dan ptrswap=-1
 *           anders ptrswap=0
 *       }
 *     }
 *     else
 *     {
 *       ptrswap=0
 *     }
 *   Uitwerking van gezette pointerswap:
 *     if (pointer length wordt opgeslagen)
 *     {
 *       if(ptrswap<0)
 *       {
 *         UNSTORE_MATCH();
 *         UNSTORE_LITERAL();
 *         STORE_MATCH(swapmatch0);
 *         STORE_LITERAL();
 *         STORE_MATCH(swapmatch1);
 *         if(swapmatch1=match0)
 *         {  we hebben dus eigenlijk match 0 opgeslagen
 *           bereken of een pointerswap mogelijk is
 *         }
 *         else
 *         {
 *           ptrswap=0;
 *         }
 *       }
 *       else
 *       {
 *         STORE_MATCH(match0)
 *         bereken of een pointerswap mogelijk is
 *       }
 *     }
 *     else
 *     { er wordt dus een literal, pointer length opgeslagen
 *       if(ptrswap>0)
 *       {
 *         UNSTORE_MATCH();
 *         STORE_LITERAL();
 *         STORE_MATCH(swapmacth0);
 *         STORE_MATCH(swapmatch1);
 *         ptrswap=0;
 *       }
 *       else
 *       {
 *         if((match0==match1) && (ptr0<ptr1))
 *         {
 *           STORE_MATCH(match0)
 *           STORE_LITERAL();
 *         }
 *         else
 *         {
 *           STORE_LITERAL();
 *           STORE_MATCH(match1);
 *           bereken of een pointerswap mogelijk is
 *         }
 *       }
 *     }
 *
 * match backtracking:
 *   sla bij het matchen de hele zoek geschiedenis op, dan krijg je een tabel
 *   van steeds langere matches die steeds verder weg komen te liggen.
 *   Als de volgende match aansluitend is (geen literal er tussen) dan
 *   kun je deze match verder terug proberen te zoeken. Op die manier wordt
 *   match0 kleiner en hopelijk komt deze op een dichter bij gelegen match uit.
 *   De pointer voor de volgende match blijft het zelfde, alleen wordt de match
 *   wel langer. Helaas kun je dit grapje niet twee maal herhalen want je kent
 *   de back track geschiedens van de nieuwe match positie niet.
 *   Pas op!
 *   match0 moet wel groter of gelijk aan drie blijven
 * Nog een manier om processortijd te doden:
 *   bij twee opeenvolgende matches: bepaal de maximale lengte waarover
 *   de twee matches overlappen. Maak die lengte zodanig dat de minimale lengte
 *   noch de maximale lengte overschreden wordt. Nu kun je de ene match langer 
 *   maken en de andere korter. Zoek hierbij de minimale som voor de lengte 
 *   codes.
 *
 */


#undef NDEBUG
#if 01
#define NDEBUG /* no debugging */
#endif

#include "gup.h"
#include "encode.h"
#include "evaluatr.h"

#if ((DIC_START_SIZE)>(DIC_SIZE))
  #error "Dictionary start size > dictionary size"
#endif
#if ((DIC_DELTA_SIZE)>(DIC_SIZE/4))
  #error "Dictionary delta size > (dictionary size)/4"
#endif
#if ((DIC_DELTA_SIZE)>(DIC_START_SIZE))
  #error "Dictionary delta size > dictionary start size"
#endif

#ifndef ALIGN_BUFP
  /*
   * x=packstruct pointer.
   * Function to allign buffer_pointer pointers, needed in some optimisations of
   * storebits, oly align when in the packstruct use_align!=0 
   */
  #define ALIGN_BUFP(x) /* */
#endif

#ifndef NDEBUG
#define ARJ_Assert_ZEEF34()     \
  if (((com->msp - com->matchstring) < 0)\
      || ((com->msp - com->matchstring) >= ((com->hufbufsize+4UL)*4))\
      || (com->ptrp-com->pointers)!=((com->msp-com->matchstring)/4))\
  {\
    fprintf(assert_redir_fptr, \
    "\nAssertion ZEEF34 failed: file %s, line %d\n"\
    "com->pointers = %8p, com->ptrp = %8p, com->ptrp - com->pointers = %8ld\n"\
    "com->msp = %8p, com->matchstring = %8p, com->msp - com->matchstring = %8ld, .../4 = %8ld\n",\
    __FILE__, __LINE__,\
    (void *)com->pointers,\
    (void *)com->ptrp,\
    (long)(com->ptrp - com->pointers),\
    (void *)com->msp,\
    (void *)com->matchstring,\
    (long)(com->msp - com->matchstring),\
    (long)((com->msp - com->matchstring)/4));\
    abort();\
  }
#else
#define ARJ_Assert_ZEEF34()
#endif

#define FMATCH(pos)                                       \
  {                                                       \
    uint8 *pp=com->dictionary+(pos);                      \
    uint8 *qq=com->msp;                                   \
    *qq++=*pp++;                                          \
    *qq++=*pp++;                                          \
    *qq++=*pp++;                                          \
    *qq++=*pp;                                            \
    com->msp=qq;                                          \
  }

#define DMSP() com->msp-=4


#ifndef NOT_USE_STD_backmatch_big

/*
// backmatch functie
// pos: positie in dictionary waar backmatch van berekend moet worden
// ptr: pointer van de match
// hist: index in hist array, -1 -> hist array is niet meer geldig
// com: packstruct, functie maakt gebruik van packstruct data 
// en kan deze ook wijzigen
// match is al opgeslagen -> charp[-1]=match
//
// return: lengte waarover backmatch correctie is uitgevoerd
*/
int backmatch_big(long pos, uint16 ptr, int hist, packstruct* com)
{
  ARJ_Assert(com->ptrp>=(com->pointers+1));
  ARJ_Assert(com->charp>=(com->chars+2));
  if(com->charp[-2]>=NLIT)
  { /* backmatch heeft alleen zin als ervoor ook een match ligt */
    int len=com->max_match-com->charp[-1]+NLIT-MIN_MATCH+2; /* +2 voor max_match correctie */
    uint8 *pp=com->dictionary+ --pos;
    uint8 *qq;

    ARJ_Assert(com->ptrp>=(com->pointers+2));
    if((pos-=ptr)<0)
    {
      qq=com->dictionary+pos-1+DIC_SIZE;
      if(pp-len<com->dictionary)
      {/* fix voor over nul terugkijken */
        pp+=DIC_SIZE;
      }
    }
    else
    {
      qq=com->dictionary+pos-1;
      if(pos<len)
      {/* fix voor over nul terugkijken */
        len=(int)pos;
      }
    }
    if(len>0)
    { /* maximale backmatch lengte >0 */
      ARJ_Assert(pp<(com->dictionary+DIC_SIZE + MAX_MATCH*4 + 6UL));
      ARJ_Assert(pp>=com->dictionary);
      ARJ_Assert(qq<(com->dictionary+DIC_SIZE + MAX_MATCH*4 + 6UL));
      ARJ_Assert(qq>=com->dictionary);
      if(*pp==*qq)
      { /* backmatch */
        len=-len;
        {
          uint8* origp=qq;
          uint8 orig=qq[len];
          qq[len]=~pp[len]; /* sentinel */
          do
          {
            if(*--pp!=*--qq) break;
            if(*--pp!=*--qq) break;
            if(*--pp!=*--qq) break;
            if(*--pp!=*--qq) break;
            if(*--pp!=*--qq) break;
            if(*--pp!=*--qq) break;
            if(*--pp!=*--qq) break;
          }
          while(*--pp==*--qq);
          ARJ_Assert(pp<(com->dictionary+DIC_SIZE + MAX_MATCH*4 + 6UL));
          ARJ_Assert(pp>=com->dictionary-1);
          ARJ_Assert(qq<(com->dictionary+DIC_SIZE + MAX_MATCH*4 + 6UL));
          ARJ_Assert(qq>=com->dictionary-1);
          origp[len]=orig;
          len=(int)(origp-qq);
          ARJ_Assert(((len+com->charp[-1]-NLIT+MIN_MATCH-2)<=com->max_match));
        }
        if(hist>=0)
        { /* history aanwezig. Maak pointerlen zo kort mogelijk */
          hist_struct *histp=com->hist[hist];
          uint8 ptrlen=LOG(com->ptrp[-2]);
          int origlen=com->charp[-2]-NLIT+MIN_MATCH-2; /* correctie voor +2 in encode_big */
          while(LOG(histp->ptr)<ptrlen)
          {
            if((origlen-histp->match) <=len)
            { /* back macth! */
              c_codetype diff;
              ptrlen=LOG(histp->ptr);
              while(LOG(histp[1].ptr)==ptrlen)
              { /* neem langste match, meeste lenteoptimalisatie mogelijkheden */
                histp++;
              }
              diff=(c_codetype)(origlen-histp->match);
              com->charp[-2]-=diff;
              com->charp[-1]+=diff;
              len-=diff;
              *com->bmp++=(uint8)len;
              ARJ_Assert(len<(com->max_match-MIN_MATCH));
              com->ptrp[-2]=histp->ptr;
              if(com->charp[-3]>=NLIT)
              { /* herbackmatch gewijzigde pointer */
                /* backmatch_big(pos+ptr-origlen-1, com->ptrp[-2], -1, com); */
                ARJ_Assert(com->ptrp>=(com->pointers+2));
                ARJ_Assert(com->charp>=(com->chars+3));

                len=com->max_match-com->charp[-2]+NLIT-MIN_MATCH+2; /* +2 voor max_match correctie */
                pos+=ptr;
                pos-=origlen+2;
                pp=com->dictionary+pos;

                if((pos-=com->ptrp[-2])<0)
                {
                  qq=com->dictionary+pos-1+DIC_SIZE;
                  if(pp-len<com->dictionary)
                  {/* fix voor over nul terugkijken */
                    pp+=DIC_SIZE;
                  }
                }
                else
                {
                  qq=com->dictionary+pos-1;
                  if(pos<len)
                  {/* fix voor over nul terugkijken */
                    len=(int)pos;
                  }
                }
                if((len>0) && (*pp==*qq))
                { /* backmatch */
                  uint8* origp=qq;
                  uint8 orig;
                  len=-len;
                  orig=qq[len];
                  qq[len]=~pp[len]; /* sentinel */
                  do
                  {
                    if(*--pp!=*--qq) break;
                    if(*--pp!=*--qq) break;
                    if(*--pp!=*--qq) break;
                    if(*--pp!=*--qq) break;
                    if(*--pp!=*--qq) break;
                    if(*--pp!=*--qq) break;
                    if(*--pp!=*--qq) break;
                  }
                  while(*--pp==*--qq);
                  origp[len]=orig;
                  com->bmp[-2]=(uint8)(origp-qq);
                }
                else
                {
                  com->bmp[-2]=0;
                }
              }
              ARJ_Assert((com->charp[-2]-NLIT+MIN_MATCH-2)<=com->max_match);
              ARJ_Assert((com->charp[-2]-NLIT+MIN_MATCH)>=MIN_MATCH);
              ARJ_Assert((com->charp[-1]-NLIT+MIN_MATCH-2)<=com->max_match);
              ARJ_Assert((com->charp[-1]-NLIT+MIN_MATCH)>=MIN_MATCH);
              return diff;
            }
            else
            {
              histp++;
            }
          }
        }
        *com->bmp++=(uint8)len;
        return 0;            
      }
    }
  }
  *com->bmp++=0;
  return 0;
}

#endif


#define DBMP() com->bmp--

/*
 * inline (un)store literal en store pointer length functies alleen
 * bedoeld voormode 1 en 7
*/
#define STORE_LITERAL()                                   \
{                                                         \
  ARJ_Assert(com->ptrp>=com->pointers);                   \
  ARJ_Assert(com->charp>=com->chars);                     \
  ARJ_Assert((long)current_pos<(long)(DIC_SIZE + MAX_MATCH*4 + 6UL)); \
  ARJ_Assert((long)current_pos>=-4);/* restrictie alleen in big*/\
  *com->charp++=com->dictionary[current_pos++];           \
  bytes_to_do--;                                          \
  refill_count--;                                         \
  ARJ_Assert_ZEEF34();                                    \
  ARJ_Assert((com->ptrp-com->pointers)==                  \
             ((com->msp-com->matchstring)/4));            \
  ARJ_Assert(((int)com->dictionary[current_pos]<NLIT));   \
  ARJ_Assert(((int)com->dictionary[current_pos]>=0));     \
  ARJ_Assert((com->charp[-1]<NLIT));                      \
  ARJ_Assert((com->charp[-1]>=0));                        \
  ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));\
  ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));\
}

#define UNSTORE_LITERAL()                                 \
{                                                         \
  ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));\
  ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));\
  com->charp--;                                           \
  current_pos--;                                          \
  bytes_to_do++;                                          \
  refill_count++;                                         \
  ARJ_Assert(com->ptrp>=com->pointers);                   \
  ARJ_Assert(com->charp>=com->chars);                     \
  ARJ_Assert_ZEEF34();                                    \
}

#define STORE_MATCH(ptr, length, hist)                    \
{                                                         \
  ARJ_Assert((ptr)<=com->maxptr);                         \
  ARJ_Assert((length)>=MIN_MATCH);                        \
  ARJ_Assert((length)<=(com->max_match+2));/* +2 only for medium and big */\
  ARJ_Assert(com->ptrp>=com->pointers);                   \
  ARJ_Assert(com->charp>=com->chars);                     \
  *com->charp++=length+NLIT-MIN_MATCH;                    \
  *com->ptrp++=ptr;                                       \
  {                                                       \
    int res;                                              \
    if((res=BACKMATCH_ENCODESIZE(current_pos,             \
                                 ptr, hist_1, com))!=0)   \
    {                                                     \
      ptrswap=0;                                          \
      hist_1=-1;                                          \
      FMATCH(current_pos-res);                            \
    }                                                     \
    else                                                  \
    {                                                     \
      FMATCH(current_pos);                                \
      hist_1=hist;                                        \
    }                                                     \
  }                                                       \
  bytes_to_do-=length;                                    \
  refill_count-=length;                                   \
  current_pos+=length;                                    \
  ARJ_Assert_ZEEF34();                                    \
  ARJ_Assert((com->ptrp-com->pointers)==                  \
             ((com->msp-com->matchstring)/4));            \
  ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));\
  ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));\
}

/* store match without backmatch */
#define STORE_MATCH_NB(ptr, length, hist)                 \
{                                                         \
  ARJ_Assert((ptr)<=com->maxptr);                         \
  ARJ_Assert((length)>=MIN_MATCH);                        \
  ARJ_Assert((length)<=(com->max_match+2));/* +2 only for medium and big */\
  ARJ_Assert(com->ptrp>=com->pointers);                   \
  ARJ_Assert(com->charp>=com->chars);                     \
  *com->charp++=length+NLIT-MIN_MATCH;                    \
  *com->ptrp++=ptr;                                       \
  *com->bmp++=0;                                          \
  FMATCH(current_pos);                                    \
  hist_1=hist;                                            \
  bytes_to_do-=length;                                    \
  refill_count-=length;                                   \
  current_pos+=length;                                    \
  ARJ_Assert_ZEEF34();                                    \
  ARJ_Assert((com->ptrp-com->pointers)==                  \
             ((com->msp-com->matchstring)/4));            \
  ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));\
  ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));\
}

#define UNSTORE_MATCH()                                   \
{                                                         \
  c_codetype length=(*--com->charp)-NLIT+MIN_MATCH;       \
  ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));\
  ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));\
  DMSP();                                                 \
  DBMP();                                                 \
  com->ptrp--;                                            \
  hist_1=hist_2;                                          \
  hist_2=-1;                                              \
  bytes_to_do+=length;                                    \
  refill_count+=length;                                   \
  current_pos-=length;                                    \
  ARJ_Assert(com->ptrp>=com->pointers);                   \
  ARJ_Assert(com->charp>=com->chars);                     \
  ARJ_Assert_ZEEF34();                                    \
}


#ifndef NOT_USE_STD_encode_big
#undef BACKMATCH_ENCODESIZE
#define BACKMATCH_ENCODESIZE backmatch_big

gup_result encode_big(packstruct *com)
{
  /*-
   * Waarschuwing: !!! rbuf_current kan 4 bytes te ver staan na compressie
   *               !!! als dit net over een grens heen is
   *               !!! kan rbuf_current dus in een geheel nieuw blok wijzen
   */
  /*
   * Controle:
   *
   * Hoe moet current pos tov refill_pos staan?
   * refill_pos+4*MAX_MATCH+6 geeft de positie aan waar de volgende 
   * keer data in de dictionary wordt gezet. We moeten er dus voor 
   * zorgen dat er nooit dictionary vergeleken wordt voorbij deze 
   * positie. Worst case situatie bij het zoeken van een macth 0:
   * match0 < MAX_MATCH   (current_pos)
   * match1 == MAX_MATCH  (current_pos+1)
   * match2 == ????       (current_pos+match0)
   * match3 == ????       (current_pos+match0+1)
   * match4 == ????       (current_pos+1+MAX_MATCH)
   * match5 == MAX_MATCH  (current_pos+1+MAX_MATCH+1)
   * Maximaal wordt er dus op positie current_pos+2*MAX_MATCH+2 gekeken,
   * althans, daar wordt de sentry neer gezet, die willen we ook mee 
   * laten doen. Dus current_pos+2*MAX_MATCH+2 moet kleiner zijn dan 
   * refill_pos+4*MAX_MATCH+6 behalve in het begin, dan staat refill_pos
   * dichter bij, maar dan moet eerst de volledige dictionary worden door gewerkt.
   * in dit geval is refill_count > (DIC_DELTA_SIZE)
   * Als refill_pos+4*MAX_MATCH+6 kleiner is dan current_pos betekent dit dat de 
   * tot het einde is gevuld, current_pos+2*MAX_MATCH+2 moet altijd kleiner 
   * zijn dan DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL - 4UL, de dictionary 
   * size minus vier, deze vier bytes staan aan het begin van de 
   * dictionary.
   * In het geval dat refill_pos+4*MAX_MATCH+6 kleiner is dan current_pos moet er 
   * 65536+2*MAX_MATCH bytes tussen refill_pos+4*MAX_MATCH+6 en current pos zitten, 
   * nl de hele 64k dictionary en 2*MAX_MATCH bytes wegens back_matches zit er 
   * tussen. Dit geldt echter niet voor de begin situatie, omdat dan de hele 
   * dictionary in een keer gevuld wordt en er nog geen 64k aan dictionary 
   * verwerkt is. In dit geval is de variabele refill_count groter dan 
   * DIC_DELTA_SIZE omdat er gevuld wordt als refill_count <= 0 en er wordt dan 
   * met DIC_DELTA_SIZE bytes gevuld.
   * Verder mag bij een match current_pos niet kleiner dan nul zijn.
   * Dus het is misse boel als een van de volgende uitdrukkingen waar is:
   * (((refill_pos+4*MAX_MATCH+6)>current_pos) && ((current_pos+2*MAX_MATCH+2)>=(refill_pos+4*MAX_MATCH+6)) && (refill_count<=DIC_DELTA_SIZE))
   * ((current_pos+2*MAX_MATCH+2)>=(DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL - 4UL))
   * (((refill_pos+4*MAX_MATCH+6)<current_pos) && ((current_pos-(refill_pos+4*MAX_MATCH+6))<(65536L+2*MAX_MATCH)) && (refill_count<=DIC_DELTA_SIZE))
   * (current_pos<0)
   * voor de asserts moeten we dus bovenstaande uitdrukkingen ontkennen.
   */

  uint16 huffbuf = com->hufbufsize + 2;  /* +2 voor pointerSWAP, pswp kan 2 waarde terughalen */
  long current_pos = 0;                /* wijst de te packen byte aan        */
  long refill_pos =M17(DIC_START_SIZE);
  unsigned long bytes_to_do;
  /* 
    refill count:
    om betrouwbare backmatches te krijgen moet een volledige sessie
    unstores mogenlijk zijn. Dat is: twee maal max_match, plus twee
    maal een literal.
    dus: DIC_START_SIZE+2*MAX_MATCH+6
    
    de maximale buffer vulling moet zo zijn dat er twee maal max_match
    vooruit kan worden gekeken plus nog eens twee literals
    dus: DIC_START_SIZE+4*MAX_MATCH
  */
  signed long refill_count = DIC_START_SIZE+2*MAX_MATCH;

  com->charp = com->chars;
  com->ptrp = com->pointers;
  { /*- dictionary buffer vullen */
    long byte_count;

    if ((byte_count = com->buf_read_crc(DIC_START_SIZE+4*MAX_MATCH+6, com->dictionary, com->brc_propagator)) < 0)
    {
      return GUP_READ_ERROR; /* ("Read error"); */
    }
    else
    {
      if (byte_count == 0)
      {
        com->packed_size = com->bytes_packed = 0;
        return GUP_OK;
      }
      #ifndef PP_AFTER
      com->print_progres(byte_count, com->pp_propagator);
      #endif
    }
    bytes_to_do = byte_count;
  }
  com->max_match-=2; /* de sld werkt met de max_match minus twee waarde */
  init_dictionary(com);
  com->packed_size = 0;
  com->bits_rest = 0;
  com->bitbuf = 0;
  com->bits_in_bitbuf = 0;
  ALIGN_BUFP(com);
  *com->charp++ = com->dictionary[current_pos++]; /* eerste karakter heeft geen match */
  bytes_to_do--;
  refill_count--;
  if(com->speed==2)
  { /* fastest least compression */
    c_codetype match0;
    pointer_type ptr0;
    while (bytes_to_do)
    { /*- hoofd_lus, deze lus zorgt voor al het pack werk */
      /* Een assert voor alle find_dictionary's. Tussen door wordt current pos niet aangepast */
      ARJ_Assert(!(((refill_pos+4*MAX_MATCH+6)>current_pos) && ((current_pos+2*MAX_MATCH+2)>=(refill_pos+4*MAX_MATCH+6)) && (refill_count<=DIC_DELTA_SIZE)));
      ARJ_Assert(!((current_pos+2*MAX_MATCH+2)>=(DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL - 4UL)));
      ARJ_Assert(!(((refill_pos+4*MAX_MATCH+6)<current_pos) && ((current_pos-(refill_pos+4*MAX_MATCH+6))<(65536L+2*MAX_MATCH)) && (refill_count<=DIC_DELTA_SIZE)));
      ARJ_Assert(!(current_pos<0));
      match0 = find_dictionary_fast(current_pos+2, com);
      if(match0>0)
      {
        if ((unsigned long)match0 >= bytes_to_do)
        {
          match0 = (c_codetype) bytes_to_do;
          if (match0 < MIN_MATCH)
          {
            *com->charp++ = com->dictionary[current_pos++];
            bytes_to_do--;
            refill_count--;
          }
          else
          {
            ptr0 = com->best_match_pos;
            *com->charp++ = match0 + NLIT - MIN_MATCH;
            *com->ptrp++ = ptr0;
            bytes_to_do -= match0;
            refill_count -= match0;
            current_pos += match0;
          }
        }
        else
        {
          c_codetype match1;
          ptr0 = com->best_match_pos;
          match1 = find_dictionary_fast(current_pos + 3, com);
          if ((unsigned long)match1 >= bytes_to_do)
          {
            match1 = (c_codetype)(bytes_to_do - 1);
            if (match1 < MIN_MATCH)
            {
              match1 = 0;
            }
          }
          if (match1 > match0)
          { /*- eerst literal, dan match */
            ARJ_Assert(com->ptrp>=com->pointers);
            ARJ_Assert(com->charp>=com->chars);
            ARJ_Assert(current_pos<(DIC_SIZE + MAX_MATCH*4 + 6UL));
            ARJ_Assert(current_pos>=0);
            *com->charp++=com->dictionary[current_pos++];
            bytes_to_do--;
            refill_count--;
            ARJ_Assert(((int)com->dictionary[current_pos]<NLIT));
            ARJ_Assert(((int)com->dictionary[current_pos]>=0));
            ARJ_Assert((com->charp[-1]<NLIT));
            ARJ_Assert((com->charp[-1]>=0));
            ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));
            ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));
            match0 = match1;
            ptr0 = com->best_match_pos;
          }
          ARJ_Assert((ptr0)<=com->maxptr);
          ARJ_Assert((match0)>=MIN_MATCH);
          ARJ_Assert((match0)<=(com->max_match+2));
          ARJ_Assert(com->ptrp>=com->pointers);
          ARJ_Assert(com->charp>=com->chars);
          *com->charp++ = match0 + NLIT - MIN_MATCH;
          *com->ptrp++ = ptr0;
          bytes_to_do -= match0;
          refill_count -= match0;
          current_pos += match0;
          ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));
          ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));
        }
      }
      else
      {
        ARJ_Assert(com->ptrp>=com->pointers);
        ARJ_Assert(com->charp>=com->chars);
        ARJ_Assert(current_pos<(DIC_SIZE + MAX_MATCH*4 + 6UL));
        ARJ_Assert(current_pos>=0);
        *com->charp++=com->dictionary[current_pos++];
        bytes_to_do--;
        refill_count--;
        ARJ_Assert(((int)com->dictionary[current_pos]<NLIT));
        ARJ_Assert(((int)com->dictionary[current_pos]>=0));
        ARJ_Assert((com->charp[-1]<NLIT));
        ARJ_Assert((com->charp[-1]>=0));
        ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));
        ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));
      }
      if (com->charp > com->chars + huffbuf)
      { /*- 64k blok vol gepropt, ga de rotzooi opslaan */
        com->max_match+=2; /* herstel max_match */
        ARJ_Assert(com->max_match<=MAX_MATCH);
        {
          gup_result res;
          if((res=com->compress(com))!=GUP_OK)
          {
            return res;
          }
        }
        if (com->mv_mode)
        {
          if (com->mv_next)
          {
            {
              gup_result res;
              if((res=com->close_packed_stream(com))!=GUP_OK)   /* flush bitbuf */
              {
                return res;
              }
            }
            return GUP_OK;
          }
        }
        com->max_match-=2; /* de sld werkt met de max_match minus twee waarde */
        ARJ_Assert(com->max_match>=MIN_MATCH);
      }
      if (refill_count <= 0)
      {
        long byte_count = 0;
        refill_count += DIC_DELTA_SIZE;
        ARJ_Assert(refill_pos>=-4);
        ARJ_Assert((refill_pos+DIC_DELTA_SIZE+4*MAX_MATCH+6)<=(DIC_SIZE + MAX_MATCH*4 + 6UL));
        if ((byte_count =
             com->buf_read_crc(DIC_DELTA_SIZE, com->dictionary + refill_pos+4*MAX_MATCH+6, com->brc_propagator)) < 0)
        {
          com->max_match+=2; /* herstel max_match */
          ARJ_Assert(com->max_match<=MAX_MATCH);
          return GUP_READ_ERROR; /* ("Read error"); */
        }
        else
        {
          if (byte_count!=0)            /* byte count kan ook nul zijn */
          {
            ARJ_Assert((refill_pos+4*MAX_MATCH+6+byte_count)<=(DIC_SIZE + MAX_MATCH*4 + 6UL));
            #ifndef PP_AFTER
            com->print_progres(byte_count, com->pp_propagator);
            #endif
            bytes_to_do+=byte_count;
          }
        }
        if (refill_pos == 0)
        {
          memcpy(com->dictionary-4,com->dictionary + DIC_SIZE-4, MAX_MATCH*4+4+6);
          current_pos = M17(current_pos);
        }
        refill_pos = M17(refill_pos + DIC_DELTA_SIZE);
      }
    }
  }
  else if(com->speed==1)
  { /* medium speed, medium compression */
    c_codetype match0;
    pointer_type ptr0;
    while (bytes_to_do)
    { /*- hoofd_lus, deze lus zorgt voor al het pack werk */
      /* Een assert voor alle find_dictionary's. Tussen door wordt current pos niet aangepast */
      ARJ_Assert(!(((refill_pos+4*MAX_MATCH+6)>current_pos) && ((current_pos+2*MAX_MATCH+2)>=(refill_pos+4*MAX_MATCH+6)) && (refill_count<=DIC_DELTA_SIZE)));
      ARJ_Assert(!((current_pos+2*MAX_MATCH+2)>=(DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL - 4UL)));
      ARJ_Assert(!(((refill_pos+4*MAX_MATCH+6)<current_pos) && ((current_pos-(refill_pos+4*MAX_MATCH+6))<(65536L+2*MAX_MATCH)) && (refill_count<=DIC_DELTA_SIZE)));
      ARJ_Assert(!(current_pos<0));
      match0 = find_dictionary(current_pos+2, com);
      if(match0>0)
      {
        if ((unsigned long)match0 >= bytes_to_do)
        {
          match0 = (c_codetype) bytes_to_do;
          if (match0 < MIN_MATCH)
          {
            *com->charp++ = com->dictionary[current_pos++];
            bytes_to_do--;
            refill_count--;
          }
          else
          {
            ptr0 = com->best_match_pos;
            *com->charp++ = match0 + NLIT - MIN_MATCH;
            *com->ptrp++ = ptr0;
            bytes_to_do -= match0;
            refill_count -= match0;
            current_pos += match0;
          }
        }
        else
        {
          c_codetype match1;
          ptr0 = com->best_match_pos;
          match1 = find_dictionary(current_pos + 3, com);
          if ((unsigned long)match1 >= bytes_to_do)
          {
            match1 = (c_codetype)(bytes_to_do - 1);
            if (match1 < MIN_MATCH)
            {
              match1 = 0;
            }
          }
          if (match1 > match0)
          { /*- eerst literal, dan match */
            ARJ_Assert(com->ptrp>=com->pointers);
            ARJ_Assert(com->charp>=com->chars);
            ARJ_Assert(current_pos<(DIC_SIZE + MAX_MATCH*4 + 6UL));
            ARJ_Assert(current_pos>=0);
            *com->charp++=com->dictionary[current_pos++];
            bytes_to_do--;
            refill_count--;
            ARJ_Assert(((int)com->dictionary[current_pos]<NLIT));
            ARJ_Assert(((int)com->dictionary[current_pos]>=0));
            ARJ_Assert((com->charp[-1]<NLIT));
            ARJ_Assert((com->charp[-1]>=0));
            ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));
            ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));
            match0 = match1;
            ptr0 = com->best_match_pos;
          }
          ARJ_Assert((ptr0)<=com->maxptr);
          ARJ_Assert((match0)>=MIN_MATCH);
          ARJ_Assert((match0)<=(com->max_match+2));
          ARJ_Assert(com->ptrp>=com->pointers);
          ARJ_Assert(com->charp>=com->chars);
          *com->charp++ = match0 + NLIT - MIN_MATCH;
          *com->ptrp++ = ptr0;
          bytes_to_do -= match0;
          refill_count -= match0;
          current_pos += match0;
          ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));
          ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));
        }
      }
      else
      {
        ARJ_Assert(com->ptrp>=com->pointers);
        ARJ_Assert(com->charp>=com->chars);
        ARJ_Assert(current_pos<(DIC_SIZE + MAX_MATCH*4 + 6UL));
        ARJ_Assert(current_pos>=0);
        *com->charp++=com->dictionary[current_pos++];
        bytes_to_do--;
        refill_count--;
        ARJ_Assert(((int)com->dictionary[current_pos]<NLIT));
        ARJ_Assert(((int)com->dictionary[current_pos]>=0));
        ARJ_Assert((com->charp[-1]<NLIT));
        ARJ_Assert((com->charp[-1]>=0));
        ARJ_Assert((com->charp<=(com->chars+com->hufbufsize+4)));
        ARJ_Assert((com->ptrp<=(com->pointers+com->hufbufsize+4)));
      }
      if (com->charp > com->chars + huffbuf)
      { /*- 64k blok vol gepropt, ga de rotzooi opslaan */
        com->max_match+=2; /* herstel max_match */
        {
          gup_result res;
          if((res=com->compress(com))!=GUP_OK)
          {
            return res;
          }
        }
        if (com->mv_mode)
        {
          if (com->mv_next)
          {
            {
              gup_result res;
              if((res=com->close_packed_stream(com))!=GUP_OK)   /* flush bitbuf */
              {
                return res;
              }
            }
            return GUP_OK;
          }
        }
        com->max_match-=2; /* de sld werkt met de max_match minus twee waarde */
      }
      if (refill_count <= 0)
      {
        long byte_count = 0;
        refill_count += DIC_DELTA_SIZE;
        ARJ_Assert(refill_pos>=-4);
        ARJ_Assert((refill_pos+DIC_DELTA_SIZE+4*MAX_MATCH+6)<=(DIC_SIZE + MAX_MATCH*4 + 6UL));
        if ((byte_count =
             com->buf_read_crc(DIC_DELTA_SIZE, com->dictionary + refill_pos+4*MAX_MATCH+6, com->brc_propagator)) < 0)
        {
          com->max_match+=2; /* herstel max_match */
          return GUP_READ_ERROR; /* ("Read error"); */
        }
        else
        {
          if (byte_count!=0)            /* byte count kan ook nul zijn */
          {
            ARJ_Assert((refill_pos+4*MAX_MATCH+6+byte_count)<=(DIC_SIZE + MAX_MATCH*4 + 6UL));
            #ifndef PP_AFTER
            com->print_progres(byte_count, com->pp_propagator);
            #endif
            bytes_to_do+=byte_count;
          }
        }
        if (refill_pos == 0)
        {
          memcpy(com->dictionary-4,com->dictionary + DIC_SIZE-4, MAX_MATCH*4+4+6);
          current_pos = M17(current_pos);
        }
        refill_pos = M17(refill_pos + DIC_DELTA_SIZE);
      }
    }
  }
  else
  { /* slow speed, best compression */
    c_codetype match0 = -1;
    c_codetype match1 = -1;
    pointer_type ptr0 = 0;
    pointer_type ptr1 = 0;
    int hist_1=0; /* history-1 */
    int hist_2=0; /* history-2 */
    int hist0=0;
    int hist1=0;
    int ptrswap = 0;
    c_codetype swapmatch0 = 0;
    c_codetype swapmatch1 = 0;
    pointer_type swapptr0 = 0;
    pointer_type swapptr1 = 0;
    int swaphist0=0;
    int swaphist1=0;
    com->msp = com->matchstring;
    com->bmp = com->backmatch;

    while (bytes_to_do)
    { /*- hoofd_lus, deze lus zorgt voor al het pack werk */
      for(;;)
      {
        /* Een assert voor alle find_dictionary's. Tussen door wordt current pos niet aangepast */
        ARJ_Assert(!(((refill_pos+4*MAX_MATCH+6)>current_pos) && ((current_pos+2*MAX_MATCH+2)>=(refill_pos+4*MAX_MATCH+6)) && (refill_count<=DIC_DELTA_SIZE)));
        ARJ_Assert(!((current_pos+2*MAX_MATCH+2)>=(DIC_SIZE + MAX_MATCH*4 + 6UL + 4UL - 4UL)));
        ARJ_Assert(!(((refill_pos+4*MAX_MATCH+6)<current_pos) && ((current_pos-(refill_pos+4*MAX_MATCH+6))<(65536L+2*MAX_MATCH)) && (refill_count<=DIC_DELTA_SIZE)));
        ARJ_Assert(!(current_pos<0));
        if (match0 < 0)
        {
          match0 = find_dictionary(current_pos+2, com);
          if (match0 == 0)
          {
            STORE_LITERAL();
            match0 = match1;
            hist0 = hist1;
            ptr0 = ptr1;
            match1 = -1;
            break;
          }
          else
          {
            hist0 = com->hist_index;
            ptr0 = com->best_match_pos;
          }
        }
        else
        {
          if (match0 == 0)
          {
            STORE_LITERAL();
            match0 = match1;
            hist0 = hist1;
            ptr0 = ptr1;
            match1 = -1;
            break;
          }
        }
        if ((unsigned long)match0 >= bytes_to_do)
        {
          int16 diff = (int16)(match0 - bytes_to_do);
  
          match0 = (c_codetype) bytes_to_do;
          match1 = 0;
          if (match0 < MIN_MATCH)
          {
            match0 = 0;
            if (ptrswap > 0)
            {
              #ifndef NDEBUG
                long current_pos_start=current_pos;
              #endif
              UNSTORE_MATCH();
              STORE_LITERAL();
              STORE_MATCH_NB(swapptr0, swapmatch0, swaphist0);
              match0 = 0;
              match1 = 0;
              ptrswap = 0;
              ARJ_Assert(current_pos_start<=current_pos);
              break;
            }
          }
          else
          {
            if (ptrswap < 0)
            {
              swapmatch1 -= diff;
              if (swapmatch1 < MIN_MATCH)
              {
                ptrswap = 0;
              }
            }
          }
          if (match0 == 0)
          {
            STORE_LITERAL();
            match0 = match1;
            hist0 = hist1;
            ptr0 = ptr1;
            match1 = -1;
            break; /* tricky */
          }
        }
        if (match1 < 0)
        {
          match1 = find_dictionary(current_pos + 3, com);
          hist1 = com->hist_index;
          ptr1 = com->best_match_pos;
        }
        if ((unsigned long)match1 >= bytes_to_do)
        {
          int16 diff = (int16)(match1 - bytes_to_do + 1);

          match1 = (c_codetype)(bytes_to_do - 1);
          if (ptrswap > 0)
          {
            swapmatch1 -= diff;
            if (swapmatch1 < MIN_MATCH)
            {
              ptrswap = 0;
            }
          }
        }
        if (match1 < match0)
        {
          if (ptrswap < 0)
          {
            #ifndef NDEBUG
              long current_pos_start=current_pos;
            #endif
            UNSTORE_MATCH();
            UNSTORE_LITERAL();
            STORE_MATCH(swapptr0, swapmatch0, swaphist0);
            STORE_LITERAL();
            STORE_MATCH_NB(swapptr1, swapmatch1, swaphist1);
            ARJ_Assert(current_pos_start<=current_pos);
          }
          else
          {
            STORE_MATCH(ptr0, match0, hist0);
          }
          match0 = -1;
          match1 = -1;
          ptrswap = 0;
        }
        else
        {
          c_codetype match2, match3, match4, match5, m0, m1;
          pointer_type ptr2, ptr3, ptr4, ptr5;
          int hist2, hist3, hist4, hist5;
          if (match1 == match0)
          {
            match2 = find_dictionary(current_pos + match0+2, com);
            if (match2 == 0)
            {
              /*
               * literal pointer length, als het andersom moet dan wordt
               * dit wel gecorrigeerd
              */
              m0 = 0;
              m1 = 8;
              match3 = -1;
              match4 = -1;
              match5 = -1;

              hist2 = 0;
              hist3 = 0;
              hist4 = 0;
              hist5 = 0;
              ptr2 = 0;
              ptr3 = 0;
              ptr4 = 0;
              ptr5 = 0;
            }
            else
            {
              hist2 = com->hist_index;
              ptr2 = com->best_match_pos;
              match3 = find_dictionary(current_pos + match0 + 3, com);
              hist3 = com->hist_index;
              ptr3 = com->best_match_pos;
              match4 = match3;
              hist4 = com->hist_index;
              ptr4 = ptr3;
              match5 = find_dictionary(current_pos + match1 + 4, com);
              hist5 = com->hist_index;
              ptr5 = com->best_match_pos;
              if (match3 > match2)
              {
                m0 = match3 - 1;
              }
              else
              {
                m0 = match2;
              }
              m1 = match5 - 1;
            }
          }
          else
          {
            match2 = find_dictionary((current_pos + match0+2), com);
            hist2 = com->hist_index;
            ptr2 = com->best_match_pos;
            match3 = find_dictionary((current_pos + match0 + 3), com);
            hist3 = com->hist_index;
            ptr3 = com->best_match_pos;
            match4 = find_dictionary((current_pos + match1 + 3), com);
            hist4 = com->hist_index;
            ptr4 = com->best_match_pos;
            match5 = find_dictionary((current_pos + match1 + 4), com);
            hist5 = com->hist_index;
            ptr5 = com->best_match_pos;
            if (match3 > match2)
            {
              m0 = match3 - 1;
            }
            else
            {
              m0 = match2;
            }
            if (match5 > match4)
            {
              m1 = match5 - 1;
            }
            else
            {
              m1 = match4;
            }
          }
          if (((match1 + m1) > (match0 + m0)) ||
              ((match1 + 2) >= (match0 + (match2 >= match3 ? match2 : (match3 /*-1???nee*/ )))))
            /*
             * Technisch gezien moet het match1+2==match0+m0 zijn,
             * practisch is dit beter
            */
          { /*- er wordt een literal, pointer length opgeslagen */
            if (ptrswap > 0)
            {
              if ((match0 == match1) 
                  && (swapmatch0 == com->charp[-1] - (NLIT - MIN_MATCH)) 
                  && ((LOG(com->ptrp[-1]) + LOG(ptr0)) < (LOG(swapptr0) + LOG(ptr1))))
              {
                STORE_MATCH(ptr0, match0, hist0);
                STORE_LITERAL();
              }
              else
              {
                #ifndef NDEBUG
                  long current_pos_start=current_pos;
                #endif
                UNSTORE_MATCH();
                STORE_LITERAL();
                STORE_MATCH_NB(swapptr0, swapmatch0, swaphist0);
                STORE_MATCH(swapptr1, swapmatch1, swaphist1);
                ARJ_Assert(current_pos_start<=current_pos);
              }
              ptrswap = 0;
            }
            else
            {
              if ((match0 == match1) && ((ptr0) < (ptr1)))
              {
                STORE_MATCH(ptr0, match0, hist0)
                STORE_LITERAL();
                ptrswap = 0;
              }
              else
              {
                if (((match0 + match3) == (match1 + match4)) && (match3 > 0) && (match4 > 0))
                {
                  if ((LOG(ptr0) + LOG(ptr3)) < (LOG(ptr1) + LOG(ptr4)))
                  {
                    ptrswap = -1;
                    hist_2=hist_1;
                    swapmatch0 = match0;
                    swaphist0 = hist0;
                    swapptr0 = ptr0;
                    swapmatch1 = match3;
                    swaphist1 = hist3;
                    swapptr1 = ptr3;
                  }
                  else
                  {
                    ptrswap = 0;
                  }
                }
                else
                {
                  ptrswap = 0;
                }
                STORE_LITERAL();
                STORE_MATCH_NB(ptr1, match1, hist1);
              }
            }
            match0 = match4;
            hist0 = hist4;
            ptr0 = ptr4;
            match1 = match5;
            hist1 = hist5;
            ptr1 = ptr5;
          }
          else
          { /*- pointer length */
            if (ptrswap < 0)
            {
              #ifndef NDEBUG
                long current_pos_start=current_pos;
              #endif
              UNSTORE_MATCH();
              UNSTORE_LITERAL();
              if (swapmatch1 == match0)
              { /*- we hebben dus eigenlijk match 0 opgeslagen */
                if (((match0 + match3) == (match1 + match4)) && (match3 > 0) && (match4 > 0))
                {
                  if ((LOG(ptr0) + LOG(ptr3)) > (LOG(ptr1) + LOG(ptr4)))
                  {
                    ptrswap = 1;
                    swapmatch0 = match1;
                    swapmatch1 = match4;
                    swaphist0 = hist1;
                    swaphist1 = hist4;
                    swapptr0 = ptr1;
                    swapptr1 = ptr4;
                  }
                  else
                  {
                    ptrswap = 0;
                  }
                }
                else
                {
                  ptrswap = 0;
                }
              }
              else
              {
                ptrswap = 0;
              }
              STORE_MATCH(swapptr0, swapmatch0, swaphist0);
              STORE_LITERAL();
              STORE_MATCH_NB(swapptr1, swapmatch1, swaphist1);
              ARJ_Assert(current_pos_start<=current_pos);
            }
            else
            {
              if (((match0 + match3) == (match1 + match4)) && (match3 > 0) 
                  && (match4 > 0))
              {
                if ((LOG(ptr0) + LOG(ptr3)) > (LOG(ptr1) + LOG(ptr4)))
                {
                  ptrswap = 1;
                  swapmatch0 = match1;
                  swapmatch1 = match4;
                  swaphist0 = hist1;
                  swaphist1 = hist4;
                  swapptr0 = ptr1;
                  swapptr1 = ptr4;
                }
                else
                {
                  ptrswap = 0;
                }
              }
              else
              {
                ptrswap = 0;
              }
              STORE_MATCH(ptr0, match0, hist0)
            }
            if((match0+match2)<=(match1+2))
            { /* voorkom find_dictionary's op posities die al geweest zijn... */
              if(bytes_to_do>0)
              {
                STORE_LITERAL();
                match1=0;
                match0 = match3;
                hist0 = hist3;
                ptr0 = ptr3;
                ARJ_Assert(ptrswap==0);
              }
              else
              {
                match0=0;
                match1 = match3;
                hist1 = hist3;
                ptr1 = ptr3;
              }
            }
            else
            {
              match0 = match2;
              ptr0 = ptr2;
              hist0 = hist2;
              match1 = match3;
              hist1 = hist3;
              ptr1 = ptr3;
            }
          }
        }
        break;
      }
      ARJ_Assert_ZEEF34();
      ARJ_Assert((com->ptrp - com->pointers) == ((com->msp - com->matchstring) / 4));
      if (com->charp > com->chars + huffbuf)
      { /*- 64k blok vol gepropt, ga de rotzooi opslaan */
        com->max_match+=2; /* herstel max_match */
        {
          gup_result res;
          if((res=com->compress(com))!=GUP_OK)
          {
            return res;
          }
        }
        if (com->mv_mode)
        {
          if (com->mv_next)
          {
            {
              gup_result res;
              if((res=com->close_packed_stream(com))!=GUP_OK)   /* flush bitbuf */
              {
                return res;
              }
            }
            return GUP_OK;
          }
        }
        com->max_match-=2; /* de sld werkt met de max_match minus twee waarde */
        if (com->charp==com->chars)
        {
          ptrswap=0; /* als alle data is verwerkt kan er geen pointerswap meer zijn */
        }
      }
      if (refill_count <= 0)
      {
        long byte_count = 0;
        refill_count += DIC_DELTA_SIZE;
        ARJ_Assert(refill_pos>=-4);
        ARJ_Assert((refill_pos+DIC_DELTA_SIZE+4*MAX_MATCH+6)<=(DIC_SIZE + MAX_MATCH*4 + 6UL));
        if ((byte_count =
             com->buf_read_crc(DIC_DELTA_SIZE, com->dictionary + refill_pos+4*MAX_MATCH+6, com->brc_propagator)) < 0)
        {
          com->max_match+=2; /* herstel max_match */
          return GUP_READ_ERROR; /* ("Read error"); */
        }
        else
        {
          if (byte_count!=0)            /* byte count kan ook nul zijn */
          {
            ARJ_Assert((refill_pos+4*MAX_MATCH+6+byte_count)<=(DIC_SIZE + MAX_MATCH*4 + 6UL));
            #ifndef PP_AFTER
            com->print_progres(byte_count, com->pp_propagator);
            #endif
            bytes_to_do+=byte_count;
          }
        }
        if (refill_pos == 0)
        {
          memcpy(com->dictionary-4,com->dictionary + DIC_SIZE-4, MAX_MATCH*4+4+6);
          current_pos = M17(current_pos);
        }
        refill_pos = M17(refill_pos + DIC_DELTA_SIZE);
      }
    }
  }
  com->max_match+=2; /* herstel max_match */
  while (com->charp - com->chars)
  {
    {
      gup_result res;
      if((res=com->compress(com))!=GUP_OK)
      {
        return res;
      }
    }
    if (com->mv_mode)
    {
      if (com->mv_next)
      {
        {
          gup_result res;
          if((res=com->close_packed_stream(com))!=GUP_OK)   /* flush bitbuf */
          {
            return res;
          }
        }
        return GUP_OK;
      }
    }
  }
  /*
   * Nu alleen compresse bitstram afsluiten
  */
  {
    gup_result res;
    if((res=com->close_packed_stream(com))!=GUP_OK)   /* flush bitbuf */
    {
      return res;
    }
  }
  return GUP_OK;
}
#endif


#include <stdint.h>
#include "encode.h"
#include "compress.h"
#include "decode.h"
#include "huffman.h"

#define M7_MAX_PTR  0x10000              /* maximale pointer offset + 1 */
#define M7_MIN_MATCH 3						/* m4 maximum match */
#define M7_MAX_MATCH 256					/* m4 maximum match */

#define TREE63_MIN_MATCH M7_MIN_MATCH
#define TREE63_MAX_PTR M7_MAX_PTR
#define TREE63_MAX_MATCH M7_MAX_MATCH
#define N_PTR ARJ_NPT
#define N_PTR_BIT ARJ_PBIT
#define LIT_LEN_SIZE (NLIT+TREE63_MAX_MATCH+1)

#define MAX_BUCKETS (N_PTR+1)

typedef uint64_t freq_t;

static gup_result compress(lit63_i bytes_to_do, packstruct *com);

static int32_t ptr_index(pointer63_t ptr);

#if 0
	/* log literal en pointer len combi's */
	static unsigned long log_pos_counter=0;
	#define LOG_LITERAL(lit)  {printf("%lX Literal: %02X\n", log_pos_counter, lit); log_pos_counter++;}
	#define LOG_PTR_LEN(len, ptr) {printf("%lX Len: %u, ptr: %u\n", log_pos_counter ,len, ptr); log_pos_counter+=len;}
	#define LOG_BIT(bit) // printf("bit = %i\n",bit);
  	#define LOG_RUN(run) printf("Run = %lu\n", run);
	#define LOG_COUNTER_RESET log_pos_counter=0;
	#define LOG_TEXT(string) printf(string);
#else
	#define LOG_LITERAL(lit) /* */
	#define LOG_PTR_LEN(len, ptr) /* */
	#define LOG_BIT(bit) /* */
	#define LOG_RUN(run) /* */
 	#define LOG_COUNTER_RESET
	#define LOG_TEXT(string) /* */
#endif

#include "sld63i.c" /* sliding dictionary routines */

static int32_t ptr_index(pointer63_t ptr)
{
    return first_bit_set32(ptr);
}

static void init_charlen(uint8_t* charlen, packstruct *com)
{ /* initial guess for charlen */
    int i;
    for(i=0; i<NLIT; i++)
    {
        charlen[i]=9;
    }
    for(i=0; i<com->max_match; i++)
    {
        charlen[NLIT+i]=first_bit_set32(i);
    }
    charlen[NLIT+com->max_match]=4;
}

static void init_ptrlen(uint8_t* ptrlen)
{ /* initial guess for ptrlen */
    int i;
    for(i=0; i<MAX_NPT; i++)
    {
        ptrlen[i]=4;
    }
}

static unsigned int cost_ptrlen(len63_t len, pointer63_t ptr, packstruct *com)
{
    unsigned int kosten=com->charlen[NLIT+len];
    unsigned int bits=ptr_index(ptr);
    kosten+=com->ptrlen[bits];
    if(bits>1)
    {
        kosten+=bits-1;
    }
    return kosten;
}

static unsigned int cost_literal(uint8_t lit, packstruct *com)
{
    return com->charlen[lit];
}

static void find_path(lit63_i start_pos, lit63_i end_pos, packstruct *com)
{
    /* We berekenen de kosten van achteren naar voren */
    lit63_i pos=0;
    mb63_i mb_pos=0;
    while(pos<end_pos)
    {
        if(com->mb63[mb_pos].len==0)
        {
            pos++;
        }
        mb_pos++;
    }
    KOSTEN[end_pos].kosten=0;
    KOSTEN[end_pos].huff_count=0;
    while(end_pos>start_pos)
    {
        end_pos--;
        len63_t len=com->mb63[mb_pos].len;
        KOSTEN[end_pos].kosten=~0;
        while((len=com->mb63[mb_pos].len)!=0)
        {
            pointer63_t ptr=com->mb63[mb_pos].u.ptr;
            while(len>=TREE63_MIN_MATCH)
            {
                uint64_t kosten;
                kosten=KOSTEN[end_pos+len].kosten;
                kosten+=cost_ptrlen(len, ptr, com);
                if(KOSTEN[end_pos].kosten>kosten)
                {
                    KOSTEN[end_pos].kosten=kosten;
                    KOSTEN[end_pos].huff_count=KOSTEN[end_pos+len].huff_count+1;
                    KOSTEN[end_pos].len=len;
                    KOSTEN[end_pos].u.ptr=ptr;
                }
                len--;
            }
            mb_pos--;
        }
        uint64_t kosten;
        kosten=KOSTEN[end_pos+1].kosten;
        kosten+=cost_literal(com->mb63[mb_pos].u.lit, com);
        if(KOSTEN[end_pos].kosten>kosten)
        {
            KOSTEN[end_pos].kosten=kosten;
            KOSTEN[end_pos].huff_count=KOSTEN[end_pos+1].huff_count+1;
            KOSTEN[end_pos].len=0;
            KOSTEN[end_pos].u.lit=com->mb63[mb_pos].u.lit;
        }
        mb_pos--;
    }
}

uint64_t header_bits(freq_t * charfreq_0, freq_t * ptrfreq, packstruct *com)
{
    uint8 charlen[LIT_LEN_SIZE];  /* karakter lengte, offset MAX_MATCH moet bereikbaar zijn */
    uint16 char2huffman[LIT_LEN_SIZE];  /* huffman codes van de karakters */
    freq_t charfreq[LIT_LEN_SIZE];
    uint8 ptrlen[MAX_NPT+MAX_NPT];  /* pointer lengte */
    uint16 ptr2huffman[MAX_NPT]; /* huffman codes van de pointers */
    unsigned long header_bits = 0;
    freq_t freq[NCPT];
    freq_t char_freq[LIT_LEN_SIZE];
    /* eerst het gat in charfreq dichtmaken */
    {
        symbol_count_t i;
        for(i=0; i<NLIT; i++)
        {
            charlen[i]=com->charlen[i];
            charfreq[i]=charfreq_0[i];
        }
        for(i=com->min_match; i<=com->max_match; i++)
        {
            charlen[NLIT+i-com->min_match]=com->charlen[NLIT+i];
            charfreq[NLIT+i-com->min_match]=charfreq_0[NLIT+i];
        }
    }
    symbol_count_t charct=get_max_character(charfreq, NC);
  /*
   * we hebben nu de huffman codes van de karakterset berekend, nu moeten
   * we de lengtes gaan coderen. deze staan in charlen c_len coderings
   * blok: lengte van de pointers die c_len coderen, er zijn 19 pointers.
   */
    { /* frequentie tabel op nul zetten voor gebruik pointers */
        int i;

        memset(freq, 0, sizeof(freq));
        /* frequentie character lengtes tellen */
        for(i = 0; i < charct; i++)
        {
            if(charlen[i])
            {
                freq[charlen[i] + 2]++;
            }
            else
            { /*- charlen nul krijgt een speciale behandeling */
                int nulct = 1;
                while(!charlen[i + nulct])
                {
                    nulct++;
                }
                if(nulct < 3)
                {
                    freq[0] += (uint16)nulct;
                }
                else
                {
                    if(nulct < 20)
                    {
                        freq[1]++;
                        if(nulct == 19)
                        {
                            freq[0]++;
                        }
                    }
                    else
                    {
                        freq[2]++;
                    }
                }
                i += nulct - 1;
            }
        }
        make_hufftable(com->ptrlen1, com->ptr2huffman1, freq, NCPT, MAX_HUFFLEN, 0);
    }
    /*
    ** Nu zijn alle ptrs gedefinieerd, stuur ze de ARJ file in
    */
    header_bits += 16;                 /* aantal huffman karakters */
    {
        /*
        * belangrijk item, wat zijn de gevallen dat er slechts 1
        * pointerlengte overgedragen hoeft te worden? er is maar 1 pointer
        * lengte er is maar 1 karakter (dat kan wel meerdere ptrlens
        * veroorzaken)
        */
        int vp = 0;
        int np = 1;
        uint8 *p = charlen;
        int len = *p;
        int i = charct;
        do
        {
            int tmp = *p++;
            if(tmp)
            {
                vp++;
                if(tmp != len)
                {
                    np = 0;
                }
            }
            else
            {
                np = 0;
            }
        } while(--i!=0);
        if((vp < 2) || np)
        { /*- special case 1, er is maar een character lengte */
            header_bits += 10;
        }
        else
        {
            int ptrct = NCPT;
            int extra_add = 0;
            while(!com->ptrlen1[ptrct - 1])
            {
                ptrct--;
            }
            if(com->ptrlen1[3] == 0)
            {
                extra_add = 1;
                if(com->ptrlen1[4] == 0)
                {
                    extra_add = 2;
                    if(com->ptrlen1[5] == 0)
                    {
                        extra_add = 3;
                    }
                }
            }
            header_bits += 5;              /* aantal pointers dat er aan komt */
            for(i = 0; i < ptrct; i++)
            {
                if(com->ptrlen1[i] < 7)
                {
                    header_bits += 3;
                }
                else
                {
                    header_bits += com->ptrlen1[i] - 7 + 3 + 1;
                }
                if(i == 2)
                {
                    header_bits += 2;
                    i += extra_add;
                }
            }
        }
    }
    /*
    ** charlen overgedragen, breng characters
    */
    {
        /*
        ** De enige special case voor de characters is dat er maar een
        ** character is.
        */
        uint16 vp = 0;
        uint8 *p = charlen;
        int i = charct;
        do
        {
            if(*p++)
            {
                vp++;
            }
        } while(--i!=0);
        if(vp < 2)
        { /*- special case 2, er is maar een karakter lengte */
            header_bits += 18;
        }
        else
        {
            freq_t *p = freq;
            uint8 *q = com->ptrlen1;
            header_bits += 9;
            i = NCPT;
            header_bits += 4 * freq[1];
            header_bits += 9 * freq[2];
            do
            {
                header_bits += (unsigned long)*p++ * (unsigned long)*q++;
            } while(--i!=0);
        }
    }
    /*
     * charlen is overgestuurd, nu weer een ptrlen
    */
    {
        /*
        ** wat is de specialcase voor de pointers? 1 er is maar een
        ** pointerlengte
        */
        uint16 vp = 0;
        uint8 *p = ptrlen;

        int i = com->n_ptr;
        do
        {
            if(*p++)
            {
                vp++;
            }
        } while(--i!=0);
        if(vp < 2)
        { /*- special case 3, er is maar een pointerlengte */
            header_bits += com->m_ptr_bit+com->m_ptr_bit;
        }
        else
        {
            int ptrct = com->n_ptr;
            while((ptrct) && (!ptrlen[ptrct - 1]))
            {
                ptrct--;
            }
            header_bits += com->m_ptr_bit;
            for(i = 0; i < ptrct; i++)
            {
                if(ptrlen[i] < 7)
                {
                    header_bits += 3;
                }
                else
                {
                    header_bits += ptrlen[i] - 7 + 3 + 1;
                }
            }
        }
    }
    return header_bits;
}

static uint64_t path_frequency_count(lit63_i start_pos, lit63_i end_pos, packstruct *com)
{
    uint64_t totaal_bits;
    freq_t char_freq[LIT_LEN_SIZE]={0};
    freq_t ptr_freq[MAX_NPT]={0};
    while(start_pos<end_pos)
    {
        if(KOSTEN[start_pos].len==0)
        { /* literal */
            char_freq[KOSTEN[start_pos].u.lit]++;
            start_pos++;
        }
        else
        { /* ptr len */
            char_freq[NLIT+KOSTEN[start_pos].len]++;
            ptr_freq[ptr_index(KOSTEN[start_pos].u.ptr)]++;
            start_pos+=KOSTEN[start_pos].len;
        }
    }
    make_hufftable(com->charlen, com->char2huffman, char_freq, LIT_LEN_SIZE, MAX_HUFFLEN, 0);
    make_hufftable(com->ptrlen, com->ptr2huffman, ptr_freq, MAX_NPT, MAX_HUFFLEN, 0);
    totaal_bits=0;
    for(int i=0; i<LIT_LEN_SIZE; i++)
    {
        totaal_bits+=char_freq[i]*com->charlen[i];
    }
    for(int i=0; i<MAX_NPT; i++)
    {
        totaal_bits+=ptr_freq[i]*com->ptrlen[i];
        if(i>1)
        {
            totaal_bits+=ptr_freq[i]*(i-1);
        }
    }
    return totaal_bits;
}

static uint64_t optimize_huffman_block(lit63_i start_pos, lit63_i end_pos, packstruct *com)
{
    uint64_t totaal_bits=~0;
    uint64_t old_bits;
    uint64_t best_bits=~0;
    uint8_t best_charlen[LIT_LEN_SIZE];  /* tabel voor de beste charlen */
    uint16_t best_char2huffman[LIT_LEN_SIZE];
    uint8_t best_ptrlen[MAX_NPT];  /* tabel voor de beste ptrlen */
    uint16_t best_ptr2huffman[MAX_NPT];
    uint64_t best_huffman=0;

   com->special_header=NORMAL_HEADER;
    if((end_pos-start_pos)<=MAX_ENTRIES)
    { /* default naar minimum ASCII header */
        best_bits=8*(end_pos-start_pos)+MIN_ASCII_HEADER;
        com->special_header=SPECIAL_MIN_ASCII_HEADER;
        best_huffman=end_pos-start_pos;
    }
    init_charlen(com->charlen, com);
    init_ptrlen(com->ptrlen);
    do
    {
        old_bits=totaal_bits;
        set_maxlen(LIT_LEN_SIZE, com->charlen);
        set_maxlen(MAX_NPT, com->ptrlen);
        find_path(start_pos, end_pos, com);
        totaal_bits=path_frequency_count(start_pos, end_pos, com);
        if((totaal_bits<best_bits) && (KOSTEN[start_pos].huff_count<=MAX_ENTRIES))
        {
            best_bits=totaal_bits;
            best_huffman=KOSTEN[start_pos].huff_count;
            for(int i=0; i<LIT_LEN_SIZE; i++)
            {
                best_charlen[i]=com->charlen[i];
                best_char2huffman[i]=com->char2huffman[i];
            }
            for(int i=0; i<MAX_NPT; i++)
            {
                best_ptrlen[i]=com->ptrlen[i];
                best_ptr2huffman[i]=com->ptr2huffman[i];
            }
        }
        printf("bytes_to_do=%lu, old=%lu, kosten=%lu, verschil=%li, huff_count=%li\n", end_pos-start_pos, old_bits, totaal_bits, (int64_t)(old_bits-totaal_bits), KOSTEN[start_pos].huff_count);
    } while(totaal_bits<old_bits);
    if(best_huffman>0)
    {
        for(int i=0; i<LIT_LEN_SIZE; i++)
        {
            com->charlen[i]=best_charlen[i];
            com->char2huffman[i]=best_char2huffman[i];
        }
        for(int i=0; i<MAX_NPT; i++)
        {
            com->ptrlen[i]=best_ptrlen[i];
            com->ptr2huffman[i]=best_ptr2huffman[i];
        }
        return best_bits;
    }
    return 0;
}

static gup_result compress(lit63_i bytes_to_do, packstruct *com)
{
    uint64_t totaal_bits;
    totaal_bits=optimize_huffman_block(0, bytes_to_do, com);
	return GUP_OK;
}

gup_result n9_decode(decode_struct *com)
{
	return GUP_OK; /* exit succes */
}

gup_result n9_init(packstruct *com)
{
	gup_result res=GUP_OK;
	init_dictionary63(com);
	lit63_i orig_size;
	lit63_i bytes_to_do;
    uint8_t charlen[2*LIT_LEN_SIZE];  /* karakter lengte, offset MAX_MATCH moet bereikbaar zijn */
    uint16_t char2huffman[LIT_LEN_SIZE];  /* huffman codes van de karakters */
    uint8_t ptrlen[MAX_NPT+MAX_NPT];  /* pointer lengte */
    uint16_t ptr2huffman[MAX_NPT]; /* huffman codes van de pointers */
    uint8_t ptrlen1[NCPT+NCPT];  /* pointer lengte */
    uint16_t ptr2huffman1[NCPT];/* huffman codes van de pointers */
    com->charlen=charlen;  /* karakter lengte */
    com->char2huffman=char2huffman;
    com->ptrlen=ptrlen;
    com->ptr2huffman=ptr2huffman;
    com->ptrlen1=ptrlen1;
    com->ptr2huffman1=ptr2huffman1;
    com->rbuf_current=com->bw_buf->current;
    com->rbuf_tail=com->bw_buf->end;
    com->n_ptr=ARJ_NPT;
    com->m_ptr_bit=ARJ_PBIT;
    com->maxptr=0xffff;
    com->max_match=ARJ_MAX_MATCH;
    com->min_match=ARJ_MIN_MATCH;
    printf("\n");

	if(res!=GUP_OK)
	{
		return res;
	}
	{ /*- dictionary buffer vullen */
		long byte_count;
		if ((byte_count = com->buf_read_crc(com->origsize, com->dictionary63, com->brc_propagator)) < 0)
		{
			return GUP_READ_ERROR; /* ("Read error"); */
		}
		else
		{
			if (com->origsize == 0)
			{
				com->packed_size = 0;
				com->bytes_packed = 0;
				return GUP_OK;
			}
			else if (com->origsize != byte_count)
			{
				return GUP_READ_ERROR; /* ("Read error"); */
			}
			#ifndef PP_AFTER
			com->print_progres(byte_count, com->pp_propagator);
			#endif
		}
		orig_size = (uint64_t)byte_count;
		bytes_to_do = orig_size;
	}
    mb63_i mb_pos=0;
    for(uint64_t i=0; i<bytes_to_do; i++)
    {
        mb_pos=match63(com, i, mb_pos);
#if 0
        printf("pos=%li\n", i);
        while(mb_pos<new_mb_pos)
        {
            if(com->mb63[mb_pos].len==0)
            {
                if((com->mb63[mb_pos].u.lit>32) && (com->mb63[mb_pos].u.lit<127))
                {
                    printf("literal = %c\n", com->mb63[mb_pos].u.lit);
                }
                else
                {
                    printf("literal = \\%02X\n", com->mb63[mb_pos].u.lit);
                }
            }
            else
            {
                printf("len=%5i, ptr=%5i, ", com->mb63[mb_pos].len, com->mb63[mb_pos].u.ptr);
                for(int j=0; j<com->mb63[mb_pos].len; j++)
                {
                    if((com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]>32) && (com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]<127))
                    {
                        printf("%c", com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]);
                    }
                    else
                    {
                        printf("\\%02X", com->dictionary63[i-com->mb63[mb_pos].u.ptr-1+j]);
                    }
                }
                printf("\n");
            }
            mb_pos++;
        }
#endif
    }
    /* matchbuffer naar boven afsluiten met een literal */
    com->mb63[mb_pos].len=0;
    com->mb63[mb_pos].u.lit=0;
    compress(orig_size, com);
#if 0
	com->rbuf_current=com->bw_buf->current;
	com->rbuf_tail=com->bw_buf->end;
	com->mv_bits_left=0;
	com->bw_buf->current=com->rbuf_current;
#endif
    free_dictionary63(com);
	return res;
}

static int32_t ptr2bucket(pointer63_t ptr)
{
    return ptr_index(ptr);
}

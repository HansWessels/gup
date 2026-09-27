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
#define HUFFMAN_BLOCK_SIZE 4096
#define GLOBAL_INIT_LEN INIT_LEN_DEFAULT

#define MAX_BUCKETS (N_PTR+1)

typedef uint64_t freq_t;

static gup_result compress(lit63_i bytes_to_do, packstruct *com);

static int32_t ptr_index(pointer63_t ptr);

#if 0
	/* log literal en pointer len combi's */
	static unsigned long log_pos_counter=0;
	#define LOG_LITERAL(lit)  {if(((lit)>32) && ((lit)<127))printf("%lX Literal: %02X, %c\n", log_pos_counter, lit, lit); else printf("%lX Literal: %02X\n", log_pos_counter, lit); log_pos_counter++;}
	#define LOG_PTR_LEN(len, ptr)  {printf("%lX Len: %u, ptr: %u\n", log_pos_counter ,len, ptr); log_pos_counter+=len;}
	#define LOG_BIT(bit)  printf("bit = %i\n",bit);
  	#define LOG_RUN(run) printf("Run = %lu\n", run);
	#define LOG_COUNTER_RESET log_pos_counter=0;
	#define LOG_TEXT(string) printf(string);
#else
	#define LOG_LITERAL(lit) /* */
	#define LOG_PTR_LEN(len, ptr) /* */
	#define LOG_BIT(bit) /* */
	#define LOG_RUN(run) /* */
 	#define LOG_COUNTER_RESET
	#define LOG_TEXT(string) printf(string);
//	#define LOG_TEXT(string) /* */
#endif


typedef struct
{
    uint64_t kosten;
    lit63_i start_pos;
    lit63_i end_pos;
} huffman_kosten63_t;

#include "sld63i.c" /* sliding dictionary routines */

static void validate_huffman(pointer63_t start_pos, pointer63_t end_pos, packstruct *com)
{
    while(start_pos<end_pos)
    {
        len63_t len=KOSTEN[start_pos].len;
        if(len!=0)
        { /* pointer len */
            pointer63_t ptr=KOSTEN[start_pos].u.ptr;
            unsigned int bits=ptr_index(ptr);
            if(com->charlen[NLIT+len]==0)
            {
                printf("Error: len=%i, charlen=%i, huffman=%X\n", (int)len, (int)com->charlen[NLIT+len], (int)com->char2huffman[NLIT+len]);
            }
            if(com->ptrlen[bits]==0)
            {
                printf("Error: len=%i, charlen=%i, huffman=%X\n", (int)bits, (int)com->ptrlen[bits], (int)com->ptr2huffman[bits]);
            }
            start_pos+=len;
        }
        else
        { /* literal */
            if(com->charlen[KOSTEN[start_pos].u.lit]==0)
            {
                printf("Error: lit=%X, charlen=%i, huffman=%X\n", (int)KOSTEN[start_pos].u.lit, (int)com->charlen[KOSTEN[start_pos].u.lit], (int)com->char2huffman[KOSTEN[start_pos].u.lit]);
            }
            start_pos++;
        }
    }
}


static int32_t ptr_index(pointer63_t ptr)
{
    return first_bit_set32(ptr);
}

enum init_len_const
{
    INIT_LEN_DEFAULT,
    INIT_LEN_CHAR,
    INIT_LEN_PTR,
    INIT_LEN_STORE_0,
    INIT_LEN_RETRIVE_0,
    INIT_LEN_STORE_1,
    INIT_LEN_RETRIVE_1,
    INIT_LEN_LIT
};

static void init_len(int type, uint8_t* ptrlen, uint8_t* charlen, packstruct *com)
{
    static uint8_t ptrlen0[MAX_NPT]={0};
    static uint8_t charlen0[LIT_LEN_SIZE]={0};
    static uint8_t ptrlen1[MAX_NPT]={0};
    static uint8_t charlen1[LIT_LEN_SIZE]={0};

    switch(type)
    {
    default:
    case INIT_LEN_DEFAULT:
        {
            for(int i=0; i<NLIT; i++)
            {
                charlen[i]=8;
            }
            for(int i=0; i<com->max_match; i++)
            {
                charlen[NLIT+i]=first_bit_set32(i);
            }
            charlen[NLIT+com->max_match]=8;
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen[i]=4;
            }
            break;
        }
    case INIT_LEN_CHAR:
        {
            for(int i=0; i<NLIT; i++)
            {
                charlen[i]=2;
            }
            for(int i=0; i<=com->max_match; i++)
            {
                charlen[NLIT+i]=12;
            }
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen[i]=7;
            }
            break;
        }
    case INIT_LEN_PTR:
        {
            for(int i=0; i<NLIT; i++)
            {
                charlen[i]=9;
            }
            for(int i=0; i<=com->max_match; i++)
            {
                charlen[NLIT+i]=4;
            }
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen[i]=2;
            }
            break;
        }
    case INIT_LEN_STORE_0:
        {
            for(int i=0; i<=com->max_match; i++)
            {
                charlen0[i]=charlen[i];
            }
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen0[i]=ptrlen[i];
            }
            break;
        }
    case INIT_LEN_RETRIVE_0:
        {
            for(int i=0; i<=com->max_match; i++)
            {
                charlen[i]=charlen0[i];
            }
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen[i]=ptrlen0[i];
            }
            break;
        }
    case INIT_LEN_STORE_1:
        {
            for(int i=0; i<=com->max_match; i++)
            {
                charlen1[i]=charlen[i];
            }
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen1[i]=ptrlen[i];
            }
            break;
        }
    case INIT_LEN_RETRIVE_1:
        {
            for(int i=0; i<=com->max_match; i++)
            {
                charlen[i]=charlen1[i];
            }
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen[i]=ptrlen1[i];
            }
            break;
        }
    case INIT_LEN_LIT:
        {
            for(int i=0; i<NLIT; i++)
            {
                charlen[i]=8;
            }
            for(int i=0; i<=com->max_match; i++)
            {
                charlen[NLIT+i]=0;
            }
            for(int i=0; i<com->n_ptr; i++)
            {
                ptrlen[i]=0;
            }
            break;
        }
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

static mb63_i find_mb_pos(lit63_i pos, packstruct *com)
{ /* gaat er van uit dat een positie entry in de match buffer begint met een literal, daarna pointer lens */
    lit63_i current_pos=0;
    mb63_i mb_pos=0;
    while(current_pos<pos)
    {
        current_pos++;
        mb_pos++;
        while(com->mb63[mb_pos].len!=0)
        {
            mb_pos++;
        }
    }
    return mb_pos;
}

static void find_path(lit63_i start_pos, lit63_i end_pos, packstruct *com)
{
    /* We berekenen de kosten van achteren naar voren */
    mb63_i mb_pos=find_mb_pos(end_pos, com)-1;
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

static uint64_t count_header_bits(lit63_i start_pos, packstruct *com)
{
    uint64_t header_bits=0;
//printf("start: %lu\n", header_bits);
    {
        symbol_count_t charct;
        header_bits+=16;
        {
            uint8 charlen[LIT_LEN_SIZE];  /* karakter lengte, offset MAX_MATCH moet bereikbaar zijn */
            symbol_count_t i;
            for(i=0; i<NLIT; i++)
            {
                charlen[i]=com->charlen[i];
            }
            for(i=com->min_match; i<=com->max_match; i++)
            {
                charlen[NLIT+i-com->min_match]=com->charlen[NLIT+i];
            }
            charct=get_max_character(charlen, NLIT+com->max_match-com->min_match+1);
            if(charct==0)
            { /* er is maar 1 karakter */
                charct=KOSTEN[start_pos].len;
                if(charct==0)
                {
                    charct=KOSTEN[start_pos].u.lit;
                }
                else
                {
                    charct+=NLIT;
                }
                header_bits+=5;
                header_bits+=5;
                header_bits+=9;
                header_bits+=9;
            }
            else
            {
                /*
                ** belangrijk item, wat zijn de gevallen dat er slechts 1
                ** pointerlengte overgedragen hoeft te worden?
                ** 1: er is maar 1 pointer lengte
                */
                freq_t freq[NCPT]={0};
                freq_t nulct=0;
                int len=charlen[0];
                for(int_fast16_t i=0; i<charct; i++)
                {
                    int karlen=charlen[i];
                    if(karlen!=0)
                    {
                        freq[karlen+2]++;
                        if(karlen!=len)
                        {
                            nulct=1;
                        }
                    }
                    else
                    { /*- charlen nul krijgt een speciale behandeling */
                        nulct=1;
                        while(charlen[i + nulct]==0)
                        {
                            nulct++;
                        }
                        if(nulct < 3)
                        {
                            freq[0] += nulct;
                        }
                        else if(nulct < 20)
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
                        i += nulct - 1;
                    }
                }
                if(nulct==0)
                { /*- special case 1, er is maar een character lengte, de mame testset triggert deze case */
                    header_bits+=5;
                    header_bits+=5;
                    com->ptr2huffman1[charlen[0]+2]=0;
                    com->ptrlen1[charlen[0]+2]=0;
                }
                else
                {
                    int ptrct;
                    int skip=0;
                    make_hufftable(com->ptrlen1, com->ptr2huffman1, freq, NCPT, MAX_HUFFLEN, 0);
                    ptrct=get_max_character(com->ptrlen1, NCPT);
                    if(com->ptrlen1[3] == 0)
                    {
                        skip=1;
                        if(com->ptrlen1[4] == 0)
                        {
                            skip=2;
                            if(com->ptrlen1[5] == 0)
                            {
                                skip=3;
                            }
                        }
                    }
                    header_bits+=5;          /* aantal pointers dat er aan komt */
                    for(int_fast8_t i=0; i<ptrct; i++)
                    {
                        if(com->ptrlen1[i] < 7)
                        {
                            header_bits+=3;
                        }
                        else
                        {
                            int tail=com->ptrlen1[i]-6;
                            header_bits+=3;
                            header_bits+=tail; /* stuur tail-1 1 bits en dan een 0 bit */
                        }
                        if(i==2)
                        {
                            header_bits+=2;
                            i+=skip;
                        }
                    }
                }
//printf("ptrlen1: %lu\n", header_bits);
                header_bits+=9;
                for(int_fast16_t i=0; i<charct; i++)
                {
                    if(charlen[i]!=0)
                    {
                        header_bits+=com->ptrlen1[charlen[i]+2];
                    }
                    else
                    {
                        int nulct = 1;
                        while(charlen[i + nulct]==0)
                        {
                            nulct++;
                        }
                        i += nulct - 1;
                        if(nulct < 3)
                        {
                            while(nulct!=0)
                            {
                                header_bits+=com->ptrlen1[0];
                                nulct--;
                            }
                        }
                        else
                        {
                            if(nulct < 20)
                            {
                                if(nulct == 19)
                                {
                                    header_bits+=com->ptrlen1[0];
                                    nulct--;
                                }
                                header_bits+=com->ptrlen1[1];
                                header_bits+=4;
                            }
                            else
                            {
                                header_bits+=com->ptrlen1[2];
                                header_bits+=9;
                            }
                        }
                    }
                }
            }
        }
//printf("charlen: %lu\n", header_bits);
        /*
        ** charlen is overgestuurd, nu weer een ptrlen
        */
        {   /* wat is de specialcase voor de pointers? 1 er is maar een pointerlengte */
            int ptrct=get_max_character(com->ptrlen, com->n_ptr);
            if(ptrct==0)
            { /*- special case 3, er is maar een pointerlengte */
                header_bits+=com->m_ptr_bit;
                header_bits+=com->m_ptr_bit;
            }
            else
            {
                header_bits+=com->m_ptr_bit;
                for(int_fast8_t i=0; i<ptrct; i++)
                {
                    if(com->ptrlen[i]<7)
                    {
                        header_bits+=3;
                    }
                    else
                    {
                        int tail=com->ptrlen[i]-6;
                        header_bits+=3;
                        header_bits+=tail; /* stuur tail-1 1 bits en dan een 0 bit */
                    }
                }
            }
        }
    }
//printf("end: %lu\n", header_bits);

    return header_bits;
}

static uint64_t path_frequency_count(lit63_i start_pos, lit63_i end_pos, packstruct *com)
{
    uint64_t totaal_bits;
    lit63_i cur_pos=start_pos;
    freq_t char_freq[LIT_LEN_SIZE]={0};
    freq_t ptr_freq[MAX_NPT]={0};
    while(cur_pos<end_pos)
    {
        if(KOSTEN[cur_pos].len==0)
        { /* literal */
            char_freq[KOSTEN[cur_pos].u.lit]++;
            cur_pos++;
        }
        else
        { /* ptr len */
            char_freq[NLIT+KOSTEN[cur_pos].len]++;
            ptr_freq[ptr_index(KOSTEN[cur_pos].u.ptr)]++;
            cur_pos+=KOSTEN[cur_pos].len;
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
    com->header_size=count_header_bits(start_pos, com);
    return totaal_bits;
}

static void huffman_literal_block(lit63_i start_pos, lit63_i end_pos, packstruct *com)
{ /* maak een literal huffman block */
    freq_t char_freq[LIT_LEN_SIZE]={0};
    freq_t ptr_freq[MAX_NPT]={0};
   /* We berekenen de kosten van achteren naar voren */
    mb63_i mb_pos=find_mb_pos(end_pos, com)-1;
    while(end_pos>start_pos)
    {
        end_pos--;
        len63_t len=com->mb63[mb_pos].len;
        while((len=com->mb63[mb_pos].len)!=0)
        {
            mb_pos--;
        }
        char_freq[com->mb63[mb_pos].u.lit]++;
        mb_pos--;
    }
    make_hufftable(com->charlen, com->char2huffman, char_freq, LIT_LEN_SIZE, MAX_HUFFLEN, 0);
    make_hufftable(com->ptrlen, com->ptr2huffman, ptr_freq, MAX_NPT, MAX_HUFFLEN, 0);
}

static uint64_t optimize_huffman_block(lit63_i start_pos, lit63_i end_pos, int init, uint64_t target, packstruct *com)
{ /* target waarde is nodig omdat de laatste ronde optimalisatie over de MAX_ENTRIES kan gaan */
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
        if(best_bits<=target)
        {
            return best_bits;
        }
        init_len(INIT_LEN_LIT, com->ptrlen, com->charlen, com);
        set_highlen(LIT_LEN_SIZE, com->charlen);
        set_highlen(MAX_NPT, com->ptrlen);
        find_path(start_pos, end_pos, com);
        totaal_bits=path_frequency_count(start_pos, end_pos, com);
        if(((totaal_bits+com->header_size)<best_bits) && (KOSTEN[start_pos].huff_count<=MAX_ENTRIES))
        {
            com->special_header=NORMAL_HEADER;
            best_bits=totaal_bits+com->header_size;
            if(best_bits<=target)
            {
                return best_bits;
            }
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
    }
    init_len(init, com->ptrlen, com->charlen, com);
//    huffman_literal_block(start_pos, end_pos, com);
    do
    {
        old_bits=totaal_bits;
        set_maxlen(LIT_LEN_SIZE, com->charlen);
        set_maxlen(MAX_NPT, com->ptrlen);
        find_path(start_pos, end_pos, com);
        totaal_bits=path_frequency_count(start_pos, end_pos, com);
        if(((totaal_bits+com->header_size)<best_bits) && (KOSTEN[start_pos].huff_count<=MAX_ENTRIES))
        {
            com->special_header=NORMAL_HEADER;
            best_bits=totaal_bits+com->header_size;
            if(best_bits<=target)
            {
                return best_bits;
            }
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
        set_highlen(LIT_LEN_SIZE, com->charlen);
        set_highlen(MAX_NPT, com->ptrlen);
        find_path(start_pos, end_pos, com);
        if(KOSTEN[start_pos].huff_count<=MAX_ENTRIES)
        {
            totaal_bits=path_frequency_count(start_pos, end_pos, com);
            totaal_bits+=com->header_size;
            if(totaal_bits<best_bits)
            {
                return totaal_bits;
            }
        }
        return best_bits;
    }
    return 0;
}


static void store_literal_block(lit63_i start_pos, lit63_i end_pos, packstruct * com)
{
    mb63_i mb_pos=find_mb_pos(start_pos, com);

    store_bits((end_pos-start_pos)&0xFFFF, 16, com);
//printf("huffcount=%i\n", (int)(end_pos-start_pos)&0xFFFF);
    /*- special case 1, er is maar een character lengte */
    store_bits(0, 5, com);
    store_bits(10, 5, com); /* charlen is 8! */
    store_bits(256, 9, com); /* char count=256 */
    /*- special case 3, er is maar een pointerlengte */
    store_bits(0, com->m_ptr_bit, com);
    store_bits(0, com->m_ptr_bit, com);
    while(start_pos<end_pos)
    {
        LOG_LITERAL(KOSTEN[start_pos].u.lit);
        store_bits(com->mb63[mb_pos].u.lit, 8, com);
        start_pos++;
        mb_pos++;
        while(com->mb63[mb_pos].len!=0)
        {
            mb_pos++;
        }
    }
}

static gup_result compress_chars(lit63_i start_pos, lit63_i end_pos, uint64_t target, packstruct * com)
{
    uint64_t bits;
    uint64_t packed_size;
    uint64_t bits_size;
    for(int init=0; init<=INIT_LEN_PTR; init++)
    {
        bits=optimize_huffman_block(start_pos, end_pos, init, target, com);
        if((bits!=0) && (bits<=target))
        {
            break;
        }
    }
    bits_size=bits;
    {
        /*
        ** we weten nu hoveel bits er aan komen, passen deze nog in de buffer,
        ** of moeten we het ding flushen?
        */
        gup_result res;
        if((res=announce(((bits+com->bits_in_bitbuf)>>3)+sizeof(com->bitbuf)-1, com))!=GUP_OK)
        {
            return res;
        }
        packed_size=com->bits_in_bitbuf+(com->rbuf_current-com->rbuf_start)*8;
        bits+=com->bits_rest;
        com->bits_rest=(int16)(bits&7);
        com->packed_size += bits>>3;
        #ifdef PP_AFTER
        com->print_progres(m_size, com->pp_propagator);
        #endif
        com->bytes_packed += end_pos-start_pos; /* alweer een paar bytes gedaan! */
    }
    if(KOSTEN[start_pos].huff_count>MAX_ENTRIES)
    {
        printf("Huffcount error! Huffcount=%lu\n", KOSTEN[start_pos].huff_count);
    }
    /*
    ** Karakter frequenties zijn bekend, karakter huffman tabel is berekend.
    ** Pointer frequenties zijn bekend, pointer huffman tabel is berekend.
    */
    /*-
    ** we hebben nu de huffman codes van de karakterset berekend, nu moeten
    ** we de lengtes gaan coderen. deze staan in charlen c_len coderings
    ** blok:
    ** lengte van de pointers die c_len coderen, er zijn 19 pointers:
    ** 0          = c_len = 0
    ** 1 + 4 bits = de volgende 3-18 karakters hebben lengte 0
    ** 2 + 9 bits = de volgende 20-531 karakters hebben lengte 0
    ** 3          = c_len = 1
    ** :
    ** n          = c_len = n-2
    ** :
    ** 18         = c_len = 16
    */
    if(com->special_header==SPECIAL_MIN_ASCII_HEADER)
    { /* minimale header, alles literal */
        LOG_TEXT("SPECIAL_MIN_ASCII_HEADER\n");
        store_literal_block(start_pos, end_pos, com);
        return GUP_OK;
    }
    else
    {
        symbol_count_t charct;
//printf("huffcount=%i\n", (int)KOSTEN[start_pos].huff_count);
//printf("start: %lu\n", com->bits_in_bitbuf+(com->rbuf_current-com->rbuf_start)*8-packed_size);
        store_bits(KOSTEN[start_pos].huff_count, 16, com); /* aantal huffman karakters */
        {
            uint8 charlen[LIT_LEN_SIZE];  /* karakter lengte, offset MAX_MATCH moet bereikbaar zijn */
            symbol_count_t i;
            for(i=0; i<NLIT; i++)
            {
                charlen[i]=com->charlen[i];
            }
            for(i=com->min_match; i<=com->max_match; i++)
            {
                charlen[NLIT+i-com->min_match]=com->charlen[NLIT+i];
            }
            charct=get_max_character(charlen, NLIT+com->max_match-com->min_match+1);
            if(charct==0)
            { /* er is maar 1 karakter */
                charct=KOSTEN[start_pos].len;
                if(charct==0)
                { /* 1 literal */
                    LOG_TEXT("Special case 0\n");
                    charct=KOSTEN[start_pos].u.lit;
                }
                else
                { /* 1 len */
                    LOG_TEXT("Special case 1\n");
                    charct+=NLIT-com->min_match;
                }
                store_bits(0, 5, com);
                store_bits(2, 5, com);
                store_bits(0, 9, com);
                store_bits(charct, 9, com);
            }
            else
            {
                /*
                ** belangrijk item, wat zijn de gevallen dat er slechts 1
                ** pointerlengte overgedragen hoeft te worden?
                ** 1: er is maar 1 pointer lengte
                */
                freq_t freq[NCPT]={0};
                freq_t nulct=0;
                int len=charlen[0];
                for(int_fast16_t i=0; i<charct; i++)
                {
                    int karlen=charlen[i];
                    if(karlen!=0)
                    {
                        freq[karlen+2]++;
                        if(karlen!=len)
                        {
                            nulct=1;
                        }
                    }
                    else
                    { /*- charlen nul krijgt een speciale behandeling */
                        nulct=1;
                        while(charlen[i + nulct]==0)
                        {
                            nulct++;
                        }
                        if(nulct < 3)
                        {
                            freq[0] += nulct;
                        }
                        else if(nulct < 20)
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
                        i += nulct - 1;
                    }
                }
                if(nulct==0)
                { /*- special case 1, er is maar een character lengte, de mame testset triggert deze case */
                    LOG_TEXT("Special case 2\n");
                    store_bits(0, 5, com);
                    store_bits(charlen[0] + 2, 5, com);
                    com->ptr2huffman1[charlen[0]+2]=0;
                    com->ptrlen1[charlen[0]+2]=0;
                }
                else
                {
                    int ptrct;
                    int skip=0;
                    make_hufftable(com->ptrlen1, com->ptr2huffman1, freq, NCPT, MAX_HUFFLEN, 0);
                    ptrct=get_max_character(com->ptrlen1, NCPT);
                    if(com->ptrlen1[3] == 0)
                    {
                        skip=1;
                        if(com->ptrlen1[4] == 0)
                        {
                            skip=2;
                            if(com->ptrlen1[5] == 0)
                            {
                                skip=3;
                            }
                        }
                    }
                    store_bits(ptrct, 5, com);          /* aantal pointers dat er aan komt */
                    for(int_fast8_t i=0; i<ptrct; i++)
                    {
                        if(com->ptrlen1[i] < 7)
                        {
                            store_bits(com->ptrlen1[i], 3, com);
                        }
                        else
                        {
                            int tail=com->ptrlen1[i]-6;
                            store_bits(7, 3, com);
                            store_bits(((1<<tail)-2), tail, com); /* stuur tail-1 1 bits en dan een 0 bit */
                        }
                        if(i==2)
                        {
                            store_bits(skip, 2, com);
                            i+=skip;
                        }
                    }
                }
                store_bits(charct, 9, com);
                for(int_fast16_t i=0; i<charct; i++)
                {
                    if(charlen[i]!=0)
                    {
                        store_bits(com->ptr2huffman1[charlen[i]+2], com->ptrlen1[charlen[i]+2], com);
                    }
                    else
                    {
                        int nulct = 1;
                        while(charlen[i + nulct]==0)
                        {
                            nulct++;
                        }
                        i += nulct - 1;
                        if(nulct < 3)
                        {
                            while(nulct!=0)
                            {
                                store_bits(com->ptr2huffman1[0], com->ptrlen1[0], com);
                                nulct--;
                            }
                        }
                        else
                        {
                            if(nulct < 20)
                            {
                                if(nulct == 19)
                                {
                                    store_bits(com->ptr2huffman1[0], com->ptrlen1[0], com);
                                    nulct--;
                                }
                                store_bits(com->ptr2huffman1[1], com->ptrlen1[1], com);
                                store_bits(nulct - 3, 4, com);
                            }
                            else
                            {
                                store_bits(com->ptr2huffman1[2], com->ptrlen1[2], com);
                                store_bits(nulct - 20, 9, com);
                            }
                        }
                    }
                }
            }
        }
//printf("charlen: %lu\n", com->bits_in_bitbuf+(com->rbuf_current-com->rbuf_start)*8-packed_size);
        /*
        ** charlen is overgestuurd, nu weer een ptrlen
        */
        {   /* wat is de specialcase voor de pointers? 1 er is maar een pointerlengte */
            int ptrct=get_max_character(com->ptrlen, com->n_ptr);
//printf("ptrct=%i\n", (int)ptrct);
//for(int i=0; i<ptrct;i++)printf("ptrlen[%i]=%i\n", i, (int)com->ptrlen[i]);
            if(ptrct==0)
            { /*- special case 3, er is maar een pointerlengte */
                store_bits(0, com->m_ptr_bit, com);
                if(charct<=NLIT)
                { /* er zijn helemaal geen pointers */
                    LOG_TEXT("Special case 3\n");
                    store_bits(0, com->m_ptr_bit, com);
                }
                else
                { /* zoek een pointer */
                    lit63_i i=start_pos;
                    LOG_TEXT("Special case 4\n");
                    while(KOSTEN[i].len==0)
                    { /* skip literals */
                        i++;
                    }
                    store_bits(ptr_index(KOSTEN[i].u.ptr), com->m_ptr_bit, com);
                }
            }
            else
            {
                store_bits(ptrct, com->m_ptr_bit, com);
                for(int_fast8_t i=0; i<ptrct; i++)
                {
                    if(com->ptrlen[i]<7)
                    {
                        store_bits(com->ptrlen[i], 3, com);
                    }
                    else
                    {
                        int tail=com->ptrlen[i]-6;
                        store_bits(7, 3, com);
                        store_bits(((1<<tail)-2), tail, com); /* stuur tail-1 1 bits en dan een 0 bit */
                    }
                }
            }
        }
    }
//printf("end: %lu\n", com->bits_in_bitbuf+(com->rbuf_current-com->rbuf_start)*8-packed_size);
    {
        uint64_t header_size;
        header_size=com->bits_in_bitbuf+(com->rbuf_current-com->rbuf_start)*8-packed_size;
        if((header_size-com->header_size)!=0)
        {
            printf("header_size_c=%lu, header_size_m=%lu, verschil=%i\n", com->header_size, header_size, (int)(header_size-com->header_size));
        }
    }
    /*
    ** alle codes overgedragen, stuur nu de gecodeerde message
    */
    {
        while(start_pos<end_pos)
        {
            len63_t len=KOSTEN[start_pos].len;
            if(len!=0)
            { /* pointer len */
                pointer63_t ptr=KOSTEN[start_pos].u.ptr;
                LOG_PTR_LEN(len, ptr);
                int bits=ptr_index(ptr);
                store_bits(com->char2huffman[NLIT+len], com->charlen[NLIT+len], com);
//printf("len: %i len: %i huff %X\n", len, com->charlen[NLIT+len], com->char2huffman[NLIT+len]);
                store_bits(com->ptr2huffman[bits], com->ptrlen[bits], com);
//printf("ptr: %i len: %i huff %X\n", ptr, com->ptrlen[bits], com->ptr2huffman[bits]);
                bits--;
                if(bits>0)
                {
//printf("bits: %i len: %i huff %X\n", bits, bits, (ptr & (0xffff >> (16 - bits))));
                    store_bits((ptr & (0xffff >> (16 - bits))), bits, com);
                }
                start_pos+=len;
            }
            else
            { /* literal */
                LOG_LITERAL(KOSTEN[start_pos].u.lit);
                store_bits(com->char2huffman[KOSTEN[start_pos].u.lit], com->charlen[KOSTEN[start_pos].u.lit], com);
//printf("lit: %c len: %i huff %X\n", KOSTEN[start_pos].u.lit, com->charlen[KOSTEN[start_pos].u.lit], com->char2huffman[KOSTEN[start_pos].u.lit]);
                start_pos++;
            }
        }
    }
    {
        uint64_t header_size;
        header_size=com->bits_in_bitbuf+(com->rbuf_current-com->rbuf_start)*8-packed_size;
        if((header_size-bits_size)!=0)
        {
            printf("block_c=%lu, block_m=%lu, verschil=%i\n", bits_size, header_size, (int)(header_size-bits_size));
        }
    }
    return GUP_OK;
}

static gup_result compress(lit63_i bytes_to_do, packstruct *com)
{
    huffman_kosten63_t *huffman_kosten;
    lit63_i totaal_huffman_count;
    { /* eerste ronde, optimize hele blok zodat het in huffman blokken verdeeld kan worden. */
        lit63_i totaal_huffman;
        lit63_i delta_huffman;
        optimize_huffman_block(0, bytes_to_do, 2, 0, com);
        totaal_huffman=KOSTEN[0].huff_count;
        totaal_huffman_count=1+totaal_huffman/HUFFMAN_BLOCK_SIZE;
        delta_huffman=totaal_huffman/totaal_huffman_count;
        huffman_kosten=(huffman_kosten63_t*)com->gmalloc((totaal_huffman_count+1)*sizeof(huffman_kosten[0]), com->gm_propagator);
        if(huffman_kosten==NULL)
        {
            return GUP_NOMEM;
        }
        totaal_huffman_count--;
        lit63_i current_pos=0;
        for(uint64_t i=0; i<totaal_huffman_count; i++)
        {
            huffman_kosten[i].start_pos=current_pos;
            uint64_t huffman_count=delta_huffman;
            while(huffman_count>0)
            {
                huffman_count--;
                if(KOSTEN[current_pos].len==0)
                { /* literal */
                    current_pos++;
                }
                else
                { /* ptr len */
                    current_pos+=KOSTEN[current_pos].len;
                }
            }
        }
        huffman_kosten[totaal_huffman_count].start_pos=current_pos;
        totaal_huffman_count++;
        huffman_kosten[totaal_huffman_count].start_pos=bytes_to_do;
//printf("size=%lu, huffman_count=%lu, totaal_huffman=%lu, delta_huffman=%lu\n", bytes_to_do, totaal_huffman_count, totaal_huffman, delta_huffman);
    }
    { /* loop van achteren naar voren de huffman_kosten buffer door voor het ideale pad */
        lit63_i huffman_pos=totaal_huffman_count;
        huffman_kosten[totaal_huffman_count].kosten=0;
        do
        {
            lit63_i huffman_end_pos;
            uint64_t block_kosten;
            int delta=1;
            huffman_pos--;
//            printf("huffman_pos=%4lu ", huffman_pos);
            huffman_kosten[huffman_pos].kosten=~0;
            huffman_end_pos=huffman_pos+1;
            do
            {
                //for(int init=0; init<=INIT_LEN_PTR; init++)
                int init=INIT_LEN_PTR;
                {
                    uint64_t kosten;
                    kosten=huffman_kosten[huffman_end_pos].kosten;
                    block_kosten=optimize_huffman_block(huffman_kosten[huffman_pos].start_pos, huffman_kosten[huffman_end_pos].start_pos, init, 0, com);
                    if(block_kosten!=0)
                    {
                        kosten+=block_kosten;
                        if(kosten<=huffman_kosten[huffman_pos].kosten)
                        { /* nieuw optimum */
//                            printf("!");
                            huffman_kosten[huffman_pos].kosten=kosten;
                            huffman_kosten[huffman_pos].end_pos=huffman_end_pos;
                        }
                        else
                        {
//                            printf(".");
                        }
                    }
                }
                if(huffman_end_pos<totaal_huffman_count)
                {
                    huffman_end_pos+=delta;
                    if(huffman_end_pos>totaal_huffman_count)
                    {
                        huffman_end_pos=totaal_huffman_count;
                    }
                    if(((huffman_end_pos-huffman_pos)*HUFFMAN_BLOCK_SIZE-64)>MAX_ENTRIES)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
                delta+=delta;
            } while(0);
//            } while(block_kosten!=0);
//            printf("\n");
			#ifndef PP_AFTER
			com->print_progres(huffman_kosten[huffman_pos+1].start_pos-huffman_kosten[huffman_pos].start_pos, com->pp_propagator);
			#endif
        } while(huffman_pos>0);
        printf("kosten=%i (bits) = %i (bytes)\n", (int)huffman_kosten[0].kosten, (int)(huffman_kosten[0].kosten+7)/8);
    }
    { /* comprimeer gevonden pad */
        lit63_i huffman_pos=0;
        do
        {
            compress_chars(huffman_kosten[huffman_pos].start_pos, huffman_kosten[huffman_kosten[huffman_pos].end_pos].start_pos,
                            huffman_kosten[huffman_pos].kosten-huffman_kosten[huffman_kosten[huffman_pos].end_pos].kosten, com);
            huffman_pos=huffman_kosten[huffman_pos].end_pos;
        } while(huffman_kosten[huffman_pos].start_pos<bytes_to_do);
    }
    com->gfree(huffman_kosten, com->gf_propagator);
	return GUP_OK;
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

    com->packed_size = 0;
    com->bits_rest = 0;
    com->bitbuf = 0;
    com->bits_in_bitbuf = 0;
	com->rbuf_current=com->bw_buf->current;
	com->rbuf_tail=com->bw_buf->end;
	com->mv_bits_left=0;

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
		}
		orig_size = (uint64_t)byte_count;
		bytes_to_do = orig_size;
	}
    mb63_i mb_pos=0;
    for(uint64_t i=0; i<bytes_to_do; i++)
    {
        mb_pos=match63(i, mb_pos, com);
    }
    /* matchbuffer naar boven afsluiten met een literal */
    com->mb63[mb_pos].len=0;
    com->mb63[mb_pos].u.lit=0;
    compress(orig_size, com);
    /*
    ** Nu alleen compresse bitstram afsluiten
    */
    {
        gup_result res;
        if((res=com->close_packed_stream(com))!=GUP_OK)   /* flush bitbuf */
        {
            return res;
        }
    }

	com->rbuf_tail=com->bw_buf->end;
	com->mv_bits_left=0;
	com->bw_buf->current=com->rbuf_current;
    free_dictionary63(com);
	return res;
}

static int32_t ptr2bucket(pointer63_t ptr)
{
    return ptr_index(ptr);
}


/*
** huffman routines
*/

#include <stdint.h>
#include <string.h>
#include "huffman.h"
#include "compress.h"

#define INSERTION_GRENS 16
#define MAX_HUFFMAN_LEN 16

static void insertion_sort_symbols(symbol_t symbols[MAX_SYMBOL_SIZE], freq_t freq[MAX_SYMBOL_SIZE], symbol_count_t symbol_count);
static void radix_sort_symbols(symbol_t symbols[MAX_SYMBOL_SIZE], freq_t freq[MAX_SYMBOL_SIZE], symbol_count_t symbol_count);
static void make_huffman_codes(huffman_t huff_codes[], uint8_t* s_len, symbol_count_t symbol_count); /* maakt de huffman codes */

#define MAX_FREQ_VALUE (~((freq_t)0))

symbol_count_t get_max_character(uint8_t len[], symbol_count_t symbol_count)
{
    while(symbol_count>0)
    {
        symbol_count--;
        if(len[symbol_count]!=0)
        {
            return symbol_count+1;
        }
    }
    return 0;
}

void set_maxlen(symbol_count_t symbol_count, uint8_t len[])
{ /* zet iedere sybbol len die 0 is op max_len+1 */
    uint8 max_len=0;
    symbol_count_t i;
    for(i=0; i<symbol_count; i++)
    {
        if(len[i]>max_len)
        {
            max_len=len[i];
        }
    }
    max_len++;
    for(i=0; i<symbol_count; i++)
    {
        if(len[i]==0)
        {
            len[i]=max_len;
        }
    }
}

void set_highlen(symbol_count_t symbol_count, uint8_t len[])
{ /* zet iedere sybbol len die 0 is op 0xFF */
    uint8 max_len=0;
    symbol_count_t i;
    for(i=0; i<symbol_count; i++)
    {
        if(len[i]==0)
        {
            len[i]=0xFF;
        }
    }
}

static void insertion_sort_symbols(symbol_t symbols[MAX_SYMBOL_SIZE], freq_t freq[MAX_SYMBOL_SIZE], symbol_count_t symbol_count)
{
    symbol_count_t i;
    for(i=1; i<symbol_count; i++)
    {
        freq_t cur_freq=freq[i];
        symbol_t cur_symbol=symbols[i];
        symbol_count_t pos=i-1;
        while(pos>=0 && (freq[pos]>cur_freq))
        {
            freq[pos+1]=freq[pos];
            symbols[pos+1]=symbols[pos];
            pos--;
        }
        freq[pos+1]=cur_freq;
        symbols[pos+1]=cur_symbol;
    }
}

static void radix_sort_symbols(symbol_t symbols[MAX_SYMBOL_SIZE], freq_t freq[MAX_SYMBOL_SIZE], symbol_count_t symbol_count)
{
    symbol_t tmp_symbols[MAX_SYMBOL_SIZE];
    freq_t tmp_freq[MAX_SYMBOL_SIZE];

    #define BUCKET_BITS 8
    #define BUCKET_SIZE (1<<BUCKET_BITS)
    #define BUCKET_MASK (BUCKET_SIZE-1)

    symbol_count_t bucket[BUCKET_SIZE];
    freq_t max=0;
    int shift=0;
    if(symbol_count<INSERTION_GRENS)
    {
        insertion_sort_symbols(symbols, freq, symbol_count);
        return;
    }
    symbol_count_t i=symbol_count;
    do
    {
        i--;
        if(max<freq[i])
        {
            max=freq[i];
        }
    } while(i>0);

    for(;;)
    {
        symbol_count_t i;
        i=symbol_count;
        memset(bucket, 0, sizeof(bucket));
        do
        {
            i--;
            bucket[((freq[i]>>shift) & BUCKET_MASK)]++;
        } while(i>0);

        symbol_count_t start;
        start=0;
        for(i=0; i<BUCKET_SIZE; i++)
        {
            symbol_count_t tmp=bucket[i];
            bucket[i]=start;
            start+=tmp;
        }
        for(i=0; i<symbol_count; i++)
        {
            int tmp=(freq[i]>>shift)&BUCKET_MASK;
            tmp_freq[bucket[tmp]]=freq[i];
            tmp_symbols[bucket[tmp]]=symbols[i];
            bucket[tmp]++;
        }
        max>>=BUCKET_BITS;
        if(max==0)
        {
            memcpy(freq, tmp_freq, symbol_count*sizeof(freq[0]));
            memcpy(symbols, tmp_symbols, symbol_count*sizeof(symbols[0]));
            return;
        }
        shift+=BUCKET_BITS;
        i=symbol_count;
        memset(bucket, 0, sizeof(bucket));
        do
        {
            i--;
            bucket[((tmp_freq[i]>>shift) & BUCKET_MASK)]++;
        } while(i>0);
        start=0;
        for(i=0; i<BUCKET_SIZE; i++)
        {
            symbol_count_t tmp=bucket[i];
            bucket[i]=start;
            start+=tmp;
        }
        for(i=0; i<symbol_count; i++)
        {
            int tmp=(tmp_freq[i]>>shift)&BUCKET_MASK;
            freq[bucket[tmp]]=tmp_freq[i];
            symbols[bucket[tmp]]=tmp_symbols[i];
            bucket[tmp]++;
        }
        max>>=BUCKET_BITS;
        if(max==0)
        {
            return;
        }
        shift+=BUCKET_BITS;
    }
}

static void make_huffman_codes(huffman_t huff_codes[], uint8_t* s_len, symbol_count_t symbol_count) /* maakt de huffman codes */
{
    int len_count[MAX_HUFFMAN_LEN+1]={0};
    huffman_t huffcode[MAX_HUFFMAN_LEN+1];
    symbol_count_t i;
    i=symbol_count;
    do
    {
        i--;
        len_count[s_len[i]]++;
    } while(i>0);
    huffman_t start_huffcode=0;
    huffcode[0]=0;
    for(i=1; i<=MAX_HUFFMAN_LEN; i++)
    {
        start_huffcode<<=1;
        huffcode[i]=start_huffcode;
        start_huffcode+=len_count[i];
    }
    for(i=0; i<symbol_count; i++)
    {
        huff_codes[i]=huffcode[s_len[i]];
        huffcode[s_len[i]]+=(s_len[i]!=0);
    }
}

void make_hufftable(uint8_t s_len[], huffman_t huff_codes[], const freq_t in_freq[], const symbol_count_t symbol_size, int max_huff_len, int sort_order)
{
    freq_t freq_array[MAX_SYMBOL_SIZE*2+1]; /* we willen een voor het array ook kunnen lezen */
    symbol_count_t tree_array[MAX_HUFFMAN_LEN*MAX_SYMBOL_SIZE*2];
    symbol_t symbols[MAX_SYMBOL_SIZE];
    symbol_count_t* tree=tree_array;
    freq_t* freq=freq_array+1;
    symbol_count_t symbol_count;
    symbol_count_t pairs_count;
    symbol_count_t i;
    symbol_count=0;
    if(sort_order==0)
    {
        i=symbol_size;
        do
        { /* hoeveel symbols zijn er met een freq>0? */
            i--;
            if(in_freq[i]!=0)
            {
                freq[symbol_count]=in_freq[i];
                symbols[symbol_count]=i;
                symbol_count++;
            }
        } while(i>0);
    }
    else
    {
        for(i=0; i<symbol_size; i++)
        { /* hoeveel symbols zijn er met een freq>0? */
            if(in_freq[i]!=0)
            {
                freq[symbol_count]=in_freq[i];
                symbols[symbol_count]=i;
                symbol_count++;
            }
        }
    }
    memset(s_len, 0, (size_t)symbol_size*sizeof(s_len[0]));
    if(symbol_count<3)
    { /* special cases 0, 1 en 2 symbolen */
        memset(huff_codes, 0, (size_t)symbol_size*sizeof(huff_codes[0]));
        if(symbol_count>1)
        {
            huffman_t code=0;
            for(i=0; i<symbol_size; i++)
            {
                if(in_freq[i]!=0)
                {
                    huff_codes[i]=code;
                    code++;
                    s_len[i]=1;
                }
            }
        }
        return;
    }
    {
        radix_sort_symbols(symbols, freq, symbol_count);
    }
    freq+=symbol_count; /* freq array index -1 based */
    { /* eerst een traditionele huffmanboom bouwen */
        symbol_count_t* len=tree+3*symbol_count;
        symbol_count_t symbol_pos=-symbol_count;
        symbol_count_t child=0;
        symbol_count_t pair_pos=symbol_count-1;
        symbol_count_t node;
        for(node=0; node<symbol_count; node++)
        {
            freq[node]=MAX_FREQ_VALUE; /* sentries */
        }
        node=symbol_count-1;
        do
        {
            freq_t node_freq;
            if(freq[symbol_pos]<=freq[pair_pos])
            {
                tree[child++]=symbol_pos;
                node_freq=freq[symbol_pos];
                symbol_pos++;
            }
            else
            {
                tree[child++]=pair_pos;
                node_freq=freq[pair_pos];
                pair_pos--;
            }
            if(freq[symbol_pos]<=freq[pair_pos])
            {
                tree[child++]=symbol_pos;
                node_freq+=freq[symbol_pos];
                symbol_pos++;
            }
            else
            {
                tree[child++]=pair_pos;
                node_freq+=freq[pair_pos];
                pair_pos--;
            }
            freq[node]=node_freq;
            node--;
        } while(node>0);
        node++;
        { /* bouw s_len */
            len[node]=0;
            do
            {
                int current_len;
                current_len=len[node]+1;
                len[tree[--child]]=current_len;
                len[tree[--child]]=current_len;
                node++;
            } while(child>0);
            len-=symbol_count;
            if(len[0]<=max_huff_len)
            {
                for(i=0; i<symbol_count; i++)
                {
                    s_len[symbols[i]]=(uint8)len[i];
                }
                make_huffman_codes(huff_codes, s_len, symbol_size);
                return;
            }
        }
    }
    freq[0]=0; /* sentry */
    {
        symbol_count_t symbol_pos;
        pairs_count=symbol_count>>1;
        symbol_pos=-1-(symbol_count&1);
        i=pairs_count;
        do
        { /* eerste ronde, gewoon de gegeven character bij elkaar voegen */
            i--;
            freq_t node_freq;
            node_freq=freq[symbol_pos];
            tree[i*2+1]=symbol_pos;
            symbol_pos--;
            node_freq+=freq[symbol_pos];
            tree[i*2]=symbol_pos;
            symbol_pos--;
            freq[i+1]=node_freq;
        } while(i>0);
        max_huff_len--;
    }
    do
    { /* merge symbols, max_huff_len-1 keer */
        symbol_count_t symbol_pos=-1;
        symbol_count_t pair_pos=pairs_count;
        freq_t next_symbol_freq=freq[symbol_pos];
        freq_t next_pair_freq=freq[pair_pos];
        tree+=symbol_count*2;
        pairs_count=symbol_count+pairs_count;
        if((pairs_count)&1)
        { /* oneven som, waarde met hoogste freq doet niet mee */
            if(next_pair_freq>=next_symbol_freq)
            {
                pair_pos--;
                next_pair_freq=freq[pair_pos];
            }
            else
            {
                symbol_pos--;
                next_symbol_freq=freq[symbol_pos];
            }
        }
        pairs_count>>=1;
        i=pairs_count;
        do
        { /* maak de nieuwe pairs */
            uint64_t node_freq;
            i--;
            if(next_pair_freq>=next_symbol_freq)
            {
                node_freq=next_pair_freq;
                tree[i*2+1]=0;
                pair_pos--;
                next_pair_freq=freq[pair_pos];
            }
            else
            {
                node_freq=next_symbol_freq;
                tree[i*2+1]=symbol_pos;
                symbol_pos--;
                next_symbol_freq=freq[symbol_pos];
            }
            if(next_pair_freq>=next_symbol_freq)
            {
                node_freq+=next_pair_freq;
                tree[i*2]=0;
                pair_pos--;
                next_pair_freq=freq[pair_pos];
            }
            else
            {
                node_freq+=next_symbol_freq;
                tree[i*2]=symbol_pos;
                symbol_pos--;
                next_symbol_freq=freq[symbol_pos];
            }
            if(node_freq<MAX_FREQ_VALUE)
            {
                freq[i+1]=(freq_t)node_freq;
            }
            else
            {
                freq[i+1]=(freq_t)MAX_FREQ_VALUE;
            }
        } while(i>0);
        max_huff_len--;
    } while(max_huff_len>0);
    {
        symbol_count_t i;
        memset(freq-symbol_count, 0, (size_t)symbol_count*sizeof(freq[0]));
        i=2*(symbol_count-1);  /* N symbolen levert altijd N-1 pairs op */
        do
        {
            freq[0]=0;
            do
            {
                i--;
                freq[tree[i]]++;
            } while(i>0);
            i=2*freq[0];
            tree-=symbol_count*2;
        } while(i>0);
        i=symbol_count;
        freq-=symbol_count; /* undo negatieve symbol index */
        do
        {
            i--;
            s_len[symbols[i]]=(uint8)freq[i];
        } while(i>0);
    }
    make_huffman_codes(huff_codes, s_len, symbol_size);
    return;
}

/*
** sliding dictionary 63 bit
**
** alle indices zijn signed 64 bit integers
** een negatieve index duidt op een no node
**
** alle matches worden als pointer length combinaties opgelsagen in de match_buffer
** als op een bepaalde positie geen verdere matches meer zijn
** wordt match lengte 0 opgelsgen en de pointer is gelijk aan de literal waarde
** als de match_buffer vol is, of als alle posities in een file gematched zijn
** wordt de functie compress aangeroepen, die comprimeerd de data in de matchbuffer
** en past de grenzen aan:
** buffer_i_t mb_start: start van de match_buffer
** buffer_i_t mb_end: einde van de match_buffer
** buffer_i_t mb_current: positie waar de volgende match wordt toegevoegd
** buffer_i_t mb_size: grootte van de match_buffer
** als mb_current voorbij mb_end is moet compress worden aangeroepen, die mag data tot
** aan mb_end comprimeren
** als bij het aanroepen van compress mb_current voor mb_end staat dan is de te comprimeren file
** in zijn geheel gematcht en moet alles gecomprimeerd worden
** als bij het aanroepen van compress mb_current achter mb_end staat dan moet de data van mb_start tot voorbij
** mb_start+mb_size/2 gecomprimeerd worden
**
**
*/

#include <stddef.h>
#include <compress.h>
#include <encode.h>

#undef NO_NODE
#define NO_NODE 0
#define TREE63_OFFSET 1                 /* maak plek voor speciale nodes, zoals 0==NO_NODE */
#define TREE63_SIZE 0x20000             /* grootte van de tree, bijvoorkeur een macht van twee, moet tenminste 1 groter dan MAX_PTR zijn */

#undef DIC
#define DIC com->dictionary63 /* 'function' to acces the dictionary */
#undef TREE
#define TREE com->tree63

#undef TREE63_RLE_LEN
#if TREE63_MAX_MATCH > 258              /* de hashfunctie seserveerd 1 byte voor de RLE LEN, die kan dus lopen van 3..258 */
    #define TREE63_RLE_LEN 258
#else
    #define TREE63_RLE_LEN TREE63_MAX_MATCH
#endif

#define HASH_SIZE63 0x20000 /* 64k normal en 64k RLE hashes */
#define RLE63_OFFSET 0x10000 /* 64k offset, zou eventueel negatief kunnen worden */


static gup_result init_dictionary63(packstruct *com);
static void free_dictionary63(packstruct *com);
static hash_t calc_hash(lit63_i key, packstruct* com);
static node63_i key2node(lit63_i key);
static node63_i init_node(node63_i node, lit63_i key, packstruct* com);
static void remove_node(lit63_i key, packstruct* com);
static mb63_i match63(packstruct* com, lit63_i key, match_buffer_i match_buffer_pos);
static int32_t ptr2bucket(pointer63_t ptr);

static gup_result init_dictionary63(packstruct *com)
{
    size_t size;
    size=com->origsize+DICTIONARY_END_OFFSET;
	com->dictionary63_mem=com->gmalloc(size*sizeof(DIC[0]), com->gm_propagator);
	if (com->dictionary63_mem == NULL)
	{
		return GUP_NOMEM;
	}
	com->dictionary63=com->dictionary63_mem;
	com->mb63_mem=com->gmalloc((com->origsize*MAX_BUCKETS)*sizeof(com->mb63[0]), com->gm_propagator);
	if (com->mb63_mem == NULL)
	{
		return GUP_NOMEM;
	}
	com->mb63=com->mb63_mem;
	com->hash_table63=com->gmalloc((HASH_SIZE63)*sizeof(com->hash_table63[0]), com->gm_propagator);
	if (com->hash_table63 == NULL)
	{
		return GUP_NOMEM;
	}
	com->tree63_mem=com->gmalloc(TREE63_SIZE*sizeof(TREE[0]), com->gm_propagator);
	if (com->tree63_mem == NULL)
	{
		return GUP_NOMEM;
	}
	com->tree63=com->tree63_mem;
	com->tree63-=TREE63_OFFSET;
	/* initialiseer de index_hashes */
	memset(com->hash_table63, 0, (HASH_SIZE63)*sizeof(com->hash_table63[0]));
	return GUP_OK;
}

static void free_dictionary63(packstruct *com)
{
	com->gfree(com->dictionary63_mem, com->gf_propagator);
	com->gfree(com->mb63_mem, com->gf_propagator);
	com->gfree(com->hash_table63, com->gf_propagator);
	com->gfree(com->tree63_mem, com->gf_propagator);
}


static hash_t calc_hash(lit63_i key, packstruct* com)
{
    hash_t hash;

	hash=(DIC[key]^DIC[key+1]);
	hash<<=8;
	hash|=(DIC[key+1]^DIC[key+2]);
	if(hash==0)
	{ /* RLE */
        lit63_t orig;
        lit63_i p=key;
        lit63_i max_match;
        if(com->origsize-key>=TREE63_RLE_LEN)
        {
            max_match=TREE63_RLE_LEN;
        }
        else
        {
            max_match=com->origsize-key;
        }
        hash=(hash_t)DIC[key];
        orig=DIC[key+max_match];
        DIC[key+max_match]=~DIC[key];
        while (DIC[p++] == DIC[key])
        {
            ;
        }
        hash<<=8;
        hash+=(hash_t)(p-key-3); /* min RLE LEN = 3, anders geen RLE */
        DIC[key+max_match]=orig;
        hash+=RLE63_OFFSET;
 	}
	return hash;
}

static node63_i key2node(lit63_i key)
{
    node63_i node;
    node=TREE63_OFFSET+key%TREE63_SIZE;
    return node;
}

static node63_i init_node(node63_i node, lit63_i key, packstruct* com)
{
    node63_i new_node=com->hash_table63[calc_hash(key, com)];
    com->hash_table63[calc_hash(key, com)]=node;
    return new_node;
}

static void remove_node(lit63_i key, packstruct* com)
{
    if(key<TREE63_MAX_PTR)
    {
        return;
    }
    node63_i node=key2node(key-TREE63_MAX_PTR);
    node63_i parent=TREE[node].parent;
    if(parent==NO_NODE)
    {
        if(com->hash_table63[calc_hash(key, com)]==node)
        {
            com->hash_table63[calc_hash(key, com)]=NO_NODE;
        }
    }
    else if(TREE[parent].c_left==node)
    {
        TREE[parent].c_left=NO_NODE;
    }
    else if(TREE[parent].c_right==node)
    {
        TREE[parent].c_right=NO_NODE;
    }
}

static mb63_i match63(packstruct* com, lit63_i key, mb63_i match_buffer_pos)
{
    mb63_t buckets[MAX_BUCKETS]={0};
    lit63_i max_match;
    remove_node(key, com);
    node63_i node=key2node(key);
    TREE[node].parent=NO_NODE;
    TREE[node].key=key;
    node63_i smaller_node=node;
    node63_i bigger_node=node;
    node=init_node(node, key, com);
    if(com->origsize-key>=TREE63_MAX_MATCH)
    {
        max_match=TREE63_MAX_MATCH;
    }
    else if(com->origsize-key>=TREE63_MIN_MATCH)
    {
        max_match=com->origsize-key;
    }
    else
    { /* we kunnen hier geen match vinden, return literal */
        com->mb63[match_buffer_pos].len=0;
        com->mb63[match_buffer_pos].u.lit=DIC[key];
        match_buffer_pos++;
        return match_buffer_pos;
    }
    lit63_t orig_lit=DIC[key+max_match];
    while(node!=NO_NODE)
    {
        if(key==TREE[node].key)
        {
            printf("key=%X, node=%X, node.key=%lX\n", key, node, TREE[node].key);
            printf("lit=%02X ~lit=%02X\n", (int)DIC[TREE[node].key+max_match], (int)~DIC[TREE[node].key+max_match]);
        }
        DIC[key+max_match]=~DIC[TREE[node].key+max_match];
        len63_t match_len=0;
        while(DIC[key+match_len]==DIC[TREE[node].key+match_len])
        {
            match_len++;
        }
        if(match_len>=TREE63_MIN_MATCH)
        {
            pointer63_t ptr=key-TREE[node].key-1;
            uint32_t bucket=ptr2bucket(ptr);
            if(match_len>buckets[bucket].len)
            {
                buckets[bucket].len=match_len;
                buckets[bucket].u.ptr=ptr;
            }
        }
        if(match_len==max_match)
        { /* max_match gevonden, we are done */
            TREE[smaller_node].c_right=TREE[node].c_right;
            if(TREE[node].c_right!=NO_NODE)
            {
                TREE[TREE[node].c_right].parent=smaller_node;
            }
            TREE[bigger_node].c_left=TREE[node].c_left;
            if(TREE[node].c_left!=NO_NODE)
            {
                TREE[TREE[node].c_left].parent=bigger_node;
            }
            smaller_node=node;
            bigger_node=node;
            node=NO_NODE;
        }
        else if(DIC[key+match_len]<DIC[TREE[node].key+match_len])
        { /* add to smaller_node */
            TREE[smaller_node].c_right=node;
            TREE[node].parent=smaller_node;
            smaller_node=node;
            node=TREE[smaller_node].c_right;
        }
        else
        { /* add to bigger_node */
            TREE[bigger_node].c_left=node;
            TREE[node].parent=bigger_node;
            bigger_node=node;
            node=TREE[bigger_node].c_left;
        }
    }
    TREE[smaller_node].c_right=NO_NODE;
    TREE[bigger_node].c_left=NO_NODE;
    /* swap c_left and c_right in the new node, because only here the smaller node is on c_left */
    node=key2node(key);
    smaller_node=TREE[node].c_right;
    TREE[node].c_right=TREE[node].c_left;
    TREE[node].c_left=smaller_node;
    DIC[key+max_match]=orig_lit;
    for(int i=0; i<MAX_BUCKETS; i++)
    {
        if(buckets[i].len!=0)
        {
            com->mb63[match_buffer_pos].len=buckets[i].len;
            com->mb63[match_buffer_pos].u.ptr=buckets[i].u.ptr;
            match_buffer_pos++;
        }
    }
    com->mb63[match_buffer_pos].len=0;
    com->mb63[match_buffer_pos].u.lit=DIC[key];
    match_buffer_pos++;
    return match_buffer_pos;
}

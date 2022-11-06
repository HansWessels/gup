/*
** sliding dictionary 32 bit
** 1 Ondersteun match lengtes van 1 t/m dictionary size
** 2 ondersteun alle pointer lengtes
** 3 alle variabelen zijn types
**   a uint8 = uint8_t; 1 byte, 1 fundamentele eenheid waarop de compressor werkt, 8 bits
**   b match_t = uint32_t: bevat de lengte van de match, is nul bij geen match
**   c ptr_t = uint32_t: relatieve offset van de match, geinverteerd, 0 is 1 karakter terug
**   d index_t = uint32_t: index in een van de array's
**   e costs_t = uint64_t: geschatte kosten in (fracties van) bits
**   f node_t = {index_t parent, index_t c_left, index_t c_right, index_t link}: node in de dictionary tree
**   g NC = 256; Number of Characters, aantal verschillende karaters in uint8
**   h HASH_SIZE = 65536+256; groote van de hash array voor de eerste karakter van een insert, we zouden deze NC*NC*NC kunnen maken dan is het altijd raak...
** 4 back match lengte
** 5 match history foot current match.. hoe lang moet deze zijn
** 6 zeef 34, gewoon terukijken in de dictionary
** 7 de gehele file inladen tot 4 GB en dan?
**   a maximum block size is gelijk aan dictionary size, daarna reset van de packer
**   b eventueel met N-bytes alles terugschuiven en de structuren aanpassen, ingewikkeld schuiven
** 8 van iedere positie de matchlengte opslaan, matchlengte == 0 -> literal
** 9 types
**   a uint8 dictionary[file_size]; sliding dictionary en original file
**   b match_t match_len[file_size]; match lengte op iedere positie van de file
**   c ptr_t ptr_len[file_size]; bij behorende pointer lengte op iedere positie van de file
**   d match_t backmatch_len[file_size]; back match lengte op iedere positie van de file
**   e uint8 compressed_data[...]; de gecomprimeerde data, daarvoor wordt de cost array gebruikt. Maximale expansie van de data is 4
**   f index_t match_1[NC]; laatst geziene locatie van een enkele byte -> matchlen=1
**   g index_t match_2[NC*NC]; laatst geziene locatie van twee bytes -> matchlen=2
**   h index_t hash_table[HASH_SIZE]; 1e locatie voor de hash
**   j tree_t tree32[file_size]; dictionary tree
**   k cost_t cost[file_size]; geschatte kosten om tot een bepaalde plek te komen
** 10 functies voor de geschatte literal, len en ptr kosten
**
** Hoe moet een insert verlopen?
** 1 verwijder de node current_node-dictionary_size
** 2 lengte 1 match? vind lengte 1 match
** 3 lengte 2 match? vind lengte 2 match
** 4 bereken lengte 3 hash
** 5 insert en match de nieuwe node
** 6 return match lengte
*/

#include "sld32.h"
#include "compress.h"
#include "encode.h"

#define NO_NODE 0

gup_result init_dictionary32(packstruct *com);
void free_dictionary32(packstruct *com);
index_t *hash(index_t pos, packstruct* com);
match_t find_dictionary32(index_t pos, packstruct* com);

gup_result init_dictionary32(packstruct *com)
{
	com->dictionary=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(uint8), com->gm_propagator);
	if (com->dictionary == NULL)
	{
		return GUP_NOMEM;
	}
	com->match_len=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(match_t), com->gm_propagator);
	if (com->match_len == NULL)
	{
		return GUP_NOMEM;
	}
	com->ptr_len=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(ptr_t), com->gm_propagator);
	if (com->ptr_len == NULL)
	{
		return GUP_NOMEM;
	}
	com->backmatch_len=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(match_t), com->gm_propagator);
	if (com->backmatch_len == NULL)
	{
		return GUP_NOMEM;
	}
	com->match_1=com->gmalloc((NC)*sizeof(index_t), com->gm_propagator);
	if (com->match_1 == NULL)
	{
		return GUP_NOMEM;
	}
	com->match_2=com->gmalloc((NC*NC)*sizeof(index_t), com->gm_propagator);
	if (com->match_2 == NULL)
	{
		return GUP_NOMEM;
	}
	com->hash_table=com->gmalloc((HASH_SIZE32)*sizeof(index_t), com->gm_propagator);
	if (com->hash_table == NULL)
	{
		return GUP_NOMEM;
	}
	com->tree32=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(node_t), com->gm_propagator);
	if (com->tree32 == NULL)
	{
		return GUP_NOMEM;
	}
	com->cost=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(cost_t), com->gm_propagator);
	if (com->cost == NULL)
	{
		return GUP_NOMEM;
	}
	com->compressed_data=(void*)com->cost;
	/* initialiseer de index_hashes en de kosten */
	memset(com->hash_table, 0, (HASH_SIZE32)*sizeof(index_t));
	memset(com->match_1, 0, (NC)*sizeof(index_t));
	memset(com->match_2, 0, (NC*NC)*sizeof(index_t));
	memset(com->cost, 0xFF, (com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(cost_t));
//	memset(com->tree32, 0, (com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(node_t));
	return GUP_OK;
}

void free_dictionary32(packstruct *com)
{
	com->gfree(com->dictionary, com->gf_propagator);
	com->gfree(com->match_len, com->gf_propagator);
	com->gfree(com->ptr_len, com->gf_propagator);
	com->gfree(com->backmatch_len, com->gf_propagator);
	com->gfree(com->match_1, com->gf_propagator);
	com->gfree(com->match_2, com->gf_propagator);
	com->gfree(com->hash_table, com->gf_propagator);
	com->gfree(com->tree32, com->gf_propagator);
	com->gfree(com->cost, com->gf_propagator);
}

index_t *hash(index_t pos, packstruct* com)
{ /* bereken de positie in de hash table en geef deze positie terug */
	uint32_t val;
	val=(com->dictionary[pos]^com->dictionary[pos+1]);
	val<<=8;
	val|=(com->dictionary[pos+1]^com->dictionary[pos+2]);
	if(val==0)
	{ /* rle match */
		val+=65536+com->dictionary[pos];
	}
	return &com->hash_table[val];
}

match_t find_dictionary32(index_t pos, packstruct* com)
{
	match_t best_match=0;
	ptr_t best_ptr=0;
	match_t max_match=com->max_match32;
	if(max_match>(com->origsize-pos+DICTIONARY_START_OFFSET))
	{
		max_match=(com->origsize-pos+DICTIONARY_START_OFFSET);
	}
	if((pos-DICTIONARY_START_OFFSET)>com->maxptr32)
	{ /* remove node op pos-maxptr32-1 */
		index_t *parent;
		parent=com->tree32[pos-com->maxptr32-1].parent;
		if(parent!=NULL)
		{
			*parent=NO_NODE;
		}
	}
	if(com->min_match32==1)
	{ /* check for length 1 match */
		index_t match_pos;
		uint8 key=com->dictionary[pos];
		match_pos=com->match_1[key];
		if(match_pos!=NO_NODE)
		{
			if(com->dictionary[match_pos]==com->dictionary[pos])
			{
				best_match=1;
				best_ptr=pos-match_pos-1;
			}
		}
		com->match_1[key]=pos;
	}
	if((com->min_match32<=2) && (max_match>=2))
	{ /* check for length 2 match */
		index_t match_pos;
		uint16 key=(com->dictionary[pos]<<8)+com->dictionary[pos+1];
		match_pos=com->match_2[key];
		if(match_pos!=NO_NODE)
		{
			if((com->dictionary[match_pos]==com->dictionary[pos]) && (com->dictionary[match_pos+1]==com->dictionary[pos+1]))
			{
				best_match=2;
				best_ptr=pos-match_pos-1;
			}
		}
		com->match_2[key]=pos;
	}
	if(max_match>2)
	{ /* insert pos into slidingdictionary tree and try to find matches */
		index_t match_pos; /* working node */
		index_t* c_leftp;
		index_t* c_rightp;
		index_t* parent;
		uint8 orig=com->dictionary[pos+max_match];
		parent=hash(pos, com);
		match_pos=*parent;
		*parent=pos;
		com->tree32[pos].parent=parent;
		c_leftp=&com->tree32[pos].c_left;
		c_rightp=&com->tree32[pos].c_right;
		while(match_pos!=NO_NODE)
		{ /* insert next_pos in current_pos */
			match_t p=pos;
			match_t q=match_pos;
			com->dictionary[pos+max_match]=~com->dictionary[match_pos+max_match]; /* sentinel */
			while(com->dictionary[p]==com->dictionary[q])
			{
				p++;
				q++;
			}
			if((p-pos)>best_match)
			{ /* found new best_match */
				best_match=p-pos;
				best_ptr=pos-match_pos-1;
				if(best_match==max_match)
				{ /* found max_match, we are done inserting */
					break;
				}	
			}
			if(com->dictionary[p]<com->dictionary[q])
			{ /* insert node on left side of the tree */
				*c_leftp=match_pos;
				com->tree32[match_pos].parent=c_leftp;
				c_leftp=&com->tree32[match_pos].c_right;
				match_pos=com->tree32[match_pos].c_right;
			}
			else
			{ /* insert node on right side on the tree */
				*c_rightp=match_pos;
				com->tree32[match_pos].parent=c_rightp;
				c_rightp=&com->tree32[match_pos].c_left;
				match_pos=com->tree32[match_pos].c_left;
			}
		}
		if(best_match==max_match)
		{ /* node op match_pos kan er uit, deze is gelijk aan de pos node wat matching betreft */
			*c_rightp=com->tree32[match_pos].c_right;
			if(com->tree32[match_pos].c_right!=NO_NODE)
			{
				com->tree32[com->tree32[match_pos].c_right].parent=c_rightp;
			}
			*c_leftp=com->tree32[match_pos].c_left;
			if(com->tree32[match_pos].c_left!=NO_NODE)
			{
				com->tree32[com->tree32[match_pos].c_left].parent=c_leftp;
			}
		}	
		else
		{ /* close the child pointers */
			*c_leftp=NO_NODE;
			*c_rightp=NO_NODE;
		}
		com->dictionary[pos+max_match]=orig;
	}
	com->match32=best_match;
	com->ptr32=best_ptr;
	return best_match;
}


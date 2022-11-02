/*
** sliding dictionary 32 bit
** 1 Ondersteun match lengtes van 1 t/m dictionary size
** 2 ondersteun alle pointer lengtes
** 3 alle variabelen zijn types
**   a byte_t = uint8_t; 1 byte, 1 fundamentele eenheid waarop de compressor werkt, 8 bits
**   b match_t = uint32_t: bevat de lengte van de match, is nul bij geen match
**   c ptr_t = uint32_t: relatieve offset van de match, geinverteerd, 0 is 1 karakter terug
**   d index_t = uint32_t: index in een van de array's
**   e costs_t = uint64_t: geschatte kosten in (fracties van) bits
**   f node_t = {index_t parent, index_t c_left, index_t c_right, index_t link}: node in de dictionary tree
**   g NC = 256; Number of Characters, aantal verschillende karaters in byte_t
**   h HASH_SIZE = 65536+256; groote van de hash array voor de eerste karakter van een insert, we zouden deze NC*NC*NC kunnen maken dan is het altijd raak...
** 4 back match lengte
** 5 match history foot current match.. hoe lang moet deze zijn
** 6 zeef 34, gewoon terukijken in de dictionary
** 7 de gehele file inladen tot 4 GB en dan?
**   a maximum block size is gelijk aan dictionary size, daarna reset van de packer
**   b eventueel met N-bytes alles terugschuiven en de structuren aanpassen, ingewikkeld schuiven
** 8 van iedere positie de matchlengte opslaan, matchlengte == 0 -> literal
** 9 types
**   a byte_t dictionary[file_size]; sliding dictionary en original file
**   b match_t match_len[file_size]; match lengte op iedere positie van de file
**   c ptr_t ptr_len[file_size]; bij behorende pointer lengte op iedere positie van de file
**   d match_t backmatch_len[file_size]; back match lengte op iedere positie van de file
**   e byte_t compressed_data[file_size]; de gecomprimeerde data
**   f index_t match_1[NC]; laatst geziene locatie van een enkele byte -> matchlen=1
**   g index_t match_2[NC*NC]; laatst geziene locatie van twee bytes -> matchlen=2
**   h index_t hash[HASH_SIZE]; 1e locatie voor de hash
**   j tree_t tree[file_size]; dictionary tree
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

#define DICTIONAY_START_OFFSET 4  /* eerste 4 posities in de dictionary zijn voor speciaal gebruik */
#define NO_NODE 0

gup_result init_dictionary32(packstruct *com)
{
	com->dictionary=com->gmalloc((com->origsize+DICTIONAY_START_OFFSET)*sizeof(byte_t), com->gm_propagator);
	if (com->dictionary == NULL)
	{
		return GUP_NOMEM;
	}
	com->compressed_data=com->gmalloc((com->origsize+DICTIONAY_START_OFFSET)*sizeof(byte_t), com->gm_propagator);
	if (com->compressed_data == NULL)
	{
		return GUP_NOMEM;
	}
	com->match_len=com->gmalloc((com->origsize+DICTIONAY_START_OFFSET)*sizeof(match_t), com->gm_propagator);
	if (com->match_len == NULL)
	{
		return GUP_NOMEM;
	}
	com->ptr_len=com->gmalloc((com->origsize+DICTIONAY_START_OFFSET)*sizeof(ptr_t), com->gm_propagator);
	if (com->ptr_len == NULL)
	{
		return GUP_NOMEM;
	}
	com->backmatch_len=com->gmalloc((com->origsize+DICTIONAY_START_OFFSET)*sizeof(match_t), com->gm_propagator);
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
	com->hash_table=com->gmalloc((HASH_SIZE)*sizeof(index_t), com->gm_propagator);
	if (com->hash == NULL)
	{
		return GUP_NOMEM;
	}
	com->tree=com->gmalloc((com->origsize+DICTIONAY_START_OFFSET)*sizeof(tree_t), com->gm_propagator);
	if (com->tree == NULL)
	{
		return GUP_NOMEM;
	}
	com->cost=com->gmalloc((com->origsize+DICTIONAY_START_OFFSET)*sizeof(cost_t), com->gm_propagator);
	if (com->cost == NULL)
	{
		return GUP_NOMEM;
	}
	/* initialiseer de index_hashes en de kosten */
	memset(com->hash, 0, (HASH_SIZE)*sizeof(index_t));
	memset(com->match_1, 0, (NC)*sizeof(index_t));
	memset(com->match_2, 0, (NC*NC)*sizeof(index_t));
	memset(com->cost, 0xFF, (com->origsize+DICTIONAY_START_OFFSET)*sizeof(cost_t));
	return GUP_OK;
}

void free_dictionary32(packstruct *com)
{
	com->gfree(com->dictionary, com->gf_propagator);
	com->gfree(com->compressed_data, com->gf_propagator);
	com->gfree(com->match_len, com->gf_propagator);
	com->gfree(com->ptr_len, com->gf_propagator);
	com->gfree(com->backmatch_len, com->gf_propagator);
	com->gfree(com->match_1, com->gf_propagator);
	com->gfree(com->match_2, com->gf_propagator);
	com->gfree(com->hash_table, com->gf_propagator);
	com->gfree(com->tree=com, com->gf_propagator);
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
	return &hash_table[val];
}

match_t find_dictionary32(index_t pos, packstruct* com)
{
	match_t best_match=0;
	ptr_t best_ptr=0;
	match_t max_match=com->max_match;
	if(max_match>(com->origsize-pos))
	{
		max_match=(com->origsize-pos);
	}
	if(pos>com->maxptr32)
	{ /* remove node op pos-maxptr32-1 */
		index_t *parent;
		parent=com->tree[pos-com->maxptr32-1].parent;
		if(parent!=NULL)
		{
			*parent=NO_NODE;
		}
	}
	if(com->min_match==1)
	{
		index_t match_pos;
		byte_t key=com->dictionary[pos];
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
	if((com->min_match==2) && (max_match>=2))
	{
		index_t match_pos;
		word_t key=(com->dictionary[pos]<<8)+com->dictionary[pos+1];
		match_pos=com->match_2[key];
		if(match_pos!=NO_NODE)
		{
			if((com->dictionary[match_pos]==com->dictionary[pos]) && (com->dictionary[match_pos+1]==com->dictionary[pos+1])
			{
				best_match=2;
				best_ptr=pos-match_pos-1;
			}
		}
		com->match_2[key]==pos;
	}
	if(max_match>2)
	{
		index_t current_pos;
		index_t next_pos;
		index_t* parent;
		byte orig=com->dictionary[pos+max_match];
		parent=hash(pos, com);
		next_pos=*parent;
		com->tree[pos].parent=parent;
		com->tree[pos].c_left=NO_NODE;
		com->tree[pos].c_right=NO_NODE;
		current_pos=pos;
		while(next_pos!=NO_NODE)
		{ /* insert next_pos in current_pos */
			
		}
		com->dictionary[pos+max_match]=orig;
	}
}


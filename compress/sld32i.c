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
**   h HASH_SIZE32 = 65536; grootte van de hash array voor de eerste karakter van een insert, we zouden deze NC*NC*NC kunnen maken dan is het altijd raak...
**                   de RLE's moeten gefilterd worden, anders bij grote matches zelfs op snelle machines erg traag, aparte insert voor RLE's
**     HASH_SIZE_RLE32 = (256*RLE32_DEPTH) 
**     RLE32_DEPTH = 1024
** 4 back match lengte? Niet van belang, de kostenfunctie met complete matchen maakt dit overbodig
** 5 match history voor current match.. hoe lang moet deze zijn? Niet van belang, de kostenfunctie met complete matchen maakt dit overbodig
** 6 zeef 34, gewoon terukijken in de dictionary
** 7 de gehele file inladen tot 4 GB en dan?
**   a maximum block size is gelijk aan dictionary size, daarna reset van de packer
**   b eventueel met N-bytes alles terugschuiven en de structuren aanpassen, ingewikkeld schuiven
** 8 van iedere positie de matchlengte opslaan, matchlengte == 0 -> literal
** 9 types
**   a uint8 dictionary[file_size]; sliding dictionary en original file
**   b match_t match_len[file_size]; match lengte op iedere positie van de file
**   c ptr_t ptr_len[file_size]; bij behorende pointer lengte op iedere positie van de file
**   //  d match_t backmatch_len[file_size]; back match lengte op iedere positie van de file
**   e uint8 compressed_data[...]; de gecomprimeerde data, daarvoor wordt de cost array gebruikt. Maximale expansie van de data is 4
**   f index_t match_1[NC]; laatst geziene locatie van een enkele byte -> matchlen=1
**   g index_t match_2[NC*NC]; laatst geziene locatie van twee bytes -> matchlen=2
**   h index_t hash_table[HASH_SIZE32]; 1e locatie voor de hash
**     index_t hash_table_rle[RLE_HASH_SIZE32]; 1e locatie voor de hash
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

#define NO_NODE 0

static gup_result init_dictionary32(packstruct *com);
static void free_dictionary32(packstruct *com);
static uint32_t hash(index_t pos, packstruct* com);
static void find_dictionary32(index_t pos, packstruct* com);
static void insert_rle(unsigned long cost, match_t max_match, index_t pos, packstruct* com);

#if (MAX_HIST!=0) 
	static ptr_t check_ptr_reuse(packstruct* com, index_t pos, unsigned long *cost, ptr_t ptr, match_t best_match);
	static void ptr_copy2(ptr_hist_t *src, ptr_hist_t *dst);
	static void ptr_copy(ptr_t ptr, index_t pos, ptr_hist_t *src, ptr_hist_t *dst);
	static void literal_pointer_swap(ptr_hist_t *ptr_hist);

	#define CHECK_PTR_REUSE(com, pos, cost, ptr, best_match) check_ptr_reuse(com, pos, cost, ptr, best_match)
	#define PTR_COPY2(src, dst) ptr_copy2(src, dst)
	#define PTR_COPY(ptr, pos, src, dst) ptr_copy(ptr, pos, src, dst)
	#define LITERAL_POINTER_SWAP(ptr_hist) literal_pointer_swap(ptr_hist)
#else
	#define CHECK_PTR_REUSE(com, pos, cost, ptr, best_match) 
	#define PTR_COPY2(src, dst) 
	#define PTR_COPY(ptr, pos, src, dst) 
	#define LITERAL_POINTER_SWAP(ptr_hist) 
#endif

#ifndef BEST_MATCH
	#define BEST_MATCH(best_match) best_match
#endif

static gup_result encode32(packstruct *com);

static gup_result init_dictionary32(packstruct *com)
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
	#if(MIN_MATCH==1)
	com->match_1=com->gmalloc((NC)*sizeof(index_t), com->gm_propagator);
	if (com->match_1 == NULL)
	{
		return GUP_NOMEM;
	}
	#endif
	#if(MIN_MATCH<=2)
	com->match_2=com->gmalloc((NC*NC)*sizeof(index_t), com->gm_propagator);
	if (com->match_2 == NULL)
	{
		return GUP_NOMEM;
	}
	#endif
	com->hash_table=com->gmalloc((HASH_SIZE32)*sizeof(index_t), com->gm_propagator);
	if (com->hash_table == NULL)
	{
		return GUP_NOMEM;
	}
	com->hash_table_rle=com->gmalloc((HASH_SIZE_RLE32)*sizeof(index_t), com->gm_propagator);
	if (com->hash_table_rle == NULL)
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
	#if (MAX_HIST!=0) 
	com->ptr_hist=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(ptr_hist_t), com->gm_propagator);
	if (com->ptr_hist == NULL)
	{
		return GUP_NOMEM;
	}
	#endif
	#ifdef LINK_HIST
	com->link2_hist=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(index_t), com->gm_propagator);
	if (com->link2_hist == NULL)
	{
		return GUP_NOMEM;
	}
	com->link3_hist=com->gmalloc((com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(index_t), com->gm_propagator);
	if (com->link3_hist == NULL)
	{
		return GUP_NOMEM;
	}
	#endif
	/* initialiseer de index_hashes en de kosten */
	memset(com->hash_table, 0, (HASH_SIZE32)*sizeof(index_t));
	memset(com->hash_table_rle, 0, (HASH_SIZE_RLE32)*sizeof(index_t));
	#if(MIN_MATCH==1)
	memset(com->match_1, 0, (NC)*sizeof(index_t));
	#endif
	#if(MIN_MATCH<=2)
	memset(com->match_2, 0, (NC*NC)*sizeof(index_t));
	#endif
	#ifdef LINK_HIST
	memset(com->link2_hist, 0, (com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(index_t));
	memset(com->link3_hist, 0, (com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(index_t));
	#endif
	memset(com->cost, 0xFF, (com->origsize+DICTIONARY_START_OFFSET+DICTIONARY_END_OFFSET)*sizeof(cost_t));
	{ /* init special cases, tree en costs vullen tot DICTIONARY_START_OFFSET */
		int i=DICTIONARY_START_OFFSET;
		while(--i>=0)
		{
			com->tree32[i].parent=NULL;
			com->tree32[i].c_left=NO_NODE;
			com->tree32[i].c_right=NO_NODE;
			com->cost[i]=0;
			#if (MAX_HIST!=0) 
			{
				int j;
				for(j=0; j<MAX_PTR_HIST; j++)
				{
					com->ptr_hist[i].ptr[j]=j;
					com->ptr_hist[i].pos[j]=DICTIONARY_START_OFFSET;
				}
			}
			#endif
		}
	}
	return GUP_OK;
}

static void free_dictionary32(packstruct *com)
{
	com->gfree(com->dictionary, com->gf_propagator);
	com->gfree(com->match_len, com->gf_propagator);
	com->gfree(com->ptr_len, com->gf_propagator);
	#if(MIN_MATCH==1)
	com->gfree(com->match_1, com->gf_propagator);
	#endif
	#if(MIN_MATCH<=2)
	com->gfree(com->match_2, com->gf_propagator);
	#endif
	com->gfree(com->hash_table, com->gf_propagator);
	com->gfree(com->hash_table_rle, com->gf_propagator);
	com->gfree(com->tree32, com->gf_propagator);
	com->gfree(com->cost, com->gf_propagator);
	#if (MAX_HIST!=0) 
	com->gfree(com->ptr_hist, com->gf_propagator);
	#endif
	#ifdef LINK_HIST
	com->gfree(com->link2_hist, com->gf_propagator);
	com->gfree(com->link3_hist, com->gf_propagator);
	#endif
}

static uint32_t hash(index_t pos, packstruct* com)
{ /* bereken de positie in de hash table en geef deze positie terug */
	uint32_t val;
	val=(com->dictionary[pos]^com->dictionary[pos+1]);
	val<<=8;
	val|=(com->dictionary[pos+1]^com->dictionary[pos+2]);
	return val;
}

#if (MAX_HIST!=0) 
static ptr_t check_ptr_reuse(packstruct* com, index_t pos, unsigned long *cost, ptr_t ptr, match_t best_match)
{ /* kijken of we de pointer kunnen hergebruiken */
	ptr_t hist_ptr;
	index_t hist_pos;
	match_t hist_len;
	index_t p;
	index_t q;
	
	hist_pos=com->ptr_hist[pos].pos[1];
	if(hist_pos<DICTIONARY_START_OFFSET)
	{
		return 0;
	}
	hist_ptr=com->ptr_hist[pos].ptr[1];
	hist_len=com->match_len[hist_pos];
	if(hist_len>0)
	{
		p=hist_pos-hist_len;
		if(ptr<(p-DICTIONARY_START_OFFSET))
		{
			q=p-ptr-1;
			do
			{
				if(com->dictionary[p++]!=com->dictionary[q++])
				{ /* neen, pointers zijn niet gelijk */
					return 0;
				}
			} while(--hist_len>0);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	{ /* we kunnen de huidige pointer ook voor de vorige match gebruiken, willen we dat ook? Let op of er al een swap gaande is... */
		unsigned long old_cost;
		unsigned long new_cost;
		hist_len=com->match_len[hist_pos];
		old_cost=COST_PTRLEN(hist_len, hist_ptr, hist_pos-hist_len, com->ptr_hist[hist_pos-hist_len].ptr);
		if(com->cost[hist_pos]==(com->cost[hist_pos-hist_len]+old_cost))
		{ /* hier is nog geen pointer swap */
			old_cost+=COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr);
			com->ptr_hist[pos].ptr[1]=ptr;
			new_cost=COST_PTRLEN(hist_len, ptr, hist_pos-hist_len, com->ptr_hist[hist_pos-hist_len].ptr);
			new_cost+=COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr);
			if(new_cost<old_cost)
			{ /* ja, swap, fix kosten */
//				printf("%X: old_cost=%lu+%lu, new_cost=%lu+%lu, old_ptr=%u new_ptr=%u, len=%u+%u\n", hist_pos-hist_len-DICTIONARY_START_OFFSET, COST_PTRLEN(hist_len, hist_ptr, hist_pos, com->ptr_hist[hist_pos-hist_len].ptr), old_cost-COST_PTRLEN(hist_len, hist_ptr, hist_pos, com->ptr_hist[hist_pos-hist_len].ptr), COST_PTRLEN(hist_len, ptr, hist_pos, com->ptr_hist[hist_pos-hist_len].ptr), COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr), hist_ptr, ptr, hist_len, best_match);
				*cost+=COST_PTRLEN(hist_len, ptr     , hist_pos, com->ptr_hist[hist_pos-hist_len].ptr);
				*cost-=COST_PTRLEN(hist_len, hist_ptr, hist_pos, com->ptr_hist[hist_pos-hist_len].ptr);
				return hist_ptr+1;
			}
			else
			{ /* nee, toch niet */
				com->ptr_hist[pos].ptr[1]=hist_ptr;
				return 0;
			}
		}
		else
		{ /* we hadden al een pointer swap */
//			old_cost+=COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr);
//			com->ptr_hist[pos].ptr[1]=ptr;
//			new_cost=COST_PTRLEN(hist_len, ptr, hist_pos-hist_len, com->ptr_hist[hist_pos-hist_len].ptr);
//			new_cost+=COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr);
//			printf("%X: ! old_cost=%lu+%lu, new_cost=%lu+%lu, old_ptr=%u new_ptr=%u, len=%u+%u\n", hist_pos-hist_len-DICTIONARY_START_OFFSET, COST_PTRLEN(hist_len, hist_ptr, hist_pos, com->ptr_hist[hist_pos-hist_len].ptr), old_cost-COST_PTRLEN(hist_len, hist_ptr, hist_pos, com->ptr_hist[hist_pos-hist_len].ptr), COST_PTRLEN(hist_len, ptr, hist_pos, com->ptr_hist[hist_pos-hist_len].ptr), COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr), hist_ptr, ptr, hist_len, best_match);
//			com->ptr_hist[pos].ptr[1]=hist_ptr;
			return 0;
		}
	}
	return 0;
}

#define UNSWAP_PTR(swap)													\
{																					\
	if(swap!=0)																	\
	{ /* zet oude pointer en originele kosten weer terug */		\
		com->ptr_hist[pos].ptr[1]=swap-1;								\
		cost=com->cost[pos];													\
	}																				\
}

static void ptr_copy2(ptr_hist_t *src, ptr_hist_t *dst)
{
	int i;
	for(i=0; i<MAX_PTR_HIST; i++)
	{
		dst->ptr[i]=src->ptr[i];
		dst->pos[i]=src->pos[i];
	}
}

static void ptr_copy(ptr_t ptr, index_t pos, ptr_hist_t *src, ptr_hist_t *dst)
{
	int i;
	int j=0;
	dst->ptr[0]=ptr;
	dst->pos[0]=pos;
	for(i=1; i<MAX_PTR_HIST; i++)
	{
		if(src->ptr[j]==ptr)
		{
			j++;
		}
		dst->ptr[i]=src->ptr[j];
		dst->pos[i]=src->pos[j];
		j++;
	}
}

static void literal_pointer_swap(ptr_hist_t *ptr_hist)
{ /* swap de eerste twee pointers */
	ptr_t ptr;
	index_t pos;
	ptr=ptr_hist->ptr[0];
	pos=ptr_hist->pos[0];
	ptr_hist->ptr[0]=ptr_hist->ptr[1];
	ptr_hist->pos[0]=ptr_hist->pos[1];
	ptr_hist->ptr[1]=ptr;
	ptr_hist->pos[1]=pos;
}

#endif	// (MAX_HIST!=0) 

static void find_dictionary32(index_t pos, packstruct* com)
{
	match_t best_match=0;
	unsigned long cost;
	match_t max_match;
	cost=com->cost[pos];
	max_match=MAX_MATCH32;
	if(max_match>(com->origsize-pos+DICTIONARY_START_OFFSET))
	{
		max_match=com->origsize-pos+DICTIONARY_START_OFFSET;
	}
	if(pos>MAX_PTR32)
	{ /* remove node op pos-maxptr32-1 */
		index_t *parent;
		parent=com->tree32[pos-MAX_PTR32-1].parent;
		if(parent!=NULL)
		{
			*parent=NO_NODE;
		}
	}
	#if (MAX_HIST!=0) 
	{ /* check for matches at pointer history positions */
		int i;
		uint8 orig=com->dictionary[pos+max_match];
		match_t best_match=MIN_MATCH-1;
		for(i=0; i<MAX_PTR_HIST; i++)
		{
			index_t match_pos=pos-com->ptr_hist[pos].ptr[i]-1;
			if((match_pos<pos) && (match_pos>DICTIONARY_START_OFFSET))
			{
				index_t p=pos;
				index_t q=match_pos;
				com->dictionary[pos+max_match]=~com->dictionary[match_pos+max_match]; /* sentinel */
				while(com->dictionary[p]==com->dictionary[q])
				{
					p++;
					q++;
				}
				if((p-pos)>BEST_MATCH(best_match))
				{ /* found new best_match */
					ptr_t ptr;
					ptr=pos-match_pos-1;
					best_match=(p-pos);
					{
						match_t i=MIN_MATCH;
						do
						{
							if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
							{
								com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
								com->match_len[pos+i]=i;
								com->ptr_len[pos+i]=ptr;
								PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+(pos+i));
							}
						} while(i++<best_match);
					}
					if(best_match==max_match)
					{
						break;
					}	
				}
			}
		}
		com->dictionary[pos+max_match]=orig;
	}
	#endif
	#if(MIN_MATCH==1)
	{ /* check for length 1 match */
		index_t match_pos;
		uint8 key=com->dictionary[pos];
		match_pos=com->match_1[key];
		if(match_pos!=NO_NODE)
		{
			if(com->dictionary[match_pos]==com->dictionary[pos])
			{
				ptr_t ptr;
				best_match=1;
				ptr=pos-match_pos-1;
				#if (MAX_HIST!=0) 
					ptr_t swap=CHECK_PTR_REUSE(com, pos, &cost, ptr, best_match);
				#endif
				if((cost+COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+best_match])
				{
					com->cost[pos+best_match]=cost+COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr);
					com->match_len[pos+best_match]=best_match;
					com->ptr_len[pos+best_match]=ptr;
					PTR_COPY(ptr, pos+best_match, com->ptr_hist+pos, com->ptr_hist+pos+best_match);
				}
				#if (MAX_HIST!=0) 
					UNSWAP_PTR(swap);
				#endif
			}
		}
		com->match_1[key]=pos;
	}
	#endif //if(MIN_MATCH==1)
	#if(MIN_MATCH<=2)
	if(max_match>=2)
	{ /* check for length 2 match */
		index_t match_pos;
		uint16 key=(com->dictionary[pos]<<8)+com->dictionary[pos+1];
		match_pos=com->match_2[key];
		if(match_pos!=NO_NODE)
		{
			#ifndef LINK_HIST
			if((com->dictionary[match_pos]==com->dictionary[pos]) && (com->dictionary[match_pos+1]==com->dictionary[pos+1]))
			{
				ptr_t ptr;
				best_match=2;
				ptr=pos-match_pos-1;
				if((cost+COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+best_match])
				{
					com->cost[pos+best_match]=cost+COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr);
					com->match_len[pos+best_match]=best_match;
					com->ptr_len[pos+best_match]=ptr;
					PTR_COPY(ptr, pos+best_match, com->ptr_hist+pos, com->ptr_hist+pos+best_match);
				}
			}
			#else
			{ /* ga kijken of we een lengte twee pointer reuse kunnen vinden */
				com->link2_hist[pos]=match_pos;
				while((match_pos!=NO_NODE) && ((match_pos+MATCH_2_CUTTOFF) >= pos))
				{
					if((com->dictionary[match_pos]==com->dictionary[pos]) && (com->dictionary[match_pos+1]==com->dictionary[pos+1]))
					{
						ptr_t ptr;
						best_match=2;
						ptr=pos-match_pos-1;
						ptr_t swap=CHECK_PTR_REUSE(com, pos, &cost, ptr, best_match);
						if((cost+COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+best_match])
						{
							com->cost[pos+best_match]=cost+COST_PTRLEN(best_match, ptr, pos, com->ptr_hist[pos].ptr);
							com->match_len[pos+best_match]=best_match;
							com->ptr_len[pos+best_match]=ptr;
							PTR_COPY(ptr, pos+best_match, com->ptr_hist+pos, com->ptr_hist+pos+best_match);
						}
						UNSWAP_PTR(swap);
						if(swap!=0)
						{ /* swap gevonden, klaar */
							break;
						}
					}
					match_pos=com->link2_hist[match_pos];
				}
			}
			#endif
		}
		com->match_2[key]=pos;
	}
	#endif //(MIN_MATCH<=2)
	if(max_match>2)
	{ /* insert pos into slidingdictionary tree and try to find matches */
		uint32_t h;
		uint8 orig=com->dictionary[pos+max_match];   
		if((com->rle_size>0) || ((h=hash(pos, com))==0))
		{ /* RLE hash */
			insert_rle(cost, max_match, pos, com);
		}
		else
		{
			index_t match_pos; /* working node */
			index_t* c_leftp;
			index_t* c_rightp;
			index_t* parent;
			#ifdef LINK_HIST
			ptr_t end_ptr=pos-DICTIONARY_START_OFFSET;
			#endif
			parent=&com->hash_table[h];
			match_pos=*parent;
			*parent=pos;
			com->tree32[pos].parent=parent;
			c_leftp=&com->tree32[pos].c_left;
			c_rightp=&com->tree32[pos].c_right;
			while(match_pos!=NO_NODE)
			{ /* insert next_pos in current_pos */
				index_t p=pos;
				index_t q=match_pos;
				com->dictionary[pos+max_match]=~com->dictionary[match_pos+max_match]; /* sentinel */
				while(com->dictionary[p]==com->dictionary[q])
				{
					p++;
					q++;
				}
				if((p-pos)>best_match)
				{ /* found new best_match */
					ptr_t ptr;
					ptr=pos-match_pos-1;
					#ifdef LINK_HIST
					if(best_match<3)
					{ /* eerste match van 3, link */
						com->link3_hist[pos]=match_pos;
					}
					if(ptr>MATCH_2_CUTTOFF)
					{
						end_ptr=ptr;
					}
					#endif
					best_match=p-pos;
					{
						match_t i=MIN_MATCH;
						do
						{
							if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
							{
								com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
								com->match_len[pos+i]=i;
								com->ptr_len[pos+i]=ptr;
								PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+pos+i);
							}
						} while(i++<best_match);
					}
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
				com->tree32[match_pos].parent=NULL;
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
			#ifdef LINK_HIST
			{ /* jaag de link pointers na op zoek naar pointer reuse */
				index_t match_pos;
				index_t repeat_match;
				index_t repeat_pos;
				index_t* list;
				match_t repeat_len;

				match_pos=com->link3_hist[pos];
				repeat_match=com->ptr_hist[pos].pos[1];
				repeat_len=com->match_len[repeat_match];
				repeat_match-=repeat_len;
				if(repeat_len==2)
				{
					list=com->link2_hist;
					if((repeat_match-DICTIONARY_START_OFFSET)>MATCH_2_CUTTOFF)
					{ /* beperk de lengte van de list tot MATCH_2_CUTTOFF */
						index_t i;
						i=list[repeat_match];
						while(i!=NO_NODE)
						{
							if((repeat_match-list[i])>MATCH_2_CUTTOFF)
							{
								list[i]=NO_NODE;
								break;
							}
							i=list[i];
						}
					}
				}
				else
				{
					list=com->link3_hist;
				}
				repeat_pos=list[repeat_match];
				while((match_pos!=NO_NODE) && (repeat_pos!=NO_NODE))
				{
					if((pos-match_pos)==(repeat_match-repeat_pos))
					{
						index_t p=pos;
						index_t q=match_pos;
						com->dictionary[pos+max_match]=~com->dictionary[match_pos+max_match]; /* sentinel */
						while(com->dictionary[p]==com->dictionary[q])
						{
							p++;
							q++;
						}
						{
							ptr_t ptr;
							match_t match=p-pos;
							ptr=pos-match_pos-1;
							ptr_t swap=CHECK_PTR_REUSE(com, pos, &cost, ptr, match);
							{
								match_t i=MIN_MATCH;
								do
								{
									if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
									{
										com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
										com->match_len[pos+i]=i;
										com->ptr_len[pos+i]=ptr;
										PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+pos+i);
									}
								} while(i++<match);
							}
							UNSWAP_PTR(swap);
						}
						match_pos=com->link3_hist[match_pos];
					}
					else
					{
						if((pos-match_pos)<(repeat_match-repeat_pos))
						{
							match_pos=com->link3_hist[match_pos];
						}
						else
						{
							repeat_pos=list[repeat_pos];
						}
					}
				}
			}
			{ /* jaag de link pointers na op zoek iets kortere match door MATCH_2_CUTTOFF */
				index_t match_pos;

				match_pos=com->link3_hist[pos];

				while(match_pos!=NO_NODE)
				{
//					if((pos-match_pos)>=MATCH_2_CUTTOFF)
					{
						index_t p=pos;
						index_t q=match_pos;
						com->dictionary[pos+max_match]=~com->dictionary[match_pos+max_match]; /* sentinel */
						while(com->dictionary[p]==com->dictionary[q])
						{
							p++;
							q++;
						}
						{
							ptr_t ptr;
							match_t match=p-pos;
							ptr=pos-match_pos-1;
							{
								match_t i=MIN_MATCH;
								do
								{
									if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
									{
										com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
										com->match_len[pos+i]=i;
										com->ptr_len[pos+i]=ptr;
										PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+pos+i);
									}
								} while(i++<match);
							}
							if(ptr>=end_ptr)
							{
								break;
							}
						}
					}
					match_pos=com->link3_hist[match_pos];
				}
			}
			#endif
		}
		com->dictionary[pos+max_match]=orig;
	}
	return;
}

static void insert_rle(unsigned long cost, match_t max_match, index_t pos, packstruct* com)
{ /* find rle matches, special case: rle_depth==(RLE32_DEPTH+2) in that case don't build tree but tread as an max_match */
	match_t best_match=0;
	index_t match_pos; /* working node */
	index_t* c_leftp;
	index_t* c_rightp;
	index_t* parent;
	uint32_t h;
	#ifdef LINK_HIST
	if(com->rle_size>0)
	{ /* op offset 1 gegarandeerd een match */
		com->link3_hist[pos]=pos-1;
	}
	else
	{
		com->link3_hist[pos]=com->hash_table_rle[com->dictionary[pos]*RLE32_DEPTH];
	}
	#endif
	if(com->rle_size>0)
	{ /* already running an RLE */
		if(com->rle_size==(RLE32_DEPTH-1))
		{ /* continue on max depth? */
			com->dictionary[pos+max_match]=~com->dictionary[pos]; /* sentinel, orig wordt door aanroeper terug gezet */
			if(com->dictionary[pos+RLE32_DEPTH+1]!=com->dictionary[pos])
			{ /* rle is getting smaller */
				com->rle_size--;
			}
		}
		else
		{
			com->rle_size--;
		}
		if(com->rle_size<(RLE32_DEPTH-1))
		{ /* hebben we een match op rle_size+1? */
			match_t rle=com->rle_size;
			match_pos=com->hash_table_rle[com->dictionary[pos]*RLE32_DEPTH+rle+1];
			if(match_pos!=NO_NODE)
			{
				match_t i=MIN_MATCH;
				ptr_t ptr=pos-match_pos-1;
				#if (MAX_HIST!=0) 
					ptr_t swap=CHECK_PTR_REUSE(com, pos, &cost, ptr, rle+3);
				#endif
				do
				{
					if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
					{
						com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
						com->match_len[pos+i]=i;
						com->ptr_len[pos+i]=ptr;
						PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+pos+i);
					}
				} while(i++<(rle+3));
				#if (MAX_HIST!=0) 
					UNSWAP_PTR(swap);
				#endif
			}
		}
	}
	else
	{ /* new RLE */
		match_t rle=3;
		com->dictionary[pos+max_match]=~com->dictionary[pos]; /* sentinel, orig wordt door aanroeper terug gezet */
		while(com->dictionary[pos+rle]==com->dictionary[pos])
		{
			rle++;
			if(rle==(RLE32_DEPTH+2))
			{
				break;
			}
		}
		rle-=3;
		com->rle_size=rle;
		while(rle>0)
		{ /* match rle's < rle */
			rle--;
			match_pos=com->hash_table_rle[com->dictionary[pos]*RLE32_DEPTH+rle];
			if(match_pos!=NO_NODE)
			{
				match_t i=MIN_MATCH;
				ptr_t ptr=pos-match_pos-1;
				#if (MAX_HIST!=0) 
					ptr_t swap=CHECK_PTR_REUSE(com, pos, &cost, ptr, rle+3);
				#endif
				do
				{
					if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
					{
						com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
						com->match_len[pos+i]=i;
						com->ptr_len[pos+i]=ptr;
						PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+pos+i);
					}
				} while(i++<(rle+3));
				#if (MAX_HIST!=0) 
					UNSWAP_PTR(swap);
				#endif
			}
		}
	}
	h=com->dictionary[pos]*RLE32_DEPTH+com->rle_size;
	best_match=0; /* reset best match */
	parent=&com->hash_table_rle[h];
	match_pos=*parent;
	*parent=pos;
	com->tree32[pos].parent=parent;
	c_leftp=&com->tree32[pos].c_left;
	c_rightp=&com->tree32[pos].c_right;
	while(match_pos!=NO_NODE)
	{ /* insert next_pos in current_pos */
		index_t p=pos+com->rle_size+3;
		index_t q=match_pos+com->rle_size+3;
		com->dictionary[pos+max_match]=~com->dictionary[match_pos+max_match]; /* sentinel */
		while(com->dictionary[p]==com->dictionary[q])
		{
			p++;
			q++;
		}
		if((p-pos)>BEST_MATCH(best_match))
		{ /* found new best_match */
			ptr_t ptr;
			best_match=p-pos;

			ptr=pos-match_pos-1;
			#if (MAX_HIST!=0) 
				ptr_t swap=CHECK_PTR_REUSE(com, pos, &cost, ptr, best_match);
			#endif
			{
				match_t i=MIN_MATCH;
				do
				{
					if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
					{
						com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
						com->match_len[pos+i]=i;
						com->ptr_len[pos+i]=ptr;
						PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+pos+i);
					}
				} while(i++<best_match);
			}
			#if (MAX_HIST!=0) 
				UNSWAP_PTR(swap);
			#endif
			if((best_match==max_match) || (com->rle_size==(RLE32_DEPTH-1)))
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
	if((best_match==max_match) || (com->rle_size==(RLE32_DEPTH-1)))
	{ /* node op match_pos kan er uit, deze is gelijk aan de pos node wat matching betreft */
		com->tree32[match_pos].parent=NULL;
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
	#ifdef LINK_HIST
	{ /* jaag de link pointers na op zoek naar pointer reuse */
		index_t match_pos;
		index_t repeat_match;
		index_t repeat_pos;
		index_t* list;
		match_t repeat_len;

		match_pos=com->link3_hist[pos];
		repeat_match=com->ptr_hist[pos].pos[1];
		repeat_len=com->match_len[repeat_match];
		repeat_match-=repeat_len;
		best_match=0;
		if(repeat_len==2)
		{
			list=com->link2_hist;
			if((repeat_match-DICTIONARY_START_OFFSET)>MATCH_2_CUTTOFF)
			{ /* beperk de lengte van de list tot MATCH_2_CUTTOFF */
				index_t i;
				i=list[repeat_match];
				while(i!=NO_NODE)
				{
					if((repeat_match-list[i])>MATCH_2_CUTTOFF)
					{
						list[i]=NO_NODE;
						break;
					}
					i=list[i];
				}
			}
		}
		else
		{
			list=com->link3_hist;
		}
		repeat_pos=list[repeat_match];
		while((match_pos!=NO_NODE) && (repeat_pos!=NO_NODE))
		{
			if((pos-match_pos)==(repeat_match-repeat_pos))
			{
				index_t p=pos;
				index_t q=match_pos;
				com->dictionary[pos+max_match]=~com->dictionary[match_pos+max_match]; /* sentinel */
				while(com->dictionary[p]==com->dictionary[q])
				{
					p++;
					q++;
				}
				if((p-pos)>best_match)
				{
					ptr_t ptr;
					best_match=p-pos;
					ptr=pos-match_pos-1;
					ptr_t swap=CHECK_PTR_REUSE(com, pos, &cost, ptr, best_match);
					{
						match_t i=MIN_MATCH;
						do
						{
							if((cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr))<com->cost[pos+i])
							{
								com->cost[pos+i]=cost+COST_PTRLEN(i, ptr, pos, com->ptr_hist[pos].ptr);
								com->match_len[pos+i]=i;
								com->ptr_len[pos+i]=ptr;
								PTR_COPY(ptr, pos+i, com->ptr_hist+pos, com->ptr_hist+pos+i);
							}
						} while(i++<best_match);
					}
					UNSWAP_PTR(swap);
				}
				if(repeat_len!=2)
				{
					do
					{ /* eet de hele link serie op */
						match_pos=com->link3_hist[match_pos];
						repeat_pos=com->link3_hist[repeat_pos];
					} while(((match_pos-1)==com->link3_hist[match_pos]) && ((repeat_pos-1)==com->link3_hist[repeat_pos]));
					repeat_pos=com->link3_hist[repeat_pos];
				}
				match_pos=com->link3_hist[match_pos];
			}
			else
			{
				if((pos-match_pos)<(repeat_match-repeat_pos))
				{
					match_pos=com->link3_hist[match_pos];
				}
				else
				{
					repeat_pos=list[repeat_pos];
				}
			}
		}
	}
	#endif
	return;
}

static gup_result encode32(packstruct *com)
{
	index_t current_pos = DICTIONARY_START_OFFSET; /* wijst de te packen byte aan */
	unsigned long bytes_to_do;
	unsigned long orig_size;
	unsigned long cost;
	int literal=0; /* we beginnen in 'pointermode' */
	{ /*- dictionary buffer vullen */
		long byte_count;
		if ((byte_count = com->buf_read_crc(com->origsize, com->dictionary+DICTIONARY_START_OFFSET, com->brc_propagator)) < 0)
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
		orig_size = byte_count;
		bytes_to_do = orig_size;
	}
	com->packed_size = 0;
	while (bytes_to_do)
	{ /*- hoofd_lus, deze lus zorgt voor al het pack werk */
		cost=com->cost[current_pos-1];
		cost+=COST_LIT(com->dictionary[current_pos]);
		if(cost<com->cost[current_pos])
		{ /* hier komen met een literal is het goedkoopst */
			PTR_COPY2(com->ptr_hist+(current_pos-1), com->ptr_hist+current_pos);
			if(literal==0)
			{ /* we zitten in pointer mode, swap de eerste twee pointers */
				LITERAL_POINTER_SWAP(com->ptr_hist+current_pos);
				literal++;
			}
			com->match_len[current_pos]=0;
			com->cost[current_pos]=cost;
		}
		else
		{
			literal=0;
		}
      find_dictionary32(current_pos, com);
		current_pos++;
		bytes_to_do--;
	}
	{ /*- de laatste stap */
		cost=com->cost[current_pos-1];
		cost+=COST_LIT(com->dictionary[current_pos]);
		if(cost<com->cost[current_pos])
		{ /* hier komen met een literal is het goedkoopst */
			com->match_len[current_pos]=0;
			com->cost[current_pos]=cost;
		}
	}
	{	/* terug rekenen voor de goedkoopste weg */
		match_t match;
		ptr_t ptr;
		match=com->match_len[current_pos];
		ptr=com->ptr_len[current_pos];
		com->match_len[current_pos]=0;
		while(current_pos>DICTIONARY_START_OFFSET)
		{
			if(match==0)
			{ /* literal */
				current_pos--;
				match=com->match_len[current_pos];
				ptr=com->ptr_len[current_pos];
				com->match_len[current_pos]=0;
			}
			else
			{
				match_t new_match;
				ptr_t new_ptr;
				current_pos-=match;
				new_match=com->match_len[current_pos];
				new_ptr=com->ptr_len[current_pos];
				com->match_len[current_pos]=match;
				com->ptr_len[current_pos]=ptr;
				#if (MAX_HIST!=0) 
				{ /* probeer ptr te vervangen door een pointer hystory match */
					unsigned long cost;
					cost=com->cost[current_pos];
					if((cost+COST_PTRLEN(match, ptr, current_pos, com->ptr_hist[current_pos].ptr))!=com->cost[current_pos+match])
					{ /* ptr reuse, detect because ptr_hist is wrong here */
						index_t hist_pos;
						ptr_t hist_ptr;
						match_t hist_len;
						hist_pos=com->ptr_hist[current_pos].pos[1];
						if(hist_pos>DICTIONARY_START_OFFSET)
						{
							int old_cost;
							int new_cost;
							int delta;
							hist_ptr=com->ptr_len[hist_pos];
							hist_len=com->match_len[hist_pos];
//							printf("%X: old_cost=%lu, new_cost=%lu, old_ptr=%u, new_ptr=%u len=%u+%u\n", hist_pos-hist_len-DICTIONARY_START_OFFSET, COST_PTRLEN(match, ptr, current_pos, com->ptr_hist[current_pos].ptr), com->cost[current_pos+match]-cost, hist_ptr, ptr, hist_len, match);
							old_cost=(int)COST_PTRLEN(hist_len, hist_ptr, hist_pos-hist_len, com->ptr_hist[hist_pos-hist_len].ptr);
							new_cost=(int)COST_PTRLEN(hist_len, ptr     , hist_pos-hist_len, com->ptr_hist[hist_pos-hist_len].ptr);
							delta=new_cost-old_cost;
//							printf("delta=%i old: cost(%u, %u)=%i new: cost(%u, %u)=%i\n", delta, hist_ptr, hist_len, old_cost, ptr, hist_len, new_cost);
							com->ptr_len[hist_pos]=ptr;
							/* fix nu alles tot current_pos */
							do
							{
								com->cost[hist_pos]+=delta;
								hist_pos++;
							} while(hist_pos<=current_pos);
						}
					}
				}
				#endif
				match=new_match;
				ptr=new_ptr;
			}
		}
	}
	{
		gup_result res;
		if((res=compress(com))!=GUP_OK)
		{
			return res;
		}
	}
	return GUP_OK;
}

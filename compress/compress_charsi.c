static void make_hufftable32(uint8* len, uint16* tabel, uint16* freq, uint16 totaalfreq, int nchar, int max_hufflen, packstruct *com); /* maakt huffman tabel */
static void make_huffmancodes32(uint16* table, uint8* len, int nchar); /* maakt de huffman codes */
static gup_result compress_chars32(index_t *pos, uint16 entries, packstruct *com);

/*************************    make_hufftable    *************************
 Function:    make_hufftable
 Purpose:     Assign Huffman codes to characters, based on the occurrence
              frequency of each character.  Normally, standard Huffman codes
              are assigned.  However, if this means that the longest Huffman
              code is longer than <max_hufflen> bits, the Huffman codes will
              be re-assigned to make the longest Huffman code not exceed
              <max_hufflen> bits.
 Input:       freq       Occurrence frequency of each character.  freq[i]
                         contains the occurrence frequency for character i.
              totalfreq  Sum of all occurrence frequencies.
              nchar      Number of characters in the character set.
 Output:      table      Huffman code for each character.  table[i] will be
                         the Huffman code for character i.
              len        Length of the Huffman codes for the characters, in
                         bits. len[i] will be the length of table[i].
 Assumptions: 1. Elements 0..N-1 of <freq> are defined by the caller.
              2. Elements -X_CHARS..-1 of <freq> exist and may be
                 modified.
              3. Elements N..2*N-1 of <freq> exist and may be modified.
              4. <totalfreq> is less than 0xFFFF .  Note that this implies that
                 all elements freq[0..N-1] are less than 0xFFFF .
              5. <nchar> is less than or equal to <NC>.
              6. Elements N..2*N-1 of <len> exist and may be modified.
******************************************************************************/

static void make_hufftable32(uint8 * len,        /* O: lengths of the Huffman codes      */
                    uint16 * table,      /* O: Huffman codes                     */
                    uint16 * freq,       /* I: occurrence frequencies            */
                    uint16 totalfreq,    /* I: sum of all occurrence frequencies */
                    int nchar,         /* I: number of characters in <freq>    */
                    int max_hufflen,   /* I: maximum huffman code length */
                    packstruct * com)
{
	printf("nchar=%i\n", nchar);
  memset(len, 0, nchar);
	printf("b\n");
  if(totalfreq != 0)
  {
    for(;;)
    {
      int up1[X_CHARS + NC + NC + 1];  /* linked list of characters          */
      uint16 xfreq[X_CHARS + NC + NC];   /* modified character frequencies     */
      int *up = (up1 + X_CHARS);

      { /*- link de X_CHARS met elkaar */
        int *p = up1;
        int i = -X_CHARS + 2;

        *p++ = 2 * nchar - 1;          /* terminate up array */
        do
        {
          *p++ = i;
        }
        while(++i < 0);
        *p = -X_CHARS;
      }
      { /*- zet X_CHARS freq op 0xffff */
        int i = HUFF_HIGH_FREQS;
        uint16 tmp = 0xffff;
        uint16 *p = freq;

        do
        {
          *--p = tmp;
        }
        while(--i!=0);
        freq[-X_CHARS] = tmp;
        /* zet freq[] boven nchar op 0xffff */
        i = nchar;
        p = freq + nchar;
        do
        {
          *p++ = tmp;
        }
        while(--i!=0);
      }
      { /*- zet karakters in linked list */
        uint16 *p = freq;                /* current character frequency        */
        int c = 0;                     /* current character                  */

        do
        {
          uint16 currfreq;               /* frequency of the current character <c> */

          if((currfreq = *p++) != 0)
          {
            if(currfreq <= HUFF_MAX_SINGLE_FREQ)
            {
              up[c] = up1[currfreq];
              up1[currfreq] = c;
            }
            else
            {
              int c1 = LOG(currfreq) - 17;  /* 17=maximale LOG waarde +1     */
              int c2;

              for(;;)
              {
                c2 = up[c1];
                if(freq[c2] >= currfreq)
                {
                  up[c1] = c;
                  up[c] = c2;
                  break;
                }
                c1 = up[c2];
                if(freq[c1] >= currfreq)
                {
                  up[c2] = c;
                  up[c] = c1;
                  break;
                }
              }
            }
          }
        }
        while(++c < nchar);
      }
      {
        int mem[NC + NC];
        int *child = mem;              /* left and right child of the pseudo-characters */
        int new_char = nchar;          /* pseudo-character                   */

        {
          int low_p = -X_CHARS + 1;    /* source index for low half          */
          int high_p = nchar;          /* source index for high half         */
          uint16 new_freq;               /* frequency of new character         */

          while((low_p = up[low_p]) < 0)
          {
            ;
          }
          if((new_freq = freq[low_p]) == totalfreq)
          { /*- there is only one character in the array, we're done */
            return;
          }
          *child++ = low_p;            /* define first child of pseudo-char */
          while((low_p = up[low_p]) < 0) /* find next char of low half */
          {
            ;
          }
          for(;;)
          {
            if (freq[low_p] <= freq[high_p])
            {
              *child++ = low_p;        /* 2nd child of pseudo-char */
              freq[new_char++] = new_freq + freq[low_p];  /* frequency of pseudo-char */
              while((low_p = up[low_p]) < 0) /* find next char */
              {
                ;
              }
            }
            else
            {
              *child++ = high_p;       /* 2nd child of pseudo-char */
              freq[new_char++] = new_freq + freq[high_p++]; /* frequency of pseudo-char */
            }
            if((new_freq = freq[low_p]) <= freq[high_p])
            {
              *child++ = low_p;        /* define first child of pseudo-char */
              while((low_p = up[low_p]) < 0) /* find next char */
              {
                ;
              }
            }
            else
            {
              if((new_freq = freq[high_p]) >= totalfreq)
              {
                break;
              }
              else
              {
                *child++ = high_p++;   /* define first child of pseudo-char */
              }
            }
          }
        }
        {
          uint8 *p = len + new_char;
          uint8 tmp;

          p[-1] = 0;                   /* len[new_char]=0 */
          new_char -= nchar;
          do
          {
            tmp = *--p + 1;
            len[*--child] = tmp;
            len[*--child] = tmp;
          }
          while(--new_char > 0);
          if(tmp <= max_hufflen)
          {
            make_huffmancodes32(table, len, nchar);
            return;
          }
          else
          {
            if((xfreq + X_CHARS) != freq)
            {
              memcpy(xfreq + X_CHARS, freq, nchar * sizeof (*freq));
              freq = xfreq + X_CHARS;
            }
            freq[*child]++;            /* fix lowest freq */
            do
            {
              child += 2;
            }
            while(*p++ != 0);
            while(*--child >= nchar)
            {
              ;
            }
            freq[*child]--;            /* fix highest freq */
          }
        }
      }
    }
  }
}


/************************    make_huffmancodes    ************************
 Function:    make_huffmancodes
 Purpose:     Generate Huffman codes, based on the Huffman code lengths of the
              characters.
 Input:       len    Length of the Huffman codes for the characters, in bits.
                     len[i] will be the length of table[i].
              nchar  Number of characters in the character set.
 Output:      table  Huffman code for each character.  table[i] will be the
                     Huffman code for character i.
 Assumptions: For all len[i] with 0<=i<nchar: 0<=len[i]<=MAHUFFLEN.

******************************************************************************/

static void make_huffmancodes32(uint16 * table,   /* Tabel waarin de huffman codes komen te staan */
                       uint8 * len,     /* lengte van de karakters            */
                       int nchar)      /* aantal karakters                   */
{
  uint16 count[MAX_HUFFLEN + 1];
  uint16 huff[MAX_HUFFLEN + 1];

  { /*- hoeveel van iedere lengte hebben we eigenlijk? */
    int i = nchar;
    uint8 *p = len;

    memset(count, 0, sizeof (count));
    do
    {
      count[*p++]++;
    }
    while(--i!=0);
  }

  { /*- bereken eerste huffmancode van iedere lengte */
    int i = MAX_HUFFLEN - 1;           /* de eerste huffcode is gegarandeerd 0 */
    uint16 *p = huff + 1;                /* huff[0] doet er niet toe           */
    uint16 *q = count + 1;               /* count[0]==0 overslaan dus          */
    uint16 tmp = 0;

    *p++ = tmp;                        /* de huffmancode van len[1] komt met nul garantie */
    do
    {
      tmp += *q++;
      tmp += tmp;
      *p++ = tmp;
    }
    while(--i!=0);
  }
  { /*- assign huffman codes to characters */
    uint16 *p = table;
    uint8 *q = len;

    do
    {
      *p++ = huff[*q++]++;
    }
    while(--nchar > 0);
  }
}

static gup_result compress_chars32(index_t *pos, uint16 entries, packstruct *com)
{
	uint16 charfreq1[NC + NC + X_CHARS];
	uint16 *charfreq = charfreq1 + X_CHARS; /* frequentie tabel voor karakters, charfreq[-1 .. -X_CHARS] bestaan */
	uint16 ptrfreq1[MAX_NPT + MAX_NPT + X_CHARS];
	uint16 *ptrfreq = ptrfreq1 + X_CHARS;        /* frequentie tabel pointers          */
	uint16 pointer_count;
	uint16 charct;

	/* character frequentie op nul zetten */
	memset(charfreq, 0, NC * sizeof (*charfreq));
	/* zet pointer count op nul */
	memset(ptrfreq, 0, MAX_NPT * sizeof (*ptrfreq));
	{ /*- character frequentie tellen */
		uint16 i = entries;
		index_t current_pos=*pos;
		pointer_count=0;
		do
		{
			match_t match=com->match_len[current_pos];
			if(match==0)
			{
				charfreq[com->dictionary[current_pos]]++;
				current_pos++;
			}
			else
			{
				charfreq[match+NLIT-MIN_MATCH]++;
				ptrfreq[first_bit_set32(com->ptr_len[current_pos])]++;
				pointer_count++;
				current_pos+=match;
			}
		} while (--i!=0);
	}
	{ /*- bepalen tot welke wat het hoogste gebruikte character is */
		uint16 *p = charfreq + NC;
		while (!*--p)
		{
			;
		}
		charct = (uint16)(p + 1 - charfreq);
	}
	printf("A\n");
	make_hufftable32(com->charlen, com->char2huffman, charfreq, entries, NC, MAX_HUFFLEN, com);
	printf("B\n");
	make_hufftable32(com->ptrlen, com->ptr2huffman, ptrfreq, pointer_count, com->n_ptr, MAX_HUFFLEN, com);
	printf("C\n");
	com->special_header=NORMAL_HEADER;
	/*
	* Karakter frequenties zijn bekend, karakter huffman tabel is berekend.
	* Pointer frequenties zijn bekend, pointer huffman tabel is berekend.
	*/
	/*-
	* we hebben nu de huffman codes van de karakterset berekend, nu moeten
	* we de lengtes gaan coderen. deze staan in charlen c_len coderings
	* blok: 
	* lengte van de pointers die c_len coderen, er zijn 19 pointers:
	* 0          = c_len = 0 
	* 1 + 4 bits = de volgende 3-18 karakters hebben lengte 0
	* 2 + 9 bits = de volgende 20-531 karakters hebben lengte 0
	* 3          = c_len = 1 
	* :
	* n          = c_len = n-2 
	* : 
	* 18         = c_len = 16
	*/
	if(com->special_header!=NORMAL_HEADER)
	{ /* minimale header, alles literal */
		ST_BITS(entries, 16);           /* aantal huffman karakters */
		{ /*- special case 1, er is maar een character lengte */
			ST_BITS(0, 5);
			ST_BITS(10, 5); /* charlen is 8! */
		}
		{
			ST_BITS(256, 9); /* alle huffman lengtes zijn 0 */
		}
		/*
		* charlen is overgestuurd, nu weer een ptrlen
		*/
		{ /*- special case 3, er is maar een pointerlengte */
			ST_BITS(0, com->m_ptr_bit);
			ST_BITS(0, com->m_ptr_bit);
		}
		{
			uint16 i;
			for(i=0;i<NLIT;i++)
			{
				com->char2huffman[i]=i;
				com->charlen[i]=8;
			}
		}
	}
	else
	{ /* stuur standaard huffmantable blok, vanaf hier hebben wij charfreq niet meer nodig! */
		uint16 ptr_count = 0;
		int i;
		/* frequentie tabel op nul zetten voor gebruik pointers */
		memset(charfreq, 0, NCPT * sizeof (*charfreq));
		/* frequentie character lengtes tellen */
		for (i = 0; i < charct; i++)
		{
			if (com->charlen[i])
			{
				ptr_count++;
				charfreq[com->charlen[i] + 2]++;
			}
			else
			{ /*- charlen nul krijgt een speciale behandeling */
				int nulct = 1;

				while (!com->charlen[i + nulct])
				{
					nulct++;
				}
				if (nulct < 3)
				{
					charfreq[0] += (uint16)nulct;
					ptr_count += (uint16)nulct;
				}
				else
				{
					if (nulct < 20)
					{
						charfreq[1]++;
						if (nulct == 19)
						{
							charfreq[0]++;
							ptr_count++;
						}
					}
					else
					{
						charfreq[2]++;
					}
					ptr_count++;
				}
				i += nulct - 1;
			}
		}
		make_hufftable32(com->ptrlen1, com->ptr2huffman1, charfreq, ptr_count, NCPT, MAX_HUFFLEN, com);
		/*
		* Nu zijn alle ptrs gedefinieerd, stuur ze de ARJ file in
		*/
		ST_BITS(entries, 16);           /* aantal huffman karakters */
		{
			/*
			* belangrijk item, wat zijn de gevallen dat er slechts 1
			* pointerlengte overgedragen hoeft te worden? er is maar 1 pointer
			* lengte er is maar 1 karakter (dat kan wel meerdere ptrlens
			* veroorzaken)
			*/
			int vp = 0;
			int np = 1;
			uint8 *p = com->charlen;
			int len = *p;

			i = charct;
			do
			{
				int tmp = *p++;

				if (tmp)
				{
					vp++;
					if (tmp != len)
					{
						np = 0;
					}
				}
				else
				{
					np = 0;
				}
			} while (--i!=0);
			if ((vp < 2) || np)
			{ /*- special case 1, er is maar een character lengte */
				ST_BITS(0, 5);
				ST_BITS(*com->charlen + 2, 5);
			}
			else
			{
				long ptrct = NCPT;
				int extra_add = 0;

				while (!com->ptrlen1[ptrct - 1])
				{
					ptrct--;
				}
				if (com->ptrlen1[3] == 0)
				{
					extra_add = 1;
					if (com->ptrlen1[4] == 0)
					{
						extra_add = 2;
						if (com->ptrlen1[5] == 0)
						{
							extra_add = 3;
						}
					}
				}
				ST_BITS(ptrct, 5);          /* aantal pointers dat er aan komt */
				for (i = 0; i < ptrct; i++)
				{
					if (com->ptrlen1[i] < 7)
					{
						ST_BITS(com->ptrlen1[i], 3);
					}
					else
					{
						int rest = com->ptrlen1[i] - 7;

						ST_BITS(7, 3);
						while (rest!=0)
						{
							rest--;
							ST_BITS(1, 1);
						}
						ST_BITS(0, 1);
					}
					if (i == 2)
					{
						ST_BITS(extra_add, 2);
						i += extra_add;
					}
				}
			}
		}
		/*
		* charlen overgedragen, breng characters
		*/
		{
			/*
			* De enige special case voor de characters is dat er maar een
			* character is.
			*/
			uint16 vp = 0;
			uint8 *p = com->charlen;

			i = charct;
			do
			{
				if (*p++)
				{
					vp++;
				}
			} while (--i!=0);
			if (vp < 2)
			{ /*- special case 2, er is maar een karakter lengte */
				match_t match=com->match_len[*pos];
				if(match==0)
				{
					match=com->dictionary[*pos];
				}
				else
				{
					match+=NLIT-MIN_MATCH;
				}
				ST_BITS(0, 9);
				ST_BITS(match, 9);
				com->charlen[match] = 0;
				com->char2huffman[match] = 0;
			}
			else
			{
				ST_BITS(charct, 9);
				for (i = 0; i < charct; i++)
				{
					if (com->charlen[i])
					{
						ST_BITS(com->ptr2huffman1[com->charlen[i] + 2], com->ptrlen1[com->charlen[i] + 2]);
					}
					else
					{
						int nulct = 1;

						while (!com->charlen[i + nulct])
						{
							nulct++;
						}
						i += nulct - 1;
						if (nulct < 3)
						{
							while (nulct!=0)
							{
								ST_BITS(com->ptr2huffman1[0], com->ptrlen1[0]);
								nulct--;
							}
						}
						else
						{
							if (nulct < 20)
							{
								if (nulct == 19)
								{
									ST_BITS(com->ptr2huffman1[0], com->ptrlen1[0]);
									nulct--;
								}
								ST_BITS(com->ptr2huffman1[1], com->ptrlen1[1]);
								ST_BITS(nulct - 3, 4);
							}
							else
							{
								ST_BITS(com->ptr2huffman1[2], com->ptrlen1[2]);
								ST_BITS(nulct - 20, 9);
							}
						}
					}
				}
			}
		}
		/*
		* charlen is overgestuurd, nu weer een ptrlen
		*/
		{
			/*
			* wat is de specialcase voor de pointers? 1 er is maar een
			* pointerlengte
			*/
			uint16 vp = 0;
			uint16 *p = ptrfreq;

			i = com->n_ptr;
			do
			{
				if (*p++)
				{
					vp++;
				}
			} while (--i!=0);
			if (vp < 2)
			{ /*- special case 3, er is maar een pointerlengte */
				int j;

				if (vp == 1)
				{
					uint16 *p = ptrfreq;

					while (*p++ == 0)
					{
						;
					}
					j = (int)(p - 1 - ptrfreq);
				}
				else
				{
					j = 0;
				}
				ST_BITS(0, com->m_ptr_bit);
				ST_BITS(j, com->m_ptr_bit);
				com->ptrlen[j] = 0;
				com->ptr2huffman[j] = 0;
			}
			else
			{
				int ptrct = com->n_ptr;

				while ((ptrct) && (!com->ptrlen[ptrct - 1]))
				{
					ptrct--;
				}
				ST_BITS(ptrct, com->m_ptr_bit);
				for (i = 0; i < ptrct; i++)
				{
					if (com->ptrlen[i] < 7)
					{
						ST_BITS(com->ptrlen[i], 3);
					}
					else
					{
						int rest = com->ptrlen[i] - 7;

						ST_BITS(7, 3);
						while (rest!=0)
						{
							rest--;
							ST_BITS(1, 1);
						}
						ST_BITS(0, 1);
					}
				}
			}
		}
	}
	/*
	* alle codes overgedragen, stuur nu de gecodeerde message
	*/
	{
		uint16 i = entries;
		index_t current_pos=*pos;

		do
		{
			match_t match=com->match_len[current_pos];
			if(match==0)
			{
				ST_BITS(com->char2huffman[com->dictionary[current_pos]], com->charlen[com->dictionary[current_pos]]);
				current_pos++;
			}
			else
			{
				int j = first_bit_set32(com->ptr_len[current_pos]);
				ST_BITS(com->char2huffman[match+NLIT-MIN_MATCH], com->charlen[match+NLIT-MIN_MATCH]);
				ST_BITS(com->ptr2huffman[j], com->ptrlen[j]);
				j--;
				if(j > 0)
				{
					ST_BITS(((com->ptr_len[current_pos]) & (0xffff >> (16 - j))), j);
				}
				current_pos+=match;
			}
		} while(--i!=0);
		*pos=current_pos;
	}
	return GUP_OK;
}

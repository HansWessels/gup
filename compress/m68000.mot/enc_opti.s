|
| encode.c assembly optimized functions
| (c) 1996 Hans 'Mr Ni!' Wessels
| 1999-06-26 changed huffman routine to work with variable huffman length
|

	.globl make_hufftable

	.globl st_align
	.globl st_bits
| XDEF command


| The following exports should be after "%include command.mac"

	.globl _make_hufftable
	.globl _st_align
	.globl _st_bits

	.even

	.text

|void make_hufftable(uint8* len,       A0  O: lengths of the Huffman codes      */
|                    uint16* table,    A1  O: Huffman codes                     */
|                    uint16* freq,   4(SP) I: occurrence frequencies            */
|                    uint16 totalfreq, D0  I: sum of all occurrence frequencies */
|                    int nchar,       D1  I: number of characters in <freq>    */
|                    int max_hufflen  D2  I: maximum hufflength code 
|           packstruct *com   8(sp)I: command)
|
| Note that if this source is assembled for use with GCC, this
| function should be called from assembly with the parameters on
| the stack.

	.equ  X_CHARS,42           | aantal hulp characters
	.equ  NC,512          | max aantal characters

	.equ  freq_offset,56
	.equ max_hufflen_offset,70
	.equ  com_offset,72

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

_make_hufftable:
make_hufftable:
	movem.l  %d2-%d7/%a2-%a6,-(%sp)   | Save registers
	move.l   48(%sp),%a0           | *len
	move.l   52(%sp),%a1           | *table
	move.l   60(%sp),%d0           | totalfreq
	move.l   64(%sp),%d1           | nchar
	move.w   %d1,%d2               | nchar
	lea      0(%a0,%d2.w),%a2       | wis adres
	moveq    #0,%d3               | clear D2 om len te wissen
	move.l   com_offset(%sp),%a6   | command
	move.l   0(%a6),com_offset(%sp) | com_offset(sp)=command.fast_log
	lsr.w    #1,%d2               | nchar/2
	bcc.s    ll_0000
	move.b   %d3,-(%a2)
ll_0000:
	lsr.w    #1,%d2               | nchar/4
	bcc.s    ll_0001
	move.w   %d3,-(%a2)
ll_0001:
	lsr.w    #1,%d2               | nchar/8
	bcc.s    ll_0002
	move.l   %d3,-(%a2)
ll_0002:
	move.l   %d3,%d4               |
	lsr.w    #1,%d2               | nchar/16
	bcc.s    ll_0003
	movem.l  %d3-%d4,-(%a2)
ll_0003:
	beq      ll_0012        |
	move.l   %d3,%d5               |
	move.l   %d3,%d6               |
	lsr.w    #1,%d2               | nchar/32
	bcc.s    ll_0004
	movem.l  %d3-%d6,-(%a2)
	beq.s    ll_0012        |
ll_0004:
	move.l   %d3,%d7               |
	move.l   %d3,%a3               |
	move.l   %d3,%a4               |
	move.l   %d3,%a5               |
	lsr.w    #1,%d2               | nchar/64
	bcc.s    ll_0005
	beq.s    ll_0011
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0005:
	lsr.w    #1,%d2               | nchar/128
	bcc.s    ll_0006
	beq.s    ll_0010
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0006:
	lsr.w    #1,%d2               | nchar/256
	bcc.s    ll_0007
	beq.s    ll_0009
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0007:
	lsr.w    #1,%d2               | nchar/512
	bcs.s    ll_0008
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0008:
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0009:
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0010:
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0011:
	movem.l  %d3-%d7/%a3-%a5,-(%a2)
ll_0012:                     | einde clear_len
	tst.w    %d0                  | totalfreq==0?
	beq      ll_0052            | yep, done
	move.l   freq_offset(%sp),%a2  | haal freq van de stack
	move.w   %d1,%d2               | nchar
	add.w    %d2,%d2               | nchar*2
	lea      -((X_CHARS*2+NC*6+1)*2)(%sp),%sp | reserveer geheugen voor up[], xfreq[] en mem[]
| Register gebruik:
| D0 = totalfreq
| D1 = nchar
| D2 = 
| D3 = 
| D4 = 
| D5 = 
| D6 = 
| D7 = 
|
| A0 = len
| A1 = table
| A2 = freq
| A3 = up
| A4 = 
| A5 = 
| A6 = 
| A7 = up1
ll_0013:
	lea      2(%a7),%a4            | up1+1
	moveq    #2*(-X_CHARS+2),%d4  | 2(-X_CHARS+2) (=-80)
ll_0014:
	move.w   %d4,(%a4)+            | link de X_CHARS met elkaar
	addq.w   #2,%d4               |
	move.w   %d4,(%a4)+            |
	addq.w   #2,%d4               |
	move.w   %d4,(%a4)+            |
	addq.w   #2,%d4               |
	move.w   %d4,(%a4)+            |
	addq.w   #2,%d4               |
	move.w   %d4,(%a4)+            |
	addq.w   #2,%d4               |
	move.w   %d4,(%a4)+            |
	addq.w   #2,%d4               |
	move.w   %d4,(%a4)+            |
	addq.w   #2,%d4               |
	move.w   %d4,(%a4)+            |
	addq.w   #2,%d4               | 8 maal
	bne.s    ll_0014          |
	move.w   %d2,%d3               | 2*nchar
	subq.w   #1,%d3               | 2*nchar-1
	add.w    %d3,%d3               | 2(2*nchar-1)
	move.w   %d3,(%a4)             |
                                  | zet X_CHARS freq op $ffff (11 stuks)
	move.l   %a2,%a3               | freq
	moveq    #-1,%d3              | $ffff
	move.l   %d3,%d4               |
	move.l   %d3,%d5               |
	move.l   %d3,%d6               |
	move.l   %d3,%d7               |
	move.l   %d3,%a4               |
	move.l   %d3,%a5               |
	move.l   %d3,%a6               |
	movem.l  %d3-%d7,-(%a3)         | 10 stuks
	move.w   %d3,-(%a3)            | 11 stuks
                                  | zet freq[] boven nchar op $ffff
	add.w    %d2,%d2               |
	lea      0(%a2,%d2.w),%a3       | freq+2*2*nchar
	move.w   %d1,%d2               | nchar
	lsr.w    #1,%d2               | nchar/2
	bcc.s    ll_0015
	move.w   %d3,-(%a3)
ll_0015:
	lsr.w    #1,%d2               | nchar/4
	bcc.s    ll_0016
	move.l   %d3,-(%a3)
ll_0016:
	lsr.w    #1,%d2               | nchar/8
	bcc.s    ll_0017
	movem.l  %d3-%d4,-(%a3)
ll_0017:
	beq      ll_0028       |
	lsr.w    #1,%d2               | nchar/16
	bcc.s    ll_0018
	movem.l  %d3-%d6,-(%a3)
	beq      ll_0028       |
ll_0018:
	lsr.w    #1,%d2               | nchar/32
	bcc.s    ll_0019
	beq      ll_0027
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0019:
	lsr.w    #1,%d2               | nchar/64
	bcc.s    ll_0020
	beq      ll_0026
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0020:
	lsr.w    #1,%d2               | nchar/128
	bcc.s    ll_0021
	beq      ll_0025
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0021:
	lsr.w    #1,%d2               | nchar/256
	bcc.s    ll_0022
	beq      ll_0024
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0022:
	lsr.w    #1,%d2               | nchar/512
	bcs.s    ll_0023
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0023:
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0024:
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0025:
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0026:
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0027:
	movem.l  %d3-%d7/%a4-%a6,-(%a3)
ll_0028:                    | einde set_freq
	move.w   %d1,%d2               | nchar
	moveq    #0,%d3               | current char
	subq.w   #1,%d2               | nchar dbra compatible
	moveq    #0,%d4               | current freq
	move.l   %a2,%a4               | freq
	moveq    #31,%d5              | HUFF_MAX_SINGLE_FREQ
	lea      2*X_CHARS(%sp),%a3    | up
	move.l   com_offset+((X_CHARS*2+NC*6+1)*2)(%sp),%a6 | command.fast_log, log tabel
	move.l   %a3,%a5               | up
ll_0029:
	move.w   (%a4)+,%d4            | current freq
	beq.s    ll_0033
	cmp.w    %d4,%d5               | curfreq<max_single_freq
	bcs.s    ll_0030   | nope
	add.w    %d4,%d4               | currfreq*2
	move.w   (%sp,%d4.w),%d7        | up1[currfreq]
	move.w   %d3,(%sp,%d4.w)        | up1[currfreq]=c
	bra.s    ll_0033   |
ll_0030:
	moveq    #-17,%d6             | d6 = -(max_logwaarde+1)
	add.b    0(%a6,%d4.l),%d6       | d6+=log(currfreq)
	add.w    %d6,%d6               | d6=c1
ll_0031:
	move.w   0(%a3,%d6.w),%d7       | c2=up[c1]
	cmp.w    0(%a2,%d7.w),%d4       | if(freq[c2]>=currfreq)
	bhi.s    ll_0032      | nope
	move.w   %d3,0(%a3,%d6.w)       | up[c1]=c
	bra.s    ll_0033   |
ll_0032:
	move.w   0(%a3,%d7.w),%d6       | c1=up[c2]
	cmp.w    0(%a2,%d6.w),%d4       | if(freq[c1]>=currfreq)
	bhi.s    ll_0031 | nope
	move.w   %d3,0(%a3,%d7.w)       | up[c2]=c
	move.w   %d6,%d7               | c1
ll_0033:
	move.w   %d7,(%a5)+            | up[c]=...
	addq.w   #2,%d3               | c++
	dbra     %d2,ll_0029     | next!
| Register gebruik:
| D0 = totalfreq
| D1 = nchar
| D2 = high_p
| D3 = new_char
| D4 = new_freq
| D5 = low_p
| D6 = 
| D7 = low_freq
|
| A0 = len
| A1 = table
| A2 = freq
| A3 = up
| A4 = freq[high_p]
| A5 = freq[new_char]
| A6 = child*
| 2(A7) = up1
                                  | maak pseudo characters
	lea      2*(X_CHARS+NC+NC+1)(%sp),%a6 | mem[]
	move.w   %d1,%d2               | nchar
	moveq    #2*(-X_CHARS+1),%d5  | low_p
ll_0034:
	move.w   0(%a3,%d5.w),%d5       | low_p=up[low_p]
	bmi.s    ll_0034   |
	move.w   0(%a2,%d5.w),%d7       | new_freq=freq[low_p]
	cmp.w    %d0,%d7               | new_freq==total_freq?
	beq      ll_0051| yep, einde
	move.w   %d5,%d4               | we gaan low_p door twee delen
	lsr.w    #1,%d4               | low_p/=2
	move.w   %d4,(%a6)+            | *child++=low_p
	move.w   %d7,%d4               | new_freq=low_freq
ll_0035:
	move.w   0(%a3,%d5.w),%d5       | low_p=up[low_p]
	bmi.s    ll_0035   |
	move.w   0(%a2,%d5.w),%d7       | low_freq=freq[low_p]
	lea      0(%a2,%d3.w),%a4       | freq+high_p
	lea      (%a4),%a5             | freq+new_char
ll_0036:
	cmp.w    (%a4),%d7             | low_freq<=high_freq
	bhi.s    ll_0038    | nope
	add.w    %d7,%d4               | new_freq+=low_freq
	move.w   %d4,(%a5)+            | freq[new_char++]=new_freq
	move.w   %d5,%d4               | we gaan low_p door twee delen
	lsr.w    #1,%d4               | low_p/=2
	move.w   %d4,(%a6)+            | *child++=low_p
ll_0037:
	move.w   0(%a3,%d5.w),%d5       | low_p=up[low_p]
	bmi.s    ll_0037   |
	move.w   0(%a2,%d5.w),%d7       | low_freq=freq[low_p]
	bra.s    ll_0039        | volgend character
ll_0038:
	add.w    (%a4)+,%d4            | new_freq+=freq[high_p]
	move.w   %d2,(%a6)+            | *child++=high_p
	move.w   %d4,(%a5)+            | freq[new_char++]=new_freq
	addq.w   #1,%d2               | high_p++
ll_0039:
	cmp.w    (%a4),%d7             | low_freq<=high_freq
	bhi.s    ll_0041    | nope
	move.w   %d5,%d4               | we gaan low_p door twee delen
	lsr.w    #1,%d4               | low_p/=2
	move.w   %d4,(%a6)+            | *child++=low_p
	move.w   %d7,%d4               | new_freq=low_freq
ll_0040:
	move.w   0(%a3,%d5.w),%d5       | low_p=up[low_p]
	bmi.s    ll_0040   |
	move.w   0(%a2,%d5.w),%d7       | low_freq=freq[low_p]
	bra.s    ll_0036 | volgend character
ll_0041:
	move.w   (%a4)+,%d4            | new_freq=freq[high_p]++
	cmp.w    %d4,%d0               | new_freq==total_freq?
	beq.s    ll_0042 | done
	move.w   %d2,(%a6)+            | *child++=low_p
	addq.w   #1,%d2               | low_p++
	bra.s    ll_0036 | volgend character
ll_0042:
	lea      0(%a0,%d2.w),%a4       | len+new_char-1
	sub.w    %d1,%d2               | low_p-=nchar (=new_char-1)
	moveq    #0,%d7               | clear d7
	move.b   %d7,(%a4)+            | len[new_char]=0
ll_0043:
	move.b   -(%a4),%d7            | n_len
	addq.b   #1,%d7               | n_len++
	move.w   -(%a6),%d6            | *--child
	move.b   %d7,0(%a0,%d6.w)       | len[child]=nlen
	move.w   -(%a6),%d6            | *--child
	move.b   %d7,0(%a0,%d6.w)       | len[child]=nlen
	dbra     %d2,ll_0043   | next len
	cmp.w    max_hufflen_offset+((X_CHARS*2+NC*6+1)*2)(%sp),%d7 | n_len>max_hufflen
	bls.s    ll_0048 | maak de huffman codes
	lea      2*(X_CHARS+NC+NC+1+NC+NC+X_CHARS)(%sp),%a3 | adres x_freq
	cmp.l    %a3,%a2               | hebben we x_freq al geinitialiseerd?
	beq.s    ll_0045            | yep, geen copy
	move.w   %d1,%d7               | nchar
	subq.w   #1,%d7               | dbra compatible
	move.l   %a3,%a5               | copy adres
ll_0044:
	move.w   (%a2)+,(%a5)+         | copy
	dbra     %d7,ll_0044       | zoef, zoef! (kuch)
	move.l   %a3,%a2               | freq=x_char
ll_0045:
	move.w   (%a6),%d7             | *child
	add.w    %d7,%d7               | double
	addq.w   #1,0(%a2,%d7.w)       | freq[*child]++
ll_0046:
	addq.w   #4,%a6               | child+=2
	tst.b    (%a4)+               | *len++==0?
	bne.s    ll_0046           | omhoog
ll_0047:
	cmp.w    -(%a6),%d1            | *--child>=nchar?
	ble.s    ll_0047            | weer naar beneden
	move.w   (%a6),%d7             | hoogste freq
	add.w    %d7,%d7               | double
	subq.w   #1,0(%a2,%d7.w)       | freq[*child--]
	move.w   %d1,%d2
	add.w    %d2,%d2
	bra      ll_0013    | try again
ll_0048:
| Register gebruik:
| D0 = 
| D1 = nchar
| D2 = 
| D3 = 
| D4 = 
| D5 = 
| D6 = 
| D7 = 
|
| A0 = len
| A1 = table
| A2 = 
| A3 = 
| A4 = 
| A5 = 
| A6 = huff+1
| A7 = count
                                  | bereken de huffman codes
	moveq    #0,%d2               |
	moveq    #0,%d3               |
	moveq    #0,%d4               |
	moveq    #0,%d5               |
	moveq    #0,%d6               |
	movem.l  %d2-%d6,(%sp)          | clear 10
	movem.l  %d2-%d5,20(%sp)        | clear 8 totaal 18<MAX_HUFFLEN+1
	subq.w   #1,%d1               | nchar dbra compatible
	move.l   %a0,%a2               | len
	move.w   %d1,%d0               | nchar
ll_0049:
	move.b   (%a0)+,%d2            | *len++
	add.w    %d2,%d2               | double
	addq.w   #1,0(%sp,%d2.w)       | count[*len++]++
	dbra     %d0,ll_0049      | next
                                  | bereken de eerste huffman code van iedere lengte
	lea      2(%sp),%a5            | count+1 (q)
	move.w   %d6,(%a6)+            | *p++=tmp
                                  | uitgeschreven huff_loop
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp
	add.w    (%a5)+,%d6            | tmp+=*q++
	add.w    %d6,%d6               | tmp+=tmp
	move.w   %d6,(%a6)+            | *p++=tmp   15 maal (MAX_HUFFLEN-1)
	lea      -2*(16+1)(%a6),%a6| zet a6 weer recht (is nu huff) MAX_HUFFLEN+1
ll_0050:
	move.b   (%a2)+,%d5            | *len++
	add.w    %d5,%d5               | double
	move.w   0(%a6,%d5.w),(%a1)+    | *table++=huff[*len++]
	addq.w   #1,0(%a6,%d5.w)       | huff[*len++]++
	dbra     %d1,ll_0050 | next
ll_0051:
	lea      ((X_CHARS*2+NC*6+1)*2)(%sp),%sp | free mem voor up[], xfreq[] en mem[]
ll_0052:
	movem.l  (%sp)+,%d2-%d7/%a2-%a6   | einde
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

|void st_align(packstruct *com)
| align buffer on word boundary 
_st_align:
st_align:
	move.l   4(%sp),%a0
	tst.w    158(%a0)         | use align macro
	beq.s    ll_0054                | nope, done
	move.l   66(%a0),%a1   | rbuf current
	move.l   %a1,%d0                 |
	and.w    #1,%d0                 | even?
	beq.s    ll_0054                | yep, done
	move.w   4(%a0),%d0 | #bits in bitbuf
	cmp.w    #8,%d0                 | bits in bitbuf >= 8?
	bcc.s    ll_0055             | yep
	addq.w   #8,%d0                 | add byte to bitbuf
	move.w   %d0,4(%a0) | store bits_in_bitbuf
	move.l   6(%a0),%d0         | bitbuf
	move.b   -(%a1),%d0              | extra bitbuf data
	move.l   %a1,66(%a0)   | store rbuf current
	ror.l    #8,%d0                 | but bits in front
	move.l   %d0,6(%a0)         | store bitbuf and done
ll_0054:
	rts
ll_0055:
	subq.w   #8,%d0                 | remove byte from bitbuf
	move.w   %d0,4(%a0) | store bits_in_bitbuf
	move.l   6(%a0),%d0         | bitbuf
	rol.l    #8,%d0                 | but high byte on low pos
	move.b   %d0,(%a1)+              | write byte
	move.l   %a1,66(%a0)   | store rbuf current
	clr.b    %d0                    | kill byte
	move.l   %d0,6(%a0)         | store bitbuf and done
	rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

| st_bits functie
|void st_bits(unsigned long val, int bit_count, packstruct *com)
| deze functie gaat er van uit dat 0<=bit_count<=16
_st_bits:
st_bits:
	move.l   4(%sp),%d0
	move.l   8(%sp),%d1
	move.l   12(%sp),%a0
	move.l   %d2,-(%sp)
	addq.l   #4,%a0    | command struct+bits_in_bitbuf
	add.w    (%a0),%d1               | bit_count+=bits_in_bitbuf
	moveq    #32,%d2                | 32
	sub.w    %d1,%d2                 | bits_in_bitbuf>32?
	ble.s    ll_0057        | yep
	move.w   %d1,(%a0)+              | store bits_in_bitbuf
	lsl.l    %d2,%d0                 | val<<(32-bits_in_bitbuf)
	add.l    %d0,(%a0)               | tel bij bitbuf op
ll_0056:
	move.l   (%sp)+,%d2
	rts                            | done
ll_0057:
	neg.w    %d2                    | -(32-bits_in_bitbuf) -> bits_in_bitbuf-32
	move.w   %d2,(%a0)+              | store bits_in_bitbuf
	ror.l    %d2,%d0                 | val >> command.bits_in_bitbuf
	move.l   (%a0),%d1               | bitbuf+=val>>bits_in_bitbuf
	add.w    %d0,%d1                 | fix_bitbuf
	clr.w    %d0                    | wis ondeste bits
	lea      66-6(%a0),%a1 | &command.rbuf_current
	move.l   %d0,(%a0)               | command.bitbuf = val << (32 - bits_in_bitbuf)
	move.l   (%a1),%a0               | rbuf_current
	move.l   %d1,(%a0)+              | *((unsigned long *)rbuf_current)++ = bitbuf;
	move.l   %a0,(%a1)               | command.rbuf_current=rbuf_current
	move.l   (%sp)+,%d2
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

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
	moveml  d2-d7/a2-a6,sp@-   | Save registers
	movel   sp@(48),a0           | *len
	movel   sp@(52),a1           | *table
	movel   sp@(60),d0           | totalfreq
	movel   sp@(64),d1           | nchar
	movew   d1,d2               | nchar
	lea      a0@(0,d2:w),a2       | wis adres
	moveq    #0,d3               | clear D2 om len te wissen
	movel   sp@(com_offset),a6   | command
	movel   a6@(0),sp@(com_offset) | com_offset(sp)=command.fast_log
	lsrw    #1,d2               | nchar/2
	bcc    ll_0000
	moveb   d3,a2@-
ll_0000:
	lsrw    #1,d2               | nchar/4
	bcc    ll_0001
	movew   d3,a2@-
ll_0001:
	lsrw    #1,d2               | nchar/8
	bcc    ll_0002
	movel   d3,a2@-
ll_0002:
	movel   d3,d4               |
	lsrw    #1,d2               | nchar/16
	bcc    ll_0003
	moveml  d3-d4,a2@-
ll_0003:
	beq      ll_0012        |
	movel   d3,d5               |
	movel   d3,d6               |
	lsrw    #1,d2               | nchar/32
	bcc    ll_0004
	moveml  d3-d6,a2@-
	beq    ll_0012        |
ll_0004:
	movel   d3,d7               |
	movel   d3,a3               |
	movel   d3,a4               |
	movel   d3,a5               |
	lsrw    #1,d2               | nchar/64
	bcc    ll_0005
	beq    ll_0011
	moveml  d3-d7/a3-a5,a2@-
ll_0005:
	lsrw    #1,d2               | nchar/128
	bcc    ll_0006
	beq    ll_0010
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
ll_0006:
	lsrw    #1,d2               | nchar/256
	bcc    ll_0007
	beq    ll_0009
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
ll_0007:
	lsrw    #1,d2               | nchar/512
	bcs    ll_0008
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
ll_0008:
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
ll_0009:
	moveml  d3-d7/a3-a5,a2@-
	moveml  d3-d7/a3-a5,a2@-
ll_0010:
	moveml  d3-d7/a3-a5,a2@-
ll_0011:
	moveml  d3-d7/a3-a5,a2@-
ll_0012:                     | einde clear_len
	tstw    d0                  | totalfreq==0?
	beq      ll_0052            | yep, done
	movel   sp@(freq_offset),a2  | haal freq van de stack
	movew   d1,d2               | nchar
	addw    d2,d2               | nchar*2
	lea      sp@(-((X_CHARS*2+NC*6+1)*2)),sp | reserveer geheugen voor up[], xfreq[] en mem[]
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
	lea      a7@(2),a4            | up1+1
	moveq    #2*(-X_CHARS+2),d4  | 2(-X_CHARS+2) (=-80)
ll_0014:
	movew   d4,a4@+            | link de X_CHARS met elkaar
	addqw   #2,d4               |
	movew   d4,a4@+            |
	addqw   #2,d4               |
	movew   d4,a4@+            |
	addqw   #2,d4               |
	movew   d4,a4@+            |
	addqw   #2,d4               |
	movew   d4,a4@+            |
	addqw   #2,d4               |
	movew   d4,a4@+            |
	addqw   #2,d4               |
	movew   d4,a4@+            |
	addqw   #2,d4               |
	movew   d4,a4@+            |
	addqw   #2,d4               | 8 maal
	bne    ll_0014          |
	movew   d2,d3               | 2*nchar
	subqw   #1,d3               | 2*nchar-1
	addw    d3,d3               | 2(2*nchar-1)
	movew   d3,a4@             |
                                  | zet X_CHARS freq op $ffff (11 stuks)
	movel   a2,a3               | freq
	moveq    #-1,d3              | $ffff
	movel   d3,d4               |
	movel   d3,d5               |
	movel   d3,d6               |
	movel   d3,d7               |
	movel   d3,a4               |
	movel   d3,a5               |
	movel   d3,a6               |
	moveml  d3-d7,a3@-         | 10 stuks
	movew   d3,a3@-            | 11 stuks
                                  | zet freq[] boven nchar op $ffff
	addw    d2,d2               |
	lea      a2@(0,d2:w),a3       | freq+2*2*nchar
	movew   d1,d2               | nchar
	lsrw    #1,d2               | nchar/2
	bcc    ll_0015
	movew   d3,a3@-
ll_0015:
	lsrw    #1,d2               | nchar/4
	bcc    ll_0016
	movel   d3,a3@-
ll_0016:
	lsrw    #1,d2               | nchar/8
	bcc    ll_0017
	moveml  d3-d4,a3@-
ll_0017:
	beq      ll_0028       |
	lsrw    #1,d2               | nchar/16
	bcc    ll_0018
	moveml  d3-d6,a3@-
	beq      ll_0028       |
ll_0018:
	lsrw    #1,d2               | nchar/32
	bcc    ll_0019
	beq      ll_0027
	moveml  d3-d7/a4-a6,a3@-
ll_0019:
	lsrw    #1,d2               | nchar/64
	bcc    ll_0020
	beq      ll_0026
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
ll_0020:
	lsrw    #1,d2               | nchar/128
	bcc    ll_0021
	beq      ll_0025
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
ll_0021:
	lsrw    #1,d2               | nchar/256
	bcc    ll_0022
	beq      ll_0024
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
ll_0022:
	lsrw    #1,d2               | nchar/512
	bcs    ll_0023
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
ll_0023:
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
ll_0024:
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
ll_0025:
	moveml  d3-d7/a4-a6,a3@-
	moveml  d3-d7/a4-a6,a3@-
ll_0026:
	moveml  d3-d7/a4-a6,a3@-
ll_0027:
	moveml  d3-d7/a4-a6,a3@-
ll_0028:                    | einde set_freq
	movew   d1,d2               | nchar
	moveq    #0,d3               | current char
	subqw   #1,d2               | nchar dbra compatible
	moveq    #0,d4               | current freq
	movel   a2,a4               | freq
	moveq    #31,d5              | HUFF_MAX_SINGLE_FREQ
	lea      sp@(2*X_CHARS),a3    | up
	movel   sp@(com_offset+((X_CHARS*2+NC*6+1)*2)),a6 | command.fast_log, log tabel
	movel   a3,a5               | up
ll_0029:
	movew   a4@+,d4            | current freq
	beq    ll_0033
	cmpw    d4,d5               | curfreq<max_single_freq
	bcs    ll_0030   | nope
	addw    d4,d4               | currfreq*2
	movew   sp@(d4:w),d7        | up1[currfreq]
	movew   d3,sp@(d4:w)        | up1[currfreq]=c
	bra    ll_0033   |
ll_0030:
	moveq    #-17,d6             | d6 = -(max_logwaarde+1)
	addb    a6@(0,d4:l),d6       | d6+=log(currfreq)
	addw    d6,d6               | d6=c1
ll_0031:
	movew   a3@(0,d6:w),d7       | c2=up[c1]
	cmpw    a2@(0,d7:w),d4       | if(freq[c2]>=currfreq)
	bhi    ll_0032      | nope
	movew   d3,a3@(0,d6:w)       | up[c1]=c
	bra    ll_0033   |
ll_0032:
	movew   a3@(0,d7:w),d6       | c1=up[c2]
	cmpw    a2@(0,d6:w),d4       | if(freq[c1]>=currfreq)
	bhi    ll_0031 | nope
	movew   d3,a3@(0,d7:w)       | up[c2]=c
	movew   d6,d7               | c1
ll_0033:
	movew   d7,a5@+            | up[c]=...
	addqw   #2,d3               | c++
	dbra     d2,ll_0029     | next!
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
	lea      sp@(2*(X_CHARS+NC+NC+1)),a6 | mem[]
	movew   d1,d2               | nchar
	moveq    #2*(-X_CHARS+1),d5  | low_p
ll_0034:
	movew   a3@(0,d5:w),d5       | low_p=up[low_p]
	bmi    ll_0034   |
	movew   a2@(0,d5:w),d7       | new_freq=freq[low_p]
	cmpw    d0,d7               | new_freq==total_freq?
	beq      ll_0051| yep, einde
	movew   d5,d4               | we gaan low_p door twee delen
	lsrw    #1,d4               | low_p/=2
	movew   d4,a6@+            | *child++=low_p
	movew   d7,d4               | new_freq=low_freq
ll_0035:
	movew   a3@(0,d5:w),d5       | low_p=up[low_p]
	bmi    ll_0035   |
	movew   a2@(0,d5:w),d7       | low_freq=freq[low_p]
	lea      a2@(0,d3:w),a4       | freq+high_p
	lea      a4@,a5             | freq+new_char
ll_0036:
	cmpw    a4@,d7             | low_freq<=high_freq
	bhi    ll_0038    | nope
	addw    d7,d4               | new_freq+=low_freq
	movew   d4,a5@+            | freq[new_char++]=new_freq
	movew   d5,d4               | we gaan low_p door twee delen
	lsrw    #1,d4               | low_p/=2
	movew   d4,a6@+            | *child++=low_p
ll_0037:
	movew   a3@(0,d5:w),d5       | low_p=up[low_p]
	bmi    ll_0037   |
	movew   a2@(0,d5:w),d7       | low_freq=freq[low_p]
	bra    ll_0039        | volgend character
ll_0038:
	addw    a4@+,d4            | new_freq+=freq[high_p]
	movew   d2,a6@+            | *child++=high_p
	movew   d4,a5@+            | freq[new_char++]=new_freq
	addqw   #1,d2               | high_p++
ll_0039:
	cmpw    a4@,d7             | low_freq<=high_freq
	bhi    ll_0041    | nope
	movew   d5,d4               | we gaan low_p door twee delen
	lsrw    #1,d4               | low_p/=2
	movew   d4,a6@+            | *child++=low_p
	movew   d7,d4               | new_freq=low_freq
ll_0040:
	movew   a3@(0,d5:w),d5       | low_p=up[low_p]
	bmi    ll_0040   |
	movew   a2@(0,d5:w),d7       | low_freq=freq[low_p]
	bra    ll_0036 | volgend character
ll_0041:
	movew   a4@+,d4            | new_freq=freq[high_p]++
	cmpw    d4,d0               | new_freq==total_freq?
	beq    ll_0042 | done
	movew   d2,a6@+            | *child++=low_p
	addqw   #1,d2               | low_p++
	bra    ll_0036 | volgend character
ll_0042:
	lea      a0@(0,d2:w),a4       | len+new_char-1
	subw    d1,d2               | low_p-=nchar (=new_char-1)
	moveq    #0,d7               | clear d7
	moveb   d7,a4@+            | len[new_char]=0
ll_0043:
	moveb   a4@-,d7            | n_len
	addqb   #1,d7               | n_len++
	movew   a6@-,d6            | *--child
	moveb   d7,a0@(0,d6:w)       | len[child]=nlen
	movew   a6@-,d6            | *--child
	moveb   d7,a0@(0,d6:w)       | len[child]=nlen
	dbra     d2,ll_0043   | next len
	cmpw    sp@(max_hufflen_offset+((X_CHARS*2+NC*6+1)*2)),d7 | n_len>max_hufflen
	bls    ll_0048 | maak de huffman codes
	lea      sp@(2*(X_CHARS+NC+NC+1+NC+NC+X_CHARS)),a3 | adres x_freq
	cmpl    a3,a2               | hebben we x_freq al geinitialiseerd?
	beq    ll_0045            | yep, geen copy
	movew   d1,d7               | nchar
	subqw   #1,d7               | dbra compatible
	movel   a3,a5               | copy adres
ll_0044:
	movew   a2@+,a5@+         | copy
	dbra     d7,ll_0044       | zoef, zoef! (kuch)
	movel   a3,a2               | freq=x_char
ll_0045:
	movew   a6@,d7             | *child
	addw    d7,d7               | double
	addqw   #1,a2@(0,d7:w)       | freq[*child]++
ll_0046:
	addqw   #4,a6               | child+=2
	tstb    a4@+               | *len++==0?
	bne    ll_0046           | omhoog
ll_0047:
	cmpw    a6@-,d1            | *--child>=nchar?
	ble    ll_0047            | weer naar beneden
	movew   a6@,d7             | hoogste freq
	addw    d7,d7               | double
	subqw   #1,a2@(0,d7:w)       | freq[*child--]
	movew   d1,d2
	addw    d2,d2
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
	moveq    #0,d2               |
	moveq    #0,d3               |
	moveq    #0,d4               |
	moveq    #0,d5               |
	moveq    #0,d6               |
	moveml  d2-d6,sp@          | clear 10
	moveml  d2-d5,sp@(20)        | clear 8 totaal 18<MAX_HUFFLEN+1
	subqw   #1,d1               | nchar dbra compatible
	movel   a0,a2               | len
	movew   d1,d0               | nchar
ll_0049:
	moveb   a0@+,d2            | *len++
	addw    d2,d2               | double
	addqw   #1,sp@(0,d2:w)       | count[*len++]++
	dbra     d0,ll_0049      | next
                                  | bereken de eerste huffman code van iedere lengte
	lea      sp@(2),a5            | count+1 (q)
	movew   d6,a6@+            | *p++=tmp
                                  | uitgeschreven huff_loop
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp
	addw    a5@+,d6            | tmp+=*q++
	addw    d6,d6               | tmp+=tmp
	movew   d6,a6@+            | *p++=tmp   15 maal (MAX_HUFFLEN-1)
	lea      a6@(-2*(16+1)),a6| zet a6 weer recht (is nu huff) MAX_HUFFLEN+1
ll_0050:
	moveb   a2@+,d5            | *len++
	addw    d5,d5               | double
	movew   a6@(0,d5:w),a1@+    | *table++=huff[*len++]
	addqw   #1,a6@(0,d5:w)       | huff[*len++]++
	dbra     d1,ll_0050 | next
ll_0051:
	lea      sp@(((X_CHARS*2+NC*6+1)*2)),sp | free mem voor up[], xfreq[] en mem[]
ll_0052:
	moveml  sp@+,d2-d7/a2-a6   | einde
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

|void st_align(packstruct *com)
| align buffer on word boundary 
_st_align:
st_align:
	movel   sp@(4),a0
	tstw    a0@(158)         | use align macro
	beq    ll_0054                | nope, done
	movel   a0@(66),a1   | rbuf current
	movel   a1,d0                 |
	andw    #1,d0                 | even?
	beq    ll_0054                | yep, done
	movew   a0@(4),d0 | #bits in bitbuf
	cmpw    #8,d0                 | bits in bitbuf >= 8?
	bcc    ll_0055             | yep
	addqw   #8,d0                 | add byte to bitbuf
	movew   d0,a0@(4) | store bits_in_bitbuf
	movel   a0@(6),d0         | bitbuf
	moveb   a1@-,d0              | extra bitbuf data
	movel   a1,a0@(66)   | store rbuf current
	rorl    #8,d0                 | but bits in front
	movel   d0,a0@(6)         | store bitbuf and done
ll_0054:
	rts
ll_0055:
	subqw   #8,d0                 | remove byte from bitbuf
	movew   d0,a0@(4) | store bits_in_bitbuf
	movel   a0@(6),d0         | bitbuf
	roll    #8,d0                 | but high byte on low pos
	moveb   d0,a1@+              | write byte
	movel   a1,a0@(66)   | store rbuf current
	clrb    d0                    | kill byte
	movel   d0,a0@(6)         | store bitbuf and done
	rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

| st_bits functie
|void st_bits(unsigned long val, int bit_count, packstruct *com)
| deze functie gaat er van uit dat 0<=bit_count<=16
_st_bits:
st_bits:
	movel   sp@(4),d0
	movel   sp@(8),d1
	movel   sp@(12),a0
	movel   d2,sp@-
	addql   #4,a0    | command struct+bits_in_bitbuf
	addw    a0@,d1               | bit_count+=bits_in_bitbuf
	moveq    #32,d2                | 32
	subw    d1,d2                 | bits_in_bitbuf>32?
	ble    ll_0057        | yep
	movew   d1,a0@+              | store bits_in_bitbuf
	lsll    d2,d0                 | val<<(32-bits_in_bitbuf)
	addl    d0,a0@               | tel bij bitbuf op
ll_0056:
	movel   sp@+,d2
	rts                            | done
ll_0057:
	negw    d2                    | -(32-bits_in_bitbuf) -> bits_in_bitbuf-32
	movew   d2,a0@+              | store bits_in_bitbuf
	rorl    d2,d0                 | val >> command.bits_in_bitbuf
	movel   a0@,d1               | bitbuf+=val>>bits_in_bitbuf
	addw    d0,d1                 | fix_bitbuf
	clrw    d0                    | wis ondeste bits
	lea      a0@(66-6),a1 | &command.rbuf_current
	movel   d0,a0@               | command.bitbuf = val << (32 - bits_in_bitbuf)
	movel   a1@,a0               | rbuf_current
	movel   d1,a0@+              | *((unsigned long *)rbuf_current)++ = bitbuf;
	movel   a0,a1@               | command.rbuf_current=rbuf_current
	movel   sp@+,d2
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

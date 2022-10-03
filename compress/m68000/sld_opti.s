|
| Sliding dictionary for ARJ ST
|
| (c) 1996 Hans 'Mr Ni!' Wessels
|

	.globl find_dictionary
	.globl insert
	.globl insert2
	.globl insertnm
	.globl insert2nm

| XDEF command


| The following exports should be after "%include command.mac"

	.globl _find_dictionary

	.even

	.equ MAXDELTA,7
	.equ MAXD_MATCH,9
	.equ MAXD_MATCH2,9
	.equ HISTSIZE,16


| node struct zit hier alsvolgt in elkaar:
|
| key
| parent
| c_left
| c_right
|

	.text




| Register gebruik:
| D0 = pos.l        |
| D1 =              | pos -> to_do-1
| D2 =              | nnode (=pos), .h=0
| D3 =
| D4 =
| D5 =
| D6 =
| D7 =
|
| A0 = command      | last_pos -> tree
| A1 =
| A2 =
| A3 =
| A4 =
| A5 =
| A6 =
| A7 = SP

|c_codetype find_dictionary(long pos, packstruct *com)
_find_dictionary:
find_dictionary:
	movel   sp@(4),d0           | pos
	movel   sp@(8),a0           | *com
	moveml  d2-d7/a2-a6,sp@-  | save registers
	moveq    #0,d1              | clear d1, d1.h is nu schoon
	lea      a0@(112),a0    | command.last_pos
	movew   d0,d1              | pos
	movel   d1,d2              | pos, d2.h is schoon
	subw    a0@,d1            | pos-command.last_pos
	movew   d0,a0@+           | command.last_pos=pos
	subqw   #1,d1              | to_do dbra compatible
	bne      multi              | er moeten meerdere gedaan worden
| Register gebruik:
| D0 = pos.l
| D1 = 0.l
| D2 = nnode, .h=0
| D3 =
| D4 =
| D5 =              | delpos
| D6 =
| D7 = 
|
| A0 = command.tree | command.delpos
| A1 =              | tmp
| A2 =              | tmp
| A3 =              | command.tree
| A4 =
| A5 =
| A6 =
| A7 = SP
|eerst de node wissen
	movel   a0@+,a3           | command.tree
	moveq    #1,d5              | init delpos
	addw    a0@,d5            | delpos++
	movew   d5,a0@+           | command.del_pos=del_pos
	lsll    #4,d5              | delpos*16
	lea      a3@(0,d5:l),a2      | command.tree[delpos]
	movel   a2@(4),d5           | tp=command.tree[delpos].parent
	beq    ll_0000            | geen parent
	movel   d5,a1              | parent
	cmpl    a1@,a2            | *parent==command.tree[delpos]?
	bne    ll_0000            | nope
	movel   d1,a1@            | *parent=NO_NODE
ll_0000:
| Register gebruik:
| D0 = pos.l
| D1 = 0.l                | delta_hash
| D2 = nnode, .h = 0
| D3 =                    | rle_size -> (nkey)
| D4 =                    | command.tree
| D5 =                    |
| D6 =                    | hash .h=0
| D7 = 
|
| A0 = command.dictionary | command.delta_hash
| A1 =
| A2 =
| A3 = command.tree
| A4 =
| A5 =
| A6 =                    | new_key+1
| A7 = SP
| nodes gedeleted, nu de insert
	addl    a0@+,d0           | +command.dictionary=new_key
	movel   d0,a6              | new_key, blijft hier
	movew   a0@,d1            | delta_hash
	moveb   a0@(110+1-124),d3 | rle_size, staat er nog een rle_open?
	bne      inrle              | yep!
	moveb   a6@+,d3           | delta hash2
	eorb    d3,d1              | delta hash
	movel   d1,d7              | delta hash==0?
	beq      innew_rle          | nope!
	lslw    #8,d1              | shift hash
	moveb   d3,d1              | delta hash
	movew   d1,a0@+           | store delta_hash

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************


| Register gebruik:
| D0 = nkey, later opt match     ; init
| D1 = tmp, d1.h is schoon
| D2 = nnode(=pos)               ; init
| D3 = (nkey)                    ; init
| D4 = nkey+1
| D5 = best_match_pos
| D6 = best_match
| D7 = hash d7.h schoon (init) -> tp 
|
| A0 = command. root (init) ; -> hist_index -> hist -> best_match_pos ; init
| A1 = tp
| A2 = tmp
| A3 = command.tree (init) ; -> left
| A4 = right
| A5 = tmp
| A6 = nkey+1 (init) ; -> later hist        ; init
| A7 = sp


insert:
	lsll    #4,d2              | nnode*16
	movel   a0@+,a1           | command.root
	addl    d2,a3              | tp=command.tree+nnode
	lsll    #2,d7              | hash*4
	movel   a3,a5              | tmp=command.tree+nnode
	movel   d0,a3@+           | tp->key=nkey
	addl    d7,a1              | command.root+command.delta_hash
	movel   a1,a3@+           | tp->parent=command.root+command.delta_hash
	moveq    #0,d0              | opt_match=0
	movel   a1@,d7            | tp=command.root[command.delta_hash]
	beq    ll_0002      | einde insert
	moveq    #1,d4              | clear d4, index++
	lea      a3@(4),a4           | right=&(tp->c_right)
	addw    a0@,d4            | history index
	moveq    #10,d6             | shift size
	andw    #HISTSIZE-1,d4     | modulo
	movew   d4,a0@+           | store
	lslw    d6,d4              | offset in hist array
	addl    a0@+,d4           | plus hist_array
	moveq    #0,d6              | best match=0
	exg      d4,a6              | verwissel nkey en hist pointer
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
ll_0001:
	movel   d7,a2              | tp
	movel   a2@+,a1           | tp->key
	cmpb    a1@+,d3           | key's gelijk? 1
	beq    ll_0004        |
| hier gegarandeerd geen max match
	bcc    ll_0003            | nkey>okey
     | nkey<okey
	movel   d7,a4@            | *right=tp
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d7            | tp=*right
	bne    ll_0001       | next!
	movel   d7,a3@            | *left=0
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	moveml  sp@+,d2-d7/a2-a6
	rts
ll_0002:                   | voor als de tree leeg was
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	movel   d7,a3@+           | *left=0
	movel   d7,a3@            | *right=0
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0003:
     | nkey>okey
	movel   d7,a3@            | *left=tp
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d7            | tp=*left
	bne    ll_0001       | next!
	movel   d7,a4@            | *right=0
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0004:
	movel   d4,a5              | nkey
	cmpb    a1@+,a5@+        | 2
	bne    ll_0005         |
	cmpb    a1@+,a5@+        | 3
	bne    ll_0005         |
	cmpb    a1@+,a5@+        | 4
	bne    ll_0005         |
	cmpb    a1@+,a5@+        | 5
	bne    ll_0005         |
	cmpb    a1@+,a5@+        | 6
	bne    ll_0005         |
	cmpb    a1@+,a5@+        | 7
	bne    ll_0005         |
	cmpb    a1@+,a5@+        | 8
	bne    ll_0005         |
	cmpb    a1@+,a5@+        | 9
	beq      ll_0013
ll_0005:
	bcc    ll_0009              | nkey>okey
     | nkey<okey
	movel   d7,a4@            | *right=tp
	movew   a5,d7              | eind positie
	subw    d4,d7              | d7=(nkey+matchlen+1)-(nkey+1)=matchlen
	cmpw    d6,d7              | matchlen<best_match?
	ble    ll_0008             | yep, geen nieuwe best_match
	movew   a1,d1              | tp->key+matchlen
	movew   d7,d6              | best_match=matchlen
	notw    d1                 |
	movew   d7,a6@+           | store match in hist array
	addw    a5,d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	movew   d1,a6@+           | store pos in hist array
	subw    d0,d7              | matchlen-optmatch
	bgt    ll_0007       | nope
	movel   a0@(0-136),a1 | log tabel
	beq    ll_0006          | matchlen-optmatch==-2
	moveq    #MAXDELTA,d7       | MAXDELTA
	subb    a1@(0,d1:l),d7      | -log(pos)
	addb    a1@(0,d5:l),d7      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	bgt    ll_0007       | yep!
	bra    ll_0008             | anders geen nieuwe optmatch
ll_0006:
	moveq    #2*MAXDELTA,d7     | MAXDELTA
	subb    a1@(0,d1:l),d7      | -log(pos)
	addb    a1@(0,d5:l),d7      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	ble    ll_0008             | anders geen nieuwe optmatch
ll_0007:
	moveq    #2,d0              | offset
	movel   d1,d5              | best_match_pos=pos
	addw    d6,d0              | optmatch=bestmatch
ll_0008:
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d7            | tp=*right
	bne      ll_0001       | next!
	movel   d7,a3@            | *left=0
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0009:
     | nkey>okey
	movel   d7,a3@            | *left=tp
	movew   a5,d7              | eind positie
	subw    d4,d7              | d7=(nkey+matchlen+1)-(nkey+1)=matchlen
	cmpw    d6,d7              | matchlen<best_match?
	ble    ll_0012             | yep, geen nieuwe best_match
	movew   a1,d1              | tp->key+matchlen
	movew   d7,d6              | best_match=matchlen
	notw    d1                 |
	movew   d7,a6@+           | store match in hist array
	addw    a5,d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	movew   d1,a6@+           | store pos in hist array
	subw    d0,d7              | matchlen-optmatch
	bgt    ll_0011       | nope
	movel   a0@(0-136),a1 | log tabel
	beq    ll_0010          | matchlen-optmatch==-2
	moveq    #MAXDELTA,d7       | MAXDELTA
	subb    a1@(0,d1:l),d7      | -log(pos)
	addb    a1@(0,d5:l),d7      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	bgt    ll_0011       | yep!
	bra    ll_0012             | anders geen nieuwe optmatch
ll_0010:
	moveq    #2*MAXDELTA,d7     | MAXDELTA
	subb    a1@(0,d1:l),d7      | -log(pos)
	addb    a1@(0,d5:l),d7      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	ble    ll_0012             | anders geen nieuwe optmatch
ll_0011:
	moveq    #2,d0              | offset
	movel   d1,d5              | best_match_pos=pos
	addw    d6,d0              | optmatch=bestmatch
ll_0012:
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d7            | tp=*left
	bne      ll_0001       | next!
	movel   d7,a4@            | *right=0
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	moveml  sp@+,d2-d7/a2-a6
	rts

| Register gebruik:
| D0 = opt match
| D1 = tmp, d1.h is schoon
| D2 = nnode*16, commandll_0024
| D3 = tmp
| D4 = nkey+MAXD_MATCH
| D5 = best_match_pos
| D6 = best_match-MAXD_MATCH
| D7 = tp
|
| A0 = command.best_match_pos
| A1 = tmp
| A2 = tp
| A3 = command.link
| A4 = command.tree
| A5 = tmp
| A6 = hist
| A7 = sp
ll_0013:                           | max match
	negw    d6
	movel   a6,d4              | save hist
	movel   a1,a2@-           | store key+MAXD_MATCH
	addql   #8,a2              | a2=tp->c_left
	movel   a2@+,d3           | tp->c_left
	beq    ll_0014          | tp->c_left==0
	movel   d3,a6              | tp->c_left
	movel   a3,a6@(4)           | (tp->c_left)->parent=left
ll_0014:
	movel   d3,a3@            | *left=tp->c_left

	movel   a2@,d3            | tp->c_right
	beq    ll_0015         | tp->c_right==0
	movel   d3,a6              | tp->c_right
	movel   a4,a6@(4)           | (tp->c_right)->parent=right
ll_0015:
	movel   d3,a4@            | *right=tp->c_right
|linking
	movel   a0@(102-136),a3 | command.link
	lsrl    #2,d2              | nnode*4
	lea      a3@(0,d2:l),a6      | command.link+nnode
	movel   d7,a6@            | command.link[nnode]=tp
	movel   a6,a2@(-8)          | tp->parent=command.link+nnode
|end_linking
	movew   a0@(14-136),d2 | commandll_0024
	movel   a0@(114-136),a4 | command.tree
	movel   d4,a6              | restore hist
	movel   a5,d4              | nkey+MAXD_MATCH
	addqw   #MAXD_MATCH-1,d6   | corrigeer best_match
	subqw   #MAXD_MATCH-1,d2   | corrigeer max_match
	moveb   a5@(-1,d2:w),d1     | orig
	swap     d2                 | orig opslag
	moveb   d1,d2              | orig
	swap     d2                 | restore max_amtch
	bra    ll_0017           | don't init link loop
ll_0016:
	movel   d7,a2              | tp
	movel   d4,a5              | nkey+MAXD_MATCH
	movel   a2@+,a1           | tp->key
ll_0017:
	moveb   a1@(-1,d2:w),d1     | kar
	movew   d4,d3              | compare waarde
	notb    d1                 | ~kar
	moveb   d1,a5@(-1,d2:w)     | sentinel
ll_0018:
	cmpb    a1@+,a5@+        | 2
	bne    ll_0019        |
	cmpb    a1@+,a5@+        | 3
	bne    ll_0019        |
	cmpb    a1@+,a5@+        | 4
	bne    ll_0019        |
	cmpb    a1@+,a5@+        | 5
	bne    ll_0019        |
	cmpb    a1@+,a5@+        | 6
	bne    ll_0019        |
	cmpb    a1@+,a5@+        | 7
	bne    ll_0019        |
	cmpb    a1@+,a5@+        | 8
	bne    ll_0019        |
	cmpb    a1@+,a5@+        | 9
	beq    ll_0018
ll_0019:
	subw    a5,d3              | d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
	cmpw    d3,d6              | matchlen<best_match?
	ble    ll_0023             | yep, geen nieuwe best_match
	movew   d3,d6              | best_match=matchlen
	moveq    #MAXD_MATCH-1,d3   | corrigeer offset
	movew   a1,d1              | tp->key+matchlen
	subw    d6,d3              | match
	notw    d1                 |
	movew   d3,a6@+           | store match in hist array
	addw    a5,d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	movew   d1,a6@+           | store pos in hist array
	subw    d0,d3              | matchlen-optmatch
	bgt    ll_0021       | nope
	movel   a0@(0-136),a1 | log tabel
	beq    ll_0020          | matchlen-optmatch==-2
	moveq    #MAXDELTA,d3       | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	bgt    ll_0021       | yep!
	bra    ll_0022               | anders geen nieuwe optmatch
ll_0020:
	moveq    #2*MAXDELTA,d3     | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	ble    ll_0022               | anders geen nieuwe optmatch
ll_0021:
	moveq    #MAXD_MATCH-1+2,d0 | corrigeer offset
	subw    d6,d0              | optmatch=bestmatch
	movel   d1,d5              | best_match_pos=pos
ll_0022:
	movew   d6,d3              | restore d3
	addw    d2,d3              | best_match+max_match
	beq    ll_0024         | zijn aan elkaar gelijk? dan einde met max_match
ll_0023:
	subl    a4,d7              | tp-command.tree
	lsrl    #2,d7              | offset in link array
	movel   a3@(0,d7:l),d7      | link[tp-command.tree]
	bne      ll_0016         | verder linken
|end
ll_0024:
	movew   d2,d6              | max_match
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	swap     d2                 | orig
	movel   d4,a6              | restore nkey
	moveb   d2,a6@(-1,d6:w)     | restore orig
	moveml  sp@+,d2-d7/a2-a6
	rts

|ll_0024:
| link de parent van deze node met de link van deze node
|end
|     sub.w    d0,d7              ; return opt_match
|     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
|     move.w   d2,d6              ; max_match
|     swap     d2                 ; orig
|     move.b   d2,-1(a6,d6.w)     ; restore orig
|if GNUC
|     movem.l  (sp)+,d2-d7/a2-a6
|else
|     movem.l  (sp)+,d3-d7/a2-a6
|%endif
|     rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

| Register gebruik:
| D0 = nkey                 |*rle_hash
| D1 = 0.l                  |*rle_size.w, d1.h = 0
| D2 = nnode, .h = 0        |*nnode, .h = 0
| D3 = (nkey)               |
| D4 =
| D5 =
| D6 =
| D7 = 0.l
|
| A0 = command.delta_hash   |*command.delta_hash
| A1 =                      |
| A2 =                      |
| A3 =                      |
| A4 =                      |
| A5 =                      |
| A6 = new_key+1            |*nkey
| A7 = SP                   |
innew_rle:
	moveb   d3,a0@(124+1-124)| store delta_hash
	movel   d0,a1              | key
	movel   d0,a6              | key
	movew   a0@(14-124),d5 | max_match
	moveq    #0,d0              | clear d0
	moveb   a6@(0,d5:w),d1      | orig
	moveb   d3,d0              | rle_char
	notb    d3                 | ~rle_char
	moveb   d3,a6@(0,d5:w)      | sentry
ll_0025:
	cmpb    a1@+,d0           | compare
	bne    ll_0026           |
	cmpb    a1@+,d0           | compare
	bne    ll_0026           |
	cmpb    a1@+,d0           | compare
	bne    ll_0026           |
	cmpb    a1@+,d0           | compare
	beq    ll_0025          |
ll_0026:
	moveb   d1,a6@(0,d5:w)      | restore orig
	movew   a6,d1              | key
	notw    d1                 | -key-1
	addw    a1,d1              | rle_size=p-key-1
	lslw    #8,d0              | rle_char<<8
	moveb   d1,d0              | rle_hash
	movel   d0,d5              | rle_hash
	subqw   #1,d5              | rle_size--
	movew   d5,a0@(110-124) | store rle_hash
	bra    insert2            | insert

| Register gebruik:
| D0 = nkey                 |*rle_hash
| D1 = delta_hash           |*rle_size.w, d1.h = 0
| D2 = nnode, .h = 0        |*nnode, .h = 0
| D3 = rle_size.b           |
| D4 =
| D5 =
| D6 =
| D7 =
|
| A0 = command.delta_hash   |*command.delta_hash
| A1 =                      |
| A2 =                      |
| A3 =                      |
| A4 =                      |
| A5 =                      |
| A6 = new_key              |*nkey
| A7 = SP                   |
insert2_ai:
	addqw   #1,d0              | rle_size++
	addqw   #1,d1              | rle_size++
	bra    insert2            | insert
inrle:
	moveq    #0,d0              | clear d0
	movew   a0@(110-124),d0 | rle_hash
	movew   d1,d4              | rle_char
	moveb   d0,d1              | rle_size.w
	cmpb    a6@(0,d1:w),d4      | rle_size==rle_max?
	beq    insert2_ai         | yep
	subqw   #1,d3              | rle_size.w--
	moveb   d3,a0@(110+1-124) | store rle_hash

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************


| Register gebruik:
| D0 = rle_hash                  |
| D1 = rle_size, d1.h=0          |
| D2 = nnode                     | nnode*16 -> sp            
| D3 =                           | tmp
| D4 =                           | .h: orig, .l: command.max_match
| D5 =                           | best_match_pos
| D6 =                           | .h: rle_size, .l: best_match
| D7 =                           | -opt_match
|
| A0 = command.delta_hash        | -> hist_index -> hist -> best_match_pos
| A1 =                           | tp
| A2 =                           | tmp
| A3 = command.tree              | left
| A4 =                           | right
| A5 =                           | tmp
| A6 = nkey                      | nkey
| A7 =                           | hist


insert2:
	lsll    #4,d2              | nnode*16
	movel   a6,a2              | nkey
	movel   a0@(138-124),a1 | command.root2
	movel   d1,d6              | rle_size, d6.h = 0
	addl    d2,a3              | tp=command.tree+nnode
	lsll    #2,d0              | hash*4
	addw    d1,a6              | nkey+rle_size
	movel   a3,a5              | tmp=command.tree+nnode
	movel   a6,a3@+           | tp->key=nkey
	addl    d0,a1              | command.root+command.delta_hash
	movel   a1,a3@+           | tp->parent=command.root+command.delta_hash
	movel   a1@,d0            | tp=command.root[command.delta_hash]
	beq      ll_0030     | einde insert
	lea      a3@(4),a4           | right=&(tp->c_right)
	movel   d2,sp@(-4)          | store nnode * 16
	addql   #6,a0              | a0 = command.hist_index
	moveq    #1,d2              | clear d2, index++
	addw    a0@,d2            | history index
	moveq    #10,d5             | shift size
	andw    #HISTSIZE-1,d2     | modulo
	movew   d2,a0@+           | store
	lslw    d5,d2              | offset in hist array
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	addl    a0@+,d2           | hist array
	moveb   a2@,d7            | rle_char
	moveq    #0,d5              | best_match_pos=0
	cmpb    a2@(-3),d7          | nkey[-3]
	exg      d2,sp              | hist array, need adress register
	beq    ll_0027               | echte rle match gevonden
	movel   d0,a2              | tp
	movew   a2@(2),d5           | tp->key
	notw    d5                 |
	addw    a6,d5              | pos=nkey-(tp->key)-1
ll_0027:
	movew   a0@(14-136),d4 | commandll_0053
	movew   d6,sp@+           | store match in hist array
	moveq    #0,d7              | -opt_match=2
	movew   d5,sp@+           | store best_max_pos in hist array
	subw    d6,d4              | commandll_0053-rle_size
	beq      ll_0029        | meteen al een max_match gevonden
	swap     d6                 | d6.h=rle_size
	moveb   a6@(0,d4:w),d1      | orig
	swap     d4                 | orig opslag
	moveb   d1,d4              | orig
	swap     d4                 | max_match
ll_0028:
	movel   d0,a2              | tp
	movel   a6,a5              | nkey
	movel   a2@+,a1           | tp->key
	cmpb    a1@+,a5@+        | key's gelijk? 1
	beq      ll_0033        |
| hier gegarandeerd geen max match
	bcc      ll_0032            | nkey>okey
     | nkey<okey
	movel   d0,a4@            | *right=tp
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d0            | tp=*right
	bne    ll_0028       | next!
	movel   d0,a3@            | *left=0
	movew   d4,d3              | max_match
	swap     d6                 | rle_size
	swap     d4                 | orig
	subw    d7,d6              | return opt_match
	moveb   d4,a6@(0,d3:w)      | restore orig
|ll_0029:
	movew   d6,d0              | +rle_size
	movel   d2,sp              | remove hist array
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	addqw   #2,d0              | match offset
	moveml  sp@+,d2-d7/a2-a6
	rts
ll_0029:
	movel   d7,a3@            | geen linker node
	movel   d7,a4@            | geen rechter node
	movew   d6,d0              | +rle_size
	movel   d2,sp              | remove hist array
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	addqw   #2,d0              | match offset
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0030:                  | voor als de tree leeg was
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	movel   d0,a3@+           | *left=0
	movel   d0,a3@            | *right=0
	moveb   a2@,d7            | rle_char
	cmpb    a2@(-3),d7          | nkey[-3]
	bne    ll_0031            | geen match gevonden
	movew   d0,a0@(136-124) | command.best_match_pos=best_match_pos
	moveq    #2,d0              | match offset
	addw    d6,d0              | opt_match
ll_0031:
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0032:
     | nkey>okey
	movel   d0,a3@            | *left=tp
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d0            | tp=*left
	bne      ll_0028       | next!
	movel   d0,a4@            | *right=0
	swap     d6                 | rle_size
	movew   d6,d0              | +rle_size
	subw    d7,d0              | return opt_match
	addqw   #2,d0              | match offset
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	movew   d4,d3              | max_match
	movel   d2,sp              | remove hist array
	swap     d4                 | orig
	moveb   d4,a6@(0,d3:w)      | restore orig
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0033:
	moveb   a1@(-1,d4:w),d1     | kar
	movew   a5,d3              | nkey+1
	notb    d1                 | ~kar
	moveb   d1,a5@(-1,d4:w)     | sentinel
	cmpb    a1@+,a5@+        | 2
	bne    ll_0034         |
	cmpb    a1@+,a5@+        | 3
	bne    ll_0034         |
	cmpb    a1@+,a5@+        | 4
	bne    ll_0034         |
	cmpb    a1@+,a5@+        | 5
	bne    ll_0034         |
	cmpb    a1@+,a5@+        | 6
	bne    ll_0034         |
	cmpb    a1@+,a5@+        | 7
	bne    ll_0034         |
	cmpb    a1@+,a5@+        | 8
	bne    ll_0034         |
	cmpb    a1@+,a5@+        | 9
	beq      ll_0042
ll_0034:
	bcc    ll_0038              | nkey>okey
     | nkey<okey
	subw    a5,d3              | d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
	cmpw    d3,d6              | matchlen<best_match?
	ble    ll_0037             | yep, geen nieuwe best_match
	movew   d3,d6              | best_match=matchlen
	movew   a1,d1              | tp->key+matchlen
	swap     d6                 | rle_size
	notw    d1                 |
	subw    d3,d6              | match-2
	addw    a5,d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	movew   d6,sp@+           | store match in hist array
	addw    d3,d6              | herstel d6
	movew   d1,sp@+           | store pos in hist array
	swap     d6                 | d6 weer omdraaien
	subw    d7,d3              | matchlen-optmatch
	addqw   #2,d3              | matchlen-optmatch>=-2?
	bmi    ll_0036       | nope
	movel   a0@(0-136),a1 | log tabel
	beq    ll_0035          | matchlen-optmatch==-2
	moveq    #MAXDELTA,d3       | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	bgt    ll_0036       | yep!
	bra    ll_0037             | anders geen nieuwe optmatch
ll_0035:
	moveq    #2*MAXDELTA,d3     | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	ble    ll_0037             | anders geen nieuwe optmatch
ll_0036:
	movew   d6,d7              | optmatch=bestmatch
	movel   d1,d5              | best_match_pos=pos
ll_0037:
	movel   d0,a4@            | *right=tp
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d0            | tp=*right
	bne      ll_0028       | next!
	movel   d0,a3@            | *left=0
	swap     d6                 | rle_size
	movew   d6,d0              | +rle_size
	subw    d7,d0              | return opt_match
	addqw   #2,d0              | match offset
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	movew   d4,d3              | max_match
	movel   d2,sp              | remove hist array
	swap     d4                 | orig
	moveb   d4,a6@(0,d3:w)      | restore orig
	moveml  sp@+,d2-d7/a2-a6
	rts
ll_0038:
     | nkey>okey
	subw    a5,d3              | d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
	cmpw    d3,d6              | matchlen<best_match?
	ble    ll_0041             | yep, geen nieuwe best_match
	movew   d3,d6              | best_match=matchlen
	movew   a1,d1              | tp->key+matchlen
	swap     d6                 | rle_size
	notw    d1                 |
	subw    d3,d6              | match-2
	addw    a5,d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	movew   d6,sp@+           | store match in hist array
	addw    d3,d6              | herstel d6
	movew   d1,sp@+           | store pos in hist array
	swap     d6                 | d6 weer omdraaien
	subw    d7,d3              | matchlen-optmatch
	addqw   #2,d3              | matchlen-optmatch>=-2?
	bmi    ll_0040       | nope
	movel   a0@(0-136),a1 | log tabel
	beq    ll_0039          | matchlen-optmatch==-2
	moveq    #MAXDELTA,d3       | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	bgt    ll_0040       | yep!
	bra    ll_0041             | anders geen nieuwe optmatch
ll_0039:
	moveq    #2*MAXDELTA,d3     | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	ble    ll_0041             | anders geen nieuwe optmatch
ll_0040:
	movew   d6,d7              | optmatch=bestmatch
	movel   d1,d5              | best_match_pos=pos
ll_0041:
	movel   d0,a3@            | *left=tp
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d0            | tp=*left
	bne      ll_0028       | next!
	movel   d0,a4@            | *right=0
	swap     d6                 | rle_size
	movew   d6,d0              | +rle_size
	subw    d7,d0              | return opt_match
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	addqw   #2,d0              | match offset
	movew   d4,d3              | max_match
	movel   d2,sp              | remove hist array
	swap     d4                 | orig
	moveb   d4,a6@(0,d3:w)      | restore orig
	moveml  sp@+,d2-d7/a2-a6
	rts

| Register gebruik:
| D0 = tp
| D1 = tmp, d1.h is schoon
| D2 = sp
| D3 = tmp
| D4 = commandll_0053
| D5 = best_match_pos
| D6 = .h: rle_size, .l: best_match-MAXD_MATCH2
| D7 = opt_match-MAXD_MATCH2
|
| A0 = command.best_match_pos
| A1 = tp
| A2 = tmp
| A3 = command.link
| A4 = command.tree
| A5 = tmp
| A6 = nkey+MAXD_MATCH2
| A7 = hist

ll_0042:                           | max match
	movel   a1,a2@-           | strore key+MAXD_MATCH2
	addql   #8,a2              | a2=tp->c_left
	movel   a2@+,d3           | tp->c_left
	beq    ll_0043          | tp->c_left==0
	movel   d3,a6              | tp->c_left
	movel   a3,a6@(4)           | (tp->c_left)->parent=left
ll_0043:
	movel   d3,a3@            | *left=tp->c_left

	movel   a2@,d3            | tp->c_right
	beq    ll_0044         | tp->c_right==0
	movel   d3,a6              | tp->c_right
	movel   a4,a6@(4)           | (tp->c_right)->parent=right
ll_0044:
	movel   d2,a3              | sp
	movel   d3,a4@            | *right=tp->c_right
|linking
	movel   a3@-,d3           | nnode*16
	movel   a0@(102-136),a3 | command.link
	lsrl    #2,d3              | nnode*4
	lea      a3@(0,d3:l),a6      | command.link+nnode
	movel   d0,a6@            | command.link[nnode]=tp
	movel   a6,a2@(-8)          | tp->parent=command.link+nnode
|end_linking
	movel   a0@(114-136),a4 | command.tree
	movel   a5,a6              | nkey+MAXD_MATCH2
	addqw   #MAXD_MATCH2-1,d6  | corrigeer best_match
	addqw   #MAXD_MATCH2-1,d7  | corrigeer opt_match
	swap     d6                 |
	subqw   #MAXD_MATCH2-1,d4  | corrigeer max_match
	addqw   #MAXD_MATCH2-1,d6  | corrigeer rle_size
	swap     d6
	bra    ll_0046           | don't init link loop
ll_0045:
	movel   d0,a2              | tp
	movel   a6,a5              | nkey+MAXD_MATCH2
	movel   a2@+,a1           | tp->key
ll_0046:
	moveb   a1@(-1,d4:w),d1     | kar
	movew   a6,d3              | compare waarde
	notb    d1                 | ~kar
	moveb   d1,a5@(-1,d4:w)     | sentinel
ll_0047:
	cmpb    a1@+,a5@+        | 2
	bne    ll_0048        |
	cmpb    a1@+,a5@+        | 3
	bne    ll_0048        |
	cmpb    a1@+,a5@+        | 4
	bne    ll_0048        |
	cmpb    a1@+,a5@+        | 5
	bne    ll_0048        |
	cmpb    a1@+,a5@+        | 6
	bne    ll_0048        |
	cmpb    a1@+,a5@+        | 7
	bne    ll_0048        |
	cmpb    a1@+,a5@+        | 8
	bne    ll_0048        |
	cmpb    a1@+,a5@+        | 9
	beq    ll_0047
ll_0048:
	subw    a5,d3              | d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
	cmpw    d3,d6              | matchlen<best_match?
	ble    ll_0052             | yep, geen nieuwe best_match
	movew   d3,d6              | best_match=matchlen
	movew   a1,d1              | tp->key+matchlen
	swap     d6                 | rle_size
	notw    d1                 |
	subw    d3,d6              | match-2
	addw    a5,d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	movew   d6,sp@+           | store match in hist array
	addw    d3,d6              | herstel d6
	movew   d1,sp@+           | store pos in hist array
	swap     d6                 | d6 weer omdraaien
	subw    d7,d3              | matchlen-optmatch
	addqw   #2,d3              | matchlen-optmatch>=-2?
	bmi    ll_0050       | nope
	movel   a0@(0-136),a1 | log tabel
	beq    ll_0049          | matchlen-optmatch==-2
	moveq    #MAXDELTA,d3       | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	bgt    ll_0050       | yep!
	bra    ll_0051               | anders geen nieuwe optmatch
ll_0049:
	moveq    #2*MAXDELTA,d3     | MAXDELTA
	subb    a1@(0,d1:l),d3      | -log(pos)
	addb    a1@(0,d5:l),d3      | MAXDELTA+log(best_match_pos)-log(pos)>0?
	ble    ll_0051               | anders geen nieuwe optmatch
ll_0050:
	movew   d6,d7              | optmatch=bestmatch
	movel   d1,d5              | best_match_pos=pos
ll_0051:
	movew   d6,d3              | restore d3
	addw    d4,d3              | best_match+max_match
	beq    ll_0053         | zijn aan elkaar gelijk? dan einde met max_match
ll_0052:
	subl    a4,d0              | tp-command.tree
	lsrl    #2,d0              | offset in link array
	movel   a3@(0,d0:l),d0      | link[tp-command.tree]
	bne      ll_0045         | verder linken
|end
	moveq    #2,d0              | corrigeer d7
	swap     d6                 | rle_size
	addw    d6,d0              | +rle_size
	subw    d7,d0              | return opt_match
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	movew   d4,d3              | max_match
	movel   d2,sp              | remove hist array
	swap     d4                 | orig
	moveb   d4,a6@(-1,d3:w)     | restore orig
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0053:
| link de parent van deze node met de link van deze node
|end
	moveq    #2,d0              | corrigeer d7
	swap     d6                 | rle_size
	addw    d6,d0              | +rle_size
	subw    d7,d0              | return opt_match
	movew   d5,a0@            | command.best_match_pos=best_match_pos
	movew   d4,d3              | max_match
	swap     d4                 | orig
	movel   d2,sp              | remove hist array
	moveb   d4,a6@(-1,d3:w)     | restore orig
	moveml  sp@+,d2-d7/a2-a6
	rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

| Register gebruik:
| D0 = pos.l                     (ini)
| D1 = d1.h is schoon            (ini)
| D2 = d2.h is schoon            (ini)
| D3 = 0
| D4 =
| D5 = 
| D6 = delpos
| D7 = to_do
|
| A0 = command.tree, moving
| A1 = tree
| A2 =
| A3 =
| A4 =
| A5 =
| A6 =
| A7 = SP

multi:
	movel   d1,d7              | to_do
|eerst de nodes wissen
	movel   a0@+,a1           | command.tree
	moveq    #0,d3              | NO_NODE
	movew   a0@,d2            | delpos
ll_0054:
	addqw   #1,d2              | delpos++
	movel   d2,d4              | delpos
	lsll    #4,d4              | delpos*16
	lea      a1@(0,d4:l),a2      | command.tree[delpos]
	movel   a2@(4),d4           | tp=command.tree[delpos].parent
	beq    ll_0055            | geen parent
	movel   d4,a3              | parent
	cmpl    a3@,a2            | *parent==command.tree[delpos]?
	bne    ll_0055            | nope
	movel   d3,a3@            | *parent=NO_NODE
ll_0055:
	dbra     d1,ll_0054       | delete next
	movew   d2,a0@+           | command.del_pos=del_pos
| Register gebruik:
| D0 = gebruikt door insertnm
| D1 = pos, d2.h is schoon
| D2 = pos, boven schoon
| D3 = hash, boven schoon
| D4 = command.tree
| D5 = 
| D6 = gebruikt door insertnm
| D7 = to_do
|
| A0 = command.dictionary -> command.delta_hash, blijft behouden
| A1 = gebruikt door insertnm
| A2 = gebruikt door insertnm
| A3 = gebruikt door insertnm
| A4 = gebruikt door insertnm
| A5 = gebruikt door insertnm
| A6 = nkey, blijft behouden
| A7 = SP
| nodes gedeleted, nu de insert no match
	movel   a1,d4              | d4=command.tree
	subl    d7,d0              | pos-=to_do
	movew   d0,d2              | pos, bovenste helft is schoon
	addl    a0@+,d0           | new_key
	movel   d0,a6              | new_key, blijft hier
	subqw   #1,d7              | d7 dbra compatible
	tstb    a0@(110+1-124)| staat er nog een rle open?
	bne      rle                | yep!
	movew   a0@,d3            | delta_hash

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

| Register gebruik:
| D0 = hash, later tp            ; init
| D1 = tmp, d1.h is schoon
| D2 = pos, boven schoon
| D3 =
| D4 = command.tree
| D5 =
| D6 = nnode*16                  ; init
| D7 = to_do
|
| A0 = command.delta_hash        ; init
| A1 = tp
| A2 = tmp
| A3 = left
| A4 = right
| A5 = tmp
| A6 = nkey                      ; init
| A7 = SP

insertnm_loop:
	moveb   a6@,d1            | delta hash2
	eorb    d1,d3              | delta hash
	movel   d3,d0              | hash==0?
	beq      new_rle            | yep!
	lslw    #8,d3              | shift hash
	movel   d2,d6              | nnode
	moveb   d1,d3              | delta hash 2

insertnm:
	lsll    #4,d6              | nnode*16
	movel   d4,a3              | command.tree
	addl    d6,a3              | tp=command.tree+nnode
	lsll    #2,d0              | hash*4
	movel   a3,a5              | tmp=command.tree+nnode
	movel   a0@(126-124),a1 | command.root
	movel   a6,a3@+           | tp->key=nkey
	addl    d0,a1              | command.root+command.delta_hash
	movel   a1,a3@+           | tp->parent=command.root+command.delta_hash
	movel   a1@,d0            | tp=command.root[command.delta_hash]
	beq    ll_0058   | einde insert
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	lea      a3@(4),a4           | right=&(tp->c_right)
ll_0056:
	movel   d0,a2              | tp
	movel   a6,a5              | nkey
	movel   a2@+,a1           | tp->key
	cmpb    a1@+,a5@+        | key's gelijk? 1
	beq    ll_0060        |
ll_0057:
	bcc    ll_0059              | nkey>okey
     | nkey<okey
	movel   d0,a4@            | *right=tp
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d0            | tp=*right
	bne    ll_0056    | next!
	movel   d0,a3@            | *left=0
	addql   #1,a6              | key++
	addqw   #1,d2              | pos++
	dbra     d7,insertnm_loop   | next
	bra    insert_nm_end      | insert
ll_0058:                | voor als de tree leeg was
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	movel   d0,a3@+           | *left=0
	movel   d0,a3@            | *right=0
	addql   #1,a6              | key++
	addqw   #1,d2              | pos++
	dbra     d7,insertnm_loop   | next
	bra    insert_nm_end      | insert
ll_0059:
     | nkey>okey
	movel   d0,a3@            | *left=tp
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d0            | tp=*left
	bne    ll_0056    | next!
	movel   d0,a4@            | *right=0
	addql   #1,a6              | key++
	addqw   #1,d2              | pos++
	dbra     d7,insertnm_loop   | next
	bra    insert_nm_end      | insert

ll_0060:
	cmpb    a1@+,a5@+        | 2
	bne    ll_0057         |
	cmpb    a1@+,a5@+        | 3
	bne    ll_0057         |
	cmpb    a1@+,a5@+        | 4
	bne    ll_0057         |
	cmpb    a1@+,a5@+        | 5
	bne    ll_0057         |
	cmpb    a1@+,a5@+        | 6
	bne    ll_0057         |
	cmpb    a1@+,a5@+        | 7
	bne    ll_0057         |
	cmpb    a1@+,a5@+        | 8
	bne    ll_0057         |
	cmpb    a1@+,a5@+        | 9
	bne    ll_0057
ll_0061:                           | max match
	movel   a1,a2@-           | store key-MAXD_MATCH
	addql   #8,a2              | a2=tp->c_left
	movel   a2@+,d1           | tp->c_left
	beq    ll_0062          | tp->c_left==0
	movel   d1,a5              | tp->c_left
	movel   a3,a5@(4)           | (tp->c_left)->parent=left
ll_0062:
	movel   d1,a3@            | *left=tp->c_left
	movel   a2@,d1            | tp->c_right
	beq    ll_0063         | tp->c_right==0
	movel   d1,a5              | tp->c_right
	movel   a4,a5@(4)           | (tp->c_right)->parent=right
ll_0063:
	movel   d1,a4@            | *right=tp->c_right
|linking
	movel   a0@(102-124),a5 | command.link
	lsrl    #2,d6              | nnode*4
	addl    d6,a5              | command.link+nnode
	movel   d0,a5@            | command.link[nnode]=tp
	movel   a5,a2@(-8)          | tp->parent=command.link+nnode
|end_linking
	addql   #1,a6              | key++
	addqw   #1,d2              | pos++
	dbra     d7,insertnm_loop   | next
|     bra      insert_nm_end      ; insert

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

insert_nm_end:
	movel   a6,d0              | nkey
	moveb   a6@+,d6           | delta hash2
	eorb    d6,d3              | delta hash
	movel   d3,d7              | delta hash==0?
	beq    ll_0064           | nope!
	lslw    #8,d3              | shift hash
	moveq    #0,d1              | clear d1.h nodig bij insert
	moveb   d6,d3              | delta hash2
	movew   d3,a0@+           | store delta_hash
	movel   d4,a3              | command.tree
	bra      insert             | insert

ll_0064:
	subql   #1,a6              | restore a6
	moveb   d6,d3              | delta hash2
	movew   d3,a0@            | store delta_hash
	movel   a6,a1              | key
	movew   a0@(14-124),d0   | max_match
	moveb   a6@(0,d0:w),d1      | orig
	movel   d3,d5              | rle_char
	notb    d6                 | ~rle_char
	moveb   d6,a6@(0,d0:w)      | sentry
ll_0065:
	cmpb    a1@+,d5           | compare
	bne    ll_0066           |
	cmpb    a1@+,d5           | compare
	bne    ll_0066           |
	cmpb    a1@+,d5           | compare
	bne    ll_0066           |
	cmpb    a1@+,d5           | compare
	beq    ll_0065          |
ll_0066:
	moveb   d1,a6@(0,d0:w)      | restore orig
	movew   a6,d1              | key
	notw    d1                 | -key-1
	addw    a1,d1              | rle_size=p-key-1
	lslw    #8,d5              | rle_char<<8
	moveb   d1,d5              | rle_hash
	movel   d5,d0              | hash
	moveq    #0,d1              | clear d1
	moveb   d5,d1              | size
	subqw   #1,d5              | rle_size--
	movew   d5,a0@(110-124) | store rle_hash
	movel   d4,a3              | command.tree
	bra      insert2            | insert

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

new_rle:
	moveq    #0,d5              | clear d5
	movel   a6,a1              | key
	moveb   d1,d5              | rle_char
	movew   a0@(14-124),d0 | max_match
	moveb   a6@(0,d0:w),d6      | orig
	notb    d1                 | ~rle_char
	moveb   d1,a6@(0,d0:w)      | sentry
ll_0067:
	cmpb    a1@+,d5           | compare
	bne    ll_0068           |
	cmpb    a1@+,d5           | compare
	bne    ll_0068           |
	cmpb    a1@+,d5           | compare
	bne    ll_0068           |
	cmpb    a1@+,d5           | compare
	beq    ll_0067          |
ll_0068:
	moveb   d6,a6@(0,d0:w)      | restore orig
	movew   a6,d1              | key
	notw    d1                 | -key-1
	addw    a1,d1              | rle_size=p-key-1
	lslw    #8,d5              | rle_char<<8
	moveb   d1,d5              | rle_hash
	movel   d5,d0              | hash
	movel   d2,d6              | nnode
	moveb   d5,d3              | rle_size
	addl    a6,d3              | key+rle_size
	exg      d3,a6              | swap regs
	bra    insert2nm          | insert

rlemaxmatch:
	addql   #1,a6              | rle_size++
	addqw   #1,d0              | rle_size++
	lsll    #4,d6              | nnode*16
	addl    d4,d6              | tp=command.tree+nnode
	movel   d6,a3              | tp=command.tree+nnode
	movel   a6,a3@+           | tp->key=nkey
	movel   a0@(138-124),a1 | command.root
	lsll    #2,d0              | hash*4
	addl    d0,a1              | command.root+command.delta_hash
	movel   d6,a1@            | command.root[command.delta_hash]=tp
	moveq    #0,d0              | clear d0
	movel   a1,a3@+           | tp->parent=command.root+command.delta_hash
	movel   d0,a3@+           | *left=0
	movel   d0,a3@            | *right=0
	addqw   #1,d2              | pos++
	dbra     d7,f_rle           | next
|     move.b   d5,d0              ; rle_size
|     neg.w    d0                 ; -rle_size
|     lea      0(a6,d0.w),a6      ; restore a6
|     moveq    #0,d3              ; clear d3
	bra       insert2_nm_cont_end|

rle:
	moveq    #0,d5              | clear d5
	movew   a0@(110-124),d5| rle_hash
	moveb   d3,a0@(110+1-124)| ga er van uit dat de rle wordt opgelost
	moveb   d5,d3              | rle_size
	addl    a6,d3              | key+rle_size
	exg      d3,a6              | swap regs
f_rle:
	movel   d5,d0              | hash
	movel   d2,d6              | nnode
	movel   d3,a1              | key
	moveb   a6@,d1            | rle_char
	cmpb    a1@,d1            | rle_size==rle_max?
	beq    rlemaxmatch        | yep

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

| Register gebruik:
| D0 = hash                      ; init
| D1 = tmp, d1.h is schoon
| D2 = 
| D3 =
| D4 = 
| D5 =
| D6 = nnode*16                  ; init
| D7 =
|
| A0 = command.delta_hash        ; init
| A1 = tp
| A2 = tmp
| A3 = left
| A4 = right
| A5 = tmp
| A6 = nkey                      ; init
| A7 = SP


insert2nm:
	lsll    #4,d6              | nnode*16
	movel   d4,a3              | command.tree
	addl    d6,a3              | tp=command.tree+nnode
	movel   a3,a5              | tmp=command.tree+nnode
	movel   a6,a3@+           | tp->key=nkey
	movel   a0@(138-124),a1 | command.root
	addl    d0,d0              | hash*2
	addl    d0,d0              | hash*4
	addl    d0,a1              | command.root+command.delta_hash
	movel   a1,a3@+           | tp->parent=command.root+command.delta_hash
	movel   a1@,d0            | tp=command.root[command.delta_hash]
	beq    ll_0072  | einde insert
	lea      a3@(4),a4           | right=&(tp->c_right)
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
ll_0069:
	movel   d0,a2              | tp
	movel   a6,a5              | nkey
	movel   a2@+,a1           | tp->key
	cmpb    a1@+,a5@+        | key's gelijk? 1
	beq    ll_0074        |
ll_0070:
	bcc    ll_0073              | nkey>okey
     | nkey<okey
	movel   d0,a4@            | *right=tp
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d0            | tp=*right
	bne    ll_0069   | next!
ll_0071:
	movel   d0,a3@            | *right=0
	addqw   #1,d2              | pos++
	subqb   #1,d5              | rle_hash--
	dbeq     d7,f_rle           | next
	bne      insert2_nm_cont_end
	moveq    #0,d3              | clear d3
	moveb   a6@(-1),d3          | vul d3 met rle char
	dbra     d7,insertnm_loop   | next
	bra      insert_nm_end      | insert
ll_0072:               | voor als de tree leeg was
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	movel   d0,a3@+           | *left=0
	bra    ll_0071
ll_0073:
     | nkey>okey
	movel   d0,a3@            | *left=tp
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d0            | tp=*left
	bne    ll_0069   | next!
	movel   d0,a4@            | *right=0
	addqw   #1,d2              | pos++
	subqb   #1,d5              | rle_hash--
	dbeq     d7,f_rle           | next
	bne    insert2_nm_cont_end
	moveq    #0,d3              | clear d3
	moveb   a6@(-1),d3          | vul d3 met rle char
	dbra     d7,insertnm_loop   | next
	bra      insert_nm_end      | insert

ll_0074:
	cmpb    a1@+,a5@+        | 2
	bne    ll_0070         |
	cmpb    a1@+,a5@+        | 3
	bne    ll_0070         |
	cmpb    a1@+,a5@+        | 4
	bne    ll_0070         |
	cmpb    a1@+,a5@+        | 5
	bne    ll_0070         |
	cmpb    a1@+,a5@+        | 6
	bne    ll_0070         |
	cmpb    a1@+,a5@+        | 7
	bne    ll_0070         |
	cmpb    a1@+,a5@+        | 8
	bne    ll_0070         |
	cmpb    a1@+,a5@+        | 9
	bne    ll_0070
ll_0075:                           | max match
	movel   a1,a2@-           | store key-MAXD_MATCH
	addql   #8,a2              | a2=tp->c_left
	movel   a2@+,d1           | tp->c_left
	beq    ll_0076          | tp->c_left==0
	movel   d1,a5              | tp->c_left
	movel   a3,a5@(4)           | (tp->c_left)->parent=left
ll_0076:
	movel   d1,a3@            | *left=tp->c_left
	movel   a2@,d1            | tp->c_right
	beq    ll_0077         | tp->c_right==0
	movel   d1,a5              | tp->c_right
	movel   a4,a5@(4)           | (tp->c_right)->parent=right
ll_0077:
	movel   d1,a4@            | *right=tp->c_right
|linking
	movel   a0@(102-124),a5 | command.link
	lsrl    #2,d6              | nnode*4
	addl    d6,a5              | command.link+nnode
	movel   d0,a5@            | command.link[nnode]=tp
	movel   a5,a2@(-8)          | tp->parent=command.link+nnode
|end_linking
	addqw   #1,d2              | pos++
	subqb   #1,d5              | rle_hash--
	dbeq     d7,f_rle           | next
	bne    insert2_nm_cont_end
	moveq    #0,d3              | clear d3
	moveb   a6@(-1),d3          | vul d3 met rle char
	dbra     d7,insertnm_loop   | next
	bra      insert_nm_end      | insert

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

insert2_nm_cont_end:
	movel   a6,a1              | key+rle_size
	moveq    #0,d1              | clear d1
	movel   d5,d0              | hash
	moveb   d5,d1              | rle_size
	moveq    #0,d3              | clear d3
	subw    d1,a6              | key
	subqw   #1,d5              | rle_size--
	moveb   a6@,d3            | rle_char
	movew   d5,a0@(110-124) | store rle_hash
	movew   d3,a0@            | store delta_hash
	movel   d4,a3              | command.tree
	cmpb    a1@,d3            | rle_size==rle_max?
	bne      insert2            | nope
	addqw   #1,d5              | rle_size++
	addqw   #1,d1              | rle_size++
	addqw   #1,d0
	movew   d5,a0@(110-124) | store rle_hash
	bra      insert2            | insert


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

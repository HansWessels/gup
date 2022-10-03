|
| Sliding dictionary for ARJ ST
|
| (c) 1996 Hans 'Mr Ni!' Wessels
|

	.globl find_dictionary_fast
	.globl insert_fast
	.globl insert2_fast
	.globl insertnm_fast
	.globl insert2nm_fast

| XDEF command


| The following exports should be after "%include command.mac"

	.globl _find_dictionary_fast

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
| D0 = gebruikt door insertnm
| D1 = gebruikt door insertnm, d1.h is schoon
| D2 = gebruikt door insertnm
| D3 = nnode
| D4 = rle_hash
| D5 = delta_hash
| D6 = 
| D7 = 
|
| A0 = command
| A1 = gebruikt door insertnm
| A2 = gebruikt door insertnm
| A3 = gebruikt door insertnm
| A4 = gebruikt door insertnm
| A5 = gebruikt door insertnm
| A6 = gebruikt door insertnm
| A7 = SP
|c_codetype find_dictionary(long pos, packstruct *com)
_find_dictionary_fast:
find_dictionary_fast:
	move.l   4(%sp),%d0           | pos
	move.l   8(%sp),%a0           | *com
	movem.l  %d2-%d7/%a2-%a6,-(%sp)  | save registers
	moveq    #0,%d1              | clear d1, d1.h is nu schoon
	move.w   %d0,%d1              | pos
	sub.w    112(%a0),%d1    | pos-command.last_pos
	subq.w   #1,%d1              | to_do dbra compatible
	bne      multi              | er moeten meerdere gedaan worden
	move.w   %d0,112(%a0)    | command.last_pos=pos
|eerst de node wissen
	moveq    #1,%d5              | init delpos
	move.l   114(%a0),%a3        | command.tree
	moveq    #0,%d2              | clear d2
	add.w    118(%a0),%d5     | delpos++
	move.w   %d5,118(%a0)     | command.del_pos=del_pos
	lsl.l    #4,%d5              | delpos*16
	lea      0(%a3,%d5.l),%a2      | command.tree[delpos]
	move.l   4(%a2),%d5           | tp=command.tree[delpos].parent
	beq.s    ll_0000            | geen parent
	move.l   %d5,%a1              | parent
	cmp.l    (%a1),%a2            | *parent==command.tree[delpos]?
	bne.s    ll_0000            | nope
	move.l   %d2,(%a1)            | *parent=NO_NODE
ll_0000:
| Register gebruik:
| D0 = gebruikt door insertnm
| D1 = gebruikt door insertnm, d1.h is schoon
| D2 = gebruikt door insertnm
| D3 = hash, boven schoon
| D4 = command.tree
| D5 = rle_hash, boven schoon
| D6 = pos, d6.h is schoon
| D7 = to_do
|
| A0 = command, blijft behouden
| A1 = gebruikt door insertnm
| A2 = gebruikt door insertnm
| A3 = gebruikt door insertnm
| A4 = gebruikt door insertnm
| A5 = gebruikt door insertnm
| A6 = nkey, blijft behouden
| A7 = SP
| nodes gedeleted, nu de insert no match
	move.w   %d0,%d2              | pos, bovenste helft is schoon
	add.l    120(%a0),%d0  | new_key
	move.l   %d0,%a6              | new_key, blijft hier
	moveq    #0,%d0              | clear d0
	move.w   110(%a0),%d0    | rle_hash
	move.w   124(%a0),%d3  | delta_hash
	tst.b    %d0                 | staat er nog een rle open?
	bne      inrle              | yep!
	move.b   (%a6),%d6            | delta hash2
	eor.b    %d6,%d3              | delta hash
	move.w   %d3,%d0              | delta hash==0?
	beq      innew_rle          | nope!
	lsl.w    #8,%d3              | shift hash
	move.b   %d6,%d3              | delta hash2
	move.w   %d3,124(%a0)  | store delta_hash

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************


| Register gebruik:
| D0 = hash, later tp            ; init
| D1 = tmp, d1.h is schoon
| D2 = nnode*16,                 ; init
| D3 = tmp
| D4 = sp
| D5 = best_match_pos
| D6 = -best_match
| D7 = -opt_match
|
| A0 = command                   ; init
| A1 = tp
| A2 = tmp
| A3 = left
| A4 = right
| A5 = tmp
| A6 = nkey                      ; init
| A7 = hist


insert_fast:
	lsl.l    #4,%d2              | nnode*16
	move.l   126(%a0),%a1        | command.root
	add.l    %d2,%a3              | tp=command.tree+nnode
	lsl.l    #2,%d0              | hash*4
	move.l   %a3,%a5              | tmp=command.tree+nnode
	move.l   %a6,(%a3)+           | tp->key=nkey
	add.l    %d0,%a1              | command.root+command.delta_hash
	move.l   %a1,(%a3)+           | tp->parent=command.root+command.delta_hash
	lea      4(%a3),%a4           | right=&(tp->c_right)
	move.l   (%a1),%d0            | tp=command.root[command.delta_hash]
	beq.s    ll_0002      | einde insert
	move.l   %a5,(%a1)            | command.root[command.delta_hash]=tmp
ll_0001:
	move.l   %d0,%a2              | tp
	move.l   %a6,%a5              | nkey
	move.l   (%a2)+,%a1           | tp->key
	cmp.b    (%a1)+,(%a5)+        | key's gelijk? 1
	beq.s    ll_0004             |
| hier gegarandeerd geen max match
	bcc.s    ll_0003            | nkey>okey
     | nkey<okey
	move.l   %d0,(%a4)            | *right=tp
	move.l   %a4,(%a2)+           | tp->parent=right
	move.l   %a2,%a4              | right=&(tp->c_left)
	move.l   (%a2),%d0            | tp=*right
	bne.s    ll_0001       | next!
	move.l   %d0,(%a3)            | *left=0
	movem.l  (%sp)+,%d2-%d7/%a2-%a6
	rts
ll_0002:                   | voor als de tree leeg was
	move.l   %a5,(%a1)            | command.root[command.delta_hash]=tmp
	move.l   %d0,(%a3)            | *left=0
	move.l   %d0,(%a4)            | *right=0
	movem.l  (%sp)+,%d2-%d7/%a2-%a6
	rts

ll_0003:
     | nkey>okey
	move.l   %d0,(%a3)            | *left=tp
	move.l   %a3,(%a2)            | tp->parent=left
	lea      8(%a2),%a3           | left=&(tp->c_right)
	move.l   (%a3),%d0            | tp=*left
	bne.s    ll_0001       | next!
	move.l   %d0,(%a4)            | *right=0
	movem.l  (%sp)+,%d2-%d7/%a2-%a6
	rts

ll_0004:
	addq.l   #4,%a2              | a2=tp->c_left
	move.l   (%a2)+,%d4           | tp->c_left
	beq.s    ll_0005          | tp->c_left==0
	move.l   %d4,%a6              | tp->c_left
	move.l   %a3,4(%a6)           | (tp->c_left)->parent=left
ll_0005:
	move.l   %d4,(%a3)            | *left=tp->c_left

	move.l   (%a2),%d4            | tp->c_right
	beq.s    ll_0006         | tp->c_right==0
	move.l   %d4,%a6              | tp->c_right
	move.l   %a4,4(%a6)           | (tp->c_right)->parent=right
ll_0006:
	move.l   %d4,(%a4)            | *right=tp->c_right

	move.w   14(%a0),%d4   | command.max_match
	move.l   %a5,%a6              | nkey+MAXD_MATCH
	move.b   -1(%a6,%d4.w),%d2     | orig
	move.b   -1(%a1,%d4.w),%d1     | kar
	move.w   %a6,%d3              | compare waarde
	not.b    %d1                 | ~kar
	move.b   %d1,-1(%a6,%d4.w)     | sentinel
ll_0007:
	cmp.b    (%a1)+,(%a5)+        | 2
	bne.s    ll_0008        |
	cmp.b    (%a1)+,(%a5)+        | 3
	bne.s    ll_0008        |
	cmp.b    (%a1)+,(%a5)+        | 4
	bne.s    ll_0008        |
	cmp.b    (%a1)+,(%a5)+        | 5
	bne.s    ll_0008        |
	cmp.b    (%a1)+,(%a5)+        | 6
	bne.s    ll_0008        |
	cmp.b    (%a1)+,(%a5)+        | 7
	bne.s    ll_0008        |
	cmp.b    (%a1)+,(%a5)+        | 8
	bne.s    ll_0008        |
	cmp.b    (%a1)+,(%a5)+        | 9
	beq      ll_0007
ll_0008:
	sub.w    %a5,%d3              | d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
	move.w   %a1,%d1              | tp->key+matchlen
	not.w    %d1                 |
	add.w    %a5,%d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	moveq    #2,%d0              | corrigeer d3
	sub.w    %d3,%d0              | return opt_match
	move.w   %d1,136(%a0)| command.best_match_pos=best_match_pos
	move.b   %d2,-1(%a6,%d4.w)     | restore orig
	movem.l  (%sp)+,%d2-%d7/%a2-%a6
	rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

innew_rle:
	move.b   %d6,124+1(%a0)| store delta_hash
	move.l   %a6,%a1              | key
	move.w   14(%a0),%d0   | max_match
	move.b   0(%a6,%d0.w),%d1      | orig
	move.b   %d6,%d5              | rle_char
	not.b    %d6                 | ~rle_char
	move.b   %d6,0(%a6,%d0.w)      | sentry
ll_0009:
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0010           |
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0010           |
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0010           |
	cmp.b    (%a1)+,%d5           | compare
	beq.s    ll_0009          |
ll_0010:
	move.b   %d1,0(%a6,%d0.w)      | restore orig
	move.w   %a6,%d1              | key
	not.w    %d1                 | -key-1
	add.w    %a1,%d1              | rle_size=p-key-1
	lsl.w    #8,%d5              | rle_char<<8
	move.b   %d1,%d5              | rle_hash
	move.w   %d5,%d0              | hash
	subq.w   #1,%d5              | rle_size--
	move.w   %d5,110(%a0)    | store rle_hash
	bra.s    insert2_fast            | insert
insert2_ai:
	addq.w   #1,%d1              | rle_size++
	move.w   %d0,110(%a0)    | store rle_hash
	addq.w   #1,%d0              | rle_size++
	bra.s    insert2_fast            | insert
inrle:
	move.w   %d0,%d5              | rle_hash
	move.b   %d0,%d1              | rle_size
	subq.w   #1,%d5              | rle_size--
	move.w   %d5,110(%a0)    | store rle_hash
	cmp.b    0(%a6,%d1.w),%d3      | rle_size==rle_max?
	beq.s    insert2_ai         | yep

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************


| Register gebruik:
| D1 = rle_size, d1.h is schoon  ; init

| A6 = nkey                      ; init
| A7 = hist


insert2_fast:
	moveq    #0,%d0
	move.b   (%a6),%d7            | rle_char
	cmp.b    -3(%a6),%d7          | nkey[-3]
	bne.s    ll_0011            | geen match gevonden
	move.w   %d0,136(%a0)| command.best_match_pos=best_match_pos
	moveq    #2,%d0              | match offset
	add.w    %d1,%d0              | opt_match
ll_0011:
	movem.l  (%sp)+,%d2-%d7/%a2-%a6
	rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

| Register gebruik:
| D0 = pos.l
| D1 = d1.h is schoon
| D2 = to_do, dbra compatible
| D3 =
| D4 =
| D5 = NO_NODE
| D6 = delpos
| D7 = to_do
|
| A0 = command
| A1 = tree
| A2 =
| A3 =
| A4 =
| A5 =
| A6 =
| A7 = SP

multi:
	move.w   %d0,112(%a0)    | command.last_pos=pos
	move.l   %d1,%d7              | to_do
|eerst de nodes wissen
	moveq    #0,%d5              | NO_NODE
	move.l   114(%a0),%a1        | command.tree
	moveq    #0,%d6              | init delpos
	move.w   118(%a0),%d6     | delpos
ll_0012:
	addq.w   #1,%d6              | delpos++
	move.l   %d6,%d4              | delpos
	lsl.l    #4,%d4              | delpos*16
	lea      0(%a1,%d4.l),%a2      | command.tree[delpos]
	move.l   4(%a2),%d4           | tp=command.tree[delpos].parent
	beq.s    ll_0013            | geen parent
	move.l   %d4,%a3              | parent
	cmp.l    (%a3),%a2            | *parent==command.tree[delpos]?
	bne.s    ll_0013            | nope
	move.l   %d5,(%a3)            | *parent=NO_NODE
ll_0013:
	dbra     %d1,ll_0012       | delete next
	move.w   %d6,118(%a0)     | command.del_pos=del_pos
| Register gebruik:
| D0 = gebruikt door insertnm
| D1 = gebruikt door insertnm, d1.h is schoon
| D2 = gebruikt door insertnm
| D3 = hash, boven schoon
| D4 = command.tree
| D5 = rle_hash, boven schoon
| D6 = pos, d6.h is schoon
| D7 = to_do
|
| A0 = command, blijft behouden
| A1 = gebruikt door insertnm
| A2 = gebruikt door insertnm
| A3 = gebruikt door insertnm
| A4 = gebruikt door insertnm
| A5 = gebruikt door insertnm
| A6 = nkey, blijft behouden
| A7 = SP
| nodes gedeleted, nu de insert no match
	move.l   %a1,%d4              | d4=command.tree
	moveq    #0,%d6              | clear d6
	sub.l    %d7,%d0              | pos-=to_do
	moveq    #0,%d3              | clear d3
	move.w   %d0,%d6              | pos, bovenste helft is schoon
	add.l    120(%a0),%d0  | new_key
	move.w   110(%a0),%d5    | rle_hash
	move.l   %d0,%a6              | new_key, blijft hier
	move.w   124(%a0),%d3  | delta_hash
	subq.w   #1,%d7              | d7 dbra compatible
	tst.b    %d5                 | staat er nog een rle open?
	bne      rle                | yep!
insertnm_loop:
	move.b   (%a6),%d1            | delta hash2
	eor.b    %d1,%d3              | delta hash
	move.l   %d3,%d0              | hash==0?
	beq      new_rle            | yep!
	lsl.w    #8,%d3              | shift hash
	move.l   %d6,%d2              | nnode
	move.b   %d1,%d3              | delta hash 2

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

| Register gebruik:
| D0 = hash, later tp            ; init
| D1 = tmp, d1.h is schoon
| D2 = nnode*16                  ; init
| D3 =
| D4 = 
| D5 =
| D6 = 
| D7 =
|
| A0 = command                   ; init
| A1 = tp
| A2 = tmp
| A3 = left
| A4 = right
| A5 = tmp
| A6 = nkey                      ; init
| A7 = SP

insertnm_fast:
	lsl.l    #4,%d2              | nnode*16
	move.l   %d4,%a3              | command.tree
	add.l    %d2,%a3              | tp=command.tree+nnode
	lsl.l    #2,%d0              | hash*4
	move.l   %a3,%a5              | tmp=command.tree+nnode
	move.l   126(%a0),%a1        | command.root
	move.l   %a6,(%a3)+           | tp->key=nkey
	add.l    %d0,%a1              | command.root+command.delta_hash
	move.l   %a1,(%a3)+           | tp->parent=command.root+command.delta_hash
	move.l   (%a1),%d0            | tp=command.root[command.delta_hash]
	beq.s    ll_0016   | einde insert
	move.l   %a5,(%a1)            | command.root[command.delta_hash]=tmp
	lea      4(%a3),%a4           | right=&(tp->c_right)
ll_0014:
	move.l   %d0,%a2              | tp
	move.l   %a6,%a5              | nkey
	move.l   (%a2)+,%a1           | tp->key
	cmp.b    (%a1)+,(%a5)+        | key's gelijk? 1
	beq.s    ll_0018              |
ll_0015:
	bcc.s    ll_0017              | nkey>okey
     | nkey<okey
	move.l   %d0,(%a4)            | *right=tp
	move.l   %a4,(%a2)+           | tp->parent=right
	move.l   %a2,%a4              | right=&(tp->c_left)
	move.l   (%a2),%d0            | tp=*right
	bne.s    ll_0014    | next!
	move.l   %d0,(%a3)            | *left=0
	addq.l   #1,%a6              | key++
	addq.w   #1,%d6              | pos++
	dbra     %d7,insertnm_loop   | next
	bra      insert_nm_end      | insert
ll_0016:                | voor als de tree leeg was
	move.l   %a5,(%a1)            | command.root[command.delta_hash]=tmp
	move.l   %d0,(%a3)+           | *left=0
	move.l   %d0,(%a3)            | *right=0
	addq.l   #1,%a6              | key++
	addq.w   #1,%d6              | pos++
	dbra     %d7,insertnm_loop   | next
	bra      insert_nm_end      | insert
ll_0017:
     | nkey>okey
	move.l   %d0,(%a3)            | *left=tp
	move.l   %a3,(%a2)            | tp->parent=left
	lea      8(%a2),%a3           | left=&(tp->c_right)
	move.l   (%a3),%d0            | tp=*left
	bne.s    ll_0014    | next!
	move.l   %d0,(%a4)            | *right=0
	addq.l   #1,%a6              | key++
	addq.w   #1,%d6              | pos++
	dbra     %d7,insertnm_loop   | next
	bra      insert_nm_end      | insert

ll_0018:                           | max match
	addq.l   #4,%a2              | a2=tp->c_left
	move.l   (%a2)+,%d2           | tp->c_left
	beq.s    ll_0019          | tp->c_left==0
	move.l   %d2,%a5              | tp->c_left
	move.l   %a3,4(%a5)           | (tp->c_left)->parent=left
ll_0019:
	move.l   %d2,(%a3)            | *left=tp->c_left
	move.l   (%a2),%d2            | tp->c_right
	beq.s    ll_0020         | tp->c_right==0
	move.l   %d2,%a5              | tp->c_right
	move.l   %a4,4(%a5)           | (tp->c_right)->parent=right
ll_0020:
	move.l   %d2,(%a4)            | *right=tp->c_right
	addq.l   #1,%a6              | key++
	addq.w   #1,%d6              | pos++
	dbra     %d7,insertnm_loop   | next
	bra      insert_nm_end      | insert

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

new_rle:
	move.l   %a6,%a1              | key
	move.b   %d1,%d5              | rle_char
	move.w   14(%a0),%d0   | max_match
	move.b   0(%a6,%d0.w),%d2      | orig
	not.b    %d1                 | ~rle_char
	move.b   %d1,0(%a6,%d0.w)      | sentry
ll_0021:
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0022           |
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0022           |
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0022           |
	cmp.b    (%a1)+,%d5           | compare
	beq.s    ll_0021          |
ll_0022:
	move.b   %d2,0(%a6,%d0.w)      | restore orig
	move.w   %a6,%d1              | key
	not.w    %d1                 | -key-1
	add.w    %a1,%d1              | rle_size=p-key-1
	lsl.w    #8,%d5              | rle_char<<8
	move.b   %d1,%d5              | rle_hash
	move.l   %d5,%d0              | hash
	move.l   %d6,%d2              | nnode
	move.b   %d5,%d3              | rle_size
	add.l    %a6,%d3              | key+rle_size
	exg      %d3,%a6              | swap regs
	bra.s    insert2nm_fast          | insert

rlemaxmatch:
	addq.l   #1,%a6              | rle_size++
	addq.w   #1,%d0              | rle_size++
	addq.w   #1,%d6              | pos++
	dbra     %d7,f_rle           | next
	move.b   %d5,%d0              | rle_size
	neg.w    %d0                 | -rle_size
	lea      0(%a6,%d0.w),%a6      | restore a6
	moveq    #0,%d3              | clear d3
	bra      insert_nm_end      | insert

rle:
	move.b   %d5,%d3              | rle_size
	add.l    %a6,%d3              | key+rle_size
	exg      %d3,%a6              | swap regs
f_rle:
	move.l   %d5,%d0              | hash
	move.l   %d6,%d2              | nnode
	move.l   %d3,%a1              | key
	move.b   (%a6),%d1            | rle_char
	cmp.b    (%a1),%d1            | rle_size==rle_max?
	beq.s    rlemaxmatch        | yep

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

| Register gebruik:
| D0 = hash                      ; init
| D1 = tmp, d1.h is schoon
| D2 = nnode*16                  ; init
| D3 =
| D4 = 
| D5 =
| D6 = 
| D7 =
|
| A0 = command                   ; init
| A1 = tp
| A2 = tmp
| A3 = left
| A4 = right
| A5 = tmp
| A6 = nkey                      ; init
| A7 = SP


insert2nm_fast:
	addq.w   #1,%d6              | pos++
	subq.b   #1,%d5              | rle_hash--
	dbeq     %d7,f_rle           | next
	bne.s    insert2_nm_cont_end
	moveq    #0,%d3              | clear d3
	move.b   -1(%a6),%d3          | vul d3 met rle char
	dbra     %d7,insertnm_loop   | next
	bra.s    insert_nm_end      | insert

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

insert2_nm_cont_end:
	moveq    #0,%d3              | clear d3
	move.b   %d5,%d3              | rle_size
	neg.w    %d3                 | -rle_size
	lea      0(%a6,%d3.w),%a6      | restore a6
	moveq    #0,%d3              | clear d3
insert_nm_end:
	tst.b    %d5                 | staat er nog een rle open?
	bne.s    ll_0026               | yep!
	move.b   (%a6),%d2            | delta hash2
	eor.b    %d2,%d3              | delta hash
	move.l   %d3,%d0              | delta hash==0?
	beq.s    ll_0023           | nope!
	lsl.w    #8,%d3              | shift hash
	move.b   %d2,%d3              | delta hash2
	move.l   %d6,%d2              | nnode
	move.w   %d5,110(%a0)    | store rle_hash
	move.w   %d3,124(%a0)  | store delta_hash
	move.l   %d4,%a3              | command.tree
	bra      insert_fast             | insert

ll_0023:
	move.b   %d2,%d3              | delta hash2
	move.w   %d3,124(%a0)  | store delta_hash
	move.l   %a6,%a1              | key
	move.w   14(%a0),%d0   | max_match
	move.b   0(%a6,%d0.w),%d1      | orig
	move.l   %d3,%d5              | rle_char
	not.b    %d2                 | ~rle_char
	move.b   %d2,0(%a6,%d0.w)      | sentry
ll_0024:
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0025           |
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0025           |
	cmp.b    (%a1)+,%d5           | compare
	bne.s    ll_0025           |
	cmp.b    (%a1)+,%d5           | compare
	beq.s    ll_0024          |
ll_0025:
	move.b   %d1,0(%a6,%d0.w)      | restore orig
	move.w   %a6,%d1              | key
	not.w    %d1                 | -key-1
	add.w    %a1,%d1              | rle_size=p-key-1
	lsl.w    #8,%d5              | rle_char<<8
	move.b   %d1,%d5              | rle_hash
	move.l   %d5,%d0              | hash
	move.l   %d6,%d2              | nnode
	moveq    #0,%d1              | clear d2
	move.b   %d5,%d1              | size
	subq.w   #1,%d5              | rle_size--
	move.w   %d5,110(%a0)    | store rle_hash
	move.l   %d4,%a3              | command.tree
	bra      insert2_fast            | insert
ll_0026:
	move.l   %d5,%d0              | hash
	move.l   %d6,%d2              | nnode
	move.b   %d5,%d3              | rle_size
	add.l    %a6,%d3              | key+rle_size
	move.l   %d3,%a1              | key
	moveq    #0,%d1              | clear d1
	move.b   %d5,%d1              |
	subq.w   #1,%d5              | rle_size--
	move.w   %d5,110(%a0)    | store rle_hash
	moveq    #0,%d3              | clear d3
	move.b   (%a6),%d3            | rle_char
	move.w   %d3,124(%a0)  | store delta_hash
	cmp.b    (%a1),%d3            | rle_size==rle_max?
	move.l   %d4,%a3              | command.tree
	bne      insert2_fast            | nope
	addq.w   #1,%d5              | rle_size++
	addq.w   #1,%d1              | rle_size++
	addq.w   #1,%d0
	move.w   %d5,110(%a0)    | store rle_hash
	bra      insert2_fast            | insert


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

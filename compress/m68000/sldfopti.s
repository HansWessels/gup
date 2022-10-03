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
	movel   sp@(4),d0           | pos
	movel   sp@(8),a0           | *com
	moveml  d2-d7/a2-a6,sp@-  | save registers
	moveq    #0,d1              | clear d1, d1.h is nu schoon
	movew   d0,d1              | pos
	subw    a0@(112),d1    | pos-command.last_pos
	subqw   #1,d1              | to_do dbra compatible
	bne      multi              | er moeten meerdere gedaan worden
	movew   d0,a0@(112)    | command.last_pos=pos
|eerst de node wissen
	moveq    #1,d5              | init delpos
	movel   a0@(114),a3        | command.tree
	moveq    #0,d2              | clear d2
	addw    a0@(118),d5     | delpos++
	movew   d5,a0@(118)     | command.del_pos=del_pos
	lsll    #4,d5              | delpos*16
	lea      a3@(0,d5:l),a2      | command.tree[delpos]
	movel   a2@(4),d5           | tp=command.tree[delpos].parent
	beq    ll_0000            | geen parent
	movel   d5,a1              | parent
	cmpl    a1@,a2            | *parent==command.tree[delpos]?
	bne    ll_0000            | nope
	movel   d2,a1@            | *parent=NO_NODE
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
	movew   d0,d2              | pos, bovenste helft is schoon
	addl    a0@(120),d0  | new_key
	movel   d0,a6              | new_key, blijft hier
	moveq    #0,d0              | clear d0
	movew   a0@(110),d0    | rle_hash
	movew   a0@(124),d3  | delta_hash
	tstb    d0                 | staat er nog een rle open?
	bne      inrle              | yep!
	moveb   a6@,d6            | delta hash2
	eorb    d6,d3              | delta hash
	movew   d3,d0              | delta hash==0?
	beq      innew_rle          | nope!
	lslw    #8,d3              | shift hash
	moveb   d6,d3              | delta hash2
	movew   d3,a0@(124)  | store delta_hash

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
	lsll    #4,d2              | nnode*16
	movel   a0@(126),a1        | command.root
	addl    d2,a3              | tp=command.tree+nnode
	lsll    #2,d0              | hash*4
	movel   a3,a5              | tmp=command.tree+nnode
	movel   a6,a3@+           | tp->key=nkey
	addl    d0,a1              | command.root+command.delta_hash
	movel   a1,a3@+           | tp->parent=command.root+command.delta_hash
	lea      a3@(4),a4           | right=&(tp->c_right)
	movel   a1@,d0            | tp=command.root[command.delta_hash]
	beq    ll_0002      | einde insert
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
ll_0001:
	movel   d0,a2              | tp
	movel   a6,a5              | nkey
	movel   a2@+,a1           | tp->key
	cmpb    a1@+,a5@+        | key's gelijk? 1
	beq    ll_0004             |
| hier gegarandeerd geen max match
	bcc    ll_0003            | nkey>okey
     | nkey<okey
	movel   d0,a4@            | *right=tp
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d0            | tp=*right
	bne    ll_0001       | next!
	movel   d0,a3@            | *left=0
	moveml  sp@+,d2-d7/a2-a6
	rts
ll_0002:                   | voor als de tree leeg was
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	movel   d0,a3@            | *left=0
	movel   d0,a4@            | *right=0
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0003:
     | nkey>okey
	movel   d0,a3@            | *left=tp
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d0            | tp=*left
	bne    ll_0001       | next!
	movel   d0,a4@            | *right=0
	moveml  sp@+,d2-d7/a2-a6
	rts

ll_0004:
	addql   #4,a2              | a2=tp->c_left
	movel   a2@+,d4           | tp->c_left
	beq    ll_0005          | tp->c_left==0
	movel   d4,a6              | tp->c_left
	movel   a3,a6@(4)           | (tp->c_left)->parent=left
ll_0005:
	movel   d4,a3@            | *left=tp->c_left

	movel   a2@,d4            | tp->c_right
	beq    ll_0006         | tp->c_right==0
	movel   d4,a6              | tp->c_right
	movel   a4,a6@(4)           | (tp->c_right)->parent=right
ll_0006:
	movel   d4,a4@            | *right=tp->c_right

	movew   a0@(14),d4   | command.max_match
	movel   a5,a6              | nkey+MAXD_MATCH
	moveb   a6@(-1,d4:w),d2     | orig
	moveb   a1@(-1,d4:w),d1     | kar
	movew   a6,d3              | compare waarde
	notb    d1                 | ~kar
	moveb   d1,a6@(-1,d4:w)     | sentinel
ll_0007:
	cmpb    a1@+,a5@+        | 2
	bne    ll_0008        |
	cmpb    a1@+,a5@+        | 3
	bne    ll_0008        |
	cmpb    a1@+,a5@+        | 4
	bne    ll_0008        |
	cmpb    a1@+,a5@+        | 5
	bne    ll_0008        |
	cmpb    a1@+,a5@+        | 6
	bne    ll_0008        |
	cmpb    a1@+,a5@+        | 7
	bne    ll_0008        |
	cmpb    a1@+,a5@+        | 8
	bne    ll_0008        |
	cmpb    a1@+,a5@+        | 9
	beq      ll_0007
ll_0008:
	subw    a5,d3              | d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
	movew   a1,d1              | tp->key+matchlen
	notw    d1                 |
	addw    a5,d1              | pos=nkey+matchlen-(tp->key+matchlen)-1
	moveq    #2,d0              | corrigeer d3
	subw    d3,d0              | return opt_match
	movew   d1,a0@(136)| command.best_match_pos=best_match_pos
	moveb   d2,a6@(-1,d4:w)     | restore orig
	moveml  sp@+,d2-d7/a2-a6
	rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

innew_rle:
	moveb   d6,a0@(124+1)| store delta_hash
	movel   a6,a1              | key
	movew   a0@(14),d0   | max_match
	moveb   a6@(0,d0:w),d1      | orig
	moveb   d6,d5              | rle_char
	notb    d6                 | ~rle_char
	moveb   d6,a6@(0,d0:w)      | sentry
ll_0009:
	cmpb    a1@+,d5           | compare
	bne    ll_0010           |
	cmpb    a1@+,d5           | compare
	bne    ll_0010           |
	cmpb    a1@+,d5           | compare
	bne    ll_0010           |
	cmpb    a1@+,d5           | compare
	beq    ll_0009          |
ll_0010:
	moveb   d1,a6@(0,d0:w)      | restore orig
	movew   a6,d1              | key
	notw    d1                 | -key-1
	addw    a1,d1              | rle_size=p-key-1
	lslw    #8,d5              | rle_char<<8
	moveb   d1,d5              | rle_hash
	movew   d5,d0              | hash
	subqw   #1,d5              | rle_size--
	movew   d5,a0@(110)    | store rle_hash
	bra    insert2_fast            | insert
insert2_ai:
	addqw   #1,d1              | rle_size++
	movew   d0,a0@(110)    | store rle_hash
	addqw   #1,d0              | rle_size++
	bra    insert2_fast            | insert
inrle:
	movew   d0,d5              | rle_hash
	moveb   d0,d1              | rle_size
	subqw   #1,d5              | rle_size--
	movew   d5,a0@(110)    | store rle_hash
	cmpb    a6@(0,d1:w),d3      | rle_size==rle_max?
	beq    insert2_ai         | yep

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************


| Register gebruik:
| D1 = rle_size, d1.h is schoon  ; init

| A6 = nkey                      ; init
| A7 = hist


insert2_fast:
	moveq    #0,d0
	moveb   a6@,d7            | rle_char
	cmpb    a6@(-3),d7          | nkey[-3]
	bne    ll_0011            | geen match gevonden
	movew   d0,a0@(136)| command.best_match_pos=best_match_pos
	moveq    #2,d0              | match offset
	addw    d1,d0              | opt_match
ll_0011:
	moveml  sp@+,d2-d7/a2-a6
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
	movew   d0,a0@(112)    | command.last_pos=pos
	movel   d1,d7              | to_do
|eerst de nodes wissen
	moveq    #0,d5              | NO_NODE
	movel   a0@(114),a1        | command.tree
	moveq    #0,d6              | init delpos
	movew   a0@(118),d6     | delpos
ll_0012:
	addqw   #1,d6              | delpos++
	movel   d6,d4              | delpos
	lsll    #4,d4              | delpos*16
	lea      a1@(0,d4:l),a2      | command.tree[delpos]
	movel   a2@(4),d4           | tp=command.tree[delpos].parent
	beq    ll_0013            | geen parent
	movel   d4,a3              | parent
	cmpl    a3@,a2            | *parent==command.tree[delpos]?
	bne    ll_0013            | nope
	movel   d5,a3@            | *parent=NO_NODE
ll_0013:
	dbra     d1,ll_0012       | delete next
	movew   d6,a0@(118)     | command.del_pos=del_pos
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
	movel   a1,d4              | d4=command.tree
	moveq    #0,d6              | clear d6
	subl    d7,d0              | pos-=to_do
	moveq    #0,d3              | clear d3
	movew   d0,d6              | pos, bovenste helft is schoon
	addl    a0@(120),d0  | new_key
	movew   a0@(110),d5    | rle_hash
	movel   d0,a6              | new_key, blijft hier
	movew   a0@(124),d3  | delta_hash
	subqw   #1,d7              | d7 dbra compatible
	tstb    d5                 | staat er nog een rle open?
	bne      rle                | yep!
insertnm_loop:
	moveb   a6@,d1            | delta hash2
	eorb    d1,d3              | delta hash
	movel   d3,d0              | hash==0?
	beq      new_rle            | yep!
	lslw    #8,d3              | shift hash
	movel   d6,d2              | nnode
	moveb   d1,d3              | delta hash 2

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
	lsll    #4,d2              | nnode*16
	movel   d4,a3              | command.tree
	addl    d2,a3              | tp=command.tree+nnode
	lsll    #2,d0              | hash*4
	movel   a3,a5              | tmp=command.tree+nnode
	movel   a0@(126),a1        | command.root
	movel   a6,a3@+           | tp->key=nkey
	addl    d0,a1              | command.root+command.delta_hash
	movel   a1,a3@+           | tp->parent=command.root+command.delta_hash
	movel   a1@,d0            | tp=command.root[command.delta_hash]
	beq    ll_0016   | einde insert
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	lea      a3@(4),a4           | right=&(tp->c_right)
ll_0014:
	movel   d0,a2              | tp
	movel   a6,a5              | nkey
	movel   a2@+,a1           | tp->key
	cmpb    a1@+,a5@+        | key's gelijk? 1
	beq    ll_0018              |
ll_0015:
	bcc    ll_0017              | nkey>okey
     | nkey<okey
	movel   d0,a4@            | *right=tp
	movel   a4,a2@+           | tp->parent=right
	movel   a2,a4              | right=&(tp->c_left)
	movel   a2@,d0            | tp=*right
	bne    ll_0014    | next!
	movel   d0,a3@            | *left=0
	addql   #1,a6              | key++
	addqw   #1,d6              | pos++
	dbra     d7,insertnm_loop   | next
	bra      insert_nm_end      | insert
ll_0016:                | voor als de tree leeg was
	movel   a5,a1@            | command.root[command.delta_hash]=tmp
	movel   d0,a3@+           | *left=0
	movel   d0,a3@            | *right=0
	addql   #1,a6              | key++
	addqw   #1,d6              | pos++
	dbra     d7,insertnm_loop   | next
	bra      insert_nm_end      | insert
ll_0017:
     | nkey>okey
	movel   d0,a3@            | *left=tp
	movel   a3,a2@            | tp->parent=left
	lea      a2@(8),a3           | left=&(tp->c_right)
	movel   a3@,d0            | tp=*left
	bne    ll_0014    | next!
	movel   d0,a4@            | *right=0
	addql   #1,a6              | key++
	addqw   #1,d6              | pos++
	dbra     d7,insertnm_loop   | next
	bra      insert_nm_end      | insert

ll_0018:                           | max match
	addql   #4,a2              | a2=tp->c_left
	movel   a2@+,d2           | tp->c_left
	beq    ll_0019          | tp->c_left==0
	movel   d2,a5              | tp->c_left
	movel   a3,a5@(4)           | (tp->c_left)->parent=left
ll_0019:
	movel   d2,a3@            | *left=tp->c_left
	movel   a2@,d2            | tp->c_right
	beq    ll_0020         | tp->c_right==0
	movel   d2,a5              | tp->c_right
	movel   a4,a5@(4)           | (tp->c_right)->parent=right
ll_0020:
	movel   d2,a4@            | *right=tp->c_right
	addql   #1,a6              | key++
	addqw   #1,d6              | pos++
	dbra     d7,insertnm_loop   | next
	bra      insert_nm_end      | insert

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

new_rle:
	movel   a6,a1              | key
	moveb   d1,d5              | rle_char
	movew   a0@(14),d0   | max_match
	moveb   a6@(0,d0:w),d2      | orig
	notb    d1                 | ~rle_char
	moveb   d1,a6@(0,d0:w)      | sentry
ll_0021:
	cmpb    a1@+,d5           | compare
	bne    ll_0022           |
	cmpb    a1@+,d5           | compare
	bne    ll_0022           |
	cmpb    a1@+,d5           | compare
	bne    ll_0022           |
	cmpb    a1@+,d5           | compare
	beq    ll_0021          |
ll_0022:
	moveb   d2,a6@(0,d0:w)      | restore orig
	movew   a6,d1              | key
	notw    d1                 | -key-1
	addw    a1,d1              | rle_size=p-key-1
	lslw    #8,d5              | rle_char<<8
	moveb   d1,d5              | rle_hash
	movel   d5,d0              | hash
	movel   d6,d2              | nnode
	moveb   d5,d3              | rle_size
	addl    a6,d3              | key+rle_size
	exg      d3,a6              | swap regs
	bra    insert2nm_fast          | insert

rlemaxmatch:
	addql   #1,a6              | rle_size++
	addqw   #1,d0              | rle_size++
	addqw   #1,d6              | pos++
	dbra     d7,f_rle           | next
	moveb   d5,d0              | rle_size
	negw    d0                 | -rle_size
	lea      a6@(0,d0:w),a6      | restore a6
	moveq    #0,d3              | clear d3
	bra      insert_nm_end      | insert

rle:
	moveb   d5,d3              | rle_size
	addl    a6,d3              | key+rle_size
	exg      d3,a6              | swap regs
f_rle:
	movel   d5,d0              | hash
	movel   d6,d2              | nnode
	movel   d3,a1              | key
	moveb   a6@,d1            | rle_char
	cmpb    a1@,d1            | rle_size==rle_max?
	beq    rlemaxmatch        | yep

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
	addqw   #1,d6              | pos++
	subqb   #1,d5              | rle_hash--
	dbeq     d7,f_rle           | next
	bne    insert2_nm_cont_end
	moveq    #0,d3              | clear d3
	moveb   a6@(-1),d3          | vul d3 met rle char
	dbra     d7,insertnm_loop   | next
	bra    insert_nm_end      | insert

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

insert2_nm_cont_end:
	moveq    #0,d3              | clear d3
	moveb   d5,d3              | rle_size
	negw    d3                 | -rle_size
	lea      a6@(0,d3:w),a6      | restore a6
	moveq    #0,d3              | clear d3
insert_nm_end:
	tstb    d5                 | staat er nog een rle open?
	bne    ll_0026               | yep!
	moveb   a6@,d2            | delta hash2
	eorb    d2,d3              | delta hash
	movel   d3,d0              | delta hash==0?
	beq    ll_0023           | nope!
	lslw    #8,d3              | shift hash
	moveb   d2,d3              | delta hash2
	movel   d6,d2              | nnode
	movew   d5,a0@(110)    | store rle_hash
	movew   d3,a0@(124)  | store delta_hash
	movel   d4,a3              | command.tree
	bra      insert_fast             | insert

ll_0023:
	moveb   d2,d3              | delta hash2
	movew   d3,a0@(124)  | store delta_hash
	movel   a6,a1              | key
	movew   a0@(14),d0   | max_match
	moveb   a6@(0,d0:w),d1      | orig
	movel   d3,d5              | rle_char
	notb    d2                 | ~rle_char
	moveb   d2,a6@(0,d0:w)      | sentry
ll_0024:
	cmpb    a1@+,d5           | compare
	bne    ll_0025           |
	cmpb    a1@+,d5           | compare
	bne    ll_0025           |
	cmpb    a1@+,d5           | compare
	bne    ll_0025           |
	cmpb    a1@+,d5           | compare
	beq    ll_0024          |
ll_0025:
	moveb   d1,a6@(0,d0:w)      | restore orig
	movew   a6,d1              | key
	notw    d1                 | -key-1
	addw    a1,d1              | rle_size=p-key-1
	lslw    #8,d5              | rle_char<<8
	moveb   d1,d5              | rle_hash
	movel   d5,d0              | hash
	movel   d6,d2              | nnode
	moveq    #0,d1              | clear d2
	moveb   d5,d1              | size
	subqw   #1,d5              | rle_size--
	movew   d5,a0@(110)    | store rle_hash
	movel   d4,a3              | command.tree
	bra      insert2_fast            | insert
ll_0026:
	movel   d5,d0              | hash
	movel   d6,d2              | nnode
	moveb   d5,d3              | rle_size
	addl    a6,d3              | key+rle_size
	movel   d3,a1              | key
	moveq    #0,d1              | clear d1
	moveb   d5,d1              |
	subqw   #1,d5              | rle_size--
	movew   d5,a0@(110)    | store rle_hash
	moveq    #0,d3              | clear d3
	moveb   a6@,d3            | rle_char
	movew   d3,a0@(124)  | store delta_hash
	cmpb    a1@,d3            | rle_size==rle_max?
	movel   d4,a3              | command.tree
	bne      insert2_fast            | nope
	addqw   #1,d5              | rle_size++
	addqw   #1,d1              | rle_size++
	addqw   #1,d0
	movew   d5,a0@(110)    | store rle_hash
	bra      insert2_fast            | insert


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
|*******************************************************************************

|
| Sliding dictionary for ARJ ST
|
| (c) 1996 Hans 'Mr Ni!' Wessels
|

	.globl insertnms
	.globl insert2nms


| The following exports should be after "%include command.mac"

	.globl _insertnms
	.globl _insert2nms

	.even

	.equ MAXDELTA,7
	.equ MAXD_MATCH,9
	.equ MAXD_MATCH2,9

| node struct zit hier alsvolgt in elkaar:
|
| key             0
| parent          2
| c_left          4
| c_right         6
|

	.text

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************


| Register gebruik:
| D0 = init nkey, later left, boven clear
| D1 = init nnode, later right, boven clear
| D2 = init hash, later current, boven clear
| D3 = nnode * 8
| D4 = tmp
| D5 = key
| D6 = NO_NODE
| D7 = 
|
| A0 = com
| A1 = com->dictionary
| A2 = com->tree
| A3 = nkey
| A4 = tp
| A5 = tmp [q]
| A6 = tmp [p]
| A7 = SP
insert2nms_q_end:
	movew   d1,a5@            | root[hash]=nnode
	movel   d6,a4@            | left en right = NO_NODE
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
|void insertnms(word nkey, word nnode, word hash, struct packstruct *com)
_insert2nms:
insert2nms:
	moveml  d2-d6/a2-a6,sp@-  | save registers
	movel   sp@(44),d0          | nkey
	movel   sp@(48),d1          | nnode
	movel   sp@(52),d2          | hash
	movel   sp@(56),a0          | *com
	moveq    #0,d3              | clear d3 voor nkey
	movew   d0,d3              | maak bovenkant nkey leeg
	movel   a0@(120),a1  | com->dictionary
	lea      a1@(0,d3:l),a3      | dictionary+nkey
	movel   a0@(114),a2        | com->tree
	movew   d1,d3              | maak bovenkant nnonde leeg
	movel   a0@(138),a5       | com->root2
	moveq    #0,d4              | clear d4
	movew   d2,d4              | hash
	movel   d3,d1              | right
	addl    d4,d4              | hash*2
	addl    d4,a5              | root+hash
	lsll    #3,d3              | nnode*8, offset in tree
	lea      a2@(0,d3:l),a4      | tp=tree+nnode
	movew   d0,a4@+           | tp->key=nkey
	movel   d1,d0              | left
	movew   d2,a4@+           | tp->parent=hash
	moveq    #0,d2              | clear d2
	moveq    #-1,d6             | NO_NODE
	movew   a5@,d2            | current
	cmpw    d2,d6              | current==NO_NODE?
	beq    insert2nms_q_end   | current = 0 stoppe!
	movew   d1,a5@            | root[hash]=nnode
	moveq    #0,d5              | clear voor tp->key
ll_0000:
	movel   d2,d4              | current
	lsll    #3,d4              | current*8, offset in tree
	lea      a2@(0,d4:l),a4      | tp=tree+current
	movel   a3,a5              | q=nkey
	movew   a4@+,d5           | tp->key
	lea      a1@(0,d5:l),a6      | p=dictionary+tp->key
	cmpb    a6@+,a5@+        | 1
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 2
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 3
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 4
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 5
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 6
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 7
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 8
	bne    ll_0005         |
	cmpb    a6@+,a5@+        | 9
	bne    ll_0005
ll_0001:
	addql   #2,a4              | a4 stond op parent
	movew   a4@+,d5           | tp->c_left
	cmpw    d5,d6              | tp->c_left==NO_NODE?
	beq    ll_0002     | tp->c_left==NO_NODE
	movel   d5,d4              | tp->c_left
	lsll    #3,d4              | offset in tree
	movew   d0,a2@(2,d4:l)      | tree[tp->c_left].parent=left
ll_0002:
	lsll    #3,d0              | offset in tree
	movew   d5,a2@(6,d0:l)      | tree[left].c_right=tp->left
	movew   a4@,d5            | tp->c_right
	cmpw    d5,d6              | tp->c_right==NO_NODE?
	beq    ll_0003     | tp->c_right==NO_NODE
	movel   d5,d4              | tp->c_right
	lsll    #3,d4              | offset in tree
	movew   d1,a2@(2,d4:l)      | tree[tp->c_right].parent=right
ll_0003:
	lsll    #3,d1              | offset in tree
	movew   d5,a2@(4,d1:l)      | tree[right].c_leftt=tp->c_right

	lea      a2@(4,d3:l),a5      | tree+nnode+c_left
	movew   a5@+,d5           | tmp=tp->c_left
	movew   a5@,a5@-         | tp->c_left=tp->c_right
	movew   d5,a5@(2)           | tp->c_right=tmp
	movel   a0@(102),d5        | com->link
	beq    ll_0004           | geen link
	movel   d5,a6              | com->link
	lsrl    #3,d3              | nnode
	movew   d3,a4@(-4)          | tp->parent=nnode
	addl    d3,d3              | offset in link array
	movew   d2,a6@(0,d3:l)      | link[nnode]=current
ll_0004:
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
     
ll_0005:
	bcc    ll_0006              | nkey>okey
     | nkey<okey
	movew   d1,a4@+           | tp->parent=right
	lsll    #3,d1              | offset in tree
	movew   d2,a2@(4,d1:l)      | tree[right].c_left=current
	movel   d2,d1              | left=current
	movew   a4@,d2            | current=tp->c_left
	cmpw    d6,d2              | current==NO_NODE?
	bne      ll_0000    | next
     
	lsll    #3,d0              | offset in tree
	movew   d2,a2@(6,d0:l)      | tree[left].c_right=NO_NODE
|     lsl.l    #3,d1              ; offset in tree
|     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
	lea      a2@(4,d3:l),a4      | tree+nnode+c_left
	movew   a4@+,d2           | tmp=tp->c_left
	movew   a4@,a4@-         | tp->c_left=tp->c_right
	movew   d2,a4@(2)           | tp->c_right=tmp
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
     
ll_0006:
     | nkey>okey
	movew   d0,a4@+           | tp->parent=left
	lsll    #3,d0              | offset in tree
	movew   d2,a2@(6,d0:l)      | tree[left].c_right=current
	movel   d2,d0              | right=current
	movew   a4@(2),d2           | current=tp->c_right
	cmpw    d6,d2              | current==NO_NODE?
	bne      ll_0000    | next

|     lsl.l    #3,d0              ; offset in tree
|     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
	lsll    #3,d1              | offset in tree
	movew   d2,a2@(4,d1:l)      | tree[right].c_left=NO_NODE
     
	lea      a2@(4,d3:l),a4      | tree+nnode+c_left
	movew   a4@+,d2           | tmp=tp->c_left
	movew   a4@,a4@-         | tp->c_left=tp->c_right
	movew   d2,a4@(2)           | tp->c_right=tmp
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
     

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

| Register gebruik:
| D0 = init nkey, later left, boven clear
| D1 = init nnode, later right, boven clear
| D2 = init hash, later current, boven clear
| D3 = nnode * 8
| D4 = tmp
| D5 = key
| D6 = NO_NODE
| D7 = 
|
| A0 = com
| A1 = com->dictionary
| A2 = com->tree
| A3 = nkey
| A4 = tp
| A5 = tmp [q]
| A6 = tmp [p]
| A7 = SP
insertnms_q_end:
	movew   d1,a5@            | root[hash]=nnode
	movel   d6,a4@            | left en right = NO_NODE
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
|void insertnms(word nkey, word nnode, word hash, struct packstruct *com)
_insertnms:
insertnms:
	moveml  d2-d6/a2-a6,sp@-  | save registers
	movel   sp@(44),d0          | nkey
	movel   sp@(48),d1          | nnode
	movel   sp@(52),d2          | hash
	movel   sp@(56),a0          | *com
	moveq    #0,d3              | clear d3 voor nkey
	movew   d0,d3              | maak bovenkant nkey leeg
	movel   a0@(120),a1  | com->dictionary
	lea      a1@(0,d3:l),a3      | dictionary+nkey
	movel   a0@(114),a2        | com->tree
	movew   d1,d3              | maak bovenkant nnonde leeg
	movel   a0@(126),a5        | com->root
	moveq    #0,d4              | clear d4
	movew   d2,d4              | hash
	movel   d3,d1              | right
	addl    d4,d4              | hash*2
	addl    d4,a5              | root+hash
	lsll    #3,d3              | nnode*8, offset in tree
	lea      a2@(0,d3:l),a4      | tp=tree+nnode
	movew   d0,a4@+           | tp->key=nkey
	movel   d1,d0              | left
	movew   d2,a4@+           | tp->parent=hash
	moveq    #0,d2              | clear d2
	moveq    #-1,d6             | NO_NODE
	movew   a5@,d2            | current
	cmpw    d2,d6              | current==NO_NODE?
	beq    insertnms_q_end    | current = 0 stoppe!
	movew   d1,a5@            | root[hash]=nnode
	moveq    #0,d5              | clear voor tp->key
ll_0007:
	movel   d2,d4              | current
	lsll    #3,d4              | current*8, offset in tree
	lea      a2@(0,d4:l),a4      | tp=tree+current
	movel   a3,a5              | q=nkey
	movew   a4@+,d5           | tp->key
	lea      a1@(0,d5:l),a6      | p=dictionary+tp->key
	cmpb    a6@+,a5@+        | 1
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 2
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 3
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 4
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 5
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 6
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 7
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 8
	bne    ll_0012         |
	cmpb    a6@+,a5@+        | 9
	bne    ll_0012
ll_0008:
	addql   #2,a4              | a4 stond op parent
	movew   a4@+,d5           | tp->c_left
	cmpw    d5,d6              | tp->c_left==NO_NODE?
	beq    ll_0009     | tp->c_left==NO_NODE
	movel   d5,d4              | tp->c_left
	lsll    #3,d4              | offset in tree
	movew   d0,a2@(2,d4:l)      | tree[tp->c_left].parent=left
ll_0009:
	lsll    #3,d0              | offset in tree
	movew   d5,a2@(6,d0:l)      | tree[left].c_right=tp->left
	movew   a4@,d5            | tp->c_right
	cmpw    d5,d6              | tp->c_right==NO_NODE?
	beq    ll_0010     | tp->c_right==NO_NODE
	movel   d5,d4              | tp->c_right
	lsll    #3,d4              | offset in tree
	movew   d1,a2@(2,d4:l)      | tree[tp->c_right].parent=right
ll_0010:
	lsll    #3,d1              | offset in tree
	movew   d5,a2@(4,d1:l)      | tree[right].c_leftt=tp->c_right

	lea      a2@(4,d3:l),a5      | tree+nnode+c_left
	movew   a5@+,d5           | tmp=tp->c_left
	movew   a5@,a5@-         | tp->c_left=tp->c_right
	movew   d5,a5@(2)           | tp->c_right=tmp
	movel   a0@(102),d5        | com->link
	beq    ll_0011           | geen link
	movel   d5,a6              | com->link
	lsrl    #3,d3              | nnode
	movew   d3,a4@(-4)          | tp->parent=nnode
	addl    d3,d3              | offset in link array
	movew   d2,a6@(0,d3:l)      | link[nnode]=current
ll_0011:
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
     
ll_0012:
	bcc    ll_0013              | nkey>okey
     | nkey<okey
	movew   d1,a4@+           | tp->parent=right
	lsll    #3,d1              | offset in tree
	movew   d2,a2@(4,d1:l)      | tree[right].c_left=current
	movel   d2,d1              | left=current
	movew   a4@,d2            | current=tp->c_left
	cmpw    d6,d2              | current==NO_NODE?
	bne      ll_0007    | next
     
	lsll    #3,d0              | offset in tree
	movew   d2,a2@(6,d0:l)      | tree[left].c_right=NO_NODE
|     lsl.l    #3,d1              ; offset in tree
|     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
	lea      a2@(4,d3:l),a4      | tree+nnode+c_left
	movew   a4@+,d2           | tmp=tp->c_left
	movew   a4@,a4@-         | tp->c_left=tp->c_right
	movew   d2,a4@(2)           | tp->c_right=tmp
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
     
ll_0013:
     | nkey>okey
	movew   d0,a4@+           | tp->parent=left
	lsll    #3,d0              | offset in tree
	movew   d2,a2@(6,d0:l)      | tree[left].c_right=current
	movel   d2,d0              | right=current
	movew   a4@(2),d2           | current=tp->c_right
	cmpw    d6,d2              | current==NO_NODE?
	bne      ll_0007    | next

|     lsl.l    #3,d0              ; offset in tree
|     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
	lsll    #3,d1              | offset in tree
	movew   d2,a2@(4,d1:l)      | tree[right].c_left=NO_NODE
     
	lea      a2@(4,d3:l),a4      | tree+nnode+c_left
	movew   a4@+,d2           | tmp=tp->c_left
	movew   a4@,a4@-         | tp->c_left=tp->c_right
	movew   d2,a4@(2)           | tp->c_right=tmp
	moveml  sp@+,d2-d6/a2-a6  | restore registers
	rts                         | done
     

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

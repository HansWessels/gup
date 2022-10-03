|
| Sliding dictionary for ARJ ST
|
| (c) 1996 Hans 'Mr Ni!' Wessels
|

	.globl insertnmm
	.globl insert2nmm


| The following exports should be after "%include command.mac"

	.globl _insertnmm
	.globl _insert2nmm

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
| D6 = 
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
insert2nmm_q_end:
	move.w   %d1,(%a5)            | root[hash]=nnode
	move.l   %d2,(%a4)            | left en right = NNODE
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
	rts                         | done
|void insertnmm(word nkey, word nnode, word hash, struct packstruct *com)
_insert2nmm:
insert2nmm:
	movem.l  %d2-%d5/%a2-%a6,-(%sp)  | save registers
	move.l   40(%sp),%d0          | nkey
	move.l   44(%sp),%d1          | nnode
	move.l   48(%sp),%d2          | hash
	move.l   52(%sp),%a0          | *com
	moveq    #0,%d3              | clear d3 voor nkey
	move.w   %d0,%d3              | maak bovenkant nkey leeg
	move.l   120(%a0),%a1  | com->dictionary
	lea      0(%a1,%d3.l),%a3      | dictionary+nkey
	move.l   114(%a0),%a2        | com->tree
	move.w   %d1,%d3              | maak bovenkant nnonde leeg
	move.l   138(%a0),%a5       | com->root2
	moveq    #0,%d4              | clear d4
	move.w   %d2,%d4              | hash
	move.l   %d3,%d1              | right
	add.l    %d4,%d4              | hash*2
	add.l    %d4,%a5              | root+hash
	lsl.l    #3,%d3              | nnode*8, offset in tree
	lea      0(%a2,%d3.l),%a4      | tp=tree+nnode
	move.w   %d0,(%a4)+           | tp->key=nkey
	move.l   %d1,%d0              | left
	move.w   %d2,(%a4)+           | tp->parent=hash
	moveq    #0,%d2              | clear d2
	move.w   (%a5),%d2            | current
	beq.s    insert2nmm_q_end   | current = 0 stoppe!
	move.w   %d1,(%a5)            | root[hash]=nnode
	moveq    #0,%d5              | clear voor tp->key
ll_0000:
	move.l   %d2,%d4              | current
	lsl.l    #3,%d4              | current*8, offset in tree
	lea      0(%a2,%d4.l),%a4      | tp=tree+current
	move.l   %a3,%a5              | q=nkey
	move.w   (%a4)+,%d5           | tp->key
	lea      0(%a1,%d5.l),%a6      | p=dictionary+tp->key
	cmp.b    (%a6)+,(%a5)+        | 1
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 2
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 3
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 4
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 5
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 6
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 7
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 8
	bne.s    ll_0005         |
	cmp.b    (%a6)+,(%a5)+        | 9
	bne.s    ll_0005
ll_0001:
	addq.l   #2,%a4              | a4 stond op parent
	move.w   (%a4)+,%d5           | tp->c_left
	beq.s    ll_0002     | tp->c_left==NO_NODE
	move.l   %d5,%d4              | tp->c_left
	lsl.l    #3,%d4              | offset in tree
	move.w   %d0,2(%a2,%d4.l)      | tree[tp->c_left].parent=left
ll_0002:
	lsl.l    #3,%d0              | offset in tree
	move.w   %d5,6(%a2,%d0.l)      | tree[left].c_right=tp->left
	move.w   (%a4),%d5            | tp->c_right
	beq.s    ll_0003     | tp->c_right==NO_NODE
	move.l   %d5,%d4              | tp->c_right
	lsl.l    #3,%d4              | offset in tree
	move.w   %d1,2(%a2,%d4.l)      | tree[tp->c_right].parent=right
ll_0003:
	lsl.l    #3,%d1              | offset in tree
	move.w   %d5,4(%a2,%d1.l)      | tree[right].c_leftt=tp->c_right

	lea      4(%a2,%d3.l),%a5      | tree+nnode+c_left
	move.w   (%a5)+,%d5           | tmp=tp->c_left
	move.w   (%a5),-(%a5)         | tp->c_left=tp->c_right
	move.w   %d5,2(%a5)           | tp->c_right=tmp
	move.l   102(%a0),%d5        | com->link
	beq.s    ll_0004           | geen link
	move.l   %d5,%a6              | com->link
	lsr.l    #3,%d3              | nnode
	move.w   %d3,-4(%a4)          | tp->parent=nnode
	add.l    %d3,%d3              | offset in link array
	move.w   %d2,0(%a6,%d3.l)      | link[nnode]=current
ll_0004:
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
	rts                         | done
     
ll_0005:
	bcc.s    ll_0006              | nkey>okey
     | nkey<okey
	move.w   %d1,(%a4)+           | tp->parent=right
	lsl.l    #3,%d1              | offset in tree
	move.w   %d2,4(%a2,%d1.l)      | tree[right].c_left=current
	move.l   %d2,%d1              | left=current
	move.w   (%a4),%d2            | current=tp->c_right
	bne      ll_0000    | next
     
|     lsl.l    #3,d0              ; offset in tree
|     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
	lsl.l    #3,%d1              | offset in tree
	move.w   %d2,4(%a2,%d1.l)      | tree[right].c_left=NO_NODE
	lea      4(%a2,%d3.l),%a4      | tree+nnode+c_left
	move.w   (%a4)+,%d2           | tmp=tp->c_left
	move.w   (%a4),-(%a4)         | tp->c_left=tp->c_right
	move.w   %d2,2(%a4)           | tp->c_right=tmp
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
	rts                         | done
     
ll_0006:
     | nkey>okey
	move.w   %d0,(%a4)+           | tp->parent=left
	lsl.l    #3,%d0              | offset in tree
	move.w   %d2,6(%a2,%d0.l)      | tree[left].c_right=current
	move.l   %d2,%d0              | right=current
	move.w   2(%a4),%d2           | current=tp->c_right
	bne      ll_0000    | next

	lsl.l    #3,%d0              | offset in tree
	move.w   %d2,6(%a2,%d0.l)      | tree[left].c_right=NO_NODE
|     lsl.l    #3,d1              ; offset in tree
|     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
     
	lea      4(%a2,%d3.l),%a4      | tree+nnode+c_left
	move.w   (%a4)+,%d2           | tmp=tp->c_left
	move.w   (%a4),-(%a4)         | tp->c_left=tp->c_right
	move.w   %d2,2(%a4)           | tp->c_right=tmp
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
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
| D6 = 
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
insertnmm_q_end:
	move.w   %d1,(%a5)            | root[hash]=nnode
	move.l   %d2,(%a4)            | left en right = NNODE
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
	rts                         | done
|void insertnmm(word nkey, word nnode, word hash, struct packstruct *com)
_insertnmm:
insertnmm:
	movem.l  %d2-%d5/%a2-%a6,-(%sp)  | save registers
	move.l   40(%sp),%d0          | nkey
	move.l   44(%sp),%d1          | nnode
	move.l   48(%sp),%d2          | hash
	move.l   52(%sp),%a0          | *com
	moveq    #0,%d3              | clear d3 voor nkey
	move.w   %d0,%d3              | maak bovenkant nkey leeg
	move.l   120(%a0),%a1  | com->dictionary
	lea      0(%a1,%d3.l),%a3      | dictionary+nkey
	move.l   114(%a0),%a2        | com->tree
	move.w   %d1,%d3              | maak bovenkant nnonde leeg
	move.l   126(%a0),%a5        | com->root
	moveq    #0,%d4              | clear d4
	move.w   %d2,%d4              | hash
	move.l   %d3,%d1              | right
	add.l    %d4,%d4              | hash*2
	add.l    %d4,%a5              | root+hash
	lsl.l    #3,%d3              | nnode*8, offset in tree
	lea      0(%a2,%d3.l),%a4      | tp=tree+nnode
	move.w   %d0,(%a4)+           | tp->key=nkey
	move.l   %d1,%d0              | left
	move.w   %d2,(%a4)+           | tp->parent=hash
	moveq    #0,%d2              | clear d2
	move.w   (%a5),%d2            | current
	beq.s    insertnmm_q_end    | current = 0 stoppe!
	move.w   %d1,(%a5)            | root[hash]=nnode
	moveq    #0,%d5              | clear voor tp->key
ll_0007:
	move.l   %d2,%d4              | current
	lsl.l    #3,%d4              | current*8, offset in tree
	lea      0(%a2,%d4.l),%a4      | tp=tree+current
	move.l   %a3,%a5              | q=nkey
	move.w   (%a4)+,%d5           | tp->key
	lea      0(%a1,%d5.l),%a6      | p=dictionary+tp->key
	cmp.b    (%a6)+,(%a5)+        | 1
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 2
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 3
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 4
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 5
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 6
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 7
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 8
	bne.s    ll_0012         |
	cmp.b    (%a6)+,(%a5)+        | 9
	bne.s    ll_0012
ll_0008:
	addq.l   #2,%a4              | a4 stond op parent
	move.w   (%a4)+,%d5           | tp->c_left
	beq.s    ll_0009     | tp->c_left==NO_NODE
	move.l   %d5,%d4              | tp->c_left
	lsl.l    #3,%d4              | offset in tree
	move.w   %d0,2(%a2,%d4.l)      | tree[tp->c_left].parent=left
ll_0009:
	lsl.l    #3,%d0              | offset in tree
	move.w   %d5,6(%a2,%d0.l)      | tree[left].c_right=tp->left
	move.w   (%a4),%d5            | tp->c_right
	beq.s    ll_0010     | tp->c_right==NO_NODE
	move.l   %d5,%d4              | tp->c_right
	lsl.l    #3,%d4              | offset in tree
	move.w   %d1,2(%a2,%d4.l)      | tree[tp->c_right].parent=right
ll_0010:
	lsl.l    #3,%d1              | offset in tree
	move.w   %d5,4(%a2,%d1.l)      | tree[right].c_leftt=tp->c_right

	lea      4(%a2,%d3.l),%a5      | tree+nnode+c_left
	move.w   (%a5)+,%d5           | tmp=tp->c_left
	move.w   (%a5),-(%a5)         | tp->c_left=tp->c_right
	move.w   %d5,2(%a5)           | tp->c_right=tmp
	move.l   102(%a0),%d5        | com->link
	beq.s    ll_0011           | geen link
	move.l   %d5,%a6              | com->link
	lsr.l    #3,%d3              | nnode
	move.w   %d3,-4(%a4)          | tp->parent=nnode
	add.l    %d3,%d3              | offset in link array
	move.w   %d2,0(%a6,%d3.l)      | link[nnode]=current
ll_0011:
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
	rts                         | done
     
ll_0012:
	bcc.s    ll_0013              | nkey>okey
     | nkey<okey
	move.w   %d1,(%a4)+           | tp->parent=right
	lsl.l    #3,%d1              | offset in tree
	move.w   %d2,4(%a2,%d1.l)      | tree[right].c_left=current
	move.l   %d2,%d1              | left=current
	move.w   (%a4),%d2            | current=tp->c_left
	bne      ll_0007    | next
     
	lsl.l    #3,%d0              | offset in tree
	move.w   %d2,6(%a2,%d0.l)      | tree[left].c_right=NO_NODE
|     lsl.l    #3,d1              ; offset in tree
|     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
	lea      4(%a2,%d3.l),%a4      | tree+nnode+c_left
	move.w   (%a4)+,%d2           | tmp=tp->c_left
	move.w   (%a4),-(%a4)         | tp->c_left=tp->c_right
	move.w   %d2,2(%a4)           | tp->c_right=tmp
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
	rts                         | done
     
ll_0013:
     | nkey>okey
	move.w   %d0,(%a4)+           | tp->parent=left
	lsl.l    #3,%d0              | offset in tree
	move.w   %d2,6(%a2,%d0.l)      | tree[left].c_right=current
	move.l   %d2,%d0              | right=current
	move.w   2(%a4),%d2           | current=tp->right
	bne      ll_0007    | next

|     lsl.l    #3,d0              ; offset in tree
|     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
	lsl.l    #3,%d1              | offset in tree
	move.w   %d2,4(%a2,%d1.l)      | tree[right].c_left=NO_NODE
     
	lea      4(%a2,%d3.l),%a4      | tree+nnode+c_left
	move.w   (%a4)+,%d2           | tmp=tp->c_left
	move.w   (%a4),-(%a4)         | tp->c_left=tp->c_right
	move.w   %d2,2(%a4)           | tp->c_right=tmp
	movem.l  (%sp)+,%d2-%d5/%a2-%a6  | restore registers
	rts                         | done
     

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************

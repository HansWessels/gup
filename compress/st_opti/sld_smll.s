;
; Sliding dictionary for ARJ ST
;
; (c) 1996 Hans 'Mr Ni!' Wessels
;

                EXPORT insertnms
                EXPORT insert2nms

include "command.mac"

; The following exports should be after "include command.mac"

if GNUC
                EXPORT _insertnms
                EXPORT _insert2nms
endif

                EVEN

MAXDELTA        equ 7
MAXD_MATCH      equ 9
MAXD_MATCH2     equ 9

; node struct zit hier alsvolgt in elkaar:
;
; key             0
; parent          2
; c_left          4
; c_right         6
;

                TEXT

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
;*******************************************************************************


; Register gebruik:
; D0 = init nkey, later left, boven clear
; D1 = init nnode, later right, boven clear
; D2 = init hash, later current, boven clear
; D3 = nnode * 8
; D4 = tmp
; D5 = key
; D6 = NO_NODE
; D7 = 
;
; A0 = com
; A1 = com->dictionary
; A2 = com->tree
; A3 = nkey
; A4 = tp
; A5 = tmp [q]
; A6 = tmp [p]
; A7 = SP
insert2nms_q_end:
     move.w   d1,(a5)            ; root[hash]=nnode
     move.l   d6,(a4)            ; left en right = NO_NODE
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
;void insertnms(word nkey, word nnode, word hash, struct packstruct *com)
if GNUC
_insert2nms:
insert2nms:
     movem.l  d2-d6/a2-a6,-(sp)  ; save registers
     move.l   44(sp),d0          ; nkey
     move.l   48(sp),d1          ; nnode
     move.l   52(sp),d2          ; hash
     move.l   56(sp),a0          ; *com
else
insert2nms:
     movem.l  d3-d6/a2-a6,-(sp)  ; save registers
endif
     moveq    #0,d3              ; clear d3 voor nkey
     move.w   d0,d3              ; maak bovenkant nkey leeg
     move.l   dictionary(a0),a1  ; com->dictionary
     lea      0(a1,d3.l),a3      ; dictionary+nkey
     move.l   tree(a0),a2        ; com->tree
     move.w   d1,d3              ; maak bovenkant nnonde leeg
     move.l   root2(a0),a5       ; com->root2
     moveq    #0,d4              ; clear d4
     move.w   d2,d4              ; hash
     move.l   d3,d1              ; right
     add.l    d4,d4              ; hash*2
     add.l    d4,a5              ; root+hash
     lsl.l    #3,d3              ; nnode*8, offset in tree
     lea      0(a2,d3.l),a4      ; tp=tree+nnode
     move.w   d0,(a4)+           ; tp->key=nkey
     move.l   d1,d0              ; left
     move.w   d2,(a4)+           ; tp->parent=hash
     moveq    #0,d2              ; clear d2
     moveq    #-1,d6             ; NO_NODE
     move.w   (a5),d2            ; current
     cmp.w    d2,d6              ; current==NO_NODE?
     beq.s    insert2nms_q_end   ; current = 0 stoppe!
     move.w   d1,(a5)            ; root[hash]=nnode
     moveq    #0,d5              ; clear voor tp->key
.insertnms_loop:
     move.l   d2,d4              ; current
     lsl.l    #3,d4              ; current*8, offset in tree
     lea      0(a2,d4.l),a4      ; tp=tree+current
     move.l   a3,a5              ; q=nkey
     move.w   (a4)+,d5           ; tp->key
     lea      0(a1,d5.l),a6      ; p=dictionary+tp->key
if MAXD_MATCH>0
     cmp.b    (a6)+,(a5)+        ; 1
     bne.s    .end_match         ;
endif
if MAXD_MATCH>1
     cmp.b    (a6)+,(a5)+        ; 2
     bne.s    .end_match         ;
endif
if MAXD_MATCH>2
     cmp.b    (a6)+,(a5)+        ; 3
     bne.s    .end_match         ;
endif
if MAXD_MATCH>3
     cmp.b    (a6)+,(a5)+        ; 4
     bne.s    .end_match         ;
endif
if MAXD_MATCH>4
     cmp.b    (a6)+,(a5)+        ; 5
     bne.s    .end_match         ;
endif
if MAXD_MATCH>5
     cmp.b    (a6)+,(a5)+        ; 6
     bne.s    .end_match         ;
endif
if MAXD_MATCH>6
     cmp.b    (a6)+,(a5)+        ; 7
     bne.s    .end_match         ;
endif
if MAXD_MATCH>7
     cmp.b    (a6)+,(a5)+        ; 8
     bne.s    .end_match         ;
endif
if MAXD_MATCH>8
     cmp.b    (a6)+,(a5)+        ; 9
     bne.s    .end_match
endif
.same:
     addq.l   #2,a4              ; a4 stond op parent
     move.w   (a4)+,d5           ; tp->c_left
     cmp.w    d5,d6              ; tp->c_left==NO_NODE?
     beq.s    .geen_koppel_0     ; tp->c_left==NO_NODE
     move.l   d5,d4              ; tp->c_left
     lsl.l    #3,d4              ; offset in tree
     move.w   d0,2(a2,d4.l)      ; tree[tp->c_left].parent=left
.geen_koppel_0:
     lsl.l    #3,d0              ; offset in tree
     move.w   d5,6(a2,d0.l)      ; tree[left].c_right=tp->left
     move.w   (a4),d5            ; tp->c_right
     cmp.w    d5,d6              ; tp->c_right==NO_NODE?
     beq.s    .geen_koppel_1     ; tp->c_right==NO_NODE
     move.l   d5,d4              ; tp->c_right
     lsl.l    #3,d4              ; offset in tree
     move.w   d1,2(a2,d4.l)      ; tree[tp->c_right].parent=right
.geen_koppel_1:
     lsl.l    #3,d1              ; offset in tree
     move.w   d5,4(a2,d1.l)      ; tree[right].c_leftt=tp->c_right

     lea      4(a2,d3.l),a5      ; tree+nnode+c_left
     move.w   (a5)+,d5           ; tmp=tp->c_left
     move.w   (a5),-(a5)         ; tp->c_left=tp->c_right
     move.w   d5,2(a5)           ; tp->c_right=tmp
     move.l   link(a0),d5        ; com->link
     beq.s    .no_link           ; geen link
     move.l   d5,a6              ; com->link
     lsr.l    #3,d3              ; nnode
     move.w   d3,-4(a4)          ; tp->parent=nnode
     add.l    d3,d3              ; offset in link array
     move.w   d2,0(a6,d3.l)      ; link[nnode]=current
.no_link:
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
     
.end_match:
     bcc.s    .left              ; nkey>okey
     ; nkey<okey
     move.w   d1,(a4)+           ; tp->parent=right
     lsl.l    #3,d1              ; offset in tree
     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=current
     move.l   d2,d1              ; left=current
     move.w   (a4),d2            ; current=tp->c_left
     cmp.w    d6,d2              ; current==NO_NODE?
     bne      .insertnms_loop    ; next
     
     lsl.l    #3,d0              ; offset in tree
     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
;     lsl.l    #3,d1              ; offset in tree
;     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
     lea      4(a2,d3.l),a4      ; tree+nnode+c_left
     move.w   (a4)+,d2           ; tmp=tp->c_left
     move.w   (a4),-(a4)         ; tp->c_left=tp->c_right
     move.w   d2,2(a4)           ; tp->c_right=tmp
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
     
.left:
     ; nkey>okey
     move.w   d0,(a4)+           ; tp->parent=left
     lsl.l    #3,d0              ; offset in tree
     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=current
     move.l   d2,d0              ; right=current
     move.w   2(a4),d2           ; current=tp->c_right
     cmp.w    d6,d2              ; current==NO_NODE?
     bne      .insertnms_loop    ; next

;     lsl.l    #3,d0              ; offset in tree
;     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
     lsl.l    #3,d1              ; offset in tree
     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
     
     lea      4(a2,d3.l),a4      ; tree+nnode+c_left
     move.w   (a4)+,d2           ; tmp=tp->c_left
     move.w   (a4),-(a4)         ; tp->c_left=tp->c_right
     move.w   d2,2(a4)           ; tp->c_right=tmp
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
     

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
;*******************************************************************************

; Register gebruik:
; D0 = init nkey, later left, boven clear
; D1 = init nnode, later right, boven clear
; D2 = init hash, later current, boven clear
; D3 = nnode * 8
; D4 = tmp
; D5 = key
; D6 = NO_NODE
; D7 = 
;
; A0 = com
; A1 = com->dictionary
; A2 = com->tree
; A3 = nkey
; A4 = tp
; A5 = tmp [q]
; A6 = tmp [p]
; A7 = SP
insertnms_q_end:
     move.w   d1,(a5)            ; root[hash]=nnode
     move.l   d6,(a4)            ; left en right = NO_NODE
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
;void insertnms(word nkey, word nnode, word hash, struct packstruct *com)
if GNUC
_insertnms:
insertnms:
     movem.l  d2-d6/a2-a6,-(sp)  ; save registers
     move.l   44(sp),d0          ; nkey
     move.l   48(sp),d1          ; nnode
     move.l   52(sp),d2          ; hash
     move.l   56(sp),a0          ; *com
else
insertnms:
     movem.l  d3-d6/a2-a6,-(sp)  ; save registers
endif
     moveq    #0,d3              ; clear d3 voor nkey
     move.w   d0,d3              ; maak bovenkant nkey leeg
     move.l   dictionary(a0),a1  ; com->dictionary
     lea      0(a1,d3.l),a3      ; dictionary+nkey
     move.l   tree(a0),a2        ; com->tree
     move.w   d1,d3              ; maak bovenkant nnonde leeg
     move.l   root(a0),a5        ; com->root
     moveq    #0,d4              ; clear d4
     move.w   d2,d4              ; hash
     move.l   d3,d1              ; right
     add.l    d4,d4              ; hash*2
     add.l    d4,a5              ; root+hash
     lsl.l    #3,d3              ; nnode*8, offset in tree
     lea      0(a2,d3.l),a4      ; tp=tree+nnode
     move.w   d0,(a4)+           ; tp->key=nkey
     move.l   d1,d0              ; left
     move.w   d2,(a4)+           ; tp->parent=hash
     moveq    #0,d2              ; clear d2
     moveq    #-1,d6             ; NO_NODE
     move.w   (a5),d2            ; current
     cmp.w    d2,d6              ; current==NO_NODE?
     beq.s    insertnms_q_end    ; current = 0 stoppe!
     move.w   d1,(a5)            ; root[hash]=nnode
     moveq    #0,d5              ; clear voor tp->key
.insertnms_loop:
     move.l   d2,d4              ; current
     lsl.l    #3,d4              ; current*8, offset in tree
     lea      0(a2,d4.l),a4      ; tp=tree+current
     move.l   a3,a5              ; q=nkey
     move.w   (a4)+,d5           ; tp->key
     lea      0(a1,d5.l),a6      ; p=dictionary+tp->key
if MAXD_MATCH>0
     cmp.b    (a6)+,(a5)+        ; 1
     bne.s    .end_match         ;
endif
if MAXD_MATCH>1
     cmp.b    (a6)+,(a5)+        ; 2
     bne.s    .end_match         ;
endif
if MAXD_MATCH>2
     cmp.b    (a6)+,(a5)+        ; 3
     bne.s    .end_match         ;
endif
if MAXD_MATCH>3
     cmp.b    (a6)+,(a5)+        ; 4
     bne.s    .end_match         ;
endif
if MAXD_MATCH>4
     cmp.b    (a6)+,(a5)+        ; 5
     bne.s    .end_match         ;
endif
if MAXD_MATCH>5
     cmp.b    (a6)+,(a5)+        ; 6
     bne.s    .end_match         ;
endif
if MAXD_MATCH>6
     cmp.b    (a6)+,(a5)+        ; 7
     bne.s    .end_match         ;
endif
if MAXD_MATCH>7
     cmp.b    (a6)+,(a5)+        ; 8
     bne.s    .end_match         ;
endif
if MAXD_MATCH>8
     cmp.b    (a6)+,(a5)+        ; 9
     bne.s    .end_match
endif
.same:
     addq.l   #2,a4              ; a4 stond op parent
     move.w   (a4)+,d5           ; tp->c_left
     cmp.w    d5,d6              ; tp->c_left==NO_NODE?
     beq.s    .geen_koppel_0     ; tp->c_left==NO_NODE
     move.l   d5,d4              ; tp->c_left
     lsl.l    #3,d4              ; offset in tree
     move.w   d0,2(a2,d4.l)      ; tree[tp->c_left].parent=left
.geen_koppel_0:
     lsl.l    #3,d0              ; offset in tree
     move.w   d5,6(a2,d0.l)      ; tree[left].c_right=tp->left
     move.w   (a4),d5            ; tp->c_right
     cmp.w    d5,d6              ; tp->c_right==NO_NODE?
     beq.s    .geen_koppel_1     ; tp->c_right==NO_NODE
     move.l   d5,d4              ; tp->c_right
     lsl.l    #3,d4              ; offset in tree
     move.w   d1,2(a2,d4.l)      ; tree[tp->c_right].parent=right
.geen_koppel_1:
     lsl.l    #3,d1              ; offset in tree
     move.w   d5,4(a2,d1.l)      ; tree[right].c_leftt=tp->c_right

     lea      4(a2,d3.l),a5      ; tree+nnode+c_left
     move.w   (a5)+,d5           ; tmp=tp->c_left
     move.w   (a5),-(a5)         ; tp->c_left=tp->c_right
     move.w   d5,2(a5)           ; tp->c_right=tmp
     move.l   link(a0),d5        ; com->link
     beq.s    .no_link           ; geen link
     move.l   d5,a6              ; com->link
     lsr.l    #3,d3              ; nnode
     move.w   d3,-4(a4)          ; tp->parent=nnode
     add.l    d3,d3              ; offset in link array
     move.w   d2,0(a6,d3.l)      ; link[nnode]=current
.no_link:
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
     
.end_match:
     bcc.s    .left              ; nkey>okey
     ; nkey<okey
     move.w   d1,(a4)+           ; tp->parent=right
     lsl.l    #3,d1              ; offset in tree
     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=current
     move.l   d2,d1              ; left=current
     move.w   (a4),d2            ; current=tp->c_left
     cmp.w    d6,d2              ; current==NO_NODE?
     bne      .insertnms_loop    ; next
     
     lsl.l    #3,d0              ; offset in tree
     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
;     lsl.l    #3,d1              ; offset in tree
;     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
     lea      4(a2,d3.l),a4      ; tree+nnode+c_left
     move.w   (a4)+,d2           ; tmp=tp->c_left
     move.w   (a4),-(a4)         ; tp->c_left=tp->c_right
     move.w   d2,2(a4)           ; tp->c_right=tmp
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
     
.left:
     ; nkey>okey
     move.w   d0,(a4)+           ; tp->parent=left
     lsl.l    #3,d0              ; offset in tree
     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=current
     move.l   d2,d0              ; right=current
     move.w   2(a4),d2           ; current=tp->c_right
     cmp.w    d6,d2              ; current==NO_NODE?
     bne      .insertnms_loop    ; next

;     lsl.l    #3,d0              ; offset in tree
;     move.w   d2,6(a2,d0.l)      ; tree[left].c_right=NO_NODE
     lsl.l    #3,d1              ; offset in tree
     move.w   d2,4(a2,d1.l)      ; tree[right].c_left=NO_NODE
     
     lea      4(a2,d3.l),a4      ; tree+nnode+c_left
     move.w   (a4)+,d2           ; tmp=tp->c_left
     move.w   (a4),-(a4)         ; tp->c_left=tp->c_right
     move.w   d2,2(a4)           ; tp->c_right=tmp
if GNUC
     movem.l  (sp)+,d2-d6/a2-a6  ; restore registers
else
     movem.l  (sp)+,d3-d6/a2-a6  ; restore registers
endif
     rts                         ; done
     

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
;*******************************************************************************

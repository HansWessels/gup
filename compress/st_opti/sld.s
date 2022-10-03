;
; Sliding dictionary for ARJ ST
;
; (c) 1996 Hans 'Mr Ni!' Wessels
;

                EXPORT find_dictionary
                EXPORT insert
                EXPORT insert2
                EXPORT insertnm
                EXPORT insert2nm

; XDEF command

include "command.mac"

; The following exports should be after "include command.mac"

if GNUC
                EXPORT _find_dictionary
endif

                EVEN

MAXDELTA        equ 7
MAXD_MATCH      equ 9
MAXD_MATCH2     equ 9
HISTSIZE        equ 16


; node struct zit hier alsvolgt in elkaar:
;
; key
; parent
; c_left
; c_right
;

                TEXT


if MAXD_MATCH>9
error "MAXD_MATCH too big!"
endif
if MAXD_MATCH2>9
error "MAXD_MATCH2 too big!"
endif

if 1

; Register gebruik:
; D0 = pos.l        |
; D1 =              | pos -> to_do-1
; D2 =              | nnode (=pos), .h=0
; D3 =
; D4 =
; D5 =
; D6 =
; D7 =
;
; A0 = command      | last_pos -> tree
; A1 =
; A2 =
; A3 =
; A4 =
; A5 =
; A6 =
; A7 = SP

;c_codetype find_dictionary(long pos, packstruct *com)
if GNUC
_find_dictionary:
find_dictionary:
     move.l   4(sp),d0           ; pos
     move.l   8(sp),a0           ; *com
     movem.l  d2-d7/a2-a6,-(sp)  ; save registers
else
find_dictionary:
     movem.l  d3-d7/a2-a6,-(sp)  ; save registers
endif
     moveq    #0,d1              ; clear d1, d1.h is nu schoon
     lea      last_pos(a0),a0    ; command.last_pos
     move.w   d0,d1              ; pos
     move.l   d1,d2              ; pos, d2.h is schoon
     sub.w    (a0),d1            ; pos-command.last_pos
     move.w   d0,(a0)+           ; command.last_pos=pos
     subq.w   #1,d1              ; to_do dbra compatible
     bne      multi              ; er moeten meerdere gedaan worden
; Register gebruik:
; D0 = pos.l
; D1 = 0.l
; D2 = nnode, .h=0
; D3 =
; D4 =
; D5 =              | delpos
; D6 =
; D7 = 
;
; A0 = command.tree | command.delpos
; A1 =              | tmp
; A2 =              | tmp
; A3 =              | command.tree
; A4 =
; A5 =
; A6 =
; A7 = SP
;eerst de node wissen
     move.l   (a0)+,a3           ; command.tree
     moveq    #1,d5              ; init delpos
     add.w    (a0),d5            ; delpos++
     move.w   d5,(a0)+           ; command.del_pos=del_pos
     lsl.l    #4,d5              ; delpos*16
     lea      0(a3,d5.l),a2      ; command.tree[delpos]
     move.l   4(a2),d5           ; tp=command.tree[delpos].parent
     beq.s    .no_fix            ; geen parent
     move.l   d5,a1              ; parent
     cmp.l    (a1),a2            ; *parent==command.tree[delpos]?
     bne.s    .no_fix            ; nope
     move.l   d1,(a1)            ; *parent=NO_NODE
.no_fix:
; Register gebruik:
; D0 = pos.l
; D1 = 0.l                | delta_hash
; D2 = nnode, .h = 0
; D3 =                    | rle_size -> (nkey)
; D4 =                    | command.tree
; D5 =                    |
; D6 =                    | hash .h=0
; D7 = 
;
; A0 = command.dictionary | command.delta_hash
; A1 =
; A2 =
; A3 = command.tree
; A4 =
; A5 =
; A6 =                    | new_key+1
; A7 = SP
; nodes gedeleted, nu de insert
     add.l    (a0)+,d0           ; +command.dictionary=new_key
     move.l   d0,a6              ; new_key, blijft hier
     move.w   (a0),d1            ; delta_hash
     move.b   rle_hash+1-delta_hash(a0),d3 ; rle_size, staat er nog een rle_open?
     bne      inrle              ; yep!
     move.b   (a6)+,d3           ; delta hash2
     eor.b    d3,d1              ; delta hash
     move.l   d1,d7              ; delta hash==0?
     beq      innew_rle          ; nope!
     lsl.w    #8,d1              ; shift hash
     move.b   d3,d1              ; delta hash
     move.w   d1,(a0)+           ; store delta_hash

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

if 1

; Register gebruik:
; D0 = nkey, later opt match     ; init
; D1 = tmp, d1.h is schoon
; D2 = nnode(=pos)               ; init
; D3 = (nkey)                    ; init
; D4 = nkey+1
; D5 = best_match_pos
; D6 = best_match
; D7 = hash d7.h schoon (init) -> tp 
;
; A0 = command. root (init) ; -> hist_index -> hist -> best_match_pos ; init
; A1 = tp
; A2 = tmp
; A3 = command.tree (init) ; -> left
; A4 = right
; A5 = tmp
; A6 = nkey+1 (init) ; -> later hist        ; init
; A7 = sp


insert:
     lsl.l    #4,d2              ; nnode*16
     move.l   (a0)+,a1           ; command.root
     add.l    d2,a3              ; tp=command.tree+nnode
     lsl.l    #2,d7              ; hash*4
     move.l   a3,a5              ; tmp=command.tree+nnode
     move.l   d0,(a3)+           ; tp->key=nkey
     add.l    d7,a1              ; command.root+command.delta_hash
     move.l   a1,(a3)+           ; tp->parent=command.root+command.delta_hash
     moveq    #0,d0              ; opt_match=0
     move.l   (a1),d7            ; tp=command.root[command.delta_hash]
     beq.s    .s_insert_end      ; einde insert
     moveq    #1,d4              ; clear d4, index++
     lea      4(a3),a4           ; right=&(tp->c_right)
     add.w    (a0),d4            ; history index
     moveq    #10,d6             ; shift size
     and.w    #HISTSIZE-1,d4     ; modulo
     move.w   d4,(a0)+           ; store
     lsl.w    d6,d4              ; offset in hist array
     add.l    (a0)+,d4           ; plus hist_array
     moveq    #0,d6              ; best match=0
     exg      d4,a6              ; verwissel nkey en hist pointer
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
.insert_loop:
     move.l   d7,a2              ; tp
     move.l   (a2)+,a1           ; tp->key
     cmp.b    (a1)+,d3           ; key's gelijk? 1
     beq.s    .match_cont        ;
; hier gegarandeerd geen max match
     bcc.s    .s_left            ; nkey>okey
     ; nkey<okey
     move.l   d7,(a4)            ; *right=tp
     move.l   a4,(a2)+           ; tp->parent=right
     move.l   a2,a4              ; right=&(tp->c_left)
     move.l   (a2),d7            ; tp=*right
     bne.s    .insert_loop       ; next!
     move.l   d7,(a3)            ; *left=0
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6
endif
     rts
.s_insert_end:                   ; voor als de tree leeg was
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
     move.l   d7,(a3)+           ; *left=0
     move.l   d7,(a3)            ; *right=0
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6
endif
     rts

.s_left:
     ; nkey>okey
     move.l   d7,(a3)            ; *left=tp
     move.l   a3,(a2)            ; tp->parent=left
     lea      8(a2),a3           ; left=&(tp->c_right)
     move.l   (a3),d7            ; tp=*left
     bne.s    .insert_loop       ; next!
     move.l   d7,(a4)            ; *right=0
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6
endif
     rts

.match_cont:
     move.l   d4,a5              ; nkey
if MAXD_MATCH>2
     cmp.b    (a1)+,(a5)+        ; 2
     bne.s    .end_match         ;
endif
if MAXD_MATCH>3
     cmp.b    (a1)+,(a5)+        ; 3
     bne.s    .end_match         ;
endif
if MAXD_MATCH>4
     cmp.b    (a1)+,(a5)+        ; 4
     bne.s    .end_match         ;
endif
if MAXD_MATCH>5
     cmp.b    (a1)+,(a5)+        ; 5
     bne.s    .end_match         ;
endif
if MAXD_MATCH>6
     cmp.b    (a1)+,(a5)+        ; 6
     bne.s    .end_match         ;
endif
if MAXD_MATCH>7
     cmp.b    (a1)+,(a5)+        ; 7
     bne.s    .end_match         ;
endif
if MAXD_MATCH>8
     cmp.b    (a1)+,(a5)+        ; 8
     bne.s    .end_match         ;
endif
     cmp.b    (a1)+,(a5)+        ; 9
     beq      .same
.end_match:
     bcc.s    .left              ; nkey>okey
     ; nkey<okey
     move.l   d7,(a4)            ; *right=tp
     move.w   a5,d7              ; eind positie
     sub.w    d4,d7              ; d7=(nkey+matchlen+1)-(nkey+1)=matchlen
     cmp.w    d6,d7              ; matchlen<best_match?
     ble.s    .nnmm0             ; yep, geen nieuwe best_match
     move.w   a1,d1              ; tp->key+matchlen
     move.w   d7,d6              ; best_match=matchlen
     not.w    d1                 ;
     move.w   d7,(a6)+           ; store match in hist array
     add.w    a5,d1              ; pos=nkey+matchlen-(tp->key+matchlen)-1
     move.w   d1,(a6)+           ; store pos in hist array
     sub.w    d0,d7              ; matchlen-optmatch
     bgt.s    .good_match0       ; nope
     move.l   fast_log-best_match_pos(a0),a1 ; log tabel
     beq.s    .diff_2_0          ; matchlen-optmatch==-2
     moveq    #MAXDELTA,d7       ; MAXDELTA
     sub.b    0(a1,d1.l),d7      ; -log(pos)
     add.b    0(a1,d5.l),d7      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     bgt.s    .good_match0       ; yep!
     bra.s    .nnmm0             ; anders geen nieuwe optmatch
.diff_2_0:
     moveq    #2*MAXDELTA,d7     ; MAXDELTA
     sub.b    0(a1,d1.l),d7      ; -log(pos)
     add.b    0(a1,d5.l),d7      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     ble.s    .nnmm0             ; anders geen nieuwe optmatch
.good_match0:
     moveq    #2,d0              ; offset
     move.l   d1,d5              ; best_match_pos=pos
     add.w    d6,d0              ; optmatch=bestmatch
.nnmm0:
     move.l   a4,(a2)+           ; tp->parent=right
     move.l   a2,a4              ; right=&(tp->c_left)
     move.l   (a2),d7            ; tp=*right
     bne      .insert_loop       ; next!
     move.l   d7,(a3)            ; *left=0
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6
endif
     rts

.left:
     ; nkey>okey
     move.l   d7,(a3)            ; *left=tp
     move.w   a5,d7              ; eind positie
     sub.w    d4,d7              ; d7=(nkey+matchlen+1)-(nkey+1)=matchlen
     cmp.w    d6,d7              ; matchlen<best_match?
     ble.s    .nnmm1             ; yep, geen nieuwe best_match
     move.w   a1,d1              ; tp->key+matchlen
     move.w   d7,d6              ; best_match=matchlen
     not.w    d1                 ;
     move.w   d7,(a6)+           ; store match in hist array
     add.w    a5,d1              ; pos=nkey+matchlen-(tp->key+matchlen)-1
     move.w   d1,(a6)+           ; store pos in hist array
     sub.w    d0,d7              ; matchlen-optmatch
     bgt.s    .good_match1       ; nope
     move.l   fast_log-best_match_pos(a0),a1 ; log tabel
     beq.s    .diff_2_1          ; matchlen-optmatch==-2
     moveq    #MAXDELTA,d7       ; MAXDELTA
     sub.b    0(a1,d1.l),d7      ; -log(pos)
     add.b    0(a1,d5.l),d7      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     bgt.s    .good_match1       ; yep!
     bra.s    .nnmm1             ; anders geen nieuwe optmatch
.diff_2_1:
     moveq    #2*MAXDELTA,d7     ; MAXDELTA
     sub.b    0(a1,d1.l),d7      ; -log(pos)
     add.b    0(a1,d5.l),d7      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     ble.s    .nnmm1             ; anders geen nieuwe optmatch
.good_match1:
     moveq    #2,d0              ; offset
     move.l   d1,d5              ; best_match_pos=pos
     add.w    d6,d0              ; optmatch=bestmatch
.nnmm1:
     move.l   a3,(a2)            ; tp->parent=left
     lea      8(a2),a3           ; left=&(tp->c_right)
     move.l   (a3),d7            ; tp=*left
     bne      .insert_loop       ; next!
     move.l   d7,(a4)            ; *right=0
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6
endif
     rts

; Register gebruik:
; D0 = opt match
; D1 = tmp, d1.h is schoon
; D2 = nnode*16, command.max_match
; D3 = tmp
; D4 = nkey+MAXD_MATCH
; D5 = best_match_pos
; D6 = best_match-MAXD_MATCH
; D7 = tp
;
; A0 = command.best_match_pos
; A1 = tmp
; A2 = tp
; A3 = command.link
; A4 = command.tree
; A5 = tmp
; A6 = hist
; A7 = sp
.same:                           ; max match
     neg.w    d6
     move.l   a6,d4              ; save hist
     move.l   a1,-(a2)           ; store key+MAXD_MATCH
     addq.l   #8,a2              ; a2=tp->c_left
     move.l   (a2)+,d3           ; tp->c_left
     beq.s    .c_left_0          ; tp->c_left==0
     move.l   d3,a6              ; tp->c_left
     move.l   a3,4(a6)           ; (tp->c_left)->parent=left
.c_left_0:
     move.l   d3,(a3)            ; *left=tp->c_left

     move.l   (a2),d3            ; tp->c_right
     beq.s    .c_right_0         ; tp->c_right==0
     move.l   d3,a6              ; tp->c_right
     move.l   a4,4(a6)           ; (tp->c_right)->parent=right
.c_right_0:
     move.l   d3,(a4)            ; *right=tp->c_right
;linking
     move.l   link-best_match_pos(a0),a3 ; command.link
     lsr.l    #2,d2              ; nnode*4
     lea      0(a3,d2.l),a6      ; command.link+nnode
     move.l   d7,(a6)            ; command.link[nnode]=tp
     move.l   a6,-8(a2)          ; tp->parent=command.link+nnode
;end_linking
     move.w   max_match-best_match_pos(a0),d2 ; command.max_match
     move.l   tree-best_match_pos(a0),a4 ; command.tree
     move.l   d4,a6              ; restore hist
     move.l   a5,d4              ; nkey+MAXD_MATCH
     addq.w   #MAXD_MATCH-1,d6   ; corrigeer best_match
     subq.w   #MAXD_MATCH-1,d2   ; corrigeer max_match
     move.b   -1(a5,d2.w),d1     ; orig
     swap     d2                 ; orig opslag
     move.b   d1,d2              ; orig
     swap     d2                 ; restore max_amtch
     bra.s    .link_in           ; don't init link loop
.link_loop:
     move.l   d7,a2              ; tp
     move.l   d4,a5              ; nkey+MAXD_MATCH
     move.l   (a2)+,a1           ; tp->key
.link_in:
     move.b   -1(a1,d2.w),d1     ; kar
     move.w   d4,d3              ; compare waarde
     not.b    d1                 ; ~kar
     move.b   d1,-1(a5,d2.w)     ; sentinel
.loop:
     cmp.b    (a1)+,(a5)+        ; 2
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 3
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 4
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 5
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 6
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 7
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 8
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 9
     beq.s    .loop
.end_match1:
     sub.w    a5,d3              ; d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
     cmp.w    d3,d6              ; matchlen<best_match?
     ble.s    .nnmm2             ; yep, geen nieuwe best_match
     move.w   d3,d6              ; best_match=matchlen
     moveq    #MAXD_MATCH-1,d3   ; corrigeer offset
     move.w   a1,d1              ; tp->key+matchlen
     sub.w    d6,d3              ; match
     not.w    d1                 ;
     move.w   d3,(a6)+           ; store match in hist array
     add.w    a5,d1              ; pos=nkey+matchlen-(tp->key+matchlen)-1
     move.w   d1,(a6)+           ; store pos in hist array
     sub.w    d0,d3              ; matchlen-optmatch
     bgt.s    .good_match2       ; nope
     move.l   fast_log-best_match_pos(a0),a1 ; log tabel
     beq.s    .diff_2_2          ; matchlen-optmatch==-2
     moveq    #MAXDELTA,d3       ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     bgt.s    .good_match2       ; yep!
     bra.s    .ngm               ; anders geen nieuwe optmatch
.diff_2_2:
     moveq    #2*MAXDELTA,d3     ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     ble.s    .ngm               ; anders geen nieuwe optmatch
.good_match2:
     moveq    #MAXD_MATCH-1+2,d0 ; corrigeer offset
     sub.w    d6,d0              ; optmatch=bestmatch
     move.l   d1,d5              ; best_match_pos=pos
.ngm:
     move.w   d6,d3              ; restore d3
     add.w    d2,d3              ; best_match+max_match
     beq.s    .max_match         ; zijn aan elkaar gelijk? dan einde met max_match
.nnmm2:
     sub.l    a4,d7              ; tp-command.tree
     lsr.l    #2,d7              ; offset in link array
     move.l   0(a3,d7.l),d7      ; link[tp-command.tree]
     bne      .link_loop         ; verder linken
;end
.max_match:
     move.w   d2,d6              ; max_match
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     swap     d2                 ; orig
     move.l   d4,a6              ; restore nkey
     move.b   d2,-1(a6,d6.w)     ; restore orig
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6
endif
     rts

;.max_match:
; link de parent van deze node met de link van deze node
;end
;     sub.w    d0,d7              ; return opt_match
;     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
;     move.w   d2,d6              ; max_match
;     swap     d2                 ; orig
;     move.b   d2,-1(a6,d6.w)     ; restore orig
;if GNUC
;     movem.l  (sp)+,d2-d7/a2-a6
;else
;     movem.l  (sp)+,d3-d7/a2-a6
;endif
;     rts

endif

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

; Register gebruik:
; D0 = nkey                 |*rle_hash
; D1 = 0.l                  |*rle_size.w, d1.h = 0
; D2 = nnode, .h = 0        |*nnode, .h = 0
; D3 = (nkey)               |
; D4 =
; D5 =
; D6 =
; D7 = 0.l
;
; A0 = command.delta_hash   |*command.delta_hash
; A1 =                      |
; A2 =                      |
; A3 =                      |
; A4 =                      |
; A5 =                      |
; A6 = new_key+1            |*nkey
; A7 = SP                   |
innew_rle:
     move.b   d3,delta_hash+1-delta_hash(a0); store delta_hash
     move.l   d0,a1              ; key
     move.l   d0,a6              ; key
     move.w   max_match-delta_hash(a0),d5 ; max_match
     moveq    #0,d0              ; clear d0
     move.b   0(a6,d5.w),d1      ; orig
     move.b   d3,d0              ; rle_char
     not.b    d3                 ; ~rle_char
     move.b   d3,0(a6,d5.w)      ; sentry
.rle_loop:
     cmp.b    (a1)+,d0           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d0           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d0           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d0           ; compare
     beq.s    .rle_loop          ;
.rle_end:
     move.b   d1,0(a6,d5.w)      ; restore orig
     move.w   a6,d1              ; key
     not.w    d1                 ; -key-1
     add.w    a1,d1              ; rle_size=p-key-1
     lsl.w    #8,d0              ; rle_char<<8
     move.b   d1,d0              ; rle_hash
     move.l   d0,d5              ; rle_hash
     subq.w   #1,d5              ; rle_size--
     move.w   d5,rle_hash-delta_hash(a0) ; store rle_hash
     bra.s    insert2            ; insert

; Register gebruik:
; D0 = nkey                 |*rle_hash
; D1 = delta_hash           |*rle_size.w, d1.h = 0
; D2 = nnode, .h = 0        |*nnode, .h = 0
; D3 = rle_size.b           |
; D4 =
; D5 =
; D6 =
; D7 =
;
; A0 = command.delta_hash   |*command.delta_hash
; A1 =                      |
; A2 =                      |
; A3 =                      |
; A4 =                      |
; A5 =                      |
; A6 = new_key              |*nkey
; A7 = SP                   |
insert2_ai:
     addq.w   #1,d0              ; rle_size++
     addq.w   #1,d1              ; rle_size++
     bra.s    insert2            ; insert
inrle:
     moveq    #0,d0              ; clear d0
     move.w   rle_hash-delta_hash(a0),d0 ; rle_hash
     move.w   d1,d4              ; rle_char
     move.b   d0,d1              ; rle_size.w
     cmp.b    0(a6,d1.w),d4      ; rle_size==rle_max?
     beq.s    insert2_ai         ; yep
     subq.w   #1,d3              ; rle_size.w--
     move.b   d3,rle_hash+1-delta_hash(a0) ; store rle_hash

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

if 1

; Register gebruik:
; D0 = rle_hash                  |
; D1 = rle_size, d1.h=0          |
; D2 = nnode                     | nnode*16 -> sp            
; D3 =                           | tmp
; D4 =                           | .h: orig, .l: command.max_match
; D5 =                           | best_match_pos
; D6 =                           | .h: rle_size, .l: best_match
; D7 =                           | -opt_match
;
; A0 = command.delta_hash        | -> hist_index -> hist -> best_match_pos
; A1 =                           | tp
; A2 =                           | tmp
; A3 = command.tree              | left
; A4 =                           | right
; A5 =                           | tmp
; A6 = nkey                      | nkey
; A7 =                           | hist


insert2:
     lsl.l    #4,d2              ; nnode*16
     move.l   a6,a2              ; nkey
     move.l   root2-delta_hash(a0),a1 ; command.root2
     move.l   d1,d6              ; rle_size, d6.h = 0
     add.l    d2,a3              ; tp=command.tree+nnode
     lsl.l    #2,d0              ; hash*4
     add.w    d1,a6              ; nkey+rle_size
     move.l   a3,a5              ; tmp=command.tree+nnode
     move.l   a6,(a3)+           ; tp->key=nkey
     add.l    d0,a1              ; command.root+command.delta_hash
     move.l   a1,(a3)+           ; tp->parent=command.root+command.delta_hash
     move.l   (a1),d0            ; tp=command.root[command.delta_hash]
     beq      .s_insert2_end     ; einde insert
     lea      4(a3),a4           ; right=&(tp->c_right)
     move.l   d2,-4(sp)          ; store nnode * 16
     addq.l   #6,a0              ; a0 = command.hist_index
     moveq    #1,d2              ; clear d2, index++
     add.w    (a0),d2            ; history index
     moveq    #10,d5             ; shift size
     and.w    #HISTSIZE-1,d2     ; modulo
     move.w   d2,(a0)+           ; store
     lsl.w    d5,d2              ; offset in hist array
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
     add.l    (a0)+,d2           ; hist array
     move.b   (a2),d7            ; rle_char
     moveq    #0,d5              ; best_match_pos=0
     cmp.b    -3(a2),d7          ; nkey[-3]
     exg      d2,sp              ; hist array, need adress register
     beq.s    .rle               ; echte rle match gevonden
     move.l   d0,a2              ; tp
     move.w   2(a2),d5           ; tp->key
     not.w    d5                 ;
     add.w    a6,d5              ; pos=nkey-(tp->key)-1
.rle:
     move.w   max_match-best_match_pos(a0),d4 ; command.max_match
     move.w   d6,(sp)+           ; store match in hist array
     moveq    #0,d7              ; -opt_match=2
     move.w   d5,(sp)+           ; store best_max_pos in hist array
     sub.w    d6,d4              ; command.max_match-rle_size
     beq      .max_match0        ; meteen al een max_match gevonden
     swap     d6                 ; d6.h=rle_size
     move.b   0(a6,d4.w),d1      ; orig
     swap     d4                 ; orig opslag
     move.b   d1,d4              ; orig
     swap     d4                 ; max_match
.insert_loop:
     move.l   d0,a2              ; tp
     move.l   a6,a5              ; nkey
     move.l   (a2)+,a1           ; tp->key
     cmp.b    (a1)+,(a5)+        ; key's gelijk? 1
     beq      .match_cont        ;
; hier gegarandeerd geen max match
     bcc      .s_left            ; nkey>okey
     ; nkey<okey
     move.l   d0,(a4)            ; *right=tp
     move.l   a4,(a2)+           ; tp->parent=right
     move.l   a2,a4              ; right=&(tp->c_left)
     move.l   (a2),d0            ; tp=*right
     bne.s    .insert_loop       ; next!
     move.l   d0,(a3)            ; *left=0
     move.w   d4,d3              ; max_match
     swap     d6                 ; rle_size
     swap     d4                 ; orig
     sub.w    d7,d6              ; return opt_match
     move.b   d4,0(a6,d3.w)      ; restore orig
;.max_match0:
     move.w   d6,d0              ; +rle_size
     move.l   d2,sp              ; remove hist array
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     addq.w   #2,d0              ; match offset
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6  ;
endif
     rts
.max_match0:
     move.l   d7,(a3)            ; geen linker node
     move.l   d7,(a4)            ; geen rechter node
     move.w   d6,d0              ; +rle_size
     move.l   d2,sp              ; remove hist array
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     addq.w   #2,d0              ; match offset
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6  ;
endif
     rts

.s_insert2_end:                  ; voor als de tree leeg was
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
     move.l   d0,(a3)+           ; *left=0
     move.l   d0,(a3)            ; *right=0
     move.b   (a2),d7            ; rle_char
     cmp.b    -3(a2),d7          ; nkey[-3]
     bne.s    .no_rle            ; geen match gevonden
     move.w   d0,best_match_pos-delta_hash(a0) ; command.best_match_pos=best_match_pos
     moveq    #2,d0              ; match offset
     add.w    d6,d0              ; opt_match
.no_rle:
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6
endif
     rts

.s_left:
     ; nkey>okey
     move.l   d0,(a3)            ; *left=tp
     move.l   a3,(a2)            ; tp->parent=left
     lea      8(a2),a3           ; left=&(tp->c_right)
     move.l   (a3),d0            ; tp=*left
     bne      .insert_loop       ; next!
     move.l   d0,(a4)            ; *right=0
     swap     d6                 ; rle_size
     move.w   d6,d0              ; +rle_size
     sub.w    d7,d0              ; return opt_match
     addq.w   #2,d0              ; match offset
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     move.w   d4,d3              ; max_match
     move.l   d2,sp              ; remove hist array
     swap     d4                 ; orig
     move.b   d4,0(a6,d3.w)      ; restore orig
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6  ;
endif
     rts

.match_cont:
     move.b   -1(a1,d4.w),d1     ; kar
     move.w   a5,d3              ; nkey+1
     not.b    d1                 ; ~kar
     move.b   d1,-1(a5,d4.w)     ; sentinel
if MAXD_MATCH2>2
     cmp.b    (a1)+,(a5)+        ; 2
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>3
     cmp.b    (a1)+,(a5)+        ; 3
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>4
     cmp.b    (a1)+,(a5)+        ; 4
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>5
     cmp.b    (a1)+,(a5)+        ; 5
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>6
     cmp.b    (a1)+,(a5)+        ; 6
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>7
     cmp.b    (a1)+,(a5)+        ; 7
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>8
     cmp.b    (a1)+,(a5)+        ; 8
     bne.s    .end_match         ;
endif
     cmp.b    (a1)+,(a5)+        ; 9
     beq      .same
.end_match:
     bcc.s    .left              ; nkey>okey
     ; nkey<okey
     sub.w    a5,d3              ; d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
     cmp.w    d3,d6              ; matchlen<best_match?
     ble.s    .nnmm0             ; yep, geen nieuwe best_match
     move.w   d3,d6              ; best_match=matchlen
     move.w   a1,d1              ; tp->key+matchlen
     swap     d6                 ; rle_size
     not.w    d1                 ;
     sub.w    d3,d6              ; match-2
     add.w    a5,d1              ; pos=nkey+matchlen-(tp->key+matchlen)-1
     move.w   d6,(sp)+           ; store match in hist array
     add.w    d3,d6              ; herstel d6
     move.w   d1,(sp)+           ; store pos in hist array
     swap     d6                 ; d6 weer omdraaien
     sub.w    d7,d3              ; matchlen-optmatch
     addq.w   #2,d3              ; matchlen-optmatch>=-2?
     bmi.s    .good_match0       ; nope
     move.l   fast_log-best_match_pos(a0),a1 ; log tabel
     beq.s    .diff_2_0          ; matchlen-optmatch==-2
     moveq    #MAXDELTA,d3       ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     bgt.s    .good_match0       ; yep!
     bra.s    .nnmm0             ; anders geen nieuwe optmatch
.diff_2_0:
     moveq    #2*MAXDELTA,d3     ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     ble.s    .nnmm0             ; anders geen nieuwe optmatch
.good_match0:
     move.w   d6,d7              ; optmatch=bestmatch
     move.l   d1,d5              ; best_match_pos=pos
.nnmm0:
     move.l   d0,(a4)            ; *right=tp
     move.l   a4,(a2)+           ; tp->parent=right
     move.l   a2,a4              ; right=&(tp->c_left)
     move.l   (a2),d0            ; tp=*right
     bne      .insert_loop       ; next!
     move.l   d0,(a3)            ; *left=0
     swap     d6                 ; rle_size
     move.w   d6,d0              ; +rle_size
     sub.w    d7,d0              ; return opt_match
     addq.w   #2,d0              ; match offset
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     move.w   d4,d3              ; max_match
     move.l   d2,sp              ; remove hist array
     swap     d4                 ; orig
     move.b   d4,0(a6,d3.w)      ; restore orig
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6  ;
endif
     rts
.left:
     ; nkey>okey
     sub.w    a5,d3              ; d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
     cmp.w    d3,d6              ; matchlen<best_match?
     ble.s    .nnmm1             ; yep, geen nieuwe best_match
     move.w   d3,d6              ; best_match=matchlen
     move.w   a1,d1              ; tp->key+matchlen
     swap     d6                 ; rle_size
     not.w    d1                 ;
     sub.w    d3,d6              ; match-2
     add.w    a5,d1              ; pos=nkey+matchlen-(tp->key+matchlen)-1
     move.w   d6,(sp)+           ; store match in hist array
     add.w    d3,d6              ; herstel d6
     move.w   d1,(sp)+           ; store pos in hist array
     swap     d6                 ; d6 weer omdraaien
     sub.w    d7,d3              ; matchlen-optmatch
     addq.w   #2,d3              ; matchlen-optmatch>=-2?
     bmi.s    .good_match1       ; nope
     move.l   fast_log-best_match_pos(a0),a1 ; log tabel
     beq.s    .diff_2_1          ; matchlen-optmatch==-2
     moveq    #MAXDELTA,d3       ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     bgt.s    .good_match1       ; yep!
     bra.s    .nnmm1             ; anders geen nieuwe optmatch
.diff_2_1:
     moveq    #2*MAXDELTA,d3     ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     ble.s    .nnmm1             ; anders geen nieuwe optmatch
.good_match1:
     move.w   d6,d7              ; optmatch=bestmatch
     move.l   d1,d5              ; best_match_pos=pos
.nnmm1:
     move.l   d0,(a3)            ; *left=tp
     move.l   a3,(a2)            ; tp->parent=left
     lea      8(a2),a3           ; left=&(tp->c_right)
     move.l   (a3),d0            ; tp=*left
     bne      .insert_loop       ; next!
     move.l   d0,(a4)            ; *right=0
     swap     d6                 ; rle_size
     move.w   d6,d0              ; +rle_size
     sub.w    d7,d0              ; return opt_match
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     addq.w   #2,d0              ; match offset
     move.w   d4,d3              ; max_match
     move.l   d2,sp              ; remove hist array
     swap     d4                 ; orig
     move.b   d4,0(a6,d3.w)      ; restore orig
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6  ;
endif
     rts

; Register gebruik:
; D0 = tp
; D1 = tmp, d1.h is schoon
; D2 = sp
; D3 = tmp
; D4 = command.max_match
; D5 = best_match_pos
; D6 = .h: rle_size, .l: best_match-MAXD_MATCH2
; D7 = opt_match-MAXD_MATCH2
;
; A0 = command.best_match_pos
; A1 = tp
; A2 = tmp
; A3 = command.link
; A4 = command.tree
; A5 = tmp
; A6 = nkey+MAXD_MATCH2
; A7 = hist

.same:                           ; max match
     move.l   a1,-(a2)           ; strore key+MAXD_MATCH2
     addq.l   #8,a2              ; a2=tp->c_left
     move.l   (a2)+,d3           ; tp->c_left
     beq.s    .c_left_0          ; tp->c_left==0
     move.l   d3,a6              ; tp->c_left
     move.l   a3,4(a6)           ; (tp->c_left)->parent=left
.c_left_0:
     move.l   d3,(a3)            ; *left=tp->c_left

     move.l   (a2),d3            ; tp->c_right
     beq.s    .c_right_0         ; tp->c_right==0
     move.l   d3,a6              ; tp->c_right
     move.l   a4,4(a6)           ; (tp->c_right)->parent=right
.c_right_0:
     move.l   d2,a3              ; sp
     move.l   d3,(a4)            ; *right=tp->c_right
;linking
     move.l   -(a3),d3           ; nnode*16
     move.l   link-best_match_pos(a0),a3 ; command.link
     lsr.l    #2,d3              ; nnode*4
     lea      0(a3,d3.l),a6      ; command.link+nnode
     move.l   d0,(a6)            ; command.link[nnode]=tp
     move.l   a6,-8(a2)          ; tp->parent=command.link+nnode
;end_linking
     move.l   tree-best_match_pos(a0),a4 ; command.tree
     move.l   a5,a6              ; nkey+MAXD_MATCH2
     addq.w   #MAXD_MATCH2-1,d6  ; corrigeer best_match
     addq.w   #MAXD_MATCH2-1,d7  ; corrigeer opt_match
     swap     d6                 ;
     subq.w   #MAXD_MATCH2-1,d4  ; corrigeer max_match
     addq.w   #MAXD_MATCH2-1,d6  ; corrigeer rle_size
     swap     d6
     bra.s    .link_in           ; don't init link loop
.link_loop:
     move.l   d0,a2              ; tp
     move.l   a6,a5              ; nkey+MAXD_MATCH2
     move.l   (a2)+,a1           ; tp->key
.link_in:
     move.b   -1(a1,d4.w),d1     ; kar
     move.w   a6,d3              ; compare waarde
     not.b    d1                 ; ~kar
     move.b   d1,-1(a5,d4.w)     ; sentinel
.loop:
     cmp.b    (a1)+,(a5)+        ; 2
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 3
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 4
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 5
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 6
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 7
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 8
     bne.s    .end_match1        ;
     cmp.b    (a1)+,(a5)+        ; 9
     beq.s    .loop
.end_match1:
     sub.w    a5,d3              ; d3=(nkey+1)-(nkey+matchlen+1)=-matchlen
     cmp.w    d3,d6              ; matchlen<best_match?
     ble.s    .nnmm2             ; yep, geen nieuwe best_match
     move.w   d3,d6              ; best_match=matchlen
     move.w   a1,d1              ; tp->key+matchlen
     swap     d6                 ; rle_size
     not.w    d1                 ;
     sub.w    d3,d6              ; match-2
     add.w    a5,d1              ; pos=nkey+matchlen-(tp->key+matchlen)-1
     move.w   d6,(sp)+           ; store match in hist array
     add.w    d3,d6              ; herstel d6
     move.w   d1,(sp)+           ; store pos in hist array
     swap     d6                 ; d6 weer omdraaien
     sub.w    d7,d3              ; matchlen-optmatch
     addq.w   #2,d3              ; matchlen-optmatch>=-2?
     bmi.s    .good_match2       ; nope
     move.l   fast_log-best_match_pos(a0),a1 ; log tabel
     beq.s    .diff_2_2          ; matchlen-optmatch==-2
     moveq    #MAXDELTA,d3       ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     bgt.s    .good_match2       ; yep!
     bra.s    .ngm               ; anders geen nieuwe optmatch
.diff_2_2:
     moveq    #2*MAXDELTA,d3     ; MAXDELTA
     sub.b    0(a1,d1.l),d3      ; -log(pos)
     add.b    0(a1,d5.l),d3      ; MAXDELTA+log(best_match_pos)-log(pos)>0?
     ble.s    .ngm               ; anders geen nieuwe optmatch
.good_match2:
     move.w   d6,d7              ; optmatch=bestmatch
     move.l   d1,d5              ; best_match_pos=pos
.ngm:
     move.w   d6,d3              ; restore d3
     add.w    d4,d3              ; best_match+max_match
     beq.s    .max_match         ; zijn aan elkaar gelijk? dan einde met max_match
.nnmm2:
     sub.l    a4,d0              ; tp-command.tree
     lsr.l    #2,d0              ; offset in link array
     move.l   0(a3,d0.l),d0      ; link[tp-command.tree]
     bne      .link_loop         ; verder linken
;end
     moveq    #2,d0              ; corrigeer d7
     swap     d6                 ; rle_size
     add.w    d6,d0              ; +rle_size
     sub.w    d7,d0              ; return opt_match
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     move.w   d4,d3              ; max_match
     move.l   d2,sp              ; remove hist array
     swap     d4                 ; orig
     move.b   d4,-1(a6,d3.w)     ; restore orig
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6  ;
endif
     rts

.max_match:
; link de parent van deze node met de link van deze node
;end
     moveq    #2,d0              ; corrigeer d7
     swap     d6                 ; rle_size
     add.w    d6,d0              ; +rle_size
     sub.w    d7,d0              ; return opt_match
     move.w   d5,(a0)            ; command.best_match_pos=best_match_pos
     move.w   d4,d3              ; max_match
     swap     d4                 ; orig
     move.l   d2,sp              ; remove hist array
     move.b   d4,-1(a6,d3.w)     ; restore orig
if GNUC
     movem.l  (sp)+,d2-d7/a2-a6
else
     movem.l  (sp)+,d3-d7/a2-a6  ;
endif
     rts

endif

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

; Register gebruik:
; D0 = pos.l                     (ini)
; D1 = d1.h is schoon            (ini)
; D2 = d2.h is schoon            (ini)
; D3 = 0
; D4 =
; D5 = 
; D6 = delpos
; D7 = to_do
;
; A0 = command.tree, moving
; A1 = tree
; A2 =
; A3 =
; A4 =
; A5 =
; A6 =
; A7 = SP

multi:
     move.l   d1,d7              ; to_do
;eerst de nodes wissen
     move.l   (a0)+,a1           ; command.tree
     moveq    #0,d3              ; NO_NODE
     move.w   (a0),d2            ; delpos
.del_loop:
     addq.w   #1,d2              ; delpos++
     move.l   d2,d4              ; delpos
     lsl.l    #4,d4              ; delpos*16
     lea      0(a1,d4.l),a2      ; command.tree[delpos]
     move.l   4(a2),d4           ; tp=command.tree[delpos].parent
     beq.s    .no_fix            ; geen parent
     move.l   d4,a3              ; parent
     cmp.l    (a3),a2            ; *parent==command.tree[delpos]?
     bne.s    .no_fix            ; nope
     move.l   d3,(a3)            ; *parent=NO_NODE
.no_fix:
     dbra     d1,.del_loop       ; delete next
     move.w   d2,(a0)+           ; command.del_pos=del_pos
; Register gebruik:
; D0 = gebruikt door insertnm
; D1 = pos, d2.h is schoon
; D2 = pos, boven schoon
; D3 = hash, boven schoon
; D4 = command.tree
; D5 = 
; D6 = gebruikt door insertnm
; D7 = to_do
;
; A0 = command.dictionary -> command.delta_hash, blijft behouden
; A1 = gebruikt door insertnm
; A2 = gebruikt door insertnm
; A3 = gebruikt door insertnm
; A4 = gebruikt door insertnm
; A5 = gebruikt door insertnm
; A6 = nkey, blijft behouden
; A7 = SP
; nodes gedeleted, nu de insert no match
     move.l   a1,d4              ; d4=command.tree
     sub.l    d7,d0              ; pos-=to_do
     move.w   d0,d2              ; pos, bovenste helft is schoon
     add.l    (a0)+,d0           ; new_key
     move.l   d0,a6              ; new_key, blijft hier
     subq.w   #1,d7              ; d7 dbra compatible
     tst.b    rle_hash+1-delta_hash(a0); staat er nog een rle open?
     bne      rle                ; yep!
     move.w   (a0),d3            ; delta_hash

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
;*******************************************************************************

; Register gebruik:
; D0 = hash, later tp            ; init
; D1 = tmp, d1.h is schoon
; D2 = pos, boven schoon
; D3 =
; D4 = command.tree
; D5 =
; D6 = nnode*16                  ; init
; D7 = to_do
;
; A0 = command.delta_hash        ; init
; A1 = tp
; A2 = tmp
; A3 = left
; A4 = right
; A5 = tmp
; A6 = nkey                      ; init
; A7 = SP

insertnm_loop:
     move.b   (a6),d1            ; delta hash2
     eor.b    d1,d3              ; delta hash
     move.l   d3,d0              ; hash==0?
     beq      new_rle            ; yep!
     lsl.w    #8,d3              ; shift hash
     move.l   d2,d6              ; nnode
     move.b   d1,d3              ; delta hash 2

insertnm:
     lsl.l    #4,d6              ; nnode*16
     move.l   d4,a3              ; command.tree
     add.l    d6,a3              ; tp=command.tree+nnode
     lsl.l    #2,d0              ; hash*4
     move.l   a3,a5              ; tmp=command.tree+nnode
     move.l   root-delta_hash(a0),a1 ; command.root
     move.l   a6,(a3)+           ; tp->key=nkey
     add.l    d0,a1              ; command.root+command.delta_hash
     move.l   a1,(a3)+           ; tp->parent=command.root+command.delta_hash
     move.l   (a1),d0            ; tp=command.root[command.delta_hash]
     beq.s    .s_insert_nm_end   ; einde insert
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
     lea      4(a3),a4           ; right=&(tp->c_right)
.insert_nm_loop:
     move.l   d0,a2              ; tp
     move.l   a6,a5              ; nkey
     move.l   (a2)+,a1           ; tp->key
     cmp.b    (a1)+,(a5)+        ; key's gelijk? 1
     beq.s    .match_cont        ;
.end_match:
     bcc.s    .left              ; nkey>okey
     ; nkey<okey
     move.l   d0,(a4)            ; *right=tp
     move.l   a4,(a2)+           ; tp->parent=right
     move.l   a2,a4              ; right=&(tp->c_left)
     move.l   (a2),d0            ; tp=*right
     bne.s    .insert_nm_loop    ; next!
     move.l   d0,(a3)            ; *left=0
     addq.l   #1,a6              ; key++
     addq.w   #1,d2              ; pos++
     dbra     d7,insertnm_loop   ; next
     bra.s    insert_nm_end      ; insert
.s_insert_nm_end:                ; voor als de tree leeg was
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
     move.l   d0,(a3)+           ; *left=0
     move.l   d0,(a3)            ; *right=0
     addq.l   #1,a6              ; key++
     addq.w   #1,d2              ; pos++
     dbra     d7,insertnm_loop   ; next
     bra.s    insert_nm_end      ; insert
.left:
     ; nkey>okey
     move.l   d0,(a3)            ; *left=tp
     move.l   a3,(a2)            ; tp->parent=left
     lea      8(a2),a3           ; left=&(tp->c_right)
     move.l   (a3),d0            ; tp=*left
     bne.s    .insert_nm_loop    ; next!
     move.l   d0,(a4)            ; *right=0
     addq.l   #1,a6              ; key++
     addq.w   #1,d2              ; pos++
     dbra     d7,insertnm_loop   ; next
     bra.s    insert_nm_end      ; insert

.match_cont:
     cmp.b    (a1)+,(a5)+        ; 2
     bne.s    .end_match         ;
if MAXD_MATCH>2
     cmp.b    (a1)+,(a5)+        ; 3
     bne.s    .end_match         ;
endif
if MAXD_MATCH>3
     cmp.b    (a1)+,(a5)+        ; 4
     bne.s    .end_match         ;
endif
if MAXD_MATCH>4
     cmp.b    (a1)+,(a5)+        ; 5
     bne.s    .end_match         ;
endif
if MAXD_MATCH>5
     cmp.b    (a1)+,(a5)+        ; 6
     bne.s    .end_match         ;
endif
if MAXD_MATCH>6
     cmp.b    (a1)+,(a5)+        ; 7
     bne.s    .end_match         ;
endif
if MAXD_MATCH>7
     cmp.b    (a1)+,(a5)+        ; 8
     bne.s    .end_match         ;
endif
if MAXD_MATCH>8
     cmp.b    (a1)+,(a5)+        ; 9
     bne.s    .end_match
endif
.same:                           ; max match
     move.l   a1,-(a2)           ; store key-MAXD_MATCH
     addq.l   #8,a2              ; a2=tp->c_left
     move.l   (a2)+,d1           ; tp->c_left
     beq.s    .c_left_0          ; tp->c_left==0
     move.l   d1,a5              ; tp->c_left
     move.l   a3,4(a5)           ; (tp->c_left)->parent=left
.c_left_0:
     move.l   d1,(a3)            ; *left=tp->c_left
     move.l   (a2),d1            ; tp->c_right
     beq.s    .c_right_0         ; tp->c_right==0
     move.l   d1,a5              ; tp->c_right
     move.l   a4,4(a5)           ; (tp->c_right)->parent=right
.c_right_0:
     move.l   d1,(a4)            ; *right=tp->c_right
;linking
     move.l   link-delta_hash(a0),a5 ; command.link
     lsr.l    #2,d6              ; nnode*4
     add.l    d6,a5              ; command.link+nnode
     move.l   d0,(a5)            ; command.link[nnode]=tp
     move.l   a5,-8(a2)          ; tp->parent=command.link+nnode
;end_linking
     addq.l   #1,a6              ; key++
     addq.w   #1,d2              ; pos++
     dbra     d7,insertnm_loop   ; next
;     bra      insert_nm_end      ; insert

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

insert_nm_end:
     move.l   a6,d0              ; nkey
     move.b   (a6)+,d6           ; delta hash2
     eor.b    d6,d3              ; delta hash
     move.l   d3,d7              ; delta hash==0?
     beq.s    .new_rle           ; nope!
     lsl.w    #8,d3              ; shift hash
     moveq    #0,d1              ; clear d1.h nodig bij insert
     move.b   d6,d3              ; delta hash2
     move.w   d3,(a0)+           ; store delta_hash
     move.l   d4,a3              ; command.tree
     bra      insert             ; insert

.new_rle:
     subq.l   #1,a6              ; restore a6
     move.b   d6,d3              ; delta hash2
     move.w   d3,(a0)            ; store delta_hash
     move.l   a6,a1              ; key
     move.w   max_match-delta_hash(a0),d0   ; max_match
     move.b   0(a6,d0.w),d1      ; orig
     move.l   d3,d5              ; rle_char
     not.b    d6                 ; ~rle_char
     move.b   d6,0(a6,d0.w)      ; sentry
.rle_loop:
     cmp.b    (a1)+,d5           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d5           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d5           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d5           ; compare
     beq.s    .rle_loop          ;
.rle_end:
     move.b   d1,0(a6,d0.w)      ; restore orig
     move.w   a6,d1              ; key
     not.w    d1                 ; -key-1
     add.w    a1,d1              ; rle_size=p-key-1
     lsl.w    #8,d5              ; rle_char<<8
     move.b   d1,d5              ; rle_hash
     move.l   d5,d0              ; hash
     moveq    #0,d1              ; clear d1
     move.b   d5,d1              ; size
     subq.w   #1,d5              ; rle_size--
     move.w   d5,rle_hash-delta_hash(a0) ; store rle_hash
     move.l   d4,a3              ; command.tree
     bra      insert2            ; insert

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
;*******************************************************************************

new_rle:
     moveq    #0,d5              ; clear d5
     move.l   a6,a1              ; key
     move.b   d1,d5              ; rle_char
     move.w   max_match-delta_hash(a0),d0 ; max_match
     move.b   0(a6,d0.w),d6      ; orig
     not.b    d1                 ; ~rle_char
     move.b   d1,0(a6,d0.w)      ; sentry
.rle_loop:
     cmp.b    (a1)+,d5           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d5           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d5           ; compare
     bne.s    .rle_end           ;
     cmp.b    (a1)+,d5           ; compare
     beq.s    .rle_loop          ;
.rle_end:
     move.b   d6,0(a6,d0.w)      ; restore orig
     move.w   a6,d1              ; key
     not.w    d1                 ; -key-1
     add.w    a1,d1              ; rle_size=p-key-1
     lsl.w    #8,d5              ; rle_char<<8
     move.b   d1,d5              ; rle_hash
     move.l   d5,d0              ; hash
     move.l   d2,d6              ; nnode
     move.b   d5,d3              ; rle_size
     add.l    a6,d3              ; key+rle_size
     exg      d3,a6              ; swap regs
     bra.s    insert2nm          ; insert

rlemaxmatch:
     addq.l   #1,a6              ; rle_size++
     addq.w   #1,d0              ; rle_size++
     lsl.l    #4,d6              ; nnode*16
     add.l    d4,d6              ; tp=command.tree+nnode
     move.l   d6,a3              ; tp=command.tree+nnode
     move.l   a6,(a3)+           ; tp->key=nkey
     move.l   root2-delta_hash(a0),a1 ; command.root
     lsl.l    #2,d0              ; hash*4
     add.l    d0,a1              ; command.root+command.delta_hash
     move.l   d6,(a1)            ; command.root[command.delta_hash]=tp
     moveq    #0,d0              ; clear d0
     move.l   a1,(a3)+           ; tp->parent=command.root+command.delta_hash
     move.l   d0,(a3)+           ; *left=0
     move.l   d0,(a3)            ; *right=0
     addq.w   #1,d2              ; pos++
     dbra     d7,f_rle           ; next
;     move.b   d5,d0              ; rle_size
;     neg.w    d0                 ; -rle_size
;     lea      0(a6,d0.w),a6      ; restore a6
;     moveq    #0,d3              ; clear d3
     bra       insert2_nm_cont_end;

rle:
     moveq    #0,d5              ; clear d5
     move.w   rle_hash-delta_hash(a0),d5; rle_hash
     move.b   d3,rle_hash+1-delta_hash(a0); ga er van uit dat de rle wordt opgelost
     move.b   d5,d3              ; rle_size
     add.l    a6,d3              ; key+rle_size
     exg      d3,a6              ; swap regs
f_rle:
     move.l   d5,d0              ; hash
     move.l   d2,d6              ; nnode
     move.l   d3,a1              ; key
     move.b   (a6),d1            ; rle_char
     cmp.b    (a1),d1            ; rle_size==rle_max?
     beq.s    rlemaxmatch        ; yep

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

; Register gebruik:
; D0 = hash                      ; init
; D1 = tmp, d1.h is schoon
; D2 = 
; D3 =
; D4 = 
; D5 =
; D6 = nnode*16                  ; init
; D7 =
;
; A0 = command.delta_hash        ; init
; A1 = tp
; A2 = tmp
; A3 = left
; A4 = right
; A5 = tmp
; A6 = nkey                      ; init
; A7 = SP


insert2nm:
     lsl.l    #4,d6              ; nnode*16
     move.l   d4,a3              ; command.tree
     add.l    d6,a3              ; tp=command.tree+nnode
     move.l   a3,a5              ; tmp=command.tree+nnode
     move.l   a6,(a3)+           ; tp->key=nkey
     move.l   root2-delta_hash(a0),a1 ; command.root
     add.l    d0,d0              ; hash*2
     add.l    d0,d0              ; hash*4
     add.l    d0,a1              ; command.root+command.delta_hash
     move.l   a1,(a3)+           ; tp->parent=command.root+command.delta_hash
     move.l   (a1),d0            ; tp=command.root[command.delta_hash]
     beq.s    .s_insert2_nm_end  ; einde insert
     lea      4(a3),a4           ; right=&(tp->c_right)
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
.insert2_nm_loop:
     move.l   d0,a2              ; tp
     move.l   a6,a5              ; nkey
     move.l   (a2)+,a1           ; tp->key
     cmp.b    (a1)+,(a5)+        ; key's gelijk? 1
     beq.s    .match_cont        ;
.end_match:
     bcc.s    .left              ; nkey>okey
     ; nkey<okey
     move.l   d0,(a4)            ; *right=tp
     move.l   a4,(a2)+           ; tp->parent=right
     move.l   a2,a4              ; right=&(tp->c_left)
     move.l   (a2),d0            ; tp=*right
     bne.s    .insert2_nm_loop   ; next!
.s_insert_nm_end_2:
     move.l   d0,(a3)            ; *right=0
     addq.w   #1,d2              ; pos++
     subq.b   #1,d5              ; rle_hash--
     dbeq     d7,f_rle           ; next
     bne      insert2_nm_cont_end
     moveq    #0,d3              ; clear d3
     move.b   -1(a6),d3          ; vul d3 met rle char
     dbra     d7,insertnm_loop   ; next
     bra      insert_nm_end      ; insert
.s_insert2_nm_end:               ; voor als de tree leeg was
     move.l   a5,(a1)            ; command.root[command.delta_hash]=tmp
     move.l   d0,(a3)+           ; *left=0
     bra.s    .s_insert_nm_end_2
.left:
     ; nkey>okey
     move.l   d0,(a3)            ; *left=tp
     move.l   a3,(a2)            ; tp->parent=left
     lea      8(a2),a3           ; left=&(tp->c_right)
     move.l   (a3),d0            ; tp=*left
     bne.s    .insert2_nm_loop   ; next!
     move.l   d0,(a4)            ; *right=0
     addq.w   #1,d2              ; pos++
     subq.b   #1,d5              ; rle_hash--
     dbeq     d7,f_rle           ; next
     bne.s    insert2_nm_cont_end
     moveq    #0,d3              ; clear d3
     move.b   -1(a6),d3          ; vul d3 met rle char
     dbra     d7,insertnm_loop   ; next
     bra      insert_nm_end      ; insert

.match_cont:
     cmp.b    (a1)+,(a5)+        ; 2
     bne.s    .end_match         ;
if MAXD_MATCH2>2
     cmp.b    (a1)+,(a5)+        ; 3
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>3
     cmp.b    (a1)+,(a5)+        ; 4
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>4
     cmp.b    (a1)+,(a5)+        ; 5
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>5
     cmp.b    (a1)+,(a5)+        ; 6
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>6
     cmp.b    (a1)+,(a5)+        ; 7
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>7
     cmp.b    (a1)+,(a5)+        ; 8
     bne.s    .end_match         ;
endif
if MAXD_MATCH2>8
     cmp.b    (a1)+,(a5)+        ; 9
     bne.s    .end_match
endif
.same:                           ; max match
     move.l   a1,-(a2)           ; store key-MAXD_MATCH
     addq.l   #8,a2              ; a2=tp->c_left
     move.l   (a2)+,d1           ; tp->c_left
     beq.s    .c_left_0          ; tp->c_left==0
     move.l   d1,a5              ; tp->c_left
     move.l   a3,4(a5)           ; (tp->c_left)->parent=left
.c_left_0:
     move.l   d1,(a3)            ; *left=tp->c_left
     move.l   (a2),d1            ; tp->c_right
     beq.s    .c_right_0         ; tp->c_right==0
     move.l   d1,a5              ; tp->c_right
     move.l   a4,4(a5)           ; (tp->c_right)->parent=right
.c_right_0:
     move.l   d1,(a4)            ; *right=tp->c_right
;linking
     move.l   link-delta_hash(a0),a5 ; command.link
     lsr.l    #2,d6              ; nnode*4
     add.l    d6,a5              ; command.link+nnode
     move.l   d0,(a5)            ; command.link[nnode]=tp
     move.l   a5,-8(a2)          ; tp->parent=command.link+nnode
;end_linking
     addq.w   #1,d2              ; pos++
     subq.b   #1,d5              ; rle_hash--
     dbeq     d7,f_rle           ; next
     bne.s    insert2_nm_cont_end
     moveq    #0,d3              ; clear d3
     move.b   -1(a6),d3          ; vul d3 met rle char
     dbra     d7,insertnm_loop   ; next
     bra      insert_nm_end      ; insert

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

insert2_nm_cont_end:
     move.l   a6,a1              ; key+rle_size
     moveq    #0,d1              ; clear d1
     move.l   d5,d0              ; hash
     move.b   d5,d1              ; rle_size
     moveq    #0,d3              ; clear d3
     sub.w    d1,a6              ; key
     subq.w   #1,d5              ; rle_size--
     move.b   (a6),d3            ; rle_char
     move.w   d5,rle_hash-delta_hash(a0) ; store rle_hash
     move.w   d3,(a0)            ; store delta_hash
     move.l   d4,a3              ; command.tree
     cmp.b    (a1),d3            ; rle_size==rle_max?
     bne      insert2            ; nope
     addq.w   #1,d5              ; rle_size++
     addq.w   #1,d1              ; rle_size++
     addq.w   #1,d0
     move.w   d5,rle_hash-delta_hash(a0) ; store rle_hash
     bra      insert2            ; insert

endif

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp, .s,rts
;*******************************************************************************

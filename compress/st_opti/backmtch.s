;/*
;// backmatch functie
;// pos: positie in dictionary waar backmatch van berekend moet worden
;// ptr: pointer van de match
;// hist: index in hist array, -1 -> hist array is niet meer geldig
;// com: packstruct, functie maakt gebruik van packstruct data 
;// en kan deze ook wijzigen
;// match is al opgeslagen -> charp[-1]=match
;//
;// return: lengte waarover backmatch correctie is uitgevoerd
;*/
;int backmatch_big(
; D0: long pos, 
; D1: uint16 ptr, 
; D2: int hist, 
; A0: packstruct* com)


   EXPORT backmatch_big

include "command.mac"

DIC_SIZE equ $20000

   EVEN

; register gebruik:
; D0: pos
; D1
; D2
; D3
; D4
; D5
; D6
; D7
; A0: com
; A1: charp-1
; A2
; A3
; A4
; A5
; A6
; A7: SP

   TEXT

backmatch_big:
;if(com->charp[-2]>=NLIT)
   move.l    charp(a0),a1        ; charp
   tst.b     -4(a1)              ; is er een bit in het eerste byte gezet?
   bne.s     .cont               ; yep, we kunnen verder
   moveq     #0,d0               ; return 0
;  *com->bmp++=0;
   move.l    bmp(a0),a1          ; bmp
   move.b    d0,(a1)+            ; *bmp++ = 0
   move.l    a1,bmp(a0)          ; bmp
   rts                           ; done
.cont:
   movem.l   a2-a4/d3-d5,-(sp)   ; save registers
; int len=com->max_match-com->charp[-1]+NLIT-MIN_MATCH+2; /* +2 voor max_match correctie */
   moveq     #0,d3               ;
   move.w    max_match(a0),d3    ; max_match
   sub.w     -(a1),d3            ; charp[-1]
   add.w     #255,d3             ; NLIT-MIN_MATCH+2
; uint8 *pp=com->dictionary+ --pos;
   subq.l    #1,d0               ; --pos
   move.l    dictionary(a0),a4   ; dictionary
   move.l    a4,a2               ; dictionary
   add.l     d0,a4               ; pp
; if((pos-=ptr)<0)
   moveq     #0,d4
   move.w    d1,d4               ; ptr
   move.l    d0,d1               ; pos
   sub.l     d4,d1               ; pos-=ptr
   bpl.s     .pos_greq_ptr
; if(pp-len<com->dictionary)
   move.l    a4,a3               ; pp
   sub.l     d3,a3               ; pp-len
   cmp.l     a2,a3               ;
; qq=com->dictionary+pos-1+DIC_SIZE;
   add.l     d1,a2               ; pos
   add.l     #DIC_SIZE-1,a2      ; DIC_SIZE-1
   bcc.s     .pos_ptr_done       ; no fix
; pp+=DIC_SIZE;
   add.l     #DIC_SIZE,a4        ; pp+=DIC_SIZE
   bra.s     .pos_ptr_done       ; no fix
.pos_greq_ptr:
; qq=com->dictionary+pos-1;
   lea       -1(a2,d1.l),a2      ; +pos-1
; if(pos<len)
   cmp.l     d3,d1               ;
   bge.s     .pos_ptr_done
; len=(int)pos;
   move.w    d1,d3               ;
.pos_ptr_done:
; if(len>0)
   tst.w     d3                  ; len>0?
   ble.s     .einde              ; nope, exit
; if(*pp==*qq)
   move.b    (a4),d1             ;
   cmp.b     (a2),d1             ;
   beq.s     .backmatch          ; backmatch gevonden
.einde:
   movem.l   (sp)+,d3-d5/a2-a4   ; restore registers
   moveq     #0,d0               ; return 0
;  *com->bmp++=0;
   move.l    bmp(a0),a1          ; bmp
   move.b    d0,(a1)+            ; *bmp++ = 0
   move.l    a1,bmp(a0)          ; bmp
   rts                           ; done
; backmatch gevonden
.backmatch:
; len=-len;
   neg.w     d3                  ; -len
; uint8* origp=qq;
   lea       0(a2,d3.w),a3       ; origp=qq
; uint8 orig=qq[len];
   move.b    (a3),d4             ; orig
; qq[len]=~pp[len]; /* sentinel */
   move.b    0(a4,d3.w),d1       ;
   not.b     d1                  ; sentinel
   move.b    d1,(a3)             ;
   move.l    a2,d1               ; origp
; while(*--pp==*--qq);
.loop1:
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   bne.s     .loop1_end
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   bne.s     .loop1_end
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   bne.s     .loop1_end
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   bne.s     .loop1_end
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   bne.s     .loop1_end
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   bne.s     .loop1_end
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   bne.s     .loop1_end
   move.b    -(a4),d3
   cmp.b     -(a2),d3
   beq.s     .loop1
.loop1_end:
; origp[len]=orig;
   move.b    d4,(a3)             ; restore orig
; len=(int)(origp-qq);
   sub.l     a2,d1               ; len, maakt d1.h schoon
; if(hist>=0)
   tst.w     d2                  ;
   bge.s     .hist
.exit_len:
   movem.l   (sp)+,d3-d5/a2-a4   ; restore registers
   moveq     #0,d0               ; return 0
;  *com->bmp++=len;
   move.l    bmp(a0),a1          ; bmp
   move.b    d1,(a1)+            ; *bmp++ = len
   move.l    a1,bmp(a0)          ; bmp
   rts                           ; done
.hist:
; hist_struct *histp=com->hist[hist];
   move.l    (a0),a4             ; fast_log
   moveq     #10,d5              ; d5.h is schoon
   lsl.w     d5,d2               ; offset in hist array
   move.l    hist(a0),a3         ; hist array
   add.w     d2,a3               ; histp
; uint8 ptrlen=LOG(com->ptrp[-2]);
   move.l    ptrp(a0),a2         ; ptrp
   subq.l    #4,a2               ; ptrp-2
   move.w    (a2),d5             ; ptr
   move.b    0(a4,d5.l),d2       ; ptrlen
; int origlen=com->charp[-2]-NLIT+MIN_MATCH-2; /* correctie voor +2 in encode_big */
   move.w    -(a1),d3            ;
   sub.w     #$ff,d3             ; -NLIT+MIN_MATCH-2
; register gebruik:
; D0: pos+ptr
; D1: len
; D2: ptrlen
; D3: origlen
; D4: histp->match, d4.h is schoon
; D5: LOG(histp->ptr), d5.h is schoon
; D6:
; D7
; A0: com
; A1: charp-2
; A2: ptrp-2
; A3: histp
; A4: fastlog
; A5
; A6
; A7: SP
; while(LOG(histp->ptr)<ptrlen)
.hist_loop:
   move.w    d3,d4               ; origlen
   sub.w     (a3)+,d4            ; origlen-histp->match
   move.w    (a3)+,d5            ; histp->ptr
   cmp.b     0(a4,d5.l),d2       ; LOG(histp->ptr)<ptrlen
   ble.s     .exit_len           ; nope -> exit
; if((origlen-histp->match) <=len)
   cmp.w     d4,d1               ; origlen-histp->match < len?
   blt.s     .hist_loop          ; nope
; back macth!
   move.b    0(a4,d5.l),d2       ; ptrlen=LOG(histp->ptr);
   subq.l    #2,a3               ; fix len
; while(LOG(histp[1].ptr)==ptrlen) histp++ /* neem langste match, meeste lengteoptimalisatie mogelijkheden */
.ptrlen_loop:
   addq.l    #4,a3               ; histp++
   move.w    (a3),d4             ; histp->ptr
   cmp.b     0(a4,d4.l),d2       ; LOG(histp)==ptrlen?
   beq.s     .ptrlen_loop        ; yep!
; register gebruik:
; D0: pos+ptr
; D1: len
; D2: ptrlen
; D3: origlen
; D4: ptrp[-2], d4.h is schoon
; D5: vrij, d5.h is schoon
; D6:
; D7
; A0: com
; A1: charp-2
; A2: ptrp-2
; A3: histp -> bmp
; A4: tmp
; A5
; A6
; A7: SP
; diff=(c_codetype)(origlen-histp->match);
; pos-=origlen+2;
   subq.l    #6,a3               ; fix a3
   sub.l     d3,d0               ; pos-=origlen
   sub.w     (a3)+,d3            ; diff = origlen - histp->match
; com->charp[-1]+=diff;
   add.w     d3,2(a1)            ; com->charp[-1]+=diff
; com->charp[-2]-=diff;
   sub.w     d3,(a1)             ; com->charp[-2]-=diff
; len-=diff;
   sub.w     d3,d1               ; len -=diff
; com->ptrp[-2]=histp->ptr;
   move.w    (a3),d4             ; ptrp[-2], a3 is nu vrij
   move.w    d4,(a2)             ; a2 is nu vrij
; *com->bmp++=(uint8)len;
   move.l    bmp(a0),a4          ; bmp
   move.l    a4,a3               ; bmp
   move.b    d1,(a4)+            ; *bmp++ = len
   move.l    a4,bmp(a0)          ; bmp
; register gebruik:
; D0: pos+ptr
; D1: len
; D2: ptrlen
; D3: diff
; D4: ptrp[-2], d4.h is schoon
; D5: vrij, d5.h is schoon
; D6:
; D7
; A0: com
; A1: charp-2
; A2: tmp
; A3: bmp
; A4: tmp
; A5
; A6
; A7: SP
; if(com->charp[-3]>=NLIT)
   tst.b     -2(a1)              ; com->charp[-3]>= NLIT?
   beq       .hbm_e              ; nope, schluss
; { /* herbackmatch gewijzigde pointer */
; pos-=origlen+2;
   subq.l    #2,d0               ; pos-=+2
; len=com->max_match-com->charp[-2]+NLIT-MIN_MATCH+2; /* +2 voor max_match correctie */
   move.w    max_match(a0),d1    ; max_match
   sub.w     (a1),d1             ; charp[-2]
   add.w     #255,d1             ; NLIT-MIN_MATCH+2
; pp=com->dictionary+pos;
   move.l    dictionary(a0),a4   ; dictionary, a0 is nu vrij
   move.l    a4,a2               ; dictionary
   add.l     d0,a4               ; pp
; if((pos-=com->ptrp[-2])<0)
   sub.l     d4,d0               ; pos-=ptr
   bpl.s     .hpos_greq_ptr
; if(pp-len<com->dictionary)
   move.l    a4,a0               ; pp
   sub.l     d1,a0               ; pp-len
   cmp.l     a2,a0               ;
; qq=com->dictionary+pos-1+DIC_SIZE;
   add.l     d0,a2               ; pos
   add.l     #DIC_SIZE-1,a2      ; DIC_SIZE-1
   bcc.s     .hpos_ptr_done      ; no fix
; pp+=DIC_SIZE;
   add.l     #DIC_SIZE,a4        ; pp+=DIC_SIZE
   bra.s     .hpos_ptr_done      ; no fix
.hpos_greq_ptr:
; qq=com->dictionary+pos-1;
   lea       -1(a2,d0.l),a2      ; qq=com->dictionary+pos-1
; if(pos<len)
   cmp.l     d1,d0               ;
   bge.s     .hpos_ptr_done
; len=(int)pos;
   move.w    d0,d1               ; d0 is nu vrij
.hpos_ptr_done:
   moveq     #0,d0
; if(len>0)
   tst.w     d1                  ; len>0?
   ble.s     .hbmf_e             ; nope, exit
; if(*pp==*qq)
   move.b    (a4),d2
   cmp.b     (a2),d2             ;
   bne.s     .hbmf_e             ; geen backmatch gevonden
; len=-len;
   neg.w     d1                  ; -len
; uint8* origp=qq;
   lea       0(a2,d1.w),a0       ; origp=qq
; uint8 orig=qq[len];
   move.b    (a0),d2             ; orig
; qq[len]=~pp[len]; /* sentinel */
   move.b    0(a4,d1.w),d0       ;
   not.b     d0                  ; sentinel
   move.b    d0,(a0)             ;
   move.w    a2,d0               ; origp
; while(*--pp==*--qq);
.hloop1:
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   bne.s     .hloop1_end
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   bne.s     .hloop1_end
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   bne.s     .hloop1_end
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   bne.s     .hloop1_end
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   bne.s     .hloop1_end
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   bne.s     .hloop1_end
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   bne.s     .hloop1_end
   move.b    -(a4),d1
   cmp.b     -(a2),d1
   beq.s     .hloop1
.hloop1_end:
; origp[len]=orig;
   move.b    d2,(a0)             ; restore sentinel
; len=(int)(origp-qq);
   sub.w     a2,d0               ; len
; com->bmp[-2]=bml;
.hbmf_e:
   move.b    d0,-(a3)            ; com->bmp[-2]=bml
; return diff;
.hbm_e:
   move.w    d3,d0               ; d0.h = 0 -> geen problemen met gcc
   movem.l   (sp)+,d3-d5/a2-a4   ; restore registers
   rts                           ; done

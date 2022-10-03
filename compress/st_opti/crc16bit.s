;
; ultra high speed 16 bit crc table
;

                EXPORT make_crc32table
                EXPORT crc32
                EXPORT make_crc16table
                EXPORT crc16
include "command.mac"

; The following exports should be after "include command.mac"

if GNUC
                EXPORT _make_crc32table
                EXPORT _crc32
                EXPORT _make_crc16table
                EXPORT _crc16
endif

                EVEN

                TEXT

;************************* ARJ CRC routs ************************************

; High speed 16 bits at a time crc routs
;void make_crctable(uint32 *crc_table)

if GNUC
_make_crc32table:
make_crc32table:
     move.l  8(sp),a0
     move.l  4(sp),d0
     movem.l d2-d4/a2,-(sp)
else
make_crc32table:
     movem.l d3-d4/a2,-(sp)
endif
     move.l  d0,d2
     moveq   #0,d0
     lea     256*4(a0),a1       ; crctable_end
     subq.b  #1,d0
     move.l  a1,a0
.loop_1:
     move.l  d0,d1
     lsr.l   #1,d1
     bcc.s   .next0
     eor.l   d2,d1
.next0:
     lsr.l   #1,d1
     bcc.s   .next1
     eor.l   d2,d1
.next1:
     lsr.l   #1,d1
     bcc.s   .next2
     eor.l   d2,d1
.next2:
     lsr.l   #1,d1
     bcc.s   .next3
     eor.l   d2,d1
.next3:
     lsr.l   #1,d1
     bcc.s   .next4
     eor.l   d2,d1
.next4:
     lsr.l   #1,d1
     bcc.s   .next5
     eor.l   d2,d1
.next5:
     lsr.l   #1,d1
     bcc.s   .next6
     eor.l   d2,d1
.next6:
     lsr.l   #1,d1
     bcc.s   .next7
     eor.l   d2,d1
.next7:
     move.l  d1,-(a0)
     dbra    d0,.loop_1
     move.l  a1,a2
     add.l   #$40000,a2         ; big table end
     move.l  #255,d0            ; count 1
.loop_2:
     move.l  #255,d1            ; count 2
     move.l  -(a1),d2           ; crc1
     move.b  d2,d3              ; need d2.low
     lsr.l   #8,d2              ; schuif
.loop_3:
     move.l  d1,d4              ; new char
     eor.b   d3,d4              ; lower part
     add.w   d4,d4
     add.w   d4,d4
     move.l  0(a0,d4.w),d4
     eor.l   d2,d4              ; new crc
     ror.w   #8,d4
     swap    d4
     ror.w   #8,d4
     swap    d4
     move.l  d4,-(a2)
     dbra    d1,.loop_3
     dbra    d0,.loop_2
if GNUC
     movem.l (sp)+,d2-d4/a2
else
     movem.l (sp)+,d3-d4/a2
endif
     rts
     
;********************************************************************************

; ulong crc_buf(char *str, ulong len, long crc, unsigned long *crc_table)
;
; CALL:
; D0 = #bytes (long)
; D1 = crc-code
; A0 = buffer
; A1 = crc table
;
; Return:
; D0 = CRC-code
;
; we do damage/change (whatever tastes best ;-)): D0-D2/A0-A1
if GNUC
_crc32:
crc32:
     move.l 4(sp),a0             ;*str
     move.l 8(sp),d0             ;len
     move.l 12(sp),d1            ;crc
     move.l 16(sp),a1            ;crc_table
     move.l d2,-(sp)
else
crc32:
endif
     move.l  a0,d2               ; buffer
     and.w   #1,d2               ; even?
     beq.s   .even               ; yep
     moveq   #0,d2               ;
     move.b  (a0)+,d2            ; eerste byte
     eor.b   d1,d2               ;
     add.w   d2,d2
     lsr.l   #8,d1
     add.w   d2,d2
     move.l  (a1,d2.w),d2
     eor.l   d2,d1               ; crc voor eerste byte gedaan
     subq.l  #1,d0
     beq.s   .einde
.even:
     subq.l  #2,d0               ;
     bcs.s   .length_is_one
     lsr.l   #1,d0               ; count/2
     bcc.s   .crc_word
     bsr.s   .crc_sub
.length_is_one:
     moveq   #0,d2
     move.b  (a0)+,d2
     eor.b   d1,d2
     lsr.l   #8,d1
     add.w   d2,d2
     add.w   d2,d2
     move.l  (a1,d2.w),d2
     eor.l   d2,d1
.einde:
     move.l  d1,d0
if GNUC
     move.l  (sp)+,d2
endif
     rts

.crc_sub:
if GNUC
     move.l  d2,-(sp)
endif
.crc_word:
     swap    d1
     move.l  a2,-(sp)
     ror.w   #8,d1
     swap    d1
     lea     256*4(a1),a2
     ror.w   #8,d1
     move.l  d1,d2
     lsr.l   #1,d0
     bcc.s   .oneven
.crc_loop:
     moveq   #0,d2
     move.w  (a0)+,d2
     eor.w   d1,d2
     add.l   d2,d2
     swap    d1
     add.l   d2,d2
     move.l  0(a2,d2.l),d2
     eor.w   d1,d2
.oneven:
     moveq   #0,d1
     move.w  (a0)+,d1
     eor.w   d2,d1
     add.l   d1,d1
     swap    d2
     add.l   d1,d1
     move.l  0(a2,d1.l),d1
     eor.w   d2,d1
     dbra    d0,.crc_loop
     addq.w  #1,d0
     subq.l  #1,d0
     bcc.s   .crc_loop
     swap    d1
     ror.w   #8,d1
     move.l  (sp)+,a2
     swap    d1
     ror.w   #8,d1
     move.l  d1,d0
if GNUC
     move.l  (sp)+,d2
endif
     rts


;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
;*******************************************************************************
;****************************************************************************
;*******************         LHA CRC routs             **********************
;****************************************************************************

; High speed 16 bits at a time crc routs
;void make_crctable(uint16 *crc_table)

if GNUC
_make_crc16table:
make_crc16table:
     move.l  8(sp),a0
     move.w  4(sp),d0
     movem.l d2-d4/a2,-(sp)
else
make_crc16table:
     movem.l d3-d4/a2,-(sp)
endif
     move.w  d0,d2
     moveq   #0,d0
     lea     256*2(a0),a1       ; crctable_end
     subq.b  #1,d0
     move.l  a1,a0
.loop_1:
     move.w  d0,d1
     lsr.w   #1,d1
     bcc.s   .next0
     eor.w   d2,d1
.next0:
     lsr.w   #1,d1
     bcc.s   .next1
     eor.w   d2,d1
.next1:
     lsr.w   #1,d1
     bcc.s   .next2
     eor.w   d2,d1
.next2:
     lsr.w   #1,d1
     bcc.s   .next3
     eor.w   d2,d1
.next3:
     lsr.w   #1,d1
     bcc.s   .next4
     eor.w   d2,d1
.next4:
     lsr.w   #1,d1
     bcc.s   .next5
     eor.w   d2,d1
.next5:
     lsr.w   #1,d1
     bcc.s   .next6
     eor.w   d2,d1
.next6:
     lsr.w   #1,d1
     bcc.s   .next7
     eor.w   d2,d1
.next7:
     move.w  d1,-(a0)
     dbra    d0,.loop_1
     move.l  a1,a2
     add.l   #$20000,a2         ; big table end
     move.l  #255,d0            ; count 1
.loop_2:
     move.l  #255,d1            ; count 2
     move.w  -(a1),d2           ; crc1
     move.b  d2,d3              ; need d2.low
     lsr.w   #8,d2              ; schuif
.loop_3:
     move.l  d1,d4              ; new char
     eor.b   d3,d4              ; lower part
     add.w   d4,d4
     move.w  0(a0,d4.w),d4
     eor.w   d2,d4              ; new crc
     ror.w   #8,d4
     move.w  d4,-(a2)
     dbra    d1,.loop_3
     dbra    d0,.loop_2
if GNUC
     movem.l (sp)+,d2-d4/a2
else
     movem.l (sp)+,d3-d4/a2
endif
     rts
     
;********************************************************************************

; ulong crc_buf(char *str, ulong len, long crc, unsigned long *crc_table)
;
; CALL:
; D0 = #bytes (long)
; D1 = crc-code
; A0 = buffer
; A1 = crc table
;
; Return:
; D0 = CRC-code
;
; we do damage/change (whatever tastes best ;-)): D0-D2/A0-A1
if GNUC
_crc16:
crc16:
     move.l 4(sp),a0             ;*str
     move.l 8(sp),d0             ;len
     move.l 12(sp),d1            ;crc
     move.l 16(sp),a1            ;crc_table
     move.l d2,-(sp)
else
crc16:
endif
     move.l  a0,d2               ; buffer
     and.w   #1,d2               ; even?
     beq.s   .even               ; yep
     moveq   #0,d2               ;
     move.b  (a0)+,d2            ; eerste byte
     eor.b   d1,d2               ;
     add.w   d2,d2
     lsr.w   #8,d1
     move.w  (a1,d2.w),d2
     eor.w   d2,d1               ; crc voor eerste byte gedaan
     subq.l  #1,d0
     beq.s   .einde
.even:
     subq.l  #2,d0               ;
     bcs.s   .length_is_one
     lsr.l   #1,d0               ; count/2
     bcc.s   .crc_word
     bsr.s   .crc_sub
.length_is_one:
     moveq   #0,d2
     move.b  (a0)+,d2
     eor.b   d1,d2
     lsr.w   #8,d1
     add.w   d2,d2
     move.w  (a1,d2.w),d2
     eor.w   d2,d1
.einde:
     move.l  d1,d0
if GNUC
     move.l  (sp)+,d2
endif
     rts

.crc_sub:
if GNUC
     move.l  d2,-(sp)
endif
.crc_word:
     move.l  a2,-(sp)
     ror.w   #8,d1
     lea     256*2(a1),a2
     lsr.l   #1,d0
     bcc.s   .oneven
.crc_loop:
     moveq   #0,d2
     move.w  (a0)+,d2
     eor.w   d1,d2
     add.l   d2,d2
     move.w  0(a2,d2.l),d1
.oneven:
     moveq   #0,d2
     move.w  (a0)+,d2
     eor.w   d1,d2
     add.l   d2,d2
     move.w  0(a2,d2.l),d1
     dbra    d0,.crc_loop
     addq.w  #1,d0
     subq.l  #1,d0
     bcc.s   .crc_loop
     ror.w   #8,d1
     move.l  (sp)+,a2
     move.l  d1,d0
if GNUC
     move.l  (sp)+,d2
endif
     rts

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp

;********************************************************************************

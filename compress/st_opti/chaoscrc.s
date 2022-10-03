;
; chaos crc routines
;
; 1999-06-26
; HWS: Fixed wrong conditional branch in arj_crc
; 1997-11-04
; HWS: Fixed routines for odd adresses
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

; nieuwe KAOS crc routines
;void make_crctable(uint32 crc_poly, unsigned long *crc_table)

if GNUC
_make_crc32table:
make_crc32table:
     move.l  8(sp),a0
     move.l  4(sp),d0
     move.l  d2,-(sp)
else
make_crc32table:
endif
     lea     1024(a0),a0         ; crc end, build normal crc table here
     move.l  d0,d2
     moveq   #0,d0
     subq.b  #1,d0
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
     move.w  #$ff,d0             ; Make special CRC's
     move.l  a2,-(sp)            ; save A2
                                 ; a0 = crc_table
     lea     1024(a0),a1         ; 4 * 256 bytes verder chaos tabel 1 
     lea     2048(a0),a2         ; 2*4*256 bytes verder chaos tabel 2
    
.crc_ch_xloop:
     move.l  (a0)+,d1            ; Get normal value
     rol.l   #8,d1               ; Create first value
     move.l  d1,(a1)+            ;
     rol.l   #8,d1               ; Create second value
     move.l  d1,(a2)+
     dbra    d0,.crc_ch_xloop
     move.l  (sp)+,a2
if GNUC
     move.l  (sp)+,d2
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
     move.w  a0,d2               ; need low bit of a0
     and.w   #1,d2               ; even?
     beq.s   .continue           ; yep!
     moveq   #0,d2               ;
     move.b  (a0)+,d2            ; eerste byte
     eor.b   d1,d2               ;
     lsr.l   #8,d1
     add.w   d2,d2
     add.w   d2,d2
     move.l  0(a1,d2.w),d2
     eor.l   d2,d1               ; crc voor eerste byte gedaan
     subq.l  #1,d0               ; dec count
     beq.s   .length_is_one      ; fast end
.continue:
     moveq   #0,d2               ;
     move.b  (a0),d2             ; eerste byte
     eor.b   d1,d2               ;
     lsr.l   #8,d1
     add.w   d2,d2
     add.w   d2,d2
     move.l  0(a1,d2.w),d2
     eor.l   d2,d1               ; crc voor eerste byte gedaan
     ; check on 2-byte & 1-byte lengths!
     subq.l  #1,d0
     beq.s   .length_is_one
     subq.l  #1,d0
     beq.s   .length_is_two
     lsr.l   #1,d0
     bcs.s   .chaos_doit_jump
     subq.l  #1,d0
     bsr.s   .chaos_doit_sub
.length_is_two:
     moveq   #0,d2
     move.b  1(a0),d2
     eor.b   d1,d2
     lsr.l   #8,d1
     add.w   d2,d2
     add.w   d2,d2
     move.l  0(a1,d2.w),d2
     eor.l   d2,d1
.length_is_one:   
     move.l  d1,d0
if GNUC
     move.l  (sp)+,d2
endif
     rts


.chaos_doit_sub:
if GNUC
     move.l d2,-(sp)
endif
.chaos_doit_jump:
; MUST be called with AT LEAST 1 word to do CRC over...
     movem.l d3-d5/a2-a3,-(sp)
     lea     1024(a1),a3         ; chaos tabel 1
     lea     2048(a1),a2         ; chaos tabel 2
     move.w  #$00ff,d4           ; and mask
     move.w  (a0)+,d5            ;
     and.w   d4,d5               ; bovenste helft d5 leeg
.chdi_mainloop:
     move.w  d5,d3               ; onderste byte to be crc'ed
     eor.b   d1,d3               ; xor met crc som
     add.w   d3,d3               ; offset in crc tabel
     add.w   d3,d3               ;
     clr.b   d1                  ; clear byte (shr 8...)
     move.l  0(a3,d3.w),d3       ; crc table waarde
     eor.l   d3,d1               ; xor crc-som met byte omgekeerde normale-tabel waarde
     move.w  (a0)+,d2            ; next word
     move.b  d2,d5               ; maak kopie van byte
     eor.w   d1,d2               ; xor crc-som met word
     clr.b   d2                  ; eeste byte bekijken (shr 8...)
     lsr.w   #6,d2               ; offset in crc tabel, table shortened, de opcode wordt aangepast aan size van table, verdwijnt bij #0
     move.l  0(a2,d2.w),d3       ; get crc BIG tabel waarde
     and.w   d4,d1               ; onderste byte crc-som vrij (shr 8...)
     eor.l   d3,d1               ; xor met tabel waarde
     swap    d1                  ; do the swap
.chdi_endloop:
     dbra    d0,.chdi_mainloop   ; next
     addq.w  #1,d0               ; werk weer terug
     subq.l  #1,d0               ; echt einde?
     bcc.s   .chdi_mainloop      ; neen
     subq.l  #2,a0               ; (a0)+ correction 
     movem.l (sp)+,d3-d5/a2-a3   ; restore registers
if GNUC
     move.l  (sp)+,d2
endif
     move.l  d1,d0               ; return crc som
     rts

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
;*******************************************************************************
;****************************************************************************
;*******************         LHA CRC routs             **********************
;****************************************************************************

; nieuwe KAOS crc routines
;void make_crc16table(uint16 crc_poly, unsigned long *crc_table)

if GNUC
_make_crc16table:
make_crc16table:
     move.l  8(sp),a0
     move.l  4(sp),d0
     move.l  d2,-(sp)
else
make_crc16table:
endif
     lea     512(a0),a0           ; crc end, build normal crc table here
     move.w  d0,d2
     moveq   #0,d0
     subq.b  #1,d0
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
     move.w  #$ff,d0             ; Make special CRC's
                                 ; a0 = crc_table
     lea     512(a0),a1          ; 2 * 256 bytes verder chaos tabel 1 
.crc_ch_xloop:
     move.w  (a0)+,d1            ; Get normal value
     rol.w   #8,d1               ; Create first value
     move.w  d1,(a1)+            ;
     dbra    d0,.crc_ch_xloop
if GNUC
     move.l  (sp)+,d2
endif
     rts
     
;********************************************************************************

; uint16 crc_buf(char *str, ulong len, uint16 crc, unsigned uint16 *crc_table)
;
; CALL:
; D0 = #bytes (long)
; D1 = crc-code
; A0 = buffer
;
; Return:
; D0 = CRC-code
;
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
; we do damage/change (whatever tastes best ;-)): D0-D2/A0-A1
     move.w  a0,d2               ; need low bit of a0
     and.w   #1,d2               ; even?
     beq.s   .continue           ; yep!
     moveq   #0,d2               ;
     move.b  (a0)+,d2            ; eerste byte
     eor.b   d1,d2               ;
     lsr.w   #8,d1
     add.w   d2,d2
     move.w  0(a1,d2.w),d2
     eor.w   d2,d1               ; crc voor eerste byte gedaan
     subq.l  #1,d0               ; dec count
     beq.s   .length_is_one      ; fast end
.continue:
     moveq   #0,d2
     move.b  (a0),d2             ; eerste byte
     eor.b   d1,d2               ;
     lsr.w   #8,d1
     add.w   d2,d2
     move.w  0(a1,d2.w),d2
     eor.w   d2,d1               ; crc voor eerste byte gedaan
     ; check on 2-byte & 1-byte lengths!
     subq.l  #1,d0
     beq.s   .length_is_one
     subq.l  #1,d0
     beq.s   .length_is_two
     lsr.l   #1,d0
     bcs.s   .chaos_doit_jump
     subq.l  #1,d0
     bsr.s   .chaos_doit_sub
.length_is_two:
     moveq   #0,d2
     move.b  1(a0),d2
     eor.b   d1,d2
     lsr.w   #8,d1
     add.w   d2,d2
     move.w  0(a1,d2.w),d2
     eor.w   d2,d1
.length_is_one:   
     move.w  d1,d0
if GNUC
     move.l  (sp)+,d2
endif
     rts

.chaos_doit_sub:
if GNUC
     move.l d2,-(sp)
endif
.chaos_doit_jump:
; MUST be called with AT LEAST 1 word to do CRC over...
     movem.l d3-d5/a2,-(sp)
     lea     512(a1),a2          ; chaos tabel 1
     move.w  #$00ff,d4           ; and mask
     move.w  (a0)+,d5            ;
     and.w   d4,d5               ; bovenste helft d5 leeg
.chdi_mainloop:
     move.w  d5,d3               ; onderste byte to be crc'ed
     eor.b   d1,d3               ; xor met crc som
     add.w   d3,d3               ; offset in crc tabel
     clr.b   d1                  ; clear byte (shr 8...)
     move.w  0(a2,d3.w),d3       ; crc table waarde
     eor.w   d3,d1               ; xor crc-som met byte omgekeerde normale-tabel waarde
     move.w  (a0)+,d2            ; next word
     move.b  d2,d5               ; maak kopie van byte
     eor.w   d1,d2               ; xor crc-som met word
     clr.b   d2                  ; eeste byte bekijken (shr 8...)
     lsr.w   #7,d2               ; offset in crc tabel, table shortened, de opcode wordt aangepast aan size van table, verdwijnt bij #0
     move.w  0(a1,d2.w),d3       ; get crc tabel waarde
     and.w   d4,d1               ; onderste byte crc-som vrij (shr 8...)
     eor.w   d3,d1               ; xor met tabel waarde
.chdi_endloop:
     dbra    d0,.chdi_mainloop   ; next
     addq.w  #1,d0               ; werk weer terug
     subq.l  #1,d0               ; echt einde?
     bcc.s   .chdi_mainloop      ; neen
     subq.l  #2,a0               ; (a0)+ correction 
     movem.l (sp)+,d3-d5/a2      ; restore registers
if GNUC
     move.l  (sp)+,d2
endif
     move.w  d1,d0               ; return crc som
     rts

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp

;********************************************************************************

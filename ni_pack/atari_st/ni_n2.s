; nrv2s decompression in pure 68k asm
; by ross, changed fot the n2 decoder by Hans -Mr Ni!- Wessels
;
; void decode_n1(char* depack_space, char* packed_data)
; CALL:
; A0 = ptr to depack space
; A1 = ptr to packed data
; (decompress from A0 to A1)
;
; Register usage:
;       a2      m_pos
;       a3      constant: -$1024
;
;       d0      bit buffer
;       d1      m_off
;       d2      m_len or -1
;
;       d3      last_m_off
;       d4      constant: 2
;       d5      reserved space on stack (max 256)
;
;
; Notes:
;       we have max_offset = 2^23, so we can use some word arithmetics on d1
;       we have max_match = 65535, so we can use word arithmetics on d2
;

export decode_n2

decode_n2:
     movem.l D3-D5/A2-A3,-(SP)
; ------------- setup constants -----------

     moveq   #-$80,D0         ; d0.b = $80 (byte refill flag)
     moveq   #-1,D2
     moveq   #-1,D3           ; last_ptr = 0
     moveq   #2,D4
     movea.w #-1024,A3

; ------------- DECOMPRESSION -------------
decompr_literal:
     move.b  (A1)+,(A0)+      ; copy literal

decompr_loop:
     add.b   D0,D0            ; shift bit
     bcc.s   decompr_literal  ; bit zero, kan nooit leeg zijn, ga naar literal
     bne.s   decompr_match    ; sentry nog aanwezig, match
     move.b  (A1)+,D0         ; refill bit buffer
     addx.b  D0,D0            ; shift bit
     bcc.s   decompr_literal  ; literal
decompr_match:
     moveq   #-2,D1           ; ptr init
get_pointer:
     add.b   D0,D0            ; get bit
     bne.s   _g_1             ; continue
     move.b  (A1)+,D0         ; refill buffer
     addx.b  D0,D0            ; get bit
_g_1:
     addx.w  D1,D1            ; shift bit in D1, max 2^23!
     add.b   D0,D0            ; nog een bit?
     bcc.s   get_pointer      ; yep
     bne.s   decompr_select   ; geen refil, select
     move.b  (A1)+,D0         ; refill bitbuf
     addx.b  D0,D0            ; get bit
     bcc.s   get_pointer      ; nog een bit!
decompr_select:
     addq.w  #3,D1            ; d1 was -3
     beq.s   decompr_get_mlen ; last m_off gebruik last pointer
     bpl.s   decompr_exit_token ; d1 was -2
     lsl.l   #8,D1            ; 8 extra bits
     move.b  (A1)+,D1         ; insert bits, dus is D1 max 2^23
     move.l  D1,D3            ; last_m_off = m_off

decompr_get_mlen:             ; implicit d2 = -1
     add.b   D0,D0            ; get bit
     bne.s   _e_1             ; geen refill
     move.b  (A1)+,D0         ; refill bit buffer
     addx.b  D0,D0            ; get bit
_e_1:
     addx.w  D2,D2            ; schuif bit in D2
     add.b   D0,D0            ; get bit
     bne.s   _e_2             ; geen refill
     move.b  (A1)+,D0         ; refill bitbuffer
     addx.b  D0,D0            ; get bit
_e_2:
     addx.w  D2,D2            ; schuif bit in D2, D2=-1..-4
     lea     0(A0,D3.l),A2    ; src ptr in A2
     addq.w  #2,D2            ;
     bgt.s   get_len          ; D1=1: lange lengte, anders D1=-2..0
decompr_tiny_mlen:
     move.l  D3,D1            ; last_offset
     sub.l   A3,D1            ; a3=-$d00, d1 = last_offset+$d00
     addx.w  D4,D2            ; d4 = 2, d2+=2+x bit, last offset groter $d00 dan extra byte copy... nice, D2=0..2 + x-bit= 2..4+xbit bytes copy
L_copy2:
     move.b  (A2)+,(A0)+      ; copy bytes
L_copy1:
     move.b  (A2)+,(A0)+      ; copy bytes
     dbra    D2,L_copy1
L_rep:
     bra.s   decompr_loop

get_len:                      ; implicit d2 = 1
     add.b   D0,D0            ; get bit
     bne.s   _g_2             ; no refill
     move.b  (A1)+,D0         ; refill bitbuffer
     addx.b  D0,D0            ; get bit
_g_2:
     addx.w  D2,D2            ; shift bit in d2
     add.b   D0,D0            ; nog een bit?
     bcc.s   get_len          ; ja, nog een bit
     bne.s   decompr_large_mlen ; geen refill
     move.b  (A1)+,D0         ; refill
     addx.b  D0,D0            ; get bit
     bcc.s   get_len          ; nog een bit
decompr_large_mlen:
     move.b  (A2)+,(A0)+      ; in ieder geval 2 bytes + D2 bytes
     move.b  (A2)+,(A0)+      ;
     cmp.l   A3,D3            ; lange offset?
     bcs.s   L_copy2          ; ja, twee extra bytes
     move.b  (A2)+,(A0)+      ; nee, 1 extra byte
     dbra    D2,L_copy1       ; copy bytes
decompr_exit_token:
     movem.l (SP)+,D3-D5/A2-A3
     rts

     END

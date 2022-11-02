;
; ni packer n1 decoder, size optimized
; 2022 Hans Wessels

;void decode_n0(uint8_t *dst, uint8_t *data)
; registers
; D0 = 
; D1 = bitbuffer
; D2 = run counter
; D3 = loop counter
; D4 = iterator
;
; A0 = dst
; A1 = src
; A2 = copy pointer
; alternatief pointer 8 of 16 bit, min match = 3

export decode_n1

decode_n1:
		movem.l	D3-D4/A2,-(SP)		; save registers
		moveq	#-128,D1			; bit buffer sentry
		moveq	#4,D0				; get 5 bit iterator
		bsr.s	.get_iterator		; D4=#bits in loopcounter, D0 =$0000FFFF, flags: status of D4
		tst.w	D4
		beq.s	.single_bit_loop	; D4=0?
		moveq	#0,D0				; init multi bit
		subq.w	#1,D4				; 1 bit minder te doen
.single_bit_loop:
		addq.w	#1,D0				; D0 =0 for D4 zero, =1 for D4 not zero
		bsr.s	.get_bits
		move.l	D0,D3
.loop:
		bsr.s	.decode_run			; literal run
		move.l	D0,D2				; run counter naar D2
.lit:
		move.b	(A1)+,(A0)+			; copy
		subq.l	#1,D2				; nog een literal?
		bcc.s	.lit				; next literal
		subq.l	#1,D3				; klaar?
		bcs.s	.done
		bsr.s	.decode_run			; ptr_len run
		move.l	D0,D2				; run counter naar D2
.ptr_len:
		moveq	#-1,D0				; D0=$FFFFFFFF
		addq.w	#4,D0				; get 4 bit iterator, D0=$FFFF0003
		bsr.s	.get_iterator		; D4=#bits in loopcounter, D0=$FFFFFFFF, flags: status of D4, nope, addx doesn't clear the Z flag
		tst.w	D4
		beq.s	.single_bit			; D4=0?
		subq.w	#1,d4				; 1 bit minder te doen
		add.w	D0,D0				; maak 0 bit
.single_bit:
		bsr.s	.get_bits			; D0 is ptr offset
		lea		0(D0.l,A0),A2		; pointer offset
		bsr.s	.get_len			; get len
.copy_loop:
		move.b	(A2)+,(A0)+			; copy
		dbra	D0,.copy_loop
		subq.l	#1,D2				; nog een ptr_len?
		bcc.s	.ptr_len			; next literal
		subq.l	#1,D3				; klaar?
		bcc.s	.loop				; nog een ronde
.done:
		movem.l	(SP)+,D3-D4/A2	; restore registers
		rts

.get_iterator:						; d0 = log(len)
		moveq	#0,D4				; iterator
.iterator_loop:
		bsr.s	.get_bit			; bit
		addx.w	D4,D4				; schuif bit naar binnen
		dbra	D0,.iterator_loop	; next
		rts
.get_bits:
		bsr.s	.get_bit			; bit
		addx.l	D0,D0				; shift in
		dbra	d4,.get_bits		; next
		rts
.decode_run:
		bsr.s	.get_bit			; klaar?
		bcc.s	.decode_done		; yep
		moveq	#1,D0				; result=1
		bra.s	.nog_een_bit
.decode_done:
		moveq	#0,D0				; result = 0
		rts

.get_len:
		moveq	#1,D0				; init waarde
.next_bit:
		bsr.s	.get_bit			; get value bit in x register
		addx.w	D0,D0				; verdubbel en tel waarde van bit op bij D0
.nog_een_bit:
		bsr.s	.get_bit			; nog een bit?
		bcs.s	.next_bit			; yep
		rts

.get_bit:							; zet bit in x-bit status register
		add.b	D1,D1				; schuif bit naar buiten
		bne.s	.get_bit_done		; klaar
		move.b	(A1)+,D1			; nieuwe bits
		addx.b	D1,D1				; bit naar buiten, sentry naar binnen
.get_bit_done:
		rts

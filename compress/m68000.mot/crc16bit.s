|
| ultra high speed 16 bit crc table
|

	.globl make_crc32table
	.globl crc32
	.globl make_crc16table
	.globl crc16

| The following exports should be after "%include command.mac"

	.globl _make_crc32table
	.globl _crc32
	.globl _make_crc16table
	.globl _crc16

	.even

	.text

|************************* ARJ CRC routs ************************************

| High speed 16 bits at a time crc routs
|void make_crctable(uint32 *crc_table)

_make_crc32table:
make_crc32table:
	move.l  8(%sp),%a0
	move.l  4(%sp),%d0
	movem.l %d2-%d4/%a2,-(%sp)
	move.l  %d0,%d2
	moveq   #0,%d0
	lea     256*4(%a0),%a1       | crctable_end
	subq.b  #1,%d0
	move.l  %a1,%a0
ll_0000:
	move.l  %d0,%d1
	lsr.l   #1,%d1
	bcc.s   ll_0001
	eor.l   %d2,%d1
ll_0001:
	lsr.l   #1,%d1
	bcc.s   ll_0002
	eor.l   %d2,%d1
ll_0002:
	lsr.l   #1,%d1
	bcc.s   ll_0003
	eor.l   %d2,%d1
ll_0003:
	lsr.l   #1,%d1
	bcc.s   ll_0004
	eor.l   %d2,%d1
ll_0004:
	lsr.l   #1,%d1
	bcc.s   ll_0005
	eor.l   %d2,%d1
ll_0005:
	lsr.l   #1,%d1
	bcc.s   ll_0006
	eor.l   %d2,%d1
ll_0006:
	lsr.l   #1,%d1
	bcc.s   ll_0007
	eor.l   %d2,%d1
ll_0007:
	lsr.l   #1,%d1
	bcc.s   ll_0008
	eor.l   %d2,%d1
ll_0008:
	move.l  %d1,-(%a0)
	dbra    %d0,ll_0000
	move.l  %a1,%a2
	add.l   #0x40000,%a2         | big table end
	move.l  #255,%d0            | count 1
ll_0009:
	move.l  #255,%d1            | count 2
	move.l  -(%a1),%d2           | crc1
	move.b  %d2,%d3              | need d2.low
	lsr.l   #8,%d2              | schuif
ll_0010:
	move.l  %d1,%d4              | new char
	eor.b   %d3,%d4              | lower part
	add.w   %d4,%d4
	add.w   %d4,%d4
	move.l  0(%a0,%d4.w),%d4
	eor.l   %d2,%d4              | new crc
	ror.w   #8,%d4
	swap    %d4
	ror.w   #8,%d4
	swap    %d4
	move.l  %d4,-(%a2)
	dbra    %d1,ll_0010
	dbra    %d0,ll_0009
	movem.l (%sp)+,%d2-%d4/%a2
	rts
     
|********************************************************************************

| ulong crc_buf(char *str, ulong len, long crc, unsigned long *crc_table)
|
| CALL:
| D0 = #bytes (long)
| D1 = crc-code
| A0 = buffer
| A1 = crc table
|
| Return:
| D0 = CRC-code
|
| we do damage/change (whatever tastes best ;-)): D0-D2/A0-A1
_crc32:
crc32:
	move.l 4(%sp),%a0             |*str
	move.l 8(%sp),%d0             |len
	move.l 12(%sp),%d1            |crc
	move.l 16(%sp),%a1            |crc_table
	move.l %d2,-(%sp)
	move.l  %a0,%d2               | buffer
	and.w   #1,%d2               | even?
	beq.s   ll_0011               | yep
	moveq   #0,%d2               |
	move.b  (%a0)+,%d2            | eerste byte
	eor.b   %d1,%d2               |
	add.w   %d2,%d2
	lsr.l   #8,%d1
	add.w   %d2,%d2
	move.l  (%a1,%d2.w),%d2
	eor.l   %d2,%d1               | crc voor eerste byte gedaan
	subq.l  #1,%d0
	beq.s   ll_0013
ll_0011:
	subq.l  #2,%d0               |
	bcs.s   ll_0012
	lsr.l   #1,%d0               | count/2
	bcc.s   ll_0015
	bsr.s   ll_0014
ll_0012:
	moveq   #0,%d2
	move.b  (%a0)+,%d2
	eor.b   %d1,%d2
	lsr.l   #8,%d1
	add.w   %d2,%d2
	add.w   %d2,%d2
	move.l  (%a1,%d2.w),%d2
	eor.l   %d2,%d1
ll_0013:
	move.l  %d1,%d0
	move.l  (%sp)+,%d2
	rts

ll_0014:
	move.l  %d2,-(%sp)
ll_0015:
	swap    %d1
	move.l  %a2,-(%sp)
	ror.w   #8,%d1
	swap    %d1
	lea     256*4(%a1),%a2
	ror.w   #8,%d1
	move.l  %d1,%d2
	lsr.l   #1,%d0
	bcc.s   ll_0017
ll_0016:
	moveq   #0,%d2
	move.w  (%a0)+,%d2
	eor.w   %d1,%d2
	add.l   %d2,%d2
	swap    %d1
	add.l   %d2,%d2
	move.l  0(%a2,%d2.l),%d2
	eor.w   %d1,%d2
ll_0017:
	moveq   #0,%d1
	move.w  (%a0)+,%d1
	eor.w   %d2,%d1
	add.l   %d1,%d1
	swap    %d2
	add.l   %d1,%d1
	move.l  0(%a2,%d1.l),%d1
	eor.w   %d2,%d1
	dbra    %d0,ll_0016
	addq.w  #1,%d0
	subq.l  #1,%d0
	bcc.s   ll_0016
	swap    %d1
	ror.w   #8,%d1
	move.l  (%sp)+,%a2
	swap    %d1
	ror.w   #8,%d1
	move.l  %d1,%d0
	move.l  (%sp)+,%d2
	rts


|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************
|****************************************************************************
|*******************         LHA CRC routs             **********************
|****************************************************************************

| High speed 16 bits at a time crc routs
|void make_crctable(uint16 *crc_table)

_make_crc16table:
make_crc16table:
	move.l  8(%sp),%a0
	move.w  4(%sp),%d0
	movem.l %d2-%d4/%a2,-(%sp)
	move.w  %d0,%d2
	moveq   #0,%d0
	lea     256*2(%a0),%a1       | crctable_end
	subq.b  #1,%d0
	move.l  %a1,%a0
ll_0018:
	move.w  %d0,%d1
	lsr.w   #1,%d1
	bcc.s   ll_0019
	eor.w   %d2,%d1
ll_0019:
	lsr.w   #1,%d1
	bcc.s   ll_0020
	eor.w   %d2,%d1
ll_0020:
	lsr.w   #1,%d1
	bcc.s   ll_0021
	eor.w   %d2,%d1
ll_0021:
	lsr.w   #1,%d1
	bcc.s   ll_0022
	eor.w   %d2,%d1
ll_0022:
	lsr.w   #1,%d1
	bcc.s   ll_0023
	eor.w   %d2,%d1
ll_0023:
	lsr.w   #1,%d1
	bcc.s   ll_0024
	eor.w   %d2,%d1
ll_0024:
	lsr.w   #1,%d1
	bcc.s   ll_0025
	eor.w   %d2,%d1
ll_0025:
	lsr.w   #1,%d1
	bcc.s   ll_0026
	eor.w   %d2,%d1
ll_0026:
	move.w  %d1,-(%a0)
	dbra    %d0,ll_0018
	move.l  %a1,%a2
	add.l   #0x20000,%a2         | big table end
	move.l  #255,%d0            | count 1
ll_0027:
	move.l  #255,%d1            | count 2
	move.w  -(%a1),%d2           | crc1
	move.b  %d2,%d3              | need d2.low
	lsr.w   #8,%d2              | schuif
ll_0028:
	move.l  %d1,%d4              | new char
	eor.b   %d3,%d4              | lower part
	add.w   %d4,%d4
	move.w  0(%a0,%d4.w),%d4
	eor.w   %d2,%d4              | new crc
	ror.w   #8,%d4
	move.w  %d4,-(%a2)
	dbra    %d1,ll_0028
	dbra    %d0,ll_0027
	movem.l (%sp)+,%d2-%d4/%a2
	rts
     
|********************************************************************************

| ulong crc_buf(char *str, ulong len, long crc, unsigned long *crc_table)
|
| CALL:
| D0 = #bytes (long)
| D1 = crc-code
| A0 = buffer
| A1 = crc table
|
| Return:
| D0 = CRC-code
|
| we do damage/change (whatever tastes best ;-)): D0-D2/A0-A1
_crc16:
crc16:
	move.l 4(%sp),%a0             |*str
	move.l 8(%sp),%d0             |len
	move.l 12(%sp),%d1            |crc
	move.l 16(%sp),%a1            |crc_table
	move.l %d2,-(%sp)
	move.l  %a0,%d2               | buffer
	and.w   #1,%d2               | even?
	beq.s   ll_0029               | yep
	moveq   #0,%d2               |
	move.b  (%a0)+,%d2            | eerste byte
	eor.b   %d1,%d2               |
	add.w   %d2,%d2
	lsr.w   #8,%d1
	move.w  (%a1,%d2.w),%d2
	eor.w   %d2,%d1               | crc voor eerste byte gedaan
	subq.l  #1,%d0
	beq.s   ll_0031
ll_0029:
	subq.l  #2,%d0               |
	bcs.s   ll_0030
	lsr.l   #1,%d0               | count/2
	bcc.s   ll_0033
	bsr.s   ll_0032
ll_0030:
	moveq   #0,%d2
	move.b  (%a0)+,%d2
	eor.b   %d1,%d2
	lsr.w   #8,%d1
	add.w   %d2,%d2
	move.w  (%a1,%d2.w),%d2
	eor.w   %d2,%d1
ll_0031:
	move.l  %d1,%d0
	move.l  (%sp)+,%d2
	rts

ll_0032:
	move.l  %d2,-(%sp)
ll_0033:
	move.l  %a2,-(%sp)
	ror.w   #8,%d1
	lea     256*2(%a1),%a2
	lsr.l   #1,%d0
	bcc.s   ll_0035
ll_0034:
	moveq   #0,%d2
	move.w  (%a0)+,%d2
	eor.w   %d1,%d2
	add.l   %d2,%d2
	move.w  0(%a2,%d2.l),%d1
ll_0035:
	moveq   #0,%d2
	move.w  (%a0)+,%d2
	eor.w   %d1,%d2
	add.l   %d2,%d2
	move.w  0(%a2,%d2.l),%d1
	dbra    %d0,ll_0034
	addq.w  #1,%d0
	subq.l  #1,%d0
	bcc.s   ll_0034
	ror.w   #8,%d1
	move.l  (%sp)+,%a2
	move.l  %d1,%d0
	move.l  (%sp)+,%d2
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp

|********************************************************************************

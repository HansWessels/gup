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
	movel  sp@(8),a0
	movel  sp@(4),d0
	moveml d2-d4/a2,sp@-
	movel  d0,d2
	moveq   #0,d0
	lea     a0@(256*4),a1       | crctable_end
	subqb  #1,d0
	movel  a1,a0
ll_0000:
	movel  d0,d1
	lsrl   #1,d1
	bcc   ll_0001
	eorl   d2,d1
ll_0001:
	lsrl   #1,d1
	bcc   ll_0002
	eorl   d2,d1
ll_0002:
	lsrl   #1,d1
	bcc   ll_0003
	eorl   d2,d1
ll_0003:
	lsrl   #1,d1
	bcc   ll_0004
	eorl   d2,d1
ll_0004:
	lsrl   #1,d1
	bcc   ll_0005
	eorl   d2,d1
ll_0005:
	lsrl   #1,d1
	bcc   ll_0006
	eorl   d2,d1
ll_0006:
	lsrl   #1,d1
	bcc   ll_0007
	eorl   d2,d1
ll_0007:
	lsrl   #1,d1
	bcc   ll_0008
	eorl   d2,d1
ll_0008:
	movel  d1,a0@-
	dbra    d0,ll_0000
	movel  a1,a2
	addl   #0x40000,a2         | big table end
	movel  #255,d0            | count 1
ll_0009:
	movel  #255,d1            | count 2
	movel  a1@-,d2           | crc1
	moveb  d2,d3              | need d2.low
	lsrl   #8,d2              | schuif
ll_0010:
	movel  d1,d4              | new char
	eorb   d3,d4              | lower part
	addw   d4,d4
	addw   d4,d4
	movel  a0@(0,d4:w),d4
	eorl   d2,d4              | new crc
	rorw   #8,d4
	swap    d4
	rorw   #8,d4
	swap    d4
	movel  d4,a2@-
	dbra    d1,ll_0010
	dbra    d0,ll_0009
	moveml sp@+,d2-d4/a2
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
	movel sp@(4),a0             |*str
	movel sp@(8),d0             |len
	movel sp@(12),d1            |crc
	movel sp@(16),a1            |crc_table
	movel d2,sp@-
	movel  a0,d2               | buffer
	andw   #1,d2               | even?
	beq   ll_0011               | yep
	moveq   #0,d2               |
	moveb  a0@+,d2            | eerste byte
	eorb   d1,d2               |
	addw   d2,d2
	lsrl   #8,d1
	addw   d2,d2
	movel  a1@(d2:w),d2
	eorl   d2,d1               | crc voor eerste byte gedaan
	subql  #1,d0
	beq   ll_0013
ll_0011:
	subql  #2,d0               |
	bcs   ll_0012
	lsrl   #1,d0               | count/2
	bcc   ll_0015
	bsr   ll_0014
ll_0012:
	moveq   #0,d2
	moveb  a0@+,d2
	eorb   d1,d2
	lsrl   #8,d1
	addw   d2,d2
	addw   d2,d2
	movel  a1@(d2:w),d2
	eorl   d2,d1
ll_0013:
	movel  d1,d0
	movel  sp@+,d2
	rts

ll_0014:
	movel  d2,sp@-
ll_0015:
	swap    d1
	movel  a2,sp@-
	rorw   #8,d1
	swap    d1
	lea     a1@(256*4),a2
	rorw   #8,d1
	movel  d1,d2
	lsrl   #1,d0
	bcc   ll_0017
ll_0016:
	moveq   #0,d2
	movew  a0@+,d2
	eorw   d1,d2
	addl   d2,d2
	swap    d1
	addl   d2,d2
	movel  a2@(0,d2:l),d2
	eorw   d1,d2
ll_0017:
	moveq   #0,d1
	movew  a0@+,d1
	eorw   d2,d1
	addl   d1,d1
	swap    d2
	addl   d1,d1
	movel  a2@(0,d1:l),d1
	eorw   d2,d1
	dbra    d0,ll_0016
	addqw  #1,d0
	subql  #1,d0
	bcc   ll_0016
	swap    d1
	rorw   #8,d1
	movel  sp@+,a2
	swap    d1
	rorw   #8,d1
	movel  d1,d0
	movel  sp@+,d2
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
	movel  sp@(8),a0
	movew  sp@(4),d0
	moveml d2-d4/a2,sp@-
	movew  d0,d2
	moveq   #0,d0
	lea     a0@(256*2),a1       | crctable_end
	subqb  #1,d0
	movel  a1,a0
ll_0018:
	movew  d0,d1
	lsrw   #1,d1
	bcc   ll_0019
	eorw   d2,d1
ll_0019:
	lsrw   #1,d1
	bcc   ll_0020
	eorw   d2,d1
ll_0020:
	lsrw   #1,d1
	bcc   ll_0021
	eorw   d2,d1
ll_0021:
	lsrw   #1,d1
	bcc   ll_0022
	eorw   d2,d1
ll_0022:
	lsrw   #1,d1
	bcc   ll_0023
	eorw   d2,d1
ll_0023:
	lsrw   #1,d1
	bcc   ll_0024
	eorw   d2,d1
ll_0024:
	lsrw   #1,d1
	bcc   ll_0025
	eorw   d2,d1
ll_0025:
	lsrw   #1,d1
	bcc   ll_0026
	eorw   d2,d1
ll_0026:
	movew  d1,a0@-
	dbra    d0,ll_0018
	movel  a1,a2
	addl   #0x20000,a2         | big table end
	movel  #255,d0            | count 1
ll_0027:
	movel  #255,d1            | count 2
	movew  a1@-,d2           | crc1
	moveb  d2,d3              | need d2.low
	lsrw   #8,d2              | schuif
ll_0028:
	movel  d1,d4              | new char
	eorb   d3,d4              | lower part
	addw   d4,d4
	movew  a0@(0,d4:w),d4
	eorw   d2,d4              | new crc
	rorw   #8,d4
	movew  d4,a2@-
	dbra    d1,ll_0028
	dbra    d0,ll_0027
	moveml sp@+,d2-d4/a2
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
	movel sp@(4),a0             |*str
	movel sp@(8),d0             |len
	movel sp@(12),d1            |crc
	movel sp@(16),a1            |crc_table
	movel d2,sp@-
	movel  a0,d2               | buffer
	andw   #1,d2               | even?
	beq   ll_0029               | yep
	moveq   #0,d2               |
	moveb  a0@+,d2            | eerste byte
	eorb   d1,d2               |
	addw   d2,d2
	lsrw   #8,d1
	movew  a1@(d2:w),d2
	eorw   d2,d1               | crc voor eerste byte gedaan
	subql  #1,d0
	beq   ll_0031
ll_0029:
	subql  #2,d0               |
	bcs   ll_0030
	lsrl   #1,d0               | count/2
	bcc   ll_0033
	bsr   ll_0032
ll_0030:
	moveq   #0,d2
	moveb  a0@+,d2
	eorb   d1,d2
	lsrw   #8,d1
	addw   d2,d2
	movew  a1@(d2:w),d2
	eorw   d2,d1
ll_0031:
	movel  d1,d0
	movel  sp@+,d2
	rts

ll_0032:
	movel  d2,sp@-
ll_0033:
	movel  a2,sp@-
	rorw   #8,d1
	lea     a1@(256*2),a2
	lsrl   #1,d0
	bcc   ll_0035
ll_0034:
	moveq   #0,d2
	movew  a0@+,d2
	eorw   d1,d2
	addl   d2,d2
	movew  a2@(0,d2:l),d1
ll_0035:
	moveq   #0,d2
	movew  a0@+,d2
	eorw   d1,d2
	addl   d2,d2
	movew  a2@(0,d2:l),d1
	dbra    d0,ll_0034
	addqw  #1,d0
	subql  #1,d0
	bcc   ll_0034
	rorw   #8,d1
	movel  sp@+,a2
	movel  d1,d0
	movel  sp@+,d2
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp

|********************************************************************************

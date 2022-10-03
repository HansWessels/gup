|
| chaos crc routines
|
| 1999-06-26
| HWS: Fixed wrong conditional branch in arj_crc
| 1997-11-04
| HWS: Fixed routines for odd adresses
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

| nieuwe KAOS crc routines
|void make_crctable(uint32 crc_poly, unsigned long *crc_table)

_make_crc32table:
make_crc32table:
	movel  sp@(8),a0
	movel  sp@(4),d0
	movel  d2,sp@-
	lea     a0@(1024),a0         | crc end, build normal crc table here
	movel  d0,d2
	moveq   #0,d0
	subqb  #1,d0
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
	movew  #0xff,d0             | Make special CRC's
	movel  a2,sp@-            | save A2
                                 | a0 = crc_table
	lea     a0@(1024),a1         | 4 * 256 bytes verder chaos tabel 1 
	lea     a0@(2048),a2         | 2*4*256 bytes verder chaos tabel 2
    
ll_0009:
	movel  a0@+,d1            | Get normal value
	roll   #8,d1               | Create first value
	movel  d1,a1@+            |
	roll   #8,d1               | Create second value
	movel  d1,a2@+
	dbra    d0,ll_0009
	movel  sp@+,a2
	movel  sp@+,d2
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
	movew  a0,d2               | need low bit of a0
	andw   #1,d2               | even?
	beq   ll_0010           | yep!
	moveq   #0,d2               |
	moveb  a0@+,d2            | eerste byte
	eorb   d1,d2               |
	lsrl   #8,d1
	addw   d2,d2
	addw   d2,d2
	movel  a1@(0,d2:w),d2
	eorl   d2,d1               | crc voor eerste byte gedaan
	subql  #1,d0               | dec count
	beq   ll_0012      | fast end
ll_0010:
	moveq   #0,d2               |
	moveb  a0@,d2             | eerste byte
	eorb   d1,d2               |
	lsrl   #8,d1
	addw   d2,d2
	addw   d2,d2
	movel  a1@(0,d2:w),d2
	eorl   d2,d1               | crc voor eerste byte gedaan
     | check on 2-byte & 1-byte lengths!
	subql  #1,d0
	beq   ll_0012
	subql  #1,d0
	beq   ll_0011
	lsrl   #1,d0
	bcs   ll_0014
	subql  #1,d0
	bsr   ll_0013
ll_0011:
	moveq   #0,d2
	moveb  a0@(1),d2
	eorb   d1,d2
	lsrl   #8,d1
	addw   d2,d2
	addw   d2,d2
	movel  a1@(0,d2:w),d2
	eorl   d2,d1
ll_0012:   
	movel  d1,d0
	movel  sp@+,d2
	rts


ll_0013:
	movel d2,sp@-
ll_0014:
| MUST be called with AT LEAST 1 word to do CRC over...
	moveml d3-d5/a2-a3,sp@-
	lea     a1@(1024),a3         | chaos tabel 1
	lea     a1@(2048),a2         | chaos tabel 2
	movew  #0x00ff,d4           | and mask
	movew  a0@+,d5            |
	andw   d4,d5               | bovenste helft d5 leeg
ll_0015:
	movew  d5,d3               | onderste byte to be crc'ed
	eorb   d1,d3               | xor met crc som
	addw   d3,d3               | offset in crc tabel
	addw   d3,d3               |
	clrb   d1                  | clear byte (shr 8...)
	movel  a3@(0,d3:w),d3       | crc table waarde
	eorl   d3,d1               | xor crc-som met byte omgekeerde normale-tabel waarde
	movew  a0@+,d2            | next word
	moveb  d2,d5               | maak kopie van byte
	eorw   d1,d2               | xor crc-som met word
	clrb   d2                  | eeste byte bekijken (shr 8...)
	lsrw   #6,d2               | offset in crc tabel, table shortened, de opcode wordt aangepast aan size van table, verdwijnt bij #0
	movel  a2@(0,d2:w),d3       | get crc BIG tabel waarde
	andw   d4,d1               | onderste byte crc-som vrij (shr 8...)
	eorl   d3,d1               | xor met tabel waarde
	swap    d1                  | do the swap
ll_0016:
	dbra    d0,ll_0015   | next
	addqw  #1,d0               | werk weer terug
	subql  #1,d0               | echt einde?
	bcc   ll_0015      | neen
	subql  #2,a0               | (a0)+ correction 
	moveml sp@+,d3-d5/a2-a3   | restore registers
	movel  sp@+,d2
	movel  d1,d0               | return crc som
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
|*******************************************************************************
|****************************************************************************
|*******************         LHA CRC routs             **********************
|****************************************************************************

| nieuwe KAOS crc routines
|void make_crc16table(uint16 crc_poly, unsigned long *crc_table)

_make_crc16table:
make_crc16table:
	movel  sp@(8),a0
	movel  sp@(4),d0
	movel  d2,sp@-
	lea     a0@(512),a0           | crc end, build normal crc table here
	movew  d0,d2
	moveq   #0,d0
	subqb  #1,d0
ll_0017:
	movew  d0,d1
	lsrw   #1,d1
	bcc   ll_0018
	eorw   d2,d1
ll_0018:
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
	movew  d1,a0@-
	dbra    d0,ll_0017
	movew  #0xff,d0             | Make special CRC's
                                 | a0 = crc_table
	lea     a0@(512),a1          | 2 * 256 bytes verder chaos tabel 1 
ll_0026:
	movew  a0@+,d1            | Get normal value
	rolw   #8,d1               | Create first value
	movew  d1,a1@+            |
	dbra    d0,ll_0026
	movel  sp@+,d2
	rts
     
|********************************************************************************

| uint16 crc_buf(char *str, ulong len, uint16 crc, unsigned uint16 *crc_table)
|
| CALL:
| D0 = #bytes (long)
| D1 = crc-code
| A0 = buffer
|
| Return:
| D0 = CRC-code
|
_crc16:
crc16:
	movel sp@(4),a0             |*str
	movel sp@(8),d0             |len
	movel sp@(12),d1            |crc
	movel sp@(16),a1            |crc_table
	movel d2,sp@-
| we do damage/change (whatever tastes best ;-)): D0-D2/A0-A1
	movew  a0,d2               | need low bit of a0
	andw   #1,d2               | even?
	beq   ll_0027           | yep!
	moveq   #0,d2               |
	moveb  a0@+,d2            | eerste byte
	eorb   d1,d2               |
	lsrw   #8,d1
	addw   d2,d2
	movew  a1@(0,d2:w),d2
	eorw   d2,d1               | crc voor eerste byte gedaan
	subql  #1,d0               | dec count
	beq   ll_0029      | fast end
ll_0027:
	moveq   #0,d2
	moveb  a0@,d2             | eerste byte
	eorb   d1,d2               |
	lsrw   #8,d1
	addw   d2,d2
	movew  a1@(0,d2:w),d2
	eorw   d2,d1               | crc voor eerste byte gedaan
     | check on 2-byte & 1-byte lengths!
	subql  #1,d0
	beq   ll_0029
	subql  #1,d0
	beq   ll_0028
	lsrl   #1,d0
	bcs   ll_0031
	subql  #1,d0
	bsr   ll_0030
ll_0028:
	moveq   #0,d2
	moveb  a0@(1),d2
	eorb   d1,d2
	lsrw   #8,d1
	addw   d2,d2
	movew  a1@(0,d2:w),d2
	eorw   d2,d1
ll_0029:   
	movew  d1,d0
	movel  sp@+,d2
	rts

ll_0030:
	movel d2,sp@-
ll_0031:
| MUST be called with AT LEAST 1 word to do CRC over...
	moveml d3-d5/a2,sp@-
	lea     a1@(512),a2          | chaos tabel 1
	movew  #0x00ff,d4           | and mask
	movew  a0@+,d5            |
	andw   d4,d5               | bovenste helft d5 leeg
ll_0032:
	movew  d5,d3               | onderste byte to be crc'ed
	eorb   d1,d3               | xor met crc som
	addw   d3,d3               | offset in crc tabel
	clrb   d1                  | clear byte (shr 8...)
	movew  a2@(0,d3:w),d3       | crc table waarde
	eorw   d3,d1               | xor crc-som met byte omgekeerde normale-tabel waarde
	movew  a0@+,d2            | next word
	moveb  d2,d5               | maak kopie van byte
	eorw   d1,d2               | xor crc-som met word
	clrb   d2                  | eeste byte bekijken (shr 8...)
	lsrw   #7,d2               | offset in crc tabel, table shortened, de opcode wordt aangepast aan size van table, verdwijnt bij #0
	movew  a1@(0,d2:w),d3       | get crc tabel waarde
	andw   d4,d1               | onderste byte crc-som vrij (shr 8...)
	eorw   d3,d1               | xor met tabel waarde
ll_0033:
	dbra    d0,ll_0032   | next
	addqw  #1,d0               | werk weer terug
	subql  #1,d0               | echt einde?
	bcc   ll_0032      | neen
	subql  #2,a0               | (a0)+ correction 
	moveml sp@+,d3-d5/a2      | restore registers
	movel  sp@+,d2
	movew  d1,d0               | return crc som
	rts

|d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp

|********************************************************************************

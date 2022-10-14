/*
** ARJ CRC32 routines
**
** 2022 Hans Wessels
*/

/*
** this function makes a CRC32 table for ARJ CRC32
** call function with pointer to an unsigned long[256] array
*/

void make_crc32_table(unsigned long crc_table[])
{
  int max_bits=8;
  int bits=1;
  crc_table[0]=0;
  crc_table[1<<(max_bits-1)]=0xEDB88320UL;
  do
  {
    int i=(1<<bits)-1;
    do
    {
      int j=(1<<bits)-1;
      unsigned int done=i<<(max_bits-2*bits);
      unsigned long crc=crc_table[done<<bits];
      int offset=(int)((crc&j)<<(max_bits-bits));
      crc>>=bits;
      do
      {
        int tmp=j<<(max_bits-bits);
        crc_table[tmp+done]=crc^crc_table[tmp^offset];
      }
      while(j--);
    }
    while(--i);
    bits+=bits;
  }
  while(bits<max_bits);
}

/*
** this function calculates the ARJ CRC32 checksum over count bytes in data
** call function with count: nuber of bytes to CRC, data: pointer to data bytes
*/

unsigned long crc32(unsigned loung count, unsigned char* data, unsigned long crc_table[])
{
	unsigned long crc=(unsigned long)-1;
	do
	{
		unsigned char kar = *data++;
		kar ^= (uint8) crc;
		crc >>= 8;
		crc ^= crc_table[kar];
	}  while (--count != 0);
	return crc;
}

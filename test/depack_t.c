/*
**
** test program for testing the Ni packer depack code
**
** in: arj file containing the compressed data
**
** the data will be depacked, integrety checked, timed, and thrown away
**
** 2022, Hans Wessels
**
*/

#include <stdio.h>
#include <stdlib.h>

#define ARJ_HEADER_ID 		0xEA60U

unsigned char get_byte(unsigned char *p)
{
	return p[0];
}

unsigned int get_word(unsigned char *p)
{
	unsigned int res;
	res=p[1];
	res<<=8;
	res|=p[0];
	return res;
}

unsigned long get_long(unsigned char *p)
{
	unsigned long res;
	res=p[3];
	res<<=8;
	res|=p[2];
	res<<=8;
	res|=p[1];
	res<<=8;
	res|=p[0];
	return res;
}

int main(int argc, char *argv[])
{
	char *filenaam;
	unsigned char *data;
	unsigned long offset;
	unsigned long file_size;
	FILE* f;
	if(argc==2)
	{
		filenaam=argv[1];
	}
	else
	{
		printf("Usage: %s <file to be tested>\n", argv[0]);
		return -1;
	}
	f=fopen(filenaam, "rb");
	if(f==NULL)
	{
		printf("File open error %s", filenaam);
		return -1;
	}
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	data = (char*)malloc(file_size + 1024);
	if (data == NULL)
	{
		printf("Malloc error voor file data!\n");
		fclose(f);
		return -1;
	}
	fseek(f, 0, SEEK_SET);
	(void)!fread(data, 1, file_size, f);
	fclose(f);
	offset=0;
   /*      DATA10.BIN                   7082          2644  10  5944648C */
	printf("File name:               original        packed mode CRC32\n");
	for(;;)
	{
		if(offset>=file_size)
		{
			printf("Unexpected end of data reached, aborting\n");
			break;
		}
		/* zoek header */
		if(get_word(data+offset)==ARJ_HEADER_ID)
		{ /* ARJ header gevonden */
			unsigned int header_size;
			unsigned int header_size_1;
			unsigned long compressed_size;
			unsigned long original_size;
			unsigned long crc32;
			int file_naam_pos;
			int method;
			char* naam;
			header_size=get_word(data+offset+2);
			if(header_size==0)
			{ /* end of archive */
				break;
			}
			header_size_1=get_byte(data+offset+4);
			if(offset+header_size>=file_size)
			{
				printf("Unexpected end of data reached, aborting\n");
				break;
			}
			if(get_byte(data+offset+0xA)!=0)
			{ /* not a compressed binary file, we are not interested */
				offset+=header_size+8;
				while(get_word(data+offset)!=0)
				{
					offset+=get_word(data+offset)+6;
					if(offset+2>=file_size)
					{
						printf("Unexpected end of data reached, aborting\n");
						break;
					}
				}
				offset+=2;
				continue;
			}
			method=get_byte(data+offset+9);
			compressed_size=get_long(data+offset+0x10);
			original_size=get_long(data+offset+0x14);
			crc32=get_long(data+offset+0x18);
			file_naam_pos=get_word(data+offset+0x1C);
			naam=data+offset+header_size_1+4;
			printf("%-20s", naam+file_naam_pos);
			printf(" %12lu ", original_size);
			printf(" %12lu ", compressed_size);
			printf(" %2X ", method);
			printf(" %08lX\n", crc32);
			offset+=header_size+8;
			while(get_word(data+offset)!=0)
			{
				printf("Oei, extra header!\n");
				offset+=get_word(data+offset)+6;
				if(offset+2>=file_size)
				{
					printf("Unexpected end of data reached, aborting\n");
					break;
				}
			}
			offset+=2;
			if(offset+compressed_size>=file_size)
			{
				printf("Unexpected end of data reached, aborting\n");
				break;
			}
			offset+=compressed_size;
		}
		else
		{
			offset++;
		}
	}
	free(data);
	return 0;
}


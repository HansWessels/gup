/*
**
** dump program for dumping packed data in (GUP)ARJ files
**
** in: arj file containing the compressed data
** out: binairy dump files of all files in the archive, the extension indicating
**      which packing mode was used for the file.
**      the data will be depacked, integrety checked and dumped.
**
** 2023, Hans Wessels
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __PUREC__
	typedef signed char             int8_t;
	typedef signed short int        int16_t;
	typedef signed long int         int32_t;
	typedef unsigned char           uint8_t;
	typedef unsigned short int      uint16_t;
	typedef unsigned long int       uint32_t;
#else
	#include <stdint.h>
#endif


extern void unstore(unsigned long size, uint8_t *dst, uint8_t *data);
extern void decode_m7(uint8_t *dst, uint8_t *data);
extern void decode_m4(unsigned long size, uint8_t *dst, uint8_t *data);
extern void decode_n0(uint8_t *dst, uint8_t *data);
extern void decode_n1(uint8_t *dst, uint8_t *data);
extern void decode_n2(uint8_t *dst, uint8_t *data);

extern unsigned long decode_m7_size(uint8_t *data);
extern unsigned long decode_m4_size(unsigned long packed_size, uint8_t *data);
extern unsigned long decode_n0_size(uint8_t *data);
extern unsigned long decode_n1_size(uint8_t *data);
extern unsigned long decode_n2_size(uint8_t *data);

extern void make_crc32_table(uint32_t crc_table[]);
extern uint32_t crc32(unsigned long count, uint8_t *data, uint32_t crc_table[]);

#define ARJ_HEADER_ID 		0xEA60U

#define STORE 0          /* general store */
#define ARJ_MODE_1 1     /* arj mode 1 */
#define ARJ_MODE_2 2     /* arj mode 2 */
#define ARJ_MODE_3 3     /* arj mode 3 */
#define ARJ_MODE_4 4     /* arj mode 4 */
#define GNU_ARJ_MODE_7 7 /* gnu arj mode 7 */
#define NI_MODE_0 0x10   /* ni packer mode 0 */
#define NI_MODE_1 0x11   /* ni packer mode 1 */
#define NI_MODE_2 0x12   /* ni packer mode 2 */
#define NI_MODE_3 0x13   /* ni packer mode 3 */
#define NI_MODE_4 0x14   /* ni packer mode 4 */
#define NI_MODE_5 0x15   /* ni packer mode 5 */
#define NI_MODE_6 0x16   /* ni packer mode 6 */
#define NI_MODE_7 0x17   /* ni packer mode 7 */
#define NI_MODE_8 0x18   /* ni packer mode 8 */
#define NI_MODE_9 0x19   /* ni packer mode 9 */

uint32_t crc_table[256]; /* CRC32 table to check the results of de depack routines */


uint8_t get_byte(uint8_t *p)
{
	return p[0];
}

uint16_t get_word(uint8_t *p)
{
	uint16_t res;
	res=p[1];
	res<<=8;
	res|=p[0];
	return res;
}

uint32_t get_long(unsigned char *p)
{
	uint32_t res;
	res=p[3];
	res<<=8;
	res|=p[2];
	res<<=8;
	res|=p[1];
	res<<=8;
	res|=p[0];
	return res;
}

static unsigned long error_count=0;
static unsigned long total_compressed_size=0;
static unsigned long total_original_size=0;

char *mode_to_str(int mode)
{
	char * str;
	switch(mode)
	{
	case STORE:
		str="m0";
		break;
	case ARJ_MODE_1:
		str="m1";
		break;
	case ARJ_MODE_2:
		str="m2";
		break;
	case ARJ_MODE_3:
		str="m3";
		break;
	case GNU_ARJ_MODE_7:
		str="m7";
		break;
	case ARJ_MODE_4:
		str="m4";
		break;
	case NI_MODE_0:
		str="n0";
		break;
	case NI_MODE_1:
		str="n1";
		break;
	case NI_MODE_2:
		str="n2";
		break;
	default:
		str="";
		break;
	}
	return str;
}

char *make_outfile_name(char *original_name, int mode)
{
	char* name=malloc(strlen(original_name+4));
	char* extension;
	if(name!=NULL)
	{
		sprintf(name, "%s.%s", original_name, mode_to_str(mode));
	}
	return name;
}

int decode(int mode, unsigned long size, unsigned long compressed_size, uint32_t crc, uint8_t *data)
{ /* decode the data pointed to data */
	int result=-1;
	uint8_t *dst;
	uint32_t res_crc=0;
	dst=(uint8_t *)malloc(size+1024);
	if(dst==NULL)
	{
		printf("Malloc error, %lu bytes\n", size+1024);
		return -1;
	}
	switch(mode)
	{
	case STORE:
		unstore(size, dst, data);
		break;
	case ARJ_MODE_1:
	case ARJ_MODE_2:
	case ARJ_MODE_3:
	case GNU_ARJ_MODE_7:
		{ /* zero last two bytes */
			uint8_t temp_bytes[2];
			temp_bytes[0]=data[compressed_size];
			temp_bytes[1]=data[compressed_size+1];
			data[compressed_size]=0;
			data[compressed_size+1]=0;
			decode_m7(dst, data);
			data[compressed_size]=temp_bytes[0];
			data[compressed_size+1]=temp_bytes[1];
		}
		break;
	case ARJ_MODE_4:
		decode_m4(size, dst, data);
		break;
	case NI_MODE_0:
		decode_n0(dst, data);
		break;
	case NI_MODE_1:
		decode_n1(dst, data);
		break;
	case NI_MODE_2:
		decode_n2(dst, data);
		break;
	default:
		printf("Unknown method: %X", mode);
		break;
	}
	if((res_crc=crc32(size, dst, crc_table))==crc)
	{
		printf("CRC OK");
		result=0;
	}
	else
	{
		printf("CRC ERROR! :%08lX", (unsigned long) res_crc);
		error_count++;
	}
	free(dst);
	return result;
}

char *find_extension(char* filenaam)
{
	char *p=filenaam+strlen(filenaam);
	while (p>=filenaam)
	{
		if(*p=='.')
		{
			return p+1;
		}
		p--;
	}
	return NULL;
}

unsigned long get_original_size(uint8_t *data, unsigned long compressed_size, int mode)
{
	unsigned long original_size=0;
	switch(mode)
	{
	case STORE:
		original_size=compressed_size;
		break;
	case ARJ_MODE_1:
	case ARJ_MODE_2:
	case ARJ_MODE_3:
	case GNU_ARJ_MODE_7:
		original_size=decode_m7_size(data);
		break;
	case ARJ_MODE_4:
		original_size=decode_m4_size(compressed_size, data);
		break;
	case NI_MODE_0:
		original_size=decode_n0_size(data);
		break;
	case NI_MODE_1:
		original_size=decode_n1_size(data);
		break;
	case NI_MODE_2:
		original_size=decode_n2_size(data);
		break;
	default:
		printf("Unknown method: %X", mode);
		break;
	}
	return original_size;
}

void dump_on_extension(char* filenaam, uint8_t *data, unsigned long compressed_size, unsigned long size, int mode)
{ /* decode the data pointed to data */
	int result=-1;
	uint8_t *dst;
	uint32_t res_crc=0;
	dst=(uint8_t *)malloc(size+1024);
	if(dst==NULL)
	{
		printf("Malloc error, %s: %lu bytes\n", filenaam, size+1024);
		return;
	}
	switch(mode)
	{
	case STORE:
		unstore(size, dst, data);
		break;
	case ARJ_MODE_1:
	case ARJ_MODE_2:
	case ARJ_MODE_3:
	case GNU_ARJ_MODE_7:
		{ /* zero last two bytes */
			uint8_t temp_bytes[2];
			temp_bytes[0]=data[compressed_size];
			temp_bytes[1]=data[compressed_size+1];
			data[compressed_size]=0;
			data[compressed_size+1]=0;
			decode_m7(dst, data);
			data[compressed_size]=temp_bytes[0];
			data[compressed_size+1]=temp_bytes[1];
		}
		break;
	case ARJ_MODE_4:
		decode_m4(size, dst, data);
		break;
	case NI_MODE_0:
		decode_n0(dst, data);
		break;
	case NI_MODE_1:
		decode_n1(dst, data);
		break;
	case NI_MODE_2:
		decode_n2(dst, data);
		break;
	default:
		printf("Unknown method: %X", mode);
		free(dst);
		return;
	}
	res_crc=crc32(size, dst, crc_table);
	printf("%s CRC:%08lX\n", filenaam, (unsigned long) res_crc);
	{
		FILE* g;
		char *extension=find_extension(filenaam);
		if(extension!=NULL)
		{
			extension[-1]=0;
			g=fopen(filenaam, "wb");
			if(g!=NULL)
			{
				printf("Dump: %s\n", filenaam);
				fwrite(dst, size, 1, g);
				fclose(g);
			}
			else
			{
				printf("File open error: %s\n", filenaam);
			}
		}
		else
		{
			printf("No extension %s\n", filenaam);
		}
	}
	free(dst);
	return;
}


void depack_on_extension(char* filenaam, uint8_t *data, unsigned long compressed_size)
{
	char *extension=find_extension(filenaam);
	int mode;
	unsigned long original_size;
	if(extension==NULL)
	{ /* geen extensie gevonden */
		printf("File has no extension: %s\n", filenaam);
		return;
	}
	if(strcmp(extension, "m0")==0)
	{ /* m0 file */
		mode=STORE;
	}
	else if(strcmp(extension, "m4")==0)
	{ /* m4 file */
		mode=ARJ_MODE_4;
	}
	else if(strcmp(extension, "m7")==0)
	{ /* m7 file */
		mode=GNU_ARJ_MODE_7;
	}
	else if(strcmp(extension, "n0")==0)
	{ /* n0 file */
		mode=NI_MODE_0;
	}
	else if(strcmp(extension, "n1")==0)
	{ /* n1 file */
		mode=NI_MODE_1;
	}
	else if(strcmp(extension, "n2")==0)
	{ /* n2 file */
		mode=NI_MODE_2;
	}
	else
	{
		printf("Not supported extension: %s\n", extension);
		return;
	}
	original_size=get_original_size(data, compressed_size, mode);
	dump_on_extension(filenaam, data, compressed_size, original_size, mode);
}

void dump_arj(char* filenaam, uint8_t *data, unsigned long file_size)
{
	unsigned long offset;
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
			uint32_t crc32;
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
			printf(" %2s ", mode_to_str(method));
			printf(" %08lX ", (unsigned long)crc32);

			total_compressed_size+=compressed_size;
			total_original_size+=original_size;

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
			if(offset+compressed_size>=file_size)
			{
				printf("Unexpected end of data reached, aborting\n");
				break;
			}
			if(decode(method, original_size, compressed_size, crc32, data+offset)==0)
			{
				char *outfile;
				printf("\n");
				if((method==ARJ_MODE_1) || (method==ARJ_MODE_2) || (method==ARJ_MODE_3))
				{
					method=GNU_ARJ_MODE_7;
				}
				outfile=make_outfile_name(naam+file_naam_pos, method);
				if(outfile!=NULL)
				{
					FILE* g;
					g=fopen(outfile, "wb");
					if(g!=NULL)
					{
						printf("Dump: %s\n", outfile);
						fwrite(data+offset, compressed_size, 1, g);
						if(method==GNU_ARJ_MODE_7)
						{ /* add two 0 bytes */
							uint8_t zeroes[]={0,0,0,0};
							fwrite(zeroes, 2, 1, g);
						}
						fclose(g);
					}
					else
					{
						printf("File open error: %s\n", outfile);
					}
					free(outfile);
					outfile=NULL;
				}
			}
			else
			{
				printf("\n");
				printf("Skipped due to error: %s\n", naam+file_naam_pos);
			}
			offset+=compressed_size;
		}
		else
		{
			offset++;
		}
	}
	printf("\n%-20s", "totaal");
	printf(" %12lu ", total_original_size);
	printf(" %12lu               ", total_compressed_size);
	if(error_count==0)
	{
		printf("CRC OK\n");
	}
	else
	{
		printf("Errors = %lu\n", error_count);
	}
}

void handle_file(char *filenaam)
{
	uint8_t *data;
	char* extension;
	unsigned long file_size;
	FILE* f;
	f=fopen(filenaam, "rb");
	if(f==NULL)
	{
		printf("File open error %s", filenaam);
		return;
	}
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	data = (uint8_t *)malloc(file_size + 1024);
	if (data == NULL)
	{
		printf("Malloc error for %s data!\n", filenaam);
		fclose(f);
		return;
	}
	fseek(f, 0, SEEK_SET);
	if(fread(data, 1, file_size, f)!=file_size)
	{
		printf("Read error %s\n", filenaam);
	}
	fclose(f);
	extension=find_extension(filenaam);
	if(strcmp(extension, "arj")==0)
	{ /* dump arj file */
		dump_arj(filenaam, data, file_size);
	}
	else
	{ /* dump file */
		depack_on_extension(filenaam, data, file_size);
	}
	free(data);
}

int main(int argc, char *argv[])
{
	char *filenaam;
	int i;
	if(argc<2)
	{
		printf("Usage: %s <files to be dumped>\n\n", argv[0]);
		printf(
"gupdump <archive>.arj dumps all the files in the arj archive into separate\n"
"files. The extension of those files depends on the packing mode:\n"
"m1, m2, m3 and m7 files get the extension m7, they can all be depacked with\n"
"    the arj_m7 depack routines.\n"
"m4 files get the extension m4, they can be depacked with\n"
"    the arj_m4 depack routines.\n"
"n0 files get the extension n0\n"
"n1 files get the extension n1\n"
"n2 files get the extension n2\n"
"Those files can be depacked with the n0, n1 and n2 depack routines\n");
		printf(
"When you specify a .m4, .m7, .n0, .n1 or .n2 file as input file, gupdump will\n"
"depack the file and dump the resulting file in a file without the .m4, .m7,\n"
".n0, .n1 or .n2 extension fiving you the original uncompressed file.\n");		       
		return -1;
	}
	i=1;
	make_crc32_table(crc_table);
	while(i<argc)
	{
		filenaam=argv[i];
		handle_file(filenaam);
		i++;
	}
	return 0;
}


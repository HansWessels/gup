/*
  Experimentele delta compressie filter voor GNU ARJ
  (c) 1996 Hans Wessels
*/

#include <stdio.h>
#include <string.h>

#define BUFFSIZE 65536UL

char buffer[BUFFSIZE];
void convert(long count);

int main(int argc, char **argv)
{
  if(argc>0)
  {
    FILE * in;
    if((in=fopen(argv[1], "rb"))!=NULL)
    {
      FILE * out;
      char filenaam[1024];
      char *p;
      strcpy(filenaam, argv[1]);
      p=filenaam+strlen(filenaam);
      p[-1]='~';
      if((out=fopen(filenaam, "wb"))!=NULL)
      {
        long count;
        do
        {
          count=fread(buffer, 1, BUFFSIZE, in);
          convert(count);
          fwrite(buffer, 1, count, out);
        }
        while(count==BUFFSIZE);
        fclose(in);
        fclose(out);
      }
      else
      {
        printf("Outfile open error!\n");
      }
    }
    else
    {
      printf("In file open error!\n");
    }
  }
  else
  {
    printf("No filename!\n");
  }
  return 0;
}

static char delta=0;

void convert(long count)
{
  char *p=buffer;
  while(count--)
  {
    delta-=*p;
    *p++=delta;
  }
}

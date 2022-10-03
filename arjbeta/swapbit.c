/* code for swapbit table */

#include <stdio.h>

int main(void)
{
  FILE* f;
  if((f=fopen("swab_bit.txt", "w"))==NULL)
  {
    printf("File open error!\n");
    return -1;
  }
  fprintf(f, "{");
  {
    int i;
    for(i=0;i<256;i++)
    {
      int w=0;
      int j;
      int ml=1;
      int mh=0x80;
      if((i&7)==0)
      {
        fprintf(f, "\n ");
      }
      for(j=0;j<8;j++)
      {
        if((i&ml)!=0)
        {
          w|=mh;
        }
        ml<<=1;
        mh>>=1;
      }
      fprintf(f, " 0x%02x,", w);
    }
  }
  fprintf(f, "\n}\n");
  fclose(f);
  return 0;
}
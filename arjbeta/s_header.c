/*
  dumps header with compile date/time
*/

#include <stdio.h>
#include "gup.h"
#include "compress.h"


void showheader(void)
{
#if 0
	printf(SCREENINIT"              ARJ-BETA 9.97 REV 3.104 alpha\n"
				 "          (c) Copyright 1995-1996 Hans Wessels\n"
				 "                Release date: 30-11-1996.\n\n");
#else
  printf(SCREENINIT
         "                ARJ-BETA mode 7 werkversie\n"
         "             (c) Copyright 1995-1997 Hans Wessels\n"
         " Not released yet! Compile time: "__TIME__" date: "__DATE__"\n\n"
        );
#endif
}
 
/*
  output functies voor arj_beta
  1997-11-08
  HWS: added NEVER_USE(x) macro to surpress the warning: 
       never used parameter x in functions
  
  1997-11-05
  Hans Wessels: added arj internal error, to stay in sync with arj_error.h
*/

#include <stdio.h>
#include "gup.h"
#include "encode.h"
#include "port.h"
#include "arjbeta.h"

/* Print progress indicator */

unsigned long progress_current;
unsigned long progress_total;

/*
// Pretty Name Print
// Prints the filename in a TOS-crew approved manner.
*/
char* pnprint(int width, const char* name, char* buf)
{
  int len = (int)strlen(name);
  if(len<=width)
  {
    strcpy(buf, name);
  }
  else if(width <=0)
  {
    *buf=0;
  }
  else if(width <= 15)
  {
    strcpy(buf, name+len-width);
  }
  else if(width <=25) /* dit is een heel erg willekeurige getal */
  {
    sprintf(buf,"...%s", name+len+3-width);
  }
  else
  {
    int first = (width-5)/3;
    sprintf(buf,"%.*s ... %s", first, name, name+len-width+5+first);
  }
  return buf;
}

void init_progres(unsigned long size)
{
  progress_current=0;
  progress_total=(size/100)+1; /* +1 om division by zero te voorkomen */
}

void print_progres(unsigned long delta, void *propagator)
{
  NEVER_USE(propagator);
  printf("%3i%%\b\b\b\b",(int)((progress_current)/progress_total));
  progress_current+=delta;
  FLUSH();
}

void init_progres_size(unsigned long size)
{
  NEVER_USE(size);
  progress_current=0;
}

void print_progres_size(unsigned long delta, void *propagator)
{
  NEVER_USE(propagator);
  progress_current+=delta;
  if(progress_current<1000)
  {
    printf("%3i \b\b\b\b",(int)progress_current);
  }
  else if(progress_current<10240L)
  {
    printf("%1ik%02i\b\b\b\b",(int)(progress_current>>10),(int)(100*(progress_current&0x3ff)>>10));
  }
  else if(progress_current<102400L)
  {
    printf("%2ik%1i\b\b\b\b",(int)(progress_current>>10),(int)(10*(progress_current&0x3ff)>>10));
  }
  else if(progress_current<1024000L)
  {
    printf("%3ik\b\b\b\b",(int)(progress_current>>10));
  }
  else if(progress_current<(1024L*10240L))
  {
    printf("%1iM%02i\b\b\b\b",(int)(progress_current>>20),(int)(100*((progress_current>>10)&0x3ff)>>10));
  }
  else if(progress_current<(1024L*102400L))
  {
    printf("%2iM%1i\b\b\b\b",(int)(progress_current>>20),(int)(10*((progress_current>>10)&0x3ff)>>10));
  }
  else if(progress_current<(1024L*1024000L))
  {
    printf("%3iM\b\b\b\b",(int)(progress_current>>20));
  }
  else /* if(progress_current<(1024L*1024*10240L)) */
  {
    printf("%1iG%02i\b\b\b\b",(int)(progress_current>>30),(int)(100*((progress_current>>20)&0x3ff)>>10));
  }
  FLUSH();
}

const char *arj_error_disc[]=
{
	/*
	 * General result codes.
	 */

	"No error",
	"Internal error.",
	"Unknown error, probably a C error code not recognized by 'gup_conv_err'.",

	/*
	 * File I/O errors.
	 */

	"Can't read file.",
	"File write error.",
	"Unexpected end of file.",
	"Can not create directory.",
	"Unable to create temporary file name.",

	/*
	 * C error codes.
	 */

	"Operation not permitted.",
	"Access denied.",
	"File or directory not found.",
	"General I/O error.",
	"No such device or address.",
	"Executable format error.",
	"Bad file handle.",
	"Out of memory.",
	"Bad (memory block) address.",
	"Device or resource busy.",
	"File already exists.",
	"Cross device link.",
	"No such device.",
	"Not a directory.",
	"Is a directory.",
	"Invalid internal parameter.",
	"File table overflow.",
	"Too many open files.",
	"Text file busy.",
	"File too large.",
	"No space left on device.",
	"Illegal seek.",
	"Read only file system.",
	"Too many links.",
	"Broken pipe.",
	"File name too long.",
	"Directory is not empty.",
	"Too many symbolic links.",
	"Device is not a stream.",
	
	/*
	 * Resultcodes of decode.c
	 */

	"Error in first huffman table.",
	"Error in second huffman table.",
	"Error in third huffman table.",

	/*
	 * Warnings of init_encode
	 */

	"Warning: Insufficient memory for -jm mode!",
	"Warning: Insufficient memory for fast mode!",
	"Warning: Insufficient memory for medium mode!",
	"Warning: Insufficient memory for linking!",
	"Warning: Insufficient memory for sieve mode!",
	"Warning: Using a smaller dictionary!",

	/*
	 * Errors in archives.
	 */

	"File is not a valid archive.",
	"Archive broken.",
	"Archive severly damaged.",
	"CRC error in file.",

	"Result code: Continue with next volume.",
	"Result code: End of archive reached.",

	/*
	 * Errors in headers.
	 */

	"CRC error in a header.",
	"Unknown method in header.",
	"Unknown OS id in header.",
	"Header not found at expected position.",

	/*
	 * Last of the errors
	 */

};


void display_error(gup_result err)
{ /* displays arj_result errors */
  if(err>=GUP_LAST_ERROR)
  {
    printf("Unkown error!\n");
  }
  else
  {
    printf("%s\n",arj_error_disc[err]);
  }
}

void init_message(gup_result message, void* propagator)
{
  NEVER_USE(propagator);
  display_error(message);
}
 
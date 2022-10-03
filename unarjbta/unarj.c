/* UNARJ.C, UNARJ, R JUNG, 02/17/93
 * Main Extractor routine
 */

#include "unarj.h"
#include "..\arjbeta\file_buf.h"
#include "..\include\arj_err.h"
#include "..\arjbeta\arj_outp.h"
#include "..\include\compr_io.h"
#include "..\include\crc.h"
#include "..\compress\st_opti\st_opti.h"
#include "..\include\decode.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define BUFFER_SIZE (8*65536UL)
#define NEVER_USE(x) (void) x

/* Global variables */

UCRC   crc;
FILE   *outfile;
ushort bitbuf;
long   compsize;
long   origsize;
uchar  subbitbuf;
uchar  header[HEADERSIZE_MAX];
char   arc_name[FNAME_MAX];
int    command;
int    bitcount;
int    file_type;
int    no_output;
int    error_count;

uint8 buffer[BUFFER_SIZE]; /* file buffer */
void   *table;             /* arj crc table */
decode_struct xcom;        /* command struct */

/* Messages */

static char *M_USAGE  [] =
{
"Usage:  UNARJ archive[.arj]    (list archive)\n",
"        UNARJ e archive        (extract archive)\n",
"        UNARJ l archive        (list archive)\n",
"        UNARJ t archive        (test archive)\n",
"        UNARJ x archive        (extract with pathnames)\n",
"\n",
"This is an ARJ demonstration program and ** IS NOT OPTIMIZED ** for speed.\n",
"You may freely use, copy and distribute this program, provided that no fee\n",
"is charged for such use, copying or distribution, and it is distributed\n",
"ONLY in its original unmodified state.  UNARJ is provided as is without\n",
"warranty of any kind, express or implied, including but not limited to\n",
"the implied warranties of merchantability and fitness for a particular\n",
"purpose.  Refer to UNARJ.DOC for more warranty information.\n",
"\n",
"ARJ Software                    Internet address :  robjung@world.std.com\n",
"Robert and Susan Jung           CompuServe userid:  72077,445\n",
"2606 Village Road West\n",
"Norwood, Massachusetts 02062\n",
"USA\n",
NULL
};

char M_VERSION [] = "UNARJ (Demo version) 2.41 Copyright (c) 1991-93 Robert K Jung\n\n";

char M_ARCDATE [] = "Archive created: %s";
char M_ARCDATEM[] = ", modified: %s";
char M_BADCOMND[] = "Bad UNARJ command: %s";
char M_BADCOMNT[] = "Invalid comment header";
char M_BADHEADR[] = "Bad header";
char M_BADTABLE[] = "Bad Huffman code";
char M_CANTOPEN[] = "Can't open %s";
char M_CANTREAD[] = "Can't read file or unexpected end of file";
char M_CANTWRIT[] = "Can't write file. Disk full?";
char M_CRCERROR[] = "CRC error!\n";
char M_CRCOK   [] = "CRC OK\n";
char M_DIFFHOST[] = "  Binary file!";
char M_ENCRYPT [] = "File is password encrypted, ";
char M_ERRORCNT[] = "%sFound %5d error(s)!";
char M_EXTRACT [] = "Extracting %-25s";
char M_FEXISTS [] = "%-25s exists, ";
char M_HEADRCRC[] = "Header CRC error!";
char M_NBRFILES[] = "%5d file(s)\n";
char M_NOMEMORY[] = "Out of memory";
char M_NOTARJ  [] = "%s is not an ARJ archive";
char M_PROCARC [] = "Processing archive: %s\n";
char M_SKIPPED [] = "Skipped %s\n";
char M_SUFFIX  [] = ARJ_SUFFIX;
char M_TESTING [] = "Testing    %-25s";
char M_UNKNMETH[] = "Unsupported method: %d, ";
char M_UNKNTYPE[] = "Unsupported file type: %d, ";
char M_UNKNVERS[] = "Unsupported version: %d, ";

#define get_crc()       get_longword()
#define fget_crc(f)     fget_longword(f)

#define setup_get(PTR)  (get_ptr = (PTR))
#define get_byte()      ((uchar)(*get_ptr++ & 0xff))

/* Local functions */

static FILE* fopen_msg(char *name, char *mode);
static uint8 bufget_byte(buf_fhandle_t *f);
static uint  fget_word(buf_fhandle_t *f);
static ulong fget_longword(buf_fhandle_t *f);
static void  fread_crc(uchar *p, int n, buf_fhandle_t *f);
static void  decode_path(char *name);
static void  get_date_str(char *str, ulong tstamp);
static int   parse_path(char *pathname, char *path, char *entry);
static void  strncopy(char *to, char *from, int len);
static uint  get_word(void);
static ulong get_longword(void);
static long  find_header(buf_fhandle_t* fd);
static int   read_header(int first, buf_fhandle_t *fd, char *name);
static void  skip(void);
static int   check_flags(void);
static int   extract(void);
static int   test(void);
static uint  ratio(long a, long b);
static void  list_start(void);
static void  list_arc(int count);
static void  execute_cmd(void);
static void  help(void);

/* Local variables */

static char   filename[FNAME_MAX];
static char   comment[COMMENT_MAX];
static char   *hdr_filename;
static char   *hdr_comment;

static ushort headersize;
static uchar  first_hdr_size;
static uchar  arj_nbr;
static uchar  arj_x_nbr;
static uchar  host_os;
static uchar  arj_flags;
static short  method;
static uint   file_mode;
static ulong  time_stamp;
static short  entry_pos;
static ushort host_data;
static uchar  *get_ptr;
static UCRC   file_crc;
static UCRC   header_crc;

static long   first_hdr_pos;
static long   torigsize;
static long   tcompsize;

static int    clock_inx;

static char   *writemode[2]  = { "wb",  "w" };

/* Functions */

arj_result buff_fill(buf_fhandle_t* com, void* propagator)
{
  NEVER_USE(propagator);
  buf_fill(com);
  return ARJ_OK;
}

arj_result write_crc(long count , void* buf, void* propagator)
{
  crc=arj_crc(buf, count, crc, table);
  fwrite(buf, 1, count, propagator);
  return ARJ_OK;
}

arj_result not_write_crc(long count , void* buf, void* propagator)
{
  crc=arj_crc(buf, count, crc, table);
  NEVER_USE(propagator);
  return ARJ_OK;
}

void crc_buf(char *str, unsigned long len)
{
  crc=arj_crc(str, len, crc, table);
}

void* xmalloc(unsigned long size, void* propagator)
{
  NEVER_USE(propagator);
  return malloc(size);
}

void xfree(void* ptr, void* propagator)
{
  NEVER_USE(propagator);
  free(ptr);
}

void error(char *fmt, char *arg)
{
  putc('\n', stdout);
  printf(fmt, arg, error_count);
  putc('\n', stdout);
  exit(EXIT_FAILURE);
}

static FILE *fopen_msg(char *name, char *mode)
{
  FILE *fd;

  fd = file_open(name, mode);
  if (fd == NULL)
  {
    error(M_CANTOPEN, name);
  }
  return fd;
}

static void fread_crc(uchar *p, int n, buf_fhandle_t *f)
{
  int i=n;
  char *q=p;
  while(--i>=0)
  {
    *q++=bufget_byte(f);
  }
  origsize += n;
  crc_buf((char *)p, n);
}

static uint fget_word(buf_fhandle_t *f)
{
  uint b0, b1;

  b0 = bufget_byte(f);
  b1 = bufget_byte(f);
  return (b1 << 8) + b0;
}

static ulong fget_longword(buf_fhandle_t *f)
{
  ulong b0, b1, b2, b3;

  b0 = bufget_byte(f);
  b1 = bufget_byte(f);
  b2 = bufget_byte(f);
  b3 = bufget_byte(f);
  return (b3 << 24) + (b2 << 16) + (b1 << 8) + b0;
}


static void decode_path(char *name)
{
  for ( ; *name; name++)
  {
    if (*name == ARJ_PATH_CHAR)
    {
      *name = PATH_CHAR;
    }
  }
}

static void get_date_str(char *str, ulong tstamp)
{
  sprintf(str, "%04u-%02u-%02u %02u:%02u:%02u",
          ts_year(tstamp), ts_month(tstamp), ts_day(tstamp),
          ts_hour(tstamp), ts_min(tstamp), ts_sec(tstamp));
}

static int parse_path(char *pathname, char *path, char *entry)
{
  char *cptr, *ptr, *fptr;
  short pos;

  fptr = NULL;
  for (cptr = PATH_SEPARATORS; *cptr; cptr++)
  {
    if ((ptr = strrchr(pathname, *cptr)) != NULL &&
            (fptr == NULL || ptr > fptr))
    {
      fptr = ptr;
    }
  }
  if (fptr == NULL)
  {
    pos = 0;
  }
  else
  {
    pos = fptr + 1 - pathname;
  }
  if (path != NULL)
  {
    strncpy(path, pathname, pos);
    path[pos] = NULL_CHAR;
  }
  if (entry != NULL)
  {
    strcpy(entry, &pathname[pos]);
  }
  return pos;
}

static void strncopy(char *to, char *from, int  len)
{
  int i;

  for (i = 1; i < len && *from; i++)
  {
    *to++ = *from++;
  }
  *to = NULL_CHAR;
}

void strlower(char *s)
{
  while (*s)
  {
    *s = (char) tolower(*s);
    s++;
  }
}

void strupper(char *s)
{
  while (*s)
  {
    *s = (char) toupper(*s);
    s++;
  }
}

static uint get_word()
{
  uint b0, b1;

  b0 = get_byte();
  b1 = get_byte();
  return (b1 << 8) + b0;
}

static ulong get_longword()
{
  ulong b0, b1, b2, b3;

  b0 = get_byte();
  b1 = get_byte();
  b2 = get_byte();
  b3 = get_byte();
  return (b3 << 24) + (b2 << 16) + (b1 << 8) + b0;
}

static long find_header(buf_fhandle_t* fd)
{
  long arcpos, lastpos;
  int c;

  arcpos = buf_tell(fd);
  buf_seek_read(0L, SEEK_END, fd);
  lastpos = buf_tell(fd) - 2;
  if (lastpos > MAXSFX)
  {
    lastpos = MAXSFX;
  }
  for ( ; arcpos < lastpos; arcpos++)
  {
    buf_seek_read(arcpos, SEEK_SET, fd);
    c = bufget_byte(fd);
    while (arcpos < lastpos)
    {
      if (c != HEADER_ID_LO)  /* low order first */
      {
        c = bufget_byte(fd);
      }
      else 
      {
        if ((c = bufget_byte(fd)) == HEADER_ID_HI)
        {
          break;
        }
      }
      arcpos++;
    }
    if (arcpos >= lastpos)
    {
      break;
    }
    if ((headersize = fget_word(fd)) <= HEADERSIZE_MAX)
    {
      crc = CRC_MASK;
      fread_crc(header, (int) headersize, fd);
      if ((crc ^ CRC_MASK) == fget_crc(fd))
      {
        buf_seek_read(arcpos, SEEK_SET, fd);
        return arcpos;
      }
    }
  }
  return -1;          /* could not find a valid header */
}

static int read_header(int  first, buf_fhandle_t *fd, char *name)
{
  ushort extheadersize, header_id;

  header_id = fget_word(fd);
  if (header_id != HEADER_ID)
  {
    if (first)
    {
      error(M_NOTARJ, name);
    }
    else
    {
      error(M_BADHEADR, "");
    }
  }

  headersize = fget_word(fd);
  if (headersize == 0)
  {
    return 0;               /* end of archive */
  }
  if (headersize > HEADERSIZE_MAX)
  {
    error(M_BADHEADR, "");
  }
  crc = CRC_MASK;
  fread_crc(header, (int) headersize, fd);
  header_crc = fget_crc(fd);
  if ((crc ^ CRC_MASK) != header_crc)
  {
    error(M_HEADRCRC, "");
  }

  setup_get(header);
  first_hdr_size = get_byte();
  arj_nbr = get_byte();
  arj_x_nbr = get_byte();
  host_os = get_byte();
  arj_flags = get_byte();
  method = get_byte();
  file_type = get_byte();
  (void)get_byte();
  time_stamp = get_longword();
  compsize = get_longword();
  origsize = get_longword();
  file_crc = get_crc();
  entry_pos = get_word();
  file_mode = get_word();
  host_data = get_word();
  hdr_filename = (char *)&header[first_hdr_size];
  strncopy(filename, hdr_filename, (int)sizeof(filename));
  if ((arj_flags & PATHSYM_FLAG) != 0)
  {
    decode_path(filename);
  }
  hdr_comment = (char *)&header[first_hdr_size + strlen(hdr_filename) + 1];
  strncopy(comment, hdr_comment, (int)sizeof(comment));
  /* if extheadersize == 0 then no CRC */
  /* otherwise read extheader data and read 4 bytes for CRC */

  while ((extheadersize = fget_word(fd)) != 0)
  {
    buf_seek_read((long) (extheadersize + 4), SEEK_CUR, fd);
  }

  return 1;                   /* success */
}

static void skip()
{
  buf_seek_read(compsize, SEEK_CUR, xcom.br_buf);
}

static int check_flags()
{
  if (arj_x_nbr > ARJ_X_VERSION)
  {
    printf(M_UNKNVERS, arj_x_nbr);
    printf(M_SKIPPED, filename);
    skip();
    return -1;
  }
  if ((arj_flags & GARBLE_FLAG) != 0)
  {
    printf(M_ENCRYPT);
    printf(M_SKIPPED, filename);
    skip();
    return -1;
  }
  if (method < 0 || method > MAXMETHOD || (method == 4 && arj_nbr == 1))
  {
    printf(M_UNKNMETH, method);
    printf(M_SKIPPED, filename);
    skip();
    return -1;
  }
  if (file_type != BINARY_TYPE && file_type != TEXT_TYPE)
  {
    printf(M_UNKNTYPE, file_type);
    printf(M_SKIPPED, filename);
    skip();
    return -1;
  }
  return 0;
}

static int extract()
{
  char name[FNAME_MAX];

  if (check_flags())
  {
    error_count++;
    return 0;
  }

  no_output = 0;
  if (command == 'E')
  {
    strcpy(name, &filename[entry_pos]);
  }
  else
  {
    strcpy(name, DEFAULT_DIR);
    strcat(name, filename);
  }

  if (host_os != OS)
  {
    default_case_path(name);
  }

  if (file_exists(name))
  {
    printf(M_FEXISTS, name);
    printf(M_SKIPPED, name);
    skip();
    error_count++;
    return 0;
  }
  outfile = file_open(name, writemode[file_type & 1]);
  if (outfile == NULL)
  {
    printf(M_CANTOPEN, name);
    putchar('\n');
    skip();
    error_count++;
    return 0;
  }
  else
  {
    xcom.wc_propagator=outfile;
  }
  printf(M_EXTRACT, name);
  if (host_os != OS && file_type == BINARY_TYPE)
  {
    printf(M_DIFFHOST);
  }
  printf("  ");

  crc = CRC_MASK;

  if (method == 0 || method == 1 || method == 2 
        || method == 3 || method == 4 || method == 7 )
  {
    long currentpos;
    unsigned long thesize=compsize;

    currentpos = buf_tell(xcom.br_buf);
    xcom.mode=method;
    xcom.origsize=origsize;
    {
      arj_result err;
      init_progres(origsize);
      err=decode(&xcom);
      if(err!=ARJ_OK)
      {
        display_error(err);
      }
    }
    buf_seek_read(currentpos+thesize, SEEK_SET, xcom.br_buf);
  }
  else
  {
    printf("Unknown method|\n");
  }
  fclose(outfile);

  set_ftime_mode(name, time_stamp, file_mode, (uint) host_os);

  if ((crc ^ CRC_MASK) == file_crc)
  {
    printf(M_CRCOK);
  }
  else
  {
    printf(M_CRCERROR);
    error_count++;
  }
  return 1;
}

static int test(void)
{
  if (check_flags())
  {
    return 0;
  }

  no_output = 1;
  printf(M_TESTING, filename);
  printf("  ");

  crc = CRC_MASK;

  if (method == 0 || method == 1 || method == 2 
      || method == 3 || method == 4 || method == 7 )
  {
    long currentpos;
    unsigned long thesize=compsize;

    currentpos = buf_tell(xcom.br_buf);
    xcom.mode=method;
    xcom.origsize=origsize;
    {
      arj_result err;
      init_progres(origsize);
      err=decode(&xcom);
      if(err!=ARJ_OK)
      {
        display_error(err);
      }
    }
    buf_seek_read(currentpos+thesize, SEEK_SET, xcom.br_buf);
  }
  else
  {
    printf("Unknown method!\n");
  }

  if ((crc ^ CRC_MASK) == file_crc)
  {
    printf(M_CRCOK);
  }
  else
  {
    printf(M_CRCERROR);
    error_count++;
  }
  return 1;
}

static uint ratio(long a, long b)
{
  int i;

  for (i = 0; i < 3; i++)
  {
    if (a <= LONG_MAX / 10)
    {
      a *= 10;
    }
    else
    {
      b /= 10;
    }
  }
  if ((long) (a + (b >> 1)) < a)
  {
    a >>= 1;
    b >>= 1;
  }
  if (b == 0)
  {
    return 0;
  }
  return (uint) ((a + (b >> 1)) / b);
}

static void list_start()
{
  printf("Filename       Original Compressed Ratio DateTime modified CRC-32   AttrBTPMGVX\n");
  printf("------------ ---------- ---------- ----- ----------------- -------- -----------\n");
}

static void list_arc(int count)
{
  uint r;
  int garble_mode, path_mode, volume_mode, extfil_mode, ftype, bckf_mode;
  char date_str[20], fmode_str[10];
  static char mode[5] = { 'B', 'T', '?', 'D', 'V' };
  static char pthf[2] = { ' ', '+' };
  static char pwdf[2] = { ' ', 'G' };  /* plain, encrypted */
  static char volf[2] = { ' ', 'V' };
  static char extf[2] = { ' ', 'X' };
  static char bckf[2] = { ' ', '*' };

  if (count == 0)
  {
    list_start();
  }

  garble_mode = ((arj_flags & GARBLE_FLAG) != 0);
  volume_mode = ((arj_flags & VOLUME_FLAG) != 0);
  extfil_mode = ((arj_flags & EXTFILE_FLAG) != 0);
  bckf_mode   = ((arj_flags & BACKUP_FLAG) != 0);
  path_mode   = (entry_pos > 0);
  r = ratio(compsize, origsize);
  torigsize += origsize;
  tcompsize += compsize;
  ftype = file_type;
  if (ftype != BINARY_TYPE && ftype != TEXT_TYPE && ftype != DIR_TYPE && ftype != LABEL_TYPE)
  {
    ftype = 3;
  }
  get_date_str(date_str, time_stamp);
  strcpy(fmode_str, "    ");
  if (host_os == OS)
  {
    get_mode_str(fmode_str, (uint) file_mode);
  }
  if (strlen(&filename[entry_pos]) > 12)
  {
    printf("%-12s\n             ", &filename[entry_pos]);
  }
  else
  {
    printf("%-12s ", &filename[entry_pos]);
  }
  printf("%10ld %10ld %u.%03u %s %08lX %4s%c%c%c%u%c%c%c\n",
        origsize, compsize, r / 1000, r % 1000, &date_str[2], file_crc,
        fmode_str, bckf[bckf_mode], mode[ftype], pthf[path_mode], method,
        pwdf[garble_mode], volf[volume_mode], extf[extfil_mode]);
}


static void execute_cmd(void)
{
  int file_count;
  char date_str[22];
  uint r;

  first_hdr_pos = 0;
  time_stamp = 0;
  first_hdr_size = FIRST_HDR_SIZE;

  if(command=='T')
  {
    xcom.write_crc=not_write_crc;
  }
  else
  {
    xcom.write_crc=write_crc;
  }
  xcom.br_buf = buf_open_read(arc_name, buffer, BUFFER_SIZE);

  printf(M_PROCARC, arc_name);

  first_hdr_pos = find_header(xcom.br_buf);
  if (first_hdr_pos < 0)
  {
    error(M_NOTARJ, arc_name);
  }
  buf_seek_read(first_hdr_pos, SEEK_SET, xcom.br_buf);
  if (!read_header(1, xcom.br_buf, arc_name))
  {
    error(M_BADCOMNT, "");
  }
  get_date_str(date_str, time_stamp);
  printf(M_ARCDATE, date_str);
  if (arj_nbr >= ARJ_M_VERSION)
  {
    get_date_str(date_str, (ulong) compsize);
    printf(M_ARCDATEM, date_str);
  }
  printf("\n");

  file_count = 0;
  while (read_header(0, xcom.br_buf, arc_name))
  {
    switch (command)
    {
    case 'E':
    case 'X':
      if (extract())
      {
        file_count++;
      }
      break;
    case 'L':
      list_arc(file_count++);
      skip();
      break;
    case 'T':
      if (test())
      {
        file_count++;
      }
      break;
    }
  }

  if (command == 'L')
  {
    printf("------------ ---------- ---------- ----- -----------------\n");
    r = ratio(tcompsize, torigsize);
    printf(" %5d files %10ld %10ld %u.%03u %s\n",
           file_count, torigsize, tcompsize, r / 1000, r % 1000, &date_str[2]);
  }
  else
  {
    printf(M_NBRFILES, file_count);
  }

  buf_close_read(xcom.br_buf);
}

static void help(void)
{
  int i;

  for (i = 0; M_USAGE[i] != NULL; i++)
  {
    printf(M_USAGE[i]);
  }
}

int main(int argc, char *argv[])
{
  long start = clock();
  int i, j, lastc;
  char *arc_p;
  
  xcom.xmalloc=xmalloc;
  xcom.xm_propagator=NULL;
  xcom.xfree=xfree;
  xcom.xf_propagator=NULL;
  xcom.print_progres=print_progres;
  xcom.pp_propagator=0;

  printf(M_VERSION);
  if(init_decode(&xcom)!=ARJ_OK)
  {
    printf("Error mallocing decode struct!\n");
    return -1;
  }
  table=make_arj_crc_table();
  xcom.buf_fill=buff_fill;

  if (argc == 1)
  {
    help();
    return EXIT_SUCCESS;
  }
  else
  {
    if (argc == 2)
    {
      command = 'L';
      arc_p = argv[1];
    }
    else
    {
      if (argc == 3)
      {
        if (strlen(argv[1]) > 1)
        {
          error(M_BADCOMND, argv[1]);
        }
        command = toupper(*argv[1]);
        if (strchr("ELTX", command) == NULL)
        {
          error(M_BADCOMND, argv[1]);
        }
        arc_p = argv[2];
      }
      else
      {
        help();
        return EXIT_FAILURE;
      }
    }
  }
  strncopy(arc_name, arc_p, FNAME_MAX);
  case_path(arc_name);
  i = (int)strlen(arc_name);
  j = parse_path(arc_name, (char *)NULL, (char *)NULL);
  lastc = arc_name[i - 1];
  if (lastc == ARJ_DOT)
  {
    arc_name[i - 1] = NULL_CHAR;
  }
  else
  {
    if (strchr(&arc_name[j], ARJ_DOT) == NULL)
    {
      strcat(arc_name, M_SUFFIX);
    }
  }
 
  error_count = 0;
  clock_inx = 0;
  outfile = NULL;

  execute_cmd();

  if (error_count > 0)
  {
    error(M_ERRORCNT, "");
  }

  free_arj_crc_table(table);
  free_decode(&xcom);
  printf("\n  Time used %lu sec.", (clock() - start) / CLK_TCK);
  return EXIT_SUCCESS;
}

/* end UNARJ.C */

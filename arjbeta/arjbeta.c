/*
 * ARJBETA (c) 1994 Mr Ni! (the Great) of the TOS-crew
 * 
 */


#include <time.h>
#include <tos.h>
#include "gup.h"
#include "arjbeta.h"
#include "compr_io.h"
#include "file_buf.h"
#include "port.h"
#include "crc.h"
#include "scan.h"
#include "arj_outp.h"
#include "arj_hdr.h"
#include "lha_hdr.h"
#include "gz_hdr.h"

#define FILE_NAME_SIZE 256


packstruct command;
commandstruct xcommand;

long fread_crc(long count, const void *buf, void* /* read_crc_struct* */ comm)
{
  read_crc_struct* com=comm;
  long res=	Fread(com->handle, count, buf);
  if(res>0)
  {
    com->crc=com->crc_func(buf, res, com->crc, com->crc_table);
  }
	return res;
}

/* routine for writing the arj file */
gup_result main_buf_write_announce (long count, buf_fhandle_t* bw_buf
                                   ,void* bw_propagator)
{
  NEVER_USE(bw_propagator);
  return buf_write_announce(count, bw_buf);
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

/*
 * Aanroep: filename[.ARJ] filename [<filename>]
*/
int main(int argc, char **argv)
{
  long start = clock();
  int i;
  char show_usage = 0;
  int returnval = 0;

  showheader();

  /* get environment, nmbr of columns */
  {
    int columns = 0;
    char* p=getenv("COLUMNS");
    if(p != NULL)
    {
      columns = (int)strtoul(p, NULL, 10);
    }
    if(columns <= 0)
    {
      int res=Getrez();
      switch(res)
      {
      case 1:
      case 2:
      case 4:
      default:
        columns = 80;
        break;
      case 0:
      case 7:
        columns = 40;
        break;
      case 6:
        columns = 160;
        break;
      }
    }
    xcommand.columns = columns;
    xcommand.strbuf = malloc(columns+2);
  }

  /*
   * support:
   * 
   * -v[{value}] -jm -m{0..7}
  */
  /*
   * First: sort all argv[] entries: all options first! Stop 'sorting'
   * when '--' is located.
  */
  {
    int cnt = argc;
    const char **argvptr = argv + 1;
    const char **argvswapptr = argv + 1;

    while (--cnt)
    {
      if (argvptr[0][0] == '-')
      {
        if (argvptr[0][1] == '-')
          break;                       /* stop when '--' is found. */
        argvptr++;
        argvswapptr++;
      }
      else
      {
        int tempcnt = cnt++;           /* *argvswapptr will be checked again after this. */

        /*
         * find next '-' option...
        */
        while (--tempcnt)
        {
          argvptr++;
          if (argvptr[0][0] == '-')
          {
            tempcnt = 1;               /* make sure tempcnt != 0 */
            break;
          }
        }
        if (!tempcnt)
          break;                       /* end of argv[] list found without locating aditional '-' option. */
        /*
         * success: argvptr points to first next '-' option. --> move
         * pointer to front.
        */
        {
          const char *tempptr = *argvptr;

          memcpy((void *)(argvswapptr + 1), (void *)(argvswapptr), (argvptr - argvswapptr) * sizeof (*argvptr));
          *argvswapptr = tempptr;
          /*
           * swap  complete!
          */
          argvptr = argvswapptr;
        }
      }
    }
  }
  /*
   * argv[] list has all options up front, terminated by non-option or
   * '--'.
  */

  /*
   * set defaults.
  */
  command.jm = 1;                      /* standaard jm en mode 7 */
  command.mode = 7;
  command.speed = 0;                   /* maximum compression */
  command.mv_mode = 0;
  command.bufbase = NULL;
  xcommand.add_mode=0;                 /* create */
  xcommand.path=1;
  xcommand.extension=1;
  command.print_progres=print_progres;
  command.pp_propagator=NULL;
  command.init_message=init_message;
  command.im_propagator=NULL;
  command.gmalloc=xmalloc;
  command.gm_propagator=NULL;
  command.gfree=xfree;
  command.gf_propagator=NULL;
  command.buf_write_announce=main_buf_write_announce;
  command.buf_read_crc=fread_crc;
  command.brc_propagator=&xcommand.rc;
  /*
   * parse arguments
  */
  argv++;
  while (--argc)
  {
    if (argv[0][0] != '-')
    {
      if(tolower(argv[0][0])== 'a')
      {
        xcommand.add_mode=1;   /* add to archive */
      }
      else
      {
        break;
      }
    }
    else
    {
      if (argv[0][1] == '-')
      {
        argc--;
        argv++;
        break;                           /* '--': stop this! Skip this argv[] entry! */
      }
      /*
       * option follows...
      */
    	{
	      const char *option = argv[0] + 1;
	
	      while (*option)
	      {
	        switch (tolower(*option))
	        {
	        default:
	          printf("Unknown option [%c] in parameter \"%s\".\n"
	                 "Ignoring parameter.\n\n",
	                 (*option ? *option : ' '),
	                 argv[0]);
	          option += strlen(option);
	          show_usage = !0;
	          returnval = 1;
	          break;
	#if 0
	        case 'x':  /* internal memory eater */
	          {
	            unsigned long waarde = 0;

	            waarde = strtoul(option + 1, NULL, 10);
	            waarde<<=10;
	            option += strlen(option);
	            if(malloc(waarde)==NULL)
	            {
	              printf("Out of mem!\n");
	              exit(-1);
	            }
	            break;
	          }
	#endif
	        case 'v':
	          {
	            unsigned long waarde = 0;
	
	            waarde = strtoul(option + 1, NULL, 10);
	            if (waarde < 8192)
	            {
	              switch ((int)waarde)
	              {
	              case 180:
	                waarde = 177152UL;
	                break;
	              case 200:
	                waarde = 197632UL;
	                break;
	              case 205:
	                waarde = 197632UL + 5 * 1024;
	                break;
	              case 360:
	                waarde = 360448UL;
	                break;
	              case 400:
	                waarde = 401408UL;
	                break;
	              case 405:
	                waarde = 401408UL + 5 * 1024;
	                break;
	              case 410:
	                waarde = 401408UL + 10 * 1024;
	                break;
	              case 720:
	                waarde = 728064UL;
	                break;
	              case 800:
	                waarde = 809984UL;
	                break;
	              case 820:
	                waarde = 830464UL;
	                break;
	              case 1440:
	                waarde = 1456640UL;
	                break;
	              case 1600:
	                waarde = 1623040UL;
	                break;
	              case 1620:
	                waarde = 1664000UL;
	                break;
	              case 2880:
	                waarde = 1456640UL * 2;
	                break;
	              case 3200:
	                waarde = 1623040UL * 2;
	                break;
	              case 3240:
	                waarde = 1664000UL * 2;
	                break;
	              default:
	                if (waarde!=0)
	                {
	                  waarde = 1456640UL;
	                  break;
	                }
	                else
	                {
	                  (void)0;
	                  /*
	                   * waarde = 0;
	                   * 
	                   * Check if target-archive has disk specified. If so,
	                   * try to determine disksize for multiple volume
	                   * size.
	                   */
	                }
	                break;
	              }
	            }
	            command.mv_mode = 1;
	            command.mv_next = 0;
	            xcommand.mv_nr = 0;
	            xcommand.mv_size = waarde;
	
	            /*
	             * correct 'option' pointer for int value!
	             */
	            option += strlen(option);
	          }
	          break;
	
	        case 'j':
	          option++;
	          switch (*option)
	          {
	          default:
	            printf("Unknown option [%c] in parameter \"%s\".\n"
	                   "Ignoring parameter.\n\n",
	                   (*option ? *option : ' '),
	                   argv[0]);
	            option += strlen(option);
	            show_usage = !0;
	            returnval = 1;
	            break;
	            case 'm':
	            option++;
	            if(*option=='-')
	            {
	              command.jm = 0;
	              option++;
	              break;
	            }
	            else
	            {
	              command.jm = 1;
	              break;
	            }
	          }
	          break;

		      case 'm':
	          option++;
	          switch (*option)
	          {
	          default:
	            printf("Unknown option [%c] in parameter \"%s\".\n"
	                   "Ignoring parameter.\n\n",
	                   (*option ? *option : ' '),
	                   argv[0]);
	            option += strlen(option);
	            show_usage = !0;
	            returnval = 1;
	            break;

            case '0':
	            command.mode = 0;
	            option++;
	            break;
	          case '1':
	            command.mode = 1;
	            option++;
	            break;
	          case '2':
	            command.mode = 2;
	            command.speed=1;
	            option++;
	            break;
	          case '3':
	            command.mode = 3;
	            command.speed=2;
	            option++;
	            break;
	          case '4':
	            command.mode = 4;
	            option++;
	            break;
	          case '5':                    /* voorlopig de ingang voor lha 5 */
	            command.mode = LHA_LH5_;
	            option++;
	            break;
	          case '6':                    /* voorlopig de ingang voor lha 6 */
	            command.mode = LHA_LH6_;
	            option++;
	            break;
	          case '7':
	            command.mode = 7;
	            option++;
	            break;
	          }
	          break;
	        case 'g':
	          option++;
	          switch (*option)
	          {
	          default:
              printf("Unknown option [%c] in parameter \"%s\".\n"
	                   "Ignoring parameter.\n\n",
	                   (*option ? *option : ' '),
	                   argv[0]);
	            option += strlen(option);
	            show_usage = !0;
	            returnval = 1;
	            break;
	          case 'z':
	              printf("OK");
	              command.mode = GZIP;
	              option++;
	              break;
	          }
	          break;
	        case 'l':
	          option++;
	          switch (*option)
	          {
	          default:
	            printf("Unknown option [%c] in parameter \"%s\".\n"
	                   "Ignoring parameter.\n\n",
	                   (*option ? *option : ' '),
	                   argv[0]);
	            option += strlen(option);
	            show_usage = !0;
	            returnval = 1;
	            break;
	          case 'h':
	            option++;
	          }
	          switch (*option)
	          {
	          default:
	            printf("Unknown option [%c] in parameter \"%s\".\n"
	                   "Ignoring parameter.\n\n",
	                   (*option ? *option : ' '),
	                   argv[0]);
	            option += strlen(option);
	            show_usage = !0;
	            returnval = 1;
	            break;
	          case '5':                    /* voorlopig de ingang voor lha 5 */
	            command.mode = LHA_LH5_;
	            option++;
	            break;
	          case '6':                    /* voorlopig de ingang voor lha 6 */
	            command.mode = LHA_LH6_;
	            option++;
	            break;
	          }
	          break;
	        case 'e': /* no paths */
	          option++;
	          xcommand.path=0;
	          break;
	        case 'f': /* don't force .arj extension */
	          option++;
	          xcommand.extension=0;
	          break;
	        case 's': /* set compression speed */
	          option++;
	          switch (*option)
	          {
	          default:
	            printf("Unknown option [%c] in parameter \"%s\".\n"
	                   "Ignoring parameter.\n\n",
	                   (*option ? *option : ' '),
	                   argv[0]);
	            option += strlen(option);
	            show_usage = !0;
	            returnval = 1;
	            break;
	          case '0':
	            command.speed = 0;
	            option++;
	            break;
	          case '1':
	            command.speed = 1;
	            option++;
	            break;
	          case '2':
	            command.speed = 2;
	            option++;
	            break;
	          }
	          break;
	        }
	      }
	    }
	  }
    argv++;
  }
  switch(command.mode)
  {
  case LHA_LH5_:
  case LHA_LH6_:
    command.mv_mode = 0;             /* multiple volume not supported bij lha */
    xcommand.headerlen=lha_headerlen;
    xcommand.init_crc=lha_init_crc;
    xcommand.crc.poly16=ANSI_CRC16_POLY;
    xcommand.close_archive=lha_close_archive;
    xcommand.make_header=lha_make_header;
    xcommand.make_main_header=lha_make_main_header;
    xcommand.make_crc_table=make_crc16_table;
    xcommand.rc.crc_func=crc16;
    break;
  case GZIP:
    command.mv_mode = 0;             /* multiple volume not supported bij gzip */
    xcommand.headerlen=gz_headerlen;
    xcommand.init_crc=arj_init_crc;
    xcommand.close_archive=gz_close_archive;
    xcommand.make_header=gz_make_header;
    xcommand.make_main_header=gz_make_main_header;
    xcommand.make_crc_table=make_crc32_table;
    xcommand.crc.poly32=CRC32_POLY;
    xcommand.rc.crc_func=crc32;
    break;
  default: /* assume by default arj meuk */
    xcommand.headerlen=arj_headerlen;
    xcommand.init_crc=arj_init_crc;
    xcommand.close_and_create_volume=arj_close_and_create_volume;
    xcommand.close_archive=arj_close_archive;
    xcommand.make_header=arj_make_header;
    xcommand.make_main_header=arj_make_main_header;
    xcommand.make_crc_table=make_crc32_table;
    xcommand.crc.poly32=CRC32_POLY;
    xcommand.rc.crc_func=crc32;
    break;
  }
  /*
   * all options handled! But... check multiple volume sizes: if zero, use
   * drive-size.
  */
  do
  {
    if (!argc)
    {
      printf("Error: No archive name specified.\n");
      returnval = 1;
      show_usage = !0;
      /*
       * print usage! ... so fall through but do not harm!
      */
      break;
    }
    else
    {
      /*
       * Determine multiple volume size...
      */
      if (command.mv_mode)
      {
        if (argv[0][1] == ':')         /* heeft archivefile een drivespec? */
        {
          xcommand.mv_drive = 'A' - toupper(argv[0][0]) + 1;
        }
        else
        {
          xcommand.mv_drive = Dgetdrv() + 1;
        }

        if (xcommand.mv_size)
        {
          command.mv_bytes_left = xcommand.mv_size - 4;  /* eind header reserveren */
        }
        else
        {
          /*
           * command.mv_size == 0
          */
          DISKINFO di;

          do
          {
            Dfree(&di, xcommand.mv_drive);
            /* diskfreeval = di.b_free * di.b_secsiz * di.b_clsiz; */
            command.mv_bytes_left =
              di.b_free * di.b_secsiz * di.b_clsiz;
            if (command.mv_bytes_left < 8192)
            {
              printf("\nNot enough space on disk in drive %c for ARJ file!\n", 'A' - 1 + xcommand.mv_drive);
              printf("Please insert disk for ARJ file in drive %c.\n"
                     "Then press any key to start packing.\n", 'A' - 1 + xcommand.mv_drive);
              getch();
            }
          }
          while (command.mv_bytes_left < 8192);
          command.mv_bytes_left -= 4;  /* eind header reserveren */
        }
      }
      /*
       * now start building the archive!
      */
      {
        char thearj_naam[FILE_NAME_SIZE];

        if ((xcommand.rc.crc_table = xcommand.make_crc_table(&xcommand.crc.poly32)) == NULL)
        {
          printf("Error: insufficient memory!\n");
          exit(-1);
        }
        if(init_encode(&command)!=GUP_OK)   /* initialiseer de encoder */
        {
          printf("Error: insufficient memory!\n");
          exit(-1);
        }
        xcommand.arj_naam = thearj_naam;
        strcpy(xcommand.arj_naam, argv[0]);
        if(xcommand.extension)
        {
          char *q;

          if ((q = strrchr(xcommand.arj_naam, '\\')) == 0)
          {
            q = xcommand.arj_naam;
          }
          if ((q = strchr(q, '.')) == 0)
          {
            q = strend(xcommand.arj_naam);
          }
          switch(command.mode)
          {
          case LHA_LH5_:
          case LHA_LH6_:
            strcpy(q, ".lzh");
            break;
          case GZIP:
            strcpy(q, ".gz");
            break;
          default:
            strcpy(q, ".arj");
            break;
          }
        }
        if(xcommand.add_mode==1)
        {
          printf("Adding to archive: %s\n", xcommand.arj_naam);
        }
        else
        {
          printf("Creating archive: %s\n", xcommand.arj_naam);
        }
        if (argc == 1)
        {
          argc++;
          argv--;
        }
        { /* open archive file */
          buf_fhandle_t* res;
          if ((res = buf_open_write(xcommand.arj_naam, command.buffer_start, 
                                    command.buffer_size, xcommand.add_mode))==NULL)
          {
            printf("ARJ file open error\n");
            returnval = 3;
            break;
          }
          else
          {
            command.bw_buf=res;
          }
        }
        if(xcommand.add_mode==1)
        {
          long filesize=buf_seek_write(0, 2, command.bw_buf);
          if(filesize==0)
          {
            xcommand.add_mode=0;
          }
          else
          {
            if(command.mv_mode)
            {
              printf("Error: adding to multiple volume archives is not supported!\n");
              returnval=-1;
              break;
            }
            else
            {
              if((command.mode==LHA_LH5_) || (command.mode==LHA_LH6_))
              {
                /* seek lha file end */
                buf_seek_and_fill(1, filesize-1, command.bw_buf);
                {
                  if(*command.bw_buf->current!=0)
                  {
                    printf("Error while seeking end of LZH archive!\n");
                    returnval=-1;
                    break;
                  }
                }
              }
              else
              {
                if(command.mode==GZIP)
                {
                  printf("Error: adding to gzip archives is not supported!\n");
                  returnval=-1;
                  break;
                }
                else
                { /* seek arj file end */
                  uint8 end_hdr[]={0x60,0xea,0x00,0x00};
                  filesize-=3;
                  do
                  {
                    filesize--;
                    buf_seek_and_fill(4, filesize, command.bw_buf);
                  }
                  while((filesize>0) && 
                        (memcmp(command.bw_buf->current, end_hdr, 4)!=0));
                  if(returnval==-1)
                  {
                    break;
                  }
                }
              }
            }
          }
        }
        if(xcommand.add_mode==0)
        {
          if (command.mv_mode)
          {
            unsigned long hs;
            xcommand.main_header = buf_seek_write(0, 1, command.bw_buf);
            hs = xcommand.make_main_header(xcommand.arj_naam, 0, &command);
            if (command.mv_bytes_left < hs)
            {
              printf("\nERROR: Volume size too small for main header!\n");
              buf_close_write(command.bw_buf);
              returnval = 2;
              break;
            }
            command.mv_bytes_left -= hs;
          }
          else
          {
            xcommand.make_main_header(xcommand.arj_naam, 0, &command);
          }
        }
        i = 1;
        while (i < argc)
        {
          scan_dir(argv[i++], &command);
        }
       
        if(xcommand.close_archive(&command)!=GUP_OK)
        {
          printf("\n\nDiskfull!!!\n");
          returnval = 2;
          break;
        }
      }
    }
  }
  while (0);

	free_encode(&command);
  free_crc32_table(xcommand.rc.crc_table);
  /* command.crc_table=NULL; zou hier eigenlijk moeten*/
  if (show_usage)
  {
    printf("  Usage:\n"
           "  ARJBETA [a][-v[#]] [-jm] [-m#] <filename>[.arj] <filename> {<filename>}\n"
           "  a : add files to archive (files are not updated\n"
           "  -e : exclude paths from filenames\n"
           "  -f : do not force the .arj extension on the archive name\n"
           "  -jm : enable better compression\n"
           "  -m# : compression mode 0..4, extended mode 7(best, default)\n"
           "  -lh# : create lzh archive (method LH5 or LH6)\n"
           "  -gz : create Gzip archive\n"
           "  -s : set speed: 0 (normal) - 2 (fastest)\n"
           "  -v# : # = volume size in BYTES; #<8192 -> standard disksizes.\n"
           "  Wildcards accepted, always recursive file handling.\n"
           "\n"
           "  If you wish the development of ARJ ST to continue please\n"
           "  send a postcard to the following adress:\n"
           "      Hans Wessels\n"
           "      Rohofstraat 116\n"
           "      7604 AK Almelo,\n"
           "      The Netherlands    Or send a message to MR_NI@MBH.ORG\n"
           "  *** Press any key to continue. ***");
    FLUSH();
    getch();
  }
  free(xcommand.strbuf);
  printf("\n  Time used %lu sec.", (clock() - start) / CLK_TCK);
  return returnval;
}
 
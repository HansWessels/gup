/* contains command line parsing e.g. setting all global options
 * what to do etc.
 * Changelog
 * 12-3-1995 - first version
 */

#include "gup.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "arj.h"
#include "gup_err.h"
#include "options.h"
#include "compress.h"
#include "arcman.h"
#include "utils.h"

#ifdef atarist
long _stksize = 65536;
#endif

OPTIONS opts;

static int default_mode = TRUE;			/* Use default method for archive type. */

/*
   <Commands>
   marked with '*' implemented,
   '-' thinking of omitting
   '?' precise effect/syntax??
   * a: Add files to archive              ? n: reName files in archive
   - b: execute Batch or dos command      - o: Order files in archive
   c: Comment archive files               p: Print files to standard output
   d: Delete files from archive           r: Remove paths from filenames
   e: Extract files from archive        - s: Sample files to screen with pause
   ? f: Freshen files in archive            t: Test integrity of archive
   g: Garble files in archive             u: Update files to archive
   - i: check Integrity of ARJ.EXE          v: Verbosely list contents of archive
   j: Join archives to archive          - w: Where are text strings in archive
   ? k: remove bacKup files in archive      x: eXtract files with full pathname
   l: List contents of archive            y: copY archive with new options
   * m: Move files to archive
 */

/*
   <Switches>
   -: disables switch char                p: match using full Pathnames
   +: inhibits GUP_SW usage              p1: match Pathname with subdirs
   !: sets list char (!)                  q: Query on each file
   &: install critical error handler      r: Recurse subdirectories
   #: select files by number              s: (All) set archive time-Stamp
   $: add/extract volume label           s1: save original time-Stamp
   $A: add/extract label to drive A   s2: set archive time-Stamp
   a: allow any file Attribute            t: set file Type (default 0)
   a1: any files and directories              t0: set binary file type
   b: Backup changed files                   t1: set C text file type
   b1: Backup + reset archive bits            t1g: set C text with graphics
   b2: Only reset archive bits             u: Update files (new + newer)
   c: skip time-stamp Check               v: enable multiple Volumes
   d: Delete added files                     vv: beep between volumes
   asks permission before deleting        vw: keep Whole files in volumes
   e: Exclude paths from names               va: auto-detect space available
   e1: Exclude base dir from names            vas: auto-detect and command
   f: Freshen existing files                 vvwas: beep,whole,auto,command
   g: Garble with password                   vascommand: -va + command
   gstew: garble with password stew       v50000: make 50000 byte archives
   g?: prompt for password                v360s: 362000 bytes and command
   i: show no progress Indicator             v360,v720,v1200,v1440: abbrevs
   i1: show bar graph Indicator            w: assign Work directory
   k: Keep a .BAK of ARJ archive             wtmp: use tmp as work directory
   l: create List_name file               x: eXclude selected files
   lnames.lst: create names.lst           x*.exe: exclude *.exe files
   m: with Method 0, 1, 2, 3, 4              x!names: exclude files in names
   m0: store (no compression)             multiple exclusions are allowed
   m1: good compression (default)      y: assume Yes on all queries
   m2: less memory and compression        use this switch for batch mode
   m3: FAST! less compression          z: supply archive comment file
   m4: FASTEST! least compression         zarc.cmt: use arc.cmt for comments
   n: only New files (not exist)             zNUL: use to strip comments
   o: On or after YYMMDDHHMMSS
   o:       on today
   o901225: on/after 12/25/90
   ob: Before YYMMDDHHMMSS
   ob:       before today
   ob901225: before 12/25/90

   <Shifted switches>
   ja: show ANSI comments                 jr: Recover broken archive files
   jb: Set backup archive type            js: Store archives by suffix
   jb1: Reset backup archive type              default is arj,arc,lzh,pak,zip
   jb2: Unmark backup file                     js.zoo.lzh: store .zoo,.lzh files
   jb3: Mark file as a backup file         jt: Test temporary archive by CRC
   jc: exit after Count of files         jt1: (tUpd) Test CRC and file contents
   jc5: exit after 5 files           jt2: Test only added file contents
   jc nam1 nam2: exit after 2 files   ju: translate UNIX style paths
   jd: ensure free Disk space             jv: set Verbose display
   e -jd50K: skip file if<50000 free jv1: set special verbose list mode
   l -jd1000: set error if<1000 free  jw: set extract output filename
   je: create self-extract archive            jwnew.fil: output to new.fil
   je1: create SFXJR archive               jx: start at eXtended position
   jf: store/use Full specified path          jx10000: start at position 10000
   jf1: store/use path minus drive         jy: suppress queries assuming Yes
   jg: select backup files                 a - skip append query
   jg1: select only backup files            c - skip create directory query
   jh: set Huffman buffer size             d - skip delete files query
   jh65535: set to 65535 bytes (max)   k - skip disk space available query
   jh2048: set to 2048 bytes (min)     n - skip new filename prompt
   ji: create Index file                   o - skip overwrite existing file query
   jiindex.lst: create index.lst       r - erase all type-ahead before query
   jk: Keep temp archive on error          s - skip scanned enough text query
   jl: display only filespecs              v - skip proceed to next volume query
   jm: set Maximum compression mode        y - accept single character Y/N/A/Q
   jm1: set faster Maximum compression      jycny: skip create, new name queries
   jn: restart at fileName                        in single character input mode
   jnBIN\X.COM: restart at BIN\X.COM  jz: supply file for file comment
   jo: (am) Query when updating file          jzfil.cmt: use fil.cmt for comments
   jo: (ex) Generate unique output names      jzNUL: use to strip file comments
   jp: Pause after each screenful
   jp50: pause, set page size to 50
   jq: set string parameter
   jqstring: set parameter to string
 */

void usage(char *n)
{
	printf("GUP " VERSION " " SYSTEM " " __DATE__ " " __TIME__ "\n"
		   "Copyright (c) 1995,1996,1998 H. Wessels, J. Klaren & W. Klaren\n\n"
		   "Type \'%s -?\' or something like it for more help.\n"
		   "Usage: %s <command> [-<sw> [-<sw>...]] <archive_name> [<file_names>...]\n"
		   "<Commands> - implemented commands marked with \'*\'\n"
		   " * a: Add files to archive          m: Move files to archive\n"
		   "   d: Delete files from archive   * t: Test integrity of archive\n"
		   "   e: Extract files from archive    u: Update files to archive\n"
		   "   f: Freshen files in archive      v: Verbosely list contents of archive\n"
		   " * l: List contents of archive    * x: eXtract files with full pathname\n"
		   "<Switches>\n"
		   "   c: skip time-stamp Check         n: only New files (not exist)\n"
		   "   d: with Delete (move)          * r: Recurse subdirectories\n"
		   " * e: Exclude paths from names      u: Update files (new and newer)\n"
		   "   f: Freshen existing files      * v: enable multiple Volumes\n"
		   "   g: Garble with password          w: assign Work directory\n"
		   "   i: no progress Indicator         x: eXclude selected files\n"
		   "   j: 'jm' option (WUT?)\n"
		   " * lh: with LZH Method 0,4,5,6      y: assume Yes on all queries\n"
		   " * lz: with LZS Method s,5\n"
		   "   gz: GZIP mode\n"
		   " * m: with Method 0, 1, 2, 3, 4, 7\n",
		   n, n);
	exit(0);
}

/* parses the option string o (starts with '-')
 */
void setoption(char *o)
{
	o++;
	switch (tolower(o[0]))
	{
	case 'r':
		if (o[1] == '-')
			opts.recursive = 0;
		break;
#if 0
	case 'd':
		opts.delete = 1;
		break;
#endif
	case 'v':
		opts.mv_mode = 1;
		if (isdigit(o[1]))		/* size follows */
		{
			unsigned long waarde = atoi(o + 1);

			if (waarde < 8192)
			{
				switch ((int) waarde)
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
					if (waarde != 0)
					{
						waarde = 1456640UL;
						break;
					}
					else
					{
						(void) 0;
						/*
						   // waarde = 0;
						   //
						   // Check if target-archive has disk specified. If so,
						   // try to determine disksize for multiple volume
						   // size.
						 */
					}
					break;
				}
			}
			opts.mv_size = waarde;
		}
		break;
	case 'm':
		switch (o[1])
		{
		default:
			printf("Unknown option [%c] in parameter \"%s\".\n"
				   "Ignoring parameter.\n\n", (o[1] ? o[1] : ' '), o);
			break;

		case '0':
			opts.mode = 0;
			default_mode = FALSE;
			break;
		case '1':
			opts.mode = ARJ_MODE_1;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '2':
			opts.mode = ARJ_MODE_2;
			opts.type = AT_ARJ;
			opts.speed = 1;
			default_mode = FALSE;
			break;
		case '3':
			opts.mode = ARJ_MODE_3;
			opts.type = AT_ARJ;
			opts.speed = 2;
			default_mode = FALSE;
			break;
		case '4':
			opts.mode = ARJ_MODE_4;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '5':						/* voorlopig de ingang voor lha 5 */
			opts.mode = LHA_LH5_;
			opts.type = AT_LHA;
			default_mode = FALSE;
			break;
		case '6':						/* voorlopig de ingang voor lha 6 */
			opts.mode = LHA_LH6_;
			opts.type = AT_LHA;
			default_mode = FALSE;
			break;
		case '7':
			opts.mode = GNU_ARJ_MODE_7;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		}
		break;
	case 'l':
		switch (o[1])
		{
		default:
			printf("Unknown option [%c] in parameter \"%s\".\n"
				   "Ignoring parameter.\n\n", (o[1] ? o[1] : ' '), o);
			break;
		case 'h':
			switch (o[2])
			{
			default:
				printf("Unknown option [%c] in parameter \"%s\".\n"
					   "Ignoring parameter.\n\n", (o[2] ? o[2] : ' '), o);
				break;
			case '0':					/* voorlopig de ingang voor lha 0 */
				opts.mode = LHA_LH0_;
				opts.type = AT_LHA;
				default_mode = FALSE;
				break;
			case '4':					/* voorlopig de ingang voor lha 4 */
				opts.mode = LHA_LH4_;
				opts.type = AT_LHA;
				default_mode = FALSE;
				break;
			case '5':					/* voorlopig de ingang voor lha 5 */
				opts.mode = LHA_LH5_;
				opts.type = AT_LHA;
				default_mode = FALSE;
				break;
			case '6':					/* voorlopig de ingang voor lha 6 */
				opts.mode = LHA_LH6_;
				opts.type = AT_LHA;
				default_mode = FALSE;
				break;
			}
			break;
		case 'z':
			switch (o[2])
			{
			default:
				printf("Unknown option [%c] in parameter \"%s\".\n"
					   "Ignoring parameter.\n\n", (o[2] ? o[2] : ' '), o);
				break;
			case 's':					/* voorlopig de ingang voor lha lzs */
				opts.mode = LHA_LZS_;
				opts.type = AT_LHA;
				default_mode = FALSE;
				break;
			case '5':					/* voorlopig de ingang voor lha lz5 */
				opts.mode = LHA_LZ5_;
				opts.type = AT_LHA;
				default_mode = FALSE;
				break;
			}
			break;
		}
		break;
	case 'g':
		if (o[1] == 'z')
		{
			opts.mode = GZIP;
			opts.type = AT_GZIP;
			default_mode = FALSE;
		}
		else
			fprintf(stderr, "unknown option: \'-%s\' ignored\n", o);
		break;
	case 'e':
		opts.exclude_paths = 1;
		break;
	case 'j':
		opts.jm = 1;
		break;
	default:
		fprintf(stderr, "unknown option: \'-%s\' ignored\n", o);
		break;
	}
}

/* parse the commands and options passed to main
 * return offset in argv where the files start
 * - code could be added to parse options starting with '/' did
 *   not do that considering UN*X / in pathnames etc.
 */
int parse(int argc, char *argv[])
{
	char *env;
	int i;

	/* parse the main command */

	if (strlen(argv[1]) != 1)
		usage(opts.programname);

	switch (tolower(argv[1][0]))
	{
	case 'a':
		opts.command = CMD_ADD;
		break;
	case 'x':
		opts.command = CMD_EXTRACT;
		break;
	case 'l':
		opts.command = CMD_LIST;
		break;
	case 't':
		opts.command = CMD_TEST;
		break;
	default:
		opts.command = CMD_NOTIMPLEMENTED;
		usage(opts.programname);
		break;
	}

	/* parse the options in the environment */

	if ((env = getenv("GUP_SW")) != NULL)
	{
		char *option = NULL;

		if ((env = strdup(env)) != NULL)
		{
			for (i = 0; i < (int) strlen(env); i++)
			{
				if (isspace(env[i]) && option)
				{
					env[i] = 0;
					setoption(option);
					option = NULL;
					continue;
				}

				if (isspace(env[i]))
					continue;

				if (env[i] == '-')
					option = &env[i];
				else
				{
					printf("Commandfile not implemented yet! (%s)", &env[i]);
					break;
				}
			}
			free(env);
	}
	}

	/* parse options on commandline */

	for (i = 2; i < argc; i++)
	{
		if (argv[i][0] == '-')
			setoption(argv[i]);
		else
			break;
	}

	return i;
}

void help(char *n)
{
	printf("This is the extensive help of %s\ncore dumped ;)", n);
	exit(0);
}

void doexit(void)
{
	if (opts.arj_name != NULL)
	{
		free(opts.arj_name);
	}
	if (opts.args != NULL)
	{
		free(opts.args);
	}
}

/* duplicate the files in the array - just to be on the safe side
 * not the fastest implementation, but uses the littlest memory
 */
char **args2f(char *files[])
{
	long n = 0, i = 0;
	char **ret, *p;

	while (files[i] != NULL)
		n += 1 + strlen(files[i++]);

	n += (i + 1) * sizeof(char *);

	if ((ret = (char **) malloc(n)) == NULL)
		return NULL;

	p = (char *) ret + (i + 1) * sizeof(char *);

	i = 0;

	while (files[i] != NULL)
	{
		ret[i] = p;
		strcpy(p, files[i]);
		p = strchr(p, 0) + 1;
		i++;
	}

	ret[i] = NULL;

	return ret;
}

extern int compress(OPTIONS * opt);
extern int decompress(OPTIONS * opt);
extern int list_arj(OPTIONS * opt);

int main(int argc, char *argv[])
{
	int error = 0;

	for (;;)
	{
		int files;

		/* set default options */

		memset(&opts, 0, sizeof(opts));
		opts.recursive = 1;
		opts.jm = 0;
		opts.speed = 0;
		opts.mode = GNU_ARJ_MODE_7;
		opts.stream = 0;
		opts.programname = "gup";
		opts.type = AT_UNKNOWN;

		atexit(doexit);

		if (argv[0] != NULL)
		{
			char *t;

			t = strrchr(argv[0], DELIM);

			if (t != NULL)
				opts.programname = t + 1;
		}

		/*
		 * Parse command line.
		 */

		if (argc == 1)
		{
			usage(opts.programname);
		}

		if ((argc == 2) &&
			(strcmp(argv[1], "-h") || strcmp(argv[1], "/h") ||
			 strcmp(argv[1], "-?") || strcmp(argv[1], "/?") ||
			 strcmp(argv[1], "--help")))
		{
			help(opts.programname);
		}

		files = parse(argc, argv);

		if ((opts.arj_name = (char *) malloc(strlen(argv[files]) + 4)) ==
			NULL)
		{
			error = 1;
			break;
		}

		strcpy(opts.arj_name, argv[files]);

		if ((opts.args = args2f(argv + files + 1)) == NULL)
		{
			error = 1;
			break;
		}

		/*
		 * Handle archive type specific options.
		 */

		switch (get_arc_type(opts.arj_name))
		{
		case AT_ARJ:
		case AT_UNKNOWN:
			if (default_mode)
			{
				opts.mode = GNU_ARJ_MODE_7;
			}
			break;
		case AT_LHA:
			if (default_mode)
			{
				opts.mode = LHA_LH6_;
			}
			break;
		case AT_GZIP:
			if (default_mode)
				opts.mode = GZIP;
			break;
		case AT_CDUMP:
		case AT_BINDUMP:
		case AT_ASMDUMP:
			if (default_mode)
				opts.mode = GNU_ARJ_MODE_7;     // GNU_ARJ_MODE_8? or something...
			break;
		}

		/*
		 * Execute command.
		 */

		if ((opts.command == CMD_ADD) || (opts.command == CMD_FRESHEN) ||
			(opts.command == CMD_GARBLE) || (opts.command == CMD_MOVE) ||
			(opts.command == CMD_UPDATE))
		{
			compress(&opts);
		}
		else if ((opts.command == CMD_EXTRACT) || (opts.command == CMD_PRINT)
				 || (opts.command == CMD_TEST)
				 || (opts.command == CMD_XTRACT))
		{
			decompress(&opts);
		}
		else if (opts.command == CMD_LIST)
		{
			list_arj(&opts);
		}
		else
		{
			printf("%s: Not implemented\n", opts.programname);
			return GUP_OK;
		}

		return error;
	}

	fprintf(stderr, "%s: Not enough memory\n", opts.programname);
	return error;
}

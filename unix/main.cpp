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
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <string>
#include <algorithm>
#include <cctype>

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

static void usage_at_2cols(const char *table[], size_t tablesize)
{
	size_t count = tablesize;
	size_t offset = (count + 1) / 2;
	const int W = 38;
	for (size_t i = 0; i < offset; i++)
	{
		if (i + offset < tablesize)
			printf("%*s %*s\n", -W, table[i], -W, table[i + offset]);
		else
			printf("%s\n", table[i]);
	}
}

static void usage(const char *n)
{
	printf("GUP " VERSION " " SYSTEM " " __DATE__ " " __TIME__ "\n"
		   "Copyright (c) 1995,1996,1998 H. Wessels, J. Klaren & W. Klaren\n\n"
		   "Type \'%s -?\' or something like it for more help.\n"
		   "Usage: %s <command> [-<sw> [-<sw>...]] <archive_name> [<file_names>...]\n"
		   "<Commands> - implemented commands marked with \'*\'\n"
		   , n, n
		   );
	const char *items[] = {
		   "* a: Add files to archive",
		   "  d: Delete files from archive",
		   "  e: Extract files from archive",
		   "  f: Freshen files in archive",
		   "* l: List contents of archive",
		   "  m: Move files to archive",
		   "* t: Test integrity of archive",
		   "  u: Update files to archive",
		   "  v: Verbosely list contents of archive",
		   "* x: eXtract files with full pathname",
	};
	usage_at_2cols(items, sizeof(items) / sizeof(items[0]));
	printf("<Switches>\n");
	const char *optitems[] = {
		   "  c: skip time-stamp Check",
		   "  d: with Delete (move)",
		   "* e: Exclude paths from names",
		   "  f: Freshen existing files",
		   "  g: Garble with password",
		   "  i: no progress Indicator",
		   "  jm: maximum compression (default)",
		   "* lh: with LZH Method 0,4,5,6",
		   "* lz: with LZS Method s,5",
		   "  gz: GZIP mode",
		   "* m?: with Method 0, 1, 2, 3, 4, 7",
		   "* n?: with Ni packer method 0-9",
		   "* r: Recurse subdirectories",
		   "  u: Update files (new and newer)",
		   "* v: enable multiple Volumes",
		   "  w: assign Work directory",
		   "  x: eXclude selected files",
		   "  y: assume Yes on all queries",
	};
	usage_at_2cols(optitems, sizeof(optitems) / sizeof(optitems[0]));
	exit(0);
}

/* parses the option string o (starts with '-')
 */
static void setoption(const char *o)
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
	case 'n':
		switch (o[1])
		{
		default:
			printf("Unknown option [%c] in parameter \"%s\".\n"
				   "Ignoring parameter.\n\n", (o[1] ? o[1] : ' '), o);
			break;

		case '0':
			opts.mode = NI_MODE_0;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '1':
			opts.mode = NI_MODE_1;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '2':
			opts.mode = NI_MODE_2;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '3':
			opts.mode = NI_MODE_3;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '4':
			opts.mode = NI_MODE_4;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '5':
			opts.mode = NI_MODE_5;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '6':
			opts.mode = NI_MODE_6;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '7':
			opts.mode = NI_MODE_7;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '8':
			opts.mode = NI_MODE_8;
			opts.type = AT_ARJ;
			default_mode = FALSE;
			break;
		case '9':
			opts.mode = NI_MODE_9;
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
static int parse(int argc, char *argv[])
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

static void help(const char *n)
{
	printf("This is the extensive help of %s\ncore dumped ;)\n", n);
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
static char **args2f(char *files[])
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

static const char *mk_basename(const char* path)
{
	ARJ_Assert(path != NULL);
	const char* p1 = strrchr(path, '/');
	const char* p2 = strrchr(path, '\\');
	if (p1 && p2)
		return p1 < p2 ? p2 + 1 : p1 + 1;
	if (p2)
		return p2 + 1;
	if (p1)
		return p1 + 1;
	return path;
}

#ifdef _MSC_VER

static int trigger_debugger = 1;

/*
 * Define our own reporting function.
 * We'll hook it into the debug reporting
 * process later using _CrtSetReportHook.
 */
int crm_dbg_report_function(int report_type, char* usermsg, int* retval)
{
	/*
	 * By setting retVal to zero, we are instructing _CrtDbgReport
	 * to continue with normal execution after generating the report.
	 * If we wanted _CrtDbgReport to start the debugger, we would set
	 * retVal to one.
	 */
	*retval = !!trigger_debugger;

	/*
	 * When the report type is for an ASSERT,
	 * we'll report some information, but we also
	 * want _CrtDbgReport to get called -
	 * so we'll return TRUE.
	 *
	 * When the report type is a WARNing or ERROR,
	 * we'll take care of all of the reporting. We don't
	 * want _CrtDbgReport to get called -
	 * so we'll return FALSE.
	 */
	switch (report_type)
	{
	default:
	case _CRT_WARN:
	case _CRT_ERROR:
	case _CRT_ERRCNT:
		fputs(usermsg, stderr);
		fflush(stderr);
		return FALSE;

	case _CRT_ASSERT:
		fputs(usermsg, stderr);
		fflush(stderr);
		break;
	}
	return TRUE;
}

#endif

int main(int argc, char *argv[])
{
	int error = 0;

#ifdef _MSC_VER
	// Get current flag
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	tmpFlag |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF;

	// Set flag to the new value.
	_CrtSetDbgFlag(tmpFlag); 

	/*
	 * Hook in our client-defined reporting function.
	 * Every time a _CrtDbgReport is called to generate
	 * a debug report, our function will get called first.
	 */
	_CrtSetReportHook(crm_dbg_report_function);

	/*
	 * Define the report destination(s) for each type of report
	 * we are going to generate.  In this case, we are going to
	 * generate a report for every report type: _CRT_WARN,
	 * _CRT_ERROR, and _CRT_ASSERT.
	 * The destination(s) is defined by specifying the report mode(s)
	 * and report file for each report type.
	 */
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

	// Get the current bits
	int c = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	// Set the debug-heap flag so that freed blocks are kept on the
	// linked list, to catch any inadvertent use of freed memory
#if 0
	c |= _CRTDBG_DELAY_FREE_MEM_DF;
#endif

	// Set the debug-heap flag so that memory leaks are reported when
	// the process terminates. Then, exit.
	//c |= _CRTDBG_LEAK_CHECK_DF;

	// Clear the upper 16 bits and OR in the desired freqency
	//c = (c & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_16_DF;

	c |= _CRTDBG_CHECK_ALWAYS_DF;

	// Set the new bits
	_CrtSetDbgFlag(c);

	// set a malloc marker we can use it in the leak dump at the end of the program:
	(void)_calloc_dbg(1, 1, _CLIENT_BLOCK, __FILE__, __LINE__);
#endif

	for (;;)
	{
		int files;
		bool is_special_appname = false;
		archive_type arc_type = AT_UNKNOWN;

		/* set default options, dependent on actual application name */
		{
			const char* appname = argv[0];
			if (!appname || !*appname)
				appname = "gup";
			appname = mk_basename(appname);

			// Windows et al: strip off the .EXE extension, convert to all-lowercase, etc.
			std::string apnm(appname);
			size_t dotpos = apnm.find_first_of('.');
			if (dotpos != std::string::npos)
				apnm.erase(dotpos);
			std::transform(apnm.begin(), apnm.end(), apnm.begin(), [](unsigned char c) { return std::tolower(c); });

			default_mode = TRUE;

			memset(&opts, 0, sizeof(opts));
			opts.recursive = 1;
			opts.jm = 1;
			opts.speed = 0;
			opts.mode = GNU_ARJ_MODE_7;
			opts.stream = 0;
			opts.programname = strdup(appname);
			opts.type = AT_UNKNOWN;

			// When the archiver isn't named GUP, it's intended to be a simple drag&drop-all-the-stuff-to-pack-onto-me 
			// application. That implies that we also need to construct a suitable archive file name & all argv[] are
			// paths to stuff to archive:
			if (apnm != "gup")
			{
				is_special_appname = true;

				opts.command = CMD_ADD;
				opts.mode = ARJ_MODE_4;
				opts.type = AT_ARJ;

				default_mode = FALSE;  // we do the defaults right here, so the default handling further down below for the generic case SHOULD NOT be executed.

				char* optstr = strdup(apnm.c_str());
				char* opt = strtok(optstr, " _-");
				// skip initial part of appname, but do infer the archive type from it, 
				// when it has the format XXX2A, where XXX is any arbitrary text and A
				// is one of the archive format identifiers: B=bindump, C=cdump, S=asmdump, A=arj, L=lzh, Z=gz
				// (default = ARJ)
				arc_type = (archive_type)opts.type;
				if (opt)
				{
					char* fmt = strrchr(opt, '2');
					switch (fmt ? tolower(fmt[1]) : 0)
					{
					case 'a':
						arc_type = AT_ARJ;
						break;
					case 'l':
						arc_type = AT_LHA;
						break;
					case 'z':
						arc_type = AT_GZIP;
						break;
					case 'b':
						arc_type = AT_BINDUMP;
						break;
					case 's':
						arc_type = AT_ASMDUMP;
						break;
					case 'c':
						arc_type = AT_CDUMP;
						break;
					default:
						break;
					}
					opts.type = arc_type;

					*opt = 0;
				}

				// rest of appname represents gup options:
				while (opt)
				{
					if (*opt)
						setoption(opt - 1 /* nasty hack to re-use setoption() under these circumstances */);
					opt = strtok(NULL, " _-");
				}
				free(optstr);
			}

			atexit(doexit);

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

			if (!is_special_appname)
			{
				files = parse(argc, argv);

				if ((opts.arj_name = (char*)malloc(strlen(argv[files]) + 4)) ==
					NULL)
				{
					error = GUP_NOMEM;
					break;
				}

				strcpy(opts.arj_name, argv[files]);
			}
			else
			{
				// use the first path as a template for the archive name in drag&drop mode:
				files = 1;
				if ((opts.arj_name = (char*)malloc(strlen(argv[files]) * 2 + 12)) ==
					NULL)
				{
					error = GUP_NOMEM;
					break;
				}

				strcpy(opts.arj_name, argv[files]);

				char* p = opts.arj_name;
				size_t len = strlen(p);

				// trim off trailing '/':
				if (strchr("\\/", p[len - 1]))
					p[len - 1] = 0;

				char* name = strrchr(p, '/');
				if (name)
					name++;
				else
					name = p;
				char* p2 = strrchr(p, '\\');
				if (p2)
					p2++;
				else
					p2 = p;
				if (p2 > name)
					name = p2;

				// is it a directory?
				osstat st;
				if ((error = gup_stat(p, &st)) != GUP_OK)
					break;

				char *dst = p + len;
				size_t dlen = len + 12;  // remainder of allocated space
				if (gup_file_type(&st) == DIR_TYPE)
				{
					snprintf(dst, dlen, "/%s", name);
					dst++;
					dlen--;
				}
				else
				{
					dst = name;
				}

				// now append the proper archive type suffix to the archive name:
				// (first chop of any existing file extension)
				p2 = strrchr(dst, '.');
				if (p2)
					*p2 = 0;
				len = strlen(name);
				dst += len;
				dlen -= len;

				const char* ext;
				switch (arc_type)
				{
				default:
				case AT_ARJ:
					ext = "arj";
					break;
				case AT_LHA:
					ext = "lzh";
					break;
				case AT_GZIP:
					ext = "gz";
					break;
				case AT_BINDUMP:
					ext = "bindump";
					break;
				case AT_ASMDUMP:
					ext = "asmdump";
					break;
				case AT_CDUMP:
					ext = "cdump";
					break;
				}
				snprintf(dst, dlen, ".%s", ext);

				// archive name has been constructed. Before we go, however, we need
				// to check this one against the incoming filename, in case the user
				// drag&dropped an *archive* on us: we DO NOT want to overwrite that
				// one!
#if defined(WIN32) || defined(WIN64)
				if (0 == stricmp(opts.arj_name, argv[files]))
#else
				if (0 == strcasecmp(opts.arj_name, argv[files]))
#endif
				{
					printf("%s: dropped an archive file (%s) on GUP in DRAG&DROP mode: not going to replace or do anything else with it for your protection.\n", opts.programname, argv[files]);
					return GUP_INVAL;
				}
				files = 0;
			}
		}

		if ((opts.args = args2f(argv + files + 1)) == NULL)
		{
			error = GUP_NOMEM;
			break;
		}

		/*
		 * Handle archive type specific options.
		 */

		arc_type = get_arc_type(opts.arj_name);
		switch (arc_type)
		{
		case AT_ARJ:
		case AT_UNKNOWN:
			if (default_mode)
			{
				opts.mode = GNU_ARJ_MODE_7;
			}
			if (opts.type == AT_UNKNOWN)
			{
				printf("%s: assuming default output mode: ARJ\n", opts.programname);
				opts.type = AT_ARJ;
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

			if (opts.type != AT_ARJ && opts.type != AT_UNKNOWN && opts.type != AT_BINDUMP && opts.type != AT_CDUMP && opts.type != AT_ASMDUMP)
			{
				printf("%s: DUMP MODE is only supported for ARJ style compression settings.\n", opts.programname);
				return GUP_INVAL;
			}
 			opts.type = arc_type;
			break;
		}
		ARJ_Assert(opts.type != AT_UNKNOWN);
		ARJ_Assert(error == GUP_OK);

		/*
		 * Execute command.
		 */

		if ((opts.command == CMD_ADD) || (opts.command == CMD_FRESHEN) ||
			(opts.command == CMD_GARBLE) || (opts.command == CMD_MOVE) ||
			(opts.command == CMD_UPDATE))
		{
			error = compress(&opts);
		}
		else if ((opts.command == CMD_EXTRACT) || (opts.command == CMD_PRINT)
				 || (opts.command == CMD_TEST)
				 || (opts.command == CMD_XTRACT))
		{
			error = decompress(&opts);
		}
		else if (opts.command == CMD_LIST)
		{
			error = list_arj(&opts);
		}
		else
		{
			printf("%s: Not implemented\n", opts.programname);
			return GUP_INVAL;
		}

		return error;
	}

	fprintf(stderr, "%s: ", opts.programname);
	display_error((gup_result)error);
	return error;
}

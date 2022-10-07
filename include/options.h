/*
 * Include files.
 *
 *     Options structure.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:38:35 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 206 $
 * $Log$
 * Revision 1.4  2000/09/03 12:38:35  wout
 * Added new option 'type', that indicates the archive type.
 *
 * Revision 1.3  2000/07/30 15:49:08  wout
 * Added prefix CMD_ before all commands.
 *
 * Revision 1.2  1998/12/27 20:48:08  klarenw
 * Removed settings.h. New names for error codes.
 *
 * Revision 1.1  1997/12/24 22:49:50  klarenw
 * Created structure for commandline options and library for missing
 * functions.
 */

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

typedef enum
{
	CMD_NOTIMPLEMENTED,
	CMD_ADD, CMD_COMMENT, CMD_DELETE, CMD_EXTRACT, CMD_FRESHEN,
	CMD_GARBLE, CMD_JOIN, CMD_LIST, CMD_MOVE, CMD_RENAME, CMD_PRINT,
	CMD_REMOVE, CMD_TEST, CMD_UPDATE, CMD_LISTVERBOSE, CMD_XTRACT
} COMMAND;

typedef struct options
{
	const char *programname;		/* Program name. */
	char *arj_name;					/* Name of the archive. */
	char **args;						/* Further args to functions (eg files to pack). */

	COMMAND command;

	int use_stdin;
	int use_stdout;						/* Are we using stdin/stdout? */
	int stream;

	int type;							/* Archive type (AT_ARJ, AT_LHA, etc.) */

	int mode;							/* Pack mode. */
	int jm;								/* -jm mode, 0 is off, 1 is on. */
	int speed;							/* Compression speed. */
	int mv_mode;					  	/* 0 = Multiple volume off. */
	unsigned long mv_size;				/* Size for multiple volumes, 0 = auto. */

	int recursive;						/* Recursive. */
	int del;							/* Delete stuff after pack. */
	int exclude_paths;					/* Exclude paths from archive/extract. */

	/*
	 * Options supported by the archive class.
	 */

	int no_crc_checking;				/* Do not check the CRC of depacked files. */
	int no_write_data;					/* Do not write depacked data (test command). */
	int repair;							/* Not implemented in commandline interface. */

	/*
	 * Not implemented.
	 */

	int rep_severe;
	int ask_next_vol;
	int wait_for_arj;
	int only_test;
	int b_mark_ok;
	int rq_passw;
	int orig_password;
	int b_m_only;
	int any_attrib;
	int x_any_attr;
	int dirtype_too;
	int do_labels;
	int label_drive;
	int verbose_level;
	int ask_createdir;
	int no_strictchk;
	int ask_append;
	int only_new;
	int extractunique;
	int freshen;
	int ask_each_file;
	int ask_overwrite;
	int update;
	int show_comments;
	int show_banner;
	int ign_r_tbit;
	int showonlyfspec;
} OPTIONS;

#endif

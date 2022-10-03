
#ifndef __ARJ_H
#define __ARJ_H

/*
 * Other defines.
 */

#define HOST_OS_NAMES	{ "MS-DOS", "PRIMOS", "UNIX", "AMIGA",				\
						  "MAC-OS", "OS/2", "APPLE GS",						\
						  "ATARI ST", "NEXT", "VAX VMS", "Win32",			\
						  "Generic OS",		\
						  "OS9", "OS68K", "OS386", "HUMAN", "CP/M",			\
						  "Flex OS", "RUNSER", "Town OS", "XOSK" }

/*
 * Default path seperator for this system.
 */

#if (OS == OS_WIN32)
#define DELIM		'\\'
#define DELIM_STR	"\\"
#else
#define DELIM		'/'
#define DELIM_STR	"/"
#endif

#endif	/* __ARJ_H */

#ifndef UTILS_H
#define UTILS_H

/*
 * Misc utils.
 */

#define FLUSH() 	fflush(stdout)

extern char *host_os_name(int os);

/*
// Pretty Name Print
// Prints the filename in a TOS-crew approved manner.
*/

extern char* pnprint(int width, const char* name, char* buf);
extern void init_progress(unsigned long size);
extern void print_progress(unsigned long delta, void *propagator);
extern void init_progress_size(unsigned long size);
extern void print_progress_size(unsigned long delta, void *propagator);
extern void display_error(gup_result err);
extern void init_message(gup_result message, void* propagator);

/*
 * File utility functions.
 */

extern gup_result builddir(const char *name);

#endif

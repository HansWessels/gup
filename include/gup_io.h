/*
 * Support library.
 *
 *     Buffered IO.
 *
 * $Author: wout $
 * $Date: 2000-07-30 17:48:29 +0200 (Sun, 30 Jul 2000) $
 * $Revision: 177 $
 * $Log$
 * Revision 1.3  2000/07/30 15:48:29  wout
 * Added function gup_io_set_position.
 *
 * Revision 1.2  1998/12/27 20:48:08  klarenw
 * Removed settings.h. New names for error codes.
 *
 * Revision 1.1  1998/03/17 18:32:46  klarenw
 * First version of filebuffering.
 */

#ifndef __GUP_IO_H__
#define __GUP_IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "compr_io.h"

/*
 * High level functions.
 */

buf_fhandle_t *gup_io_open(const char *name, unsigned char *buf_start,
						   unsigned long buf_size, int omode,
						   gup_result *result);
gup_result gup_io_close(buf_fhandle_t *file);

gup_result gup_io_write(buf_fhandle_t *file, const void *buffer, unsigned long count,
						unsigned long *real_count);
gup_result gup_io_read(buf_fhandle_t *file, void *buffer, unsigned long count,
					   unsigned long *real_count);

gup_result gup_io_seek(buf_fhandle_t *file, long offset, int seekmode,
					   long *new_pos);
gup_result gup_io_tell(buf_fhandle_t *file, long *fpos);

/*
 * Low level functions.
 */

gup_result gup_io_write_announce(buf_fhandle_t *file, unsigned long count);
gup_result gup_io_fill(buf_fhandle_t *file);
uint8 *gup_io_get_current(buf_fhandle_t *file, unsigned long *bytes_left);
void gup_io_set_current(buf_fhandle_t *file, uint8 *new_pos);
void gup_io_set_position(buf_fhandle_t *file, long position);

#ifdef __cplusplus
}
#endif
#endif

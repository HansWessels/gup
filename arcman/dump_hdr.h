/*
 * Archive manager.
 *
 *     *DUMP main and file header classes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 15:03:31 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 215 $
 * $Log$
 * Revision 1.5  2000/09/03 13:01:59  wout
 * Better multiple volume support.
 *
 * Revision 1.4  2000/07/30 14:58:35  wout
 * Added update_file_attributes and update_ext_headers.
 *
 * Revision 1.3  1998/12/28 14:58:09  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 *
 * Revision 1.2  1998/01/03 19:24:43  klarenw
 * Added multiple volume support.
 *
 * Revision 1.1  1997/12/24 22:54:49  klarenw
 * First working version. Only ARJ support, no multiple volume.
 */


#include "gup.h"

#if ENABLE_DUMP_OUTPUT_MODES

#include "arj_hdr.h"
#include "arj_arc.h"

/*
 * Main header class.
 */

class dump_mainheader : public arj_mainheader
{
  public:
	dump_mainheader(const char *comment);	/* Constructor. */
	dump_mainheader(const dump_mainheader&);	/* Copy constructor. */
	~dump_mainheader(void);				/* Destructor. */
};

/*
 * File header class.
 */

class dump_fileheader : public arj_fileheader
{
  public:
	dump_fileheader(const char *filename, const char *comment);
	dump_fileheader(const char *filename, const char *comment, const osstat *stat);
	~dump_fileheader(void);
};

#endif // ENABLE_DUMP_OUTPUT_MODES


/*
 * Archive manager.
 *
 *     *DUMP main and file header classes.
 */


#include "gup.h"

#if ENABLE_DUMP_OUTPUT_MODES

#include <string>

#include "arj_hdr.h"
#include "arj_arc.h"

/*
 * Main header class.
 */

class dump_mainheader : public arj_mainheader
{
  public:
	dump_mainheader(const char *archive_filepath, const char *comment);	/* Constructor. */
	dump_mainheader(const dump_mainheader&);	                           /* Copy constructor. */
	~dump_mainheader();				                                       /* Destructor. */

protected:
	size_t arc_output_size;
	std::string archive_path;

	long current_file_pack_start_offset;

friend class dump_archive;	
friend class bindump_archive;	
friend class asmdump_archive;	
friend class cdump_archive;	
};

/*
 * File header class.
 */

class dump_fileheader : public arj_fileheader
{
  public:
	dump_fileheader(const char *filename, const char *comment);
	dump_fileheader(const char *filename, const char *comment, const osstat *stat);
	~dump_fileheader();
};

#endif // ENABLE_DUMP_OUTPUT_MODES


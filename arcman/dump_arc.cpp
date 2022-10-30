/*
 * Archive manager.
 *
 *     *DUMP archive class.
 *
 */

#include "gup.h"

#if ENABLE_DUMP_OUTPUT_MODES

#include "arc_util.h"
#include "gup_err.h"
#include "compress.h"
#include "gup_io.h"
#include "options.h"
#include "sysdep/arcman.h"
#include "header.h"
#include "archive.h"
#include "arcctl.h"
#include "dump_hdr.h"
#include "dump_arc.h"
#include "crc.h"
#include "os.h"
#include "support.h"

#include <fstream>
#include <string>
#include <iostream>



static uint32_t hash_string(const char *s)
{
    uint32_t hash = 0;

    for(; *s; ++s)
    {
        hash += *s;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

static uint32_t folded_hash_string(const char* s)
{
    uint32_t h = hash_string(s);

    h ^= h >> 16;
    return h & 0xFFFF;
}

static std::string basename(const char *path)
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

// generate a decent variable name for the unambiguous file path, i.e. we use the entire (relative?) path:
static const char *mk_variable_name(char *dst, size_t dstsize, const char *fpath)
{
	// calc a simple hash of the input string, sans Windows/DOS drive:
	const char *drv = strchr(fpath, ':');
	if (drv)
		fpath = drv + 1;
	
	uint32_t h = folded_hash_string(fpath);
	
	// convert string to variable-name-safe:
	
	// filter and restrict length to N (N = 40 by default) and a maximum directory tree depth of 4
#define LENGTH_LIMIT  40
#define DIRTREE_DEPTH_LIMIT  4

	assert(dstsize > 20);
	snprintf(dst, dstsize, "packed_%04X_", (unsigned int)h);

	char *e = dst + dstsize - 1;
	*e = 0;
	char *d = e;
	bool has_seen_underscore = true;
    int dirtree_depth_level = 0;
	size_t len = strlen(fpath);
	
    for (size_t i = len - 1, stop = (len >= LENGTH_LIMIT ? len - LENGTH_LIMIT : 0); i >= 0 && i > stop && d > dst + 7 + 5; i--)
	{
		char c = fpath[i];
		
		if (c >= 'A' && c <= 'Z')
			has_seen_underscore = false;
		else if (c >= 'a' && c <= 'z')
			has_seen_underscore = false;
		else if (c >= '0' && c <= '9')
			has_seen_underscore = false;
		else 
		{
			if (c == '\\' || c == '/')
			{
				c = '_';
				e = d + 1;
				dirtree_depth_level++;
				if (dirtree_depth_level >= DIRTREE_DEPTH_LIMIT)
					break;
			}
			else
			{
				c = '_';
			}
			
			if (has_seen_underscore)
				continue;
			has_seen_underscore = true;
		}
		
		*--d = c;
	}
	
	if (d != dst + 7 + 5)
		memmove(dst + 7 + 5, d, strlen(d) + 1);
	assert(strlen(dst) < dstsize);
	
	return dst;
}
	

// generate a decent filename part for the unambiguous input file path, i.e. we use the entire (relative?) path:
static const char *mk_filename_part(char *dst, size_t dstsize, const char *fpath)
{
	// calc a simple hash of the input string, sans Windows/DOS drive:
	const char *drv = strchr(fpath, ':');
	if (drv)
		fpath = drv + 1;
	
	uint32_t h = folded_hash_string(fpath);
	
	// convert string to file-name-safe:
	
	assert(dstsize > 20);
	snprintf(dst, dstsize, "%04X.", (unsigned int)h);
	char *d0 = dst + strlen(dst);

	char *e = dst + dstsize - 1;
	*e = 0;
	char *d = e;
	bool has_seen_underscore = true;
    int dirtree_depth_level = 0;
	size_t len = strlen(fpath);
	
	for (int i = len - 1, stop = len - LENGTH_LIMIT; i >= 0 && i > stop && d > d0; i--)
	{
		char c = fpath[i];
		
		if (c >= 'A' && c <= 'Z')
			has_seen_underscore = false;
		else if (c >= 'a' && c <= 'z')
			has_seen_underscore = false;
		else if (c >= '0' && c <= '9')
			has_seen_underscore = false;
		else 
		{
			if (c == '\\' || c == '/')
			{
				c = '.';
				e = d + 1;
				dirtree_depth_level++;

				if (dirtree_depth_level >= DIRTREE_DEPTH_LIMIT)
					break;
			}
			else if (c != '.')
			{
				c = '_';
			}
			
			if (has_seen_underscore)
				continue;
			has_seen_underscore = true;
		}
		
		*--d = c;
	}

	if (d != d0)
		memmove(d0, d, strlen(d) + 1);
	assert(strlen(dst) < dstsize);
	
	return dst;
}
	

/*****************************************************************************
 *                                                                           *
 * Constructor and destructor.                                               *
 *                                                                           *
 *****************************************************************************/

dump_archive::dump_archive() : cur_main_hdr(NULL), increment_file_no(false), archive_file_offset(0), file_no(0), at_end_of_archive_action(false)
{
    TRACE_ME();
}

dump_archive::~dump_archive()
{
    TRACE_ME();

    delete cur_main_hdr;
}

/*****************************************************************************
 *                                                                           *
 * Functions for writing an archive.                                         *
 *                                                                           *
 *****************************************************************************/

/*
 * gup_result dump_archive::write_end_of_volume(int mv)
 *
 * Write end of archive.
 *
 * Parameters:
 *
 * mv       - 1 if the volume is part of a multiple volume archive and
 *              the volume is not the last volume of the archive.
 */

gup_result dump_archive::write_end_of_volume(int mv)
{
    TRACE_ME();

    gup_result result;
    long arc_len;
    unsigned long bytes_left;

    dump_output_bufptr_t buf = generate_end(!mv);

    if ((result = gup_io_write_announce(file, buf->length())) == GUP_OK)
    {
        uint8 *p = gup_io_get_current(file, &bytes_left);

        memcpy(p, buf->get_start_ref(), buf->length());
        p += buf->length();
        gup_io_set_current(file, p);
    }

    if ((result = gup_io_flush_header(file)) != GUP_OK)
        return result;

    /*
     * Determine the length of the archive.
     */

    if ((result = tell(arc_len)) != GUP_OK)
        return result;

    /*
     * Update the main header.
     */

    if (cur_main_hdr == NULL)
        return GUP_INTERNAL;

    if ((result = seek(0, SEEK_SET)) != GUP_OK)
        return result;

    /*
     * Set the length of the archive.
     */

    cur_main_hdr->arc_output_size = arc_len;

    {
        uint16 old_mv_mode;

        /*
         * Trick to prevent write_main_header() from changing 'mv_bytes_left'
         * This should be changed !!!
         */

        old_mv_mode = st.pack_str.mv_mode;
        st.pack_str.mv_mode = 0;

        result = write_main_header(cur_main_hdr);

        st.pack_str.mv_mode = old_mv_mode;
    }

    return result;
//  return arj_archive::write_end_of_volume(mv);
}

/*
 * gup_result dump_archive::write_main_header(const mainheader *header)
 *
 * Write a main header to the archive.
 */
gup_result dump_archive::write_main_header(const mainheader *header)
{
    TRACE_ME();

    gup_result result;
    const char *src;
    uint16 fspecpos = 0;
    unsigned long total_hdr_size, bytes_left;

    if (!opened || !rw)
        return GUP_INTERNAL;

    // reset the file index counter
    file_no = 1;

    src = cur_volume->name();
    const char *comment = header->get_comment();
    if (!comment)
        comment = "";

    if (!cur_main_hdr)
        cur_main_hdr = new dump_mainheader(src, comment);

        //header->host_os;      /* Host os. */
        //arj_conv_from_os_time(header->ctime); /* Creation time. */
    uint32_t t = arj_conv_from_os_time(gup_time()); /* Modification time (now). */
        //header->arc_size; /* Archive size. */
        fspecpos;       /* File spec position in file name. */

    dump_output_bufptr_t buf = generate_main_header(src, comment, t, cur_main_hdr->arc_output_size);

    if ((result = gup_io_write_announce(file, buf->length())) == GUP_OK)
    {
        uint8 *p = gup_io_get_current(file, &bytes_left);

        memcpy(p, buf->get_start_ref(), buf->length());
        p += buf->length();
        gup_io_set_current(file, p);
    }

    return result;
//  return arj_archive::write_main_header(header);
}

gup_result dump_archive::write_file_header(const fileheader *header)
{
    TRACE_ME();

    gup_result result;
    char *name_ptr;
    unsigned long total_hdr_size, bytes_left;
    const char *src;
    uint16 fspec_pos;

    if (!opened || !rw)
        return GUP_INTERNAL;

    if (cur_main_hdr == NULL)
        return GUP_INTERNAL;

    if ((result = tell(header_pos)) != GUP_OK)  /* Store current position for
                                           use by 'write_file_tailer'. */
        return result;

    // do not generate meta files ("headers") for DIRECTORIES, only for actual FILES:
    switch (header->file_type)
    {
    case BINARY_TYPE:
    case TEXT_TYPE:
    {
        dump_output_bufptr_t buf = generate_file_header(header);
        TRACE_ME();
        if ((result = gup_io_write_announce(file, buf->length())) == GUP_OK)
        {
            uint8* p = gup_io_get_current(file, &bytes_left);

            memcpy(p, buf->get_start_ref(), buf->length());
            p += buf->length();
            gup_io_set_current(file, p);
        }
    }
        break;

    default:
        break;
    }

    // now make sure we've written our stuff to disk, so we can be assured
    // that any subsequent write/flush is actual packed data!
    if ((result = gup_io_flush_header(file)) != GUP_OK)
        return result;

    if ((result = tell(cur_main_hdr->current_file_pack_start_offset)) != GUP_OK)
        return result;

    archive_file_offset = cur_main_hdr->current_file_pack_start_offset;

    if (increment_file_no)
    {
        file_no++;
        increment_file_no = false;
    }

    return result;
//  return arj_archive::write_file_header(header);
}

gup_result dump_archive::write_file_trailer(const fileheader* header)
{
    TRACE_ME();
    long current_pos;
    gup_result result;
    unsigned long bytes_left;
    unsigned long binsize_read;

    if (cur_main_hdr == NULL)
        return GUP_INTERNAL;

    // now make sure we've written our stuff to disk, so we can be assured
    // that any subsequent write/flush is actual header rewriting!
    // All flushed data to disk right now is packed data!
    if ((result = gup_io_flush_packed_data(file)) != GUP_OK)
        return result;

    // do not generate meta files ("headers") for DIRECTORIES, only for actual FILES:
    switch (header->file_type)
    {
    case BINARY_TYPE:
    case TEXT_TYPE:
    {
        if ((result = tell(current_pos)) != GUP_OK)
            return result;

        unsigned long binsize = current_pos - cur_main_hdr->current_file_pack_start_offset;

        // go to start of binary packed chunk in file:
        if ((result = seek(cur_main_hdr->current_file_pack_start_offset, SEEK_SET)) != GUP_OK)
            return result;

        uint8_t* binbuf = new uint8_t[binsize];

        if ((result = ::gup_io_reload(file, binbuf, binsize, &binsize_read)) != GUP_OK)
            return result;

        if (binsize_read != binsize)
        {
            fprintf(stderr, "couldn't read/reload all packed bytes: %lu != %lu\n", binsize_read, binsize);
            return GUP_INTERNAL;
        }

        // Special service in the DUMP modes: we perform a CRC check on the compressed output to doublecheck our packer
        // didn't produce some unexpected, 'insane' output. DUMP MODE is aimed at DEMOSCENE and other specialized
        // use cases, where CRC checking the loaded data (our packed output) is not the norm.
        //
        // In order to prevent disputes and confusion about any data corruption in such scenarios, we'ld better make
        // sure ours is squeaky clean, so we can start pointing fingers *outside* our own GUP application.  ;-) ;-)
        //
        {
            long start;
            WRITE_CRC_STRUCT wcs;

            wcs.crc = init_crc();
            wcs.handle = -1;
            wcs.arc = this;

            st.unpack_str.wc_propagator = &wcs;
            st.unpack_str.mode = header->method;
            st.unpack_str.origsize = header->origsize;

            st.unpack_str.write_crc = buf_write_crc_test;

            st.unpack_str.br_buf = file;

            unsigned int old_no_wr = opt_no_write;
            opt_no_write = 1;

            if ((result = seek(cur_main_hdr->current_file_pack_start_offset, SEEK_SET)) != GUP_OK)
                return result;

            if ((result = ::decode(&st.unpack_str)) != GUP_OK)
                return result;

            opt_no_write = old_no_wr;

            if ((result = seek(current_pos, SEEK_SET)) != GUP_OK)
                return result;

            /*
            * Set the CRC calculated while decompressing.
            */
            uint32 real_crc = post_process_crc(wcs.crc);
            if (real_crc != header->file_crc)
            {
                fprintf(stderr, "Verified depacked content CRC does not match original file CRC: this is a CATASTROPHIC INTERNAL FAILURE of the packer! CRC: 0x%08lx != 0x%08lx\n", (unsigned long)real_crc, (unsigned long)header->file_crc);
                return GUP_INTERNAL;
            }
        }

        // now we have the binary packed data in file/buffer.
        // What must be done next is fetch that binary data, copy it into
        // a temporary buffer and then rewind the output file to the start
        // of the packed data zone and rewrite the output, now using the desired
        // output encoding/format:

        dump_output_bufptr_t buf = generate_file_content(binbuf, binsize_read, header);
        TRACE_ME();

        // signal that the file index should be updated:
        increment_file_no = true;

        unsigned long real_len;
        write(buf->get_start_ref(), buf->length(), real_len);

        if ((result = tell(current_pos)) != GUP_OK)
            return result;

        if ((result = seek(header_pos, SEEK_SET)) != GUP_OK)
            return result;

        {
            uint16 old_mv_mode;

            /*
             * Trick to prevent write_file_header() from changing 'mv_bytes_left'
             * This should be changed !!!
             */

            old_mv_mode = st.pack_str.mv_mode;
            st.pack_str.mv_mode = 0;

            result = write_file_header(header);

            st.pack_str.mv_mode = old_mv_mode;

            if (result != GUP_OK)
                return result;
        }

        result = seek(current_pos, SEEK_SET);
    }
       break;

    default:
        break;
    }
    return result;
    //  return arj_archive::write_file_trailer(header);
}



gup_result dump_archive::gup_io_flush_header(buf_fhandle_t *file)
{
    TRACE_ME();
    return ::gup_io_flush(file);
}

gup_result dump_archive::gup_io_flush_packed_data(buf_fhandle_t *file)
{
    TRACE_ME();
    return ::gup_io_flush(file);
}

/*****************************************************************************
 *                                                                           *
 * GUP I/O virtualization functions.                                                         *
 *                                                                           *
 *****************************************************************************/

// We're virtualizing the core gup I/O functions here, the idea being:
// when we want to output arbitrary encodings of the packed data (the *DUMP modes)
// we can achieve such most easily when we keep everything else intact and
// "just find a way to hook into the raw I/O and reroute and encode it the way
// we want right now": that's what the {bin,asm,c}dump_archive classes are
// going to accomplish for us, but they need a bit of a leg up before they *can*:
// that's us, the *base class*, hooking into raw archive file I/O by providing
// these conveniently same-name-as-the-C-functions-that-do-the-actual-work
// virtual(=overridable) methods!

buf_fhandle_t *dump_archive::gup_io_open(const char *name, unsigned char *buf_start, unsigned long buf_size, int omode, gup_result *result)
{
    TRACE_ME();
    return ::gup_io_open(name, buf_start, buf_size, omode, result);
}

gup_result dump_archive::gup_io_close(buf_fhandle_t *file)
{
    TRACE_ME();
    return ::gup_io_close(file);
}

// Seek to a position in the file.
gup_result dump_archive::gup_io_seek(buf_fhandle_t *file, long offset, int seekmode, long *new_pos)
{
    TRACE_ME();
    return ::gup_io_seek(file, offset, seekmode, new_pos);
}

// Return the current position in the file.
gup_result dump_archive::gup_io_tell(buf_fhandle_t *file, long *fpos)
{
    TRACE_ME();
    return ::gup_io_tell(file, fpos);
}

// Write data to a file.
gup_result dump_archive::gup_io_write(buf_fhandle_t *file, const void *buffer, unsigned long count, unsigned long *real_count)
{
    TRACE_ME();
    return ::gup_io_write(file, buffer, count, real_count);
}

// Read data from a file.
gup_result dump_archive::gup_io_read(buf_fhandle_t *file, void *buffer, unsigned long count, unsigned long *real_count)
{
    TRACE_ME();
    return ::gup_io_read(file, buffer, count, real_count);
}

// Make sure there at least 'count' bytes free in the file buffer. If necessary the file buffer is flushed.
gup_result dump_archive::gup_io_write_announce(buf_fhandle_t *file, unsigned long count)
{
    TRACE_ME();
    return ::gup_io_write_announce(file, count);
}

// Fill the file buffer.
gup_result dump_archive::gup_io_fill(buf_fhandle_t *file)
{
    TRACE_ME();
    return ::gup_io_fill(file);
}

// Get a pointer to the current position in the file buffer and return the number of bytes that can be written into the buffer or can be read from the buffer.
uint8 *dump_archive::gup_io_get_current(buf_fhandle_t *file, unsigned long *bytes_left)
{
    TRACE_ME();
    return ::gup_io_get_current(file, bytes_left);
}

// Set the pointer in the file buffer to a new position.
void dump_archive::gup_io_set_current(buf_fhandle_t *file, uint8 *new_pos)
{
    TRACE_ME();
    return ::gup_io_set_current(file, new_pos);
}

// Set the current position in the file to the given value. Any bytes after this position that are in the buffer are discarded.
void dump_archive::gup_io_set_position(buf_fhandle_t *file, long position)
{
    TRACE_ME();
    return ::gup_io_set_position(file, position);
}

//===================================================================================
//===================================================================================
//===================================================================================

/*****************************************************************************
 *                                                                           *
 * Constructor and destructor.                                               *
 *                                                                           *
 *****************************************************************************/

bindump_archive::bindump_archive()
{
    TRACE_ME();
}

bindump_archive::~bindump_archive()
{
    TRACE_ME();
}

dump_output_bufptr_t bindump_archive::generate_main_header(const char *archive_path, const char *comment, uint32_t timestamp, size_t arc_output_size)
{
    TRACE_ME();

    if (!comment)
        comment = "";

    // ALT:: write the archive metadata to *another* output file, whose name is derived off `archive_path`?
    std::string metafile_path(archive_path);

    metafile_path += ".meta.nfo";

    dump_output_bufptr_t buf(new dump_output_buffer(1024 + strlen(archive_path) * 2 + strlen(comment)));

    char *filename;
    const char *src;
    uint16 fspecpos = 0;
    unsigned long total_hdr_size, bytes_left;

    filename = arj_conv_from_os_name(archive_path, fspecpos, PATHSYM_FLAG);

    //uint32_t t = arj_conv_from_os_time(gup_time());   /* Modification time (now). */

        // DO NOT rewrite the general main header metadata block: we haven't written anything in there that needs rewriting anyway.
    if (!at_end_of_archive_action)
    {
        char* dst = reinterpret_cast<char*>(buf->get_append_ref());
        snprintf(dst, buf->get_remaining_usable_size(), "\
DUMP name: %s\n\
DUMP filename: %s\n\
comment: %s\n\
creation time: %12lu\n\
\n\
--------------------------------------------------------------------\n\
\n", archive_path, filename, comment, (unsigned long)timestamp);

        size_t hdr_len = strlen(dst);
        buf->set_appended_length(hdr_len);

        {
            std::ofstream out(metafile_path);
            dst = reinterpret_cast<char*>(buf->get_start_ref());
            std::string msg(dst, buf->length());
            out << msg;
            //out.close();
        }
    }
    else
    {
        // write tail / end of the metadata file:
        char* dst = reinterpret_cast<char*>(buf->get_append_ref());
        snprintf(dst, buf->get_remaining_usable_size(), "\
archive size: %12zu\n\
\n", arc_output_size);

        size_t hdr_len = strlen(dst);
        buf->set_appended_length(hdr_len);

        {
            std::ofstream out(metafile_path, std::ios_base::app);
            dst = reinterpret_cast<char*>(buf->get_start_ref());
            std::string msg(dst, buf->length());
            out << msg;
            //out.close();
        }
    }
    
    delete[] filename;

    buf->clear();
    
    return buf;
}

dump_output_bufptr_t bindump_archive::generate_file_header(const fileheader* header)
{
    TRACE_ME();

    char* name_ptr;
    unsigned long total_hdr_size, bytes_left;
    const char* src;
    uint16 fspec_pos;

    src = header->get_filename();
    const char* comment = header->get_comment();
    if (!comment)
        comment = "";

    const osstat* file_stat = header->get_file_stat();

    name_ptr = arj_conv_from_os_name(src, fspec_pos, PATHSYM_FLAG);

    header->method; /* Packing mode. */
    header->file_type;  /* File type. */

    //      header->orig_time_stamp;    /* Time stamp. */
    header->compsize;   /* Compressed size. */
    header->origsize;   /* Original size. */
    header->file_crc;   /* File CRC. */
    fspec_pos;          /* File spec position in filename. */
    //      header->orig_file_mode; /* File attributes. */
    //      header->host_data;  /* Host data. */

    header->offset; /* Extended file position. */


    // ALT:: write the archive metadata to *another* output file, whose name is derived off `archive_path`?
    std::string metafile_path(cur_main_hdr->archive_path);

    metafile_path += ".meta.nfo";

    char var_name[80];
    mk_variable_name(var_name, sizeof(var_name), name_ptr);

    dump_output_bufptr_t buf(new dump_output_buffer(strlen(comment) + strlen(src) * 2 + 1024));

    TRACE_ME();
    char* dst = reinterpret_cast<char*>(buf->get_append_ref());
    snprintf(dst, buf->get_remaining_usable_size(), "\
FILE index no.: %d\n\
FILE name: %s\n\
FILE filename: %s\n\
FILE VAR name: %s\n\
comment: %s\n\
creation time:         %12lu\n\
filesize uncompressed: %12lu\n\
filesize packed:       %12lu\n\
DUMP FILE offset:      %12lu\n\
FRAGMENT offset:       %12lu\n\
CRC:                     0x%08lx\n\
\n\
--------------------------------------------------------------------\n\
\n",
        file_no, src, name_ptr, var_name, comment,
        (unsigned long)arj_conv_from_os_time(file_stat->ctime),
        (unsigned long)header->origsize,
        (unsigned long)header->compsize,
        (unsigned long)archive_file_offset,
        (unsigned long)header->offset,
        (unsigned long)header->file_crc
    );

    TRACE_ME();
    size_t hdr_len = strlen(dst);
    buf->set_appended_length(hdr_len);

    std::string msg(dst, buf->length());

    // PLUS: append to the archive metafile. But only when we're completely done with the file, as gup code will invoke this method TWICE per file!
    if (increment_file_no)
    {
        std::ofstream out_arc(metafile_path, std::ios_base::app);
        out_arc << msg;
        //out_arc.close();
    }

    delete[] name_ptr;

    buf->clear();

    return buf;
}

dump_output_bufptr_t bindump_archive::generate_file_content(const uint8_t *data, size_t datasize, const fileheader *header)
{
    TRACE_ME();
    
    if (datasize == 0)
    {
        return dump_output_bufptr_t(new dump_output_buffer());
    }

    dump_output_bufptr_t buf(new dump_output_buffer(datasize + 1));

    size_t dstsize = buf->get_remaining_usable_size();
    assert(dstsize == datasize + 1);
    memcpy(buf->get_append_ref(), data, datasize);

    buf->set_appended_length(datasize);

    return buf;
}

dump_output_bufptr_t bindump_archive::generate_end(bool end_of_archive)
{
    TRACE_ME();
    dump_output_bufptr_t buf(new dump_output_buffer());

    at_end_of_archive_action = true;

    // no sentinel bytes at all. nada. zilch. noppes. niente.

    return buf;
}



//===================================================================================
//===================================================================================
//===================================================================================

/*****************************************************************************
 *                                                                           *
 * Constructor and destructor.                                               *
 *                                                                           *
 *****************************************************************************/

cdump_archive::cdump_archive()
{
    TRACE_ME();
}

cdump_archive::~cdump_archive()
{
    TRACE_ME();
}

dump_output_bufptr_t cdump_archive::generate_main_header(const char *archive_path, const char *comment, uint32_t timestamp, size_t arc_output_size)
{
    TRACE_ME();

    if (!comment)
        comment = "";

    dump_output_bufptr_t buf(new dump_output_buffer(1024 + strlen(archive_path) * 2 + strlen(comment)));

    char *filename;
    const char *src;
    uint16 fspecpos = 0;
    unsigned long total_hdr_size, bytes_left;

    filename = arj_conv_from_os_name(archive_path, fspecpos, PATHSYM_FLAG);

    //uint32_t t = arj_conv_from_os_time(gup_time());   /* Modification time (now). */

    char *dst = reinterpret_cast<char *>(buf->get_append_ref());
    snprintf(dst, buf->get_remaining_usable_size(), "/*\n\
    DUMP name: %s\n\
    DUMP filename: %s\n\
    comment: %s\n\
    creation time: %12lu\n\
    archive size: %12zu\n\
*/\n", archive_path, filename, comment, (unsigned long)timestamp, arc_output_size);

    size_t hdr_len = strlen(dst);
    buf->set_appended_length(hdr_len);

    delete[] filename;

    return buf;
}

dump_output_bufptr_t cdump_archive::generate_file_header(const fileheader *header)
{
    TRACE_ME();

    char *name_ptr;
    unsigned long total_hdr_size, bytes_left;
    const char *src;
    uint16 fspec_pos;

    src = header->get_filename();
    const char *comment = header->get_comment();
    if (!comment)
        comment = "";
    const osstat *file_stat = header->get_file_stat();

    name_ptr = arj_conv_from_os_name(src, fspec_pos, PATHSYM_FLAG);

        header->method; /* Packing mode. */
        header->file_type;  /* File type. */

//      header->orig_time_stamp;    /* Time stamp. */
        header->compsize;   /* Compressed size. */
        header->origsize;   /* Original size. */
        header->file_crc;   /* File CRC. */
        fspec_pos;          /* File spec position in filename. */
//      header->orig_file_mode; /* File attributes. */
//      header->host_data;  /* Host data. */

            header->offset; /* Extended file position. */

    dump_output_bufptr_t buf(new dump_output_buffer(strlen(comment) + strlen(src) * 2 + 1024));

    TRACE_ME();
    char *dst = reinterpret_cast<char *>(buf->get_append_ref());
    snprintf(dst, buf->get_remaining_usable_size(), "/*\n\
    FILE index no.: %d\n\
    FILE name: %s\n\
    FILE filename: %s\n\
    comment: %s\n\
    creation time:         %12lu\n\
    filesize uncompressed: %12lu\n\
    filesize packed:       %12lu\n\
    CRC:                     0x%08lx\n\
*/\n", file_no, src, name_ptr, comment,
        (unsigned long)arj_conv_from_os_time(file_stat->ctime),
        (unsigned long)header->origsize,
        (unsigned long)header->compsize,
        (unsigned long)header->file_crc
);

    TRACE_ME();
    size_t hdr_len = strlen(dst);
    buf->set_appended_length(hdr_len);

    delete[] name_ptr;

    return buf;
}

dump_output_bufptr_t cdump_archive::generate_file_content(const uint8_t *data, size_t datasize, const fileheader *header)
{
    TRACE_ME();

    // reckon with additional costs per *line*, calc those as per-input-byte and exaggerate that scaled estimate:
    dump_output_bufptr_t buf(new dump_output_buffer(datasize * (6+1) + 512));

    buf->append_string("const uint8_t *data = {\n");

    for (size_t i = 0; i < datasize; i += 20)
    {
        char *dst = reinterpret_cast<char *>(buf->get_append_ref());
        size_t dstsize = buf->get_remaining_usable_size();

        strcpy(dst, "  ");
        dstsize -= strlen(dst);
        dst += strlen(dst);
        for (int j = 0; j < 20 && i + j < datasize; j++)
        {
            snprintf(dst, dstsize, "0x%02X, ", data[i + j]);
            dstsize -= strlen(dst);
            dst += strlen(dst);
        }
        strcpy(dst, "\n");

        dst = reinterpret_cast<char *>(buf->get_append_ref());
        size_t hdr_len = strlen(dst);
        buf->set_appended_length(hdr_len);
    }

    buf->append_string("};\n\n");

    return buf;
}

dump_output_bufptr_t cdump_archive::generate_end(bool end_of_archive)
{
    TRACE_ME();

    dump_output_bufptr_t buf(new dump_output_buffer());

    char *dst = reinterpret_cast<char *>(buf->get_append_ref());
    snprintf(dst, buf->get_remaining_usable_size(), "\n\n/*\n\
    END OF DUMP\n\
*/\n\n");

    size_t hdr_len = strlen(dst);
    buf->set_appended_length(hdr_len);

    return buf;
}


//===================================================================================
//===================================================================================
//===================================================================================

/*****************************************************************************
 *                                                                           *
 * Constructor and destructor.                                               *
 *                                                                           *
 *****************************************************************************/

asmdump_archive::asmdump_archive()
{
    TRACE_ME();
}

asmdump_archive::~asmdump_archive()
{
    TRACE_ME();
}

dump_output_bufptr_t asmdump_archive::generate_main_header(const char *archive_path, const char *comment, uint32_t timestamp, size_t arc_output_size)
{
    TRACE_ME();

    if (!comment)
        comment = "";

    dump_output_bufptr_t buf(new dump_output_buffer(1024 + strlen(archive_path) * 2 + strlen(comment)));

    char *filename;
    const char *src;
    uint16 fspecpos = 0;
    unsigned long total_hdr_size, bytes_left;

    filename = arj_conv_from_os_name(archive_path, fspecpos, PATHSYM_FLAG);

    //uint32_t t = arj_conv_from_os_time(gup_time());   /* Modification time (now). */

    char *dst = reinterpret_cast<char *>(buf->get_append_ref());
    snprintf(dst, buf->get_remaining_usable_size(), "/*\n\
    DUMP name: %s\n\
    DUMP filename: %s\n\
    comment: %s\n\
    creation time: %12lu\n\
    archive size: %12zu\n\
*/\n\n", archive_path, filename, comment, (unsigned long)timestamp, arc_output_size);

    size_t hdr_len = strlen(dst);
    buf->set_appended_length(hdr_len);

    delete[] filename;

    return buf;
}

dump_output_bufptr_t asmdump_archive::generate_file_header(const fileheader *header)
{
    TRACE_ME();

    char *name_ptr;
    unsigned long total_hdr_size, bytes_left;
    const char *src;
    uint16 fspec_pos;

    src = header->get_filename();
    const char *comment = header->get_comment();
    if (!comment)
        comment = "";

    const osstat *file_stat = header->get_file_stat();

    name_ptr = arj_conv_from_os_name(src, fspec_pos, PATHSYM_FLAG);

        header->method; /* Packing mode. */
        header->file_type;  /* File type. */

//      header->orig_time_stamp;    /* Time stamp. */
        header->compsize;   /* Compressed size. */
        header->origsize;   /* Original size. */
        header->file_crc;   /* File CRC. */
        fspec_pos;          /* File spec position in filename. */
//      header->orig_file_mode; /* File attributes. */
//      header->host_data;  /* Host data. */

            header->offset; /* Extended file position. */

    dump_output_bufptr_t buf(new dump_output_buffer(strlen(comment) + strlen(src) * 2 + 1024));

    TRACE_ME();
    char *dst = reinterpret_cast<char *>(buf->get_append_ref());
    snprintf(dst, buf->get_remaining_usable_size(), "\n/*\n\
    FILE index no.: %d\n\
    FILE name: %s\n\
    FILE filename: %s\n\
    comment: %s\n\
    creation time:         %12lu\n\
    filesize uncompressed: %12lu\n\
    filesize packed:       %12lu\n\
    CRC:                     0x%08lx\n\
*/\n\n", file_no, src, name_ptr, comment,
        (unsigned long)arj_conv_from_os_time(file_stat->ctime),
        (unsigned long)header->origsize,
        (unsigned long)header->compsize,
        (unsigned long)header->file_crc
);

    TRACE_ME();
    size_t hdr_len = strlen(dst);
    buf->set_appended_length(hdr_len);

    delete[] name_ptr;

    return buf;
}

dump_output_bufptr_t asmdump_archive::generate_file_content(const uint8_t *data, size_t datasize, const fileheader *header)
{
    TRACE_ME();

    // reckon with additional costs per *line*, calc those as per-input-byte and exaggerate that scaled estimate:
    dump_output_bufptr_t buf(new dump_output_buffer(datasize * (6+1) + 512));

    buf->append_string("file_data:\n");

    for (size_t i = 0; i < datasize; i += 20)
    {
        char *dst = reinterpret_cast<char *>(buf->get_append_ref());
        size_t dstsize = buf->get_remaining_usable_size();

        strcpy(dst, "    dc.b ");
        dstsize -= strlen(dst);
        dst += strlen(dst);
        for (int j = 0; j < 20 && i + j < datasize; j++)
        {
            snprintf(dst, dstsize, "0x%02X, ", data[i + j]);
            dstsize -= strlen(dst);
            dst += strlen(dst);
        }
		dst -= 2;
        strcpy(dst, "\n");

        dst = reinterpret_cast<char *>(buf->get_append_ref());
        size_t hdr_len = strlen(dst);
        buf->set_appended_length(hdr_len);
    }

    buf->append_string("\n; --------------------- end of packed file data ---------------------\n\n");

    return buf;
}

dump_output_bufptr_t asmdump_archive::generate_end(bool end_of_archive)
{
    TRACE_ME();

    dump_output_bufptr_t buf(new dump_output_buffer());

    char *dst = reinterpret_cast<char *>(buf->get_append_ref());
    snprintf(dst, buf->get_remaining_usable_size(), "\n\n/*\n\
    END OF DUMP\n\
*/\n\n");

    size_t hdr_len = strlen(dst);
    buf->set_appended_length(hdr_len);

    return buf;
}

#endif // ENABLE_DUMP_OUTPUT_MODES


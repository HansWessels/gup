/*
 * Archive manager.
 *
 *     *DUMP archive class.
 */

#include "gup.h"

#if ENABLE_DUMP_OUTPUT_MODES

#include <memory>
#include <string>
#include <assert.h>

#include "arj_hdr.h"
#include "arj_arc.h"


// buffer type used to hold dump mode headers while they are generated.
class dump_output_buffer
{
protected:
    uint8_t *base_ptr;      // pointer to allocated buffer space
    size_t used_length;     // length (in bytes) filled with useful data
    size_t alloc_size;  
    
public: 
    dump_output_buffer() : dump_output_buffer(512)
    {
    }   

    dump_output_buffer(size_t max_size) : used_length(0)
    {
        base_ptr = new uint8_t[max_size];
        alloc_size = max_size;
    }   

    ~dump_output_buffer()
    {
        delete[] base_ptr;
    }   

    // hacky, can cause memory failures when used incorrectly:
    // get a non-const reference to the buffer space.
    uint8_t *get_start_ref() 
    {
        return base_ptr;
    }

    size_t size() 
    {
        return alloc_size;
    }

    size_t length()
    {
        return used_length;
    }

    // ditto: pointer + size for append to already buffered data:
    uint8_t *get_append_ref() 
    {
        return base_ptr + used_length;
    }

    size_t get_remaining_usable_size()
    {
        assert(alloc_size > used_length);  // still some space left to fill? no? then barf!
        return alloc_size - used_length;
    }

    // and to complement the hacky/unsafe interface above: here's where
    // you tell how much useful stuff you appended to the buffer space:
    void set_appended_length(size_t len) 
    {
        assert(used_length + len <= alloc_size);
        used_length += len;
    }

    void append_string(const char *str) 
    {
        assert(str);
        size_t len = strlen(str);
        assert(len < get_remaining_usable_size());  // still some space left to fill? no? then barf!
        memmove(get_append_ref(), str, len);
		set_appended_length(len);
    }

    void set_total_used_length(size_t len) 
    {
        assert(len <= alloc_size);
        used_length = len;
    }

    void clear() 
    {
        used_length = 0;
    }
};

typedef std::unique_ptr<dump_output_buffer> dump_output_bufptr_t;



class dump_archive : public arj_archive
{
  protected:
    dump_mainheader *cur_main_hdr;

    int file_no;                                    // 'index' number of the file in the archive, starts counting at 1 (1-based index)
    bool increment_file_no;                         // signal that the file content has been written and the index should be updated

    long archive_file_offset;                       // offset in archive file (output) where current compresed file data is written
    bool at_end_of_archive_action;
        
  public:
    dump_archive(void);
    virtual ~dump_archive(void);

    virtual gup_result write_end_of_volume(int mv);
    virtual gup_result close_archive(int ok);

    virtual gup_result write_main_header(const mainheader *header);
    virtual gup_result write_file_header(const fileheader *header);
    virtual gup_result write_file_trailer(const fileheader *header);

    // extra:
    
    virtual gup_result gup_io_flush_header(buf_fhandle_t *file);
    virtual gup_result gup_io_flush_packed_data(buf_fhandle_t *file);

    /*
     * GUP I/O virtualization functions.
     */

    virtual buf_fhandle_t *gup_io_open(const char *name, unsigned char *buf_start, unsigned long buf_size, int omode, gup_result *result);
    virtual gup_result gup_io_close(buf_fhandle_t *file);
    // Seek to a position in the file.
    virtual gup_result gup_io_seek(buf_fhandle_t *file, long offset, int seekmode, long *new_pos);
    // Return the current position in the file.
    virtual gup_result gup_io_tell(buf_fhandle_t *file, long *fpos);
    // Write data to a file.
    virtual gup_result gup_io_write(buf_fhandle_t *file, const void *buffer, unsigned long count, unsigned long *real_count);
    // Read data from a file.
    virtual gup_result gup_io_read(buf_fhandle_t *file, void *buffer, unsigned long count, unsigned long *real_count);
    // Make sure there at least 'count' bytes free in the file buffer. If necessary the file buffer is flushed.
    virtual gup_result gup_io_write_announce(buf_fhandle_t *file, unsigned long count);
    // Fill the file buffer.
    virtual gup_result gup_io_fill(buf_fhandle_t *file);
    // Get a pointer to the current position in the file buffer and return the number of bytes that can be written into the buffer or can be read from the buffer.
    virtual uint8 *gup_io_get_current(buf_fhandle_t *file, unsigned long *bytes_left);
    // Set the pointer in the file buffer to a new position.
    virtual void gup_io_set_current(buf_fhandle_t *file, uint8 *new_pos);
    // Set the current position in the file to the given value. Any bytes after this position that are in the buffer are discarded.
    virtual void gup_io_set_position(buf_fhandle_t *file, long position);
    
    /*
     * Overloadable handlers used for producing the various dump output formats.
     */
    virtual dump_output_bufptr_t generate_main_header(const char *archive_path, const char *comment, uint32_t timestamp, size_t arc_output_size) = 0;
    virtual dump_output_bufptr_t generate_file_header(const fileheader *header) = 0;
    virtual dump_output_bufptr_t generate_file_content(const uint8_t *data, size_t datasize, const fileheader *header) = 0;
    virtual dump_output_bufptr_t generate_end(bool end_of_archive) = 0;
};




class bindump_archive : public dump_archive
{
  public:
    bindump_archive(void);
    virtual ~bindump_archive(void);

    virtual gup_result close_archive(int ok);

    virtual dump_output_bufptr_t generate_main_header(const char *archive_path, const char *comment, uint32_t timestamp, size_t arc_output_size);
    virtual dump_output_bufptr_t generate_file_header(const fileheader *header);
    virtual dump_output_bufptr_t generate_file_content(const uint8_t *data, size_t datasize, const fileheader *header);
    virtual dump_output_bufptr_t generate_end(bool end_of_archive);
};


class cdump_archive : public dump_archive
{
  public:
    cdump_archive(void);
    virtual ~cdump_archive(void);

    virtual gup_result close_archive(int ok);

    virtual dump_output_bufptr_t generate_main_header(const char *archive_path, const char *comment, uint32_t timestamp, size_t arc_output_size);
    virtual dump_output_bufptr_t generate_file_header(const fileheader *header);
    virtual dump_output_bufptr_t generate_file_content(const uint8_t *data, size_t datasize, const fileheader *header);
    virtual dump_output_bufptr_t generate_end(bool end_of_archive);
};


class asmdump_archive : public dump_archive
{
  public:
    asmdump_archive(void);
    virtual ~asmdump_archive(void);

    virtual gup_result close_archive(int ok);

    virtual dump_output_bufptr_t generate_main_header(const char *archive_path, const char *comment, uint32_t timestamp, size_t arc_output_size);
    virtual dump_output_bufptr_t generate_file_header(const fileheader *header);
    virtual dump_output_bufptr_t generate_file_content(const uint8_t *data, size_t datasize, const fileheader *header);
    virtual dump_output_bufptr_t generate_end(bool end_of_archive);
};

#endif

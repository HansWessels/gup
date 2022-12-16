/*
 * simple arj file buffering by Mr Ni!
 */

#ifndef __FILE_BUF_H__
#define __FILE_BUF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ARJ_SEEK_SET    0
#define ARJ_SEEK_CUR    1
#define ARJ_SEEK_END    2

/* file create, for compression */
buf_fhandle_t* buf_open_write(const char* naam, char* buf_start, long buf_size, int add_mode);

/* file close */
gup_result buf_close_write(buf_fhandle_t* buf);

/* buf_write announce, for compression */
gup_result buf_write_announce(long count, buf_fhandle_t* buf);

/* file seek, for compression */
long buf_seek_write(long offset, int seekmode, buf_fhandle_t* buf);

/* file seek special, for compression */
long buf_seek_and_fill(long count, long offset, buf_fhandle_t* buf);

/* file open, for decompression */
buf_fhandle_t* buf_open_read(const char* naam, char* buf_start, long buf_size);

/* file seek, for decompression, fills buffer too */
long buf_seek_read(long offset, int seekmode, buf_fhandle_t* buf);

/* buf fill, for decompression */
gup_result buf_fill(buf_fhandle_t* br_buf);

/* buf_tell, get file pos, for decompression */
unsigned long buf_tell(buf_fhandle_t* buf);

/* getch */
uint8 bufget_byte(buf_fhandle_t* buf);

/* buf close for decompression */
void buf_close_read(buf_fhandle_t* buf);

#ifdef __cplusplus
}
#endif
#endif
 
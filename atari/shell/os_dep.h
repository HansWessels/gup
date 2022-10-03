#ifndef __OS_DEP_H__
#define __OS_DEP_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern gup_result set_file_info(const char *filename, fileheader *header);
extern gup_result make_writeable(const char *filename);
extern int is_readonly(const char *filename);
extern int is_hidden(fileheader *header);

#ifdef __cplusplus
}
#endif

#endif

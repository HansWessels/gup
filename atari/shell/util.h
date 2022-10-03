
#ifndef __UTIL_H
#define __UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

extern char *make_path(const char *path, const char *name, int extra);
extern const char *get_fname_pos(const char *pathname);
extern int exist(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_H */

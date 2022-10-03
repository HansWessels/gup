
#ifndef __ARJ_HEADER_H__
#define __ARJ_HEADER_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern void arj_close_and_create_volume(packstruct* com); /* maakt nieuw ARJ volume */
extern unsigned long arj_make_main_header(char *naam, uint8 flags, packstruct* com);
extern long arj_headerlen(const char *naam, packstruct *com); /* calculates the header length */
extern gup_result arj_make_header(const char *naam, long origsize, long packedsize,
                                 long filestart, int mode, int attrib,
                                 uint16 datum, uint16 tijd, uint8 flag, packstruct* com);
extern gup_result arj_close_archive(packstruct *com); /* sluit archive af */

#define strend(s)   (s + strlen(s))


#ifdef __cplusplus
}
#endif

#endif
 
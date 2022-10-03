
#ifndef __LHA_HEADER_H__
#define __LHA_HEADER_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern unsigned long lha_make_main_header(char *naam, uint8 flags, packstruct* com);
extern long lha_headerlen(const char *naam, packstruct *com); /* calculates the header length */
extern gup_result lha_make_header(const char *naam, long origsize, long packedsize,
                            long filestart, int mode, int attrib,
                            uint16 datum, uint16 tijd, uint8 flag, packstruct* com);
extern gup_result lha_close_archive(packstruct *com); /* sluit archive af */

#define strend(s)   (s + strlen(s))


#ifdef __cplusplus
}
#endif

#endif
 
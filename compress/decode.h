
#ifndef __DECODE_H__
#define __DECODE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "compr_io.h"

#define CHARS 256               /* aantal karakters */
#define MAXMATCH 258            /* maximale match */
#define ARJ_NPT 17                     /* arj: max aantal pointers */
#define ARJ_PBIT 5                     /* arj: aantal bits in pointer */
#define LHA_NPT 14                     /* lha: max aantal pointers */
#define LHA_PBIT 4                     /* lha: aantal bits in pointer */

#ifdef __cplusplus
}

#endif

#endif


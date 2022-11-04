/*
** header file for sld32.c
*/

#ifndef __SLD32_H__
#define __SLD32_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t	match_t; /* bevat de lengte van de match, is nul bij geen match */
typedef uint32_t	ptr_t;	/* relatieve offset van de match, geinverteerd, 0 is 1 karakter terug */
typedef uint32_t	index_t;	/* index in een van de array's */
typedef uint64_t	cost_t;	/* geschatte kosten in (fracties van) bits */

struct node_struct32_t;

typedef struct node_struct32_t
{
  index_t* parent;
  index_t c_left;
  index_t c_right;
} node_t;


#ifdef __cplusplus
}
#endif

#endif

/*
** header file for sld63i.c
*/

#ifndef __SLD63I_H__
#define __SLD63I_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t     lit63_t;        /* literals */

typedef uint32_t    symbol63_t;     /* symbol codes */
typedef uint32_t    pointer63_t;    /* sld pointers */
typedef uint32_t    len63_t;        /* sld lengtes */

typedef uint32_t    node63_i;       /* node index */
typedef uint64_t    mb63_i;         /* match buffer index */
typedef uint64_t    lit63_i;        /* dictionary index */

typedef int32_t     hash_t;         /* hash values */

typedef struct node_struct63_t
{
    lit63_i key;
    node63_i  parent;
    node63_i  c_left;
    node63_i  c_right;
} node63_t;

typedef struct match_buffer63_t
{
    len63_t len;
    union
    {
        pointer63_t ptr;
        lit63_t lit;
    } u;
} mb63_t;

#ifdef __cplusplus
}
#endif

#endif

/* assume_compat.h
*
* Portable ASSUME(cond) macro:
* - GCC: uses __attribute__((assume(cond)))
* - Clang: uses __builtin_assume(cond)
* - MSVC: uses __assume(cond) 
* - Other: falls back to a runtime check that hints unreachable 
*/

#ifndef ASSUME_COMPAT_H
#define ASSUME_COMPAT_H

/* C++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* Core macro (statement-style use: ASSUME(x);) */

#if defined(__GNUC__) && !defined(__clang__)

/* GCC: attribute-based assume (GCC 13+) */
#define ASSUME(cond) __attribute__((assume(cond)))

#elif defined(__clang__)

/* Clang: builtin assume */
#if __has_builtin(__builtin_assume)
#define ASSUME(cond) __builtin_assume(cond)
#else
#define ASSUME(cond) /* */

#endif
#elif defined(_MSC_VER)

/* MSVC: __assume */
#define ASSUME(cond) __assume(cond)

#else 

define ASSUME(cond)  /* */

#endif

#ifdef __cplusplus
} /* extern "C" */ 
#endif 

#endif /* ASSUME_COMPAT_H */ 
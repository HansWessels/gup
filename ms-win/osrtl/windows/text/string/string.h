
/**************************************************************************
 * File:     osrtl/windows/text/string/string.h                           *
 * Contents: Text strings.                                                *
 * Document: osrtl/windows/text/string/string.d                           *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/text/string/string.h  -  Text strings

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:21  erick
// First version of the MS-Windows shell (currently only osrtl).
//


/**************************************************************************
 * Detailed history.                                                      *
 **************************************************************************/

// -------------------------------------------------------------------------
// Revision  Concise description of the changes
// Dates
// Author
// -------------------------------------------------------------------------
//  1.1.000  First issue of this header file.
// 19970622
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_TEXT_STRING
#define OSRTL_TEXT_STRING


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

// Actual functionality.

unsigned long                 // number of characters in a string
StringLength                  // determines the number of chars in a string
(wchar_t const *  s);         // I: string

unsigned long                 // number of characters in a string
StringLength                  // determines the number of chars in a string
(char const *  s);            // I: string

wchar_t *                     // destination string
StringCopy                    // copies a string to another string
(wchar_t *        dst,        // O: destination string
 wchar_t const *  src);       // I: source string

inline
wchar_t *                     // duplicate of the string, on the heap
StringDuplicate               // creates a copy of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the duplicate
    wchar_t const *  s);      // I: string to copy
#else
   (wchar_t const *  ,        // -: description of the duplicate
    wchar_t const *  s);      // I: string to copy
#endif

inline
osresult                      // result code
StringDuplicate               // creates a converted duplicate of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the converted duplicate
    char const *     a,       // I: string to duplicate and convert
    wchar_t * *      u);      // O: converted duplicate of the string
#else
   (wchar_t const *  ,        // -: description of the converted duplicate
    char const *     a,       // I: string to duplicate and convert
    wchar_t * *      u);      // O: converted duplicate of the string
#endif

inline
osresult                      // result code
StringDuplicate               // creates a converted duplicate of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the converted duplicate
    wchar_t const *  u,       // I: string to duplicate and convert
    char * *         a);      // O: converted duplicate of the string
#else
   (wchar_t const *  ,        // -: description of the converted duplicate
    wchar_t const *  u,       // I: string to duplicate and convert
    char * *         a);      // O: converted duplicate of the string
#endif

osbool                        // strings are equal?
StringEqual                   // checks if two strings are equal
(wchar_t const *  s1,         // I: string to compare
 wchar_t const *  s2);        // I: string to compare

inline
osbool                        // strings are different?
StringDifferent               // checks if two strings are different
(wchar_t const *  s1,         // I: string to compare
 wchar_t const *  s2);        // I: string to compare

// Helper functions.

wchar_t *                     // duplicate of the string, on the heap
priv_StringDuplicate          // creates a copy of a string
(wchar_t const *  descr,      // I: description of the duplicate
 wchar_t const *  s);         // I: string to copy

osresult                      // result code
priv_StringDuplicate          // creates a converted duplicate of a string
(wchar_t const *  descr,      // I: description of the converted duplicate
 char const *     a,          // I: string to duplicate and convert
 wchar_t * *      u);         // O: converted duplicate of the string

osresult                      // result code
priv_StringDuplicate          // creates a converted duplicate of a string
(wchar_t const *  descr,      // I: description of the converted duplicate
 wchar_t const *  u,          // I: string to duplicate and convert
 char * *         a);         // O: converted duplicate of the string


/**************************************************************************
 * Function: StringDuplicate                                              *
 **************************************************************************/

inline
wchar_t *                     // duplicate of the string, on the heap
StringDuplicate               // creates a copy of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the duplicate
    wchar_t const *  s)       // I: string to copy
#else
   (wchar_t const *  ,        // -: description of the duplicate
    wchar_t const *  s)       // I: string to copy
#endif

{  // StringDuplicate

   #ifdef OSRTL_DEBUGGING
      return priv_StringDuplicate(descr, s);
   #else
      return priv_StringDuplicate(static_cast<wchar_t const *>(OSNULL), s);
   #endif

}  // StringDuplicate


/**************************************************************************
 * Function: StringDuplicate                                              *
 **************************************************************************/

inline
osresult                      // result code
StringDuplicate               // creates a converted duplicate of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the converted duplicate
    char const *     a,       // I: string to duplicate and convert
    wchar_t * *      u)       // O: converted duplicate of the string
#else
   (wchar_t const *  ,        // -: description of the converted duplicate
    char const *     a,       // I: string to duplicate and convert
    wchar_t * *      u)       // O: converted duplicate of the string
#endif

{  // StringDuplicate

   #ifdef OSRTL_DEBUGGING
      return priv_StringDuplicate(descr,
                                  a,
                                  u);
   #else
      return priv_StringDuplicate(static_cast<wchar_t const *>(OSNULL),
                                  a,
                                  u);
   #endif

}  // StringDuplicate


/**************************************************************************
 * Function: StringDuplicate                                              *
 **************************************************************************/

inline
osresult                      // result code
StringDuplicate               // creates a converted duplicate of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the converted duplicate
    wchar_t const *  u,       // I: string to duplicate and convert
    char * *         a)       // O: converted duplicate of the string
#else
   (wchar_t const *  ,        // -: description of the converted duplicate
    wchar_t const *  u,       // I: string to duplicate and convert
    char * *         a)       // O: converted duplicate of the string
#endif

{  // StringDuplicate

   #ifdef OSRTL_DEBUGGING
      return priv_StringDuplicate(descr,
                                  u,
                                  a);
   #else
      return priv_StringDuplicate(static_cast<wchar_t const *>(OSNULL),
                                  u,
                                  a);
   #endif

}  // StringDuplicate


/**************************************************************************
 * Function: StringDifferent                                              *
 **************************************************************************/

inline
osbool                  // strings are different?
StringDifferent         // checks if two strings are different
(wchar_t const *  s1,   // I: string to compare
 wchar_t const *  s2)   // I: string to compare

{  // StringDifferent

   return StringEqual(s1, s2) ? OSFALSE : OSTRUE;

}  // StringDifferent


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#endif


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/text/char/char.h .                                *
 **************************************************************************/


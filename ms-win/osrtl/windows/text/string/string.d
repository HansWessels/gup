
----------------------------------------------------------------------------
---  File:     osrtl/windows/text/string/string.d
---  Contents: Text strings.
---  Module:   <osrtl><text><string>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/text/string/string.d  -  Text strings
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:21  erick
---  First version of the MS-Windows shell (currently only osrtl).
---
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Detailed history
----------------------------------------------------------------------------
Revision  Concise description of the changes
Dates
Author
----------------------------------------------------------------------------
 1.1.000  First issue of this documentation file.
19970628
19970822
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><text><string>
----------------------------------------------------------------------------

   The module <osrtl><text><string> supplies methods to handle text strings.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><text><string>
----------------------------------------------------------------------------

   <osrtl><text><string> supplies a bunch of functions that handle text
strings.  These functions are independent.  All of these functions can be
used directly by any other part of the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><text><string> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <string> uses basic types and constants, such as <osbool> and
         <NULL>.

      Component <osrtl><error>.
         <string> uses the error type <osresult>.

      Component <osrtl><heap>.
         <string> needs access to the heap to allocate a copy of a string.

      Module <osrtl><text><string>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><text><string>
----------------------------------------------------------------------------

unsigned long           // number of characters in a string
StringLength            // determines the number of chars in a string
(wchar_t const *  s);   // I: string

unsigned long           // number of characters in a string
StringLength            // determines the number of chars in a string
(char const *  s);      // I: string

   Counts the number of characters in <s>.  Does not count the terminating
null character.  Returns 0 if a null pointer is passed to the function.
   This function is the platform-independent version of <strlen>.

----------------------------------------------------------------------------

wchar_t *                 // destination string
StringCopy                // copies a string to another string
(wchar_t *        dst,    // O: destination string
 wchar_t const *  src);   // I: source string

   Copies the source string <src>, including its terminating null character,
to the destination string <dst>.  <dst> must be large enough to hold a copy
of <src> including the terminating null character.  The function returns
<dst> if successful, or static_cast<wchar_t *>(OSNULL) if the string could
not be copied for whatever reason.
   This function is the platform-independent version of <strcpy>.

----------------------------------------------------------------------------

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

   Allocates memory on the heap for a copy of the source string, then copies
the source string to the copy.  The last two versions of <StringDuplicate>
allow to convert the string from ANSI to UNICODE or vice versa while making
the copy.  In debugging mode, <descr> will be the description of the copy on
the heap.
   If the source string is a null pointer, no memory will be allocated on
the heap; instead, the output string will also be a null pointer.
   All versions of this function are <inline>; they just call the
corresponding function <priv_StringDuplicate> to do the actual work.
This approach allows the compiler to optimise away the description string
when compiling in release mode.
   This function is the platform-independent version of the (non-ANSI)
function <strdup>.

----------------------------------------------------------------------------

osbool                   // strings are equal?
StringEqual              // checks if two strings are equal
(wchar_t const *  s1,    // I: string to compare
 wchar_t const *  s2);   // I: string to compare

   Checks if two strings are equal.  They are if both pointers are
static_cast<wchar_t const *>(OSNULL) , or if the pointers point to strings
that contain the same characters up to and including the L'\0' .

----------------------------------------------------------------------------

inline
osbool                   // strings are different?
StringDifferent          // checks if two strings are different
(wchar_t const *  s1,    // I: string to compare
 wchar_t const *  s2);   // I: string to compare

   Checks if two strings are different.  This call translates into a call to
<StringEqual> and the use of the negation operator.

----------------------------------------------------------------------------

wchar_t *                  // duplicate of the string, on the heap
priv_StringDuplicate       // creates a copy of a string
(wchar_t const *  descr,   // I: description of the duplicate
 wchar_t const *  s);      // I: string to copy

osresult                   // result code
priv_StringDuplicate       // creates a converted duplicate of a string
(wchar_t const *  descr,   // I: description of the converted duplicate
 char const *     a,       // I: string to duplicate and convert
 wchar_t * *      u);      // O: converted duplicate of the string

osresult                   // result code
priv_StringDuplicate       // creates a converted duplicate of a string
(wchar_t const *  descr,   // I: description of the converted duplicate
 wchar_t const *  u,       // I: string to duplicate and convert
 char * *         a);      // O: converted duplicate of the string

   These functions implement the functionality of <StringDuplicate>.  Refer
to the descriptions of <StringDuplicate> for an explanation of the purpose
of <priv_StringDuplicate>.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/text/string/string.d .
----------------------------------------------------------------------------



/**************************************************************************
 * File:     osrtl/windows/text/string/string.cpp                         *
 * Contents: Text strings.                                                *
 * Document: osrtl/windows/text/string/string.d                           *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/text/string/string.cpp  -  Text strings

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:20  erick
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
//  1.1.000  First issue of this source module.
// 19970622
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other components of the unit <osrtl>
#include "osrtl/windows/basics/basics.h"   // basic definitions
#include "osrtl/windows/error/error.h"     // error handling
#include "osrtl/windows/heap/heap.h"       // heap

// Header file of the current module.
#include "osrtl/windows/text/string/string.h"


/*************************************************************************
 * Function: StringLength                                                *
 *************************************************************************/

unsigned long          // number of characters in the string
StringLength           // determines the number of chars in a string
(wchar_t const *  s)   // I: string

{  // StringLength

   unsigned long  length = 0UL;   // length of <s>

   if (s != static_cast<wchar_t const *>(OSNULL))
      while (*s++ != L'\0')
         ++length;

   return length;

}  // StringLength


/*************************************************************************
 * Function: StringLength                                                *
 *************************************************************************/

unsigned long       // number of characters in the string
StringLength        // determines the number of chars in a string
(char const *  s)   // I: string

{  // StringLength

   unsigned long  length = 0UL;   // length of <s>

   if (s != static_cast<char const *>(OSNULL))
      while (*s++ != '\0')
         ++length;

   return length;

}  // StringLength


/*************************************************************************
 * Function: StringCopy                                                  *
 *************************************************************************/

wchar_t *                // destination string
StringCopy               // copies a string to another string
(wchar_t *        dst,   // O: destination string
 wchar_t const *  src)   // I: source string

{  // StringCopy

   wchar_t *  result = dst;   // function result

   if (result != static_cast<wchar_t *>(OSNULL))
      if (src != static_cast<wchar_t const *>(OSNULL))
         while ((*dst++ = *src++) != L'\0')
            (void)0;
      else
         *result = L'\0';

   return result;

}  // StringCopy


/**************************************************************************
 * Function: StringEqual                                                  *
 **************************************************************************/

osbool                  // strings are equal?
StringEqual             // checks if two strings are equal
(wchar_t const *  s1,   // I: string to compare
 wchar_t const *  s2)   // I: string to compare

{  // StringEqual

   osbool  equal = OSFALSE;   // strings are equal?

   if (s1 == static_cast<wchar_t const *>(OSNULL))
   {
      if (s2 == static_cast<wchar_t const *>(OSNULL))
         equal = OSTRUE;
   }
   else
      if (s2 != static_cast<wchar_t const *>(OSNULL))
      {
         unsigned long  i = 0UL;   // index into <s1> and <s2>
         wchar_t        c;         // current character of <s1>

         while ((c = s1[i]) == s2[i]  &&  c != L'\0')
            ++i;
         if (c == s2[i])
            equal = OSTRUE;
      }

   return equal;

}  // StringEqual


/**************************************************************************
 * Function: priv_StringDuplicate                                         *
 **************************************************************************/

wchar_t *                  // duplicate of the string, on the heap
priv_StringDuplicate       // creates a copy of a string
(wchar_t const *  descr,   // I: description of the duplicate
 wchar_t const *  s)       // I: string to copy

{  // priv_StringDuplicate

   wchar_t *  duplicate;   // copy of <s>, allocated on the heap

   duplicate = static_cast<wchar_t *>(OSNULL);
   if (s != static_cast<wchar_t const *>(OSNULL))
   {
      unsigned long  length;  // length of <s>, plus 1 for terminating L'\0'

      length = StringLength(s) + 1;

      {
         osheap<wchar_t>  heap;

         duplicate = heap.Allocate(descr, length);
      }

      if (duplicate != static_cast<wchar_t *>(OSNULL))
      {
         unsigned long  i;

         for (i=0UL; i<length; i++)
            duplicate[i] = s[i];
      }
   }

   return duplicate;

}  // priv_StringDuplicate


/**************************************************************************
 * Function: priv_StringDuplicate                                         *
 **************************************************************************/

osresult                   // result code
priv_StringDuplicate       // creates a converted duplicate of a string
(wchar_t const *  descr,   // I: description of the converted duplicate
 char const *     a,       // I: string to duplicate and convert
 wchar_t * *      u)       // O: converted duplicate of the string

{  // priv_StringDuplicate

   osresult  result;   // result code

   if (a != static_cast<char const *>(OSNULL))
   {
      osheap<wchar_t>  heap;   // heap

      *u = heap.Allocate(descr, StringLength(a)+1);

      if (*u != static_cast<wchar_t *>(OSNULL))
         if (StringConvert(a, *u))
            result = OSRESULT_OK;
         else
         {
            heap.Deallocate(*u);
            result = OSERROR_NOCHARCONVERT;
         }
      else
         result = OSERROR_NOMEMORY;
   }
   else
   {
      *u = static_cast<wchar_t *>(OSNULL);
      result = OSRESULT_OK;
   }

   return result;

}  // priv_StringDuplicate


/**************************************************************************
 * Function: priv_StringDuplicate                                         *
 **************************************************************************/

osresult                   // result code
priv_StringDuplicate       // creates a converted duplicate of a string
(wchar_t const *  descr,   // I: description of the converted duplicate
 wchar_t const *  u,       // I: string to duplicate and convert
 char * *         a)       // O: converted duplicate of the string

{  // priv_StringDuplicate

   osresult  result;   // result code

   if (u != static_cast<wchar_t const *>(OSNULL))
   {
      osheap<char>  heap;   // heap

      *a = heap.Allocate(descr, StringLength(u)+1);

      if (*a != static_cast<char *>(OSNULL))
         if (StringConvert(u, *a))
            result = OSRESULT_OK;
         else
         {
            heap.Deallocate(*a);
            result = OSERROR_NOCHARCONVERT;
         }
      else
         result = OSERROR_NOMEMORY;
   }
   else
   {
      *a = static_cast<char *>(OSNULL);
      result = OSRESULT_OK;
   }

   return result;

}  // priv_StringDuplicate


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/text/string/string.cpp .                          *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/basics/convert/convert.cpp                     *
 * Contents: Type conversions.                                            *
 * Document: osrtl/windows/basics/convert/convert.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/basics/convert/convert.cpp  -  Type conversions

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:04  erick
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
//  1.1.000  First issue of this module source file.
// 19970714
// 19970818
// EAS
// -------------------------------------------------------------------------
//  1.1.001  A minor change was made to the argument list of the second form
// 19970822  <RoundFloatToIntegral>: <l> was changed to <i>.  This makes
// 19970822  this file conform to osrtl/windows/basics/convert/convert.h .
// EAS
// -------------------------------------------------------------------------
//  1.2.000  Identical to revision 1.1.001.  Formalised revision.
// 19970825
// 19970825
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// The unit <osrtl> is the only unit that is allowed to include files that
// have to do with the operating system and the RTL.
#include <limits.h>    // ranges of basic types
#include <stdlib.h>    // standard library

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other modules of the component <basics>.
#include "osrtl/windows/basics/type/type.h"   // types

// Header file of the current module.
#include "osrtl/windows/basics/convert/convert.h"


/**************************************************************************
 * Function: RoundFloatToIntegral                                         *
 **************************************************************************/

osbool                 // value can be converted?
RoundFloatToIntegral   // converts a <float> to an <int>, rounding it
(float  f,             // I: value to convert
 int *  i)             // O: converted value

{  // RoundFloatToIntegral

   osbool  result;   // value can be converted?
   long    l;        // converted value

   result = RoundFloatToIntegral(f, &l);
   if (result)
      if (    l >= static_cast<long>(INT_MIN)
          &&  l <= static_cast<long>(INT_MAX))
         *i = static_cast<int>(l);
      else
         result = OSFALSE;

   return result;

}  // RoundFloatToIntegral


/**************************************************************************
 * Function: RoundFloatToIntegral                                         *
 **************************************************************************/

osbool                 // value can be converted?
RoundFloatToIntegral   // converts a <float> to a <long>, rounding it
(float   f,            // I: value to convert
 long *  i)            // O: converted value

{  // RoundFloatToIntegral

   osbool  result = OSFALSE;   // value can be converted?

   if (    f >= static_cast<float>(LONG_MIN) - 0.5
       &&  f <  static_cast<float>(LONG_MAX) + 0.5)
   {
      result = OSTRUE;
      if (f < static_cast<float>(LONG_MIN) + 0.5)
         *i = LONG_MIN;
      else
         if (f >= static_cast<float>(LONG_MAX) - 0.5)
            *i = LONG_MAX;
         else
         {
            int  conv;   // converted value

            conv = static_cast<int>(f);
            if (static_cast<float>(conv) <= f - 0.5)
               ++conv;
            else
               if (static_cast<float>(conv) > f + 0.5)
                  ++conv;
            *i = conv;
         }
   }

   return result;

}  // RoundFloatToIntegral


/**************************************************************************
 * Function: CharConvert                                                  *
 **************************************************************************/

osbool           // character can be converted?
CharConvert      // converts an ANSI character to a UNICODE character
(char       a,   // I: ANSI character
 wchar_t *  u)   // O: UNICODE character

{  // CharConvert

   osbool   ok;    // character can be converted?
   wchar_t  uni;   // UNICODE equivalent of <a> - prevents modification
                   //    of *u if the conversion cannot be made

   switch (mbtowc(&uni, &a, 1))
   {
      case -1: // <a> is an invalid character
      default: // invalid outcome
               ok = OSFALSE;
               break;

      case  0: // <a> is the null character
               *u = L'\0';
               ok = OSTRUE;
               break;

      case  1: // <a> has been converted
               *u = uni;
               ok = OSTRUE;
               break;
   }

   return ok;

}  // CharConvert


/**************************************************************************
 * Function: CharConvert                                                  *
 **************************************************************************/

osbool         // character can be converted?
CharConvert    // converts a UNICODE character to an ANSI character
(wchar_t  u,   // I: UNICODE character
 char *   a)   // O: ANSI character

{  // CharConvert

   osbool  ok;                   // character can be converted?
   char    ansi[MB_CUR_MAX+1];   // multi-byte equivalent of <u>

   if (u != L'\0')
      switch (wctomb(ansi, u))
      {
         case -1: // <u> is not a valid multi-byte character
         default: // <u> could not be converted, or <u> results in a multi-
                  //    byte character that consists of more than one byte
                  ok = OSFALSE;
                  break;

         case  1: // <u> has been converted
                  *a = ansi[0];
                  ok = OSTRUE;
                  break;
      }
   else
   {
      *a = '\0';
      ok = OSTRUE;
   }

   return ok;

}  // CharConvert


/**************************************************************************
 * Function: StringConvert                                                *
 **************************************************************************/

osbool              // string can be converted?
StringConvert       // converts an ANSI string to a UNICODE string
(char const *  a,   // I: ANSI string
 wchar_t *     u)   // O: UNICODE string

{  // StringConvert

   osbool  ok = OSFALSE;   // string can be converted?

   while (CharConvert(*a, u))
      if (*u != L'\0')
      {
         a++;
         u++;
      }
      else
      {
         ok = OSTRUE;
         break;
      }

   return ok;

}  // StringConvert


/**************************************************************************
 * Function: StringConvert                                                *
 **************************************************************************/

osbool                 // string can be converted?
StringConvert          // converts a UNICODE string to an ANSI string
(wchar_t const *  u,   // I: UNICODE string
 char *           a)   // O: ANSI string

{  // StringConvert

   osbool  ok = OSFALSE;   // string can be converted?

   while (CharConvert(*u, a))
      if (*a != '\0')
      {
         u++;
         a++;
      }
      else
      {
         ok = OSTRUE;
         break;
      }

   return ok;

}  // StringConvert


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/basics/convert/convert.cpp .                      *
 **************************************************************************/


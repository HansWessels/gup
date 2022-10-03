
/**************************************************************************
 * File:     osrtl/windows/text/number/number.cpp                         *
 * Contents: Converts string <-> number.                                  *
 * Document: osrtl/windows/text/number/number.d                           *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/text/number/number.cpp  -  Converts string <-> number

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:18  erick
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
// 19970711
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

// Header file of the current module.
#include "osrtl/windows/text/number/number.h"


/**************************************************************************
 * Function: StringLength                                                 *
 **************************************************************************/

unsigned long   // length of string
StringLength    // finds length of string representation of number
(int  value)    // I: number

{  // StringLength

   return StringLength(static_cast<long>(value));

}  // StringLength


/**************************************************************************
 * Function: StringLength                                                 *
 **************************************************************************/

unsigned long           // length of string
StringLength            // finds length of string representation of number
(unsigned int  value)   // I: number

{  // StringLength

   return StringLength(static_cast<unsigned long>(value));

}  // StringLength


/**************************************************************************
 * Function: StringLength                                                 *
 **************************************************************************/

unsigned long   // length of string
StringLength    // finds length of string representation of number
(long  value)   // I: number

{  // StringLength

   unsigned long  length;  // length of the string representation of <value>

   length = 0UL;

   // It is not possible to use StringLength(unsigned long) on -value
   // because, on systems that use two's complement notation, computing
   // -value is not safe.
   if (value < 0L)
      length = 1UL;

   do
   {
      value /= 10L;
      ++length;
   } while (value != 0L);

   return length;

}  // StringLength


/**************************************************************************
 * Function: StringLength                                                 *
 **************************************************************************/

unsigned long            // length of string
StringLength             // finds length of string representation of number
(unsigned long  value)   // I: number

{  // StringLength

   unsigned long  length;  // length of the string representation of <value>

   length = 0UL;

   do
   {
      value /= 10UL;
      ++length;
   } while (value != 0UL);

   return length;

}  // StringLength


/**************************************************************************
 * Function: ValueToString                                                *
 **************************************************************************/

wchar_t *            // resulting string
ValueToString        // converts a number to its string representation
(int        value,   // I: number to convert
 wchar_t *  s)       // O: resulting string

{  // ValueToString

   return ValueToString(static_cast<long>(value), s);

}  // ValueToString


/**************************************************************************
 * Function: ValueToString                                                *
 **************************************************************************/

wchar_t *               // resulting string
ValueToString           // converts a number to its string representation
(unsigned int  value,   // I: number to convert
 wchar_t *     s)       // O: resulting string

{  // ValueToString

   return ValueToString(static_cast<unsigned long>(value), s);

}  // ValueToString


/**************************************************************************
 * Function: ValueToString                                                *
 **************************************************************************/

wchar_t *            // resulting string
ValueToString        // converts a number to its string representation
(long       value,   // I: number to convert
 wchar_t *  s)       // O: resulting string

{  // ValueToString

   wchar_t *      result = s;   // return value
   unsigned long  i;            // element of <s> to define

   i = StringLength(value);

   // On systems with two's complement representation of integral numbers,
   // it is not safe to negate <value>.
   if (value < 0)
   {
      *s++ = L'-';
      --i;
   }

   s[i] = L'\0';

   while (i > 0UL)
   {
      long  digit;   // last digit of <value>

      digit = value % 10L;
      if (digit < 0)
         digit = -digit;
      s[--i] = static_cast<wchar_t>(static_cast<long>(L'0') + digit);
      value /= 10L;
   }

   return result;

}  // ValueToString


/**************************************************************************
 * Function: ValueToString                                                *
 **************************************************************************/

wchar_t *                // resulting string
ValueToString            // converts a number to its string representation
(unsigned long  value,   // I: number to convert
 wchar_t *      s)       // O: resulting string

{  // ValueToString

   unsigned long  i;   // element of <s> to define

   i = StringLength(value);
   s[i] = L'\0';

   while (i > 0UL)
   {
      s[--i] = static_cast<wchar_t>
                  (static_cast<unsigned long>(L'0') + (value % 10UL));
      value /= 10UL;
   }

   return s;

}  // ValueToString


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/text/number/number.cpp .                          *
 **************************************************************************/


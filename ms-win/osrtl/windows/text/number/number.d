
----------------------------------------------------------------------------
---  File:     osrtl/windows/text/number/number.d
---  Contents: Converts string <-> number
---  Module:   <osrtl><text><number>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/text/number/number.d  -  Converts string <-> number
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:18  erick
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
 1.1.000  First issue of this module documentation file.
19970711
19970818
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><text><number>
----------------------------------------------------------------------------

   The module <osrtl><text><number> supplies methods to convert numeric
values to strings.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><text><number>
----------------------------------------------------------------------------

   The module <osrtl><text><number> supplies functions to convert numeric
values to strings.  All these functions can be used directly from any other
part of the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><text><number> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Module <osrtl><text><number>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><text><number>
----------------------------------------------------------------------------

unsigned long             // length of string
StringLength              // finds length of string representation of number
(int  value);             // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(unsigned int  value);    // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(long  value);            // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(unsigned long  value);   // I: number

   Determines the length, in characters, that <value> will have after it is
converted to a string.

----------------------------------------------------------------------------

wchar_t *                // resulting string
ValueToString            // converts a number to its string representation
(int        value,       // I: number to convert
 wchar_t *  s);          // O: resulting string

wchar_t *                // resulting string
ValueToString            // converts a number to its string representation
(unsigned int  value,    // I: number to convert
 wchar_t *     s);       // O: resulting string

wchar_t *                // resulting string
ValueToString            // converts a number to its string representation
(long       value,       // I: number to convert
 wchar_t *  s);          // O: resulting string

wchar_t *                // resulting string
ValueToString            // converts a number to its string representation
(unsigned long  value,   // I: number to convert
 wchar_t *      s);      // O: resulting string

   Converts <value> to a string.  Stores the string in <s>.  Returns <s>.
<s> will be terminated with static_cast<wchar_t>(L'\0') .
   This function does not check if <s> is large enough to hold all
characters of <value>.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/text/number/number.d .
----------------------------------------------------------------------------


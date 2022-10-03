
----------------------------------------------------------------------------
---  File:     osrtl/windows/basics/convert/convert.d
---  Contents: Type conversions.
---  Module:   <osrtl><basics><convert>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/basics/convert/convert.d  -  Type conversions
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:04  erick
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
19970714
19970803
EAS
----------------------------------------------------------------------------
 1.1.001  A minor typographical change was made to the argument list of
19970822  <RoundFloatToIntegral> in chapter 4.
19970822
EAS
----------------------------------------------------------------------------
 1.2.000  Identical to revision 1.1.001.  Formalised revision.
19970825
19970825
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><basics><convert>
----------------------------------------------------------------------------

   The module <osrtl><basics><convert> supplies functions that convert a
value from one data type or representation to another.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><basics><convert>
----------------------------------------------------------------------------

   All other parts of the project can directly use the conversion functions
that are provided by <osrtl><basics><convert>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><basics><convert> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Module <osrtl><basics><type>.
         The conversion functions indicate success or failure through a
         return value of type <osbool>.

      Module <osrtl><basics><convert>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><basics><convert>
----------------------------------------------------------------------------

osbool                  // value can be converted?
RoundFloatToIntegral    // converts a <float> to an <int>, rounding it
(float  f,              // I: value to convert
 int *  i);             // O: converted value

osbool                  // value can be converted?
RoundFloatToIntegral    // converts a <float> to a <long>, rounding it
(float   f,             // I: value to convert
 long *  i);            // O: converted value

   <RoundFloatToIntegral> converts a floating-point value to an integral
type.  The floating-point value is rounded mathematically:

       6.499 -->  6
       6.500 -->  7
       6.501 -->  7
      -6.499 --> -6
      -6.500 --> -6   (Yes, that's mathematical rounding!)
      -6.501 --> -7

The return value indicates if the conversion could be done:

      OSTRUE:  The conversion has been performed successfully.  *i has been
               defined.
      OSFALSE: It was not possible to perform the conversion because <f> was
               too large (for positive values) or too small (for negative
               values) to be converted to the integral representation.  *i
               has not been changed.

----------------------------------------------------------------------------

osbool            // character can be converted?
CharConvert       // converts an ANSI character to a UNICODE character
(char       a,    // I: ANSI character
 wchar_t *  u);   // O: UNICODE character

osbool            // character can be converted?
CharConvert       // converts a UNICODE character to an ANSI character
(wchar_t  u,      // I: UNICODE character
 char *   a);     // O: ANSI character

   <CharConvert> converts an ANSI character to its UNICODE equivalent, or
vice versa.  The return value indicates if the conversion could be done:

      OSTRUE:  The conversion has been performed successfully.  The output
               argument has been defined.
      OSFALSE: It was not possible to perform the conversion because the
               target character set does not contain a character that is
               equivalent to the input character.  The output argument has
               not been changed.

----------------------------------------------------------------------------

osbool                  // string can be converted?
StringConvert           // converts an ANSI string to a UNICODE string
(char const *  a,       // I: ANSI string
 wchar_t *     u);      // O: UNICODE string

osbool                  // string can be converted?
StringConvert           // converts a UNICODE string to an ANSI string
(wchar_t const *  u,    // I: UNICODE string
 char *           a);   // O: ANSI string

   <StringConvert> converts an ANSI string to its UNICODE equivalent, or
vice versa.  It assumes that the input string is null-terminated, and that
the output string is large enough to hold the converted input string.  The
output string will be null-terminated too.
   The return value indicates if the conversion could be done:

      OSTRUE:  The conversion has been performed successfully.  All
               characters of the input string, including the terminating
               null character, have been converted and stored in the output
               string.
      OSFALSE: It was not possible to perform the conversion because the
               input string contains one or more characters that do not have
               an equivalent in the output character set.  All characters of
               the input string up to but not including the first character
               that could not been converted, have been converted and stored
               in the output string.  The output string is, however, not
               null-terminated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/basics/convert/convert.d .
----------------------------------------------------------------------------


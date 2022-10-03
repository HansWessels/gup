
----------------------------------------------------------------------------
---  File:     osrtl/windows/text/char/char.d
---  Contents: Characters.
---  Module:   <osrtl><text><char>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/text/char/char.d  -  Characters
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:15  erick
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
19970723
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><text><char>
----------------------------------------------------------------------------

   The module <osrtl><text><char> supplies methods to handle character data.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><text><char>
----------------------------------------------------------------------------

   <osrtl><text><char> supplies a bunch of functions that handle character
data.  These functions are independent.  All these functions can be used
directly by any part of the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><text><char> depends on the following units, components
and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <char> uses basic types, such as <osbool>.

      Module <osrtl><text><char>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><text><char>
----------------------------------------------------------------------------

osbool             // whether the character is whitespace
IsCharWhiteSpace   // checks if a character is whitespace
(wchar_t  ch);     // I: character to check

   Checks if <ch> is a whitespace character.  This function is the platform-
independent version of <isspace>.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/text/char/char.d .
----------------------------------------------------------------------------


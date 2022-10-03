
----------------------------------------------------------------------------
---  File:      osrtl/windows/text/text.d
---  Contents:  Textual data.
---  Component: <osrtl><text>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/text/text.d  -  Textual data
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:13  erick
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
Chapter 1  -  Purpose of the component <osrtl><text>
----------------------------------------------------------------------------

   The component <osrtl><text> supplies everything that is needed to handle
textual data, that is, characters and text strings, and to convert numeric
values to text strings.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><text>
----------------------------------------------------------------------------

   The component <osrtl><text> consists of the modules <char>, <string>, and
<number>.  It only joins these modules.  Therefore, refer to the document-
ation files for these modules for a description of how to use the component
<osrtl><text>.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of component <osrtl><text>
----------------------------------------------------------------------------

   Because <osrtl><text> is nothing but an aggregate of the modules <char>,
<string>, and <number>, refer to the documentation files for these modules
for a detailed explanation of the component <osrtl><text>.
   To give an overview of the component <osrtl><text>, here follows a list
of all items provided by the modules that make up <osrtl><text>.

Function: IsCharWhiteSpace
Use:      The platform-independent equivalent of <isspace>.

Function: StringLength
Use:      If used with a string value: The platform-independent equivalent
             of <strlen>.
          If used with a numeric value: Determines the length, in
             characters, that the numeric value will have after it is
             converted to a string.

Function: StringCopy
Use:      The platform-independent equivalent of <strcpy>.

Function: StringDuplicate
Use:      Allocates a copy of a source string on the heap, and copies the
          source string to the copy.  If necessary, converts the string
          from ANSI to UNICODE or vice versa while copying.

Function: StringEqual
Use:      Checks if two strings are equal, or if they are both
          static_cast<wchar_t const *>(OSNULL) .

Function: StringDifferent
Use:      The inverse of <StringEqual>.

Function: ValueToString
Use:      Converts a numeric value to a string.


----------------------------------------------------------------------------
---  End of file osrtl/windows/text/text.d .
----------------------------------------------------------------------------


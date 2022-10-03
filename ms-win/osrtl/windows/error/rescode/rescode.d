
----------------------------------------------------------------------------
---  File:     osrtl/windows/error/rescode/rescode.d
---  Contents: Result codes.
---  Module:   <osrtl><error><rescode>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/error/rescode/rescode.d  -  Result codes
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:20  erick
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
19970629
19970807
EAS
----------------------------------------------------------------------------
 1.1.001  This documentation file has been brought in line with the new
19970822  integer values of the result codes that were introduced in
19970822  revision 1.1.001 of osrtl/windows/error/rescode/rescode.h , and
EAS       with the new marker values that were introduced in that file.
----------------------------------------------------------------------------
 1.2.000  Identical to revision 1.1.001.  Formalised revision.
19970826
19970826
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><error><rescode>
----------------------------------------------------------------------------

   The module <osrtl><error><rescode> supplies a type for result codes.
This type may be used to indicate the result of an operation.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><error><rescode>
----------------------------------------------------------------------------

   All other parts of the project can directly use the type <resultcode>
that is provided by <osrtl><error><rescode>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><error><rescode> does not depend on any other part of
the program.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><error><rescode>
----------------------------------------------------------------------------

Type: osresult

   <osresult> is a result code that may be used to indicate the result of
an operation.  A variable of this data type may have any of the following
values:

   OSRESULT_OK
      No problem encountered.

   OSERROR_VALUEOUTOFRANGE
      A numeric value was out of range.

   OSERROR_NOCHARCONVERT
      A character could not be converted from ANSI to UNICODE or vice versa.

   OSERROR_NOMEMORY
      There was not enough free memory available to complete the operation.

   OSERROR_NORSRC_MCURSOR
      The system does not have enough resources available to select a mouse
      cursor.

   OSERROR_NORSRC_FONT
      The system does not have enough resources available to create a font.

   OSERROR_NORSRC_WINDOW
      The system does not have enough resources available to create a
      window.

   OSERROR_NORSRC_DRAW
      The system does not have enough resources available to draw to a
      window.

   OSERROR_NORSRC_WRITE
      The system does not have enough resources available to write text to a
      window.

   OSERROR_NODEFAULTFONT
      None of the predefined default fonts is present in the system.

   OSINTERNAL_MUTEXADMIN
      The program contains an internal error in the part that handles the
      administration of mutex semaphores.

   OSINTERNAL_HEAPADMIN
      The program contains an internal error in the part that handles the
      administration of the heap.

   OSINTERNAL_MOUSECURSOR
      The program contains an internal error in the part that selects a new
      mouse cursor.

   OSINTERNAL_FONTADMIN
      The program contains an internal error in the part that handles the
      administration of fonts.

   OSINTERNAL_WINDOWADMIN
      The program contains an internal error in the part that handles the
      administration of windows.

   OSINTERNAL_DRAW
      The program contains an internal error in the part that handles
      drawing to a window.

   OSINTERNAL_WRITE
      The program contains an internal error in the part that handles
      writing text to a window.

Apart from these values, marker values have been defined to mark the first
and last values of a category of values:

   OSERROR_FIRST
   OSERROR_LAST
      The first and the last error code, respectively.

   OSINTERNAL_FIRST
   OSINTERNAL_LAST
      The first and the last internal error code, respectively.

   OSRESULT_LAST
      The last result code.

Each of the result codes has, of course, a corresponding integer value.
<rescode> guarantees that, in all future revisions of this modules, the
integer values of the existing result codes will not change.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/error/rescode/rescode.d .
----------------------------------------------------------------------------


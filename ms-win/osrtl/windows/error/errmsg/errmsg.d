
----------------------------------------------------------------------------
---  File:     osrtl/windows/error/errmsg/errmsg.d
---  Contents: Error messages.
---  Module:   <osrtl><error><errmsg>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/error/errmsg/errmsg.d  -  Error messages
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:17  erick
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
19970717
19970724
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><error><errmsg>
----------------------------------------------------------------------------

   The module <osrtl><error><errmsg> supplies the possibility to display
simple (error) messages on the screen.  It is primarily intended as an aid
in the debugging phase of program development.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><error><errmsg>
----------------------------------------------------------------------------

   All other parts of the project can directly use the functions that are
provided by <errmsg>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><error><errmsg> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         This module provides the basic constants (for example, <OSNULL>)
         that <errmsg> needs.

      Module <osrtl><error><errmsg>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><error><errmsg>
----------------------------------------------------------------------------

inline
void
OsErrorMessage                // displays an error message
(wchar_t const *  message,    // I: error message
 wchar_t const *  title);     // I: title of the message box

inline
void
OsErrorMessage                // displays an error message
(wchar_t const *  message1,   // I: error message, part 1
 wchar_t const *  message2,   // I: error message, part 2
 wchar_t const *  title);     // I: title of the message box

void
OsErrorMessage                // displays an error message
(wchar_t const *  message1,   // I: error message, part 1
 wchar_t const *  message2,   // I: error message, part 2
 wchar_t const *  message3,   // I: error message, part 3
 wchar_t const *  title);     // I: title of the message box

   Displays a simple error message.  The message consists of one, two, or
three strings which are concatenated (without inserting additional line
breaks, spaces, or other characters).  The message is displayed in a simple
message box, the title of which is <title>.
   These functions display an error message in such a way that the
probability of the operating system being able to actually display the
message is maximised, even if the system is out of resources, or if any
other non-nominal condition occurs or has occurred.  Therefore, the
messages are displayed in a very simple way.  The total number of characters
in the message and in the title is limited, the total number of lines in the
message is limited, and the caller must insert line breaks were needed.  If
the message is too long, it will be truncated.  If the message consists of
too many lines, excess lines will not be displayed.
   This unsophisticated method to display messages is intended for use in
the following situations:

      1. For tracing and debugging in debugging mode.  In debugging mode,
         the beauty of the display is not important.
      2. As a back-up for the display of an ordinary message, in case the
         message cannot be displayed using the normal, sophisticated method
         because the system is in a non-nominal situation.

----------------------------------------------------------------------------

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message);   // I: error message
#else
   (wchar_t const *  );          // -: error message
#endif

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2);  // I: error message, part 2
#else
   (wchar_t const *  ,           // -: error message, part 1
    wchar_t const *  );          // -: error message, part 2
#endif

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2,   // I: error message, part 2
    wchar_t const *  message3);  // I: error message, part 3
#else
   (wchar_t const *  ,           // -: error message, part 1
    wchar_t const *  ,           // -: error message, part 2
    wchar_t const *  );          // -: error message, part 3
#endif

   <OsDebugErrorMessage> is functional in debugging mode only.  In release
mode, it translates to an empty statement.
   In debugging mode, <OsDebugErrorMessage> displays a simple error message.
The message consists of one, two, or three strings which are concatenated
(without inserting additional line breaks, spaces, or other characters).
The message is displayed in a simple message box, the title of which is
always L"Debug error" .
   In debugging mode, <OsDebugErrorMessage> translates into a call to
<OsErrorMessage>, which is described above.  Refer to the description of
<OsErrorMessage> for details.

----------------------------------------------------------------------------

static
void
AddMessage                // adds a message to a message string
(wchar_t * *      dst,    // O: destination message string
 wchar_t const *  src,    // I: message to add
 int              max);   // I: maximum number of characters to add

   Used by <OsErrorMessage> to construct the message that must be displayed
from the parts of the message that it was given.  Copies at most <max>
characters from <src> to *dst .  Does not copy the terminating L'\0' ;
does not add a L'\0' to *dst .  <OsErrorMessage> does nothing if
src==static_cast<wchar_t const *>(OSNULL) .

----------------------------------------------------------------------------

static
void
Convert
(wchar_t const *  src,          // I: source string
 char *           dst,          // O: destination string
 int              maxlength);   // I: maximum length of destination string

   Exists only if the OS does not use UNICODE.  Converts the UNICODE string
<src> to the ANSI string <dst>, taking the maximum length <maxlength> into
account.  Terminates <dst> with a null character.  Does not check if
src==static_cast<wchar_t const *>(OSNULL) .


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   In Windows, calling <MessageBox> with the flags set to
(MB_SYSTEMMODAL | MB_ICONHAND) ensures that Windows will display the message
box regardless of available memory and other resources.  Windows achieves
this by using an extremely simple type of message box.  The draw-back of
using such a simple type of message box is that the possibilities for
display are limited as described under <OsErrorMessage>.


----------------------------------------------------------------------------
---  End of file osrtl/windows/error/errmsg/errmsg.d .
----------------------------------------------------------------------------


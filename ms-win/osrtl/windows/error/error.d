
----------------------------------------------------------------------------
---  File:      osrtl/windows/error/error.d
---  Contents:  Error handling.
---  Component: <osrtl><error>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/error/error.d  -  Error handling
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:15  erick
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
Chapter 1  -  Purpose of the component <osrtl><error>
----------------------------------------------------------------------------

   The component <osrtl><error> handles errors that are encountered while
executing the program.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><error>
----------------------------------------------------------------------------

   The component <osrtl><error> consists of the modules <rescode> and
<errmsg>.  It only exports these modules.  Therefore, refer to the
documentation files for the modules <osrtl><error><rescode> and
<osrtl><error><errmsg> for a description of how to use the component
<osrtl><error>.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of component <osrtl><error>
----------------------------------------------------------------------------

   Because <osrtl><error> is nothing but a higher-level view of the modules
<osrtl><error><rescode> and <osrtl><error><errmsg>, refer to the document-
ation files for these modules for a detailed explanation of the component
<osrtl><error>.
   To give an overview of the component <osrtl><error>, here follows a
list of all items provided by <osrtl><error><rescode> and
<osrtl><error><errmsg> that are exported to the rest of the program.

Type:     osresult
Use:      Indicates the result of an operation.  Possible values are

             OK code       Errors                    Internal errors
             -------------------------------------------------------------
             OSRESULT_OK   OSERROR_VALUEOUTOFRANGE   OSINTERNAL_MUTEXADMIN
                           OSERROR_NOCHARCONVERT     OSINTERNAL_HEAPADMIN
                           OSERROR_NOMEMORY          OSINTERNAL_MOUSECURSOR
                           OSERROR_NORSRC_MCURSOR    OSINTERNAL_FONTADMIN
                           OSERROR_NORSRC_FONT       OSINTERNAL_WINDOWADMIN
                           OSERROR_NORSRC_WINDOW     OSINTERNAL_DRAW
                           OSERROR_NORSRC_DRAW       OSINTERNAL_WRITE
                           OSERROR_NORSRC_WRITE
                           OSERROR_NODEFAULTFONT

          The integer values that correspond to these result codes will not
          change in future revisions of <osrtl><error>.
          In addition to these values, so-called marker values have been
          defined.  These marker values mark the begin and end of a category
          of result codes.

             Category          Begin marker       End marker
             ----------------------------------------------------
             Errors            OSERROR_FIRST      OSERROR_LAST
             Internal errors   OSINTERNAL_FIRST   OSINTERNAL_LAST
             Result codes      (none)             OSRESULT_LAST

Function: OsErrorMessage
Use:      Displays a very simple, unsophisticated, even ugly, error message.
          The system is almost always able to display the message.  Refer
          to osrtl/windows/error/errmsg/errmsg.d for restrictions on the
          message text.

Function: OsDebugErrorMessage
Use:      In debugging mode, translates to a call of <OsErrorMessage> with
          a title of L"Debug error" .  In release mode, translates to an
          empty statement. 


----------------------------------------------------------------------------
---  End of file osrtl/windows/error/error.d .
----------------------------------------------------------------------------


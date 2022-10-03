
----------------------------------------------------------------------------
---  File:      osrtl/windows/startup/startup.d
---  Contents:  Start-up.
---  Component: <osrtl><startup>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/startup/startup.d  -  Start-up
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:56  erick
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
Chapter 1  -  Purpose of the component <osrtl><startup>
----------------------------------------------------------------------------

   The component <osrtl><startup> handles the start-up process of the
program.  It contains the function that is "called by the operating system"
("main"), and it calls the entry point of the user code of the program
(<ProgramStart>).


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><startup>
----------------------------------------------------------------------------

   The component <osrtl><startup> consists of the module <startup>.  It
exports only this module.
   User code must supply two entry points for <osrtl><startup> to call:

      - <ApplicationName>.  This function must return the name of the
        application.  <osrtl><startup> will make a copy of this name,
        so the user code need not preserve the application name.
        <ApplicationName> is called exactly once, before <ProgramStart> is
        called.
      - <ProgramStart>.  This function must execute the program.  It must
        return a status code; <startup> will pass the status code to the
        operating system on program termination.
        <ProgramStart> is called exactly once, after <ApplicationName> has
        been called.

   Refer to the documentation file for the module <osrtl><startup><startup>
for a more detailed description of how to use the component
<osrtl><startup>.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of component <osrtl><startup>
----------------------------------------------------------------------------

   Because <osrtl><startup> is nothing but a higher-level view of the module
<osrtl><startup><startup>, refer to the documentation file for that module
for a detailed explanation of the component <osrtl><startup>.
   To give an overview of the component <osrtl><startup>, here follows a
list of all items provided by <osrtl><startup><startup> that are exported
to the rest of the program.

Type:     programinstance
Use:      Holds the instance of the program.  This is an operating-system
          specific item; therefore, it must be used by <osrtl> only.

Function: ProgramName
Use:      Returns the name of the program.  This is a copy of the name that
          was supplied by the user-defined function <ApplicationName>.

Function: ProgramInstance
Use:      Returns the instance handle of the current instance of the
          application.  This is an operating-system specific item;
          therefore, this function must be called from <osrtl> only.


----------------------------------------------------------------------------
---  End of file osrtl/windows/startup/startup.d .
----------------------------------------------------------------------------


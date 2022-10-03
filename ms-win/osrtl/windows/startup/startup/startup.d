
----------------------------------------------------------------------------
---  File:     osrtl/windows/startup/startup/startup.d
---  Contents: Program start-up.
---  Module:   <osrtl><startup><startup>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/startup/startup/startup.d  -  Program start-up
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:59  erick
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
19970727
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><startup><startup>
----------------------------------------------------------------------------

   The module <osrtl><startup><startup> handles the start-up process of the
program.  It contains the function that is "called by the operating system"
("main"), and it calls entry points of the user code of the program.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><startup><startup>
----------------------------------------------------------------------------

   The start-up process is handled transparently; the user code just has to
provide three so-called entry points, which are the functions
<ApplicationName>, <ApplicationInitialise>, and <ApplicationCloseDown>.
   <osrtl><startup><startup> will do the following things in the indicated
order:

   1. It calls <ApplicationName> to obtain the name of the application.
      The user code _must_ supply <ApplicationName>.
   2. It calls <ApplicationInitialise> to allow the application to
      initialise.
      The user code _must_ supply <ApplicationInitialise>.
   3. It allows the program to run by driving the message engine that
      sends messages from the system to the application's windows.
   4. It calls <ApplicationCloseDown> to allow the application to close
      down.
      The user code _must_ supply <ApplicationCloseDown>.

   The operating system passed some platform-specific information to
<osrtl><startup><startup>.  This information is used only by the unit
<osrtl> because it is platform-specific.  To obtain this information,
a module of <osrtl> must call the appropriate function.

   - <ProgramInstance> returns the instance handle of the current instance
     of the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><startup><startup> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <startup> uses basic items, like <OSNULL>.

      Component <osrtl><error>.
         <startup> uses <OsErrorMessage> and <OsDebugErrorMessage> if it
         encounters a problem.

      Module <osrtl><startup><startup>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><startup><startup>
----------------------------------------------------------------------------

typedef  void *  programinstance;   // program instance

   Other parts of <osrtl> need the program instance.  Therefore, <startup>
must export a type for the program instance.  The consequence is that other
units import this type.
   The type of the program instance really is <HINSTANCE>.  This type, how-
ever, is OS-specific.  To prevent other units from importing an OS-specific
type, an OS-independent synonym for that type is created.

----------------------------------------------------------------------------

wchar_t const *   // name of the program
ProgramName       // obtains the name of the program
();

   Obtains the name of the program.  This is a copy of the name that was
returned to <startup> by the user-supplied function <ApplicationName>.

----------------------------------------------------------------------------

programinstance   // program instance
ProgramInstance   // obtains the instance of the application
();

   Obtains the instance of the current program.  Intended for use by other
parts of <osrtl> only.

----------------------------------------------------------------------------

void
WindowExists      // tells <osrtl><startup><startup> whether windows exist
(osbool  exist);  // I: whether application windows exist

   Tells <osrtl><startup><startup> whether at least one application-created
window exists.  Intended for use by other parts of <osrtl> only.  This
function _must_ be used to keep <osrtl><startup><startup> informed about the
actual status of the system.

----------------------------------------------------------------------------

int                     // status code
PASCAL
WinMain                 // main function; program entry point
(HINSTANCE  instance,   // I: current instance of application
 HINSTANCE  ,           // -: previous instance of application
 LPSTR      ,           // -: command line
 int        )           // -: display mode for main window

   The program entry point.  This is the function that receives control when
the program is started.  It is exported because one of the system libraries
must link to it.  To the rest of the program, the existence of this function
is transparent.
   <WinMain> calls the user-supplied function <ApplicationName> to obtain
the name of the application.  It makes a local copy of this name, which can
be retrieved through <ProgramName>.  Then, <WinMain> calls
<ApplicationInitialise> to allow the user code to initialise.  If the
initialisation is successful, <WinMain> starts the main message loop that
retrieves messages from the system and sends them to the appropriate windows
of the application.  When the message loop terminates, <WinMain> calls
<ApplicationCloseDown> to allow the user code to close down.
   <WinMain> passes the first non-zero return code which it receives to the
operating system.  If all return codes are zero, <WinMain> passes zero to
the operating system.
   <WinMain> must not be called from within the program.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   Because functions like <ProgramInstance> must be callable from all other
components and modules of <osrtl>, <osrtl><startup><startup> must be as low
as possible in the hierarchy of components and modules.  Yet, its task is to
call the entry points of the user code, which are at the very top of the
hierarchy of units, components and modules.  Therefore, the entry point of
the user code is declared locally to be an external function.  The same
argument applies to the user-supplied function that returns the name of the
application.

   All functions of <osrtl><startup><startup> are platform-specific.  If
this module is to be ported to a different environment, all functions must
be deleted, and a completely new set of functions must be written.  The set
of functions must fulfill the following requirements:

   1. Obtain control of execution from the operating system.
   2. Call the entry point <ApplicationName> of the user code.
   3. Make a copy of the application name that <ApplicationName> returned.
      This allows <ApplicationName> to return a name with a limited
      lifetime.
   4. Call the entry point <ApplicationInitialise> of the user code.
   5. Handle the return code that <ApplicationInitialise> gives.
   6. Make sure that messages are sent from the system to the application.
   7. When the program must terminate, call the entry point
      <ApplicationCloseDown> of the user code.
   8. Handle the return code that <ApplicationCloseDown> gives.
   9. Provide platform-specific information to other components of <osrtl>.


----------------------------------------------------------------------------
---  End of file osrtl/windows/startup/startup/startup.d .
----------------------------------------------------------------------------


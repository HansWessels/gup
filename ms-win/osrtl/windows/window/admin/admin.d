
----------------------------------------------------------------------------
---  File:     osrtl/windows/window/admin/admin.d
---  Contents: Window administration.
---  Module:   <osrtl><window><admin>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/window/admin/admin.d  -  Window administration
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:30  erick
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
19970819
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><window><admin>
----------------------------------------------------------------------------

   The module <osrtl><window><admin> takes care of the administration needed
to allow Windows to create windows.  It is intended to be used solely by the
component <osrtl><window>; other parts of the system are not supposed to
deal with such administrative chores.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><window><admin>
----------------------------------------------------------------------------

   All functions of the module <osrtl><window><admin> can be used directly
by all other parts of the program.  Refer, however, to chapter 1, which
states that this module is not intended to be used by modules that are not
part of the component <osrtl><window>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><window><admin> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <admin> uses basic entities, such as <osobject>.

      Component <osrtl><error>.
         <admin> uses entities of error handling, such as <osresult> and
         <OsDebugErrorMessage>.

      Component <osrtl><startup>.
         <admin> needs the program instance, which is provided by the
         function <ProgramInstance> in <osrtl><startup>.

      Component <osrtl><tasking>.
         <admin> uses mutexes to guard its operations.

      Component <osrtl><heap>.
         <admin> uses the heap to temporarily allocate some intermediate
         storage.

      Component <osrtl><text>.
         <admin> uses <StringDuplicate> to duplicate strings.

      Module <osrtl><window><admin>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><window><admin>
----------------------------------------------------------------------------

class windowadmin:   // window administrator
   public osobject   //    generic object
{
   //...
};  // windowadmin

   This class handles the administration that is necessary to let Windows
create windows when they are needed.  Refer to the descriptions of the
components of this class, below, for an explanation of the administration
that is necessary.

----------------------------------------------------------------------------

typedef
wchar_t const *   // ID of a window class
windowclassid;

   Using <windowclassid> instead of <wchar_t const *> serves two purposes:

      1. It makes the code clearer, because it is clear what the string
         means.
      2. It allows <admin> to change the representation of a window class ID
         without having to modify the code that uses <admin>.

----------------------------------------------------------------------------

typedef
void *        // window procedure
windowproc;

   A <windowproc> really is a <WNDPROC>.  <WNDPROC>, however, is platform-
specific, so it must not proliferate to other units.  Therefore, <WNDPROC>
must not be used in header files.  To allow the user of <admin> to pass a
<WNDPROC>, a <typedef> to an innocent type is necessary.

----------------------------------------------------------------------------

windowadmin::windowadmin   // initialises a <windowadmin>
();

   Initialises a <windowadmin> by setting its name to L"<windowadmin>" .
This facilitates debugging.

----------------------------------------------------------------------------

static
osresult                     // result code
windowadmin::Create          // administers creation of a window
(windowproc       Handler,   // I: window procedure
 windowclassid *  id);       // O: ID of the window class

   Administers creation of a window.  Whenever a window must be created,
<Create> must be called before creating the window.  <Create> supplies an
ID that must be given to Windows in order to allow Windows to create the
window.
   <Create> counts the number of existing windows.  If this number is zero,
the window that the caller wishes to create is the first window that this
application creates.  <Create> will register a window class ("class" is to
be understood in Windows terms, not in C++ terms) for the window.  It will
put the ID of the window class in *id .
   If the number of existing windows is not zero, the window class has
already been registered with Windows.  <Create> then simply returns the
ID of the existing window class to the caller.
   The caller must use <Destroy> to administer destruction of a window.
Refer to the description of <Destroy>, below.

----------------------------------------------------------------------------

static
osresult               // result code
windowadmin::Destroy   // administers destruction of a window
();

   Administers destruction of a window.  Whenever a window is destroyed,
<Destroy> must be called after destroying the window.
   <Destroy> counts the number of existing windows.  If this number becomes
zero, the window that the caller has just deleted was the last window that
this application destroyed.  In principle, <Destroy> could unregister the
window class ("class" is to be understood in Windows terms, not in C++
terms) for the window in such a case.  However, <window> calls <Destroy>
just before the system actually destroys the window.  This means that
<Destroy> must wait until the window is completely destroyed before it can
unregister the class.  Such waiting not being easily implemented in Windows,
unregistering the window class is deferred to the time when <admin> is
closed down.  Then, <guard> will handle the unregistration.  Refer to the
documentation for guard_osrtl_window_admin::~guard_osrtl_window_admin for
an explanation of the unregistration process.
   The caller must use <Create> to administer creation of a window.  Refer
to the description of <Create>, above.

----------------------------------------------------------------------------

static
inline
unsigned int           // number of existing windows
windowadmin::Windows   // obtains the number of existing windows
();

   Obtains the number of currently existing windows.  This number of windows
is maintained by <Create> and <Destroy>.
   <Windows> is intended to be used by <guard_osrtl_window_admin> only.
Refer to the description of <guard_osrtl_window_admin>, below.

----------------------------------------------------------------------------

static
inline
osbool              // has a window ever existed
windowadmin::Ever   // checks if a window ever has existed
();

   Checks if a window has ever existed during the run-time of this program.
This status is maintained by <Create> and <Destroy>.
   <Ever> is intended to be used by <guard_osrtl_window_admin> only.  Refer
to the description of <guard_osrtl_window_admin>, below.

----------------------------------------------------------------------------

static
inline
windowclassid const      // name of the window class
windowadmin::ClassName   // obtains the name of the window class
();

   Obtains the name of the window class.
   <ClassName> is intended to be used by <guard_osrtl_window_admin> only.
Refer to the description of <guard_osrtl_window_admin>, below.

----------------------------------------------------------------------------

static
unsigned int           // number of existing windows
windowadmin::windows
= 0;

   Contains the number of currently existing windows.  This window count is
maintained by <Create> and <Destroy>.  It can be obtained through <Windows>.

----------------------------------------------------------------------------

static
osbool              // has a window ever existed?
windowadmin::ever
= OSFALSE;

   Keeps record of the history.  This member is <OSFALSE> as long as not a
single window has ever been created in the current session of the program.
As soon as a window is created, this member becomes <OSTRUE>.  It is then
never reset to <OSFALSE>.
   This member can be obtained through <Ever>.

----------------------------------------------------------------------------

static
windowclassid const      // name of the window class
windowadmin::classname
= modulename;

   The name of the window class ("class" is to be understood in Windows
terms, not in C++ terms).  This class name is used by <Create> and <Destroy>
to register and unregister the window class with Windows.  It must be a
string that is unique throughout the program, and that is not used by other
programs.  These requirements are very likely to be met by setting the class
name to the full name of the module <osrtl><window><admin>.

----------------------------------------------------------------------------

const  wchar_t const * const  modulename  =  L"OSRTL_WINDOW_ADMIN";

   The full name of this module.  This string can be assumed to be unique
throughout the program.  It is used to give windowadmin::classname a unique
value.  Refer to windowadmin::classname , above.

----------------------------------------------------------------------------

class guard_osrtl_window_admin:   // guards this module
   private windowadmin            //    window administrator
{
public:
   //...
};  // guard_osrtl_window_admin

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

guard_osrtl_window_admin::~guard_osrtl_window_admin  // performs exit checks
();                                                  //    on the module

   This destructor checks the number of existing windows.  In debugging
mode, if not all windows have been destroyed, it warns the developer about
this anomaly.  Both in debugging mode and in release mode, if not all
windows have been destroyed, it attempts to mark the windows as having been
destroyed, in an attempt to allow <Destroy> to unregister the window class
with Windows.
   Both in debugging mode and in release mode, this destructor performs the
nominal action of checking if the number of windows is zero (the expected
situation) and if a window has ever existed during the life of the program.
If these conditions are met, the window class must have been registered with
Windows, but it has not yet been unregistered (refer to <Destroy> for
related details).  Therefore, the class is unregistered now.

----------------------------------------------------------------------------

static
guard_osrtl_window_admin   // guards this module
guard;

   Instantiates a global <guard_osrtl_window_admin>.  This ensures that the
destructor of <guard_osrtl_window_admin> is activated when the module
<admin> is closed down.  Refer to the description of that destructor for
details on the checks that are performed when <admin> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/window/admin/admin.d .
----------------------------------------------------------------------------


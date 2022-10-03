
----------------------------------------------------------------------------
---  File:     osrtl/windows/window/safewin/safewin.d
---  Contents: Generic window.
---  Module:   <osrtl><window><safewin>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/window/safewin/safewin.d  -  Generic window
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:34  erick
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
19970720
19970819
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><window><safewin>
----------------------------------------------------------------------------

   The module <osrtl><window><safewin> allows to create, manage, destroy,
and work with windows.  It provides all services that are needed to manage
windows on the screen.  It manages all interactions between the user code
and the operating system, including the messages that the operating system
sends to the window.
   In addition, <osrtl><window><safewin> provides safety features that
detect any illegal use of its functionality.  These safety features guard
the integrity of the entire program.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><window><safewin>
----------------------------------------------------------------------------

   All other parts of the program may freely use the services that are
provided by <osrtl><window><safewin>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><window><safewin> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <osrtl><window><safewin> depends on <osrtl><window><window>, which
         in turn depends on <osrtl><basics>.

      Component <osrtl><error>.
         <osrtl><window><safewin> uses <osresult>s to inform other functions
         about the result of its actions.

      Component <osrtl><tasking>.
         <osrtl><window><safewin> uses semaphores to protect its critical
         code sections from simultaneous access by several processes.

      Component <osrtl><heap>.
         <osrtl><window><safewin> allocates information on the heap.

      Component <osrtl><drawing>.
         <osrtl><window><safewin> depends on <osrtl><window><window>, which
         in turn depends on <osrtl><drawing>.

      Module <osrtl><window><winpaint>.
         <osrtl><window><safewin> depends on <osrtl><window><window>, which
         in turn depends on <osrtl><window><winpaint>.

      Module <osrtl><window><winfont>.
         <osrtl><window><safewin> depends on <osrtl><window><window>, which
         in turn depends on <osrtl><window><winfont>.

      Module <osrtl><window><window>.
         An <oswindow> is derived from an <osbasicwindow>.

      Module <osrtl><window><safewin>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><window><safewin>
----------------------------------------------------------------------------


class oswindow:           // generic window
   public osbasicwindow   //    basic window
{
   //...
};  // oswindow

   This class implements a window.  It supplies all the actions and all
information that are necessary to manage a window, including the interface
to the operating system.
   Refer to the descriptions of the components of <oswindow> for an
explanation of the functionality of this class.

----------------------------------------------------------------------------

oswindow::oswindow   // initialises an <oswindow>
();

   Initialises the internal administration of the <oswindow>.

----------------------------------------------------------------------------

virtual
oswindow::~oswindow   // closes down an <oswindow>
();

   Closes down the <oswindow>.  If the <oswindow> hasn't yet been properly
destroyed, this destructor destroys the <oswindow>.  In debugging mode, it
warns the developer about the fact that the <oswindow> hasn't yet been
properly destroyed.

----------------------------------------------------------------------------

virtual
osresult                    // result code
oswindow::Create            // creates an <oswindow>
(float            left,     // I: x co-ordinate of left   edge
 float            right,    // I: x co-ordinate of right  edge
 float            top,      // I: y co-ordinate of top    edge
 float            bottom,   // I: y co-ordinate of bottom edge
 wchar_t const *  name,     // I: name of the window
 osbool           vis);     // I: must the window be visible?

   Creates an <oswindow>.  Functionally, oswindow::Create just calls
osbasicwindow::Create to create the window.  In reality, oswindow::Create
also stores a pointer to the newly created window in the list of all
currently existing windows <windowlist>.  This list is used internally by
<safewin>.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowDestruction   // the window is being destroyed
(long   time,                 // I: time of window destruction
 float  x,                    // I: x position of the mouse, WA
 float  y);                   // I: y position of the mouse, WA

   Administers the window as having been destroyed.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowActivation   // the window is being activated
(oswindow *  ,               // I: window that is being de-activated
 long        ,               // I: time of activation
 float       ,               // I: x position of the mouse, WA
 float       );              // I: y position of the mouse, WA

   Informs the window that it is being activated.  If the window that is
being de-activated is a window of this program, the first parameter
is the window that is being de-activated.  If the window that is being
de-activated belongs to a different application, the first parameter is
static_cast<oswindow *>(OSNULL) .
   This function is called by the second version of
oswindow::WindowActivation , which overrides osbasicwindow::WindowActivation
and hides it from derived classes because the second version of
oswindow::WindowActivation is <private>.
   This function is intended to be overridden by derived classes.
   oswindow::WindowActivation does nothing.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowDeactivation   // the window is being de-activated
(oswindow *  ,                 // I: window that is being activated
 long        ,                 // I: time of activation
 float       ,                 // I: x position of the mouse, WA
 float       );                // I: y position of the mouse, WA

   Informs the window that it is being de-activated.  If the window that is
being activated is a window of this program, the first parameter is the
window that is being activated.  If the window that is being de-activated
belongs to a different application, the first parameter is
static_cast<oswindow *>(OSNULL) .
   This function is called by the second version of
oswindow::WindowDeactivation , which overrides
osbasicwindow::WindowDeactivation and hides it from derived classes because
the second version of oswindow::WindowDeactivation is <private>.
   This function is intended to be overridden by derived classes.
   oswindow::WindowDeactivation does nothing.

----------------------------------------------------------------------------

typedef  osbasicwindow  inherited;   // the base class of <oswindow>

   This type allows <oswindow> to easily access its base class,
<osbasicwindow>.

----------------------------------------------------------------------------

oswindow::oswindow     // dummy private copy constructor
(oswindow const &)     //    prevents copying
{}

oswindow const &       // dummy private assignment operator
oswindow::operator =   //    prevents assignment
(oswindow const &)
const
{ return *this; }

   These two functions make sure that it is not possible to copy one
<oswindow> to another <oswindow>.  Making such a copy would endanger the
integrity of <osrtl><window><safewin>.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowActivation   // the window is being activated
(priv_windowhandle  other,   // I: window that is being de-activated
 long               time,    // I: time of activation
 float              x,       // I: x position of the mouse, WA
 float              y);      // I: y position of the mouse, WA

   Overrides osbasicwindow::WindowActivation and hides it from the outside
world because oswindow::WindowActivation is <private>.  The purpose of this
function is to translate <priv_windowhandle> to <oswindow *>, which is both
friendlier and more useful.
   This function is not intended to be overridden by derived classes.
Derived classes should override the first version of
oswindow::WindowActivation .

----------------------------------------------------------------------------

virtual
void
oswindow::WindowDeactivation   // the window is being de-activated
(priv_windowhandle  other,     // I: window that is being activated
 long               time,      // I: time of activation
 float              x,         // I: x position of the mouse, WA
 float              y);        // I: y position of the mouse, WA

   Overrides osbasicwindow::WindowDeactivation and hides it from the outside
world because oswindow::WindowDeactivation is <private>.  The purpose of
this function is to translate <priv_windowhandle> to <oswindow *>, which is
both friendlier and more useful.
   This function is not intended to be overridden by derived classes.
Derived classes should override the first version of
oswindow::WindowDeactivation .

----------------------------------------------------------------------------

osresult                    // result code
oswindow::MarkDestruction   // marks destruction of an <oswindow>
();

   Helper function that marks destruction of an <oswindow>.  The destruction
is marked by removing the pointer to the window from the list of all
currently existing windows <windowlist>.  This list is used internally by
<safewin>.

----------------------------------------------------------------------------

osbool              // does the <oswindow> exist?
oswindow::exists;

   Tells if the <oswindow> has been created and not yet destroyed.  This
data member is managed internally by <safewin>.

----------------------------------------------------------------------------

struct windowdata          // data on an <osbasicwindow>
{
   oswindow *    window;   // an existing <oswindow>
   windowdata *  next;     // next <windowdata> in the list
};  // windowdata

   An element of the list of currently existing windows <windowlist> that is
maintained internally by <safewin>.

----------------------------------------------------------------------------

class guard_osrtl_window_safewin   // guards this module
{
   //...
};  // guard_osrtl_window_safewin

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

virtual
guard_osrtl_window_safewin::~guard_osrtl_window_safewin   // performs exit
();                                                       //    checks on
                                                          //    the module

   This destructor checks if all windows have been properly destroyed.  If
not, it destroys all windows that haven't yet been destroyed.  In debugging
mode, it warns the developer about each window that was created but not
destroyed.

----------------------------------------------------------------------------

// List of all currently existing <oswindow>s.
static  windowdata *  windowlist = static_cast<windowdata *>(OSNULL);

   The head of a list of all currently existing windows.  This list is
maintained internally by <safewin>.  It is used internally by <safewin>.

----------------------------------------------------------------------------

static
guard_osrtl_window_safewin   // guards this module
guard;

   Instantiates a global <guard_osrtl_window_safewin>.  This ensures that
the destructor of <guard_osrtl_window_safewin> is activated when the module
<safewin> is closed down.  Refer to the description of that destructor for
details on the checks that are performed when <safewin> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/window/safewin/safewin.d .
----------------------------------------------------------------------------


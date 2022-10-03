
/**************************************************************************
 * File:     osrtl/windows/window/safewin/safewin.cpp                     *
 * Contents: Generic window.                                              *
 * Document: osrtl/windows/window/safewin/safewin.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/safewin/safewin.cpp  -  Generic window

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:33  erick
// First version of the MS-Windows shell (currently only osrtl).
//


/**************************************************************************
 * Detailed history.                                                      *
 **************************************************************************/

// -------------------------------------------------------------------------
// Revision  Concise description of the changes
// Dates
// Author
// -------------------------------------------------------------------------
//  1.1.000  First issue of this module source file.
// 19970720
// 19970819
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"     // basic definitions
#include "osrtl/windows/error/error.h"       // error handling
#include "osrtl/windows/tasking/tasking.h"   // task control
#include "osrtl/windows/heap/heap.h"         // heap
#include "osrtl/windows/drawing/drawing.h"   // attributes for drawing

// Other modules of the component <window>.
#include "osrtl/windows/window/winpaint/winpaint.h"   // window painter
#include "osrtl/windows/window/winfont/winfont.h"     // window font
#include "osrtl/windows/window/window/window.h"       // basic window

// Header file of the current module.
#include "osrtl/windows/window/safewin/safewin.h"


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

struct windowdata          // data on an <osbasicwindow>
{
   oswindow *    window;   // an existing <oswindow>
   windowdata *  next;     // next <windowdata> in the list
};  // windowdata


class guard_osrtl_window_safewin   // guards this module
{
public:
   virtual
   ~guard_osrtl_window_safewin     // performs exit checks on the module
   ();

};  // guard_osrtl_window_safewin


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

// List of all currently existing <oswindow>s.
static  windowdata *  windowlist = static_cast<windowdata *>(OSNULL);

static
guard_osrtl_window_safewin   // guards this module
guard;


/**************************************************************************
 * Function: oswindow::oswindow                                           *
 **************************************************************************/

oswindow::oswindow   // initialises an <oswindow>
()

{  // oswindow::oswindow

   exists = OSFALSE;

}  // oswindow::oswindow


/**************************************************************************
 * Function: oswindow::~oswindow                                          *
 **************************************************************************/

oswindow::~oswindow   // closes down an <oswindow>
()

{  // oswindow::~oswindow

   if (exists)
   {
      OsDebugErrorMessage(L"An <oswindow> was not properly closed down.\n"
                          L"Name: \"", Name(), L"\".");

      if (Destroy() != OSRESULT_OK)
         OsDebugErrorMessage(L"Cannot destroy an <oswindow> that was "
                             L"not properly closed down.");
   }

}  // oswindow::~oswindow


/**************************************************************************
 * Function: oswindow::Create                                             *
 **************************************************************************/

osresult                    // result code
oswindow::Create            // creates an <oswindow>
(float            left,     // I: x co-ordinate of left   edge
 float            right,    // I: x co-ordinate of right  edge
 float            top,      // I: y co-ordinate of top    edge
 float            bottom,   // I: y co-ordinate of bottom edge
 wchar_t const *  name,     // I: name of the window
 osbool           vis)      // I: must the window be visible?

{  // oswindow::Create

   osresult  result;   // result code

   result = inherited::Create(left, right, top, bottom, name, vis);
   if (result == OSRESULT_OK)
   {
      windowdata *  data;   // data on this window

      // Allocate <data>.
      {
         osheap<windowdata>  heap;

         data = heap.Allocate(L"<windowdata>");
      }

      if (data != static_cast<windowdata *>(OSNULL))
      {
         osmutex  mutex;   // mutex for managing the window list

         if (    (result = mutex.Connect(MUTEX_SAFEWINDOW)) == OSRESULT_OK
             &&  (result = mutex.EnterSection())            == OSRESULT_OK)
         {
            data->window = this;
            data->next   = windowlist;
            windowlist   = data;

            if ((result = mutex.LeaveSection()) == OSRESULT_OK)
               exists = OSTRUE;
            else
            {
               // We have a problem.  Remove <data> from <windowlist>.
               windowlist = windowlist->next;
               data->next = static_cast<windowdata *>(OSNULL);
            }
         }

         // In case of a problem, de-allocate <data>.
         if (result != OSRESULT_OK)
         {
            osheap<windowdata>  heap;

            if (heap.Deallocate(data) != OSRESULT_OK)
               OsDebugErrorMessage(L"Cannot de-allocate a <windowdata>\n"
                                   L"after a problem in using it.");
         }
      }
      else
         result = OSERROR_NOMEMORY;

      // In case of a problem, destroy the window.
      if (result != OSRESULT_OK)
         if (inherited::Destroy() != OSRESULT_OK)
            OsDebugErrorMessage(L"Cannot destroy a basic window after\n"
                                L"a failure to add it to the list "
                                L"of existing windows.");
   }

   return result;

}  // oswindow::Create


/**************************************************************************
 * Function: oswindow::WindowDestruction                                  *
 **************************************************************************/

void
oswindow::WindowDestruction   // the window is being destroyed
(long   time,                 // I: time of window destruction
 float  x,                    // I: x position of the mouse
 float  y)                    // I: y position of the mouse

{  // oswindow::WindowDestruction

   if (MarkDestruction() != OSRESULT_OK)
      OsDebugErrorMessage(L"Cannot mark the destruction of an <oswindow>.");
   inherited::WindowDestruction(time, x, y);

}  // oswindow::WindowDestruction


/**************************************************************************
 * Function: oswindow::WindowActivation                                   *
 **************************************************************************/

void
oswindow::WindowActivation   // the window is being activated
(oswindow *  ,               // -: window that is being de-activated
 long        ,               // -: time of activation
 float       ,               // -: x position of the mouse
 float       )               // -: y position of the mouse

{  // oswindow::WindowActivation

   // Do not react to this message.
   (void)0;

}  // oswindow::WindowActivation


/**************************************************************************
 * Function: oswindow::WindowDeactivation                                 *
 **************************************************************************/

void
oswindow::WindowDeactivation   // the window is being de-activated
(oswindow *  ,                 // -: window that is being activated
 long        ,                 // -: time of activation
 float       ,                 // -: x position of the mouse
 float       )                 // -: y position of the mouse

{  // oswindow::WindowDeactivation

   // Do not react to this message.
   (void)0;

}  // oswindow::WindowDeactivation


/**************************************************************************
 * Function: oswindow::WindowActivation                                   *
 **************************************************************************/

void
oswindow::WindowActivation   // the window is being activated
(priv_windowhandle  other,   // I: window that is being de-activated
 long               time,    // I: time of activation
 float              x,       // I: x position of the mouse
 float              y)       // I: y position of the mouse

{  // oswindow::WindowActivation

   osmutex  mutex;   // mutex for accessing the window list

   if (    mutex.Connect(MUTEX_SAFEWINDOW) == OSRESULT_OK
       &&  mutex.EnterSection()            == OSRESULT_OK)
   {
      oswindow *  deactive;   // window that is being de-activated

      // Check if one of the windows in <windowlist> is <other>.
      {
         windowdata *  window;   // element of list of existing windows

         window = windowlist;
         while (    window != static_cast<windowdata *>(OSNULL)
                &&  window->window->priv_handle != other)
            window = window->next;

         if (window != static_cast<windowdata *>(OSNULL))
            deactive = window->window;
         else
            deactive = static_cast<oswindow *>(OSNULL);
      }

      // Inform the window about its activation.
      WindowActivation(deactive, time, x, y);

      if (mutex.LeaveSection() != OSRESULT_OK)
         OsDebugErrorMessage(L"Cannot leave the critical section\n"
                             L"for doing window administration.");
   }
   else
      OsDebugErrorMessage(L"Cannot enter the critical section\n"
                          L"for doing window administration.");

}  // oswindow::WindowActivation


/**************************************************************************
 * Function: oswindow::WindowDeactivation                                 *
 **************************************************************************/

void
oswindow::WindowDeactivation   // the window is being de-activated
(priv_windowhandle  other,     // I: window that is being activated
 long               time,      // I: time of activation
 float              x,         // I: x position of the mouse
 float              y)         // I: y position of the mouse

{  // oswindow::WindowDeactivation

   osmutex  mutex;   // mutex for accessing the window list

   if (    mutex.Connect(MUTEX_SAFEWINDOW) == OSRESULT_OK
       &&  mutex.EnterSection()            == OSRESULT_OK)
   {
      oswindow *  active;   // window that is being activated

      // Check if one of the windows in <windowlist> is <other>.
      {
         windowdata *  window;   // element of list of existing windows

         window = windowlist;
         while (    window != static_cast<windowdata *>(OSNULL)
                &&  window->window->priv_handle != other)
            window = window->next;

         if (window != static_cast<windowdata *>(OSNULL))
            active = window->window;
         else
            active = static_cast<oswindow *>(OSNULL);
      }

      // Inform the window about its de-activation.
      WindowDeactivation(active, time, x, y);

      if (mutex.LeaveSection() != OSRESULT_OK)
         OsDebugErrorMessage(L"Cannot leave the critical section\n"
                             L"for doing window administration.");
   }
   else
      OsDebugErrorMessage(L"Cannot enter the critical section\n"
                          L"for doing window administration.");

}  // oswindow::WindowDeactivation


/**************************************************************************
 * Function: oswindow::MarkDestruction                                    *
 **************************************************************************/

osresult                    // result code
oswindow::MarkDestruction   // marks destruction of an <oswindow>
()

{  // oswindow::MarkDestruction

   osresult  result;   // result code
   osmutex   mutex;    // mutex for managing the window list

   if (    (result = mutex.Connect(MUTEX_SAFEWINDOW)) == OSRESULT_OK
       &&  (result = mutex.EnterSection())            == OSRESULT_OK)
   {
      windowdata *  prev;   // list element before target <windowdata>
      windowdata *  curr;   // current list element

      // Locate the <windowdata> in <windowlist>.
      {
         curr = windowlist;
         prev = static_cast<windowdata *>(OSNULL);
         while (    curr != static_cast<windowdata *>(OSNULL)
                &&  curr->window != this)
         {
            prev = curr;
            curr = curr->next;
         }
      }

      if (curr != static_cast<windowdata *>(OSNULL))
      {
         // Remove <curr> from the list of existing windows.
         {
            if (prev != static_cast<windowdata *>(OSNULL))
               prev->next = curr->next;
            else
               windowlist = curr->next;

            curr->next = static_cast<windowdata *>(OSNULL);
         }

         // De-allocate <curr>.
         {
            osheap<windowdata>  heap;

            result = heap.Deallocate(curr);
         }

         // Mark the window as having been destroying.
         exists = OSFALSE;
      }
      else
         // <this> is not present in the list of existing windows.
         result = OSINTERNAL_WINDOWADMIN;

      // Leave the critical section.
      {
         osresult  leave;   // result of leaving the critical section

         leave = mutex.LeaveSection();
         if (result == OSRESULT_OK)
            result = leave;
         else
            if (leave != OSRESULT_OK)
               OsDebugErrorMessage(L"Cannot leave the critical "
                                   L"section after\n"
                                   L"failure to destroy "
                                   L"an <oswindow>.");
      }
   }

   return result;

}  // oswindow::MarkDestruction


/**************************************************************************
 * Function: guard_osrtl_window_safewin::~guard_osrtl_window_safewin      *
 **************************************************************************/

guard_osrtl_window_safewin::~guard_osrtl_window_safewin
()

{  // guard_osrtl_window_safewin::~guard_osrtl_window_safewin

   while (windowlist != static_cast<windowdata *>(OSNULL))
   {
      OsDebugErrorMessage(L"An <oswindow> was not properly "
                          L"destroyed.\nName of the <oswindow>:\n\"",
                          windowlist->window->Name(),
                          L"\".");

      if (windowlist->window->Destroy() != OSRESULT_OK)
         OsDebugErrorMessage(L"Cannot destroy an undestroyed "
                             L"<oswindow>.");
   }

}  // guard_osrtl_window_safewin::~guard_osrtl_window_safewin


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/window/safewin/safewin.cpp .                      *
 **************************************************************************/


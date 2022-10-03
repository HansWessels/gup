
/**************************************************************************
 * File:     osrtl/windows/window/admin/admin.cpp                         *
 * Contents: Window administration.                                       *
 * Document: osrtl/windows/window/admin/admin.d                           *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/admin/admin.cpp  -  Window administration

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:30  erick
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
// 19970711
// 19970806
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// The unit <osrtl> is the only unit that is allowed to include files that
// have to do with the operating system and the RTL.
#define STRICT
#include <windows.h>   // header file of the OS

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"     // basic definitions
#include "osrtl/windows/error/error.h"       // error handling
#include "osrtl/windows/startup/startup.h"   // start-up
#include "osrtl/windows/tasking/tasking.h"   // task control
#include "osrtl/windows/heap/heap.h"         // heap
#include "osrtl/windows/text/text.h"         // textual data

// Header file of the current module.
#include "osrtl/windows/window/admin/admin.h"


/**************************************************************************
 * Local constants.                                                       *
 **************************************************************************/

// The full name of this module.
const  wchar_t const * const  modulename  =  L"OSRTL_WINDOW_ADMIN";


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class guard_osrtl_window_admin:   // guards this module
   private windowadmin            //    window administrator
{
public:
   // Life cycle.

   ~guard_osrtl_window_admin      // performs exit checks on the module
   ();

};  // guard_osrtl_window_admin


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

unsigned int                       // number of existing windows
windowadmin::windows
= 0;

osbool                             // has a window ever existed?
windowadmin::ever
= OSFALSE;

windowadmin::windowclassid const   // name of the window class
windowadmin::classname
= modulename;

static
guard_osrtl_window_admin           // guards this module
guard;


/**************************************************************************
 * Function: windowadmin::windowadmin                                     *
 **************************************************************************/

windowadmin::windowadmin   // initialises a <windowadmin>
()

{  // windowadmin::windowadmin

   if (!SetName(L"<windowadmin>"))
      OsDebugErrorMessage(L"Cannot set the name of a <windowadmin>.");

}  // windowadmin::windowadmin


/**************************************************************************
 * Function: windowadmin::Create                                          *
 **************************************************************************/

osresult                     // result code
windowadmin::Create          // administers creation of a window
(windowproc       Handler,   // I: window procedure
 windowclassid *  id)        // O: ID of the window class

{  // windowadmin::Create

   osresult  result;   // result code
   osmutex   mutex;    // mutex for window administration

   if (    (result = mutex.Connect(MUTEX_WINDOWADMIN)) == OSRESULT_OK
       &&  (result = mutex.EnterSection())             == OSRESULT_OK)
   {
      // If no windows exist, register the window class with Windows.
      if (windows == 0)
      {
         WNDCLASS  wc;   // window class for generic windows

         wc.style         =   CS_HREDRAW
                            | CS_VREDRAW
                            | CS_PARENTDC
                            | CS_DBLCLKS;
         wc.lpfnWndProc   = reinterpret_cast<WNDPROC>(Handler);
         wc.cbClsExtra    = 0;
         wc.cbWndExtra    = 0;
         wc.hInstance     = static_cast<HINSTANCE>(ProgramInstance());
         wc.hIcon         = static_cast<HICON>(OSNULL);
         wc.hCursor       = static_cast<HCURSOR>(OSNULL);
         wc.hbrBackground = static_cast<HBRUSH>(OSNULL);

         #ifdef OSRTL_UNICODE
            wc.lpszMenuName  = static_cast<wchar_t const *>(OSNULL);
            wc.lpszClassName = classname;
         #else
            wc.lpszMenuName = static_cast<char const *>(OSNULL);
            result = StringDuplicate
                        (L"name of the window class",
                         classname,
                         const_cast<char * *>(&wc.lpszClassName));
         #endif

         if (result == OSRESULT_OK)
            if (!RegisterClass(&wc))
               result = OSERROR_NORSRC_WINDOW;

         #ifndef OSRTL_UNICODE
         {
            osheap<char>  heap;
            osresult      dealloc;   // de-allocation result

            dealloc = heap.Deallocate(const_cast<char *>(wc.lpszClassName));
            if (result == OSRESULT_OK)
               result = dealloc;
            else
               if (dealloc != OSRESULT_OK)
                  OsDebugErrorMessage(L"Cannot de-allocate the ANSI\n"
                                      L"version of the name of the\n"
                                      L"window class.");
         }
         #endif
      }

      if (result == OSRESULT_OK)
      {
         ++windows;
         ever = OSTRUE;
         *id = classname;
         result = mutex.LeaveSection();
      }
      else
         if (mutex.LeaveSection() != OSRESULT_OK)
            OsDebugErrorMessage(L"Cannot leave a critical section\n"
                                L"while creating a window.");
   }

   return result;

}  // windowadmin::Create


/**************************************************************************
 * Function: windowadmin::Destroy                                         *
 **************************************************************************/

osresult               // result code
windowadmin::Destroy   // administers destruction of a window
()

{  // windowadmin::Destroy

   osresult  result;   // result code
   osmutex   mutex;    // mutex for window administration

   if (    (result = mutex.Connect(MUTEX_WINDOWADMIN)) == OSRESULT_OK
       &&  (result = mutex.EnterSection())             == OSRESULT_OK)
   {
      --windows;
      result = mutex.LeaveSection();
   }

   return result;

}  // windowadmin::Destroy


/**************************************************************************
 * Function: guard_osrtl_window_admin::~guard_osrtl_window_admin          *
 **************************************************************************/

guard_osrtl_window_admin::~guard_osrtl_window_admin  // performs exit checks
()                                                   //    on the module

{  // guard_osrtl_window_admin::~guard_osrtl_window_admin

   if (Windows() != 0)
   {
      OsDebugErrorMessage(L"According to the window administration,\n"
                          L"not all windows have been destroyed.");

      // Destroy all yet undestroyed windows.
      while (Destroy() == OSRESULT_OK  &&  Windows() != 0)
         (void)0;
   }

   // If all windows have been destroyed, unregister the window class.
   if (Windows() == 0  &&  Ever())
   #ifdef OSRTL_UNICODE
   {
      if (!UnregisterClass(classname,
                           static_cast<HINSTANCE>(ProgramInstance())))
         OsDebugErrorMessage(L"Cannot unregister the window class.");
   }
   #else
   {
      char *  classnameansi;   // ANSI version of <classname>

      if (StringDuplicate(L"ANSI version of the name of the window class",
                          ClassName(),
                          &classnameansi) == OSRESULT_OK)
      {
         if (!UnregisterClass(classnameansi,
                              static_cast<HINSTANCE>(ProgramInstance())))
            OsDebugErrorMessage(L"Cannot unregister the window class.");

         {
            osheap<char>  heap;

            if (heap.Deallocate(classnameansi) != OSRESULT_OK)
               OsDebugErrorMessage(L"Cannot de-allocate the\n"
                                   L"ANSI duplicate of the name\n"
                                   L"of the window class.");
         }
      }
      else
         OsDebugErrorMessage(L"Cannot create an ANSI duplicate\n"
                             L"of the name of the window class.");
   }
   #endif

}  // guard_osrtl_window_admin::~guard_osrtl_window_admin


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/window/admin/admin.cpp .                          *
 **************************************************************************/


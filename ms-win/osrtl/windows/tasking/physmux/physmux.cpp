
/**************************************************************************
 * File:     osrtl/windows/tasking/physmux/physmux.cpp                    *
 * Contents: Physical mutexes.                                            *
 * Document: osrtl/windows/tasking/physmux/phyxmux.d                      *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/tasking/physmux/physmux.cpp  -  Physical mutexes

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:10  erick
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
// 19970716
// 19970724
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
#include "osrtl/windows/basics/basics.h"   // basic definitions
#include "osrtl/windows/error/error.h"     // error handling

// Other modules of the component <tasking>.
#include "osrtl/windows/tasking/mutexid/mutexid.h"   // mutex IDs

// Header file of the current module.
#include "osrtl/windows/tasking/physmux/physmux.h"


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class guard_osrtl_tasking_physmux   // guards this module
{
public:
   // Life cycle.

   virtual
   ~guard_osrtl_tasking_physmux     // performs exit checks on the module
   ();

};  // guard_osrtl_tasking_physmux


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

static
int
alive         // total number of existing physical mutexes
= 0;

static
int
connections   // total number of connections to physical mutexes
= 0;

static
int
criticals     // total number of active critical sections
= 0;

static
guard_osrtl_tasking_physmux   // guards this module
guard;


/**************************************************************************
 * Function: physmutex::physmutex                                         *
 **************************************************************************/

physmutex::physmutex   // initialises a physical mutex
(mutexid  id)          // I: ID of the physical mutex

{  // physmutex::physmutex

   if (!SetName(L"<physmutex>"))
      OsDebugErrorMessage(L"Cannot set the name of a <physmutex>.");

   if (++alive <= 0)
      OsDebugErrorMessage(L"Error in the administration "
                          L"of a physical mutex.");

   ident  = id;
   inside = OSFALSE;
   users  = 0;
   next   = static_cast<physmutex *>(OSNULL);

}  // physmutex::physmutex


/**************************************************************************
 * Function: physmutex::~physmutex                                        *
 **************************************************************************/

physmutex::~physmutex   // closes down a physical mutex
()

{  // physmutex::~physmutex

   if (--alive < 0)
      OsDebugErrorMessage(L"Error in the administration "
                          L"of a physical mutex.");

   if (inside)
      OsDebugErrorMessage(L"A physical mutex is destroyed while some\n"
                          L"code is still inside a critical section\n"
                          L"that is guarded by that physical mutex.");

   if (users != 0)
      OsDebugErrorMessage(L"A physical mutex is destroyed while some\n"
                          L"access mutexes are still connected to it.");

   if (next != static_cast<physmutex *>(OSNULL))
      OsDebugErrorMessage(L"A physical mutex is destroyed while it\n"
                          L"is still in a list of physical mutexes.");

}  // physmutex::~physmutex


/**************************************************************************
 * Function: physmutex::Connect                                           *
 **************************************************************************/

void
physmutex::Connect   // an access mutex connects to the physical mutex
()

{  // physmutex::Connect

   ++users;
   ++connections;

   if (users <= 0  ||  connections <= 0)
      OsDebugErrorMessage(L"Error in the administration "
                          L"of a physical mutex.");

}  // physmutex::Connect


/**************************************************************************
 * Function: physmutex::Disconnect                                        *
 **************************************************************************/

void
physmutex::Disconnect   // an access mutex disconnects from the phys. mutex
()

{  // physmutex::Disconnect

   --users;
   --connections;

   if (users < 0  ||  connections < 0)
      OsDebugErrorMessage(L"Error in the administration "
                          L"of a physical mutex.");

}  // physmutex::Disconnect


/**************************************************************************
 * Function: physmutex::EnterSection                                      *
 **************************************************************************/

void
physmutex::EnterSection   // waits until it's safe to enter critical section
()

{  // physmutex::EnterSection

   // The current version of this function is targeted for a single-threaded
   // environment.  In such an environment, it is not possible that more
   // than one process enters a critical section at one time.  Therefore,
   // do nothing.
   // This is a point of improvement (**POI**).

   if (!inside)
   {
      inside = OSTRUE;
      if (++criticals <= 0)
         OsDebugErrorMessage(L"Error in the administration "
                             L"of a physical mutex");
   }
   else
      OsDebugErrorMessage(L"Entering a critical section "
                          L"that was already entered.");

   (void)0;

}  // physmutex::EnterSection


/**************************************************************************
 * Function: physmutex::LeaveSection                                      *
 **************************************************************************/

void
physmutex::LeaveSection   // tells that critical section was left
()

{  // physmutex::LeaveSection

   // The current version of this function is targeted for a single-threaded
   // environment.  In such an environment, it is not possible that more
   // than one process enters a critical section at one time.  Therefore,
   // do nothing.
   // This is a point of improvement (**POI**).

   if (inside)
   {
      if (--criticals < 0)
         OsDebugErrorMessage(L"Error in the administration "
                             L"of a physical mutex");
      inside = OSFALSE;
   }
   else
      OsDebugErrorMessage(L"Leaving a critical section "
                          L"that was not entered.");

   (void)0;

}  // physmutex::LeaveSection


/**************************************************************************
 * Function: guard_osrtl_tasking_physmux::~guard_osrtl_tasking_physmux    *
 **************************************************************************/

guard_osrtl_tasking_physmux::~guard_osrtl_tasking_physmux   // performs exit
()                                                          //    checks on
                                                            //    the module

{  // guard_osrtl_tasking_physmux::~guard_osrtl_tasking_physmux

   if (alive != 0)
      OsDebugErrorMessage(L"Some physical mutexes "
                          L"have not been destroyed.");

   if (connections != 0)
      OsDebugErrorMessage(L"Some access mutexes are still connected\n"
                          L"to physical mutexes.");

   if (criticals != 0)
      OsDebugErrorMessage(L"Some code is still inside a critical section\n"
                          L"that is guarded by a physical mutex.");

}  // guard_osrtl_tasking_physmux::~guard_osrtl_tasking_physmux


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/tasking/physmux/physmux.cpp .                     *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/tasking/mutex/mutex.cpp                        *
 * Contents: Mutex semaphores.                                            *
 * Document: osrtl/windows/tasking/mutex/mutex.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/tasking/mutex/mutex.cpp  -  Mutex semaphores

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:05  erick
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
// 19970710
// 19970727
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
#include "osrtl/windows/tasking/physmux/physmux.h"   // physical mutexes

// Header file of the current module.
#include "osrtl/windows/tasking/mutex/mutex.h"


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class guard_osrtl_tasking_mutex   // guards this module
{
public:
   // Life cycle.

   virtual
   ~guard_osrtl_tasking_mutex     // performs exit checks on the module
   ();

};  // guard_osrtl_tasking_mutex


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

static
physmutex                    // mutex to access mutex administration
access(MUTEX_MUTEXADMIN);

static
physmutex *                  // list of all physical mutexes
mutexlist
= &access;

static
int                          // total number of existing access mutexes
alive
= 0;

static
guard_osrtl_tasking_mutex    // guards this module
guard;


/**************************************************************************
 * Function: osmutex::osmutex                                             *
 **************************************************************************/

osmutex::osmutex   // initialises an <osmutex>
()

{  // osmutex::osmutex

   if (!SetName(L"<osmutex>"))
      OsDebugErrorMessage(L"Cannot set the name of an <osmutex>.");

   if (++alive <= 0)
      OsDebugErrorMessage(L"Error in the administration "
                          L"of an access mutex.");

   mutex = static_cast<physmutex *>(OSNULL);

}  // osmutex::osmutex


/**************************************************************************
 * Function: osmutex::~osmutex                                            *
 **************************************************************************/

osmutex::~osmutex   // closes down an <osmutex>
()

{  // osmutex::~osmutex

   if (--alive < 0)
      OsDebugErrorMessage(L"Error in the administration "
                          L"of an access mutex.");

   access.EnterSection();

   if (mutex != static_cast<physmutex *>(OSNULL))
   {
      // We are connected to *mutex .  Disconnect.

      mutex->Disconnect();

      // Check if some other <osmutex> is still connected to *mutex .  If
      // not, remove *mutex from the list of physical mutexes and delete it.
      if (!mutex->Connected())
      {
         physmutex *  before;   // physical mutex before *mutex

         // Find <before>.  Test if *before exists.  This test should
         // always return <OSTRUE>, so it is not really necessary.
         // Nonetheless, it is included for additional safety.
         if ((before = SearchMutex(mutex->Id()))
                                      != static_cast<physmutex *>(OSNULL))
         {
            before->SetNextElement(mutex->NextElement());
            mutex->SetNextElement(static_cast<physmutex *>(OSNULL));

            // <delete> may throw an exception.  To prevent any exception
            // from reaching the rest of the program, intercept all such
            // exceptions.
            try
            {
               delete mutex;
            }
            catch (...)
            {
               OsDebugErrorMessage(L"Error deleting physical mutex.");
            }
         }
         else
            OsDebugErrorMessage(L"Cannot find physical mutex to destroy.");
      }
   }

   access.LeaveSection();

}  // osmutex::~osmutex


/**************************************************************************
 * Function: osmutex::Connect                                             *
 **************************************************************************/

osresult           // result code
osmutex::Connect   // connects the <osmutex> to a physical mutex
(mutexid  id)      // I: ID of the physical mutex

{  // osmutex::Connect

   osresult  result;   // result code

   access.EnterSection();

   if (mutex == static_cast<physmutex *>(OSNULL))
   {
      physmutex *  before;   // physical mutex before existing mutex <id>

      if ((before = SearchMutex(id)) == static_cast<physmutex *>(OSNULL))
      {
         // A physical mutex with ID <id> does not yet exist.  Create it.

         // <new> may throw an exception.  To prevent any exception from
         // reaching the rest of the program, intercept all such exceptions.
         try
         {
            mutex = new physmutex(id);
         }
         catch (...)
         {
            mutex = static_cast<physmutex *>(OSNULL);
         }

         if (mutex != static_cast<physmutex *>(OSNULL))
         {
            mutex->SetNextElement(mutexlist->NextElement());
            mutexlist->SetNextElement(mutex);
            mutex->Connect();
            result = OSRESULT_OK;
         }
         else
            result = OSERROR_NOMEMORY;
      }
      else
      {
         // A physical mutex with ID <id> already exists.
         mutex = before->NextElement();
         mutex->Connect();
         result = OSRESULT_OK;
      }
   }
   else
      result = OSINTERNAL_MUTEXADMIN;

   access.LeaveSection();

   return result;

}  // osmutex::Connect


/**************************************************************************
 * Function: osmutex::EnterSection                                        *
 **************************************************************************/

osresult                // result code
osmutex::EnterSection   // waits until it's safe to enter a critical section
()

{  // osmutex::EnterSection

   osresult  result;   // result code

   access.EnterSection();

   if (mutex != static_cast<physmutex *>(OSNULL))
   {
      mutex->EnterSection();
      result = OSRESULT_OK;
   }
   else
      result = OSINTERNAL_MUTEXADMIN;

   access.LeaveSection();

   return result;

}  // osmutex::EnterSection


/**************************************************************************
 * Function: osmutex::LeaveSection                                        *
 **************************************************************************/

osresult                // result code
osmutex::LeaveSection   // indicates that a critical section has been left
()

{  // osmutex::LeaveSection

   osresult  result;   // result code

   access.EnterSection();

   if (mutex != static_cast<physmutex *>(OSNULL))
   {
      mutex->LeaveSection();
      result = OSRESULT_OK;
   }
   else
      result = OSINTERNAL_MUTEXADMIN;

   access.LeaveSection();

   return result;

}  // osmutex::LeaveSection


/**************************************************************************
 * Function: osmutex::SearchMutex                                         *
 **************************************************************************/

physmutex *            // physical mutex _before_ target mutex
osmutex::SearchMutex   // searches an existing physical mutex
(mutexid  id)          // I: ID of the target physical mutex
     
{  // osmutex::SearchMutex

   physmutex *  prev;   // previous phys. mutex in list of physical mutexes
   physmutex *  curr;   // current  phys. mutex in list of physical mutexes

   prev = mutexlist;
   while (    (curr = prev->NextElement())
                                != static_cast<physmutex *>(OSNULL)
          &&  curr->Id() != id)
      prev = curr;

   return   curr != static_cast<physmutex *>(OSNULL)
          ? prev
          : static_cast<physmutex *>(OSNULL);

}  // osmutex::SearchMutex


/**************************************************************************
 * Function: guard_osrtl_tasking_mutex::~guard_osrtl_tasking_mutex        *
 **************************************************************************/

guard_osrtl_tasking_mutex::~guard_osrtl_tasking_mutex   // performs exit
()                                                      //    checks on
                                                        //    this module

{  // guard_osrtl_tasking_mutex::~guard_osrtl_tasking_mutex

   if (alive != 0)
      OsDebugErrorMessage(L"Some access mutexes have not been destroyed.");

   if (mutexlist == &access)
   {
      // If any physical mutexes are still alive, destroy them.
      // Do not check the first physical mutex in the list: that's
      // osmutex::access , which must not be destroyed.
      while (mutexlist->NextElement() != static_cast<physmutex *>(OSNULL))
      {
         OsDebugErrorMessage(L"A physical mutex in the list of physical\n"
                             L"mutexes was not destroyed.");

         // Destroy the physical mutex.
         {
            physmutex *  victim;   // physical mutex to destroy

            victim = mutexlist->NextElement();
            mutexlist->SetNextElement(victim->NextElement());
            victim->SetNextElement(static_cast<physmutex *>(OSNULL));

            // <delete> may throw an exception.  To prevent any exception
            // from reaching the rest of the program, intercept all such
            // exceptions.
            try
            {
               delete victim;
            }
            catch (...)
            {
               OsDebugErrorMessage(L"Error deleting physical mutex.");
            }
         }
      }
   }
   else
      OsDebugErrorMessage(L"Error in the administration of\n"
                          L"the list of physical mutexes.");

}  // guard_osrtl_tasking_mutex::~guard_osrtl_tasking_mutex


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/tasking/mutex/mutex.cpp .                         *
 **************************************************************************/


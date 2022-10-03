
/**************************************************************************
 * File:     osrtl/windows/tasking/mutex/mutex.h                          *
 * Contents: Mutex semaphores.                                            *
 * Document: osrtl/windows/tasking/mutex.mutex.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/tasking/mutex/mutex.h  -  Mutex semaphores

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
//  1.1.000  First issue of this module header file.
// 19970710
// 19970719
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_TASKING_MUTEX
#define OSRTL_TASKING_MUTEX


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class osmutex:               // mutex semaphore
   public osobject           //    generic object
{
public:
   // Life cycle.

   osmutex                   // initialises an <osmutex>
   ();

   virtual
   ~osmutex                  // closes down an <osmutex>
   ();

   // Actual functionality.

   virtual
   osresult                  // result code
   Connect                   // connects the <osmutex> to a physical mutex
   (mutexid  id);            // I: ID of the physical mutex

   virtual
   osresult                  // result code
   EnterSection              // waits until safe to enter critical section
   ();

   virtual
   osresult                  // result code
   LeaveSection              // indicates: critical section has been left
   ();

private:
   // Protective functions.
   osmutex                   // dummy private copy constructor
   (osmutex const &)         //    prevents copying
   {}

   osmutex const &           // dummy private assignment operator
   operator =                //    prevents assignment
   (osmutex const &)
   const
   { return *this; }

   // Helper functions.

   static
   physmutex *               // physical mutex _before_ target mutex
   SearchMutex               // searches an existing physical mutex
   (mutexid  id);            // I: ID of the target physical mutex
                 
   // Data.

   physmutex *               // phys. mutex to which <osmutex> is connected
   mutex;

};  // osmutex


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#endif


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/tasking/mutex/mutex.h .                           *
 **************************************************************************/


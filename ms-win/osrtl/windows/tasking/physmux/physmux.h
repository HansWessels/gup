
/**************************************************************************
 * File:     osrtl/windows/tasking/physmux/physmux.h                      *
 * Contents: Physical mutexes.                                            *
 * Document: osrtl/windows/tasking/physmux/phyxmux.d                      *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/tasking/physmux/physmux.h  -  Physical mutexes

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:11  erick
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
// 19970716
// 19970724
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_TASKING_PHYSMUX
#define OSRTL_TASKING_PHYSMUX


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class physmutex:     // physical mutex
   public osobject   //    generic object
{
public:
   // Life cycle.

   physmutex         // initialises a physical mutex
   (mutexid  id);    // I: ID of the physical mutex

   virtual
   ~physmutex        // closes down a physical mutex
   ();

   // Actual functionality.

   void
   Connect           // an access mutex connects to the physical mutex
   ();

   void
   Disconnect        // an access mutex disconnects from the physical mutex
   ();

   void
   EnterSection      // waits until it's safe to enter a critical section
   ();

   void
   LeaveSection      // tells that critical section was left
   ();

   // Obtaining information.

   inline
   mutexid           // the ID of the physical mutex
   Id                // obtains the ID of the physical mutex
   ()
   const;

   inline
   osbool            // any access mutex is connected to the physical mutex?
   Connected         // checks if any access mutex is connected
   ()
   const;

   // List management.

   inline
   void
   SetNextElement        // defines the next element in the list
   (physmutex *  elm);   // I: the next element in the list

   inline
   physmutex *           // next element in the list
   NextElement           // retrieves the next element in the list
   ()
   const;

private:
   // Protective functions.

   physmutex             // dummy private default constructor
   ()                    //    prevents default construction
   {}

   physmutex             // dummy private copy constructor
   (physmutex const &)   //    prevents copying
   {}

   physmutex const &     // dummy private assignment operator
   operator =            //    prevents assignment
   (physmutex const &)
   const
   { return *this; }

   // Data.

   mutexid        // ID of this physical mutex
   ident;

   osbool         // any task inside critical section?
   inside;

   int            // #users of this physical mutex
   users;

   physmutex *    // next physical mutex in the list
   next;

};  // physmutex


/**************************************************************************
 * Function: physmutex::Id                                                *
 **************************************************************************/

inline
mutexid         // the ID of the physical mutex
physmutex::Id   // obtains the ID of the physical mutex
()
const

{  // physmutex::Id

   return ident;

}  // physmutex::Id


/**************************************************************************
 * Function: physmutex::Connected                                         *
 **************************************************************************/

inline
osbool                 // any access mutex connected to the physical mutex?
physmutex::Connected   // checks if any access mutex is connected
()
const

{  // physmutex::Connected

   return static_cast<osbool>(users > 0);

}  // physmutex::Connected


/**************************************************************************
 * Function: physmutex::SetNextElement                                    *
 **************************************************************************/

inline
void
physmutex::SetNextElement   // defines the next element in the list
(physmutex *  elm)          // I: the next element in the list

{  // physmutex::SetNextElement

   next = elm;

}  // physmutex::SetNextElement


/**************************************************************************
 * Function: physmutex::NextElement                                       *
 **************************************************************************/

inline
physmutex *              // the next element in the list
physmutex::NextElement   // retrieves the next element in the list
()
const

{  // physmutex::NextElement

   return next;

}  // physmutex::NextElement


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
 * End of osrtl/windows/tasking/physmux/physmux.h .                       *
 **************************************************************************/


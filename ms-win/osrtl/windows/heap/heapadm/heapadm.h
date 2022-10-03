
/**************************************************************************
 * File:     osrtl/windows/heap/heapadm/heapadm.h                         *
 * Contents: Heap administration.                                         *
 * Document: osrtl/windows/heap/heapadm/heapadm.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/heapadm/heapadm.h  -  Heap administration

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:37  erick
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
// 19970717
// 19970806
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_HEAP_HEAPADM
#define OSRTL_HEAP_HEAPADM


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class heapadministrator:       // heap administrator
   public osobject             //    generic object
{
public:
   // Life cycle.

   inline
   heapadministrator           // initialises a <heapadministrator>
   ();

   // Actual functionality.

   static
   osbool                      // block could be marked as allocated?
   MarkAllocation              // marks a block as having been allocated
   (void const *     block,    // I: block to mark
    wchar_t const *  descr);   // I: description of the block

   static
   osbool                      // block could be marked as de-allocated?
   MarkDeallocation            // marks a block as having been de-allocated
   (void const *  block);      // I: block to mark

   static
   osbool                      // block is allocated?
   Allocated                   // checks if a block is allocated
   (void const *  block);      // I: block to check

private:
   // Protective functions.

   heapadministrator             // dummy private copy constructor
   (heapadministrator const &)   //    prevents copying
   {}

   heapadministrator const &     // dummy private assignment operator
   operator =                    //    prevents assignment
   (heapadministrator const &)
   const
   { return *this; }

   // Implementation of the actual functionality.

   static
   osbool                     // block is allocated?
   Allocated                  // checks if a block is allocated
   (void const *     block,   // I: block to check
    osheapblock * *  prev);   // O: info block before info block of <block>

};  // heapadministrator


/**************************************************************************
 * Function: heapadministrator::heapadministrator                         *
 **************************************************************************/

inline
heapadministrator::heapadministrator   // initialises a <heapadministrator>
()

{  // heapadministrator::heapadministrator

   if (!SetName(L"<heapadministrator>"))
      OsDebugErrorMessage(L"Cannot set the name of a <heapadministrator>.");

}  // heapadministrator::heapadministrator


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
 * End of osrtl/windows/heap/heapadm/heapadm.h .                          *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/heap/heapblck/heapblck.h                       *
 * Contents: Heap blocks.                                                 *
 * Document: osrtl/windows/heap/heapadm/heapblck.d                        *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/heapblck/heapblck.h  -  Heap blocks

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:40  erick
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

#ifndef OSRTL_HEAP_HEAPBLCK
#define OSRTL_HEAP_HEAPBLCK


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class osheapblock:   // data on an allocated block
   public osobject   //    generic object
{
public:
   // Life cycle.

   osheapblock       // initialises an <osheapblock>
   ();

   // Data.

   void const *      // address of the allocated block
   block;

   osheapblock *     // next <osheapblock> in the list
   next;

private:
   // Protective functions.

   osheapblock             // dummy private copy constructor
   (const osheapblock &)   //    prevents copying
   {}

   osheapblock const &     // dummy private assignment operator
   operator =              //    prevents assignment
   (osheapblock const &)
   const
   { return *this; }

};  // osheapblock


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
 * End of osrtl/windows/heap/heapblck/heapblck.h .                        *
 **************************************************************************/


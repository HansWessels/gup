
/**************************************************************************
 * File:     osrtl/windows/heap/heap/heap.h                               *
 * Contents: Heap management.                                             *
 * Document: osrtl/windows/heap/heap/heap.d                               *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/heap/heap.h  -  Heap management

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:33  erick
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
// 19970708
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_HEAP_HEAP
#define OSRTL_HEAP_HEAP


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class priv_osbasicheapadmin:   // administrator for the basic heap manager
   public osobject             //    generic object
{
protected:
   static
   inline
   void
   MarkAllocation              // marks a successful allocation
   ();

   static
   inline
   void
   MarkDeallocation            // marks a successful de-allocation
   ();

   static
   inline
   unsigned long               // total number of allocated blocks
   Allocations                 // obtains total number of allocated blocks
   ();

private:
   // Data.

   static
   unsigned long               // total number of allocated blocks
   allocations;

};  // priv_osbasicheapadmin


template <class datatype>
class osbasicheap:                // basic heap manager
   public priv_osbasicheapadmin   //    administrator
{
public:
   // Life cycle.

   inline
   osbasicheap                    // initialises an <osbasicheap>
   ();

   // Actual functionality.

   static
   datatype *                     // allocated data
   HeapAllocate                   // allocates data on the heap
   (unsigned long  n = 1UL);      // I: number of data elements to allocate

   static
   osresult                       // result code
   HeapDeallocate                 // de-allocates data from the heap
   (datatype *  data);            // I: data to de-allocate

private:
   // Protective functions.

   osbasicheap                    // dummy private copy constructor
   (osbasicheap const &)          //    prevents copying
   {}

   osbasicheap const &            // dummy private assignment operator
   operator =                     //    prevents assignment
   (osbasicheap const &)
   const
   { return *this; }

};  // osbasicheap<datatype>


/**************************************************************************
 * Function: osbasicheap<datatype>::osbasicheap                           *
 **************************************************************************/

template <class datatype>
inline
osbasicheap<datatype>::osbasicheap   // initialises an <osbasicheap>
()

{  // osbasicheap::osbasicheap

   if (!SetName(L"<osbasicheap>"))
      OsDebugErrorMessage(L"Cannot set the name of an <osbasicheap>.");

}  // osbasicheap::osbasicheap


/**************************************************************************
 * Function: osbasicheap<datatype>::HeapAllocate                          *
 **************************************************************************/

template <class datatype>
datatype *                            // allocated data, or <OSNULL>
osbasicheap<datatype>::HeapAllocate   // allocates data on the heap
(unsigned long  n)                    // I: #data elements to allocate

{  // osbasicheap<datatype>::HeapAllocate

   datatype *  block;   // allocated block

   // new[] may throw an exception.  To prevent any exception from
   // reaching the rest of the program, intercept all such exceptions.
   try
   {
      block = new datatype[n];
      if (block != static_cast<datatype *>(OSNULL))
         MarkAllocation();
   }
   catch (...)
   {
      block = static_cast<datatype *>(OSNULL);
   }

   return block;

}  // osbasicheap<datatype>::HeapAllocate


/**************************************************************************
 * Function: osbasicheap<datatype>::HeapDeallocate                        *
 **************************************************************************/

template <class datatype>
osresult                                // result code
osbasicheap<datatype>::HeapDeallocate   // de-allocates data from the heap
(datatype *  data)                      // I: data to de-allocate

{  // osbasicheap<datatype>::HeapDeallocate

   osresult  result = OSRESULT_OK;   // result code

   if (data != static_cast<datatype *>(OSNULL))   // allow de-allocation
   {                                              //    of null pointers
      // delete[] may throw an exception.  To prevent any exception from
      // reaching the rest of the program, intercept all such exceptions.
      try
      {
         delete[] data;

         // If execution arrives here, delete[] was successful; therefore,
         // it is OK to mark the de-allocation.
         MarkDeallocation();
      }
      catch (...)
      {
         // It is not possible to de-allocate data from the heap.  This
         // problem must have been caused by some internal error in the
         // administration of the heap.
         result = OSINTERNAL_HEAPADMIN;
      }
   }

   return result;

}  // osbasicheap<datatype>::HeapDeallocate


/**************************************************************************
 * Function: priv_osbasicheapadmin::MarkAllocation                        *
 **************************************************************************/

inline
void
priv_osbasicheapadmin::MarkAllocation   // marks a successful allocation
()

{  // priv_osbasicheapadmin::MarkAllocation

   if (++allocations == 0UL)
      OsDebugErrorMessage(L"Error counting the number "
                          L"of allocated blocks.");

}  // priv_osbasicheapadmin::MarkAllocation


/**************************************************************************
 * Function: priv_osbasicheapadmin::MarkDeallocation                      *
 **************************************************************************/

inline
void
priv_osbasicheapadmin::MarkDeallocation  // marks a successful de-allocation
()

{  // priv_osbasicheapadmin::MarkDeallocation

   if (allocations-- == 0UL)
      OsDebugErrorMessage(L"More de-allocations than allocations.");

}  // priv_osbasicheapadmin::MarkDeallocation


/**************************************************************************
 * Function: priv_osbasicheapadmin::Allocations                           *
 **************************************************************************/

inline
unsigned long                        // total number of allocated blocks
priv_osbasicheapadmin::Allocations   // obtains total #allocated blocks
()

{  // priv_osbasicheapadmin::Allocations

   return allocations;

}  // priv_osbasicheapadmin::Allocations


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
 * End of osrtl/windows/heap/heap/heap.h .                                *
 **************************************************************************/


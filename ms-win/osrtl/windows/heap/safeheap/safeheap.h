
/**************************************************************************
 * File:     osrtl/windows/heap/safeheap/safeheap.h                       *
 * Contents: Checked heap.                                                *
 * Document: osrtl/windows/heap/safeheap/safeheap.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/safeheap/safeheap.h  -  Checked heap

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:42  erick
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
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_HEAP_SAFEHEAP
#define OSRTL_HEAP_SAFEHEAP


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

template <class datatype>
class osheap:                      // checked heap
   private osbasicheap<datatype>   //    basic heap
{
public:
   // Life cycle.

   osheap       // initialises an <osheap>
   ();

   // Actual functionality.

   virtual
   inline
   datatype *                     // allocated data
   Allocate                       // allocates data on the heap
   #ifdef OSRTL_DEBUGGING
      (wchar_t const *  descr,    // I: description of the data elements
       unsigned long    n = 1UL); // I: number of data elements to allocate
   #else
      (wchar_t const *  ,         // -: description of the data elements
       unsigned long    n = 1UL); // I: number of data elements to allocate
   #endif

   virtual
   osresult                       // result code
   Deallocate                     // de-allocates data from the heap
   (datatype *  data);            // I: data to de-allocate

private:
   // Protective functions.

   osheap              // dummy private copy constructor
   (osheap const &)    //    prevents copying
   {}

   osheap const &      // dummy private assignment operator
   operator =          //    prevents assignment
   (osheap const &)
   const
   { return *this; }

   // Implementation of the actual functionality.

   static
   datatype *                 // allocated data
   SafeAllocate               // allocates data on the heap
   (wchar_t const *  descr,   // I: description of the data elements
    unsigned long    n);      // I: number of data elements to allocate

};  // osheap<datatype>


/**************************************************************************
 * Function: osheap<datatype>::osheap                                     *
 **************************************************************************/

template <class datatype>
osheap<datatype>::osheap    // initialises an <osheap>
()

{  // osheap<datatype>::osheap

   if (!SetName(L"<osheap>"))
      OsDebugErrorMessage(L"Cannot set the name of an <osheap>.");

}  // osheap<datatype>::osheap


/**************************************************************************
 * Function: osheap<datatype>::Allocate                                   *
 **************************************************************************/

template <class datatype>
inline
datatype *                     // allocated data
osheap<datatype>::Allocate     // allocates data on the heap
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,    // I: description of the data elements
    unsigned long    n = 1UL)  // I: number of data elements to allocate
#else
   (wchar_t const *  ,         // -: description of the data elements
    unsigned long    n = 1UL)  // I: number of data elements to allocate
#endif

{  // osheap<datatype>::Allocate

   #ifdef OSRTL_DEBUGGING
      return SafeAllocate(descr, n);
   #else
      return SafeAllocate(static_cast<wchar_t const *>(OSNULL), n);
   #endif

}  // osheap<datatype>::Allocate


/**************************************************************************
 * Function: osheap<datatype>::Deallocate                                 *
 **************************************************************************/

template <class datatype>
osresult                       // result code
osheap<datatype>::Deallocate   // de-allocates data from the heap
(datatype *  data)             // I: data to de-allocate

{  // osheap<datatype>::Deallocate

   osresult  result = OSRESULT_OK;   // result code

   // Allow de-allocation of NULL blocks.
   if (data != static_cast<datatype *>(OSNULL))
   {
      heapadministrator  admin;   // administration of allocated blocks

      if (admin.Allocated(static_cast<void const *>(data)))
      {
         if ((result = HeapDeallocate(data)) == OSRESULT_OK)
         {
            if (!admin.MarkDeallocation(static_cast<void const *>(data)))
            {
               OsDebugErrorMessage(L"Could not mark heap block\n"
                                   L"as de-allocated.");
               result = OSINTERNAL_HEAPADMIN;
            }
         }
      }
      else
      {
         OsDebugErrorMessage(L"Attempt to de-allocate a block\n"
                             L"that is not allocated.");
         result = OSINTERNAL_HEAPADMIN;
      }
   }

   return result;

}  // osheap<datatype>::Deallocate


/**************************************************************************
 * Function: osheap<datatype>::SafeAllocate                               *
 **************************************************************************/

template <class datatype>
datatype *                       // allocated data
osheap<datatype>::SafeAllocate   // allocates data on the heap
(wchar_t const *  descr,         // I: description of the data elements
 unsigned long    n)             // I: number of data elements to allocate

{  // osheap<datatype:::SafeAllocate

   datatype *  block;   // allocated data elements

   if ((block = HeapAllocate(n)) != static_cast<datatype *>(OSNULL))
   {
      heapadministrator  admin;   // administration of allocated blocks

      if (!admin.MarkAllocation(static_cast<void const *>(block), descr))
      {
         if (HeapDeallocate(block) != OSRESULT_OK)
            OsDebugErrorMessage(L"Could not mark heap block as allocated.\n"
                                L"Could not de-allocate the heap block.");
         block = static_cast<datatype *>(OSNULL);
      }
   }

   return block;

}  // osheap<datatype:::SafeAllocate


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
 * End of osrtl/windows/heap/safeheap/safeheap.h .                        *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/heap/heapadm/heapadm.cpp                       *
 * Contents: Heap administration.                                         *
 * Document: osrtl/windows/heap/heapadm/heapadm.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/heapadm/heapadm.cpp  -  Heap administration

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:36  erick
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
// 19970717
// 19970818
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

// Other modules of the component <heap>.
#include "osrtl/windows/heap/heap/heap.h"           // heap management
#include "osrtl/windows/heap/heapblck/heapblck.h"   // heap blocks

// Header file of the current module.
#include "osrtl/windows/heap/heapadm/heapadm.h"


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class guard_osrtl_heap_heapadm   // guards this module
{
public:
   // Life cycle.

   virtual
   ~guard_osrtl_heap_heapadm     // performs exit checks on the module
   ();

};  // guard_osrtl_heap_heapadm


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

osheapblock *
allocated                               // list of allocated blocks
= static_cast<osheapblock *>(OSNULL);

static
guard_osrtl_heap_heapadm                // guards this module
guard;


/**************************************************************************
 * Function: heapadministrator::MarkAllocation                            *
 **************************************************************************/

osbool                              // block could be marked as allocated?
heapadministrator::MarkAllocation   // marks a block as being allocated
(void const *     block,            // I: block to mark
 wchar_t const *  descr)            // I: description of the block

{  // heapadministrator::MarkAllocation

   osbool         result = OSFALSE;   // block could be marked as allocated?
   osheapblock *  heapblock;          // information on the block <block>

   // Allocate <heapblock>.
   {
      osbasicheap<osheapblock>  heap;

      heapblock = heap.HeapAllocate();
   }

   if (    heapblock != static_cast<osheapblock *>(OSNULL)
       &&  heapblock->SetName(descr))
   {
      heapblock->block = block;

      // *heapblock has been defined.  Insert it into the list.
      {
         osmutex  mutex;   // mutex for doing heap administration

         if (    mutex.Connect(MUTEX_HEAPADMIN) == OSRESULT_OK
             &&  mutex.EnterSection()           == OSRESULT_OK)
         {
            heapblock->next = allocated;
            allocated       = heapblock;
            result          = OSTRUE;

            if (mutex.LeaveSection() != OSRESULT_OK)
            {
               // Remove *heapblock from the list.
               {
                  allocated = allocated->next;
                  heapblock->next = static_cast<osheapblock *>(OSNULL);
               }

               result = OSFALSE;
            }
         }

         if (!result)
         {
            osbasicheap<osheapblock>  heap;

            heap.HeapDeallocate(heapblock);
         }
      }
   }

   return result;

}  // heapadministrator::MarkAllocation


/**************************************************************************
 * Function: heapadministrator::MarkDeallocation                          *
 **************************************************************************/

osbool                                // block could be marked?
heapadministrator::MarkDeallocation   // marks a block as being de-allocated
(void const *  block)                 // I: block to mark

{  // heapadministrator::MarkDeallocation

   osbool   result = OSFALSE;   // block could be marked as de-allocated?
   osmutex  mutex;              // mutex for doing heap administration

   if (    mutex.Connect(MUTEX_HEAPADMIN) == OSRESULT_OK
       &&  mutex.EnterSection()           == OSRESULT_OK)
   {
      osheapblock *  prev;   // heap block that comes _before_ the heap
                             //    block about <block> in the list

      if (Allocated(block, &prev))
      {
         osheapblock *  about;   // heap block about <block>

         if (prev != static_cast<osheapblock *>(OSNULL))
         {
            about      = prev->next;
            prev->next = about->next;
         }
         else
         {
            about     = allocated;
            allocated = about->next;
         }

         result = OSTRUE;

         // De-allocate <about>.
         {
            osbasicheap<osheapblock>  heap;

            if (heap.HeapDeallocate(about) != OSRESULT_OK)
               result = OSFALSE;
         }
      }
      else
         OsDebugErrorMessage(L"Attempt to mark de-allocation\n"
                             L"of a block that is not allocated.");

      if (mutex.LeaveSection() != OSRESULT_OK)
         result = OSFALSE;
   }

   return result;

}  // heapadministrator::MarkDeallocation


/**************************************************************************
 * Function: heapadministrator::Allocated                                 *
 **************************************************************************/

osbool                         // block is allocated?
heapadministrator::Allocated   // checks if a block is allocated
(void const *  block)          // I: block to check

{  // heapadministrator::Allocated

   osbool   result = OSFALSE;   // block is allocated?
   osmutex  mutex;              // mutex for doing heap administration

   if (mutex.Connect(MUTEX_HEAPADMIN) == OSRESULT_OK)
      if (mutex.EnterSection() == OSRESULT_OK)
      {
         result = Allocated(block, static_cast<osheapblock * *>(OSNULL));
         if (mutex.LeaveSection() != OSRESULT_OK)
            OsDebugErrorMessage(L"Cannot leave a critical section while\n"
                                L"checking if a heap block is allocated.");
      }
      else
         OsDebugErrorMessage(L"Cannot enter critical section for\n"
                             L"heap administration when checking\n"
                             L"if a block is allocated.");
   else
      OsDebugErrorMessage(L"Cannot connect to the mutex "
                          L"for heap administration\n"
                          L"when checking if a block is allocated.");

   return result;

}  // heapadministrator::Allocated


/**************************************************************************
 * Function: heapadministrator::Allocated                                 *
 **************************************************************************/

osbool                         // block is allocated?
heapadministrator::Allocated   // checks if a block is allocated
(void const *     block,       // I: block to check
 osheapblock * *  prev)        // O: info block before info block of <block>

{  // heapadministrator::Allocated

   osbool         result;             // block is allocated?
   osheapblock *  curr = allocated;   // an info block in the list
   osheapblock *  prv;                // block before *curr

   prv = static_cast<osheapblock *>(OSNULL);

   while (    curr != static_cast<osheapblock *>(OSNULL)
          &&  curr->block != block)
   {
      prv  = curr;
      curr = curr->next;
   }

   result = static_cast<osbool>(curr != static_cast<osheapblock *>(OSNULL));
   if (result  &&  prev != static_cast<osheapblock * *>(OSNULL))
      *prev = prv;

   return result;

}  // heapadministrator::Allocated


/**************************************************************************
 * Function: guard_osrtl_heap_heapadm::~guard_osrtl_heap_heapadm          *
 **************************************************************************/

guard_osrtl_heap_heapadm::~guard_osrtl_heap_heapadm  // performs exit checks
()                                                   //    on the module

{  // guard_osrtl_heap_heapadm::~guard_osrtl_heap_heapadm

   osmutex  mutex;   // mutex for doing heap administration

   if (mutex.Connect(MUTEX_HEAPADMIN) == OSRESULT_OK)
      if (mutex.EnterSection() == OSRESULT_OK)
      {
         while (allocated != static_cast<osheapblock *>(OSNULL))
         {
            osheapblock *  block;   // info on block that is still allocated

            block       = allocated;
            allocated   = allocated->next;
            block->next = static_cast<osheapblock *>(OSNULL);

            OsDebugErrorMessage(L"A block of memory is still allocated.\n"
                                L"Description:\n\"",
                                block->ObjectName(),
                                L"\".");

            // We should de-allocate *(block->block) , but we can't because
            // the actual type of *(block->block) is not known.

            // De-allocate <block>.
            {
               osbasicheap<osheapblock>  heap;

               if (heap.HeapDeallocate(block) != OSRESULT_OK)
                  OsDebugErrorMessage(L"Cannot de-allocate a heap block.");
            }
         }

         if (mutex.LeaveSection() != OSRESULT_OK)
            OsDebugErrorMessage(L"Cannot leave a critical section for\n"
                                L"heap administration when cleaning up\n"
                                L"<osrtl><heap><heapadm>.");
      }
      else
         OsDebugErrorMessage(L"Cannot enter a critical section for\n"
                             L"heap administration when cleaning up\n"
                             L"<osrtl><heap><heapadm>.");
   else
      OsDebugErrorMessage(L"Cannot connect to the mutex for "
                          L"heap administration\n"
                          L"when cleaning up <osrtl><heap><heapadm>.");

}  // guard_osrtl_heap_heapadm::~guard_osrtl_heap_heapadm


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/heap/heapadm/heapadm.cpp .                        *
 **************************************************************************/


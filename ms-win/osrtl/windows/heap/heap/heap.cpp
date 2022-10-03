
/**************************************************************************
 * File:     osrtl/windows/heap/heap/heap.cpp                             *
 * Contents: Heap management.                                             *
 * Document: osrtl/windows/heap/heap/heap.d                               *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/heap/heap.cpp  -  Heap management

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
//  1.1.000  First issue of this module source file.
// 19970718
// 19970806
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

// Header file of the current module.
#include "osrtl/windows/heap/heap/heap.h"


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class guard_osrtl_heap_heap:      // guards this module
   private priv_osbasicheapadmin  //    administrator for basic heap manager
{
public:
   // Life cycle.

   virtual
   ~guard_osrtl_heap_heap          // performs exit checks on the module
   ();

};  // guard_osrtl_heap_heap


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

unsigned long                        // total number of allocated blocks
priv_osbasicheapadmin::allocations
= 0;

static
guard_osrtl_heap_heap                // guards this module
guard;


/**************************************************************************
 * Function: guard_osrtl_heap_heap::~guard_osrtl_heap_heap                *
 **************************************************************************/

guard_osrtl_heap_heap::~guard_osrtl_heap_heap   // performs exit checks
()                                              //    on the module

{  // guard_osrtl_heap_heap::~guard_osrtl_heap_heap

   if (Allocations() != 0UL)
      OsDebugErrorMessage(L"Some blocks were allocated\n"
                          L"but never de-allocated.");

}  // guard_osrtl_heap_heap::~guard_osrtl_heap_heap


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/heap/heap/heap.cpp .                              *
 **************************************************************************/


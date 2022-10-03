
/**************************************************************************
 * File:     osrtl/windows/heap/heapblck/heapblck.cpp                     *
 * Contents: Heap blocks.                                                 *
 * Document: osrtl/windows/heap/heapadm/heapblck.d                        *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/heapblck/heapblck.cpp  -  Heap blocks

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:39  erick
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
#include "osrtl/windows/basics/basics.h"   // basic definitions
#include "osrtl/windows/error/error.h"     // error handling

// Header file of the current module.
#include "osrtl/windows/heap/heapblck/heapblck.h"


/**************************************************************************
 * Function: osheapblock::osheapblock                                     *
 **************************************************************************/

osheapblock::osheapblock   // initialises an <osheapblock>
()

{  // osheapblock::osheapblock

   if (!SetName(L"<osheapblock>"))
      OsDebugErrorMessage(L"Cannot set the name of an <osheapblock>.");

   block = static_cast<void const *> (OSNULL);
   next  = static_cast<osheapblock *>(OSNULL);

}  // osheapblock::osheapblock


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/heap/heapblck/heapblck.cpp .                      *
 **************************************************************************/


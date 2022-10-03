
/**************************************************************************
 * File:     osrtl/windows/heap/heap.h                                    *
 * Contents: Heap.                                                        *
 * Document: osrtl/windows/heap/heap.d                                    *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/heap/heap.h  -  Heap

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:31  erick
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
//  1.1.000  First issue of this component header file.
// 19970708
// 19970717
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_HEAP
#define OSRTL_HEAP


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Modules of the component <heap>.
#include "osrtl/windows/heap/heap/heap.h"           // heap management
#include "osrtl/windows/heap/heapblck/heapblck.h"   // heap blocks
#include "osrtl/windows/heap/heapadm/heapadm.h"     // heap administration
#include "osrtl/windows/heap/safeheap/safeheap.h"   // checked heap


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
 * End of osrtl/windows/heap/heap.h .                                     *
 **************************************************************************/


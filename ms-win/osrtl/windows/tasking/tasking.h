
/**************************************************************************
 * File:     osrtl/windows/tasking/tasking.h                              *
 * Contents: Task control.                                                *
 * Document: osrtl/windows/tasking/tasking.d
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/tasking/tasking.h  -  Task control

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:02  erick
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
// 19970710
// 19970716
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_TASKING
#define OSRTL_TASKING


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Modules of the component <tasking>.
#include "osrtl/windows/tasking/mutexid/mutexid.h"   // mutex IDs
#include "osrtl/windows/tasking/physmux/physmux.h"   // physical mutexes
#include "osrtl/windows/tasking/mutex/mutex.h"       // mutex semaphores


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
 * End of osrtl/windows/tasking/tasking.h .                               *
 **************************************************************************/


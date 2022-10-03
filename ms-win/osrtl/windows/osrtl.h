
/**************************************************************************
 * File:     osrtl/osrtl.h                                                *
 * Contents: NIL between program and (OS and RTL).                        *
 * Document: osrtl/osrtl.d                                                *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/osrtl.h  -  NIL between program and (OS and RTL)

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:40:56  erick
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
//  1.1.000  First issue of this unit header file.
// 19970531
// 19970825
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_PCWINDOWS
#define OSRTL_PCWINDOWS


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"       // basic definitions
#include "osrtl/windows/error/error.h"         // error handling
         // (this component is not exported)   // hardware characteristics
#include "osrtl/windows/startup/startup.h"     // start-up
#include "osrtl/windows/tasking/tasking.h"     // task control
#include "osrtl/windows/heap/heap.h"           // heap
#include "osrtl/windows/text/text.h"           // textual data
#include "osrtl/windows/sound/sound.h"         // sounds
#include "osrtl/windows/scrnobj/scrnobj.h"     // screen objects
#include "osrtl/windows/drawing/drawing.h"     // attributes for drawing
#include "osrtl/windows/window/window.h"       // window management


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
 * End of osrtl/osrtl.h .                                                 *
 **************************************************************************/


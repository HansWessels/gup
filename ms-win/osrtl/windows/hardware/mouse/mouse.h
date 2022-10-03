
/**************************************************************************
 * File:     osrtl/windows/hardware/mouse/mouse.h                         *
 * Contents: Mouse hardware.                                              *
 * Document: osrtl/windows/hardware/mouse/mouse.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/hardware/mouse/mouse.h  -  Mouse hardware

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:25  erick
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
// 19970725
// 19970725
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_HARDWARE_MOUSE
#define OSRTL_HARDWARE_MOUSE


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

int            // number of mouse buttons, or 0 if no mouse present
MouseButtons   // determines the number of mouse buttons
();


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
 * End of osrtl/windows/hardware/mouse/mouse.h .                          *
 **************************************************************************/


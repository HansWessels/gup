
/**************************************************************************
 * File:     osrtl/windows/hardware/screen/screen.h                       *
 * Contents: Screen characteristics.                                      *
 * Document: osrtl/windows/hardware/screen/screen.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/hardware/screen/screen.h  -  Screen characteristics

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:28  erick
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
// 19970714
// 19970801
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_HARDWARE_SCREEN
#define OSRTL_HARDWARE_SCREEN


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

int                  // screen width, in pixels
ScreenWidthPixels    // obtains the width of the screen, in pixels
();

int                  // screen height, in pixels
ScreenHeightPixels   // obtains the height of the screen, in pixels
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
 * End of osrtl/windows/hardware/screen/screen.h .                        *
 **************************************************************************/


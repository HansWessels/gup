
/**************************************************************************
 * File:     osrtl/windows/hardware/screen/screen.cpp                     *
 * Contents: Screen characteristics.                                      *
 * Document: osrtl/windows/hardware/screen/screen.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/hardware/screen/screen.cpp  -  Screen characteristics

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:27  erick
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
// 19970719
// 19970801
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// The unit <osrtl> is the only unit that is allowed to include files that
// have to do with the operating system and the RTL.
#define STRICT
#include <windows.h>   // header file of the OS

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Header file of the current module.
#include "osrtl/windows/hardware/screen/screen.h"


/**************************************************************************
 * Function: ScreenWidthPixels                                            *
 **************************************************************************/

int                 // screen width, in pixels
ScreenWidthPixels   // obtains the width of the screen, in pixels
()

{  // ScreenWidthPixels

   return GetSystemMetrics(SM_CXSCREEN);

}  // ScreenWidthPixels


/**************************************************************************
 * Function: ScreenHeightPixels                                           *
 **************************************************************************/

int                  // screen height, in pixels
ScreenHeightPixels   // obtains the height of the screen, in pixels
()

{  // ScreenHeightPixels

   return GetSystemMetrics(SM_CYSCREEN);

}  // ScreenHeightPixels


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/hardware/screen/screen.cpp .                      *
 **************************************************************************/


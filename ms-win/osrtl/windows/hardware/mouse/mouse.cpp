
/**************************************************************************
 * File:     osrtl/windows/hardware/mouse/mouse.cpp                       *
 * Contents: Mouse hardware.                                              *
 * Document: osrtl/windows/hardware/mouse/mouse.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/hardware/mouse/mouse.cpp  -  Mouse hardware

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:24  erick
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
// 19970725
// 19970725
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
#include "osrtl/windows/hardware/mouse/mouse.h"


/**************************************************************************
 * Function: MouseButtons                                                 *
 **************************************************************************/

int            // number of mouse buttons, or 0 if no mouse present
MouseButtons   // determines the number of mouse buttons
()

{  // MouseButtons

   int  buttons;   // number of mouse buttons, or 0 if no mouse present

   if (GetSystemMetrics(SM_MOUSEPRESENT) != 0)
   {
      // Mouse present.  Determine the number of buttons.
      buttons = GetSystemMetrics(SM_CMOUSEBUTTONS);
      if (buttons == 0)
         // A mouse is present, but it has no buttons...?  This actually
         // happens on one of our development systems!
         // The best we can do is assume an ordinary two-button mouse.
         buttons = 2;
   }
   else
      // No mouse present.
      buttons = 0;

   return buttons;

}  // MouseButtons


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/hardware/mouse/mouse.cpp .                        *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/scrnobj/mouse/mouse.cpp                        *
 * Contents: Mouse cursors.                                               *
 * Document: osrtl/windows/scrnobj/mouse/mouse.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/scrnobj/mouse/mouse.cpp  -  Mouse cursors

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:46  erick
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
// 19970818
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

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"       // basic definitions
#include "osrtl/windows/error/error.h"         // error handling
#include "osrtl/windows/hardware/hardware.h"   // hardware characteristics
#include "osrtl/windows/startup/startup.h"     // start-up

// Header file of the current module.
#include "osrtl/windows/scrnobj/mouse/mouse.h"


/**************************************************************************
 * Function: SelectMouseCursor                                            *
 **************************************************************************/

osresult                  // result code
SelectMouseCursor         // selects a mouse cursor
(mousecursortype  type)   // I: type of the mouse cursor to select

{  // SelectMouseCursor

   osresult  result = OSRESULT_OK;   // result code

   // Do nothing if no mouse is present.
   if (MouseButtons() > 0)
   {
      osbool   systemcursor = OSTRUE;   // use one of the system cursors?
      LPCTSTR  cursorid;                // ID of the mouse cursor

      switch (type)
      {
         case MOUSE_ARROW:
            cursorid = IDC_ARROW;
            break;

         case MOUSE_BUSY:
            cursorid = IDC_WAIT;
            break;

         default:
            result = OSINTERNAL_MOUSECURSOR;
            break;
      }

      if (result == OSRESULT_OK)
      {
         HCURSOR  newcursor;   // new mouse cursor

         newcursor = LoadCursorA(systemcursor
                                      ? static_cast<HINSTANCE>(OSNULL)
                                      : ProgramInstance(),
                                cursorid);
         if (newcursor != static_cast<HCURSOR>(OSNULL))
            SetCursor(newcursor);
         else
            result = systemcursor ? OSERROR_NORSRC_MCURSOR
                                  : OSINTERNAL_MOUSECURSOR;
      }

      // If not successful, try to set a default arrow cursor.
      if (result != OSRESULT_OK)
         SetCursor(LoadCursor(static_cast<HINSTANCE>(OSNULL), IDC_ARROW));
   }

   return result;

}  // SelectMouseCursor


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/scrnobj/mouse/mouse.cpp .                         *
 **************************************************************************/


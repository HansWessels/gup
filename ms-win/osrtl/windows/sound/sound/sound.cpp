
/**************************************************************************
 * File:     osrtl/windows/sound/sound/sound.cpp                          *
 * Contents: Sounds.                                                      *
 * Document: osrtl/windows/sound/sound/sound.d                            *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/sound/sound/sound.cpp  -  Sounds

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:53  erick
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
// 19970720
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

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"    // basic definitions

// Header file of the current module.
#include "osrtl/windows/sound/sound/sound.h"


/**************************************************************************
 * Function: OsSound                                                      *
 **************************************************************************/

osbool            // sound could be played?
OsSound           // plays a sound
(ossound  type,   // I: type of sound to play
 int      n)      // I: number of times to play the sound

{  // OsSound

   UINT    soundtype;       // type of sound to play
   osbool  play = OSTRUE;   // sound could be played?

   switch (type)
   {
      case OSSOUND_ALERT:
         // In the Control Panel of Windows 3.1, this is called "Asterisk".
         soundtype = static_cast<UINT>(MB_ICONASTERISK);
         break;

      case OSSOUND_QUESTION:
         // In the Control Panel of Windows 3.1, this is called "Question".
         soundtype = static_cast<UINT>(MB_ICONQUESTION);
         break;

      case OSSOUND_PROBLEM:
         // In the Control Panel of Windows 3.1, this is called
         // "Critical Stop".
         soundtype = static_cast<UINT>(MB_ICONHAND);
         break;

      default:
         play = OSFALSE;
         break;
   }

   while (play  &&  n-- > 0)
      if (!MessageBeep(soundtype))
         play = OSFALSE;

   return play;

}  // OsSound


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/sound/sound/sound.cpp .                           *
 **************************************************************************/


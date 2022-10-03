
/**************************************************************************
 * File:     osrtl/windows/sound/sound/sound.h                            *
 * Contents: Sounds.                                                      *
 * Document: osrtl/windows/sound/sound/sound.d                            *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/sound/sound/sound.h  -  Sounds

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:54  erick
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
// 19970720
// 19970725
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_SOUND_SOUND
#define OSRTL_SOUND_SOUND


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

enum
ossound                // sound type
{
   OSSOUND_ALERT,      // alert user: a nominal (expected) event occurred
   OSSOUND_QUESTION,   // user must answer a question
   OSSOUND_PROBLEM     // notify user of a problem
};  // ossound


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

osbool              // sound could be played?
OsSound             // plays a sound
(ossound  type,     // I: type of sound to play
 int      n = 1);   // I: number of times to play the sound


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
 * End of osrtl/windows/sound/sound/sound.h .                             *
 **************************************************************************/


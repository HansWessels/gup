
/**************************************************************************
 * File:     osrtl/windows/scrnobj/mouse/mouse.h                          *
 * Contents: Mouse cursors.                                               *
 * Document: osrtl/windows/scrnobj/mouse/mouse.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/scrnobj/mouse/mouse.h  -  Mouse cursors

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:47  erick
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
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_SCRNOBJ_MOUSE
#define OSRTL_SCRNOBJ_MOUSE


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

enum
mousecursortype          // type of mouse cursor
{
   MOUSE_ARROW,          // standard mouse arrow
   MOUSE_BUSY            // "system is busy"
};  // mousecursortype


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

osresult                   // result code
SelectMouseCursor          // selects a mouse cursor
(mousecursortype  type);   // I: type of the mouse cursor to select


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
 * End of osrtl/windows/scrnobj/mouse/mouse.h .                           *
 **************************************************************************/


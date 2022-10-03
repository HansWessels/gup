
/**************************************************************************
 * File:     shell/windows/startup/entry/entry.h                          *
 * Contents: Program entry.                                               *
 * Document: -                                                            *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) shell/windows/startup/entry/entry.h  -  Program entry

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:55  erick
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
// 19970716
// 19970727
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef SHELL_STARTUP_ENTRY
#define SHELL_STARTUP_ENTRY


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

int                     // status code
ApplicationInitialise   // initialises the program
();

int                     // status code
ApplicationCloseDown    // closes down the program
();

wchar_t const *         // name of the application
ApplicationName         // obtains the name of the application
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
 * End of shell/windows/startup/entry/entry.h .                           *
 **************************************************************************/


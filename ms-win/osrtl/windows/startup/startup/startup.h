
/**************************************************************************
 * File:     osrtl/windows/startup/startup/startup.h                      *
 * Contents: Program start-up.                                            *
 * Document: osrtl/windows/startup/startup/startup.d                      *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/startup/startup/startup.h  -  Program start-up

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:59  erick
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
//  1.1.000  First issue of this header file.
// 19970609
// 19970727
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_STARTUP_STARTUP
#define OSRTL_STARTUP_STARTUP


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

typedef  void *  programinstance;   // program instance


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

wchar_t const *   // name of the program
ProgramName       // obtains the name of the program
();

programinstance   // program instance
ProgramInstance   // obtains the instance of the application
();

void
WindowExists      // tells <osrtl><startup><startup> whether windows exist
(osbool  exist);  // I: whether application windows exist


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
 * End of osrtl/windows/startup/startup/startup.h .                       *
 **************************************************************************/


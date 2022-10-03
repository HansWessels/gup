
/**************************************************************************
 * File:     global/settings.h                                            *
 * Contents: Global settings for GNU Pack.                                *
 * Document: global/settings.d                                            *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) pack.h  -  Global settings for GNU Pack

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:40:50  erick
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
// 19970629
// 19970820
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef GLOBAL_SETTINGS
#define GLOBAL_SETTINGS


/**************************************************************************
 * Enforcing a compilation mode.                                          *
 **************************************************************************/

// To enforce compilation in debugging mode, the next lines must read
//    #define FORCE_DEBUGGINGMODE
//    #undef  FORCE_RELEASEMODE
// To enforce compilation in release mode, the next lines must read
//    #undef  FORCE_DEBUGGINGMODE
//    #define FORCE_RELEASEMODE
// To allow each unit to determine its own compilation mode, the next lines
// must read
//    #undef FORCE_DEBUGGINGMODE
//    #undef FORCE_RELEASEMODE

#undef FORCE_DEBUGGINGMODE
#undef FORCE_RELEASEMODE


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
 * End of global/settings.h .                                             *
 **************************************************************************/


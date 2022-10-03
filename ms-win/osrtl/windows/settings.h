
/**************************************************************************
 * File:     osrtl/windows/settings.h                                     *
 * Contents: Global settings for unit <osrtl>.                            *
 * Document: osrtl/windows/settings.d                                     *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/settings.h  -  Global settings for unit <osrtl>

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:40:57  erick
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
// 19970820
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_SETTINGS
#define OSRTL_SETTINGS


/**************************************************************************
 * Global settings.                                                       *
 **************************************************************************/

// Compilation mode: debugging or release.

#ifdef FORCE_DEBUGGINGMODE
   #ifdef FORCE_RELEASEMODE
      #error Both debugging mode and release mode are specified.
   #else
      #define OSRTL_DEBUGGING
   #endif
#else
   #ifdef FORCE_RELEASEMODE
      #undef OSRTL_DEBUGGING
   #else
      // Neither debugging mode nor release mode are enforced.  The unit
      // <osrtl> must decide for itself in which mode to compile.
      // To compile in debugging mode, the next statement must read
      //    #define OSRTL_DEBUGGING
      // To compile in release mode, the next statement must read
      //    #undef OSRTL_DEBUGGING
      #define OSRTL_DEBUGGING
   #endif
#endif


// The character set used by the operating system: UNICODE or ANSI.
// Define or undefine the symbol <OSRTL_UNICODE> here.

#undef OSRTL_UNICODE


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
 * End of osrtl/windows/settings.h .                                      *
 **************************************************************************/


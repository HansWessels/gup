
/**************************************************************************
 * File:     osrtl/windows/tasking/mutexid/mutexid.h                      *
 * Contents: Mutex IDs.                                                   *
 * Document: osrtl/windows/tasking/mutexid/mutexid.d                      *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/tasking/mutexid/mutexid.h  -  Mutex IDs

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:08  erick
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
// 19970813
// EAS
// -------------------------------------------------------------------------
//  1.1.001  <mutexid> is no longer an <enum>; now, it is a set of
// 19970823  predefined constants, and a new mutex ID <MUTEX_APPLICATION>
// 19970823  has been added.  This allows applications built on top of
// EAS       <osrtl> to define their own mutex IDs.
// -------------------------------------------------------------------------
//  1.2.000  Identical to revision 1.1.001.  Formalised revision.
// 19970826
// 19970826
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_TASKING_MUTEXID
#define OSRTL_TASKING_MUTEXID


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

typedef  unsigned long  mutexid;


/**************************************************************************
 * Exported constants.                                                    *
 **************************************************************************/

const  mutexid  MUTEX_MUTEXADMIN   = 0UL;    // mutex administration
const  mutexid  MUTEX_HEAPADMIN    = 1UL;    // heap administration
const  mutexid  MUTEX_FONTADMIN    = 2UL;    // font administration
const  mutexid  MUTEX_FONTMANAGE   = 3UL;    // font management
const  mutexid  MUTEX_WINDOWADMIN  = 4UL;    // window administration
const  mutexid  MUTEX_WINDOWMANAGE = 5UL;    // window management
const  mutexid  MUTEX_SAFEWINDOW   = 6UL;    // management of safe windows

const  mutexid  MUTEX_APPLICATION  = 1000UL;   // first mutex ID for use by
                                               //    the application


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
 * End of osrtl/windows/tasking/mutexid/mutexid.h .                       *
 **************************************************************************/


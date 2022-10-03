
/**************************************************************************
 * File:     osrtl/osrtl.h                                                *
 * Contents: NIL between program and (OS and RTL).                        *
 * Document: osrtl/osrtl.d                                                *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/osrtl.h  -  NIL between program and (OS and RTL)

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:40:53  erick
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
//  1.1.000  First issue of this unit header file.
// 19970531
// 19970825
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL
#define OSRTL


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings for the project.
#include "global/settings.h"

// The target platform for which <osrtl> is to be compiled.
#include "osrtl/windows/osrtl.h"   // header file for PC-windows


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
 * End of osrtl/osrtl.h .                                                 *
 **************************************************************************/


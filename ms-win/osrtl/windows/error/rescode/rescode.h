
/**************************************************************************
 * File:     osrtl/windows/error/rescode/rescode.h                        *
 * Contents: Result codes.                                                *
 * Document: osrtl/windows/error/rescode/rescode.d                        *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/error/rescode/rescode.h  -  Result codes

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:20  erick
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
// 19970615
// 19970807
// EAS
// -------------------------------------------------------------------------
//  1.1.001  The integer values of the result codes have been defined such
// 19970822  that new result codes can be inserted without changing the
// 19970822  integer values of the existing result codes.  Furthermore,
// EAS       marker values have been added.
// -------------------------------------------------------------------------
//  1.2.000  Identical to revision 1.1.001.  Formalised revision.
// 19970826
// 19970826
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_ERROR_RESCODE
#define OSRTL_ERROR_RESCODE


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

enum
osresult                     // result code
{
   // OK code.
   OSRESULT_OK,              // no error

   // Errors.
   OSERROR_FIRST = 1000,     // (marker: first error code)
   OSERROR_VALUEOUTOFRANGE,  // numeric value out of range
   OSERROR_NOCHARCONVERT,    // cannot convert char between UNICODE and ANSI
   OSERROR_NOMEMORY,         // not enough memory
   OSERROR_NORSRC_MCURSOR,   // not enough resources to set a mouse cursor
   OSERROR_NORSRC_FONT,      // not enough resources to create a font
   OSERROR_NORSRC_WINDOW,    // not enough resources to create a window
   OSERROR_NORSRC_DRAW,      // not enough resources to draw to a window
   OSERROR_NORSRC_WRITE,     // not enough resources to write text to window
   OSERROR_NODEFAULTFONT,    // no suitable default font present
   OSERROR_LAST,             // (marker: last error code)

   // Internal errors.
   OSINTERNAL_FIRST = 2000,  // (marker: first internal error code)
   OSINTERNAL_MUTEXADMIN,    // mutex administration
   OSINTERNAL_HEAPADMIN,     // heap administration
   OSINTERNAL_MOUSECURSOR,   // setting the mouse cursor
   OSINTERNAL_FONTADMIN,     // font administration
   OSINTERNAL_WINDOWADMIN,   // window administration
   OSINTERNAL_DRAW,          // drawing to a window
   OSINTERNAL_WRITE,         // writing text to a window
   OSINTERNAL_LAST,          // (marker: last internal error code)

   OSRESULT_LAST             // (marker: last result code)

};  // osresult


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
 * End of osrtl/windows/error/rescode/rescode.h .                         *
 **************************************************************************/


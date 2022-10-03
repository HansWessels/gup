
/**************************************************************************
 * File:     osrtl/windows/basics/convert/convert.h                       *
 * Contents: Type conversions.                                            *
 * Document: osrtl/windows/basics/convert/convert.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/basics/convert/convert.h  -  Type conversions

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:05  erick
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
// 19970714
// 19970724
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_BASICS_CONVERT
#define OSRTL_BASICS_CONVERT


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

osbool                  // value can be converted?
RoundFloatToIntegral    // converts a <float> to an <int>, rounding it
(float  f,              // I: value to convert
 int *  i);             // O: converted value

osbool                  // value can be converted?
RoundFloatToIntegral    // converts a <float> to a <long>, rounding it
(float   f,             // I: value to convert
 long *  i);            // O: converted value

osbool                  // character can be converted?
CharConvert             // converts an ANSI character to a UNICODE character
(char       a,          // I: ANSI character
 wchar_t *  u);         // O: UNICODE character

osbool                  // character can be converted?
CharConvert             // converts a UNICODE character to an ANSI character
(wchar_t  u,            // I: UNICODE character
 char *   a);           // O: ANSI character

osbool                  // string can be converted?
StringConvert           // converts an ANSI string to a UNICODE string
(char const *  a,       // I: ANSI string
 wchar_t *     u);      // O: UNICODE string

osbool                  // string can be converted?
StringConvert           // converts a UNICODE string to an ANSI string
(wchar_t const *  u,    // I: UNICODE string
 char *           a);   // O: ANSI string


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
 * End of osrtl/windows/basics/convert/convert.h .                        *
 **************************************************************************/


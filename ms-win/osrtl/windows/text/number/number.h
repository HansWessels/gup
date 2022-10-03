
/**************************************************************************
 * File:     osrtl/windows/text/number/number.h                           *
 * Contents: Converts string <-> number.                                  *
 * Document: osrtl/windows/text/number/number.d                           *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/text/number/number.h  -  Converts string <-> number

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:18  erick
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
// 19970711
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_TEXT_NUMBER
#define OSRTL_TEXT_NUMBER


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

unsigned long             // length of string
StringLength              // finds length of string representation of number
(int  value);             // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(unsigned int  value);    // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(long  value);            // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(unsigned long  value);   // I: number

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(int        value,        // I: number to convert
 wchar_t *  s);           // O: resulting string

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(unsigned int  value,     // I: number to convert
 wchar_t *     s);        // O: resulting string

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(long       value,        // I: number to convert
 wchar_t *  s);           // O: resulting string

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(unsigned long  value,    // I: number to convert
 wchar_t *      s);       // O: resulting string


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
 * End of osrtl/windows/text/number/number.h .                            *
 **************************************************************************/


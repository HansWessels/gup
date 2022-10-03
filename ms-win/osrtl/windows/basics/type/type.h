
/**************************************************************************
 * File:     osrtl/windows/basics/type/type.h                             *
 * Contents: Types.                                                       *
 * Document: osrtl/windows/basics/type/type.d                             *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/basics/type/type.h  -  Types

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:11  erick
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
// 19970727
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_BASICS_TYPE
#define OSRTL_BASICS_TYPE


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

// Two's complement integral types.    S   #bits    range
// S==Signed, U==Unsigned              U  (exact)  (exact)
//--------------------------------------------------------------------------
typedef    signed char   int8;    //   S      8     -128..+127
typedef  unsigned char   uint8;   //   U      8     0..255
typedef    signed short  int16;   //   S     16     -32768..+32767
typedef  unsigned short  uint16;  //   U     16     0..65535
typedef    signed int    int32;   //   S     32     -2147483648..+2147483647
typedef  unsigned int    uint32;  //   U     32     0..4294967295

enum
osbool                                   // boolean type
{
   OSFALSE = static_cast<int>(0 == 1),   // false, not true, invalid, no
   OSTRUE  = static_cast<int>(0 == 0)    // true, valid, yes
};  // osbool


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
 * End of osrtl/windows/basics/type/type.h .                              *
 **************************************************************************/


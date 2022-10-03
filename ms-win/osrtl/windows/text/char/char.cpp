
/**************************************************************************
 * File:     osrtl/windows/text/char/char.cpp                             *
 * Contents: Characters.                                                  *
 * Document: osrtl/windows/text/char/char.d                               *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/text/char/char.cpp  -  Characters

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:15  erick
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
//  1.1.000  First issue of this module source file.
// 19970622
// 19970718
// EAS
// -------------------------------------------------------------------------
//  1.1.001  <IsCharWhiteSpace> no longer converts its argument to an ANSI
// 19970822  character before checking if the character is a whitespace
// 19970822  character.
// EAS
// -------------------------------------------------------------------------
//  1.2.000  Identical to revision 1.1.001.  Formalised revision.
// 19970826
// 19970826
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other components of the unit <osrtl>
#include "osrtl/windows/basics/basics.h"   // basic definitions

// Header file of the current module.
#include "osrtl/windows/text/char/char.h"


/**************************************************************************
 * Function: IsCharWhiteSpace                                             *
 **************************************************************************/

osbool             // whether the character is whitespace
IsCharWhiteSpace   // checks if a character is whitespace
(wchar_t  ch)      // I: character to check

{  // IsCharWhiteSpace

   return static_cast<osbool>(    ch == L' '
                              ||  ch == L'\t'
                              ||  ch == L'\n'
                              ||  ch == L'\r'
                              ||  ch == L'\v'
                              ||  ch == L'\b'
                              ||  ch == L'\f');

}  // IsCharWhiteSpace


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/text/char/char.cpp .                              *
 **************************************************************************/


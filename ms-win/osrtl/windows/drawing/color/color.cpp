
/**************************************************************************
 * File:     osrtl/windows/drawing/color/color.cpp                        *
 * Contents: Colors.                                                      *
 * Document: osrtl/windows/drawing/color/color.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/drawing/color/color.cpp  -  Colors

// $RCSfile$
// $Author: erick $
// $Revision: 85 $
// $Date: 1997-12-30 14:49:19 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:49:14  erick
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
// 19970722
// 19970806
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings for the project.     
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"    // basic definitions
#include "osrtl/windows/error/error.h"      // error handling

// Header file of the current module.
#include "osrtl/windows/drawing/color/color.h"


/**************************************************************************
 * Function: oscolor::oscolor                                             *
 **************************************************************************/

oscolor::oscolor     // initialises an <oscolor>
(float  red,         // I: amount of red   (0..1)
 float  green,       // I: amount of green (0..1)
 float  blue)        // I: amout of blue   (0..1)

{  // oscolor::oscolor

   if (!SetName(L"<oscolor>"))
      OsDebugErrorMessage(L"Cannot set the name of a <oscolor>.");

   Define(red, green, blue);

}  // oscolor::oscolor


/**************************************************************************
 * Function: oscolor::oscolor                                             *
 **************************************************************************/

oscolor::oscolor         // initialises an <oscolor>
(oscolor const &  src)   // I: source for the <oscolor>

{  // oscolor::oscolor

   if (!SetName(src.ObjectName()))
      OsDebugErrorMessage(L"Cannot set the name of a <oscolor>.");

   Define(src.r, src.g, src.b);

}  // oscolor::oscolor


/**************************************************************************
 * Function: oscolor::operator =                                          *
 **************************************************************************/

oscolor const &          // copy of the source <oscolor>
oscolor::operator =      // copies a source color
(oscolor const &  src)   // I: source color

{  // oscolor::operator =

   if (!SetName(src.ObjectName()))
      OsDebugErrorMessage(L"Cannot set the name of a <oscolor>.");

   Define(src.r, src.g, src.b);

   return *this;

}  // oscolor::operator =


/**************************************************************************
 * Function: oscolor::Define                                              *
 **************************************************************************/

void
oscolor::Define   // defines an <oscolor>
(float  red,      // I: amount of red   (0..1)
 float  green,    // I: amount of green (0..1)
 float  blue)     // I: amout of blue   (0..1)

{  // oscolor::Define

   float  max = 1.0;   // highest of the three input values

   if (red < 0.0)
      red = -red;
   if ((r = red) > max)
      max = r;

   if (green < 0.0)
      green = -green;
   if ((g = green) > max)
      max = g;

   if (blue < 0.0)
      blue = -blue;
   if ((b = blue) > max)
      max = b;

   // Normalise the color by mapping the range 0..max to 0..1 .
   {
      r /= max;
      g /= max;
      b /= max;
   }

}  // oscolor::Define


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/drawing/color/color.cpp .                         *
 **************************************************************************/


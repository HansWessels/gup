
/**************************************************************************
 * File:     osrtl/windows/drawing/color/color.h                          *
 * Contents: Colors.                                                      *
 * Document: osrtl/windows/drawing/color/color.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/drawing/color/color.h  -  Colors

// $RCSfile$
// $Author: erick $
// $Revision: 85 $
// $Date: 1997-12-30 14:49:19 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:49:15  erick
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
// 19970722
// 19970806
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_DRAWING_COLOR
#define OSRTL_DRAWING_COLOR


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class oscolor:               // color
   public osobject           //    generic object
{
public:
   // Life cycle.

   oscolor                   // initialises an <oscolor>
   (float  red,              // I: amount of red   (0..1)
    float  green,            // I: amount of green (0..1)
    float  blue);            // I: amout of blue   (0..1)

   oscolor                   // initialises an <oscolor>
   (oscolor const &  src);   // I: source for the <oscolor>

   oscolor const &           // copy of the source <oscolor>
   operator =                // copies a source color
   (oscolor const &  src);   // I: source color

   // Actual functionality.

   void
   Define           // defines an <oscolor>
   (float  red,     // I: amount of red   (0..1)
    float  green,   // I: amount of green (0..1)
    float  blue);   // I: amout of blue   (0..1)

   inline
   float            // amount of red (0..1)
   R                // obtains the amount of red in the color
   ()
   const;

   inline
   float            // amount of green (0..1)
   G                // obtains the amount of green in the color
   ()
   const;

   inline
   float            // amount of blue (0..1)
   B                // obtains the amount of blue in the color
   ()
   const;

private:
   // Protective functions.

   oscolor     // dummy private default constructor
   ()          //    prevents default construction
   {}

   // Data.

   float  r;    // amount of red   in the color
   float  g;    // amount of green in the color
   float  b;    // amount of blue  in the color

};  // color


/**************************************************************************
 * Function: oscolor::R                                                   *
 **************************************************************************/

inline
float        // amount of red (0..1)
oscolor::R   // obtains the amount of red in the color
()
const

{  // oscolor::R

   return r;

}  // oscolor::R


/**************************************************************************
 * Function: oscolor::G                                                   *
 **************************************************************************/

inline
float        // amount of green (0..1)
oscolor::G   // obtains the amount of green in the color
()
const

{  // oscolor::G

   return g;

}  // oscolor::G


/**************************************************************************
 * Function: oscolor::B                                                   *
 **************************************************************************/

inline
float        // amount of blue (0..1)
oscolor::B   // obtains the amount of blue in the color
()
const

{  // oscolor::B

   return b;

}  // oscolor::B


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
 * End of osrtl/windows/drawing/color/color.h .                           *
 **************************************************************************/


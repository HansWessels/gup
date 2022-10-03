
/**************************************************************************
 * File:     osrtl/windows/window/winfont/winfont.h                       *
 * Contents: Window font.                                                 *
 * Document: osrtl/windows/window/winfont/winfont.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/winfont/winfont.h  -  Window font

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:42  erick
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
// 19970801
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_WINDOW_WINFONT
#define OSRTL_WINDOW_WINFONT


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class oswindowfont:               // window font
   public osfont                  //    generic font
{
public:
   // Life cycle.

   oswindowfont                   // initialises an <oswindowfont>
   ();

   osresult                       // result code
   Create                         // creates a window font
   (wchar_t const *    name,      // I: name of the font
    float              vsize,     // I: vertical size of font, screen rel.
    float              hsize,     // I: horizontal size of font; 1.0==normal
    ospainter const &  painter);  // I: window painter

   inline
   osresult                       // result code
   Destroy                        // destroys a window font
   ();

   // Obtaining information.

   inline
   float                          // height of the font, screen relative
   Height                         // obtains the actual height of the font
   ()
   const;

   inline
   float                          // horizontal size of font; 1.0==normal
   HorizontalSize                 // obtains the horizontal size of the font
   ()
   const;

   osresult                       // result code
   TextWidth                      // obtains widths of text string, scrn rel
   (wchar_t const *    text,      // I: text
    ospainter const &  painter,   // I: window painter
    float &            left,      // O: width of left margin
    float &            middle,    // O: width of text without margins
    float &            full)      // O: width of text including both margins
   const;

   osresult                       // result code
   TextWidthIdeal                 // obtains widths of text string, scrn rel
   (wchar_t const *    text,      // I: text
    ospainter const &  painter,   // I: window painter
    float &            left,      // O: width of left margin
    float &            middle,    // O: width of text without margins
    float &            full)      // O: width of text including both margins
   const;

private:
   // Implementation of the functionality.

   osresult                       // result code
   TextWidth                      // obtains widths of text string, scrn rel
   (wchar_t const *    text,      // I: text
    ospainter const &  painter,   // I: window painter
    osfont const &     font,      // I: font to use for writing the string
    float &            left,      // O: width of left margin
    float &            middle,    // O: width of text without margins
    float &            full)      // O: width of text including both margins
   const;

   // Types.

   typedef  osfont  inherited;    // base class

   // Data.

   float   horsize;   // commanded horizontal size of the font; 1.0==normal
   float   vertsize;  // commanded vertical size of the font, screen rel.
   float   above;     // actual height of the font, screen relative
   osbool  exists;    // whether the <oswindowfont> has been created

};  // oswindowfont


/**************************************************************************
 * Function: oswindowfont::Destroy                                        *
 **************************************************************************/

inline
osresult                // result code
oswindowfont::Destroy   // destroys a window font
()

{  // oswindowfont::Destroy

   exists   = OSFALSE;
   horsize  = 0.0;
   vertsize = 0.0;
   above    = 0.0;
   return inherited::Destroy();

}  // oswindowfont::Destroy


/**************************************************************************
 * Function: oswindowfont::Height                                         *
 **************************************************************************/

inline
float                  // height of the font, screen relative
oswindowfont::Height   // obtains the actual height of the font
()
const

{  // oswindowfont::Height

   return above;

}  // oswindowfont::Height


/**************************************************************************
 * Function: oswindowfont::HorizontalSize                                 *
 **************************************************************************/

inline
float                          // horizontal size of font; 1.0==normal
oswindowfont::HorizontalSize   // obtains the horizontal size of the font
()
const

{  // oswindowfont::HorizontalSize

   return horsize;

}  // oswindowfont::HorizontalSize


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
 * End of osrtl/windows/window/winfont/winfont.h .                        *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/drawing/font/font.h                            *
 * Contents: Fonts.                                                       *
 * Document: osrtl/windows/drawing/font/font.d                            *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/drawing/font/font.h  -  Fonts

// $RCSfile$
// $Author: erick $
// $Revision: 85 $
// $Date: 1997-12-30 14:49:19 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:49:19  erick
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
// 19970729
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_DRAWING_FONT
#define OSRTL_DRAWING_FONT


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class osfont:        // font
   public osobject   //    generic object
{
public:
   // Types.

   typedef  void *  osfonthandle;   // handle of the font

   // Life cycle.

   osfont                        // initialises an <osfont>
   ();

   ~osfont                       // closes down an <osfont>
   ();

   osresult                      // result code
   Create                        // creates a font
   (wchar_t const *  name,       // I: name of the font
    float            cheight);   // I: character height, screen relative

   osresult                      // result code
   Create                        // creates a font
   (wchar_t const *  name,       // I: name of the font
    float            cheight,    // I: character height,   screen relative
    float            width);     // I: average char width, screen relative

   osresult                      // result code
   Destroy                       // destroys a font
   ();

   // Transferring font information.

   osresult              // result code
   Transfer              // transfers font information
   (osfont &  target);   // O: target for the transfer

   // Obtaining information.

   inline
   float              // commanded height of the font, screen relative
   Height             // obtains the commanded height of the font
   ()
   const;

   inline
   float              // commanded width of the font, screen relative
   Width              // obtains the commanded width of the font
   ()
   const;

   inline
   osbool             // whether one of the sizes of the font is zero
   Zero               // checks if one of the sizes of the font is zero
   ()
   const;

   inline
   wchar_t const *    // name of the font
   FontName           // obtains the name of the font
   ()
   const;

   inline
   osfonthandle       // handle of the <osfont>
   Handle             // obtains the handle of the <osfont>
   ()
   const;

private:
   // Protective functions.

   osfont             // dummy private copy constructor
   (osfont const &)   //    prevents copying
   {};

   osfont const &     // dummy private assignment operator
   operator =         //    prevents assignment
   (osfont const &)
   const
   { return *this; }

   // Implementation of the functionality.

   osresult                      // result code
   Create                        // creates a font
   (wchar_t const *  name,       // I: name of the font
    int              vsize,      // I: character height,        pixels
    int              hsize);     // I: average character width, pixels

   // Data.

   osfonthandle     handle;      // handle of the font
   wchar_t const *  fname;       // name of the font
   osbool           zero;        // font size is zero?
   int              fcheight;    // character height
   int              fwidth;      // average character width
   float            cmdheight;   // commanded character height, scrn rel.
   float            cmdwidth;    // commanded character width,  scrn rel.

};  // osfont


/**************************************************************************
 * Function: osfont::Height                                               *
 **************************************************************************/

inline
float            // commanded height of the font, screen relative
osfont::Height   // obtains the commanded height of the font
()
const

{  // osfont::Height

   return cmdheight;

}  // osfont::Height


/**************************************************************************
 * Function: osfont::Width                                                *
 **************************************************************************/

inline
float           // commanded width of the font, screen relative
osfont::Width   // obtains the commanded width of the font
()
const

{  // osfont::Width

   return cmdwidth;

}  // osfont::Width


/**************************************************************************
 * Function: osfont::Zero                                                 *
 **************************************************************************/

inline
osbool         // whether one of the sizes of the font is zero
osfont::Zero   // checks if one of the sizes of the font is zero
()
const

{  // osfont::Zero

   return zero;

}  // osfont::Zero


/**************************************************************************
 * Function: osfont::FontName                                             *
 **************************************************************************/

inline
wchar_t const *    // name of the font
osfont::FontName   // obtains the name of the font
()
const

{  // osfont::FontName

   return fname;

}  // osfont::FontName


/**************************************************************************
 * Function: osfont::Handle                                               *
 **************************************************************************/

inline
osfont::osfonthandle   // handle of the <osfont>
osfont::Handle         // obtains the handle of the <osfont>
()
const

{  // osfont::Handle

   return handle;

}  // osfont::Handle


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
 * End of osrtl/windows/drawing/font/font.h .                             *
 **************************************************************************/


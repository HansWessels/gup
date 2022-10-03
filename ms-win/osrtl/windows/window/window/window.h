
/**************************************************************************
 * File:     osrtl/windows/window/window/window.h                         *
 * Contents: Basic window.                                                *
 * Document: osrtl/windows/window/window/window.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/window/window.h  -  Basic window

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:38  erick
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
// 19970719
// 19970824
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_WINDOW_WINDOW
#define OSRTL_WINDOW_WINDOW


/**************************************************************************
 * Co-ordinate systems.                                                   *
 **************************************************************************/

// The window system uses a number of different co-ordinate systems.  This
// section explains these co-ordinate systems.
//
// AS (Available Screen)
// This co-ordinate system covers the area of the screen that is available
// to a window when the borders of the window are within the boundaries of
// the screen.
// x: From 0.0 (inclusive) to 1.0 (exclusive), left to right.
// y: From 0.0 (inclusive) to 1.0 (exclusive), top to bottom.
// Example: y==0.0 is the highest possible position of the top of the usable
//          part of a window (the so-called window area).  This position is
//          identical to the top of the screen only if the height of the top
//          border is zero.
// Example: x==1.0 is the left edge of the right border of a window if the
//          right edge of that border touches the right edge of the screen.
//          x==1.0 is identical to a position just right of the right edge
//          of the screen only if the width of the right border is zero.
//
// FS (Full Screen)
// This co-ordinate system covers the entire screen.
// x: From 0.0 (inclusive) to 1.0 (exclusive), left to right.
// y: From 0.0 (inclusive) to 1.0 (exclusive), top to bottom.
// Example: (x, y) == (0.0, 0.0) is the top left corner of the screen.
// Example: (x, y) == (1.0, 0.5) is a point that lies immediately to the
//          right of the right edge of the screen, half-way down the screen.
//
// WA (Window Area)
// This co-ordinate system covers the usable area of a window, that is, the
// entire window except the window's borders.  Because this co-ordinate
// system is related to a window, its absolute measure may differ between
// windows.
// x: From 0.0 (inclusive) to 1.0 (exclusive), left to right.
// y: From 0.0 (inclusive) to 1.0 (exclusive), top to bottom.
// Example: (x, y) == (0.0, 0.0) is the top left corner of the usable area
//          of the window.
// Example: (x, y) == (0.5, 0.0) is a point that lies just inside the window
//          area.  It is at the very top of the window area.  It lies half-
//          way to the right of the window area, touching the top border of
//          the window.
// Example: Assume the following:
//             - Window A is a small window in the top-left quadrant of the
//               screen.  No part of window a lies outside the screen.
//             - Window B is a large window in the bottom-right quadrant of
//               the screen.  No part of window B lies outside the screen.
//             - Windows A and B do not overlap.
//          Let W(x, y) denote position (x, y) in window W, where W is
//          either A or B.  Now, the following statements are valid:
//             - For all 0.0<=p<1.0 and 0.0<=q<1.0 , A(p, q) and B(p, q)
//               correspond to different points on the screen.
//               Reason: A and B do not overlap.
//             - For all 0.0<=p0<1.0 , 0.0<=p1<1.0 , 0.0<=q0<1.0 ,
//               0.0<=q1<1.0 , the distance between A(p0, q0) and A(p1, q1)
//               is smaller than the distance between B(p0, q0) and
//               B(p1, q1) .
//               Reason: A is smaller than B.
//
// Whenever one of these co-ordinate systems is used, a comment identifies
// the co-ordinate system by its two-character code (AS, FS, or WA).


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

struct priv_bordersize   // sizes of the borders of the window
{
   int (*Left  )();      // width  of the left   border of the window, SP
   int (*Right )();      // width  of the right  border of the window, SP
   int (*Top   )();      // height of the top    border of the window, SP
   int (*Bottom)();      // height of the bottom border of the window, SP
};  // priv_bordersize


struct priv_windowarea   // co-ordinates of the edges of the window area, FS
{
   float  left;          // x co-ordinate of the left   edge, inclusive
   float  right;         // x co-ordinate of the right  edge, exclusive
   float  top;           // y co-ordinate of the top    edge, inclusive
   float  bottom;        // y co-ordinate of the bottom edge, exclusive
};  // priv_windowarea


class osbasicwindow:            // basic window
   public osobject              //    generic object
{
public:
   // Types.

   typedef
   void *                       // handle of a window
   priv_windowhandle;

   // Life cycle.

   osbasicwindow                // initialises an <osbasicwindow>
   ();

   virtual
   ~osbasicwindow               // closes down an <osbasicwindow>
   ();

   virtual
   osresult                     // result code
   Create                       // creates an <osbasicwindow>
   (float            left,      // I: x co-ordinate of left   edge, AS
    float            right,     // I: x co-ordinate of right  edge, AS
    float            top,       // I: y co-ordinate of top    edge, AS
    float            bottom,    // I: y co-ordinate of bottom edge, AS
    wchar_t const *  name,      // I: name of the window
    osbool           vis);      // I: must the window be visible?

   virtual
   osresult                     // result code
   Destroy                      // destroys an <osbasicwindow>
   (osbool  forced = OSFALSE);  // I: force destruction?

   // Handling text.

   osresult                         // result code
   CreateWindowFont                 // creates a window font
   (wchar_t const *    name,        // I: name of the font
    float              vsize,       // I: vertical size, WA
    float              hsize,       // I: horizontal size; 1.0==normal
    ospainter const &  painter,     // I: window painter
    oswindowfont &     font)        // O: window font
   const;

   inline
   osresult                         // result code
   DestroyWindowFont                // destroys a window font
   (oswindowfont &  font)           // I: window font
   const;

   float                            // height of the font, WA
   FontHeight                       // obtains the height of a font
   (oswindowfont const &  font)     // I: font
   const;

   osresult                         // result code
   TextWidth                        // obtains the widths of a text string
   (oswindowfont const &  font,     // I: font
    ospainter const &     painter,  // I: window painter
    wchar_t const *       text,     // I: text
    float &               left,     // O: width of left margin, WA
    float &               middle,   // O: width of text without margins, WA
    float &               full)     // O: width of text plus margins, WA
   const;

   // Controlling the window.

   osresult           // result code
   Update             // tells the window to update an area
   (float  left,      // I: left   edge of the area to update, WA
    float  right,     // I: right  edge of the area to update, WA
    float  top,       // I: top    edge of the area to update, WA
    float  bottom);   // I: bottom edge of the area to update, WA

   // Obtaining information.

   inline
   float              // width of the window, AS
   Width              // obtains the current width of the window
   ()
   const;

   inline
   float              // height of the window, AS
   Height             // obtains the current height of the window
   ()
   const;

   inline
   wchar_t const *    // the name of the window
   Name               // obtains the name of the window
   ()
   const;

   inline
   osbool             // is the window visible?
   Visible            // checks if the window is currently visible
   ()
   const;

   // Handling all messages that Windows sends.

   int32                  // result code
   priv_MessageHandler    // handles all messages that Windows sends
   (uint32  message,      // I: message code
    uint32  param1,       // I: first parameter to the message
    int32   param2);      // I: second parameter to the message

   // Data.

   priv_windowhandle      // handle of the window
   priv_handle;

protected:
   // Paint functions.

   virtual
   osresult                       // result code
   Write                          // writes text
   (float                 x,      // I: x co-ordinate of the text, WA
    float                 y,      // I: y co-ordinate of the text, WA
    wchar_t const *       text,   // I: text to write
    oswindowfont const &  font,   // I: font to use
    oscolor const &       color,  // I: color of the text
    ospainter const &     p);     // I: window painter

   virtual
   osresult                       // result code
   Line                           // draws a straight line
   (float              x0,        // I: x co-ord of start point (incl.), WA
    float              y0,        // I: y co-ord of start point (incl.), WA
    float              x1,        // I: x co-ord of end   point (excl.), WA
    float              y1,        // I: y co-ord of end   point (excl.), WA
    oscolor const &    color,     // I: color of the line
    ospainter const &  p);        // I: window painter

   virtual
   osresult                       // result code
   OutlineRectangle               // draws the outline of a rectangle
   (float              left,      // I: left   edge of the rectangle, WA
    float              right,     // I: right  edge of the rectangle, WA
    float              top,       // I: top    edge of the rectangle, WA
    float              bottom,    // I: bottom edge of the rectangle, WA
    oscolor const &    color,     // I: color of the rectangle
    ospainter const &  p);        // I: window painter

   virtual
   osresult                       // result code
   FilledRectangle                // draws a filled rectangle
   (float              left,      // I: left   edge of the rectangle, WA
    float              right,     // I: right  edge of the rectangle, WA
    float              top,       // I: rop    edge of the rectangle, WA
    float              bottom,    // I: bottom edge of the rectangle, WA
    oscolor const &    color,     // I: color of the rectangle
    ospainter const &  p);        // I: window painter

   // Signal functions.

   virtual
   osresult             // result code
   WindowCreation       // the window is being created
   (long   ,            // -: time of window creation
    float  ,            // -: x position of the mouse, WA
    float  );           // -: y position of the mouse, WA

   virtual
   void
   WindowDestruction    // the window is being destroyed
   (long   ,            // -: time of window destruction
    float  ,            // -: x position of the mouse, WA
    float  );           // -: y position of the mouse, WA

   virtual
   void
   WindowPosSize        // position and size of window have changed
   (long   ,            // -: time of change in position and size
    float  ,            // -: x position of the mouse, WA
    float  );           // -: y position of the mouse, WA

   virtual
   void
   WindowActivation         // the window is being activated
   (priv_windowhandle  ,    // -: window that is being de-activated
    long               ,    // -: time of activation
    float              ,    // -: x position of the mouse, WA
    float              );   // -: y position of the mouse, WA

   virtual
   void
   WindowDeactivation       // the window is being de-activated
   (priv_windowhandle  ,    // -: window that is being activated
    long               ,    // -: time of de-activation
    float              ,    // -: x position of the mouse, WA
    float              );   // -: y position of the mouse, WA

   virtual
   void
   MousePosition            // the position of the mouse has changed
   (long    ,               // -: time of the change of the mouse position
    float   x,              // I: new x position of the mouse, WA
    float   y,              // I: new y position of the mouse, WA
    osbool  ,               // -: left  mouse button is down?
    osbool  ,               // -: right mouse button is down?
    osbool  ,               // -: shift key 1 is down?
    osbool  ,               // -: shift key 2 is down?
    osbool  );              // -: shift key 3 is down?

   virtual
   void
   MouseLeftSelect          // the left mouse button was used to select
   (long    ,               // -: time of selection with left mouse button
    float   ,               // -: new x position of the mouse, WA
    float   ,               // -: new y position of the mouse, WA
    osbool  ,               // -: left  mouse button is down?
    osbool  ,               // -: right mouse button is down?
    osbool  ,               // -: shift key 1 is down?
    osbool  ,               // -: shift key 2 is down?
    osbool  );              // -: shift key 3 is down?

   virtual
   void
   MouseLeftActivate        // the left mouse button was used to activate
   (long    ,               // -: time of activation with left mouse button
    float   ,               // -: new x position of the mouse, WA
    float   ,               // -: new y position of the mouse, WA
    osbool  ,               // -: left  mouse button is down?
    osbool  ,               // -: right mouse button is down?
    osbool  ,               // -: shift key 1 is down?
    osbool  ,               // -: shift key 2 is down?
    osbool  );              // -: shift key 3 is down?

   virtual
   void
   MouseLeftRelease         // the left mouse button was released
   (long    ,               // -: time of release of left mouse button
    float   ,               // -: new x position of the mouse, WA
    float   ,               // -: new y position of the mouse, WA
    osbool  ,               // -: left  mouse button is down?
    osbool  ,               // -: right mouse button is down?
    osbool  ,               // -: shift key 1 is down?
    osbool  ,               // -: shift key 2 is down?
    osbool  );              // -: shift key 3 is down?

   virtual
   void
   MouseRightSelect         // the right mouse button was used to select
   (long    ,               // -: time of selection with right mouse button
    float   ,               // -: new x position of the mouse, WA
    float   ,               // -: new y position of the mouse, WA
    osbool  ,               // -: left  mouse button is down?
    osbool  ,               // -: right mouse button is down?
    osbool  ,               // -: shift key 1 is down?
    osbool  ,               // -: shift key 2 is down?
    osbool  );              // -: shift key 3 is down?

   virtual
   void
   MouseRightActivate       // the right mouse button was used to activate
   (long    ,               // -: time of activation with right mouse button
    float   ,               // -: new x position of the mouse, WA
    float   ,               // -: new y position of the mouse, WA
    osbool  ,               // -: left  mouse button is down?
    osbool  ,               // -: right mouse button is down?
    osbool  ,               // -: shift key 1 is down?
    osbool  ,               // -: shift key 2 is down?
    osbool  );              // -: shift key 3 is down?

   virtual
   void
   MouseRightRelease        // the right mouse button was released
   (long    ,               // -: time of release of right mouse button
    float   ,               // -: new x position of the mouse, WA
    float   ,               // -: new y position of the mouse, WA
    osbool  ,               // -: left  mouse button is down?
    osbool  ,               // -: right mouse button is down?
    osbool  ,               // -: shift key 1 is down?
    osbool  ,               // -: shift key 2 is down?
    osbool  );              // -: shift key 3 is down?

   // Inquiry functions.

   virtual
   osbool                  // window may be closed?
   CanBeClosed             // checks if it is OK to close the window
   (long   ,               // -: time of the closure request
    float  ,               // -: x position of the mouse, WA
    float  );              // -: y position of the mouse, WA

   virtual
   void
   SizeLimits              // ask for size limits for the window
   (float &  minwidth,     // O: minimum width  of the window, AS
    float &  maxwidth,     // O: maximum width  of the window, AS
    float &  minheight,    // O: minimum height of the window, AS
    float &  maxheight);   // O: maximum height of the window, AS

   // Command functions.

   virtual
   osresult                     // result code
   Paint                        // paints an area of the window
   (float              left,    // I: L edge of area that needs painting, WA
    float              right,   // I: R edge of area that needs painting, WA
    float              top,     // I: T edge of area that needs painting, WA
    float              bottom,  // I: B edge of area that needs painting, WA
    long               ,        // -: time of the paint request
    float              ,        // -: x position of the mouse, WA
    float              ,        // -: y position of the mouse, WA
    ospainter const &  p)       // I: window painter
   = 0;                         // derived class _must_ provide this func

   // Implementation of the actual functionality.

   osresult                     // result code
   priv_Create                  // creates an <osbasicwindow>
   (float            left,      // I: x co-ordinate of left   edge, AS
    float            right,     // I: x co-ordinate of right  edge, AS
    float            top,       // I: y co-ordinate of top    edge, AS
    float            bottom,    // I: y co-ordinate of bottom edge, AS
    wchar_t const *  name,      // I: name of the window
    osbool           vis);      // I: must the window be visible?

private:
   // Protective functions.

   osbasicwindow                // dummy private copy constructor
   (osbasicwindow const &)      //    prevents copying
   {}

   osbasicwindow const &        // dummy private assignment operator
   operator =                   //    prevents assignment
   (osbasicwindow const &)
   const
   { return *this; }

   // Implementation of the functionality.

   osresult                            // result code
   Write                               // writes text to the window
   #ifdef OSRTL_UNICODE
      (wchar_t const *       text,     // I: text to write
       wchar_t const *       wtext,    // I: text to write
       int                   length,   // I: length of the text
       int                   extra,    // I: amount of extra space
       float                 x,        // I: x co-ordinate of the text, WA
       float                 y,        // I: y co-ordinate of the text, WA
       ospainter const &     paint,    // I: window painter
       oswindowfont const &  font);    // I: used font
   #else
      (char *                text,     // I: text to write
       wchar_t const *       wtext,    // I: text to write
       int                   length,   // I: length of the text
       int                   extra,    // I: amount of extra space
       float                 x,        // I: x co-ordinate of the text, WA
       float                 y,        // I: y co-ordinate of the text, WA
       ospainter const &     paint,    // I: window painter
       oswindowfont const &  font);    // I: used font
   #endif

   // Data.

   priv_bordersize  border;    // sizes of the borders of the window, SP

   priv_windowarea  area;      // co-ordinates of edges of window area, AS

   osbool     created;   // creation of the window has completed?

   wchar_t *  title;     // window title

   osbool     visible;   // whether the window is actually visible

   float      lastmx;    // last known x co-ordinate of the mouse cursor, WA
   float      lastmy;    // last known y co-ordinate of the mouse cursor, WA

   osbool     forcedestruction;   // force destruction of the window?

   osresult   result_destroy;     // result of attempt to destroy window

};  // osbasicwindow


/**************************************************************************
 * Function: osbasicwindow::DestroyWindowFont                             *
 **************************************************************************/

inline
osresult                           // result code
osbasicwindow::DestroyWindowFont   // destroys a window font
(oswindowfont &  font)             // I: window font
const

{  // osbasicwindow::DestroyWindowFont

   return font.Destroy();

}  // osbasicwindow::DestroyWindowFont


/**************************************************************************
 * Function: osbasicwindow::Width                                         *
 **************************************************************************/

inline
float                  // width of the window, AS
osbasicwindow::Width   // obtains the current width of the window
()
const

{  // osbasicwindow::Width

   return area.right - area.left;

}  // osbasicwindow::Width


/**************************************************************************
 * Function: osbasicwindow::Height                                        *
 **************************************************************************/

inline
float                   // height of the window, AS
osbasicwindow::Height   // obtains the current height of the window
()
const

{  // osbasicwindow::Height

   return area.bottom - area.top;

}  // osbasicwindow::Height


/**************************************************************************
 * Function: osbasicwindow::Name                                          *
 **************************************************************************/

inline
wchar_t const *       // the name of the window
osbasicwindow::Name   // obtains the name of the window
()
const

{  // osbasicwindow::Name

   return const_cast<wchar_t const *>(title);

}  // osbasicwindow::Name


/**************************************************************************
 * Function: osbasicwindow::Visible                                       *
 **************************************************************************/

inline
osbool                   // is the window visible?
osbasicwindow::Visible   // checks if the window is currently visible
()
const

{  // osbasicwindow::Visible

   return visible;

}  // osbasicwindow::Visible


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
 * End of osrtl/windows/window/window/window.h .                          *
 **************************************************************************/


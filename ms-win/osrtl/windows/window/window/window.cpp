
/**************************************************************************
 * File:     osrtl/windows/window/window/window.cpp                       *
 * Contents: Basic window.                                                *
 * Document: osrtl/windows/window/window/window.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/window/window.cpp  -  Basic window

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:36  erick
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
// 19970719
// 19970825
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// The unit <osrtl> is the only unit that is allowed to include files that
// have to do with the operating system and the RTL.
#include <limits.h>    // ranges of basic types
#define STRICT
#include <windows.h>   // header file of the OS

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"       // basic definitions
#include "osrtl/windows/error/error.h"         // error handling
#include "osrtl/windows/hardware/hardware.h"   // hardware characteristics
#include "osrtl/windows/startup/startup.h"     // start-up
#include "osrtl/windows/tasking/tasking.h"     // task control
#include "osrtl/windows/heap/heap.h"           // heap
#include "osrtl/windows/text/text.h"           // textual data
#include "osrtl/windows/sound/sound.h"         // sounds
#include "osrtl/windows/scrnobj/scrnobj.h"     // screen objects
#include "osrtl/windows/drawing/drawing.h"     // attributes for drawing

// Other modules of the component <window>.
#include "osrtl/windows/window/winpaint/winpaint.h" // window painter
#include "osrtl/windows/window/winfont/winfont.h"   // window font
#include "osrtl/windows/window/admin/admin.h"       // window administration

// Header file of the current module.
#include "osrtl/windows/window/window/window.h"


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
//               screen.  No part of window A lies outside the screen.
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
// SP (Screen Pixels)
// This co-ordinate system is the pixel equivalent of FS.  It covers the
// entire screen.  Co-ordinates in this system are expressed as integer
// values, not as floating-point values.
// x: From 0 (inclusive) to ScreenWidthPixels()-1 (inclusive), left to
//    right.  ScreenWidthPixels() is the width of the entire screen,
//    measured in pixels.
// y: From 0 (inclusive) to ScreenHeightPixels()-1 (inclusive), top to
//    bottom.  ScreenHeightPixels() is the height of the entire screen,
//    measured in pixels.
// Example: (x, y) == (0, 0) is the top-left pixel of the screen.
// Example: (x, y) == (ScreenWidthPixels()-1, ScreenHeightPixels()-1) is the
//          bottom-right pixel of the screen.
//
// WP (Window Pixels)
// This co-ordinate system is the pixel equivalent of WA.  It covers the
// usable area of a window, that is, the entire window without the window's
// borders.  Unlike WA, the absolute measure of this system does not differ
// between windows.
// x: From 0 (inclusive) to WindowWidth()-1 (inclusive), left to right.
//    WindowWidth() is the width of the usable area of the window, measured
//    in pixels.
// y: From 0 (inclusive) to WindowHeight()-1 (inclusive), top to bottom.
//    WindowHeight() is the height of the usable area of the window,
//    measured in pixels.
// Example: (x, y) == (0, 0) is the top-left pixel of the window area.  This
//          pixel touches both the top border and the left border of the
//          window.
// Example: (x, y) == (WindowWidthPixels()-1, WindowHeightPixels()-1) is the
//          bottom-right pixel of the window area.  This pixel touches both
//          the right border and the bottom border of the window.
// Example: Assume the following:
//             - Window A is a small window in the top-left quadrant of the
//               screen.  No part of window A lies outside the screen.
//             - Window B is a large window in the bottom-right quadrant of
//               the screen.  No part of window B lies outside the screen.
//             - Windows A and B do not overlap.
//          Let W(x, y) denote pixel (x, y) in window W, where W is either
//          A or B.  Now, the following statements are valid:
//             - For all 0<=p<=WindowWidthPixels(A)-1 and
//               0<=q<=WindowHeightPixels(B)-1 , A(p, q) and B(p, q)
//               correspond to different points on the screen.
//               Reason: A and B do not overlap.
//             - For all 0<=p0<=WindowWidthPixels(A)-1 ,
//               0<=p1<=WindowWidthPixels(A)-1 ,
//               0<=q0<=WindowHeightPixels(A)-1 ,
//               0<=q1<=WindowHeightPixels(A)-1 , the distance between
//               A(p0, q0) and A(p1, q1) is equal to the distance between
//               B(p0, q0) and B(p1, q1) .
//               Reason: WP is expressed in pixels; therefore, the absolute
//                       measure of WP does not depend on the window for
//                       which it is used.  The fact that A is smaller than
//                       B is accounted for by the fact that
//                       WindowWidthPixels(A) < WindowWidthPixels(B) and
//                       WindowHeightPixels(A) < WindowHeightPixels(B) .
//
// Whenever one of these co-ordinate systems is used, a comment identifies
// the co-ordinate system by its two-character code (AS, FS, WA, SP or WP).


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class masterwindow:            // master window
   private osbasicwindow       //    basic window
{
public:
   // Life cycle.

   virtual
   osresult                    // result code
   Create                      // creates a <masterwindow>
   (float            left,     // I: x co-ordinate of left   edge (AS)
    float            right,    // I: x co-ordinate of right  edge (AS)
    float            top,      // I: y co-ordinate of top    edge (AS)
    float            bottom,   // I: y co-ordinate of bottom edge (AS)
    wchar_t const *  name,     // I: name of the window
    osbool           vis);     // I: must the window be visible?

protected:
   // Signal functions.

   virtual
   void
   WindowDestruction   // the window is being destroyed
   (long   ,           // -: time of window destruction
    float  ,           // -: x position of the mouse
    float  );          // -: y position of the mouse

   // Command functions.

   virtual
   osresult                      // result code
   Paint                         // paints an area of the window
   (float              left,     // I: L edge of area that needs painting
    float              right,    // I: R edge of area that needs painting
    float              top,      // I: T edge of area that needs painting
    float              bottom,   // I: B edge of area that needs painting
    long               time,     // I: time of window creation
    float              x,        // I: x position of the mouse
    float              y,        // I: y position of the mouse
    ospainter const &  p);       // I: window painter

private:
   // Types.

   typedef  osbasicwindow  inherited;   // base class

};  // masterwindow


class guard_osrtl_window_window   // guards this module
{
public:
   // Life cycle.

   virtual
   ~guard_osrtl_window_window     // performs exit checks on the module
   ();

};  // guard_osrtl_window_window


/**************************************************************************
 * Local functions.                                                       *
 **************************************************************************/

static
int                    // width of the left border of a named window
NamedLeftBorder        // obtains width of left border of a named window
();

static
int                    // width of the right border of a named window
NamedRightBorder       // obtains width of right border of a named window
();

static
int                    // height of the top border of a named window
NamedTopBorder         // obtains height of top border of a named window
();

static
int                    // height of the bottom border of a named window
NamedBottomBorder      // obtains height of bottom border of a named window
();

static
int                    // width of the left border of a nameless window
NamelessLeftBorder     // obtains width of left border of a nameless window
();

static
int                    // width of the right border of a nameless window
NamelessRightBorder    // obtains width of right border of a nameless window
();

static
int                    // height of the top border of a nameless window
NamelessTopBorder      // obtains height of top border of a nameless window
();

static
int                    // height of the bottom border of a nameless window
NamelessBottomBorder   // obtains height of bottom border of nameless window
();

static
osresult                           // result code
PositionAStoSP                     // converts co-ordinates from AS to SP
(float                    asx,     // I: x co-ordinate, AS
 float                    asy,     // I: y co-ordinate, AS
 priv_bordersize const &  border,  // I: border sizes
 int &                    spx,     // O: x co-ordinate, SP
 int &                    spy);    // O: y co-ordinate, SP

static
void
PositionSPtoAS                     // converts co-ordinates from SP to AS
(int                      spx,     // I: x co-ordinate, SP
 int                      spy,     // I: y co-ordinate, SP
 priv_bordersize const &  border,  // I: border sizes
 float &                  asx,     // O: x co-ordinate, AS
 float &                  asy);    // O: y co-ordinate, AS

static
void
PositionSPtoWA                     // converts co-ordinates from SP to WA
(int                      spx,     // I: x co-ordinate, SP
 int                      spy,     // I: y co-ordinate, SP
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  wax,     // O: x co-ordinate, WA
 float &                  way);    // O: y co-ordinate, WA

static
osresult                           // result code
PositionWAtoWP                     // converts co-ordinates from WA to WP
(float                    wax,     // I: x co-ordinate, WA
 float                    way,     // I: y co-ordinate, WA
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 int &                    wpx,     // O: x co-ordinate, WP
 int &                    wpy);    // O: y co-ordinate, WP

static
void
PositionWPtoWA                     // converts co-ordinates from WP to WA
(int                      wpx,     // I: x co-ordinate, WP
 int                      wpy,     // I: y co-ordinate, WP
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  wax,     // O: x co-ordinate, WA
 float &                  way);    // O: y co-ordinate, WA

static
osresult                           // result code
SizeAStoSP                         // converts sizes from AS to SP
(float                    ash,     // I: horizontal size, AS
 float                    asv,     // I: vertical   size, AS
 priv_bordersize const &  border,  // I: border sizes, SP
 int &                    sph,     // O: horizontal size, SP
 int &                    spv);    // O: vertical   size, SP

static
osresult                           // result code
SizeFStoSP                         // converts sizes from FS to SP
(float                    fsh,     // I: horizontal size, FS
 float                    fsv,     // I: vertical   size, FS
 int &                    sph,     // O: horizontal size, SP
 int &                    spv);    // O: vertical   size, SP

static
void
SizeWAtoFS                         // converts sizes from WA to FS
(float                    wah,     // I: horizontal size, WA
 float                    wav,     // I: vertical   size, WA
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  fsh,     // O: horizontal size, FS
 float &                  fsv);    // O: vertical   size, FS

static
void
SizeFStoWA                         // converts sizes from FS to WA
(float                    fsh,     // I: horizontal size, FS
 float                    fsv,     // I: vertical   size, FS
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  wah,     // O: horizontal size, WA
 float &                  wav);    // O: vertical   size, WA

static
void
GetMouseData                       // obtains data on a mouse event
(float &                  mx,      // O: x co-ordinate of the mouse, WA
 float &                  my,      // O: y co-ordinate of the mouse, WA
 osbool &                 mleft,   // O: left  mouse button is down?
 osbool &                 mright,  // O: right mouse button is down?
 osbool &                 shift1,  // O: first  shift key is down?
 osbool &                 shift2,  // O: second shift key is down?
 osbool &                 shift3,  // O: third  shift key is down?
 uint32                   param1,  // I: parameter to the mouse message
 int32                    param2,  // I: parameter to the mouse message
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area);   // I: edges of window area, AS

inline
int                  // signed integer value
WordToInt            // converts 16-bit unsigned word to 32-bit signed int
(WORD  w);           // I: word to convert

extern "C"
LRESULT              // result of processing a window message
CALLBACK
GenericHandler       // processes all messages for a window
(HWND    window,     // I: window that receives the message
 UINT    message,    // I: message for the window
 WPARAM  wparam,     // I: parameter to <message>, additional information
 LPARAM  lparam);    // I: parameter to <message>, additional information


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

static
HWND                           // handle of the master window
masterhandle
= static_cast<HWND>(OSNULL);

static
int                            // number of existing <osbasicwindow>s,
windows                        //    excluding the master window
= 0;

static
masterwindow                   // master window
master;

static
guard_osrtl_window_window      // guards this module
guard;


/**************************************************************************
 * Function: WordToInt                                                    *
 **************************************************************************/

inline
int         // signed integer value
WordToInt   // converts 16-bit unsigned word to 32-bit signed int
(WORD  w)   // I: word to convert

{  // WordToInt

   // This function is highly dependent on
   //    1. The fact that a <WORD> is an unsigned value of exactly 16 bits.
   //    2. The fact that the target machine uses two's complement
   //       representation for integers.

   uint32  u32;   // steps in the
   int32   i32;   //    conversion process

   u32 = static_cast<uint32>(w);    // zero-extend from 16 to 32 bits
   i32 = static_cast<int32>(u32);   // convert to signed (is no change)
   if (i32 >= 0x00008000)           // if the original sign bit was set,
      i32 |= ~0x0000FFFF;           //    sign-extend the original value
   return static_cast<int>(i32);

}  // WordToInt


/**************************************************************************
 * Function: osbasicwindow::osbasicwindow                                 *
 **************************************************************************/

osbasicwindow::osbasicwindow   // initialises an <osbasicwindow>
()

{  // osbasicwindow::osbasicwindow

   if (!SetName(L"<osbasicwindow>"))
      OsDebugErrorMessage(L"Cannot set the name of an <osbasicwindow>.");

   created     = OSFALSE;
   title       = static_cast<wchar_t *>        (OSNULL);
   priv_handle = static_cast<priv_windowhandle>(OSNULL);
   lastmx      = 1.0;
   lastmy      = 1.0;
   visible     = OSFALSE;

}  // osbasicwindow::osbasicwindow


/**************************************************************************
 * Function: osbasicwindow::~osbasicwindow                                *
 **************************************************************************/

osbasicwindow::~osbasicwindow   // closes down an <osbasicwindow>
()

{  // osbasicwindow::~osbasicwindow

   if (created)
      OsDebugErrorMessage(L"An <osbasicwindow> was not "
                          L"properly closed down:\n"
                          L"it is still marked as existing.");

   if (priv_handle != static_cast<priv_windowhandle>(OSNULL))
      OsDebugErrorMessage(L"An <osbasicwindow> was not "
                          L"properly closed down:\n",
                          L"it was not destroyed, "
                          L"its handle still exists.");

   if (title != static_cast<wchar_t *>(OSNULL))
      OsDebugErrorMessage(L"An <osbasicwindow> was not "
                          L"properly closed down:\n",
                          L"its name was not de-allocated.");

}  // osbasicwindow::~osbasicwindow


/**************************************************************************
 * Function: osbasicwindow::Create                                        *
 **************************************************************************/

osresult                    // result code
osbasicwindow::Create       // creates an <osbasicwindow>
(float            left,     // I: x co-ordinate of left   edge, AS
 float            right,    // I: x co-ordinate of right  edge, AS
 float            top,      // I: y co-ordinate of top    edge, AS
 float            bottom,   // I: y co-ordinate of bottom edge, AS
 wchar_t const *  name,     // I: name of the window
 osbool           vis)      // I: must the window be visible?

{  // osbasicwindow::Create

   osresult  result;   // result code
   osmutex   mutex;    // mutex for doing window administration

   if (    (result = mutex.Connect(MUTEX_WINDOWMANAGE)) == OSRESULT_OK
       &&  (result = mutex.EnterSection())              == OSRESULT_OK)
   {
      if (windows == 0)
      {
         // A window must be created while no windows exist.  Initialise
         // the window system by creating the master window.
         #ifdef OSRTL_DEBUGGING
            result = master.Create(0.25, 0.75, 0.1, 0.6,
                                   ProgramName(),
                                   OSFALSE);
         #else
            result = master.Create(1.25, 1.75, -0.6, -0.1,
                                   ProgramName(),
                                   OSFALSE);
         #endif
      }

      if (result == OSRESULT_OK)
      {
         result = priv_Create(left, right, top, bottom, name, vis);
         if (result == OSRESULT_OK)
            if (++windows <= 0)
            {
               OsDebugErrorMessage(L"Error counting the number "
                                   L"of existing <osbasicwindow>s.");
               result = OSINTERNAL_WINDOWADMIN;
            }
      }

      // Leave the critical section.
      {
         osresult  leave;   // result of leaving the critical section

         leave = mutex.LeaveSection();
         if (result == OSRESULT_OK)
            result = leave;
         else
            if (leave != OSRESULT_OK)
               OsDebugErrorMessage(L"Cannot leave the critical section\n"
                                   L"while creating an <osbasicwindow>.");
      }
   }

   return result;

}  // osbasicwindow::Create


/**************************************************************************
 * Function: osbasicwindow::Destroy                                       *
 **************************************************************************/

osresult                 // result code
osbasicwindow::Destroy   // destroys an <osbasicwindow>
(osbool  forced)         // I: force destruction?

{  // osbasicwindow::Destroy

   osbool  destroyed = OSFALSE;   // window was destroyed successfully?

   if (priv_handle != static_cast<priv_windowhandle>(OSNULL))
   {
      result_destroy = OSRESULT_OK;

      forcedestruction = forced;
      if (DestroyWindow(reinterpret_cast<HWND>(priv_handle)))
         destroyed = OSTRUE;
      forcedestruction = OSFALSE;
   }
   else
      destroyed = OSTRUE;

   return destroyed ? OSRESULT_OK : result_destroy;

}  // osbasicwindow::Destroy


/**************************************************************************
 * Function: osbasicwindow::CreateWindowFont                              *
 **************************************************************************/

osresult                          // result code
osbasicwindow::CreateWindowFont   // creates a window font
(wchar_t const *    name,         // I: name of the font
 float              vsize,        // I: vertical size, WA
 float              hsize,        // I: horizontal size; 1.0==normal
 ospainter const &  painter,      // I: window painter
 oswindowfont &     font)         // O: window font
const

{  // osbasicwindow::CreateWindowFont

   {
      float  dummy;   // (no need to convert the horizontal size)

      SizeWAtoFS(0.0, vsize, border, area, dummy, vsize);
   }

   return font.Create(name, vsize, hsize, painter);

}  // osbasicwindow::CreateWindowFont


/**************************************************************************
 * Function: osbasicwindow::FontHeight                                    *
 **************************************************************************/

float                          // height of the font, WA
osbasicwindow::FontHeight      // obtains the height of a font
(oswindowfont const &  font)   // I: font
const

{  // osbasicwindow::FontHeight

   float  fontheight;   // height of the font, FS

   fontheight = font.Height();

   {
      float  dummy;   // (no need to convert a horizontal size)

      SizeFStoWA(0.0, fontheight, border, area, dummy, fontheight);
   }

   return fontheight;

}  // osbasicwindow::FontHeight


/**************************************************************************
 * Function: osbasicwindow::TextWidth                                     *
 **************************************************************************/

osresult                          // result code
osbasicwindow::TextWidth          // obtains the widths of a text string
(oswindowfont const &  font,      // I: font
 ospainter const &     painter,   // I: window painter
 wchar_t const *       text,      // I: text
 float &               left,      // O: width of left margin, WA
 float &               middle,    // O: width of text without margins, WA
 float &               full)      // O: width of text plus margins, WA
const

{  // osbasicwindow::TextWidth

   osresult  result;   // result code

   result = font.TextWidthIdeal(text, painter, left, middle, full);
   if (result == OSRESULT_OK)
   {
      float  dummy;   // (no need to convert a vertical size)

      SizeFStoWA(left,   0.0, border, area, left,   dummy);
      SizeFStoWA(middle, 0.0, border, area, middle, dummy);
      SizeFStoWA(full,   0.0, border, area, full,   dummy);
   }

   return result;

}  // osbasicwindow::TextWidth


/**************************************************************************
 * Function: osbasicwindow::Update                                        *
 **************************************************************************/

osresult               // result code
osbasicwindow::Update  // tells the window to update an area
(float  left,          // I: left   edge of the area to update, WA
 float  right,         // I: right  edge of the area to update, WA
 float  top,           // I: top    edge of the area to update, WA
 float  bottom)        // I: bottom edge of the area to update, WA

{  // osbasicwindow::Update

   osresult  result;   // result code
   RECT      update;   // area to update

   // Convert the co-ordinates from WA to WP.
   {
      int  x;   // x co-ordinate of one of the edges of the area, WP
      int  y;   // y co-ordinate of one of the edges of the area, WP

      result = PositionWAtoWP(left, top, border, area, x, y);
      if (result == OSRESULT_OK)
      {
         update.left = static_cast<LONG>(x);
         update.top  = static_cast<LONG>(y);

         result = PositionWAtoWP(right, bottom, border, area, x, y);
         if (result == OSRESULT_OK)
         {
            update.right  = static_cast<LONG>(x);
            update.bottom = static_cast<LONG>(y);
         }
      }
   }

   if (result == OSRESULT_OK)
      if (!InvalidateRect(reinterpret_cast<HWND>(priv_handle),
                          &update,
                          FALSE))
         result = OSINTERNAL_DRAW;

   return result;

}  // osbasicwindow::Update


/**************************************************************************
 * Function: osbasicwindow::priv_MessageHandler                           *
 **************************************************************************/

int32                                // result code
osbasicwindow::priv_MessageHandler   // handles all messages Windows sends
(uint32  message,                    // I: message code
 uint32  param1,                     // I: first parameter to the message
 int32   param2)                     // I: second parameter to the message

{  // osbasicwindow::priv_MessageHandler

   int32  result;        // result of processing the message
   long   messagetime;   // time when the message was sent
   float  mousex;        // x pos. of mouse cursor when message was sent, WA
   float  mousey;        // y pos. of mouse cursor when message was sent, WA

   // Obtain the time when the message was sent.
   messagetime = GetMessageTime();

   // Obtain the position of the cursor when the message was sent.
   if (MouseButtons() > 0)
   {
      int  x;   // x position of cursor when message was sent, SP
      int  y;   // y position of cursor when message was sent, SP

      {
         DWORD  cursorpos;   // position of cursor when the message was sent

         cursorpos = GetMessagePos();
         x = WordToInt(LOWORD(cursorpos));
         y = WordToInt(HIWORD(cursorpos));
      }

      PositionSPtoWA(x, y, border, area, mousex, mousey);
   }
   else
      mousex = mousey = 1.0;

   // Process the message.
   switch (static_cast<UINT>(message))
   {
      case WM_CREATE:    // window is being created

         if (WindowCreation(messagetime, mousex, mousey) == OSRESULT_OK)
            result = static_cast<int32>( 0);  // continue creation of window
         else
            result = static_cast<int32>(-1);  // destroy window

         break;


      case WM_CLOSE:    // request to close the window

         if (CanBeClosed(messagetime, mousex, mousey))
            // Let the default message handler handle the request.
            result = static_cast<int32>
                        (DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                                       WM_CLOSE,
                                       static_cast<WPARAM>(param1),
                                       static_cast<LPARAM>(param2)));
         else
            // The message has been processed, even if the window will not
            // be closed.
            result = static_cast<int32>(0);

         break;


      case WM_DESTROY:    // window is being destroyed

         // Inform the window about the destruction.
         WindowDestruction(messagetime, mousex, mousey);

         // Destroy the window.
         {
            static  osbool  destroyingmaster = OSFALSE;
                                         // destroying the master window?

            osmutex  mutex;   // mutex for doing window administration

            result_destroy = OSRESULT_OK;
            if (    destroyingmaster
                ||      (result_destroy = mutex.Connect(MUTEX_WINDOWMANAGE))
                                                           == OSRESULT_OK
                    &&  (result_destroy = mutex.EnterSection())
                                                           == OSRESULT_OK)
            {
               // Check if someone else hasn't destroyed the window while we
               // were busy.
               if (priv_handle != static_cast<priv_windowhandle>(OSNULL))
               {
                  // Do not consider the window as existing.
                  created = OSFALSE;

                  // Administer destruction of the window.
                  {
                     windowadmin  admin;   // window administrator

                     result_destroy = admin.Destroy();
                  }

                  // Mark the fact that the window has been destroyed.
                  priv_handle = static_cast<priv_windowhandle>(OSNULL);
                  WindowExists(OSFALSE);

                  // De-allocate the window name.
                  {
                     osresult  dealloc;   // result of the de-allocation

                     // Do the de-allocation.
                     {
                        osheap<wchar_t>  heap;

                        dealloc = heap.Deallocate(title);
                     }

                     if (result_destroy == OSRESULT_OK)
                        result_destroy = dealloc;
                     else
                        if (dealloc != OSRESULT_OK)
                           OsDebugErrorMessage(L"Cannot de-allocate "
                                               L"the name of a window.");

                     title = static_cast<wchar_t *>(OSNULL);
                  }

                  // Handle the interface to the master window.
                  if (destroyingmaster)
                     masterhandle = static_cast<HWND>(OSNULL);
                  else
                     if (--windows == 0)
                     {
                        destroyingmaster = OSTRUE;
                        SendMessage(reinterpret_cast<HWND>(masterhandle),
                                    WM_CLOSE,
                                    static_cast<WPARAM>(0),
                                    static_cast<LPARAM>(0));
                        destroyingmaster = OSFALSE;
                     }
                     else
                        if (windows < 0)
                        {
                           OsDebugErrorMessage(L"Error counting "
                                               L"the number of "
                                               L"existing windows.");
                           result_destroy = OSINTERNAL_WINDOWADMIN;
                        }
               }

               // Leave the critical section.
               if (!destroyingmaster)
               {
                  osresult  leave;   // result of leaving critical section

                  leave = mutex.LeaveSection();
                  if (result_destroy == OSRESULT_OK)
                     result_destroy = leave;
                  else
                     if (leave != OSRESULT_OK)
                        OsDebugErrorMessage(L"Cannot leave the critical "
                                            L"section\nwhile destroying "
                                            L"a window.");
               }
            }
         }

         result = static_cast<int32>(0);   // message has been processed

         break;


      case WM_WINDOWPOSCHANGED:    // window's size, position or place in
                                   //    the Z order has (may have) changed

         // Update the co-ordinates of the window edges.
         {
            WINDOWPOS *  windata;   // data on position and size of window

            windata = reinterpret_cast<WINDOWPOS *>(param2);
            PositionSPtoAS(windata->x + border.Left(),
                           windata->y + border.Top (),
                           border,
                           area.left,
                           area.top);
            PositionSPtoAS(windata->x + windata->cx - border.Right (),
                           windata->y + windata->cy - border.Bottom(),
                           border,
                           area.right,
                           area.bottom);
         }

         // Inform the window about the change.
         WindowPosSize(messagetime, mousex, mousey);

         result = static_cast<int32>(0);   // message has been processed

         break;


      case WM_GETMINMAXINFO:    // ask for the minimum and the maximum size
                                //    of the window

         {
            float  minwidth;    // minimum width  of the window, AS
            float  maxwidth;    // maximum widht  of the window, AS
            float  minheight;   // minimum height of the window, AS
            float  maxheight;   // maximum height of the window, AS
            int    wmin;        // minimum width  of the window, SP
            int    wmax;        // maximum width  of the window, SP
            int    hmin;        // minimum height of the window, SP
            int    hmax;        // maximum height of the window, SP

            SizeLimits(minwidth, maxwidth, minheight, maxheight);
            if (    SizeAStoSP(minwidth, minheight, border, wmin, hmin)
                                                             == OSRESULT_OK
                &&  SizeAStoSP(maxwidth, maxheight, border, wmax, hmax)
                                                             == OSRESULT_OK)
            {
               // Correct for the fact that the co-ordinate conversion did
               // not take the borders into account.
               {
                  int  width;   // width of the left and right borders

                  width = border.Left() + border.Right();

                  if (wmin == 0)
                     wmin = width - 1;
                  else
                     wmin += width;

                  if (wmax == 0)
                     wmax = width - 1;
                  else
                     wmax += width;
               }
               {
                  int  height;   // height of the top and bottom borders

                  height = border.Top() + border.Bottom();

                  if (hmin == 0)
                     hmin = height - 1;
                  else
                     hmin += height;

                  if (hmax == 0)
                     hmax = height - 1;
                  else
                     hmax += height;
               }

               // Put the new size limits into the data structure that
               // Windows provided.
               {
                  MINMAXINFO *  info;   // information about the minimum and
                                        //    the maximum size of the window

                  info = reinterpret_cast<MINMAXINFO *>(param2);

                  // If both maximum sizes of the window are such that the
                  // window covers the entire screen, allow the client area
                  // to completely cover the screen, that is, allow the
                  // client area to become so large that the borders are
                  // just outside the screen.  This allows the window to
                  // be truely maximised, without the borders remaining
                  // visible.
                  if (    wmax == ScreenWidthPixels()
                      &&  hmax == ScreenHeightPixels())
                  {
                     wmax += border.Left() + border.Right();
                     hmax += NamelessTopBorder() + border.Bottom();
                  }

                  info->ptMinTrackSize.x = static_cast<LONG>(wmin);
                  info->ptMinTrackSize.y = static_cast<LONG>(hmin);
                  info->ptMaxTrackSize.x = static_cast<LONG>(wmax);
                  info->ptMaxTrackSize.y = static_cast<LONG>(hmax);
               }
            }
         }

         result = static_cast<int32>(0);   // message has been processed

         break;


      case WM_ACTIVATE:    // the window is either activated or de-activated

         if (created)
         {
            osbool  activate;  // is this window being activated?
                               //    (else: de-activated)
            osbool  error;     // error in handling parameters?

            error = OSFALSE;
            switch (LOWORD(param1))
            {
               case WA_ACTIVE:          // activated by any non-mouse method
               case WA_CLICKACTIVE:     // activated by a mouse click
                  activate = OSTRUE;
                  break;

               case WA_INACTIVE:        // de-activated
                  activate = OSFALSE;
                  break;

               default:                 // unknown code
                  error = OSTRUE;
                  break;
            }

            if (!error)
            {
               priv_windowhandle  other;   // other window
               // If this window is being activated, <other> is the window
               // that is being de-activated, and vice versa.

               other = reinterpret_cast<priv_windowhandle>(param2);
               if (activate)
               {
                  // Inform the window about its activation.
                  WindowActivation(other, messagetime, mousex, mousey);

                  // Inform the window that contains the mouse about the
                  // position of the mouse.  Note that the mouse needs not
                  // be in the window that is being activated.
                  {
                     POINT  mousepos;   // position of the mouse cursor,
                                        //    screen pixels

                     if (GetCursorPos(&mousepos))
                     {
                        HWND  mousewindow;   // window that contains mouse

                        mousewindow = WindowFromPoint(mousepos);
                        if (mousewindow != static_cast<HWND>(OSNULL))
                        {
                           DWORD  x;   // x part of mouse position
                           DWORD  y;   // y part of mouse position

                           x = static_cast<DWORD>
                                  (static_cast<WORD>(mousepos.x));
                           y = static_cast<DWORD>
                                  (static_cast<WORD>(mousepos.y));
                           SendMessage(mousewindow,
                                       WM_MOUSEMOVE,
                                       static_cast<WPARAM>(0),
                                       x + (y << 16));
                        }
                     }
                  }
               }
               else
                  // Inform the window about its de-activation.
                  WindowDeactivation(other, messagetime, mousex, mousey);
            }
         }

         result = static_cast<int32>(0);   // message has been processed

         break;


      case WM_PAINT:    // paint part of the window

         {
            PAINTSTRUCT  ps;   // painting information
            HDC          dc;   // DC for painting

            // Painting is necessary only if the window has an update
            // rectangle.
            // Painting is possible only if a display DC is available.
            if (    GetUpdateRect(reinterpret_cast<HWND>(priv_handle),
                                  static_cast<RECT *>(OSNULL),
                                  FALSE)
                &&  (dc = BeginPaint(reinterpret_cast<HWND>(priv_handle),
                                     &ps))
                             != static_cast<HDC>(OSNULL))
            {
               // Part of the window is invalid (there is an update region).
               // Paint that part.
               {
                  float  left;     // co-ordinates of the rectangle
                  float  right;    //    that must be painted, WA
                  float  top;
                  float  bottom;

                  PositionWPtoWA(ps.rcPaint.left, ps.rcPaint.top,
                                 border, area,
                                 left, top);
                  PositionWPtoWA(ps.rcPaint.right, ps.rcPaint.bottom,
                                 border, area,
                                 right, bottom);
                  Paint(left, right, top, bottom,
                        messagetime, mousex, mousey,
                        reinterpret_cast<ospainter>(dc));
               }

               // Painting has been completed.
               EndPaint(reinterpret_cast<HWND>(priv_handle), &ps);

               result = static_cast<int32>(0);   // message processed
            }
            else
               // There is no update region, or no DC is available.  Let the
               // default window procedure handle the painting.
               result = static_cast<int32>
                           (DefWindowProc
                               (reinterpret_cast<HWND>(priv_handle),
                                WM_PAINT,
                                static_cast<WPARAM>(param1),
                                static_cast<LPARAM>(param2)));
         }

         break;


      case WM_MOUSEMOVE:    // the mouse has moved

         {
            float   x;        // new x position of the mouse cursor, WA
            float   y;        // new y position of the mouse cursor, WA
            osbool  left;     // left mouse button is down?
            osbool  right;    // right mouse button is down?
            osbool  shift1;   // shift key 1 is down?
            osbool  shift2;   // shift key 2 is down?
            osbool  shift3;   // shift key 3 is down?

            GetMouseData(x, y,
                         left, right,
                         shift1, shift2, shift3,
                         param1, param2,
                         border, area);

            if (x != lastmx  ||  y != lastmy)
            {
               lastmx = x;
               lastmy = y;

               // Inform the window about the new position of the mouse.
               MousePosition(messagetime,
                             x, y,
                             left, right,
                             shift1, shift2, shift3);
            }
         }

         result = static_cast<int32>(0);   // message processed

         break;


      case WM_LBUTTONDOWN:   // the left mouse button was pressed

         {
            float   x;        // new x position of the mouse cursor, WA
            float   y;        // new y position of the mouse cursor, WA
            osbool  left;     // left mouse button is down?
            osbool  right;    // right mouse button is down?
            osbool  shift1;   // shift key 1 is down?
            osbool  shift2;   // shift key 2 is down?
            osbool  shift3;   // shift key 3 is down?

            GetMouseData(x, y,
                         left, right,
                         shift1, shift2, shift3,
                         param1, param2,
                         border, area);

            if (x != lastmx  ||  y != lastmy)
            {
               lastmx = x;
               lastmy = y;

               // Inform the window about the new position of the mouse.
               MousePosition(messagetime,
                             x, y,
                             OSTRUE, right,
                             shift1, shift2, shift3);
            }

            // Inform the window about the left button being pressed.
            MouseLeftSelect(messagetime,
                            x, y,
                            OSTRUE, right,
                            shift1, shift2, shift3);
         }

         result = static_cast<int32>(0);   // message processed

         break;


      case WM_LBUTTONDBLCLK:    // the left mouse button was double-clicked

         {
            float   x;        // new x position of the mouse cursor, WA
            float   y;        // new y position of the mouse cursor, WA
            osbool  left;     // left mouse button is down?
            osbool  right;    // right mouse button is down?
            osbool  shift1;   // shift key 1 is down?
            osbool  shift2;   // shift key 2 is down?
            osbool  shift3;   // shift key 3 is down?

            GetMouseData(x, y,
                         left, right,
                         shift1, shift2, shift3,
                         param1, param2,
                         border, area);

            if (x != lastmx  ||  y != lastmy)
            {
               lastmx = x;
               lastmy = y;

               // Inform the window about the new position of the mouse.
               MousePosition(messagetime,
                             x, y,
                             left, right,
                             shift1, shift2, shift3);
            }

            // Inform the window about the left button being double-clicked.
            MouseLeftActivate(messagetime,
                              x, y,
                              left, right,
                              shift1, shift2, shift3);
         }

         result = static_cast<int32>(0);   // message processed

         break;


      case WM_LBUTTONUP:   // the left mouse button was released

         {
            float   x;        // new x position of the mouse cursor, WA
            float   y;        // new y position of the mouse cursor, WA
            osbool  left;     // left mouse button is down?
            osbool  right;    // right mouse button is down?
            osbool  shift1;   // shift key 1 is down?
            osbool  shift2;   // shift key 2 is down?
            osbool  shift3;   // shift key 3 is down?

            GetMouseData(x, y,
                         left, right,
                         shift1, shift2, shift3,
                         param1, param2,
                         border, area);

            if (x != lastmx  ||  y != lastmy)
            {
               lastmx = x;
               lastmy = y;

               // Inform the window about the new position of the mouse.
               MousePosition(messagetime,
                             x, y,
                             OSFALSE, right,
                             shift1, shift2, shift3);
            }

            // Inform the window about the left button being released.
            MouseLeftRelease(messagetime,
                             x, y,
                             OSFALSE, right,
                             shift1, shift2, shift3);
         }

         result = static_cast<int32>(0);   // message processed

         break;


      case WM_RBUTTONDOWN:   // the right mouse button was pressed

         {
            float   x;        // new x position of the mouse cursor, WA
            float   y;        // new y position of the mouse cursor, WA
            osbool  left;     // left mouse button is down?
            osbool  right;    // right mouse button is down?
            osbool  shift1;   // shift key 1 is down?
            osbool  shift2;   // shift key 2 is down?
            osbool  shift3;   // shift key 3 is down?

            GetMouseData(x, y,
                         left, right,
                         shift1, shift2, shift3,
                         param1, param2,
                         border, area);

            if (x != lastmx  ||  y != lastmy)
            {
               lastmx = x;
               lastmy = y;

               // Inform the window about the new position of the mouse.
               MousePosition(messagetime,
                             x, y,
                             left, OSTRUE,
                             shift1, shift2, shift3);
            }

            // Inform the window about the right button being pressed.
            MouseRightSelect(messagetime,
                             x, y,
                             left, OSTRUE,
                             shift1, shift2, shift3);
         }

         result = static_cast<int32>(0);   // message processed

         break;


      case WM_RBUTTONDBLCLK:    // the right mouse button was double-clicked

         {
            float   x;        // new x position of the mouse cursor, WA
            float   y;        // new y position of the mouse cursor, WA
            osbool  left;     // left mouse button is down?
            osbool  right;    // right mouse button is down?
            osbool  shift1;   // shift key 1 is down?
            osbool  shift2;   // shift key 2 is down?
            osbool  shift3;   // shift key 3 is down?

            GetMouseData(x, y,
                         left, right,
                         shift1, shift2, shift3,
                         param1, param2,
                         border, area);

            if (x != lastmx  ||  y != lastmy)
            {
               lastmx = x;
               lastmy = y;

               // Inform the window about the new position of the mouse.
               MousePosition(messagetime,
                             x, y,
                             left, right,
                             shift1, shift2, shift3);
            }

            // Inform the window about right button being double-clicked.
            MouseRightActivate(messagetime,
                               x, y,
                               left, right,
                               shift1, shift2, shift3);
         }

         result = static_cast<int32>(0);   // message processed

         break;


      case WM_RBUTTONUP:   // the right mouse button was released

         {
            float   x;        // new x position of the mouse cursor, WA
            float   y;        // new y position of the mouse cursor, WA
            osbool  left;     // left mouse button is down?
            osbool  right;    // right mouse button is down?
            osbool  shift1;   // shift key 1 is down?
            osbool  shift2;   // shift key 2 is down?
            osbool  shift3;   // shift key 3 is down?

            GetMouseData(x, y,
                         left, right,
                         shift1, shift2, shift3,
                         param1, param2,
                         border, area);

            if (x != lastmx  ||  y != lastmy)
            {
               lastmx = x;
               lastmy = y;

               // Inform the window about the new position of the mouse.
               MousePosition(messagetime,
                             x, y,
                             left, OSFALSE,
                             shift1, shift2, shift3);
            }

            // Inform the window about the right button being released.
            MouseRightRelease(messagetime,
                              x, y,
                              left, OSFALSE,
                              shift1, shift2, shift3);
         }

         result = static_cast<int32>(0);   // message processed

         break;


      case WM_MOVE:    // window has moved
         // This message should not be sent.  It has been handled when the
         // message <WM_WINDOWPOSCHANGED> was handled.

         // Fall through to the <default> label.


      case WM_SIZE:    // window size has changed
         // This message should not be sent.  It has been handled when the
         // message <WM_WINDOWPOSCHANGED> was handled.

         // Fall through to the <default> label.


      default:    // any other message

         // The message is not handled by us.  Pass it on to the default
         // message handler that is provided by Windows.
         result = static_cast<int32>
                     (DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                                    static_cast<UINT>(message),
                                    static_cast<WPARAM>(param1),
                                    static_cast<LPARAM>(param2)));
         break;
   }

   return result;

}  // osbasicwindow::priv_MessageHandler


/**************************************************************************
 * Function: osbasicwindow::Write                                         *
 **************************************************************************/

osresult                       // result code
osbasicwindow::Write           // writes text
(float                 x,      // I: x co-ordinate of the text, WA
 float                 y,      // I: y co-ordinate of the text, WA
 wchar_t const *       text,   // I: text to write
 oswindowfont const &  font,   // I: font to use
 oscolor const &       color,  // I: color of the text
 ospainter const &     p)      // I: window painter

{  // osbasicwindow::Write

   osresult  result;     // result code
   HDC       dc;         // device context for writing the text
   COLORREF  orgcolor;   // original text color

   dc = reinterpret_cast<HDC>(p);

   // Select the text color <color>.
   {
      int  red;     // red   component of <color>, 0..255
      int  green;   // green component of <color>, 0..255
      int  blue;    // blue  component of <color>, 0..255

      RoundFloatToIntegral(color.R()*255.0, &red  );
      RoundFloatToIntegral(color.G()*255.0, &green);
      RoundFloatToIntegral(color.B()*255.0, &blue );

      orgcolor = SetTextColor(dc, RGB(red, green, blue));
   }

   if (orgcolor != CLR_INVALID)
   {
      int  orgbkmode;   // original background mode for writing text

      // Select transparent writing.
      orgbkmode = SetBkMode(dc, TRANSPARENT);
      if (orgbkmode != 0)
      {
         UINT  orgalign;   // original text-alignment mode

         // Select left-and-baseline alignment, without updating of the
         // current output position.
         orgalign = SetTextAlign(dc, TA_LEFT | TA_BASELINE | TA_NOUPDATECP);
         if (orgalign != GDI_ERROR)
         {
            HFONT  orgfont;   // original font

            // Select the font <font> into the device context.
            orgfont = static_cast<HFONT>
                         (SelectObject
                            (dc, static_cast<HGDIOBJ>(font.Handle())));

            {
               int  length;   // length of the text <text>

               // Obtain <length>.
               {
                  unsigned long  len;   // length of the text <text>

                  len = StringLength(text);
                  if (len <= static_cast<unsigned long>(INT_MAX))
                  {
                     length = static_cast<int>(len);
                     result = OSRESULT_OK;
                  }
                  else
                     result = OSINTERNAL_WRITE;
               }

               if (result == OSRESULT_OK)
               {
                  #ifdef OSRTL_UNICODE
                     wchar_t const *  t = text;   // text to write
                  #else
                     char *  t;   // text to write

                     result = StringDuplicate
                                 (L"Copy of text to write to a window",
                                  text, &t);
                  #endif

                  if (result == OSRESULT_OK)
                  {
                     int  insert;   // number of pixels to insert between
                                    //    the characters of <t>

                     // Obtain the widths of the text.
                     {
                        float  wdesired;   // desired text width, FS
                        float  wactual;    // actual  text width, FS

                        {
                           float  dummy;

                           result = font.TextWidth(text, p,
                                                   dummy, dummy,
                                                   wactual);
                        }

                        if (result == OSRESULT_OK)
                        {
                           {
                              float  dummy;

                              result = font.TextWidthIdeal(text, p,
                                                           dummy, dummy,
                                                           wdesired);
                           }

                           if (result == OSRESULT_OK)
                           {
                              {
                                 int  dummy;

                                 result = SizeFStoSP(wdesired-wactual,
                                                     0.0,
                                                     insert,
                                                     dummy);

                              }
                           }
                        }
                     }

                     if (result == OSRESULT_OK)
                        result = Write(t, text, length,
                                       insert, x, y, p, font);

                     #ifndef OSRTL_UNICODE
                        // De-allocate <t>.
                        {
                           osresult  dealloc;  // result of de-allocation

                           {
                              osheap<char>  heap;

                              dealloc = heap.Deallocate(t);
                           }

                           if (result == OSRESULT_OK)
                              result = dealloc;
                           else
                              if (dealloc != OSRESULT_OK)
                                 OsDebugErrorMessage
                                    (L"Cannot de-allocate the copy of "
                                     L"the string\nthat should have "
                                     L"been written to a window.");
                        }
                     #endif
                  }
               }
            }

            // Restore the original font.
            SelectObject(dc, static_cast<HGDIOBJ>(orgfont));

            // Restore the original text-alignment mode.
            if (    SetTextAlign(dc, orgalign) != (TA_LEFT | TA_BASELINE
                                                           | TA_NOUPDATECP)
                &&  result == OSRESULT_OK)
               result = OSERROR_NORSRC_WRITE;
         }
         else
            result = OSERROR_NORSRC_WRITE;

         // Restore the original background mode for writing text.
         if (    SetBkMode(dc, orgbkmode) != TRANSPARENT
             &&  result == OSRESULT_OK)
            result = OSERROR_NORSRC_WRITE;
      }
      else
         result = OSERROR_NORSRC_WRITE;

      // Restore the original text color.
      if (    SetTextColor(dc, orgcolor) == CLR_INVALID
          &&  result == OSRESULT_OK)
         result = OSERROR_NORSRC_WRITE;
   }
   else
      result = OSERROR_NORSRC_WRITE;

   return result;

}  // osbasicwindow::Write


/**************************************************************************
 * Function: osbasicwindow::Line                                          *
 **************************************************************************/

osresult                     // result code
osbasicwindow::Line          // draws a straight line
(float              x0,      // I: x co-ordinate of start point (incl.), WA
 float              y0,      // I: y co-ordinate of start point (incl.), WA
 float              x1,      // I: x co-ordinate of end   point (excl.), WA
 float              y1,      // I: y co-ordinate of end   point (excl.), WA
 oscolor const &    color,   // I: color of the line
 ospainter const &  p)       // I: window painter

{  // osbasicwindow::Line

   osresult  result;   // result code
   int       wpx0;     // x co-ordinate of start point (inclusive), WP
   int       wpy0;     // y co-ordinate of start point (inclusive), WP
   int       wpx1;     // x co-ordinate of end   point (exclusive), WP
   int       wpy1;     // y co-ordinate of end   point (exclusive), WP

   if (    (result = PositionWAtoWP(x0, y0, border, area, wpx0, wpy0))
              == OSRESULT_OK
       &&  (result = PositionWAtoWP(x1, y1, border, area, wpx1, wpy1))
              == OSRESULT_OK)
   {
      // Filter out zero-size lines.  Consider an attempt to draw such
      // a line normal.
      if (wpx1 != wpx0  ||  wpy1 != wpy0)
      {
         HPEN  pen;   // pen of color <c>

         // Create the pen <pen>.
         {
            int  red;     // red   component of <color>, 0..255
            int  green;   // green component of <color>, 0..255
            int  blue;    // blue  component of <color>, 0..255

            RoundFloatToIntegral(color.R()*255.0, &red  );
            RoundFloatToIntegral(color.G()*255.0, &green);
            RoundFloatToIntegral(color.B()*255.0, &blue );

            pen = CreatePen(PS_SOLID, 0, RGB(red, green, blue));
         }

         if (pen != static_cast<HPEN>(OSNULL))
         {
            HDC  dc;   // device context for drawing the line

            dc = reinterpret_cast<HDC>(p);

            // Select the pen into the device context.
            pen = static_cast<HPEN>
                     (SelectObject(dc, static_cast<HGDIOBJ>(pen)));

            if (    !MoveToEx(dc, wpx0, wpy0, static_cast<POINT *>(OSNULL))
                ||  !LineTo(dc, wpx1, wpy1))
               result = OSERROR_NORSRC_DRAW;

            // Select the pen out of the device context.
            pen = static_cast<HPEN>
                     (SelectObject(dc, static_cast<HGDIOBJ>(pen)));

            // Delete the pen.
            if (!DeleteObject(static_cast<HGDIOBJ>(pen)))
               if (result == OSRESULT_OK)
                  result = OSINTERNAL_DRAW;
               else
                  OsDebugErrorMessage(L"Cannot delete a pen after\n"
                                      L"drawing a straight line.");
         }
         else
            result = OSERROR_NORSRC_DRAW;
      }
   }

   return result;

}  // osbasicwindow::Line


/**************************************************************************
 * Function: osbasicwindow::OutlineRectangle                              *
 **************************************************************************/

osresult                          // result code
osbasicwindow::OutlineRectangle   // draws the outline of a rectangle
(float              left,         // I: left   edge of the rectangle, WA
 float              right,        // I: right  edge of the rectangle, WA
 float              top,          // I: top    edge of the rectangle, WA
 float              bottom,       // I: bottom edge of the rectangle, WA
 oscolor const &    color,        // I: color of the rectangle
 ospainter const &  p)            // I: window painter

{  // osbasicwindow::OutlineRectangle

   // We might draw the rectangle by calling <Line> four times.  However,
   // that would not be the fastest way to draw the rectangle.  Therefore,
   // replicate the basic operations of <Line>.

   osresult  result;     // result code
   int       wpleft;     // left   edge of the rectangle, WP
   int       wpright;    // right  edge of the rectangle, WP
   int       wptop;      // top    edge of the rectangle, WP
   int       wpbottom;   // bottom edge of the rectangle, WP

   if (    (result = PositionWAtoWP(left, top,
                                    border, area,
                                    wpleft, wptop))     == OSRESULT_OK
       &&  (result = PositionWAtoWP(right, bottom,
                                    border, area,
                                    wpright, wpbottom)) == OSRESULT_OK)
   {
      // Filter out zero-size rectangles.  Consider an attempt to draw such
      // a rectangle normal.
      if (wpleft != wpright  ||  wptop != wpbottom)
      {
         HPEN  pen;   // pen of color <color>

         // Create the pen <pen>.
         {
            int  red;     // red   component of <color>, 0..255
            int  green;   // green component of <color>, 0..255
            int  blue;    // blue  component of <color>, 0..255

            RoundFloatToIntegral(color.R()*255.0, &red  );
            RoundFloatToIntegral(color.G()*255.0, &green);
            RoundFloatToIntegral(color.B()*255.0, &blue );

            pen = CreatePen(PS_SOLID, 0, RGB(red, green, blue));
         }

         if (pen != static_cast<HPEN>(OSNULL))
         {
            HDC  dc;   // device context for drawing the rectangle

            dc = reinterpret_cast<HDC>(p);

            // Select the pen into the device context.
            pen = static_cast<HPEN>
                     (SelectObject(dc, static_cast<HGDIOBJ>(pen)));

            // Draw the horizontal lines of the rectangle.
            if (wpright != wpleft)
               if (    !MoveToEx(dc, wpleft, wptop,
                                               static_cast<POINT *>(OSNULL))
                   ||  !LineTo(dc, wpright, wptop)
                   ||  !MoveToEx(dc, wpleft, wpbottom,
                                               static_cast<POINT *>(OSNULL))
                   ||  !LineTo(dc, wpright, wpbottom))
                  result = OSERROR_NORSRC_DRAW;

            // Draw the vertical lines of the rectangle.
            if (wpbottom != wptop)
               if (    !MoveToEx(dc, wpleft, wptop,
                                               static_cast<POINT *>(OSNULL))
                   ||  !LineTo(dc, wpleft, wpbottom)
                   ||  !MoveToEx(dc, wpright, wptop,
                                               static_cast<POINT *>(OSNULL))
                   ||  !LineTo(dc, wpright, wpbottom+1))
                  result = OSERROR_NORSRC_DRAW;

            // Select the pen out of the device context.
            pen = static_cast<HPEN>
                     (SelectObject(dc, static_cast<HGDIOBJ>(pen)));

            // Delete the pen.
            if (!DeleteObject(static_cast<HGDIOBJ>(pen)))
               if (result == OSRESULT_OK)
                  result = OSINTERNAL_DRAW;
               else
                  OsDebugErrorMessage(L"Cannot delete a pen after\n"
                                      L"drawing a straight line.");
         }
         else
            result = OSERROR_NORSRC_DRAW;
      }
   }

   return result;

}  // osbasicwindow::OutlineRectangle


/**************************************************************************
 * Function: osbasicwindow::FilledRectangle                               *
 **************************************************************************/

osresult                         // result code
osbasicwindow::FilledRectangle   // draws a filled rectangle
(float              left,        // I: left   edge of the rectangle, WA
 float              right,       // I: right  edge of the rectangle, WA
 float              top,         // I: rop    edge of the rectangle, WA
 float              bottom,      // I: bottom edge of the rectangle, WA
 oscolor const &    color,       // I: color of the rectangle
 ospainter const &  p)           // I: window painter

{  // osbasicwindow::FilledRectangle

   osresult  result;    // result code
   int       wpleft;     // left   edge of the rectangle, WP
   int       wpright;    // right  edge of the rectangle, WP
   int       wptop;      // top    edge of the rectangle, WP
   int       wpbottom;   // bottom edge of the rectangle, WP

   if (    (result = PositionWAtoWP(left, top,
                                    border, area,
                                    wpleft, wptop))     == OSRESULT_OK
       &&  (result = PositionWAtoWP(right, bottom,
                                    border, area,
                                    wpright, wpbottom)) == OSRESULT_OK)
   {
      // Filter out empty (zero-size) rectangles.  Consider an attempt to
      // draw such an empty rectange normal.
      if (wpleft != wpright  &&  wptop != wpbottom)
      {
         HBRUSH  brush;   // brush of color <color>

         // Create the brush <brush>.
         {
            int  red;     // red   component of <color>, 0..255
            int  green;   // green component of <color>, 0..255
            int  blue;    // blue  component of <color>, 0..255

            RoundFloatToIntegral(color.R()*255.0, &red  );
            RoundFloatToIntegral(color.G()*255.0, &green);
            RoundFloatToIntegral(color.B()*255.0, &blue );

            brush = CreateSolidBrush(RGB(red, green, blue));
         }

         if (brush != static_cast<HBRUSH>(OSNULL))
         {
            RECT  r;   // rectangle to be filled

            if (wpleft < wpright)
            {
               r.left  = static_cast<LONG>(wpleft );
               r.right = static_cast<LONG>(wpright);
            }
            else
            {
               r.left  = static_cast<LONG>(wpright);
               r.right = static_cast<LONG>(wpleft );
            }

            if (wptop < wpbottom)
            {
               r.top    = static_cast<LONG>(wptop   );
               r.bottom = static_cast<LONG>(wpbottom);
            }
            else
            {
               r.top    = static_cast<LONG>(wpbottom);
               r.bottom = static_cast<LONG>(wptop   );
            }

            if (!FillRect(reinterpret_cast<HDC>(p), &r, brush))
               result = OSERROR_NORSRC_DRAW;

            if (!DeleteObject(static_cast<HGDIOBJ>(brush)))
               if (result == OSRESULT_OK)
                  result = OSINTERNAL_DRAW;
               else
                  OsDebugErrorMessage(L"Cannot delete a brush after\n"
                                      L"drawing a filled rectangle.");
         }
         else
            result = OSERROR_NORSRC_DRAW;
      }
   }

   return result;

}  // osbasicwindow::FilledRectangle


/**************************************************************************
 * Function: osbasicwindow::WindowCreation                                *
 **************************************************************************/

osresult                        // result code
osbasicwindow::WindowCreation   // the window is being created
(long   ,                       // -: time of window creation
 float  ,                       // -: x position of the mouse
 float  )                       // -: y position of the mouse

{  // osbasicwindow::WindowCreation

   return OSRESULT_OK;

}  // osbasicwindow::WindowCreation


/**************************************************************************
 * Function: osbasicwindow::WindowDestruction                             *
 **************************************************************************/

void
osbasicwindow::WindowDestruction   // the window is being destroyed
(long   ,                          // -: time of window creation
 float  ,                          // -: x position of the mouse
 float  )                          // -: y position of the mouse

{  // osbasicwindow::WindowDestruction

   // Do not react to this message.
   (void)0;

}  // osbasicwindow::WindowDestruction


/**************************************************************************
 * Function: osbasicwindow::WindowPosSize                                 *
 **************************************************************************/

void
osbasicwindow::WindowPosSize   // position and size of window have changed
(long   ,                      // -: time of change in position and size
 float  ,                      // -: x position of the mouse
 float  )                      // -: y position of the mouse

{  // osbasicwindow::WindowPosSize

   // Do not react to this message.
   (void)0;

}  // osbasicwindow::WindowPosSize


/**************************************************************************
 * Function: osbasicwindow::WindowActivation                              *
 **************************************************************************/

void
osbasicwindow::WindowActivation   // the window is being activated
(priv_windowhandle  ,             // -: window that is being de-activated
 long               ,             // -: time of activation
 float              ,             // -: x position of the mouse
 float              )             // -: y position of the mouse

{  // osbasicwindow::WindowActivation

   // Do not react to this message.
   (void)0;

}  // osbasicwindow::WindowActivation


/**************************************************************************
 * Function: osbasicwindow::WindowDeactivation                            *
 **************************************************************************/

void
osbasicwindow::WindowDeactivation   // the window is being de-activated
(priv_windowhandle  ,               // I: window that is being activated
 long               ,               // -: time of de-activation
 float              ,               // -: x position of the mouse
 float              )               // -: y position of the mouse

{  // osbasicwindow::WindowDeactivation

   // Do not react to this message.
   (void)0;

}  // osbasicwindow::WindowDeactivation


/**************************************************************************
 * Function: osbasicwindow::MousePosition                                 *
 **************************************************************************/

void
osbasicwindow::MousePosition   // the position of the mouse has changed
(long    ,                     // -: time of the change of mouse position
 float   x,                    // I: new x position of the mouse
 float   y,                    // I: new y position of the mouse
 osbool  ,                     // -: left  mouse button is down?
 osbool  ,                     // -: right mouse button is down?
 osbool  ,                     // -: shift key 1 is down?
 osbool  ,                     // -: shift key 2 is down?
 osbool  )                     // -: shift key 3 is down?

{  // osbasicwindow::MousePosition

   if (x >= 0.0  &&  x < 1.0  &&  y >= 0.0  &&  y < 1.0)
      // Select an arrow as the mouse cursor.  It's the least we can do...
      if (SelectMouseCursor(MOUSE_ARROW) != OSRESULT_OK)
         OsDebugErrorMessage(L"Cannot set the mouse cursor to an arrow\n"
                             L"in osbasicwindow::MousePosition .");

}  // osbasicwindow::MousePosition


/**************************************************************************
 * Function: osbasicwindow::MouseLeftSelect                               *
 **************************************************************************/

void
osbasicwindow::MouseLeftSelect   // the left mouse button was used to select
(long    ,                       // -: time of selection
 float   ,                       // -: new x position of the mouse
 float   ,                       // -: new y position of the mouse
 osbool  ,                       // -: left  mouse button is down?
 osbool  ,                       // -: right mouse button is down?
 osbool  ,                       // -: shift key 1 is down?
 osbool  ,                       // -: shift key 2 is down?
 osbool  )                       // -: shift key 3 is down?

{  // osbasicwindow::MouseLeftSelect

   // Pretend that the user pressed the left mouse button in the title bar
   // of the window.  To make sure that Windows does not assign the event
   // to one of the system areas of the title bar, and to prevent that a
   // resident utility that resides in the title bar intercepts the message,
   // the position of the event is set to (0x37A6, 0x37A6); a title bar
   // cannot possibly cover that position.
   // The value 0x37A6 is a little less than half the maximum value of a
   // 16-bit signed integer; this prevents possible processing problems with
   // co-ordinates that are close to the maximum representable value of
   // 0x7FFF .  It is not exactly half the maximum value of a 16-bit signed
   // integer (0x3FFF) to prevent this program from interfering with other
   // programs which might be monitoring that value.
   DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                 WM_NCLBUTTONDOWN,
                 (WPARAM)HTCAPTION,
                 (LPARAM)0x37A637A6);

}  // osbasicwindow::MouseLeftSelect


/**************************************************************************
 * Function: osbasicwindow::MouseLeftActivate                             *
 **************************************************************************/

void
osbasicwindow::MouseLeftActivate   // left mouse button was used to activate
(long    ,                         // -: time of activation
 float   ,                         // -: new x position of the mouse
 float   ,                         // -: new y position of the mouse
 osbool  ,                         // -: left  mouse button is down?
 osbool  ,                         // -: right mouse button is down?
 osbool  ,                         // -: shift key 1 is down?
 osbool  ,                         // -: shift key 2 is down?
 osbool  )                         // -: shift key 3 is down?

{  // osbasicwindow::MouseLeftActivate

   // Pretend that the user double-clicked the left mouse button in the
   // title bar of the window.  To make sure that Windows does not assign
   // the event to one of the system areas of the title bar, and to prevent
   // that a resident utility that resides in the title bar intercepts the
   // message, the position of the event is set to (0x37A6, 0x37A6); a title
   // bar cannot possibly cover that position.
   // The value 0x37A6 is a little less than half the maximum value of a
   // 16-bit signed integer; this prevents possible processing problems with
   // co-ordinates that are close to the maximum representable value of
   // 0x7FFF .  It is not exactly half the maximum value of a 16-bit signed
   // integer (0x3FFF) to prevent this program from interfering with other
   // programs which might be monitoring that value.
   DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                 WM_NCLBUTTONDBLCLK,
                 (WPARAM)HTCAPTION,
                 (LPARAM)0x37A637A6);

}  // osbasicwindow::MouseLeftActivate


/**************************************************************************
 * Function: osbasicwindow::MouseLeftRelease                              *
 **************************************************************************/

void
osbasicwindow::MouseLeftRelease   // the left mouse button was released
(long    ,                        // -: time of release of left mouse button
 float   ,                        // -: new x position of the mouse
 float   ,                        // -: new y position of the mouse
 osbool  ,                        // -: left  mouse button is down?
 osbool  ,                        // -: right mouse button is down?
 osbool  ,                        // -: shift key 1 is down?
 osbool  ,                        // -: shift key 2 is down?
 osbool  )                        // -: shift key 3 is down?

{  // osbasicwindow::MouseLeftRelease

   // Pretend that the user released the left mouse button in the title bar
   // of the window.  To make sure that Windows does not assign the event
   // to one of the system areas of the title bar, and to prevent that a
   // resident utility that resides in the title bar intercepts the message,
   // the position of the event is set to (0x37A6, 0x37A6); a title bar
   // cannot possibly cover that position.
   // The value 0x37A6 is a little less than half the maximum value of a
   // 16-bit signed integer; this prevents possible processing problems with
   // co-ordinates that are close to the maximum representable value of
   // 0x7FFF .  It is not exactly half the maximum value of a 16-bit signed
   // integer (0x3FFF) to prevent this program from interfering with other
   // programs which might be monitoring that value.
   DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                 WM_NCLBUTTONUP,
                 (WPARAM)HTCAPTION,
                 (LPARAM)0x37A637A6);

}  // osbasicwindow::MouseLeftRelease


/**************************************************************************
 * Function: osbasicwindow::MouseRightSelect                              *
 **************************************************************************/

void
osbasicwindow::MouseRightSelect   // right mouse button was used to select
(long    ,                        // -: time of selection
 float   ,                        // -: new x position of the mouse
 float   ,                        // -: new y position of the mouse
 osbool  ,                        // -: left  mouse button is down?
 osbool  ,                        // -: right mouse button is down?
 osbool  ,                        // -: shift key 1 is down?
 osbool  ,                        // -: shift key 2 is down?
 osbool  )                        // -: shift key 3 is down?

{  // osbasicwindow::MouseRightSelect

   // Pretend that the user pressed the right mouse button in the title bar
   // of the window.  To make sure that Windows does not assign the event
   // to one of the system areas of the title bar, and to prevent that a
   // resident utility that resides in the title bar intercepts the message,
   // the position of the event is set to (0x37A6, 0x37A6); a title bar
   // cannot possibly cover that position.
   // The value 0x37A6 is a little less than half the maximum value of a
   // 16-bit signed integer; this prevents possible processing problems with
   // co-ordinates that are close to the maximum representable value of
   // 0x7FFF .  It is not exactly half the maximum value of a 16-bit signed
   // integer (0x3FFF) to prevent this program from interfering with other
   // programs which might be monitoring that value.
   DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                 WM_NCRBUTTONDOWN,
                 (WPARAM)HTCAPTION,
                 (LPARAM)0x37A637A6);

}  // osbasicwindow::MouseRightSelect


/**************************************************************************
 * Function: osbasicwindow::MouseRightActivate                            *
 **************************************************************************/

void
osbasicwindow::MouseRightActivate   // right mouse button used to activate
(long    ,                          // -: time of activation
 float   ,                          // -: new x position of the mouse
 float   ,                          // -: new y position of the mouse
 osbool  ,                          // -: left  mouse button is down?
 osbool  ,                          // -: right mouse button is down?
 osbool  ,                          // -: shift key 1 is down?
 osbool  ,                          // -: shift key 2 is down?
 osbool  )                          // -: shift key 3 is down?

{  // osbasicwindow::MouseRightActivate

   // Pretend that the user double-clicked the right mouse button in the
   // title bar of the window.  To make sure that Windows does not assign
   // the event to one of the system areas of the title bar, and to prevent
   // that a resident utility that resides in the title bar intercepts the
   // message, the position of the event is set to (0x37A6, 0x37A6); a title
   // bar / cannot possibly cover that position.
   // The value 0x37A6 is a little less than half the maximum value of a
   // 16-bit signed integer; this prevents possible processing problems with
   // co-ordinates that are close to the maximum representable value of
   // 0x7FFF .  It is not exactly half the maximum value of a 16-bit signed
   // integer (0x3FFF) to prevent this program from interfering with other
   // programs which might be monitoring that value.
   DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                 WM_NCRBUTTONDBLCLK,
                 (WPARAM)HTCAPTION,
                 (LPARAM)0x37A637A6);

}  // osbasicwindow::MouseRightActivate


/**************************************************************************
 * Function: osbasicwindow::MouseRightRelease                             *
 **************************************************************************/

void
osbasicwindow::MouseRightRelease   // the right mouse button was released
(long    ,                         // -: time of release of right button
 float   ,                         // -: new x position of the mouse
 float   ,                         // -: new y position of the mouse
 osbool  ,                         // -: left  mouse button is down?
 osbool  ,                         // -: right mouse button is down?
 osbool  ,                         // -: shift key 1 is down?
 osbool  ,                         // -: shift key 2 is down?
 osbool  )                         // -: shift key 3 is down?

{  // osbasicwindow::MouseRightRelease

   // Pretend that the user released the right mouse button in the title bar
   // of the window.  To make sure that Windows does not assign the event
   // to one of the system areas of the title bar, and to prevent that a
   // resident utility that resides in the title bar intercepts the message,
   // the position of the event is set to (0x37A6, 0x37A6); a title bar
   // cannot possibly cover that position.
   // The value 0x37A6 is a little less than half the maximum value of a
   // 16-bit signed integer; this prevents possible processing problems with
   // co-ordinates that are close to the maximum representable value of
   // 0x7FFF .  It is not exactly half the maximum value of a 16-bit signed
   // integer (0x3FFF) to prevent this program from interfering with other
   // programs which might be monitoring that value.
   DefWindowProc(reinterpret_cast<HWND>(priv_handle),
                 WM_NCRBUTTONUP,
                 (WPARAM)HTCAPTION,
                 (LPARAM)0x37A637A6);

}  // osbasicwindow::MouseRightRelease


/**************************************************************************
 * Function: osbasicwindow::CanBeClosed                                   *
 **************************************************************************/

osbool                       // window may be closed?
osbasicwindow::CanBeClosed   // checks if it is OK to close the window
(long   ,                    // -: time of the closure request
 float  ,                    // -: x position of the mouse
 float  )                    // -: y position of the mouse

{  // osbasicwindow::CanBeClosed

   return OSTRUE;

}  // osbasicwindow::CanBeClosed


/**************************************************************************
 * Function: osbasicwindow::SizeLimits                                    *
 **************************************************************************/

void
osbasicwindow::SizeLimits   // ask for size limits for the window
(float &  minwidth,         // O: minimum width  of the window, 0..1
 float &  maxwidth,         // O: maximum width  of the window, 0..1
 float &  minheight,        // O: minimum height of the window, 0..1
 float &  maxheight)        // O: maximum height of the window, 0..1

{  // osbasicwindow::SizeLimits

   minwidth = minheight = 0.0;
   maxwidth = maxheight = 1.0;

}  // osbasicwindow::SizeLimits


/**************************************************************************
 * Function: osbasicwindow::Paint                                         *
 **************************************************************************/

osresult                      // result code
osbasicwindow::Paint          // paints an area of the window
(float              left,     // I: left   edge of area that needs painting
 float              right,    // I: right  edge of area that needs painting
 float              top,      // I: top    edge of area that needs painting
 float              bottom,   // I: bottom edge of area that needs painting
 long               ,         // -: time of the paint request
 float              ,         // -: x position of the mouse
 float              ,         // -: y position of the mouse
 ospainter const &  p)        // I: paint-specific data

{  // osbasicwindow::Paint

   // Make the entire rectangle black.
   return FilledRectangle(left, right, top, bottom,
                          oscolor(0.0, 0.0, 0.0),
                          p);

}  // osbasicwindow::Paint


/**************************************************************************
 * Function: osbasicwindow::priv_Create                                   *
 **************************************************************************/

osresult                     // result code
osbasicwindow::priv_Create   // creates an <osbasicwindow>
(float            left,      // I: x co-ordinate of left   edge, AS
 float            right,     // I: x co-ordinate of right  edge, AS
 float            top,       // I: y co-ordinate of top    edge, AS
 float            bottom,    // I: y co-ordinate of bottom edge, AS
 wchar_t const *  name,      // I: name of the window
 osbool           vis)       // I: must the window be visible?

{  // osbasicwindow::priv_Create

   osresult  result = OSRESULT_OK;   // result code

   if (created)
      result = OSINTERNAL_WINDOWADMIN;

   // Define the function pointers that are used throughout the code.
   if (result == OSRESULT_OK)
   {
      if (name != static_cast<wchar_t const *>(OSNULL))
      {
         border.Left   = NamedLeftBorder;
         border.Right  = NamedRightBorder;
         border.Top    = NamedTopBorder;
         border.Bottom = NamedBottomBorder;
      }
      else
      {
         border.Left   = NamelessLeftBorder;
         border.Right  = NamelessRightBorder;
         border.Top    = NamelessTopBorder;
         border.Bottom = NamelessBottomBorder;
      }
   }

   // Correct the co-ordinates.
   if (result == OSRESULT_OK)
   {
      if ((area.left = left) > (area.right = right))
      {
         area.left  = right;
         area.right = left;
      }

      if ((area.top = top) > (area.bottom = bottom))
      {
         area.top    = bottom;
         area.bottom = top;
      }
   }

   // Actual operations.
   if (result == OSRESULT_OK)
   {
      int  bleft;     // x co-ordinate of left edge including border, SP
      int  btop;      // y co-ordinate of top edge including border, SP
      int  bwidth;    // width of window including border, SP
      int  bheight;   // height of window including border, SP

      // Convert the screen co-ordinates into screen pixels.
      {
         result = PositionAStoSP(area.left, area.top, border, bleft, btop);
         if (result == OSRESULT_OK)
         {
            bleft -= border.Left();
            btop  -= border.Top();

            {
               int  bright;   // x co-ordinate of right edge, SP
               int  bbottom;  // y co-ordinate of bottom edge, SP

               result = PositionAStoSP(area.right, area.bottom, border,
                                                           bright, bbottom);
               if (result == OSRESULT_OK)
               {
                  bright  += border.Right();
                  bbottom += border.Bottom();

                  bwidth  = bright  - bleft;
                  bheight = bbottom - btop;
               }
            }
         }
      }  // Convert the screen co-ordinates into screen pixels.

      if (result == OSRESULT_OK)   // were the co-ordinates converted?
      {
         // Make a private copy of the name of the window.
         title = StringDuplicate(L"window title", name);
         if (    name != static_cast<wchar_t const *>(OSNULL)
             &&  title == static_cast<wchar_t *>(OSNULL))
            result = OSERROR_NOMEMORY;
         else
         {
            windowadmin                 admin;    // window administrator
            windowadmin::windowclassid  id;       // ID of the window class

            // Administer creation of the window.
            result = admin.Create(GenericHandler, &id);
            if (result == OSRESULT_OK)
            {
               HWND  window;   // handle of the new window

               visible          = vis;
               forcedestruction = OSFALSE;

               // Create the window.
               #ifdef OSRTL_UNICODE
               {
                  {
                     DWORD  style;   // style of the window

                     if (name != static_cast<wchar_t const *>(OSNULL))
                        style = WS_OVERLAPPEDWINDOW;
                     else
                        style = WS_POPUPWINDOW | WS_THICKFRAME;

                     if (visible)
                        style |= WS_VISIBLE;

                     window = CreateWindowEx
                                 (WS_EX_ACCEPTFILES | WS_EX_NOPARENTNOTIFY,
                                  id,
                                  title,
                                  style,
                                  bleft,
                                  btop,
                                  bwidth,
                                  bheight,
                                  reinterpret_cast<HWND>(masterhandle),
                                  static_cast<HMENU>(OSNULL),
                                  reinterpret_cast<HINSTANCE>
                                     (ProgramInstance()),
                                  static_cast<void *>(this));
                  }

                  if (window == static_cast<HWND>(OSNULL))
                     result = OSERROR_NORSRC_WINDOW;
                  else
                     if (window != reinterpret_cast<HWND>(priv_handle))
                     {
                        // This situation should be impossible.
                        DestroyWindow(window);
                        result = OSINTERNAL_WINDOWADMIN;
                     }
               }
               #else
               {
                  char *  idansi;      // ANSI equivalent of <id>
                  char *  titleansi;   // ANSI equivalent of <title>

                  result = StringDuplicate
                              (L"ANSI equivalent of window title",
                               title,
                               &titleansi);
                  if (result == OSRESULT_OK)
                     result = StringDuplicate
                                 (L"ANSI equivalent of window class ID",
                                  id,
                                  &idansi);
                  else
                     idansi = static_cast<char *>(OSNULL);

                  if (result == OSRESULT_OK)
                  {
                     {
                        DWORD  style;   // style of the window

                        if (name != static_cast<wchar_t const *>(OSNULL))
                           style = WS_OVERLAPPEDWINDOW;
                        else
                           style = WS_POPUPWINDOW | WS_THICKFRAME;

                        if (visible)
                           style |= WS_VISIBLE;

                        window = CreateWindowExA
                                    (WS_EX_ACCEPTFILES
                                        | WS_EX_NOPARENTNOTIFY,
                                     idansi,
                                     titleansi,
                                     style,
                                     bleft,
                                     btop,
                                     bwidth,
                                     bheight,
                                     reinterpret_cast<HWND>(masterhandle),
                                     static_cast<HMENU>(OSNULL),
                                     reinterpret_cast<HINSTANCE>
                                                       (ProgramInstance()),
                                     static_cast<void *>(this));
                     }

                     if (window == static_cast<HWND>(OSNULL))
                        result = OSERROR_NORSRC_WINDOW;
                     else
                        if (window != reinterpret_cast<HWND>(priv_handle))
                        {
                           // This situation should be impossible.
                           DestroyWindow(window);
                           result = OSINTERNAL_WINDOWADMIN;
                        }
                  }  // if (result == OSRESULT_OK)

                  // De-allocate <titleansi> and <idansi>.
                  {
                     osresult  dealloc_title;   // result code
                     osresult  dealloc_id;      // result code

                     // Do the de-allocations.
                     {
                        osheap<char>  heap;

                        dealloc_title = heap.Deallocate(titleansi);
                        dealloc_id    = heap.Deallocate(idansi   );
                     }

                     if (result == OSRESULT_OK)
                     {
                        result = dealloc_title;
                        if (result == OSRESULT_OK)
                           result = dealloc_id;
                        else
                           if (dealloc_id != OSRESULT_OK)
                              OsDebugErrorMessage
                                 (L"Cannot de-allocate the ANSI\n"
                                  L"version of the window class ID.");

                        if (result != OSRESULT_OK)
                           if (!DestroyWindow(window))
                              OsDebugErrorMessage
                                 (L"Cannot destroy a window after failure\n"
                                  L"to de-allocate temporary variables.");
                     }
                     else
                     {
                        if (dealloc_title != OSRESULT_OK)
                           OsDebugErrorMessage
                              (L"Cannot de-allocate the ANSI version\n"
                               L"of the window title.");
                        if (dealloc_id != OSRESULT_OK)
                           OsDebugErrorMessage
                              (L"Cannot de-allocate the ANSI version\n"
                               L"of the window class ID.");
                     }
                  }  // De-allocate <titleansi> and <idansi>.
               }
               #endif

               if (result == OSRESULT_OK)
               {
                  RECT  rect;

                  if (GetWindowRect(reinterpret_cast<HWND>(priv_handle),
                                    &rect))
                  {
                     // Update the co-ordinates of the window.  They must
                     // reflect the actual co-ordinates, not the commanded
                     // ones.
                     PositionSPtoAS(rect.left + border.Left(),
                                    rect.top  + border.Top (),
                                    border,
                                    area.left,
                                    area.top);
                     PositionSPtoAS(rect.right  - border.Right (),
                                    rect.bottom - border.Bottom(),
                                    border,
                                    area.right,
                                    area.bottom);

                     // The window has been created!
                     created = OSTRUE;
                     WindowExists(OSTRUE);

                     // Inform the window about the fact that its
                     // co-ordinates were updated (see the code above).
                     {
                        long   time;   // current time
                        float  mx;     // current position of the mouse, x
                        float  my;     // current position of the mouse, y

                        // Obtain the current time.
                        time = static_cast<long>(GetTickCount());

                        // Obtain the current position of the mouse cursor.
                        if (MouseButtons() > 0)
                        {
                           POINT  mouse;   // current position of mouse

                           if (GetCursorPos(&mouse))
                              // Convert the position of the mouse cursor.
                              PositionSPtoWA(static_cast<int>(mouse.x),
                                             static_cast<int>(mouse.y),
                                             border,
                                             area,
                                             mx,
                                             my);
                           else
                              mx = my = 1.0;
                        }
                        else
                           mx = my = 1.0;

                        WindowPosSize(time, mx, my);
                     }
                  }  
                  else
                  {
                     result = OSINTERNAL_WINDOWADMIN;
                     if (!DestroyWindow
                             (reinterpret_cast<HWND>(priv_handle)))
                        OsDebugErrorMessage(L"Cannot destroy newly created "
                                            L"window after\na failure to "
                                            L"obtain its position.");
                  }
               }

               // In case of a problem, administer destruction of the
               // window.
               if (result != OSRESULT_OK)
                  if (admin.Destroy() != OSRESULT_OK)
                     OsDebugErrorMessage(L"Cannot administer destruction"
                                         L"of a window\nafter failure "
                                         L"to create that window.");
            }

            // In case of a problem, de-allocate <title>.
            if (result != OSRESULT_OK)
            {
               osheap<wchar_t>  heap;

               if (heap.Deallocate(title) != OSRESULT_OK)
                  OsDebugErrorMessage(L"Cannot de-allocate the window "
                                      L"title\nafter a failure to create "
                                      L"a window.");
               title = static_cast<wchar_t *>(OSNULL);
            }
         }
      }  // were the co-ordinates converted?
   }  // Actual operations.

   return result;

}  // osbasicwindow::priv_Create


/**************************************************************************
 * Function: osbasicwindow::Write                                         *
 **************************************************************************/

osresult                            // result code
osbasicwindow::Write                // writes text to the window
#ifdef OSRTL_UNICODE
   (wchar_t const *       text,     // I: text to write
    wchar_t const *       wtext,    // I: text to write
    int                   length,   // I: length of the text
    int                   extra,    // I: amount of extra space
    float                 x,        // I: x co-ordinate of the text, WA
    float                 y,        // I: y co-ordinate of the text, WA
    ospainter const &     paint,    // I: window painter
    oswindowfont const &  font)     // I: used font
#else
   (char *                text,     // I: text to write
    wchar_t const *       wtext,    // I: text to write
    int                   length,   // I: length of the text
    int                   extra,    // I: amount of extra space
    float                 x,        // I: x co-ordinate of the text, WA
    float                 y,        // I: y co-ordinate of the text, WA
    ospainter const &     paint,    // I: window painter
    oswindowfont const &  font)     // I: used font
#endif

{  // osbasicwindow::Write

   osresult  result;   // result code
   int       wpx;      // x co-ordinate of the text, WP
   int       wpy;      // y co-ordinate of the text, WP

   result = PositionWAtoWP(x, y, border, area, wpx, wpy);
   if (result == OSRESULT_OK)
   {
      // Add the width of the text to <wpx>.
      {
         float  full;    // width of the text including both margins, FS
         float  dummy;   // (unused widths of the text)

         result = font.TextWidthIdeal(wtext, paint, dummy, dummy, full);
         if (result == OSRESULT_OK)
         {
            int  width;   // width of the text incl. margins, SP
            int  dummy;   // (unused vertical text size)

            result = SizeFStoSP(full, 0.0, width, dummy);
            if (result == OSRESULT_OK)
               wpx += width;
         }
      }

      if (result == OSRESULT_OK)
      {
         int     written;  // #characters already written
         osbool  insert;   // insert additional pixels? (else: drop pixels)
         float   ppcs;     // #pixels to drop/insert per character spacing
         float   pixels;   // #pixels to drop/insert before curr. char group

         insert = OSTRUE;
         ppcs   = 0.0;
         if (length > 1)
         {
            ppcs = static_cast<float>(extra) / static_cast<float>(length-1);
            if (ppcs < 0.0)
            {
               insert = OSFALSE;
               ppcs   = -ppcs;
            }
         }

         text  += length;
         wtext += length;

         written = 0;
         pixels  = 0.0;
         while (written < length  &&  result == OSRESULT_OK)
         {
            int  write;   // #characters to write as part of current group
            int  xpos;    // x co-ordinate of current group, WP

            write = 0;
            do
            {
               ++write;
               ++written;
               pixels += ppcs;
            } while (pixels < 0.5  &&  written < length);

            // Determine the x co-ordinate of the group of <write> chars.
            {
               float  width;   // width of the group of <write> chars, FS

               {
                  float  dummy;   // (no need to know all text widths)

                  result = font.TextWidth(wtext-written, paint,
                                          dummy, dummy, width);
               }

               if (result == OSRESULT_OK)
               {
                  int  hor;   // width of the group of <write> chars, SP

                  {
                     int  dummy;   // (no need to convert vertical size)

                     result = SizeFStoSP(width, 0.0, hor, dummy);
                  }

                  if (result == OSRESULT_OK)
                  {
                     int  pixcorr;   // correction of x co-ordinate, WP

                     if (RoundFloatToIntegral(pixels, &pixcorr))
                     {
                        pixels -= static_cast<float>(pixcorr);

                        if (insert)
                           pixcorr = -pixcorr;
                        wpx += pixcorr;
                        xpos = wpx - hor;
                     }
                     else
                        result = OSINTERNAL_WRITE;
                  }
               }
            }

            if (result == OSRESULT_OK)
               if (!TextOutA(reinterpret_cast<HDC>(paint),
                            xpos, wpy,
                            text-written, write))
                  result = OSINTERNAL_WRITE;
         }
      }
   }

   return result;

}  // osbasicwindow::Write


/**************************************************************************
 * Function: masterwindow::Create                                         *
 **************************************************************************/

osresult                    // result code
masterwindow::Create        // creates a <masterwindow>
(float            left,     // I: x co-ordinate of left   edge, AS
 float            right,    // I: x co-ordinate of right  edge, AS
 float            top,      // I: y co-ordinate of top    edge, AS
 float            bottom,   // I: y co-ordinate of bottom edge, AS
 wchar_t const *  name,     // I: name of the window
 osbool           vis)      // I: must the window be visible?

{  // masterwindow::Create

   osresult  result;   // result code

   // Create the master window.
   result = priv_Create(left, right, top, bottom, name, vis);

   // If successful, define <masterhandle>.
   if (result == OSRESULT_OK)
      masterhandle = reinterpret_cast<HWND>(priv_handle);

   return result;

}  // masterwindow::Create


/**************************************************************************
 * Function: masterwindow::WindowDestruction                              *
 **************************************************************************/

void
masterwindow::WindowDestruction   // the window is being destroyed
(long   ,                         // -: time of window destruction
 float  ,                         // -: x position of the mouse
 float  )                         // -: y position of the mouse

{  // masterwindow::WindowDestruction

   // If the master window is destroyed, life has no meaning anymore.
   PostQuitMessage(0 /*!!!*/);

}  // masterwindow::WindowDestruction


/**************************************************************************
 * Function: masterwindow::Paint                                          *
 **************************************************************************/

osresult                      // result code
masterwindow::Paint           // paints an area of the window
(float              left,     // I: left   edge of area that needs painting
 float              right,    // I: right  edge of area that needs painting
 float              top,      // I: top    edge of area that needs painting
 float              bottom,   // I: bottom edge of area that needs painting
 long               time,     // I: time of window creation
 float              x,        // I: x position of the mouse
 float              y,        // I: y position of the mouse
 ospainter const &  p)        // I: window painter

{  // masterwindow::Paint

   return inherited::Paint(left, right, top, bottom, time, x, y, p);

}  // masterwindow::Paint


/**************************************************************************
 * Function: GenericHandler                                               *
 **************************************************************************/

LRESULT             // result of processing a window message
CALLBACK
GenericHandler      // processes all messages for a window
(HWND    window,    // I: window that receives the message
 UINT    message,   // I: message for the window
 WPARAM  wparam,    // I: parameter to <message> with additional information
 LPARAM  lparam)    // I: parameter to <message> with additional information

{  // GenericHandler

   LRESULT  result;   // result of processing the message

   if (message == WM_NCCREATE)
   {
      CREATESTRUCT *   data;      // data for creating the window
      osbasicwindow *  newborn;   // the newly born window <window>

      data    = reinterpret_cast<CREATESTRUCT *>(lparam);
      newborn = reinterpret_cast<osbasicwindow *>(data->lpCreateParams);

      // Store <newborn> in the window <window>.
      SetWindowLongA(window, GWL_USERDATA, reinterpret_cast<LONG>(newborn));

      // Define the handle of the new window.
      newborn->priv_handle =
                 reinterpret_cast<osbasicwindow::priv_windowhandle>(window);
   }

   // Send the message to its destination.
   {
      osbasicwindow *  destination;   // destination for the message

      // Find the destination for the message.
      destination = reinterpret_cast<osbasicwindow *>
                            (GetWindowLongA(window, GWL_USERDATA));

      // Check if *destination is indeed the destination for the message.
      if (    destination != static_cast<osbasicwindow *>(OSNULL)
          &&  reinterpret_cast<HWND>(destination->priv_handle) == window)
      {
         // *destination is the destination for the message <message>.
         // Pass the message on to the message handler of *destination .

         // Throwing an exception in a call-back function is fatal.
         // Therefore, intercept all such exceptions, even if
         // destination->MessageHandler should not have thrown them.
         try
         {
            result = static_cast<LRESULT>
                        (destination->priv_MessageHandler
                            (static_cast<uint32>(message),
                             static_cast<uint32>(wparam ),
                             static_cast< int32>(lparam )));
         }
         catch (...)
         {
            #ifdef OSRTL_DEBUGGING
               // Generate some alarm sounds.  In this case, displaying an
               // error message is not sufficient, because the very fact
               // that an error message is being displayed may recursively
               // result in more messages being sent to the window <window>
               // and thus in more entries into this piece of code.
               OsSound(OSSOUND_PROBLEM, 5);
            #endif

            // Display the error message, hoping it reaches the user.
            OsDebugErrorMessage(L"An exception was thrown in the message "
                                L"handler\nfor window \"",
                                destination->Name(),
                                L"\".");

            // The normal message handler failed.  Try the default one.
            result = DefWindowProc(window, message, wparam, lparam);
         }
      }
      else
         // The window <window> has not yet been constructed.  Pass the
         // message on to the default message handler.
         result = DefWindowProc(window, message, wparam, lparam);
   }


   return result;

}  // GenericHandler


/**************************************************************************
 * Function: guard_osrtl_window_window::~guard_osrtl_window_window        *
 **************************************************************************/

guard_osrtl_window_window::~guard_osrtl_window_window   // performs exit
()                                                      //    checks on
                                                        //    the module

{  // guard_osrtl_window_window::~guard_osrtl_window_window

   if (windows != 0)
      OsDebugErrorMessage(L"Not all windows have been properly destroyed.");

}  // guard_osrtl_window_window::~guard_osrtl_window_window


/**************************************************************************
 * Function: NamedLeftBorder                                              *
 **************************************************************************/

static
int               // width of the left border of a named window
NamedLeftBorder   // obtains width of left border of a named window
()

{  // NamedLeftBorder

   return GetSystemMetrics(SM_CXFRAME);

}  // NamedLeftBorder


/**************************************************************************
 * Function: NamedRightBorder                                             *
 **************************************************************************/

static
int                // width of the right border of a named window
NamedRightBorder   // obtains width of right border of a named window
()

{  // NamedRightBorder

   return GetSystemMetrics(SM_CXFRAME);

}  // NamedRightBorder


/**************************************************************************
 * Function: NamedTopBorder                                               *
 **************************************************************************/

static
int              // height of the top border of a named window
NamedTopBorder   // obtains height of top border of a named window
()

{  // NamedTopBorder

   return GetSystemMetrics(SM_CYFRAME)
             + GetSystemMetrics(SM_CYCAPTION)
             - GetSystemMetrics(SM_CYBORDER);

}  // NamedTopBorder


/**************************************************************************
 * Function: NamedBottomBorder                                            *
 **************************************************************************/

static
int                 // height of the bottom border of a named window
NamedBottomBorder   // obtains height of bottom border of a named window
()

{  // NamedBottomBorder

   return GetSystemMetrics(SM_CYFRAME);

}  // NamedBottomBorder


/**************************************************************************
 * Function: NamelessLeftBorder                                           *
 **************************************************************************/

static
int                  // width of the left border of a nameless window
NamelessLeftBorder   // obtains width of left border of a nameless window
()

{  // NamelessLeftBorder

   return GetSystemMetrics(SM_CXFRAME);

}  // NamelessLeftBorder


/**************************************************************************
 * Function: NamelessRightBorder                                          *
 **************************************************************************/

static
int                   // width of the right border of a nameless window
NamelessRightBorder   // obtains width of right border of a nameless window
()

{  // NamelessRightBorder

   return GetSystemMetrics(SM_CXFRAME);

}  // NamelessRightBorder


/**************************************************************************
 * Function: NamelessTopBorder                                            *
 **************************************************************************/

static
int                 // height of the top border of a nameless window
NamelessTopBorder   // obtains height of top border of a nameless window
()

{  // NamelessTopBorder

   return GetSystemMetrics(SM_CYFRAME);

}  // NamelessTopBorder


/**************************************************************************
 * Function: NamelessBottomBorder                                         *
 **************************************************************************/

static
int                    // height of the bottom border of a nameless window
NamelessBottomBorder   // obtains height of bottom border of nameless window
()

{  // NamelessBottomBorder

   return GetSystemMetrics(SM_CYFRAME);

}  // NamelessBottomBorder


/**************************************************************************
 * Function: PositionAStoSP                                               *
 **************************************************************************/

static
osresult                           // result code
PositionAStoSP                     // converts co-ordinates from AS to SP
(float                    asx,     // I: x co-ordinate, AS
 float                    asy,     // I: y co-ordinate, AS
 priv_bordersize const &  border,  // I: border sizes
 int &                    spx,     // O: x co-ordinate, SP
 int &                    spy)     // O: y co-ordinate, SP

{  // PositionAStoSP

   osresult  result;   // result code
   int       left;     // width of the left border,      pixels
   int       width;    // available width of the screen, pixels

   result = OSERROR_VALUEOUTOFRANGE;

   left  = border.Left();
   width = ScreenWidthPixels() - left - border.Right();
   if (RoundFloatToIntegral(asx*static_cast<float>(width), &spx))
   {
      spx += left;

      {
         int  top;      // height of the top border,       pixels
         int  height;   // available height of the screen, pixels

         top    = border.Top();
         height = ScreenHeightPixels() - top - border.Bottom();
         if (RoundFloatToIntegral(asy*static_cast<float>(height), &spy))
         {
            spy += top;
            result = OSRESULT_OK;
         }
      }
   }

   return result;

}  // PositionAStoSP


/**************************************************************************
 * Function: PositionSPtoAS                                               *
 **************************************************************************/

static
void
PositionSPtoAS                     // converts co-ordinates from SP to AS
(int                      spx,     // I: x co-ordinate, SP
 int                      spy,     // I: y co-ordinate, SP
 priv_bordersize const &  border,  // I: border sizes
 float &                  asx,     // O: x co-ordinate, AS
 float &                  asy)     // O: y co-ordinate, AS

{  // PositionSPtoAS

   {
      int  left;    // width of the left border,      pixels
      int  width;   // available width of the screen, pixels

      left  = border.Left();
      width = ScreenWidthPixels() - left - border.Right();
      asx   = static_cast<float>(spx - left) / static_cast<float>(width);
   }

   {
      int  top;      // height of the top border,       pixels
      int  height;   // available height of the screen, pixels

      top    = border.Top();
      height = ScreenHeightPixels() - top - border.Bottom();
      asy    = static_cast<float>(spy - top) / static_cast<float>(height);
   }

}  // PositionSPtoAS


/**************************************************************************
 * Function: PositionSPtoWA                                               *
 **************************************************************************/

static
void
PositionSPtoWA                     // converts co-ordinates from SP to WA
(int                      spx,     // I: x co-ordinate, SP
 int                      spy,     // I: y co-ordinate, SP
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  wax,     // O: x co-ordinate, WA
 float &                  way)     // O: y co-ordinate, WA

{  // PositionSPtoWA

   // Convert the x co-ordinate.
   {
      int  screenwidth;   // available screen width, FS
      int  leftborder;    // width of the left border, FS

      leftborder  = border.Left();
      screenwidth = ScreenWidthPixels() - leftborder - border.Right();

      // Beware of the (unlikely) situation that the left border and the
      // right border cover the entire width of the screen.
      if (screenwidth > 0)
      {
         float  windowwidth;   // width of the window area, AS

         windowwidth = area.right - area.left;

         // Beware of a window that has a width of zero.
         if (windowwidth > 1e-9)
            wax =   (    static_cast<float>(spx - leftborder)
                       / static_cast<float>(screenwidth     ) - area.left)
                  / windowwidth;
         else
            wax = 0.0;
      }
      else
         wax = 0.0;
   }

   // Convert the y co-ordinate.
   {
      int  screenheight;   // available screen height, FS
      int  topborder;      // height of the top border, FS

      topborder    = border.Top();
      screenheight = ScreenHeightPixels() - topborder - border.Bottom();

      // Beware of the (unlikely) situation that the top border and the
      // bottom border cover the entire height of the screen.
      if (screenheight > 0)
      {
         float  windowheight;   // height of the window area, AS

         windowheight = area.bottom - area.top;

         // Beware of a window that has a height of zero.
         if (windowheight > 1e-9)
            way =   (    static_cast<float>(spy - topborder)
                       / static_cast<float>(screenheight   ) - area.top)
                  / windowheight;
         else
            way = 0.0;
      }
      else
         way = 0.0;
   }

}  // PositionSPtoWA


/**************************************************************************
 * Function: PositionWAtoWP                                               *
 **************************************************************************/

static
osresult                           // result code
PositionWAtoWP                     // converts co-ordinates from WA to WP
(float                    wax,     // I: x co-ordinate, WA
 float                    way,     // I: y co-ordinate, WA
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 int &                    wpx,     // O: x co-ordinate, WP
 int &                    wpy)     // O: y co-ordinate, WP

{  // PositionWAtoWP

   osresult  result = OSRESULT_OK;   // result code

   {
      float  width;   // available width of the screen, SP

      width = static_cast<float>(ScreenWidthPixels() - border.Left()
                                                     - border.Right());
      if (!RoundFloatToIntegral(wax * (area.right - area.left) * width,
                                &wpx))
         result = OSERROR_VALUEOUTOFRANGE;
   }

   if (result == OSRESULT_OK)
   {
      float  height;   // available height of the screen, SP

      height = static_cast<float>(ScreenHeightPixels() - border.Top()
                                                       - border.Bottom());
      if (!RoundFloatToIntegral(way * (area.bottom - area.top) * height,
                                &wpy))
         result = OSERROR_VALUEOUTOFRANGE;
   }

   return result;

}  // PositionWAtoWP


/**************************************************************************
 * Function: PositionWPtoWA                                               *
 **************************************************************************/

static
void
PositionWPtoWA                     // converts co-ordinates from WP to WA
(int                      wpx,     // I: x co-ordinate, WP
 int                      wpy,     // I: y co-ordinate, WP
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  wax,     // O: x co-ordinate, WA
 float &                  way)     // O: y co-ordinate, WA

{  // PositionWPtoWA

   {
      float  winwidth;   // window width, SP

      winwidth = (area.right - area.left)
                    * static_cast<float>(  ScreenWidthPixels()
                                         - border.Left()
                                         - border.Right());
      if (winwidth < 1e-9)    // beware of zero-size windows
         wax = 0.0;
      else
         wax = static_cast<float>(wpx) / winwidth;
   }

   {
      float  winheight;   // window height, SP

      winheight = (area.bottom - area.top)
                     * static_cast<float>(  ScreenHeightPixels()
                                          - border.Top()
                                          - border.Bottom());
      if (winheight < 1e-9)    // beware of zero-size windows
         way = 0.0;
      else
         way = static_cast<float>(wpy) / winheight;
   }

}  // PositionWPtoWA


/**************************************************************************
 * Function: SizeAStoSP                                                   *
 **************************************************************************/

static
osresult                           // result code
SizeAStoSP                         // converts sizes from AS to SP
(float                    ash,     // I: horizontal size, AS
 float                    asv,     // I: vertical   size, AS
 priv_bordersize const &  border,  // I: border sizes, SP
 int &                    sph,     // O: horizontal size, SP
 int &                    spv)     // O: vertical   size, SP

{  // SizeAStoSP

   osresult  result = OSERROR_VALUEOUTOFRANGE;   // result code

   if (    RoundFloatToIntegral(static_cast<float>(  ScreenWidthPixels()
                                                   - border.Left()
                                                   - border.Right()) * ash,
                                &sph)
       &&  RoundFloatToIntegral(static_cast<float>(  ScreenHeightPixels()
                                                   - border.Top()
                                                   - border.Bottom()) * asv,
                                &spv))
      result = OSRESULT_OK;

   return result;

}  // SizeAStoSP


/**************************************************************************
 * Function: SizeFStoSP                                                   *
 **************************************************************************/

static
osresult       // result code
SizeFStoSP     // converts sizes from FS to SP
(float  fsh,   // I: horizontal size, FS
 float  fsv,   // I: vertical   size, FS
 int &  sph,   // O: horizontal size, SP
 int &  spv)   // O: vertical   size, SP

{  // SizeFStoSP

   osresult  result = OSERROR_VALUEOUTOFRANGE;   // result code

   if (    RoundFloatToIntegral
              (static_cast<float>(ScreenWidthPixels()) * fsh,
               &sph)
       &&  RoundFloatToIntegral
              (static_cast<float>(ScreenHeightPixels()) * fsv,
               &spv))
      result = OSRESULT_OK;

   return result;

}  // SizeFStoSP


/**************************************************************************
 * Function: SizeWAtoFS                                                   *
 **************************************************************************/

static
void
SizeWAtoFS                         // converts sizes from WA to FS
(float                    wah,     // I: horizontal size, WA
 float                    wav,     // I: vertical   size, WA
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  fsh,     // O: horizontal size, FS
 float &                  fsv)     // O: vertical   size, FS

{  // SizeWAtoFS

   // Convert the horizontal size.
   {
      int  sw;   // screen width, SP

      sw  = ScreenWidthPixels();
      fsh = wah * (area.right - area.left)
                * static_cast<float>(sw - border.Left() - border.Right())
                / static_cast<float>(sw                                 );
   }

   // Convert the vertical size.
   {
      int  sh;   // screen height, SP

      sh  = ScreenHeightPixels();
      fsv = wav * (area.bottom - area.top)
                * static_cast<float>(sh - border.Top() - border.Bottom())
                / static_cast<float>(sh                                 );
   }

}  // SizeWAtoFS


/**************************************************************************
 * Function: SizeFStoWA                                                   *
 **************************************************************************/

static
void
SizeFStoWA                         // converts sizes from FS to WA
(float                    fsh,     // I: horizontal size, FS
 float                    fsv,     // I: vertical   size, FS
 priv_bordersize const &  border,  // I: border sizes, SP
 priv_windowarea const &  area,    // I: edges of window area, AS
 float &                  wah,     // O: horizontal size, WA
 float &                  wav)     // O: vertical   size, WA

{  // SizeFStoWA

   // Convert the horizontal size.
   {
      float  windowwidth;   // width of the window area, AS

      windowwidth = area.right - area.left;

      // Beware of a window that has a width of zero.
      if (windowwidth > 1e-9)
      {
         int  screenwidth;   // width of the screen,           SP
         int  availwidth;    // available width of the screen, SP

         screenwidth = ScreenWidthPixels();
         availwidth  = screenwidth - border.Left() - border.Right();

         // Beware of the (unlikely) situation that the left border and the
         // right border cover the entire width of the screen.
         if (availwidth > 0)
            wah = fsh * static_cast<float>(screenwidth)
                      / static_cast<float>(availwidth )
                      / windowwidth;
         else
            wah = 0.0;
      }
      else
         wah = 0.0;
   }

   // Convert the vertical size.
   {
      float  windowheight;   // height of the window area, AS

      windowheight = area.bottom - area.top;

      // Beware of a window that has a height of zero.
      if (windowheight > 1e-9)
      {
         int  screenheight;   // height of the screen,           SP
         int  availheight;    // available height of the screen, SP

         screenheight = ScreenHeightPixels();
         availheight  = screenheight - border.Top() - border.Bottom();

         // Beware of the (unlikely) situation that the top border and the
         // bottom border cover the entire height of the screen.
         if (availheight > 0)
            wav = fsv * static_cast<float>(screenheight)
                      / static_cast<float>(availheight )
                      / windowheight;
         else
            wav = 0.0;
      }
      else
         wav = 0.0;
   }

}  // SizeFStoWA


/**************************************************************************
 * Function: GetMouseData                                                 *
 **************************************************************************/

static
void
GetMouseData                        // obtains data on a mouse event
(float &                  mx,       // O: x co-ordinate of the mouse, WA
 float &                  my,       // O: y co-ordinate of the mouse, WA
 osbool &                 mleft,    // O: left  mouse button is down?
 osbool &                 mright,   // O: right mouse button is down?
 osbool &                 shift1,   // O: first  shift key is down?
 osbool &                 shift2,   // O: second shift key is down?
 osbool &                 shift3,   // O: third  shift key is down?
 uint32                   param1,   // I: parameter to the mouse message
 int32                    param2,   // I: parameter to the mouse message
 priv_bordersize const &  border,   // I: border sizes, SP
 priv_windowarea const &  area)     // I: edges of window area, AS

{  // GetMouseData

   // Obtain the position of the mouse.
   {
      int  wpmx;   // new x position of the mouse, WP
      int  wpmy;   // new y position of the mouse, WP

      wpmx = static_cast<int>(LOWORD(static_cast<DWORD>(param2)));
      wpmy = static_cast<int>(HIWORD(static_cast<DWORD>(param2)));
      PositionWPtoWA(wpmx, wpmy, border, area, mx, my);
   }

   // Obtain the statuses of the left and right mouse buttons.
   {
      mleft  = static_cast<osbool>((param1 & MK_LBUTTON) != 0);
      mright = static_cast<osbool>((param1 & MK_RBUTTON) != 0);
   }

   // Obtain the statuses of the three shift keys.
   {
      shift1 = static_cast<osbool>((param1 & MK_SHIFT  ) != 0);
      shift2 = static_cast<osbool>((param1 & MK_CONTROL) != 0);
      shift3 = static_cast<osbool>((GetKeyState(VK_MENU) & 0x8000) != 0);
   }

}  // GetMouseData


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/window/window/window.cpp .                        *
 **************************************************************************/


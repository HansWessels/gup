
----------------------------------------------------------------------------
---  File:     osrtl/windows/window/window/window.d
---  Contents: Basic window.
---  Module:   <osrtl><window><window>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/window/window/window.d  -  Basic window
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:38  erick
---  First version of the MS-Windows shell (currently only osrtl).
---
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Detailed history
----------------------------------------------------------------------------
Revision  Concise description of the changes
Dates
Author
----------------------------------------------------------------------------
 1.1.000  First issue of this module documentation file.
19970711
19970824
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><window><window>
----------------------------------------------------------------------------

   The module <osrtl><window><window> allows to create, manage, destroy, and
work with windows.  It provides all services that are needed to manage
windows on the screen.  It manages all interactions between the user code
and the operating system, including the messages that the operating system
sends to the window.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><window><window>
----------------------------------------------------------------------------

   All other parts of the program may freely use the services that are
provided by <osrtl><window><window>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><window><window> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <window> is based on an <osobject>.

      Component <osrtl><error>.
         <window> uses result codes (<osresult>) to inform the caller about
         the result of its operations.

      Component <osrtl><hardware>.
         <window> needs to know the hardware characteristics of the screen.

      Component <osrtl><startup>.
         To create windows, <window> must know the program instance.

      Component <osrtl><tasking>.
         <window> must protect its critical code sections from simultaneous
         access by several processes.  To this end, it uses semaphores.

      Component <osrtl><heap>.
         <window> allocates information on the heap.

      Component <osrtl><text>.
         <window> manages strings.

      Component <osrtl><sound>.
         <window> contains a piece of code that has been designed to work
         seamlessly.  However, in case a serious problem that should not
         have occurred does occur in that piece of code (that is, a serious
         internal error occurs in <osrtl><window><window>), the developer
         must be warned (in debugging mode only).  This piece of code can
         not reliably display a warning message on the screen, because such
         a warning message might recursively cause the same piece of code to
         be executed.  Therefore, the developer is warned through a series
         of alarm sounds.

      Component <osrtl><scrnobj>.
         <window> selects the default, arrow-shaped mouse cursor when the
         mouse cursor enters a window.

      Component <osrtl><drawing>.
         <window> uses colors and fonts to draw to the screen.

      Module <osrtl><window><winpaint>.
         <window> uses window painters to draw to the screen.

      Module <osrtl><window><winfont>.
         <window> uses window fonts to write text to the screen.

      Module <osrtl><window><admin>.
         <window> must administer creation and destruction of windows.

      Module <osrtl><window><window>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><window><window>
----------------------------------------------------------------------------

   Before discussing the language elements (types, variables, functions, ...)
of <osrtl><window><window>, the various co-ordinate systems must be
explained.  These co-ordinate systems play a central role in indicating
positions and sizes of objects within a window, or positions and sizes of
windows on the screen.
   The following co-ordinate systems are used to interface to the outside
world, as well as for internal processing inside <window>:

      1.   AS    Available Screen    float    0.0 to 1.0
      2.   FS    Full Screen         float    0.0 to 1.0
      3.   WA    Window Area         float    0.0 to 1.0

In addition, <window> uses the following co-ordinate systems for its
internal processing:

      4.   SP    Screen Pixels       int      >= 0
      5.   WP    Window Pixels       int      >= 0

SP and WP are for internal use only, because they depend on the fact that
the screen consists of pixels.  Strictly speaking, this is not necessarily
the case: it is possible that an output device exists that does not consist
of discrete pixels.  Therefore, pixels are to be regarded as platform-
dependent entities.  Hence, their existence is wrapped invisibly inside
<window>, and <window> only exposes those co-ordinate systems that are both
platform independent and device independent.
   Before explaining the co-ordinate systems in detail, some knowledge is
required about the lay-out of a window.  In general, a window consists of
the so-called window area, surrounded by the window borders.  The window
area is the part of the window that is available to the application.  Here,
all the window's information is displayed.  The window borders are used by
the system to indicate the extent of the window.  They can be used by the
user to move and resize the window.


1.   AS    Available Screen    float    0.0 to 1.0
--------------------------------------------------

   This co-ordinate system covers the part of the screen that is available
to the window area when the borders of the window are within the confines of
the screen.  In other words, given the widths of the left and rights borders
and the heights of the top and bottom borders, the AS system covers the
following area:

      Left:   A distance L to the right of the left edge of the screen,
              where L is equal to the width of the left border of the
              window.
      Right:  A distance R to the left of the right edge of the screen,
              where R is equal to the width of the right border of the
              window.
      Top:    A distance T below the top edge of the screen, where T is
              equal to the height of the top border of the window.
      Bottom: A distance B above the bottom edge of the screen, where B is
              equal to the height of the bottom border of the window.

   The origin of the AS system is in the upper-left corner of this area.
The x axis extends to the right, the y axis extends to the bottom.  The
point x==y==0.0 , the origin of the AS system, is the point that lies a
distance L to the right of the left edge of the screen (L is described
above) and a distance T below the top edge of the screen (T is described
above).  The point x==y==1.0 is the point that lies a distance R to the left
edge of the screen (R is described above) and a distance B above the bottom
edge of the screen (B is described above).  The point x==y==0.0 lies just
inside the available area of the screen (that is, 0.0 is inclusive for both
axes); the point x==y==1.0 lies just outside the available area of the
screen (that is, 1.0 is exclusive for both axes).


2.   FS    Full Screen    float    0.0 to 1.0
---------------------------------------------

   This co-ordinate system covers the entire screen.  The origin of the FS
system is the upper-left corner of the screen.  The x axis extends to the
right, the y axis extends to the bottom.  The point x==y==0.0 , the origin
of the FS system, corresponds to the upper-left corner of the screen; this
point lies just inside the boundaries of the screen, that is, 0.0 is
inclusive for both axes.  The point x==y==1.0 corresponds to the lower-right
corner of the screen; this point lies just outside the boundaries of the
screen, that is, 1.0 is exclusive for both axes.


3.   WA    Window Area    float    0.0 to 1.0
---------------------------------------------

   This co-ordinate system covers the so-called window area, that is, the
part of the window that is available to the application.  The window area
is the part of the window between the window's borders.
   The origin of the WA system is the upper-left corner of the window area.
The x axis extends to the right, the y axis extends to the bottom.  The
point x==y==0.0 , the origin of the WA system, corresponds to the upper-left
corner of the window area.  This point lies just inside the window area: 0.0
is inclusive for both axes.  The point x==y==1.0 corresponds to the lower-
right corner of the window area.  This point lies just outside the window
area; it corresponds to the point where the left edge of the right border
and the top edge of the bottom border meet.  This means that 1.0 is
exclusive for both axes.
   Note that the WA system is a relative rather than an absolute system.
The position of the origin of the WA system depends on the position of the
window on the screen.  If an object has a size of, say, 0.5 in the WA
system, its absolute size (relative to the screen) depends on the size of
the window.


4.   SP    Screen Pixels    int    >= 0
---------------------------------------

   This co-ordinate system covers the entire screen.  The origin of the SP
system is the upper-left corner of the screen.  The x axis extends to the
right, the y axis extends to the bottom.  The point x==y==0 , the origin of
the SP system, corresponds to the upper-left corner of the screen.  This
point lies just inside the boundaries of the screen, that is, 0 is inclusive
for both axes.  The point x==W , y==H , where W is the width of the screen
expressed in pixels and H is the height of the screen expressed in pixels,
corresponds to the lower-right corner of the screen.  This point lies just
outside the boundaries of the screen, that is, W is exclusive for the x axis
and H is exclusive for the y axis.  In other words, x==W-1 , y==H-1 lies
just inside the boundaries of the screen, at the bottom-right edge.


5.   WP    Window Pixels    int    >= 0
---------------------------------------

   This co-ordinate system covers the so-called window area, that is, the
part of the window that is available to the application.  The window area
is the part of the window between the window's borders.
   The origin of the WP system is the upper-left corner of the window area.
The x axis extends to the right, the y axis extends to the bottom.  The
point x==y==0 , the origin of the WP system, corresponds to the upper-left
corner of the window area.  This point lies just inside the boundaries of
the window area, that is, 0 is inclusive for both axes.  The point x==WW ,
y==WH , where WW is the width of the window area expressed in pixels and WH
is the height of the window area expressed in pixels, corresponds to the
lower-right corner of the window area.  This point lies just outside the
boundaries of the window area, that is, WW is exclusive for the x axis and
WH is exclusive for the y axis.  In other words, x==WW-1 , y==WH-1 lies just
inside the boundaries of the window area, at the bottom-right edge, while
x==WW , y==WH corresponds to the point where the left edge of the right
border and the top edge of the bottom border meet.
   WP is expressed in pixels, which are absolute units.  This means that WP
is an absolute co-ordinate system, as opposed to the related WA system which
is relative.  The only aspect of the WP system that is relative is its
origin.  This means that the position of the origin of the WP system depends
on the position of the window on the screen.  If an object has a given size
in the WP system, it has the same size in the SP system.  The consequence of
this fact for the code of <window> is that, when sizes are used, the systems
SP and WP may be exchanged without performing any conversion.


----------------------------------------------------------------------------

struct priv_bordersize   // sizes of the borders of the window
{
   int (*Left  )();      // width  of the left   border of the window, SP
   int (*Right )();      // width  of the right  border of the window, SP
   int (*Top   )();      // height of the top    border of the window, SP
   int (*Bottom)();      // height of the bottom border of the window, SP
};  // priv_bordersize

   This data type is not intended to be used by other modules.  It contains
pointers to functions that compute the widths and heights of the window's
borders.  These function pointers are used internally by <window>.

----------------------------------------------------------------------------

struct priv_windowarea   // co-ordinates of the edges of the window area, FS
{
   float  left;          // x co-ordinate of the left   edge, inclusive
   float  right;         // x co-ordinate of the right  edge, exclusive
   float  top;           // y co-ordinate of the top    edge, inclusive
   float  bottom;        // y co-ordinate of the bottom edge, exclusive
};  // priv_windowarea

   This data type is not intended to be used by other modules.  It contains
the co-ordinates of the window area.  It is maintained by <window>.

----------------------------------------------------------------------------

class osbasicwindow:   // basic window
   public osobject     //    generic object
{
   //...
};  // osbasicwindow

   This class implements a basic window.  It supplies all the actions and
all information that are necessary to manage a window, including the inter-
face to the operating system.
   Refer to the descriptions of the components of <osbasicwindow> for an
explanation of the functionality of this class.

----------------------------------------------------------------------------

typedef
void *                              // handle of a window
osbasicwindow::priv_windowhandle;

   An osbasicwindow::priv_windowhandle really is a <HWND>.  <HWND>, however,
is platform-specific, so it must not proliferate to other units.  Therefore,
osbasicwindow::priv_windowhandle must not be used in header files.  To allow
<window> to specify a type for a window handle, a <typedef> to an innocent
type is necessary.

----------------------------------------------------------------------------

osbasicwindow::osbasicwindow   // initialises an <osbasicwindow>
();

   Initialises an <osbasicwindow>, bringing it into a well-defined state.
In debugging mode, the name of the <osbasicwindow> is set to
"<osbasicwindow>" to facilitate debugging.

----------------------------------------------------------------------------

virtual
osbasicwindow::~osbasicwindow   // closes down an <osbasicwindow>
();

   Closes down an <osbasicwindow>.  In debugging mode, this destructor warns
the developer if the <osbasicwindow> was not properly destroyed.

----------------------------------------------------------------------------

virtual
osresult                    // result code
osbasicwindow::Create       // creates an <osbasicwindow>
(float            left,     // I: x co-ordinate of left   edge, AS
 float            right,    // I: x co-ordinate of right  edge, AS
 float            top,      // I: y co-ordinate of top    edge, AS
 float            bottom,   // I: y co-ordinate of bottom edge, AS
 wchar_t const *  name,     // I: name of the window
 osbool           vis);     // I: must the window be visible?

   Creates a new <osbasicwindow>.  The four co-ordinates <left>, <right>,
<top> and <bottom> are the co-ordinates of the window area.  If <name>
specifies a name (that is, if it is not
static_cast<wchar_t const *>(OSNULL) ), the window will have a large top
border that contains the name <name>.  If <name> does not specify a name
(that is, if it is static_cast<wchar_t const *>(OSNULL) ), the window will
have a small top border that is so small that it cannot display a name.
If name==L"" (the empty string), the window will have a large top border
that may be used later to display a name, but the top border will initially
not display a name.
   The implementation of <Create> checks if a master window already exists.
If not, it first creates an invisible master window that serves as the
master of all other windows.  To the operating system, all other windows
are children of the master window.
   If <Create> is used on a window that already exists, an error is
returned.

----------------------------------------------------------------------------

virtual
osresult                     // result code
osbasicwindow::Destroy       // destroys an <osbasicwindow>
(osbool  forced = OSFALSE);  // I: force destruction?

   Destroys an <osbasicwindow>.
   <forced> indicates if the window is destroyed even if it indicates that
is should not be destroyed.  Normally, if forced==OSFALSE , <Destroy> uses
<CanBeClosed> to ask the window if it is OK to destroy that window; the
window is destroyed only if <CanBeClosed> allows this.  If, however,
forced==OSTRUE , <CanBeClosed> is not asked for consent, and the window is
destroyed unconditionally.
   In both cases, <Destroy> calls <WindowDestruction> to inform the window
that it is about to be destroyed.
   It is allowed to use <Destroy> on a window that has already been des-
troyed.  The reason for allowing to "destroy" a non-existing window is that
some other process that is running on the system may have destroyed the
window without the program being aware of this fact.

----------------------------------------------------------------------------

osresult                         // result code
osbasicwindow::CreateWindowFont  // creates a window font
(wchar_t const *    name,        // I: name of the font
 float              vsize,       // I: vertical size, WA
 float              hsize,       // I: horizontal size; 1.0==normal
 ospainter const &  painter,     // I: window painter
 oswindowfont &     font)        // O: window font
const;

   Creates a window font.  The window font may be used by the text-handling
functions of <osbasicwindow>.
   <CreateWindowFont> is the <osbasicwindow>-specific form of
oswindowfont::Create .  Refer to oswindowfont::Create for more information
on the characteristics of <CreateWindowFont> ( oswindowfont::Create is in
the module <osrtl><window><winfont>).
   Use <DestroyWindowFont> to destroy the window font.

----------------------------------------------------------------------------

inline
osresult                           // result code
osbasicwindow::DestroyWindowFont   // destroys a window font
(oswindowfont &  font)             // I: window font
const;

   Destroys a window font that was created by <CreateWindowFont>.
   <DestroyWindowFont> is the <osbasicwindow>-specific form of
oswindowfont::Destroy .  Refer to oswindowfont::Destroy for more information
on the characteristics of <DestroyWindowFont> ( oswindowfont::Destroy is in
the module <osrtl><window><winfont>).

----------------------------------------------------------------------------

float                          // height of the font, WA
osbasicwindow::FontHeight      // obtains the height of a font
(oswindowfont const &  font)   // I: font
const;

   Returns the height of the <oswindowfont>.
   <FontHeight> is the <osbasicwindow>-specific form of
oswindowfont::Height .  Refer to oswindowfont::Height of more information on
the characteristics of <FontHeight> ( oswindowfont::Height is in the module
<osrtl><window><winfont>).

----------------------------------------------------------------------------

osresult                         // result code
osbasicwindow::TextWidth         // obtains the widths of a text string
(oswindowfont const &  font,     // I: font
 ospainter const &     painter,  // I: window painter
 wchar_t const *       text,     // I: text
 float &               left,     // O: width of left margin, WA
 float &               middle,   // O: width of text without margins, WA
 float &               full)     // O: width of text plus margins, WA
const;

   Obtains the three significant widths of a text string.  These three
widths are described under oswindowfont::TextWidth , which is in the module
<osrtl><window><winfont>.  Note that these widths are the widths that the
string should ideally have if it were written to the screen in such a way
that rounding effects do not occur; refer to oswindowfont::TextWidthIdeal
for details (note that osbasicwindow::Write corrects for the rounding
effects, so <TextWidth> returns the widths of the text string as it will
appear when written using <Write>).
   <TextWidth> is the <osbasicwindow>-specific form of
oswindowfont::TextWidthIdeal .  Refer to oswindowfont::TextWidthIdeal for
more information on the characteristics of <TextWidth>
( oswindowfont::TextWidthIdeal is in the module <osrtl><window><winfont>).

----------------------------------------------------------------------------

inline
float                   // width of the window, AS
osbasicwindow::Width    // obtains the current width of the window
()
const;

inline
float                   // height of the window, AS
osbasicwindow::Height   // obtains the current height of the window
()
const;

   These two functions return the current width and height, respectively, of
the window.  Initially, the width and height are (almost) identical to the
commanded width and height.  Later, the width and height of the window may
change, either as commanded by the program, or because the user changed the
dimensions of the window.

----------------------------------------------------------------------------

inline
wchar_t const *       // the name of the window
osbasicwindow::Name   // obtains the name of the window
()
const;

   Obtains the current name of the window.  Initially, this is the name that
was passed to <Create>.
   If the name that was passed to <Create> was
static_cast<wchar_t const *>(OSNULL) , the window does not have a name and
will never have a name.

----------------------------------------------------------------------------

inline
osbool                   // is the window visible?
osbasicwindow::Visible   // checks if the window is currently visible
()
const;

   Checks if the window is currently visible.  Note that both the program
itself and other programs running on the same system may change the window
from visible to invisible and vice versa.

----------------------------------------------------------------------------

int32                                // result code
osbasicwindow::priv_MessageHandler   // handles all messages Windows sends
(uint32  message,                    // I: message code
 uint32  param1,                     // I: first parameter to the message
 int32   param2);                    // I: second parameter to the message

   Handles all messages that Windows sends to the window.  This function is
the functional entry point of message processing.  It is called by the
actual entry point of message processing, <GenericHandler>.  It is the first
member function of <osbasicwindow> to receive control after the OS sends a
message to the window.
   <priv_MessageHandler> must be <public> to allow <GenericHandler> to call
it.
   The task of <priv_MessageHandler> is to pre-process the messages, to
dispatch them to the appropriate signal functions, inquiry functions, or
command functions, and possibly to post-process the results of those
functions.  The functionality of <priv_MessageHandler> is transparent to
the rest of the program.

----------------------------------------------------------------------------

osbasicwindow::priv_windowhandle   // handle of the window
osbasicwindow::priv_handle;

   The handle of the window.  Refer to the description of
osbasicwindow::priv_windowhandle for the reason to use this specific type
for the window handle.
   <priv_handle> must be <public> to allow <GenericHandler> to access it.

----------------------------------------------------------------------------

virtual
osresult                       // result code
osbasicwindow::Write           // writes text
(float                 x,      // I: x co-ordinate of the text, WA
 float                 y,      // I: y co-ordinate of the text, WA
 wchar_t const *       text,   // I: text to write
 oswindowfont const &  font,   // I: font to use
 oscolor const &       color,  // I: color of the text
 ospainter const &     p);     // I: window painter

   Writes the text <text> to the window, using the font <font> and the color
<color>.  The text will be written at position (x,y) .  This position is the
left of the left margin of the text, on the baseline of the text.
   <Write> is implemented such that it corrects for the rounding effects
that would occur if the text were simply output to the screen.  To achieve
this, it uses the second version of <Write>, which is described below.
   <Write> handles single lines only.  If <text> contains newline characters
or other non-printable characters, the result of <Write> is not defined.  If
<text> is so long that it does not fit in the window, or if the co-ordinates
are such that part of the text (or the entire text) is outside the window
area, the text will be clipped at the edges of the window area.
   <Write> must be used only if <Paint> indicates that (part of) the window
needs repainting.

----------------------------------------------------------------------------

virtual
osresult                     // result code
osbasicwindow::Line          // draws a straight line
(float              x0,      // I: x co-ord of start point (incl.), WA
 float              y0,      // I: y co-ord of start point (incl.), WA
 float              x1,      // I: x co-ord of end   point (excl.), WA
 float              y1,      // I: y co-ord of end   point (excl.), WA
 oscolor const &    color,   // I: color of the line
 ospainter const &  p);      // I: window painter

   Draws a straight line from (x0,y0) (inclusive) to (x1,y1) (exclusive).
The line will have the color <color>.  It will be as thin as the display
allows without becoming invisibly thin.
   If (part of) the line is outside the window area, the line will be
clipped at the edges of the window area.
   <Line> must be used only if <Paint> indicates that (part of) the window
needs repainting.

----------------------------------------------------------------------------

virtual
osresult                          // result code
osbasicwindow::OutlineRectangle   // draws the outline of a rectangle
(float              left,         // I: left   edge of the rectangle, WA
 float              right,        // I: right  edge of the rectangle, WA
 float              top,          // I: top    edge of the rectangle, WA
 float              bottom,       // I: bottom edge of the rectangle, WA
 oscolor const &    color,        // I: color of the rectangle
 ospainter const &  p);           // I: window painter

   Draws the outline of a rectangle, that is, draws only the bounding edge
of a rectangle without touching the rectangle's interior.
   If (part of) the rectangle is outside the window area, the rectangle will
be clipped at the edges of the window area.
   <OutlineRectangle> must be used only if <Paint> indicates that (part of)
the window needs repainting.
   The implementation of <OutlineRectangle> might simply call <Line> four
times, once for each of the edges of the rectangle.  That would, however,
not be a fast way to draw the rectangle.  To speed up drawing,
<OutlineRectangle> duplicates the basic operations of <Line>.

----------------------------------------------------------------------------

virtual
osresult                         // result code
osbasicwindow::FilledRectangle   // draws a filled rectangle
(float              left,        // I: left   edge of the rectangle, WA
 float              right,       // I: right  edge of the rectangle, WA
 float              top,         // I: rop    edge of the rectangle, WA
 float              bottom,      // I: bottom edge of the rectangle, WA
 oscolor const &    color,       // I: color of the rectangle
 ospainter const &  p);          // I: window painter

   Draws a filled rectangle, that is, draws both the bounding edge of a
rectangle and the rectangle's interior.  The bounding edge and the interior
will have the same color.
   If (part of) the rectangle is outside the window area, the rectangle will
be clipped at the edges of the window area.
   <FilledRectangle> must be used only if <Paint> indicates that (part of)
the window needs repainting.

----------------------------------------------------------------------------

virtual
osresult                        // result code
osbasicwindow::WindowCreation   // the window is being created
(long   ,                       // -: time of window creation
 float  ,                       // -: x position of the mouse, WA
 float  );                      // -: y position of the mouse, WA

   Informs the window that it is about to be created.  This function is
intended to be overridden by derived classes.  The window will actually be
created only if <WindowCreation> returns <OSRESULT_OK>.
   osbasicwindow::WindowCreation returns <OSRESULT_OK>.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::WindowDestruction   // the window is being destroyed
(long   ,                          // -: time of window destruction
 float  ,                          // -: x position of the mouse, WA
 float  );                         // -: y position of the mouse, WA

   Informs the window that it is about to be destroyed.  This function is
intended to be overridden by derived classes.  If this function is called,
the window will be destroyed unconditionally, even if it does not want to
be destroyed.  Refer to the description of <Destroy> for an explanation of
the destruction process.
   osbasicwindow::WindowDestruction does nothing.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::WindowPosSize   // position and size of window have changed
(long   ,                      // -: time of change in position and size
 float  ,                      // -: x position of the mouse, WA
 float  );                     // -: y position of the mouse, WA

   Informs the window that either its position or its size has changed, or
that both its position and its size have changed.  This function is intended
to be overridden by derived classes.
   <WindowPosSize> may also be called if neither the position nor the size
of the window have changed.
   osbasicwindow::WindowPosSize does nothing.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::WindowActivation   // the window is being activated
(priv_windowhandle  ,             // -: window that is being de-activated
 long               ,             // -: time of activation
 float              ,             // -: x position of the mouse, WA
 float              );            // -: y position of the mouse, WA

   Informs the window that it is being activated.  If the window that is
being de-activated is a window of this program, the first parameter
identifies the window that is being de-activated.  If the window that is
being de-activated belongs to a different application, the first parameter
does not identify any window.
   This function is intended to be overridden by derived classes.
   osbasicwindow::WindowActivation does nothing.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::WindowDeactivation   // the window is being de-activated
(priv_windowhandle  ,               // -: window that is being activated
 long               ,               // -: time of de-activation
 float              ,               // -: x position of the mouse, WA
 float              );              // -: y position of the mouse, WA

   Informs the window that it is being de-activated.  If the window that is
being activated is a window of this program, the first parameter identifies
the window that is being activated.  If the window that is being
de-activated belongs to a different application, the first parameter does
not identify any window.
   This function is intended to be overridden by derived classes.
   osbasicwindow::WindowDeactivation does nothing.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::MousePosition  // the position of the mouse has changed
(long    ,                    // -: time of the change of the mouse position
 float   x,                   // I: new x position of the mouse, WA
 float   y,                   // I: new y position of the mouse, WA
 osbool  ,                    // -: left  mouse button is down?
 osbool  ,                    // -: right mouse button is down?
 osbool  ,                    // -: shift key 1 is down?
 osbool  ,                    // -: shift key 2 is down?
 osbool  );                   // -: shift key 3 is down?

   Informs the window that the position of the mouse has changed.  This
function is intended to be overridden by derived classes.
   osbasicwindow::MousePosition checks if the new position of the mouse is
inside the window area.  If it is, the mouse cursor is set to the standard
arrow.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::MouseLeftSelect   // the left mouse button was used to select
(long    ,                       // -: time of selection with left button
 float   ,                       // -: new x position of the mouse, WA
 float   ,                       // -: new y position of the mouse, WA
 osbool  ,                       // -: left  mouse button is down?
 osbool  ,                       // -: right mouse button is down?
 osbool  ,                       // -: shift key 1 is down?
 osbool  ,                       // -: shift key 2 is down?
 osbool  );                      // -: shift key 3 is down?

   Informs the window that the left mouse button was used to select a
position in the window.  With Windows, this means that the left mouse button
was single-clicked.  This function is intended to be overridden by derived
classes.
   Before <MouseLeftSelect> is called, <osbasicwindow> calls
<MousePosition>.
   osbasicwindow::MouseLeftSelect activates the window.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::MouseLeftActivate // left mouse button was used to activate
(long    ,                       // -: time of activation with left button
 float   ,                       // -: new x position of the mouse, WA
 float   ,                       // -: new y position of the mouse, WA
 osbool  ,                       // -: left  mouse button is down?
 osbool  ,                       // -: right mouse button is down?
 osbool  ,                       // -: shift key 1 is down?
 osbool  ,                       // -: shift key 2 is down?
 osbool  );                      // -: shift key 3 is down?

   Informs the window that the left mouse button was used to activate a
position in the window.  With Windows, this means that the left mouse button
was double-clicked.  This function is intended to be overridden by derived
classes.
   Before <MouseLeftActivate> is called, <osbasicwindow> calls
<MousePosition>.
   osbasicwindow::MouseLeftActivate toggles the window between its normal
size and a full-screen size where the borders are just outside the window,
such that the window area covers the entire screen.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::MouseLeftRelease  // the left mouse button was released
(long    ,                       // -: time of release of left mouse button
 float   ,                       // -: new x position of the mouse, WA
 float   ,                       // -: new y position of the mouse, WA
 osbool  ,                       // -: left  mouse button is down?
 osbool  ,                       // -: right mouse button is down?
 osbool  ,                       // -: shift key 1 is down?
 osbool  ,                       // -: shift key 2 is down?
 osbool  );                      // -: shift key 3 is down?

   Informs the window that the left mouse button was released.  This
function is intended to be overridden by derived classes.
   Before <MouseLeftRelease> is called, <osbasicwindow> calls
<MousePosition>.
   osbasicwindow::MouseLeftRelease does nothing.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::MouseRightSelect  // right mouse button was used to select
(long    ,                       // -: time of selection with right button
 float   ,                       // -: new x position of the mouse, WA
 float   ,                       // -: new y position of the mouse, WA
 osbool  ,                       // -: left  mouse button is down?
 osbool  ,                       // -: right mouse button is down?
 osbool  ,                       // -: shift key 1 is down?
 osbool  ,                       // -: shift key 2 is down?
 osbool  );                      // -: shift key 3 is down?

   Informs the window that the right mouse button was used to select a
position in the window.  With Windows, this means that the right mouse
button was single-clicked.  This function is intended to be overridden by
derived classes.
   Before <MouseRightSelect> is called, <osbasicwindow> calls
<MousePosition>.
   osbasicwindow::MouseRightSelect does nothing.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::MouseRightActivate // right mouse button was used to activate
(long    ,                        // -: time of activation with right button
 float   ,                        // -: new x position of the mouse, WA
 float   ,                        // -: new y position of the mouse, WA
 osbool  ,                        // -: left  mouse button is down?
 osbool  ,                        // -: right mouse button is down?
 osbool  ,                        // -: shift key 1 is down?
 osbool  ,                        // -: shift key 2 is down?
 osbool  );                       // -: shift key 3 is down?

   Informs the window that the right mouse button was used to activate a
position in the window.  With Windows, this means that the right mouse
button was double-clicked.  This function is intended to be overridden by
derived classes.
   Before <MouseRightActivate> is called, <osbasicwindow> calls
<MousePosition>.
   osbasicwindow::MouseRightActivate does nothing.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::MouseRightRelease // the right mouse button was released
(long    ,                       // -: time of release of right mouse button
 float   ,                       // -: new x position of the mouse, WA
 float   ,                       // -: new y position of the mouse, WA
 osbool  ,                       // -: left  mouse button is down?
 osbool  ,                       // -: right mouse button is down?
 osbool  ,                       // -: shift key 1 is down?
 osbool  ,                       // -: shift key 2 is down?
 osbool  );                      // -: shift key 3 is down?

   Informs the window that the right mouse button was released.  This
function is intended to be overridden by derived classes.
   Before <MouseRightRelease> is called, <osbasicwindow> calls
<MousePosition>.
   osbasicwindow::MouseRightRelease does nothing.

----------------------------------------------------------------------------

virtual
osbool                       // window may be closed?
osbasicwindow::CanBeClosed   // checks if it is OK to close the window
(long   ,                    // -: time of the closure request
 float  ,                    // -: x position of the mouse, WA
 float  );                   // -: y position of the mouse, WA

   Asks the window if it is OK to close the window.  This function is called
if <Destroy> is called with its parameter <forced> set to <OSFALSE>.  The
window must indicate if it can safely be destroyed.  It may ask the user for
permission to close the window.  Refer to the description of <Destroy> for a
detailed explanation of the destruction process.
   This function is intended to be overridden by derived classes.
   osbasicwindow::CanBeClosed returns <OSTRUE>.

----------------------------------------------------------------------------

virtual
void
osbasicwindow::SizeLimits   // ask for size limits for the window
(float &  minwidth,         // O: minimum width  of the window, AS
 float &  maxwidth,         // O: maximum width  of the window, AS
 float &  minheight,        // O: minimum height of the window, AS
 float &  maxheight);       // O: maximum height of the window, AS

   Asks the window to supply limits to its size.  This function is intended
to be overridden by derived classes.
   osbasicwindow::SizeLimits sets the minimum width and height to 0.0 , and
the maximum width and height to 1.0 .

----------------------------------------------------------------------------

virtual
osresult                     // result code
osbasicwindow::Paint         // paints an area of the window
(float              left,    // I: L edge of area that needs painting, WA
 float              right,   // I: R edge of area that needs painting, WA
 float              top,     // I: T edge of area that needs painting, WA
 float              bottom,  // I: B edge of area that needs painting, WA
 long               ,        // -: time of the paint request
 float              ,        // -: x position of the mouse, WA
 float              ,        // -: y position of the mouse, WA
 ospainter const &  p)       // I: window painter
= 0;                         // derived class _must_ provide this function

   Informs the window that (part of) the window area needs repainting.
Derived classes _must_ override this function in order to be useful.  If
<Paint> is called, the indicated area _must_ be repainted for the window to
look normal.  To paint, only the paint functions may be used.  When calling
a paint function, <Paint> _must_ pass <p> to the paint function.
   osbasicwindow::Paint makes the entire rectangle black.  It does not take
the function result of <Paint> into account.

----------------------------------------------------------------------------

osresult                     // result code
osbasicwindow::priv_Create   // creates an <osbasicwindow>
(float            left,      // I: x co-ordinate of left   edge, AS
 float            right,     // I: x co-ordinate of right  edge, AS
 float            top,       // I: y co-ordinate of top    edge, AS
 float            bottom,    // I: y co-ordinate of bottom edge, AS
 wchar_t const *  name,      // I: name of the window
 osbool           vis);      // I: must the window be visible?

   Implements the functionality of <Create>.  Refer to the description of
<Create> for an explanation of the creation process.

----------------------------------------------------------------------------

osbasicwindow::osbasicwindow   // dummy private copy constructor
(osbasicwindow const &)        //    prevents copying
{}

osbasicwindow const &          // dummy private assignment operator
osbasicwindow::operator =      //    prevents assignment
(osbasicwindow const &)
const
{ return *this; }

   These two functions make sure that it is not possible to copy one
<osbasicwindow> to another <osbasicwindow>.  Making such a copy would
endanger the integrity of <osrtl><window><window>.

----------------------------------------------------------------------------

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

   This second version of <Write> is used by the first version of <Write> to
actually write the text <text> to the screen.  The first version of <Write>
has calculated the number of pixels that must be inserted between the char-
acters of <text> in order to make <text> as wide as it would ideally be
(this is the value of <extra>, which may be positive, zero, or negative).
This second version of <Write> distributes the <extra> pixels over the
string to give it its "ideal" width (the width it would have if writing to
the screen would not have introduced rounding effects).

----------------------------------------------------------------------------

priv_bordersize  border;   // sizes of the borders of the window, SP

   The functions that return the widths and heights of the window's borders.
Refer to the description of <priv_bordersize>, above.  This variable is used
internally by <osbasicwindow>.

----------------------------------------------------------------------------

priv_windowarea  area;   // co-ordinates of edges of window area, AS

   The actual co-ordinates (not the commanded co-ordinates) of the edges of
the window area.  Refer to the description of <priv_windowarea>, above.
This variable is used internally by <osbasicwindow>.  The width of the
window area, which follows from <area>, can be obtained through <Width>.
The height of the window area, which follows from <area>, can be obtained
through <Height>.

----------------------------------------------------------------------------

osbool  created;   // creation of the window has completed?

   Whether the window exists.  This variable is used internally by
<osbasicwindow>.

----------------------------------------------------------------------------

wchar_t *  title;   // window title

   The name, or title, of the window.  Originally, this is the name of the
window that was passed to <Create>.  This variable can be obtained through
<Name>.

----------------------------------------------------------------------------

osbool  visible;   // whether the window is actually visible

   Whether the window is currently visible.  This variable can be obtained
through <Visible>.

----------------------------------------------------------------------------

float  lastmx;   // last known x co-ordinate of the mouse cursor, WA
float  lastmy;   // last known y co-ordinate of the mouse cursor, WA

   The last known co-ordinate of the mouse cursor.  These variables are used
internally by <osbasicwindow> to avoid sending unnecessary <MousePosition>
messages.

----------------------------------------------------------------------------

osbool  forcedestruction;   // force destruction of the window?

   Used in the destruction process.  This variable is set to the value of
<forced> in <Destroy>.  Refer to the description of <Destroy> for details on
the destruction process.
   This variable is used internally by <osbasicwindow>.

----------------------------------------------------------------------------

osresult  result_destroy;   // result of attempt to destroy window

   The result of the attempt to destroy the window.  Used internally by
<osbasicwindow>.

----------------------------------------------------------------------------

class masterwindow:            // master window
   private osbasicwindow       //    basic window
{
   //...
};  // masterwindow

   The master window that serves as the parent of all other windows that are
created by osbasicwindow::Create .  osbasicwindow::Create creates a single
master window as soon as the first normal window is cretaed;
osbasicwindow::Destroy destroys the master window when the last normal
window is destroyed.  At all times, at most one master window exists.
   Refer to the descriptions of the components of <masterwindow> for more
information on the master window.

----------------------------------------------------------------------------

virtual
osresult                    // result code
masterwindow::Create        // creates a <masterwindow>
(float            left,     // I: x co-ordinate of left   edge (AS)
 float            right,    // I: x co-ordinate of right  edge (AS)
 float            top,      // I: y co-ordinate of top    edge (AS)
 float            bottom,   // I: y co-ordinate of bottom edge (AS)
 wchar_t const *  name,     // I: name of the window
 osbool           vis);     // I: must the window be visible?

   Creates the master window by calling osbasicwindow::priv_Create .  If the
master window was created, defines <masterhandle>.  <masterhandle> is used
by the functions of <osbasicwindow>.

----------------------------------------------------------------------------

virtual
void
masterwindow::WindowDestruction   // the window is being destroyed
(long   ,                         // -: time of window destruction
 float  ,                         // -: x position of the mouse
 float  );                        // -: y position of the mouse

   The master window is destroyed by osbasicwindow::Destroy after the last
normal window of the program has been destroyed.  No window existing
anymore, there is no way for the user to interact with the program.  That
means that there is nothing more to do for the program.  Therefore,
masterwindow::WindowDestruction terminates the program.

----------------------------------------------------------------------------

virtual
osresult                      // result code
masterwindow::Paint           // paints an area of the window
(float              left,     // I: L edge of area that needs painting
 float              right,    // I: R edge of area that needs painting
 float              top,      // I: T edge of area that needs painting
 float              bottom,   // I: B edge of area that needs painting
 long               time,     // I: time of window creation
 float              x,        // I: x position of the mouse
 float              y,        // I: y position of the mouse
 ospainter const &  p);       // I: window painter

   This function has no purpose, if only because the master window is
invisible.  However, osbasicwindow::Paint is a pure virtual function,
so <masterwindow> is required to define a <Paint> member function.
   masterwindow::Paint simply calls osbasicwindow::Paint .

----------------------------------------------------------------------------

typedef  osbasicwindow  inherited;   // base class

   Allows easy identification of the base class of <masterwindow>.

----------------------------------------------------------------------------

class guard_osrtl_window_window   // guards this module
{
public:
   //...
};  // guard_osrtl_window_window

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

virtual
guard_osrtl_window_window::~guard_osrtl_window_window   // performs exit
();                                                     //    checks on
                                                        //    the module

   This destructor checks the value of the global variable <windows> to see
if all windows have been properly destroyed.  In debugging mode, it warns
the developer if not.

----------------------------------------------------------------------------

static
int                 // width of the left border of a named window
NamedLeftBorder     // obtains width of left border of a named window
();

static
int                 // width of the right border of a named window
NamedRightBorder    // obtains width of right border of a named window
();

static
int                 // height of the top border of a named window
NamedTopBorder      // obtains height of top border of a named window
();

static
int                 // height of the bottom border of a named window
NamedBottomBorder   // obtains height of bottom border of a named window
();

   Compute the widths or heights of the borders of a window that has a name.
For a named window, the fields of the data member osbasicwindow::border
point to these four functions.
   Refer to the description of <priv_bordersize> for an explanation of the
use of functions rather than variables that are computed only once.

----------------------------------------------------------------------------

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

   Compute the widths or heights of the borders of a window that has no
name.  For a nameless window, the fields of the data member
osbasicwindow::border point to these four functions.
   Refer to the description of <priv_bordersize> for an explanation of the
use of functions rather than variables that are computed only once.

----------------------------------------------------------------------------

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

   These functions perform transformations between the various co-ordinate
systems.  Only those transformations that are actually needed by <window>
have been implemented.
   These functions transform positions, not sizes.

----------------------------------------------------------------------------

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

   These functions perform transformations between the various co-ordinate
systems.  Only those transformations that are actually needed by <window>
have been implemented.
   These functions transform sizes, not positions.

----------------------------------------------------------------------------

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

   This is a helper function that obtains data on a mouse event that was
just received by the event-handling mechanism.

----------------------------------------------------------------------------

inline
int          // signed integer value
WordToInt    // converts 16-bit unsigned word to 32-bit signed int
(WORD  w);   // I: word to convert

   This is a helper function that transforms a 16-bit unsigned value to a
32-bit signed value.

----------------------------------------------------------------------------

extern "C"
LRESULT             // result of processing a window message
CALLBACK
GenericHandler      // processes all messages for a window
(HWND    window,    // I: window that receives the message
 UINT    message,   // I: message for the window
 WPARAM  wparam,    // I: parameter to <message>, additional information
 LPARAM  lparam);   // I: parameter to <message>, additional information

   This function is called by the event-handling mechanism of Windows when
an event is sent to a window.  <GenericHandler> determines to which window
object the message must be sent.  It then calls <priv_MessageHandler> of
that particular window.

----------------------------------------------------------------------------

static
HWND                           // handle of the master window
masterhandle
= static_cast<HWND>(OSNULL);

   The handle of the master window, or static_cast<HWND>(OSNULL) if the
master window does not exist.  This value is used by the functions of
<osbasicwindow>.

----------------------------------------------------------------------------

static
int       // number of existing <osbasicwindow>s,
windows   //    excluding the master window
= 0;

   The total number of existing <osbasicwindow>s.  This value is maintained
by <window>.  It is used by the destructor of <guard> to check if all
windows have been properly destroyed before the program terminates.

----------------------------------------------------------------------------

static
masterwindow   // master window
master;

   The master window.  osbasicwindow::Create creates the master window as
soon as the first <osbasicwindow> is created.  osbasicwindow::Destroy
destroys the master window after the last <osbasicwindow> has been
destroyed.
   The master window serves as the parent of all <osbasicwindow>s.

----------------------------------------------------------------------------

static
guard_osrtl_window_window   // guards this module
guard;

   Instantiates a global <guard_osrtl_window_window>.  This ensures that
the destructor of <guard_osrtl_window_window> is activated when the module
<window> is closed down.  Refer to the description of that destructor for
details on the checks that are performed when <window> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   Functions are used to determine the width and height of the screen, the
widths and heights of borders, because this allows the system to react to
changes to these values without having to take specific measures.  The other
possible approach consists of determining these values only once, at program
initialisation.  This has the advantage of being faster.  However, the speed
improvement is negligible since creating windows is a very slow process any-
way.  Therefore, the advantage of being able to immediately take changes in
system settings into account is more important than a small increase in
execution speed.


----------------------------------------------------------------------------
---  End of file osrtl/windows/window/window/window.d .
----------------------------------------------------------------------------


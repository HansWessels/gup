
----------------------------------------------------------------------------
---  File:     osrtl/windows/window/winfont/winfont.d
---  Contents: Window font.
---  Module:   <osrtl><window><winfont>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/window/winfont/winfont.d  -  Window font
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:42  erick
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
19970801
19970818
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><window><winfont>
----------------------------------------------------------------------------

   The module <osrtl><window><winfont> supplies a font type that may be used
by windows to create fonts, to use them for writing text to a window, and to
destroy them.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><window><winfont>
----------------------------------------------------------------------------

   The module <osrtl><window><winfont> may be freely used by any other
part of the program.  The only restrictions are that a window font must
be created before it can be used, and that it must be destroyed after it
has been used for the last time.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><window><winfont> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <winfont> uses basic entities such as <osobject>.

      Component <osrtl><error>.
         <winfont> uses result codes (<osresult>) to indicate the results of
         its operations.

      Component <osrtl><hardware>.
         <winfont> needs to know the hardware characteristics of the screen.

      Component <osrtl><tasking>.
         <winfont> must protect certain critical code sections from
         simultaneous access by several processes.

      Component <osrtl><heap>.
         <winfont> allocates information on the heap.

      Component <osrtl><text>.
         <winfont> duplicates text strings.

      Component <osrtl><drawing>.
         <winfont> is based on an <osfont>.

      Module <osrtl><window><winpaint>.
         <winfont> needs access to window painters.

      Module <osrtl><window><winfont>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><window><winfont>
----------------------------------------------------------------------------

class oswindowfont:   // window font
   public osfont      //    generic font
{
   //...
};  oswindowfont

   A window font.  Refer to the descriptions of the components of this class
for an explanation of the functionality of an <oswindowfont>.

----------------------------------------------------------------------------

oswindowfont::oswindowfont   // initialises an <oswindowfont>
();

   Initialises an <oswindowfont>, bringing it to a well-defined state.

----------------------------------------------------------------------------

osresult                       // result code
oswindowfont::Create           // creates a window font
(wchar_t const *    name,      // I: name of the font
 float              vsize,     // I: vertical size of font, screen rel.
 float              hsize,     // I: horizontal size of font; 1.0==normal
 ospainter const &  painter);  // I: window painter

   Creates an <oswindowfont>.  The looks of the font are primarily
determined by the font name <name>.
If name==static_cast<wchar_t const *>(OSNULL) , a default font is selected.
   The font has a height of zero if vsize==0.0 ; the font is as large as
the height of the screen if vsize==1.0 .
   The width-to-height ratio of the font is normal if hsize==1.0 ; the font
is condensed (narrow and tall) if 0.0<hsize<1.0 ; the font is expanded (wide
and low) if hsize>1.0 .
   The caller must supply a window painter for <Create> to perform its
tasks.
   It is allowed to create a font more than once without destroying it.
This does not consume system resources other than processing time.

----------------------------------------------------------------------------

inline
osresult                // result code
oswindowfont::Destroy   // destroys a window font
();

   Destroys a font that was created by <Create>.
   It is allowed to destroy a font several times without re-creating it.
<Destroy> will not do anything when the font has already been destroyed;
it will just return <OSRESULT_OK>.

----------------------------------------------------------------------------

inline
float                  // height of the font, screen relative
oswindowfont::Height   // obtains the actual height of the font
()
const;

   Obtains the actual height (not the commanded height) of the font.  The
font height is defined as the height of the top of the font above the base-
line of the font.  The height is usually positive, which means that the top
of the highest character of the font is above the baseline.
   <Height> returns 0.0 if the top of the highest character of the font is
on the baseline; 1.0 if the top of the highest character of the font is at
the top of the screen if the baseline is at the bottom of the screen; and
-1.0 if the top of the highest character of the font is at the bottom of the
screen if the baseline is at the top of the screen.

----------------------------------------------------------------------------

inline
float                          // horizontal size of font; 1.0==normal
oswindowfont::HorizontalSize   // obtains the horizontal size of the font
()
const;

   Obtains the actual horizontal size (not the commanded horizontal size) of
the font.  The horizontal size is defined as the width-to-height ratio of
the font.  The width-to-height ratio of the font is normal if hsize==1.0 ;
the font is condensed (narrow and tall) if 0.0<hsize<1.0 ; the font is
expanded (wide and low) if hsize>1.0 .

----------------------------------------------------------------------------

osresult                       // result code
oswindowfont::TextWidth        // obtains widths of text string, scrn rel
(wchar_t const *    text,      // I: text
 ospainter const &  painter,   // I: window painter
 float &            left,      // O: width of left margin
 float &            middle,    // O: width of text without margins
 float &            full)      // O: width of text including both margins
const;

   Obtains the widths that a text string would have if it were written to
the screen using the given font and window painter.  These widths are the
actual widths the string would have if no special measures were taken (note
<that winfont> does not take any special measures to adjust the widths of
the image of the string on the screen).  The actual widths of the string may
deviate from the ideal widths of the string because of rounding effects that
occur while writing the string.
   A text string consists of three parts, each of which has its own width:

      1. The left marging (<left>).
         Before drawing the first character of the string, the "pen"
         advances a certain distance relative to its current position.
         This distance is the width of the left margin.  The width of the
         left margin may be positive, zero, or negative.
      2. The width of the text string itself, not including margins
         (<middle>).
         This is the actual width of the text string itself, not taking any
         margin into account.  It is the distance between the leftmost point
         the "pen" will reach while writing the string, and the rightmost
         point the "pen" will reach.
         The width of the text string is zero if the string is empty, or
         positive if the string is not empty.
      3. The right margin.
         After drawing the last character of the string, the "pen" advances
         a certain distance to the right of the rightmost part of the image
         of the string.  This distance is the width of the right margin.
         The width of the right margin may be positive, zero, or negative.k

<full> is identical to <left> plus <middle> plus the width of the right
margin.
   Each of the three text widths is 0.0 if it is zero, and 1.0 if it is as
wide as the entire screen.
   Use <TextWidthIdeal> to obtain the ideal widths of a text string, without
taking rounding effects into account.

----------------------------------------------------------------------------

osresult                       // result code
oswindowfont::TextWidthIdeal   // obtains widths of text string, scrn rel
(wchar_t const *    text,      // I: text
 ospainter const &  painter,   // I: window painter
 float &            left,      // O: width of left margin
 float &            middle,    // O: width of text without margins
 float &            full)      // O: width of text including both margins
const;

   Obtains the widths that a text string would have if it were written to
the screen using the given font and window painter.  These widths are the
ideal widths the string would have if the process of writing the string to
the screen would not suffer from rounding effects.
   Refer to <TextWidth> for a description of the parameters of this
function.
   Use <TextWidth> to obtain the actual widths of a text string, taking
rounding effects into account.

----------------------------------------------------------------------------

osresult                       // result code
oswindowfont::TextWidth        // obtains widths of text string, scrn rel
(wchar_t const *    text,      // I: text
 ospainter const &  painter,   // I: window painter
 osfont const &     font,      // I: font to use for writing the string
 float &            left,      // O: width of left margin
 float &            middle,    // O: width of text without margins
 float &            full)      // O: width of text including both margins
const;

   This <private> function is used by the first version of <TextWidth> and
by <TextWidthIdeal> to obtain their results.  Refer to the description of
the first version of <TextWidth> for an explanation of the parameters of
this function.  The only additional parameter of this version of
<TextWidth>, <font>, is an explicit indication of the font that should be
used to compute the text widths.

----------------------------------------------------------------------------

typedef  osfont  inherited;    // base class

   This type allows easy access to the base class of <oswindowfont>.

----------------------------------------------------------------------------

float  horsize;    // commanded horizontal size of the font; 1.0==normal
float  vertsize;   // commanded vertical size of the font, screen rel.

   These data members hold the commanded (not necessarily the actual)
sizes of the font.  Refer to the description of <Create>, above, for an
explanation of the values of these data members.
   <horsize> and <vertsize> are used internally by <oswindowfont>.

----------------------------------------------------------------------------

float  above;   // actual height of the font, screen relative

   This data member holds the height of the font above the baseline.  Refer
to the description of <Height>, above, for the definition of the height of
the font above the baseline.
   The value of this data member is obtained through <Height>.

----------------------------------------------------------------------------

osbool  exists;    // whether the <oswindowfont> has been created

   This data member is used by the <oswindowfont> to remember if it has been
created.  It is used internally by <oswindowfont>.

----------------------------------------------------------------------------

class fontadmin   // font administrator
{
public:
   //...
};  // fontadmin

   The font administrator.  For each font name that is created, this class
maintains an administration font that is used to obtain precise information
about all fonts with that name.  Using a font administrator saves time,
memory, and system resources while dealing with fonts.
   Refer to the components of <fontadmin> for an explanation of the
functionality of this class.

----------------------------------------------------------------------------

virtual
fontadmin::~fontadmin   // closes down a <fontadmin>
();

   Closes down a <fontadmin>.
   This is a dummy destructor.  It serves no purpose.  It is defined only
because the Coding Standard requires that every base class have a virtual
destructor.

----------------------------------------------------------------------------

static
osresult                      // result code
fontadmin::Metrics            // determines important font metrics
(wchar_t const *    name,     // I: name of the font
 ospainter const &  p,        // I: painter for the font
 float &            hv,       // O: H/V size ratio, screen sizes, of the
                              //    font when the H size matches the V size
 float &            height,   // O: actual height of the font as a fraction
                              //    of the commanded vertical font size;
                              //    >0 if characters extend above baseline
 float &            depth);   // O: actual depth of the font as a fraction
                              //    of the commanded vertical font size;
                              //    >0 if characters extend below baseline

   Obtains metrics about the indicated font.  The comments after each of the
output parameters indicate the meanings of the metrics.
   <Metrics> checks the list <metricdata> for data on the font <name>.  If
such data is already present in the list, <Metrics> uses those data to
define <hv>, <height>, and <depth>.  If such data are not yet present,
<Metrics> creates a font with name <name> that is as high as the design size
of that font; this allows <Metrics> to obtain actual, non-rounded metrics
about the font.  After creating this font, it stores that font and its most
important metrics in a new element of the list <metricdata>.  It then
defines <hv>, <height>, and <depth>.

----------------------------------------------------------------------------

static
osresult                   // result code
fontadmin::ActualSize      // determines the actual size of a font
(osfont const &     f,     // I: font to measure
 ospainter const &  p,     // I: painter for the font
 float &            h,     // O: height above baseline, screen relative
 float &            th,    // O: total height, screen relative
 float &            tw);   // O: total width, screen relative

   Obtains the actual sizes of a font.  The actual sizes may not be equal to
the expected (commanded) sizes because of rounding effects that occurred
when the system created the font.

----------------------------------------------------------------------------

static
osresult                   // result code
fontadmin::GetMetricFont   // obtains font used to obtain font metrics
(wchar_t const *   name,   // I: font name
 osfont const * &  font);  // O: font used to obtain font metrics

   Returns the metric font that <fontadmin> uses to obtain font metrics.
The metric font is the font that is as high as that font's design size;
refer to fontadmin::Metrics for more information.
   <GetMetricFont> assumes that the metric font is already present in the
list <metricdata>.  It returns an error if this is not the case.

----------------------------------------------------------------------------

static
void
fontadmin::Destroy    // destroys all administration data
();

   Destroys all data in the list <metricdata>.  This function is used by the
destructor of <guard_osrtl_window_winfont>.  Refer to the description of
that destructor for more information.

----------------------------------------------------------------------------

struct fontmetrics
{
   //...
};  // fontmetrics

   Holds information about a metric font.  A metric font is a font with the
same name as a font that was created by <Create>, but that is as high as the
design size of that font.  Refer to fontadmin::Metrics for more information.

----------------------------------------------------------------------------

static
fontadmin::fontmetrics *                           // data on the metrics
fontadmin::metricdata                              //    of all fonts
= static_cast<fontadmin::fontmetrics *>(OSNULL);   //    that were ever used

   This data member holds the start of a linked list that holds data on the
metrics of all fonts that were ever used.

----------------------------------------------------------------------------

class guard_osrtl_window_winfont:   // guards this module
   private fontadmin                //    font administrator
{
   //...
};  // guard_osrtl_window_winfont

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

guard_osrtl_window_winfont::~guard_osrtl_window_winfont   // performs exit
();                                                       //    checks on
                                                          //    the module

   This destructor calls fontadmin::Destroy to destroy all data that any
<fontadmin> allocated.  Refer to the description of fontadmin::Destroy for
more information.

----------------------------------------------------------------------------

static
guard_osrtl_window_winfont    // guards this module
guard;

   Instantiates a global <guard_osrtl_window_winfont>.  This ensures that
the destructor of <guard_osrtl_window_winfont> is activated when the module
<winfont> is closed down.  Refer to the description of that destructor for
details on the checks that are performed when <winfont> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.

----------------------------------------------------------------------------

static
osresult                // result code
GetCharMetrics          // obtains metrics about a character
#ifdef OSRTL_UNICODE
   (HDC      dc,        // I: device context
    wchar_t  ch,        // I: subject character
    int &    a,         // O: A width of the character
    int &    b,         // O: B width of the character
    int &    c)         // O: C width of the character
#else
   (HDC      dc,        // I: device context
    char     ch,        // I: subject character
    int &    a,         // O: A width of the character
    int &    b,         // O: B width of the character
    int &    c)         // O: C width of the character
#endif

   This function obtains the A, B and C widths of a character.  The font
that should be used to write the character must have been selected into the
device context <dc> before this function can be used.
   The A width of a character is similar to the left margin <left> of a text
string as described under the first version of oswindowfont::TextWidth .
The B width of a character is similar to the width <middle> of the text
string itself, not including any margins, as described under
oswindowfont::TextWidth .  The C width of a character is similar to the
width of the right margin of a text string as described under
oswindowfont::TextWidth .

----------------------------------------------------------------------------

static
osresult                           // result code
GetFontMetrics                     // obtains metrics about a font
(osfont const &         font,      // I: subject font
 ospainter const &      painter,   // I: window painter
 OUTLINETEXTMETRIC * &  metric)    // O: metrics about the font

   This function is used by fontadmin::Metrics to obtain metrics about a
font.  The metrics are returned in an OS-defined structure that is allocated
on the heap.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   <fontadmin> is not strictly necessary.  However, to obtain accurate data
about a font without suffering from the rounding effects that normally occur
when a font is created that is smaller than that font's design size, a font
must be created that is as high as the font's design size.  <oswindowfont>
might do this each time it needs accurate data.
   However, creating a font is a time-consuming process.  Therefore, it is
better to create a metric font (a font that is as high as that font's design
size) only once, when a font with a given name is first created.
<fontadmin> handles this task.  Thus, by using a <fontadmin>, <oswindowfont>
can quickly obtain accurate data about its font.
   Once a font with a given name is created by <oswindowfont>, <fontadmin>
maintains a metric font for that font during the rest of the lifetime of the
program.  This uses system resources.  This effect is considered acceptable,
because any font that is used once by the program is expected to be used
another time.  The only exception occurs when the user changes the font that
must be used to display certain types of information; in that case, the
original font for those types of information may never be used again.
This situation, however, is not nominal: the user is not expected to change
the display fonts every time he uses the program.

   The "depth" of a font below the baseline is computed as tm->tmDescent + 1
instead of tm->tmDescent (<tm> is a variable of type <TEXTMETRIC *>).  This
is done because it more closely matches the actual depth of the font on the
screen.

----------------------------------------------------------------------------
---  End of file osrtl/windows/window/winfont/winfont.d .
----------------------------------------------------------------------------


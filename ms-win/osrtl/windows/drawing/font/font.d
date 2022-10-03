
----------------------------------------------------------------------------
---  File:     osrtl/windows/drawing/font/font.d
---  Contents: Fonts.
---  Module:   <osrtl><drawing><font>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/drawing/font/font.d  -  Fonts
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 85 $
---  $Date: 1997-12-30 14:49:19 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:49:19  erick
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
19970729
19970818
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><drawing><font>
----------------------------------------------------------------------------

   The module <osrtl><drawing><font> provides fonts.  These fonts can be
created and destroyed.  The fonts are intended to be used by some other
module that wishes to use them to write text.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><drawing><font>
----------------------------------------------------------------------------

   All parts of the program can directly use the class <osfont> that is
provided by the module <osrtl><drawing><color>.  The only limitation to
<osfont> is that an <osfont> must be created before it is used for the
first time, and that it must be destroyed after it has been used for the
last time.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><drawing><font> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <font> uses basic entities such as <osobject>.

      Component <osrtl><error>.
         <font> uses <osresult>s to communicate the results of operations.

      Component <osrtl><hardware>.
         <font> must know the dimensions of the screen.

      Component <osrtl><tasking>.
         <font> uses semaphores to restrict access to critical sections of
         code.

      Component <osrtl><heap>.
         <font> stores information on the heap.

      Component <osrtl><text>.
         <font> compares strings, converts them, and duplicates them.

      Module <osrtl><drawing><font>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><drawing><font>
----------------------------------------------------------------------------

class osfont:        // font
   public osobject   //    generic object
{
   //...
};  // osfont

   A font.  Refer to the descriptions of the components of this class to
understand the functionality of <osfont>.

----------------------------------------------------------------------------

typedef  void *  osfont::osfonthandle;   // handle of the font

   <osfont> needs to communicate the handle of its "physical" font to the
outside world.  This handle is of an OS-defined type.  To make <osrtl> a
true NIL, none of the parts of <osrtl> must make the use of such OS-defined
types visible to the outside world.  Therefore, a type is defined that is
used to secretly hold OS-specific information.

----------------------------------------------------------------------------

osfont::osfont   // initialises an <osfont>
();

   Initialises an <osfont>: brings the <osfont> into a well-defined state.
Sets the name of the <osfont> to "<osfont>" to facilitate debugging.

----------------------------------------------------------------------------

osfont::~osfont   // closes down an <osfont>
();

   Closes down an <osfont>.  Destroys the <osfont> if the user code hasn't
done so.  In debugging mode, displays a warning that tells the developer
that an <osfont> is being closed without properly having been destroyed.

----------------------------------------------------------------------------

osresult                      // result code
osfont::Create                // creates a font
(wchar_t const *  name,       // I: name of the font
 float            cheight);   // I: character height, screen relative

osresult                      // result code
osfont::Create                // creates a font
(wchar_t const *  name,       // I: name of the font
 float            cheight,    // I: character height,   screen relative
 float            width);     // I: average char width, screen relative

   Creates an <osfont>.  The looks of the font are primarily determined by
the font name <name>.  If name==static_cast<wchar_t const *>(OSNULL) , a
default font is selected.
   The font has a height of zero if cheight==0.0 ; the font is as large as
the height of the screen if cheight==1.0 .
   The first version of <Create> creates a font with a default character
width; the absolute width of the characters will be chosen such that the
width of the characters matches their height.
   The second version of <Create> allows to specify the average character
width.  If width==0.0 , the characters will have a width of zero; if
width==1.0 , the average character width will be as large as the entire
screen.  Using the second version of <Create> allows to create condensed
or expanded fonts.
   It is allowed to create a font more than once without destroying it.
This does not consume system resources other than processing time.

----------------------------------------------------------------------------

osresult          // result code
osfont::Destroy   // destroys a font
();

   Destroys a font that was created by <Create>.
   It is allowed to destroy a font several times without re-creating it.
<Destroy> will not do anything when the font has already been destroyed;
it will just return <OSRESULT_OK>.

----------------------------------------------------------------------------

osresult              // result code
osfont::Transfer      // transfers font information
(osfont &  target);   // O: target for the transfer

   Transfers a font to the indicated target.  Transferring a font can be
seen as a shortcut for destroying both the current <osfont> and <target>,
followed by creating <target> with the same parameters that were used when
the current <osfont> was created.

   Points of attention:
      - If <target> existed before <Transfer> was called, the original font
        <target> will be destroyed by <Transfer>.
      - After use, <target> must be destroyed by <Destroy>.
      - The current <osfont> is destroyed by <Transfer>.  This means that it
        is not necessary to use <Destroy> to destroy the current <osfont>.
        Destroying the current <osfont> after using <Transfer> is, however,
        not forbidden - it will come down to a non-operation.

----------------------------------------------------------------------------

inline
float            // commanded height of the font, screen relative
osfont::Height   // obtains the commanded height of the font
()
const;

   Returns the height that was commanded when the font was created.  This is
the value of <cheight> of both versions of <Create>.

----------------------------------------------------------------------------

inline
float           // commanded width of the font, screen relative
osfont::Width   // obtains the commanded width of the font
()
const;

   Returns the width that was commanded when the font was created.  This is
the value of <width> of the second version of <Create>.  If the font was
created by the first version of <Create>, <Width> returns 0.0 .

----------------------------------------------------------------------------

inline
osbool         // whether one of the sizes of the font is zero
osfont::Zero   // checks if one of the sizes of the font is zero
()
const;

   Checks if either the height of the font or the average character width of
the font is zero, or if both of them are zero.  The height or the width of
the font may be zero even if a non-zero height and width were commanded.
This is caused by the internal processing of <osfont>.

----------------------------------------------------------------------------

inline
wchar_t const *    // name of the font
osfont::FontName   // obtains the name of the font
()
const;

   Returns the name of the font as commanded to <Create> when the font was
created.  If <Create> was passed a font name of
static_cast<wchar_t const *>(OSNULL) , <FontName> returns the actual name of
the font that <font> selected.

----------------------------------------------------------------------------

inline
osfonthandle     // handle of the <osfont>
osfont::Handle   // obtains the handle of the <osfont>
()
const;

   Returns the handle of the <osfont>.  The handle is returned in a
platform-independent format - refer to the description of <osfonthandle>
for an explanation.
   Modules that use <Handle> must either know what an <osfonthandle> really
is (which means that they must be part of <osrtl>), or they must restrict
themselves to storing the handle or passing it to other functions.

----------------------------------------------------------------------------

osfont::osfont       // dummy private copy constructor
(osfont const &)     //    prevents copying
{};

osfont const &       // dummy private assignment operator
osfont::operator =   //    prevents assignment
(osfont const &)
const
{ return *this; }

   These functions are in the <private> section of <osfont>.  They prevent
inadvertent copying of <osfont>s because such copying endangers the
integrity of the font system.

----------------------------------------------------------------------------

osresult                    // result code
osfont::Create              // creates a font
(wchar_t const *  name,     // I: name of the font
 int              vsize,    // I: character height,        pixels
 int              hsize);   // I: average character width, pixels

   This, <private>, version of <Create> actually creates the font.  It is
used by the two <public> versions of <Create>.  Refer to the two <public>
versions of <Create> for an explanation of the functionality of <Create>.

----------------------------------------------------------------------------

osfont::osfonthandle  osfont::handle;   // handle of the font

   The handle of the font.  This handle is used internally by <osfont>.  It
can be obtained through <Handle>.

----------------------------------------------------------------------------

wchar_t const *  osfont::fname;   // name of the font

   The name of the font.  This name is used internally by <osfont>.  It can
be obtained through <FontName>.

----------------------------------------------------------------------------

osbool  osfont::zero;   // font size is zero?

   Indicates whether either the horizontal or the vertical size of the font,
or both of them, are zero.  This variable can be obtained through <Zero>.

----------------------------------------------------------------------------

int  osfont::fcheight;   // character height
int  osfont::fwidth;     // average character width

   These variables hold the resulting height and width, respectively, of the
font.  They are used internally by <osfont>.

----------------------------------------------------------------------------

float  osfont::cmdheight;   // commanded character height, scrn rel.
float  osfont::cmdwidth;    // commanded character width,  scrn rel.

   These variables hold the commanded height and width, respectively, of
the font, with the exception that osfont::cmdwidth is zero if the font was
created by the first version of <Create>.  The variables can be obtained
through <Height> and <Width>, respectively.

----------------------------------------------------------------------------

class guard_osrtl_drawing_font   // guards this module
{
public:
   //...
};  // guard_osrtl_drawing_font

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

virtual
guard_osrtl_drawing_font::~guard_osrtl_drawing_font  // performs exit checks
();                                                  //    on the module

   This destructor checks the number of fonts that have been created but not
destroyed.  In debugging mode, it warns the developer if this number is not
zero.

----------------------------------------------------------------------------

static
int      // total number of existing fonts
fonts
= 0;

   This variable contains the total number of existing fonts.  Each time a
font is successfully created, this variable is incremented; each time a font
is successfully destroyed, the variable is decremented.
   <fonts> is used by the destructor of <guard_osrtl_drawing_font> to check
if all fonts have been properly destroyed.

----------------------------------------------------------------------------

static
guard_osrtl_drawing_font   // guards this module
guard;

   Instantiates a global <guard_osrtl_drawing_font>.  This ensures that the
destructor of <guard_osrtl_drawing_font> is activated when the module <font>
is closed down.  Refer to the description of that destructor for details on
the checks that are performed when <font> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.

----------------------------------------------------------------------------

static
wchar_t const *   // name of the default font
DefaultFontName   // determines the name of the default font
();

   This function is used by the third version of osfont::Create when a font
is created with a font name of static_cast<wchar_t const *>(OSNULL) .  It
tries to find a font that can be created on the system on which the program
is running.  If it finds such a font, it returns the name of that font.  If
it does not find any usable font, it returns
static_cast<wchar_t const *>(OSNULL) .

----------------------------------------------------------------------------

static
osresult    // result code
CountFont   // counts creation of a font
(int  n);   // I: number of fonts that were created

   This function updates the global static variable <fonts>.  It is called
by the third version of osfont::Create and by osfont::Destroy .  The third
version of osfont::Create sets <n> to +1 , osfont::Destroy sets <n> to -1 .
<CountFont> basically adds <n> to <fonts>.
   The value of <fonts> is used by the destructor of
<guard_osrtl_drawing_font> to check if all fonts that were created have been
properly destroyed.  Refer to the description of the destructor of
<guard_osrtl_drawing_font> for details.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/drawing/font/font.d .
----------------------------------------------------------------------------


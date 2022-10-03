
----------------------------------------------------------------------------
---  File:      osrtl/windows/drawing/drawing.d
---  Contents:  Attributes for drawing.
---  Component: <osrtl><drawing>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/drawing/drawing.d  -  Attributes for drawing
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:13  erick
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
 1.1.000  First issue of this component documentation file.
19970722
19970818
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the component <osrtl><drawing>
----------------------------------------------------------------------------

   The component <osrtl><drawing> supplies colors and fonts that are needed
to draw to the screen.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><drawing>
----------------------------------------------------------------------------

   The component <osrtl><drawing> consists of the modules <color> and
<font>.  It only joins these modules.  Therefore, refer to the document-
ation files for these modules for a description of how to use the component
<osrtl><drawing>.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the component <osrtl><drawing>
----------------------------------------------------------------------------

   Because <osrtl><drawing> is nothing but an aggregate of the modules
<color> and <font>, refer to the documentation files for these modules
for a detailed explanation of the component <osrtl><drawing>.
   To give an overview of the component <osrtl><drawing>, here follows a
list of all items provided by the modules that make up <osrtl><drawing>.

Class:    oscolor
Use:      A color.

Function: oscolor::oscolor
          (float  red,
           float  green,
           float  blue);
Use:      Initialises an <oscolor>, using the indicated saturations (from
          0.0 (inclusive) to 1.0 (inclusive) each) for the components of the
          color.

Function: oscolor::oscolor
          (oscolor const &  src);
Use:      Initialises an <oscolor> to the same color as the given <oscolor>.

Function: oscolor const &
          oscolor::operator =
          (oscolor const &  src);
Use:      Initialises an <oscolor> to the same color as the given <oscolor>.

Function: oscolor::Define
          (float  red,
           float  green,
           float  blue);
Use:      Re-defines an <oscolor>, using the indicated saturations (from
          0.0 (inclusive) to 1.0 (inclusive) each) for the components of the
          color.

Function: oscolor::R();
          oscolor::G();
          oscolor::B();
Use:      Obtain the respective components of the color.

Class:    osfont
Use:      A font.  An <osfont> supplies no functionality by itself; it is
          intended to be used by other modules.

Type:     osfont::osfonthandle
Use:      The handle of a font, expressed as a platform-independent type.

Function: osfont::osfont
Use:      Initialises an <osfont>.

Function: osfont::~osfont
Use:      Closes down an <osfont>.

Function: osfont::Create
Use:      Creates an <osfont>, destroying a previous font if the <osfont>
          had already been created.

Function: osfont::Destroy
Use:      Destroys an <osfont> if it has been created.

Function: osfont::Transfer
Use:      Transfers a (physical) font from the current <osfont> to an
          indicated <osfont>, effectively destroying the current <osfont>
          and creating the indicated <osfont> such that it has the same
          properties as the current <osfont> before the current <osfont>
          was implicitly destroyed.

Function: osfont::Height
Use:      Obtains the commanded height of the <osfont>.

Function: osfont::Width
Use:      Obtains the commanded average character with of the <osfont>.
          Returns 0.0 if the <osfont> was created with a default width.

Function: osfont::Zero
Use:      Checks if the internal processing of the <osfont> resulted in a
          font that has a vertical or a horizontal size (or both) of zero.

Function: osfont::FontName
Use:      Obtains the commanded name of the font, or the actual (default)
          name of the font if the font was created without specifying a
          name.

Function: osfont::Handle
Use:      Obtains the handle of the font.


----------------------------------------------------------------------------
---  End of file osrtl/windows/drawing/drawing.d .
----------------------------------------------------------------------------


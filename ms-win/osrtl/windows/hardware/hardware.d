
----------------------------------------------------------------------------
---  File:      osrtl/windows/hardware/hardware.d
---  Contents:  Hardware characteristics.
---  Component: <osrtl><hardware>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/hardware/hardware.d  -  Hardware characteristics
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:22  erick
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
19970725
19970801
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the component <osrtl><hardware>
----------------------------------------------------------------------------

   The component <osrtl><hardware> provides information about the character-
istics of the hardware.  It can be used to check if certain hardware is
present, and, if the hardware is present, to obtain the characteristics
of that hardware.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><hardware>
----------------------------------------------------------------------------

   All functions that are provided by the component <osrtl><hardware> may be
used directly by all other parts of the program.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the component <osrtl><hardware>
----------------------------------------------------------------------------

   The component <osrtl><hardware> consists of two modules, <screen> and
<mouse>.  Refer to the documentation of those modules for details on the
functionality that <osrtl><hardware> provides.

   To give an easily accessible overview, here follows a list of all items
that are exported by the component <osrtl><hardware>.

Function: ScreenWidthPixels
Use:      Determines the width of the screen, in pixels.

Function: ScreenHeightPixels
Use:      Determines the height of the screen, in pixels.

Function: MouseButtons
Use:      Returns the number of buttons on the active mouse device if a
          mouse is present.  Returns 0 if no mouse is present.


----------------------------------------------------------------------------
---  End of file osrtl/windows/hardware/hardware.d .
----------------------------------------------------------------------------


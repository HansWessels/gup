
----------------------------------------------------------------------------
---  File:     osrtl/windows/hardware/mouse/mouse.d
---  Contents: Mouse hardware.
---  Module:   <osrtl><hardware><mouse>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/hardware/mouse/mouse.d  -  Mouse hardware
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:25  erick
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
19970725
19970725
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><hardware><mouse>
----------------------------------------------------------------------------

   The module <osrtl><hardware><mouse> provides information about the hard-
ware characteristics of the mouse device.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><hardware><mouse>
----------------------------------------------------------------------------

   The module <osrtl><hardware><mouse> provides but a single function,
<MouseButtons>.  This function may be used directly by any other part of
the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><hardware><mouse> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Module <osrtl><hardware><mouse>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><hardware><mouse>
----------------------------------------------------------------------------

int            // number of mouse buttons, or 0 if no mouse present
MouseButtons   // determines the number of mouse buttons
();

   Determines if a mouse device is present.  If it is, returns the number
of buttons on the active mouse device.  If no mouse device is present,
returns 0.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/hardware/mouse/mouse.d .
----------------------------------------------------------------------------


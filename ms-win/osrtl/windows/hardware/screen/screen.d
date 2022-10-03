
----------------------------------------------------------------------------
---  File:     osrtl/windows/window/screen/screen.d
---  Contents: Screen characteristics.
---  Module:   <osrtl><window><screen>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/window/screen/screen.d  -  Screen characteristics
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:28  erick
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
19970714
19970725
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><window><screen>
----------------------------------------------------------------------------

   The module <osrtl><window><screen> provides information about the hard-
ware of the active screen.  It is intended to be used solely by the
component <osrtl><window>; other parts of the system are not supposed to
know about the existence of pixels.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><window><screen>
----------------------------------------------------------------------------

   All functions of the module <osrtl><window><screen> can be used directly
by all other parts of the program.  Refer, however, to chapter 1, which
states that this module is not intended to be used by modules that are not
part of the component <osrtl><window>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><window><screen> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Module <osrtl><window><screen>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><window><screen>
----------------------------------------------------------------------------

int                 // screen width, in pixels
ScreenWidthPixels   // obtains the width of the screen, in pixels
();

   Obtains the current width of the screen, measured in pixels.

----------------------------------------------------------------------------

int                  // screen height, in pixels
ScreenHeightPixels   // obtains the height of the screen, in pixels
();

   Obtains the current height of the screen, measured in pixels.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   The functions <ScreenWidthPixels> and <ScreenHeightPixels> might have
been implemented as functions, either inline or normal, that returned the
value of a variable that would have been set during initialisation of the
module <osrtl><window><screen>.  That would have made these functions much
faster.  The advantage of the current approach of making true system calls,
however, is that the system is robust against changes in the resolution of
the screen while the program is running.


----------------------------------------------------------------------------
---  End of file osrtl/windows/window/screen/screen.d .
----------------------------------------------------------------------------


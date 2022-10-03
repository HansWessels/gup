
----------------------------------------------------------------------------
---  File:     osrtl/windows/window/winpaint/winpaint.d
---  Contents: Window painter.
---  Module:   <osrtl><window><winpaint>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/window/winpaint/winpaint.d  -  Window painter
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:45  erick
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
19970802
19970818
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><window><winpaint>
----------------------------------------------------------------------------

   The module <osrtl><window><winpaint> supplies a window painter.  A window
painter is needed whenever an application wishes to draw inside a window.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><window><winpaint>
----------------------------------------------------------------------------

   The type <ospainter> that is supplied by <osrtl><window><winpaint> may be
used freely by all other parts of the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><window><winpaint> does not depends on any other part
of the program.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><window><winpaint>
----------------------------------------------------------------------------

typedef  void *  ospainter;    // window painter

   An <ospainter> is an abstract window painter.  It exists to avoid
exposing the true nature (type) of a window painter to other units.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   The sole reason of existence of the module <osrtl><window><winpaint> is
to provide an abstract, platform-independent representation of a window
painter.  A window painter has been expressed as a <void *> because that
type allows to hide any actual data type.


----------------------------------------------------------------------------
---  End of file osrtl/windows/window/winpaint/winpaint.d .
----------------------------------------------------------------------------


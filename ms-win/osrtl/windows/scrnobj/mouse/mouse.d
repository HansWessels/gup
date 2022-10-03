
----------------------------------------------------------------------------
---  File:     osrtl/windows/drawing/mouse/mouse.d
---  Contents: Mouse cursors.
---  Module:   <osrtl><drawing><mouse>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/drawing/mouse/mouse.d  -  Mouse cursors
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:47  erick
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
19970726
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><drawing><mouse>
----------------------------------------------------------------------------

   The module <osrtl><drawing><mouse> allows user code to select the shape
of the mouse cursor.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><drawing><mouse>
----------------------------------------------------------------------------

   The module <osrtl><drawing><mouse> supplies but a single function.  This
function may be used without restrictions by any other part of the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><drawing><mouse> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <mouse> uses basic entities such as <osbool>.

      Component <osrtl><error>.
         <mouse> uses error-related types such as <osresult>.

      Component <osrtl><hardware>.
         <mouse> is functional only if a mouse device is present.  This is
         detected using the component <hardware>.

      Component <osrtl><startup>.
         <mouse> needs the instance handle of the application, which is
         supplied by the function <ProgramInstance> in the component
         <startup>.

      Module <osrtl><drawing><mouse>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><drawing><mouse>
----------------------------------------------------------------------------

enum
mousecursortype          // type of mouse cursor
{
   MOUSE_ARROW,          // standard mouse arrow
   MOUSE_BUSY            // "system is busy"
};  // mousecursortype

   This type is used to tell <SelectMouseCursor> which type of mouse cursor
must be selected.

----------------------------------------------------------------------------

osresult                   // result code
SelectMouseCursor          // selects a mouse cursor
(mousecursortype  type);   // I: type of the mouse cursor to select

   Selects a mouse cursor of type <type>.
   <SelectMouseCursor> loads the indicated mouse cursor from the system (if
<type> indicates one of the system's mouse cursors) or from the executable
file of the program (if <type> indicates a custom mouse cursor).
   <SelectMouseCursor> is functional only if a mouse device is present.
If no mouse is present, <SelectMouseCursor> does nothing but return
<OSRESULT_OK>.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/drawing/mouse/mouse.d .
----------------------------------------------------------------------------


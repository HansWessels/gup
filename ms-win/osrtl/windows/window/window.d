
----------------------------------------------------------------------------
---  File:      osrtl/windows/window/window.d
---  Contents:  Window management.
---  Component: <osrtl><window>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/window/window.d  -  Window management
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:23  erick
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
19970711
19970824
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the component <osrtl><window>
----------------------------------------------------------------------------

   The component <osrtl><window> supplies everything that is needed to
manage generic windows on the screen.  It allows to create and destroy
windows, to change their properties, and to paint their window areas.
It provides an easy interface between the program and the operating system.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><window>
----------------------------------------------------------------------------

   The component <osrtl><window> consists of the modules <winpaint>,
<winfont>, <admin>, <window>, and <safewin>.  <winfont>, though it is
exported, is not intended to be used directly by other parts of the program;
<admin> is not exported; <window> is intended to be used only through the
interface provided by <safewin>.  This means that <osrtl><window>
effectively only joins the modules <winpaint> and <safewin>.  Therefore,
refer to the documentation files for these two modules for a description of
how to use the component <osrtl><window>.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the component <osrtl><window>
----------------------------------------------------------------------------

   Because <osrtl><window> is nothing but an aggregate of the modules
<winpaint> and <safewin>, refer to the documentation files for these modules
for a detailed explanation of the component <osrtl><window>.
   To give an overview of the component <osrtl><window>, here follows a list
of items provided by the modules that make up <osrtl><window>.  The list
contains only those items that are intended to be used by other parts of the
program.

Type:     ospainter
Use:      A variable of this type is supplied by <oswindow> to its virtual
          function member <Paint>.  If <Paint> uses any of the dedicated
          paint functions, it must pass this variable to the paint functions
          it uses.

Class:    oswindow
Use:      A generic window.  This is a window that is displayed on the
          screen.  Refer to the descriptions of the components of
          <oswindow>, below, for details.

Function: oswindow::Create
Use:      Creates an <oswindow>.  The caller must specify the size of the
          <oswindow>.  A name for the <oswindow> is optional.  The
          <oswindow> may be visible or invisible.
          oswindow::Create creates the window without yet displaying it.
          Then, it calls the signal function <WindowCreation>.  If
          <WindowCreation> indicates success, the window is displayed (only
          if it should be visible); if <WindowCreation> indicates failure,
          the window is destroyed.

Function: oswindow::Destroy
Use:      Destroys an <oswindow> that was created by oswindow::Create .
          If the window is destroyed voluntarily (not forced),
          oswindow::Destroy calls <CanBeClosed> to ask the window if it can
          be closed.  If <CanBeClosed> indicates that the window can not be
          closed, the window is not destroyed.  If <CanBeClosed> indicates
          that the window can be closed, or if the destruction of the window
          is forced, oswindow::Destroy calls the signal function
          <WindowDestruction>.  It then closes and destroys the window.

Function: oswindow::CreateWindowFont
Use:      Creates a window font for use with the <oswindow>.  Refer to
          oswindowfont::Create , above, for details.

Function: oswindow::DestroyWindowFont
Use:      Destroys a window font that was created by
          oswindow::CreateWindowFont .

Function: oswindow::FontHeight
Use:      Obtains the distance between the top of the highest character of
          the font and the font's baseline.  Positive if the highest
          character extends above the baseline.

Function: oswindow::TextWidth
Use:      Given a text string, obtains the width of the left margin, the
          right margin, and the text itself if the text would be written to
          the window using a given window font.  These widths are the widths
          the string would have if it were written to the window while
          correcting for rounding effects (as oswindow::Write does).

Function: oswindow::Width
Use:      Obtains the current (actual) width of the window.

Function: oswindow::Height
Use:      Obtains the current (actual) height of the window.

Function: oswindow::Name
Use:      Obtains the current (actual) name of the window.

Function: oswindow::Visible
Use:      Obtains the current (actual) visibility state of the window.

Function: oswindow::Write
Use:      Writes a text to the window, using a given window font and a given
          text color.
          oswindow::Write is a paint function.  It must be called only in
          response to a call to the command function oswindow::Paint .  It
          must be passed the window painter that was passed to
          oswindow::Paint .

Function: oswindow::Line
Use:      Draws a thin, straight, solid line to the window, using a given
          color.
          oswindow::Line is a paint function.  It must be called only in
          response to a call to the command function oswindow::Paint .  It
          must be passed the window painter that was passed to
          oswindow::Paint .

Function: oswindow::OutlineRectangle
Use:      Draws the edges (not the interior) of a rectangle to the window,
          using a given color.
          oswindow::OutlineRectangle is a paint function.  It must be called
          only in response to a call to the command function
          oswindow::Paint .  It must be passed the window painter that was
          passed to oswindow::Paint .

Function: oswindow::FilledRectangle
Use:      Draws the edges and the interior of a rectangle to the window,
          using a given color.  The edges and the interior will have the
          same color.
          oswindow::FilledRectangle is a paint function.  It must be called
          only in response to a call to the command function
          oswindow::Paint .  It must be passed the window painter that was
          passed to oswindow::Paint .

Function: oswindow::WindowCreation
Use:      Called when the window is about to be created.  Allows the window
          to initialise, and possibly to discontinue the creation process.
          oswindow::WindowCreation is a signal function.
          oswindow::WindowCreation returns <OSRESULT_OK> to continue the
          creation process.

Function: oswindow::WindowDestruction
Use:      Called when the window is about to be destroyed.  Allows the
          window to close down.
          Note that the window will be destroyed unconditionally after this
          function is called: when this function is called, the window has
          no way to prevent its destruction.
          oswindow::WindowDestruction is a signal function.
          oswindow::WindowDestruction effectively does nothing.

Function: oswindow::WindowPosSize
Use:      Called when the position or the size of the window have changed,
          or when both the position and the size of the window have changed.
          May also be called when neither the position nor the size of the
          window have changed.
          oswindow::WindowPosSize is a signal function.
          oswindow::WindowPosSize does nothing.

Function: oswindow::WindowActivation
Use:      Called when the window is being activated after having been
          inactive.
          oswindow::WindowActivation is a signal function.
          oswindow::WindowActivation does nothing.

Function: oswindow::WindowDeactivation
Use:      Called when the window is being de-activated after having been
          active.
          oswindow::WindowDeactivation is a signal function.
          oswindow::WindowDeactivation does nothing.

Function: oswindow::MousePosition
Use:      Called when the position of the mouse cursor has changed.  This
          function is not called when the position of the mouse cursor has
          not changed.
          oswindow::MousePosition is a signal function.
          oswindow::MousePosition selects a default arrow for the mouse
          cursor.

Function: oswindow::MouseLeftSelect
Use:      Called when the user selected a point in the window with the left
          mouse button (in Windows, this means that the user single-clicked
          the left mouse button).
          oswindow::MouseLeftSelect is a signal function.
          oswindow::MouseLeftSelect allows the user to move the window if
          the window is not a full-screen window.

Function: oswindow::MouseLeftActivate
Use:      Called when the user activated a point in the window with the left
          mouse button (in Windows, this means that the user double-clicked
          the left mouse button).
          oswindow::MouseLeftActivate is a signal function.
          oswindow::MouseLeftActivate toggles the window between the full-
          screen and the non-full-screen state.

Function: oswindow::MouseLeftRelease
Use:      Called when the user released the left mouse button.
          oswindow::MouseLeftRelease is a signal function.
          oswindow::MouseLeftRelease does nothing.

Function: oswindow::MouseRightSelect
Use:      Called when the user selected a point in the window with the right
          mouse button (in Windows, this means that the user single-clicked
          the right mouse button).
          oswindow::MouseRightSelect is a signal function.
          oswindow::MouseRightSelect does nothing.

Function: oswindow::MouseRightActivate
Use:      Called when the user activated a point in the window with the
          right mouse button (in Windows, this means that the user double-
          clicked the right mouse button).
          oswindow::MouseRightActivate is a signal function.
          oswindow::MouseRightActivate does nothing.

Function: oswindow::MouseRightRelease
Use:      Called when the user released the right mouse button.
          oswindow::MouseRightRelease is a signal function.
          oswindow::MouseRightRelease does nothing.

Function: oswindow::CanBeClosed
Use:      Called to ask the window for permission to be closed and
          destroyed.  Note that there are two ways for a window to be
          closed: either voluntarily, in which case <CanBeClosed> is called
          to ask if the window agrees to being closed; or forced, in which
          case <CanBeClosed> is not called and the window is closed
          unconditionally.
          oswindow::CanBeClosed is an inquiry function.
          oswindow::CanBeClosed returns <OSTRUE> to indicate that the window
          can be closed.

Function: oswindow::SizeLimits
Use:      Called to ask the window to indicate limits to its size.  This
          allows the window to specify a minimum width and/or height, and/or
          a maximum width and/or height.
          oswindow::SizeLimits is an inquiry function.
          oswindow::SizeLimits sets the minimum width and height to 0.0 ,
          and the maximum width and height to 1.0 .

Function: oswindow::Paint
Use:      Tells the window that part of its window area needs repainting.
          The window is obliged to repaint the indicated area.  Painting
          must be done using one or more of the dedicated paint functions,
          described above.
          oswindow::Paint is a command function.
          oswindow::Paint makes the entire area that needs repainting black.


----------------------------------------------------------------------------
---  End of file osrtl/windows/window/window.d .
----------------------------------------------------------------------------


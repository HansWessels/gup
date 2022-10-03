
/**************************************************************************
 * File:     osrtl/windows/window/safewin/safewin.h                       *
 * Contents: Generic window.                                              *
 * Document: osrtl/windows/window/safewin/safewin.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/safewin/safewin.h  -  Generic window

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:34  erick
// First version of the MS-Windows shell (currently only osrtl).
//


/**************************************************************************
 * Detailed history.                                                      *
 **************************************************************************/

// -------------------------------------------------------------------------
// Revision  Concise description of the changes
// Dates
// Author
// -------------------------------------------------------------------------
//  1.1.000  First issue of this module header file.
// 19970720
// 19970820
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_WINDOW_SAFEWIN
#define OSRTL_WINDOW_SAFEWIN


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class oswindow:                 // generic window
   public osbasicwindow         //    basic window
{
public:
   // Life cycle.

   oswindow                     // initialises an <oswindow>
   ();

   virtual
   ~oswindow                    // closes down an <oswindow>
   ();

   virtual
   osresult                     // result code
   Create                       // creates an <oswindow>
   (float            left,      // I: x co-ordinate of left   edge, AS
    float            right,     // I: x co-ordinate of right  edge, AS
    float            top,       // I: y co-ordinate of top    edge, AS
    float            bottom,    // I: y co-ordinate of bottom edge, AS
    wchar_t const *  name,      // I: name of the window
    osbool           vis);      // I: must the window be visible?

protected:
   // Signal functions.

   virtual
   void
   WindowDestruction    // the window is being destroyed
   (long   time,        // I: time of window destruction
    float  x,           // I: x position of the mouse, WA
    float  y);          // I: y position of the mouse, WA

   virtual
   void
   WindowActivation     // the window is being activated
   (oswindow *  ,       // -: window that is being de-activated
    long        ,       // -: time of activation
    float       ,       // -: x position of the mouse, WA
    float       );      // -: y position of the mouse, WA

   virtual
   void
   WindowDeactivation   // the window is being de-activated
   (oswindow *  ,       // -: window that is being activated
    long        ,       // -: time of activation
    float       ,       // -: x position of the mouse, WA
    float       );      // -: y position of the mouse, WA

private:
   // Types.

   typedef  osbasicwindow  inherited;   // the base class of <oswindow>

   // Protective functions.

   oswindow             // dummy private copy constructor
   (oswindow const &)   //    prevents copying
   {}

   oswindow const &     // dummy private assignment operator
   operator =           //    prevents assignment
   (oswindow const &)
   const
   { return *this; }

   // Signal functions.

   virtual
   void
   WindowActivation             // the window is being activated
   (priv_windowhandle  other,   // I: window that is being de-activated
    long               time,    // I: time of activation
    float              x,       // I: x position of the mouse, WA
    float              y);      // I: y position of the mouse, WA

   virtual
   void
   WindowDeactivation           // the window is being de-activated
   (priv_windowhandle  other,   // I: window that is being activated
    long               time,    // I: time of activation
    float              x,       // I: x position of the mouse, WA
    float              y);      // I: y position of the mouse, WA

   // Supporting functions.

   osresult          // result code
   MarkDestruction   // marks destruction of an <oswindow>
   ();

   // Data.

   osbool      // does the <oswindow> exist?
   exists;

};  // oswindow


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#endif


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/window/safewin/safewin.h .                        *
 **************************************************************************/



/**************************************************************************
 * File:     shell/windows/startup/entry/entry.cpp                        *
 * Contents: Program entry.                                               *
 * Document: -                                                            *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) shell/windows/startup/entry/entry.cpp  -  Program entry

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:55  erick
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
//  1.1.000  First issue of this module source file.
// 19970716
// 19970825
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <shell>.
#include "shell/windows/settings.h"

// Units.
#include "osrtl/osrtl.h"    // NIL between program and (OS and RTL)

// Header file of the current module.
#include "shell/windows/startup/entry/entry.h"


// !!!
   class nicewindow:
      public oswindow
   {
   public:
      ~nicewindow();

   protected:
      virtual
      osresult                      // result code
      Paint                         // paints an area of the window
      (float              left,     // I: L edge of area that needs painting
       float              right,    // I: R edge of area that needs painting
       float              top,      // I: T edge of area that needs painting
       float              bottom,   // I: B edge of area that needs painting
       long               time,     // -: time of window creation
       float              x,        // -: x position of the mouse
       float              y,        // -: y position of the mouse
       ospainter const &  p);       // I: window painter

      virtual
      void
      MouseRightActivate   // the right mouse button was used to activate
      (long    ,           // -: time of activation with right mouse button
       float   ,           // -: new x position of the mouse
       float   ,           // -: new y position of the mouse
       osbool  ,           // -: left  mouse button is down?
       osbool  ,           // -: right mouse button is down?
       osbool  ,           // -: shift key 1 is down?
       osbool  ,           // -: shift key 2 is down?
       osbool  );          // -: shift key 3 is down?

   };  // nicewindow

   nicewindow::~nicewindow()
   {
      (void)0;
   }

   osresult                      // result code
   nicewindow::Paint             // paints an area of the window
   (float              left,     // I: L edge of area that needs painting
    float              right,    // I: R edge of area that needs painting
    float              top,      // I: T edge of area that needs painting
    float              bottom,   // I: B edge of area that needs painting
    long               time,     // -: time of window creation
    float              x,        // -: x position of the mouse
    float              y,        // -: y position of the mouse
    ospainter const &  p)        // I: window painter
   {
      // Make sure the window is black.
      oswindow::Paint(left, right, top, bottom, time, x, y, p);

      // Write text.
      {
         static  float  hor;

         oswindowfont     narrow;
         wchar_t const *  text = L"Erick Silkens";
         float            left;
         float            middle;
         float            full;

         CreateWindowFont(L"Prut", 0.18, 1.0, p, narrow);

         {
            static  osbool  first = OSTRUE;

            if (first)
            {
               first = OSFALSE;
               TextWidth(narrow, p, text, left, middle, full);
               hor = full / Height();
            }
         }

         OutlineRectangle(0.2, 0.2+hor*Height(), 0.12, 0.30,
                          oscolor(1.0, 1.0, 1.0), p);
         Write(0.2, 0.3, text, narrow, oscolor(0.0, 1.0, 1.0), p);

         DestroyWindowFont(narrow);
      }

      return OSRESULT_OK;
   }

   void
   nicewindow::MouseRightActivate   // the right mouse button was used to activate
   (long    ,           // -: time of activation with right mouse button
    float   ,           // -: new x position of the mouse
    float   ,           // -: new y position of the mouse
    osbool  ,           // -: left  mouse button is down?
    osbool  ,           // -: right mouse button is down?
    osbool  ,           // -: shift key 1 is down?
    osbool  ,           // -: shift key 2 is down?
    osbool  )           // -: shift key 3 is down?
   {
      Destroy();
   }


// !!!
   static  nicewindow  named;


/**************************************************************************
 * Function: ApplicationInitialise                                        *
 **************************************************************************/

int                     // status code
ApplicationInitialise   // initialises the program
()

{  // ApplicationInitialise

   // !!!
      named.Create(0.05, 0.95, 0.0, 1.0, L"Test", OSTRUE);
                                         

   // !!!
      return 0;

}  // ApplicationInitialise


/**************************************************************************
 * Function: ApplicationCloseDown                                         *
 **************************************************************************/

int                    // status code
ApplicationCloseDown   // closes down the program
()

{  // ApplicationCloseDown

   // !!!
      return 0;

}  // ApplicationCloseDown


/**************************************************************************
 * Function: ApplicationName                                              *
 **************************************************************************/

wchar_t const *   // name of the application
ApplicationName   // obtains the name of the application
()

{  // ApplicationName

   return L"GNU Pack";

}  // ApplicationName


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of shell/windows/startup/entry/entry.cpp .                         *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/window/admin/admin.h                           *
 * Contents: Window administration.                                       *
 * Document: osrtl/windows/window/admin/admin.d                           *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/admin/admin.h  -  Window administration

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:31  erick
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
// 19970711
// 19970806
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_WINDOW_ADMIN
#define OSRTL_WINDOW_ADMIN


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

class windowadmin:              // window administrator
   public osobject              //    generic object
{
public:
   // Types.

   typedef
   wchar_t const *              // ID of a window class
   windowclassid;

   typedef
   void *                       // window procedure
   windowproc;

   // Life cycle.

   windowadmin                  // initialises a <windowadmin>
   ();

   static
   osresult                     // result code
   Create                       // administers creation of a window
   (windowproc       Handler,   // I: window procedure
    windowclassid *  id);       // O: ID of the window class

   static
   osresult                     // result code
   Destroy                      // administers destruction of a window
   ();

protected:
   // Informational functions.

   static
   inline
   unsigned int                 // number of existing windows
   Windows                      // obtains the number of existing windows
   ();

   static
   inline
   osbool                       // has a window ever existed
   Ever                         // checks if a window ever has existed
   ();

   static
   inline
   windowclassid const          // name of the window class
   ClassName                    // obtains the name of the window class
   ();

private:
   // Preventive functions.

   windowadmin                  // dummy private copy constructor
   (windowadmin const &)        //    prevents copying
   {}

   windowadmin const &          // dummy private assignment operator
   operator =                   //    prevents assignment
   (windowadmin const &)
   const
   { return *this; }

   // Data.

   static
   unsigned int                 // number of existing windows
   windows;

   static
   osbool                       // has a window ever existed?
   ever;

   static
   windowclassid const          // name of the window class
   classname;

};  // windowadmin


/**************************************************************************
 * Function: windowadmin::Windows                                         *
 **************************************************************************/

inline
unsigned int           // number of existing windows
windowadmin::Windows   // obtains the number of existing windows
()

{  // windowadmin::Windows

   return windows;

}  // windowadmin::Windows


/**************************************************************************
 * Function: windowadmin::Ever                                            *
 **************************************************************************/

inline
osbool              // has a window ever existed
windowadmin::Ever                         // checks if a window ever has existed
()

{  // windowadmin::Ever

   return ever;

}  // windowadmin::Ever


/**************************************************************************
 * Function: windowadmin::ClassName                                       *
 **************************************************************************/

inline
windowadmin::windowclassid const   // name of the window class
windowadmin::ClassName             // obtains the name of the window class
()

{  // windowadmin::ClassName

   return classname;

}  // windowadmin::ClassName


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
 * End of osrtl/windows/window/admin/admin.h .                            *
 **************************************************************************/


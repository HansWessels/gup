
/**************************************************************************
 * File:     osrtl/windows/basics/object/object.cpp                       *
 * Contents: Generic object.                                              *
 * Document: osrtl/windows/basics/object/object.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/basics/object/object.cpp  -  Generic object

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:08  erick
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
//  1.1.000  First issue of this source module.
// 19970621
// 19970806
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// The unit <osrtl> is the only unit that is allowed to include files that
// have to do with the operating system and the RTL.
#define STRICT
#include <windows.h>   // header file of the OS

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other modules of the component <basics>
#include "osrtl/windows/basics/constant/constant.h"   // constants
#include "osrtl/windows/basics/type/type.h"           // types

// Header file of the current module.
#include "osrtl/windows/basics/object/object.h"


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

#ifndef OSRTL_DEBUGGING
   osbool
   osobject::truetrue   // <OSTRUE>
   = OSTRUE;
#endif


/**************************************************************************
 * Function: osobject::~osobject                                          *
 **************************************************************************/

osobject::~osobject   // closes down an <osobject>
()

{  // osobject::~osobject

   #ifdef OSRTL_DEBUGGING
      // Kill the name of the object, if it has been defined.
      if (!KillName())
         #ifdef OSRTL_UNICODE
            MessageBox(static_cast<HWND>(OSNULL),
                       L"Cannot kill the name of an <osobject>.",
                       L"Debug error",
                       MB_SYSTEMMODAL | MB_ICONHAND);
         #else
            MessageBoxA(static_cast<HWND>(OSNULL),
                       "Cannot kill the name of an <osobject>.",
                       "Debug error",
                       MB_SYSTEMMODAL | MB_ICONHAND);
         #endif
   #else
      // This is a dummy destructor that is provided only because
      // the coding standard requires that every base class have a
      // virtual destructor, even if such a destructor is empty.
      (void)0;
   #endif

}  // osobject::~osobject


/**************************************************************************
 * Function: osobject::DefineName                                         *
 **************************************************************************/

osbool                          // object name could be set?
osobject::DefineName            // sets the name of the object
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  newname)   // I: name of the object
#else
   (wchar_t const *  )          // -: name of the object
#endif

{  // osobject::DefineName

   osbool  result = OSTRUE;   // function result

   #ifdef OSRTL_DEBUGGING
   {
      wchar_t *  copy;   // copy of <newname> on the heap

      // Do not delete <name> first: <newname> might be identical to <name>,
      // or <newname> might point somewhere inside <name>.

      if (newname != static_cast<wchar_t const *>(OSNULL))
      {
         int  length;   // length, in characters, of <newname>

         // Count the number of characters in <newname>.
         {
            length = 0;
            while (newname[length] != L'\0')
               ++length;
         }

         // Allocate space for <copy>.
         {
            // new[] may throw an exception.  To prevent any exception
            // from reaching the rest of the program, intercept all such
            // exceptions.
            try
            {
               copy = new wchar_t[length+1];
            }
            catch (...)
            {
               copy = static_cast<wchar_t *>(OSNULL);
            }
         }

         if (copy != static_cast<wchar_t *>(OSNULL))
         {
            wchar_t *  nm = copy;   // name of the object

            // Copy <newname> to <copy>.
            while ((*nm++ = *newname++) != L'\0')
               (void)0;
         }
         else
            result = OSFALSE;
      }
      else
         copy = static_cast<wchar_t *>(OSNULL);

      if (result)
         if (KillName())
            name = copy;
         else
         {
            // Delete <copy>.
            if (copy != static_cast<wchar_t *>(OSNULL))
            {
               // delete[] may throw an exception.  To prevent any exception
               // from reaching the rest of the program, intercept all such
               // exceptions.
               try
               {
                  delete[] copy;
               }
               catch (...)
               {
                  (void)0;
               }
            }

            result = OSFALSE;
         }
   }
   #endif

   return result;

}  // osobject::DefineName


/**************************************************************************
 * Function: osobject::RetrieveName                                       *
 **************************************************************************/

wchar_t const *          // name of the object
osobject::RetrieveName   // retrieves the name of the object
()
const

{  // osobject::RetrieveName

   wchar_t const *  result;   // function result

   #ifdef OSRTL_DEBUGGING
      result = const_cast<wchar_t const *>(name);
   #else
      result = static_cast<wchar_t const *>(OSNULL);
   #endif

   return result;

}  // osobject::RetrieveName


/**************************************************************************
 * Function: osobject::KillName                                           *
 **************************************************************************/

osbool               // whether the name could be killed
osobject::KillName   // kills the name of the object
()

{  // osobject::KillName

   osbool  result = OSTRUE;   // function result

   #ifdef OSRTL_DEBUGGING
      if (name != static_cast<wchar_t *>(OSNULL))
      {
         // <delete> may throw an exception.  To prevent any exception from
         // reaching the rest of the program, intercept all such exceptions.
         try
         {
            delete[] name;
            name = static_cast<wchar_t *>(OSNULL);
         }
         catch (...)
         {
            result = OSFALSE;
         }
      }
   #endif

   return result;

}  // osobject::KillName


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/basics/object/object.cpp .                        *
 **************************************************************************/


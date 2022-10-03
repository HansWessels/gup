
/**************************************************************************
 * File:     osrtl/windows/startup/startup/startup.cpp                    *
 * Contents: Program start-up.                                            *
 * Document: osrtl/windows/startup/startup/startup.d                      *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/startup/startup/startup.cpp  -  Program start-up

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:59  erick
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
// 19970609
// 19970727
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

// Other components of the unit <osrtl>
#include "osrtl/windows/basics/basics.h"   // basic definitions
#include "osrtl/windows/error/error.h"     // error handling

// Header file of the current module.
#include "osrtl/windows/startup/startup/startup.h"


/**************************************************************************
 * Local functions.                                                       *
 **************************************************************************/

// Because this module is meant to start the program, it must invoke the
// user's code.  This module is at the bottom of the hierarchy of modules;
// it must invoke the entry point of the user's code, which is at the top
// of the hierarchy.  To make this possible while adhering to the Coding
// Standard, the entry point into the user's code is defined _locally_ as
// an external function (which it is).
// A similar reasoning applies to the user's code that returns the name of
// the application.

extern
int                     // status code
ApplicationInitialise   // initialises the program
();

extern
int                     // status code
ApplicationCloseDown    // closes down the program
();

extern
wchar_t const *         // name of the application
ApplicationName         // obtains the name of the application
();


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

static
wchar_t const *                           // name of the application
progname
= static_cast<wchar_t const *>(OSNULL);

static
programinstance                           // instance of the application
proginst
= static_cast<programinstance>(OSNULL);

static
osbool                                    // whether one or more windows
windows                                   //    exist that were created by
= OSFALSE;                                //    the application


/**************************************************************************
 * Function: WinMain                                                      *
 **************************************************************************/

int                     // status code
PASCAL                  // calling convention - required by Windows
WinMain                 // main function; program entry point
(HINSTANCE  instance,   // I: current instance of application
 HINSTANCE  ,           // -: previous instance of application
 LPSTR      ,           // -: command line
 int        )           // -: display mode for main window

{  // WinMain

   int  status = 0 /*!!!*/;   // status code

   // Store important data.
   proginst = static_cast<programinstance>(instance);

   // Ask the user code for the name of the application.
   {
      wchar_t const *  appname;   // name of the application

      try
      {
         appname = ApplicationName();
      }
      catch (...)
      {
         appname = static_cast<wchar_t const *>(OSNULL);
      }

      if (appname != static_cast<wchar_t const *>(OSNULL))
      {
         int        length = 0;   // length of <appname>
         wchar_t *  name;         // copy of <appname>

         while (appname[length] != L'\0')
            ++length;

         try
         {
            name = new wchar_t[length+1];
         }
         catch (...)
         {
            name = static_cast<wchar_t *>(OSNULL);
         }

         if (name != static_cast<wchar_t *>(OSNULL))
         {
            int  i = length;

            // Copy <appname> to <name>.
            while (i >= 0)
            {
               name[i] = appname[i];
               --i;
            }
         }
         else
            status = 2468;   // !!!

         progname = const_cast<wchar_t const *>(name);
      }
      else
         progname = static_cast<wchar_t const *>(OSNULL);
   }

   if (status == 0 /*!!!*/)
   {
      // Initialise the user code.
      try
      {
         status = ApplicationInitialise();
      }
      catch (...)
      {
         OsDebugErrorMessage(L"Application error:\n"
                             L"An exception was thrown but not caught.");
         /*!!!*/ status = 123456789;
      }

      // Run the main message loop that drives execution of the program.
      if (status == 0 /*!!!*/)
      {
         if (windows)
         {
            osbool  terminate = OSFALSE;   // terminate the program?

            do
            {
               MSG  msg;   // message for one of our windows

               if (GetMessage(&msg,
                              static_cast<HWND>(OSNULL),
                              static_cast<UINT>(0),
                              static_cast<UINT>(0)))
               {
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }
               else
                  // A "quit" message was retrieved.  Terminate the program.
                  terminate = OSTRUE;
            } while (!terminate);
         }
         else
         {
            OsDebugErrorMessage(L"The application didn't indicate\n"
                                L"that it has created windows.");
            status = 4477;  /*!!!*/
         }
      }

      // Close down the user code.
      try
      {
         int  closedownstatus;   // status of closing down the program

         closedownstatus = ApplicationCloseDown();
         if (windows)
            OsDebugErrorMessage(L"The application has closed down,\n"
                                L"but some windows still exist.");
         if (status == 0 /*!!!*/)
            status = closedownstatus;
      }
      catch (...)
      {
         OsDebugErrorMessage(L"Application error:\n"
                             L"An exception was thrown but not caught.");
         /*!!!*/ status = 123456789;
      }

      // Delete <progname>.
      try
      {
         delete[] const_cast<wchar_t *>(progname);
      }
      catch (...)
      {
         OsDebugErrorMessage(L"Cannot delete the name of the program.");
         if (status == 0 /*!!!*/)
            status = 13579;  /*!!!*/
      }
   }
   else
      OsErrorMessage(L"Out of memory.  Cannot start the program.",
                     L"Error");

   return status;

}  // WinMain


/**************************************************************************
 * Function: ProgramName                                                  *
 **************************************************************************/

wchar_t const *   // name of the program
ProgramName       // obtains the name of the program
()

{  // ProgramName

   return progname;

}  // ProgramName


/**************************************************************************
 * Function: ProgramInstance                                              *
 **************************************************************************/

programinstance   // program instance
ProgramInstance   // instance of the application
()

{  // ProgramInstance

   return proginst;

}  // ProgramInstance


/**************************************************************************
 * Function: WindowExists                                                 *
 **************************************************************************/

void
WindowExists      // tells <osrtl><startup><startup> whether windows exist
(osbool  exist)   // I: whether application windows exist

{  //  WindowExists

   windows = exist;

}  // WindowExists


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/startup/startup/startup.cpp .                     *
 **************************************************************************/


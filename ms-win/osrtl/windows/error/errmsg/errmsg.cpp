
/**************************************************************************
 * File:     osrtl/windows/error/errmsg/errmsg.cpp                        *
 * Contents: Error messages.                                              *
 * Document: osrtl/windows/error/errmsg/errmsg.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/error/errmsg/errmsg.cpp  -  Error messages

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:17  erick
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
// 19970717
// 19970724
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

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"   // basic definitions

// Header file of the current module.
#include "osrtl/windows/error/errmsg/errmsg.h"


/**************************************************************************
 * Local constants.                                                       *
 **************************************************************************/

const  int  MAXMESSAGELENGTH = 180;   // maximum number of characters in
                                      //    the text of an error message
const  int  MAXTITLELENGTH   =  60;   // maximum number of characters in
                                      //    the title of a message box


/**************************************************************************
 * Local functions.                                                       *
 **************************************************************************/

static
void
AddMessage                        // adds a message to a message string
(wchar_t * *      dst,            // O: destination message string
 wchar_t const *  src,            // I: message to add
 int              max);           // I: maximum number of characters to add

#ifndef OSRTL_UNICODE
   static
   void
   Convert
   (wchar_t const *  src,         // I: source string
    char *           dst,         // O: destination string
    int              maxlength);  // I: maximum length of destination string
#endif


/**************************************************************************
 * Function: OsErrorMessage                                               *
 **************************************************************************/

void
OsErrorMessage                // displays an error message
(wchar_t const *  message1,   // I: error message, part 1
 wchar_t const *  message2,   // I: error message, part 2
 wchar_t const *  message3,   // I: error message, part 3
 wchar_t const *  title)      // I: title of the message box

{  // OsErrorMessage

   wchar_t  message[MAXMESSAGELENGTH+1];

   {
      wchar_t *  next = message;   // address of next message part

      AddMessage(&next, message1, ((message+sizeof(message))-next)-1);
      AddMessage(&next, message2, ((message+sizeof(message))-next)-1);
      AddMessage(&next, message3, ((message+sizeof(message))-next)-1);
      *next = L'\0';
   }

   #ifdef OSRTL_UNICODE
      MessageBox(static_cast<HWND>(OSNULL),
                 message,
                 title,
                 MB_SYSTEMMODAL | MB_ICONHAND);
   #else
      char  ansimessage[MAXMESSAGELENGTH+1];   // ANSI version of <message>
      char  ansititle[MAXTITLELENGTH+1];       // ANSI version of <title>

      Convert(message, ansimessage, MAXMESSAGELENGTH);
      Convert(title,   ansititle,   MAXTITLELENGTH  );

      MessageBoxA(static_cast<HWND>(OSNULL),
                 ansimessage,
                 ansititle,
                 MB_SYSTEMMODAL | MB_ICONHAND);
   #endif

}  // OsErrorMessage


/**************************************************************************
 * Function: AddMessage                                                   *
 **************************************************************************/

static
void
AddMessage               // adds a message to a message string
(wchar_t * *      dst,   // O: destination message string
 wchar_t const *  src,   // I: message to add
 int              max)   // I: maximum number of characters to add

{  // AddMessage

   if (src != static_cast<wchar_t const *>(OSNULL))
   {
      wchar_t *  target = *dst;   // destination string

      while (*src != L'\0'  &&  max > 0)
      {
         *target++ = *src++;
         --max;
      }

      *dst = target;
   }

}  // AddMessage


/**************************************************************************
 * Function: Convert                                                      *
 **************************************************************************/

#ifndef OSRTL_UNICODE

   static
   void
   Convert
   (wchar_t const *  src,         // I: source string
    char *           dst,         // O: destination string
    int              maxlength)   // I: maximum length of destination string

   {  // Convert

      while (*src != L'\0'  &&  maxlength > 0)
         if (CharConvert(*src++, dst))
         {
            ++dst;
            --maxlength;
         }

      *dst = '\0';

   }  // Convert

#endif


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/error/errmsg/errmsg.cpp .                         *
 **************************************************************************/


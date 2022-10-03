
/**************************************************************************
 * File:     osrtl/windows/error/errmsg/errmsg.h                          *
 * Contents: Error messages.                                              *
 * Document: osrtl/windows/error/errmsg/errmsg.d                          *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/error/errmsg/errmsg.h  -  Error messages

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:41:18  erick
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
// 19970717
// 19970724
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_ERROR_ERRMSG
#define OSRTL_ERROR_ERRMSG


/**************************************************************************
 * Exported functions.                                                    *
 **************************************************************************/

inline
void
OsErrorMessage                   // displays an error message
(wchar_t const *  message,       // I: error message
 wchar_t const *  title);        // I: title of the message box

inline
void
OsErrorMessage                   // displays an error message
(wchar_t const *  message1,      // I: error message, part 1
 wchar_t const *  message2,      // I: error message, part 2
 wchar_t const *  title);        // I: title of the message box

void
OsErrorMessage                   // displays an error message
(wchar_t const *  message1,      // I: error message, part 1
 wchar_t const *  message2,      // I: error message, part 2
 wchar_t const *  message3,      // I: error message, part 3
 wchar_t const *  title);        // I: title of the message box

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message);   // I: error message
#else
   (wchar_t const *  );          // -: error message
#endif

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2);  // I: error message, part 2
#else
   (wchar_t const *  ,           // -: error message, part 1
    wchar_t const *  );          // -: error message, part 2
#endif

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2,   // I: error message, part 2
    wchar_t const *  message3);  // I: error message, part 3
#else
   (wchar_t const *  ,           // -: error message, part 1
    wchar_t const *  ,           // -: error message, part 2
    wchar_t const *  );          // -: error message, part 3
#endif


/**************************************************************************
 * Function: OsErrorMessage                                               *
 **************************************************************************/

inline
void
OsErrorMessage               // displays an error message
(wchar_t const *  message,   // I: error message
 wchar_t const *  title)     // I: title of the message box

{  // OsErrorMessage

   OsErrorMessage(message,
                  static_cast<wchar_t const *>(OSNULL),
                  static_cast<wchar_t const *>(OSNULL),
                  title);

}  // OsErrorMessage


/**************************************************************************
 * Function: OsErrorMessage                                               *
 **************************************************************************/

inline
void
OsErrorMessage                // displays an error message
(wchar_t const *  message1,   // I: error message, part 1
 wchar_t const *  message2,   // I: error message, part 2
 wchar_t const *  title)      // I: title of the message box

{  // OsErrorMessage

   OsErrorMessage(message1,
                  message2,
                  static_cast<wchar_t const *>(OSNULL),
                  title);

}  // OsErrorMessage


/**************************************************************************
 * Function: OsDebugErrorMessage                                          *
 **************************************************************************/

inline
void
OsDebugErrorMessage             // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message)   // I: error message
#else
   (wchar_t const *  )          // I: error message
#endif

{  // OsDebugErrorMessage

   #ifdef OSRTL_DEBUGGING
      OsErrorMessage(message, L"Debug error");
   #else
      (void)0;
   #endif

}  // OsDebugErrorMessage


/**************************************************************************
 * Function: OsDebugErrorMessage                                          *
 **************************************************************************/

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2)   // I: error message, part 2
#else
   (wchar_t const *  ,           // I: error message, part 1
    wchar_t const *  )           // I: error message, part 2
#endif

{  // OsDebugErrorMessage

   #ifdef OSRTL_DEBUGGING
      OsErrorMessage(message1, message2, L"Debug error");
   #else
      (void)0;
   #endif

}  // OsDebugErrorMessage


/**************************************************************************
 * Function: OsDebugErrorMessage                                          *
 **************************************************************************/

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2,   // I: error message, part 2
    wchar_t const *  message3)   // I: error message, part 3
#else
   (wchar_t const *  ,           // I: error message, part 1
    wchar_t const *  ,           // I: error message, part 2
    wchar_t const *  )           // I: error message, part 3
#endif

{  // OsDebugErrorMessage

   #ifdef OSRTL_DEBUGGING
      OsErrorMessage(message1, message2, message3, L"Debug error");
   #else
      (void)0;
   #endif

}  // OsDebugErrorMessage


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
 * End of osrtl/windows/error/errmsg/errmsg.h .                           *
 **************************************************************************/


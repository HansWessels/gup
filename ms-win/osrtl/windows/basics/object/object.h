
/**************************************************************************
 * File:     osrtl/windows/basics/object/object.h                         *
 * Contents: Generic object.                                              *
 * Document: osrtl/windows/basics/object/object.d                         *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/basics/object/object.h  -  Generic object

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
//  1.1.000  First issue of this header file.
// 19970621
// 19970726
// EAS
// -------------------------------------------------------------------------
//  1.1.001  0. osobject::DefineName , osobject::RetrieveName and
// 19970822     osobject::KillName have been changed into normal (non-
// 19970822     virtual) functions.  There was no need for these functions
// EAS          to be virtual.
// -------------------------------------------------------------------------
//  1.2.000  Identical to revision 1.1.001.  Formalised revision.
// 19970825
// 19970825
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Prevent multiple inclusion.                                            *
 **************************************************************************/

#ifndef OSRTL_BASICS_OBJECT
#define OSRTL_BASICS_OBJECT


/**************************************************************************
 * Exported types.                                                        *
 **************************************************************************/

// Basic class type (empty) from which any other class type may be derived.
// This allows any such class type to be accessed as if it were an
// <osobject>.

class osobject                     // basic type
{
public:
   // Life cycle.

   inline
   osobject                        // initialises an <osobject>
   ();

   virtual
   ~osobject                       // closes down an <osobject>
   ();

   // Actual functionality.

   inline
   osbool                          // object name could be set?
   SetName                         // sets the name of the object
   #ifdef OSRTL_DEBUGGING
      (wchar_t const *  newname);  // I: new name of the object
   #else
      (wchar_t const *  );         // -: new name of the object
   #endif

   inline
   wchar_t const *                 // name of the object
   ObjectName                      // retrieves the name of the object
   ()
   const;

private:
   // Supporting functions.

   osbool                          // object name could be defined?
   DefineName                      // defines the name of the object
   #ifdef OSRTL_DEBUGGING
      (wchar_t const *  newname);  // I: new name of the object
   #else
      (wchar_t const *  );         // -: new name of the object
   #endif

   wchar_t const *                 // name of the object
   RetrieveName                    // retrieves the name of the object
   ()
   const;

   osbool                          // whether the name could be killed
   KillName                        // kills the name of the object
   ();

   #ifdef OSRTL_DEBUGGING
      wchar_t *                    // name of the object
      name;
   #else
      static
      osbool                       // <OSTRUE>
      truetrue;
   #endif

};  // osobject


/**************************************************************************
 * Function: osobject::osobject                                           *
 **************************************************************************/

inline
osobject::osobject   // initialises an <osobject>
()

{  // osobject::osobject

   #ifdef OSRTL_DEBUGGING
      name = static_cast<wchar_t *>(OSNULL);
   #else
      (void)0;
   #endif

}  // osobject::osobject


/**************************************************************************
 * Function: osobject::SetName                                            *
 **************************************************************************/

inline
osbool                          // object name could be set?
osobject::SetName               // sets the name of the object
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  newname)   // I: new name of the object
#else
   (wchar_t const *  )          // -: new name of the object
#endif

{  // osobject::SetName

   #ifdef OSRTL_DEBUGGING
      return DefineName(newname);
   #else
      return truetrue;
   #endif

}  // osobject::SetName


/**************************************************************************
 * Function: osobject::ObjectName                                         *
 **************************************************************************/

inline
wchar_t const *        // name of the object
osobject::ObjectName   // retrieves the name of the object
()
const

{  // osobject::ObjectName

   #ifdef OSRTL_DEBUGGING
      return RetrieveName();
   #else
      return static_cast<wchar_t const *>(OSNULL);
   #endif

}  // osobject::ObjectName


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
 * End of osrtl/windows/basics/object/object.h .                          *
 **************************************************************************/


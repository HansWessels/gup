
----------------------------------------------------------------------------
---  File:     osrtl/windows/tasking/mutexid/mutexid.d
---  Contents: Mutex IDs.
---  Module:   <osrtl><tasking><mutexid>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/tasking/mutexid/mutexid.d  -  Mutex IDs
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:08  erick
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
19970716
19970813
EAS
----------------------------------------------------------------------------
 1.1.001  The description of <mutexid> has been adapted according to the
19970823  changes that were made in revision 1.1.001 of
19970823  osrtl/windows/tasking/mutexid/mutexid.h .
EAS
----------------------------------------------------------------------------
 1.2.000  Identical to revision 1.1.001.  Formalised revision.
19970826
19970826
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><tasking><mutexid>
----------------------------------------------------------------------------

   The module <osrtl><tasking><mutexid> supplies IDs for physical mutexes.
Refer to the documentation of the component <osrtl><tasking> for a descript-
ion of physical mutexes and access mutexes.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><tasking><mutexid>
----------------------------------------------------------------------------

   All other parts of the project can directly use the mutex IDs that are
provided by <mutexid>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><tasking><mutexid> does not depend on any other part of
the program.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><tasking><mutexid>
----------------------------------------------------------------------------

typedef  unsigned long  mutexid;

   A <mutexid> is a unique ID for a physical mutex.

----------------------------------------------------------------------------

const  mutexid  MUTEX_MUTEXADMIN   = 0UL;    // mutex administration
const  mutexid  MUTEX_HEAPADMIN    = 1UL;    // heap administration
const  mutexid  MUTEX_FONTADMIN    = 2UL;    // font administration
const  mutexid  MUTEX_FONTMANAGE   = 3UL;    // font management
const  mutexid  MUTEX_WINDOWADMIN  = 4UL;    // window administration
const  mutexid  MUTEX_WINDOWMANAGE = 5UL;    // window management
const  mutexid  MUTEX_SAFEWINDOW   = 6UL;    // management of safe windows

   These are the predefined mutex IDs, for use by other parts of <osrtl>.

----------------------------------------------------------------------------

const  mutexid  MUTEX_APPLICATION  = 1000UL;   // first mutex ID for use by
                                               //    the application

   This mutex ID is not really the ID of a mutex.  Instead, it contains the
lowest value for a mutex ID that may be used by applications that are built
on top of <osrtl>.  Mutex IDs below <MUTEX_APPLICATION> are reserved for use
by <osrtl>; those mutex IDs must not be used by applications.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/tasking/mutexid/mutexid.d .
----------------------------------------------------------------------------


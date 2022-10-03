
----------------------------------------------------------------------------
---  File:     osrtl/windows/tasking/physmux/physmux.d
---  Contents: Physical mutexes.
---  Module:   <osrtl><tasking><physmux>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/tasking/physmux/physmux.d  -  Physical mutexes
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:11  erick
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
19970723
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><tasking><physmux>
----------------------------------------------------------------------------

   The module <osrtl><tasking><physmux> supplies physical mutexes.  Refer to
the documentation of the component <osrtl><tasking> for a description of
physical mutexes and access mutexes.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><tasking><physmux>
----------------------------------------------------------------------------

   Other parts of the program must not use physical mutexes directly.
Instead, they must use the access mutexes that are provided by the module
<osrtl><tasking><mutex>; these access mutexes deal with the physical
mutexes.
   Refer to the documentation of the component <osrtl><tasking> for a
description of physical mutexes and access mutexes.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><tasking><physmux> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <physmux> uses basic types like <osbool>, and basic constants like
         <OSNULL>.  The class <physmutex> is derived from the basic class
         <osobject>.

      Component <osrtl><error>.
         <physmux> uses <OsDebugErrorMessage> to display errors.

      Module <osrtl><tasking><mutexed>.
         Each physical mutex has a mutex ID.

      Module <osrtl><tasking><physmux>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><tasking><physmux>
----------------------------------------------------------------------------

class physmutex:     // physical mutex
   public osobject   //    generic object
{
   // ...
};  // physmutex

   A physical mutex.  Refer to the descriptions of the components of
<physmutex>, below, for an explanation of the workings of this class type.

----------------------------------------------------------------------------

physmutex::physmutex   // initialises a physical mutex
(mutexid  id);         // I: ID of the physical mutex

   Initialises a physical mutex, giving it the ID <id>.

----------------------------------------------------------------------------

virtual
physmutex::~physmutex   // closes down a physical mutex
();

   Closes down a physical mutex.  In debugging mode, performs some checks on
the administration and the use of the mutex, and warns the user if any non-
nominal condition was found.

----------------------------------------------------------------------------

void
physmutex::Connect   // an access mutex connects to the physical mutex
();

   Administers the fact that an access mutex connects to the physical mutex
by updating the internal administration of the physical mutex.
   This function uses global variables.  In doing so, it assumes that no
other physical mutex is accessing the same global variables at the same
time.  As long as <physmux> executes in a single thread only, this condition
is satisfied.

----------------------------------------------------------------------------

void
physmutex::Disconnect   // an access mutex disconnects from the phys. mutex
();

   Administers the fact that an access mutex disconnects from the physical
mutex by updating the internal administration of the physical mutex.
   This function uses global variables.  In doing so, it assumes that no
other physical mutex is accessing the same global variables at the same
time.  As long as <physmux> executes in a single thread only, this condition
is satisfied.

----------------------------------------------------------------------------

void
physmutex::EnterSection   // waits until it's safe to enter critical section
();

   Enters a critical section of code.  If another process is already inside
a critical section that is guarded by the same physical mutex, this function
waits until that process leaves the critical section.  If no other process
is inside such a critical section, this function just marks the critical
section as being "occupied".  The net result is that at any time at most one
process can be inside a critical section that is guarded by this physical
mutex.
   The current version of this function is targeted for a single-threaded
environment.  In such an environment, it is not possible that more than one
process enters a critical section at one time.  Therefore, this function
does nothing except from maintaining some internal administration.
   Each critical section must be left with physmutex::LeaveSection .

----------------------------------------------------------------------------

void
physmutex::LeaveSection   // tells that critical section was left
();

   Leaves a critical section of code.  This allows other processes that want
to enter a critical section of code that is guarded by the same physical
mutex to continue.
   The current version of this function is targeted for a single-threaded
environment.  In such an environment, it is not possible that more than one
process enters a critical section at one time.  Therefore, this function
does nothing except from maintaining some internal administration.
   Each critical section must be entered with physmutex::EnterSection .

----------------------------------------------------------------------------

inline
physmutex::mutexid   // the ID of the physical mutex
Id                   // obtains the ID of the physical mutex
()
const;

   Returns the ID of the physical mutex.  This ID was given to the mutex
when it was constructed.

----------------------------------------------------------------------------

inline
osbool                 // any access mutex connected to the physical mutex?
physmutex::Connected   // checks if any access mutex is connected
()
const;

   Check if one or more access mutexes are connected to the physical mutex.
This function is intended for managing physical mutexes by the module
<mutex>.

----------------------------------------------------------------------------

inline
void
physmutex::SetNextElement   // defines the next element in the list
(physmutex *  elm);         // I: the next element in the list

   Each physical mutex is prepared to be part of a linked list of physical
mutexes.  This function defines the next element in such a list.

----------------------------------------------------------------------------

inline
physmutex *              // next element in the list
physmutex::NextElement   // retrieves the next element in the list
()
const;

   Each physical mutex is prepared to be part of a linked list of physical
mutexes.  This function retrieves the next element in such a list.

----------------------------------------------------------------------------

physmutex::physmutex   // dummy private default constructor
()                     //    prevents default construction
{}

   A physical mutex must have an ID at all times.  Therefore, default
construction of a physical mutex is disabled.  The only available
constructor is the one that takes a mutex ID as an argument.

----------------------------------------------------------------------------

physmutex::physmutex    // dummy private copy constructor
(physmutex const &)     //    prevents copying
{}

physmutex const &       // dummy private assignment operator
physmutex::operator =   //    prevents assignment
(physmutex const &)
const
{ return *this; }

   A physical mutex must be unique: there must be at most one physical mutex
with a given mutex ID.  Therefore, making copies of a physical mutex is dis-
abled.

----------------------------------------------------------------------------

mutexid             // ID of this physical mutex
physmutex::ident;

   This data member contains the mutex ID of the physical mutex.

----------------------------------------------------------------------------

osbool               // any task inside critical section?
physmutex::inside;

   This data member remembers if any process is inside a critical section
that is guarded by this physical mutex.

----------------------------------------------------------------------------

int                 // #users of this physical mutex
physmutex::users;

   This data member counts the number of access mutexes that are connected
to the physical mutex.

----------------------------------------------------------------------------

physmutex *        // next physical mutex in the list
physmutex::next;

   This data member allows each physical mutex to be part of a linked list
of physical mutexes.

----------------------------------------------------------------------------

class guard_osrtl_tasking_physmux   // guards this module
{
   // ...
};  // guard_osrtl_tasking_physmux

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

guard_osrtl_tasking_physmux::~guard_osrtl_tasking_physmux   // performs exit
();                                                         //    checks on
                                                            //    the module

   In debugging mode, this destructor checks the global variables that
are maintained by the physical mutexes for anomalies.  If any anomaly is
detected, <OsDebugErrorMessage> is used to warn the developer.
   In release mode, this destructor does nothing.

----------------------------------------------------------------------------

static
int
alive    // total number of existing physical mutexes
= 0;

   The physical mutexes use this global variable to count the total number
of physical mutexes that are alive.  <guard_osrtl_tasking_physmux> uses this
variable to check if all physical mutexes have been destroyed properly.
   <alive> is a global variable instead of a static data member of
<physmutex> to allow <guard_osrtl_tasking_physmux> to access it.
<guard_osrtl_tasking_physmux> cannot be a friend of <physmutex> because that
would unnecessarily expose the existence of <guard_osrtl_tasking_physmux> to
the outside world.

----------------------------------------------------------------------------

static
int
connections   // total number of connections to physical mutexes
= 0;

   The physical mutexes use this global variable to count the total number
of connections of access mutexes to physical mutexes.  All connections of
all access mutexes to any physical mutex are counted.
<guard_osrtl_tasking_physmux> uses this variable to check if all access
mutexes have properly disconnected from the physical mutexes.
   <connections> is a global variable instead of a static data member of
<physmutex> to allow <guard_osrtl_tasking_physmux> to access it.
<guard_osrtl_tasking_physmux> cannot be a friend of <physmutex> because that
would unnecessarily expose the existence of <guard_osrtl_tasking_physmux> to
the outside world.

----------------------------------------------------------------------------

static
int
criticals   // total number of active critical sections
= 0;

   The physical mutexes use this global variable to count the total number
of processes that are inside a critical section (of course, this can be at
most one process per physical mutex).  <guard_osrtl_tasking_physmux> uses
this variable to check if all critical code sections have been left before
the module <physmux> is closed down.
   <criticals> is a global variable instead of a static data member of
<physmutex> to allow <guard_osrtl_tasking_physmux> to access it.
<guard_osrtl_tasking_physmux> cannot be a friend of <physmutex> because that
would unnecessarily expose the existence of <guard_osrtl_tasking_physmux> to
the outside world.

----------------------------------------------------------------------------

static
guard_osrtl_tasking_physmux   // guards this module
guard;

   Instantiates a global <guard_osrtl_tasking_physmux>.  This ensures that
the destructor of <guard_osrtl_tasking_physmux> is activated when the module
<physmux> is closed down.  Refer to the description of that destructor for
details on the checks that are performed when <physmux> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations, except for the
fact that the current implementation of the module <physmux> is valid only
for single-threaded environments.  <physmux> must be redesigned to allow it
to be used in a multi-threading environment.
   This is a point of improvement (**POI**).


----------------------------------------------------------------------------
---  End of file osrtl/windows/tasking/physmux/physmux.d .
----------------------------------------------------------------------------


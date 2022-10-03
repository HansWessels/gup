
----------------------------------------------------------------------------
---  File:     osrtl\windows\tasking\mutex\mutex.d
---  Contents: Mutex semaphores.
---  Module:   <osrtl><tasking><mutex>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl\windows\tasking\mutex\mutex.d  -  Mutex semaphores
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:05  erick
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
19970710
19970724
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><tasking><mutex>
----------------------------------------------------------------------------

   The module <osrtl><tasking><mutex> provides access mutexes.  Refer to
the documentation of the component <osrtl><tasking> for a description of
physical mutexes and access mutexes.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><tasking><mutex>
----------------------------------------------------------------------------

   To understand how to use the module <osrtl><tasking><mutex>, the user
must have a good understanding of the concept of access mutexes and the
underlying physical mutexes.  Refer to the documentation of the component
<osrtl><tasking> for a description of physical mutexes and access mutexes.

   To guard a critical section, the user must instantiate an access mutex,
<osmutex>.  This access mutex must then be connected to an underlying
physical mutex.  Once the access mutex has been connected to a physical
mutex, the user code uses <EnterSection> to safely enter a critical section,
and <LeaveSection> to leave the critical section.  The access mutex is
automatically disconnected from its physical mutex when the access mutex
is destroyed.
   It is not possible to re-connect an access mutex to a different physical
mutex.
   Summarising, the sequence of actions to guard critical code is:

      1. Instantiate an <osmutex>.
      2. Use osmutex::Connect to connect the <osmutex> to a physical mutex.
      3. Use osmutex::EnterSection to enter a critical section.
      4. Use osmutex::LeaveSection to leave a critical section.
      5. Destroy the <osmutex> (normally, this is done automatically by
         deleting it or by leaving the scope of the <osmutex>).


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><tasking><mutex> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <mutex> uses basic entities like <OSNULL>, <osbool>, and
         <osobject>.

      Component <osrtl><error>.
         <mutex> uses, among others, <osresult>.

      Module <osrtl><tasking><mutexid>.
         Because <mutex> uses physical mutexes, it must know about mutex
         IDs.

      Module <osrtl><tasking><physmux>.
         <mutex> depends on physical mutexes for all of its operations.

      Module <osrtl><tasking><mutex>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><tasking><mutex>
----------------------------------------------------------------------------

class osmutex:       // mutex semaphore
   public osobject   //    generic object
{
   //...
};  // osmutex

   The access mutex.  This access mutex must be connected to a physical
mutex in order to be useful.  Refer to the descriptions of the components of
<osmutex>, below, for an explanation of the workings of an access mutex.
Refer to the documentation of the component <osrtl><tasking> for a
description of physical mutexes and access mutexes.

----------------------------------------------------------------------------

osmutex::osmutex   // initialises an <osmutex>
();

   Assigns a name to the <osmutex> to facilitate debugging.  Initialises the
private data fields of the <osmutex>.

----------------------------------------------------------------------------

virtual
osmutex::~osmutex   // closes down an <osmutex>
();

   Closes down an <osmutex>.  Closing down is a non-operation if the
<osmutex> is not connected to a physical mutex.
   If the <osmutex> is connected to a physical mutex, the <osmutex> discon-
nects from the physical mutex.  If, after having disconnected, no <osmutex>
is connected to the physical mutex, the physical mutex is destroyed.  This
is done by searching it in the list of physical mutexes <mutexlist>,
removing it from that list, and deleting it.
   The operations of osmutex::~osmutex are guarded by placing them in a
critical section.  The physical mutex <access> serves as the guard.

----------------------------------------------------------------------------

virtual
osbool             // <osmutex> could be connected?
osmutex::Connect   // connects the <osmutex> to a mutex
(mutexid  id);     // I: ID of the mutex

   Connects the <osmutex> to a physical mutex.  The physical mutex is ident-
ified by an ID.
   It is not possible to connect an <osmutex> to a physical mutex more than
once; in other words, it is not possible to re-connect an <osmutex>, not
even to the same physical mutex.
   osmutex::Connect starts by checking if a physical mutex with the given ID
already exists.  The check is done by searching the list of physical mutexes
<mutexlist>.  If the physical mutex already exists, <mutex> will point to it
and the <osmutex> will connect to it.
   If a physical mutex with the given ID does not yet exist, it is created.
This is done by allocating a new physical mutex on the heap.  If there is
insufficient memory to allocate the physical mutex, osmutex::Connect fails.
If there is sufficient memory, the new physical mutex is inserted into the
list of physical mutexes <mutexlist>, immediately after the first element
<access> of that list.  The <osmutex> then connects to the newly created
physical mutex.
   The operations of osmutex::Connect are guarded by placing them in a
critical section.  The physical mutex <access> serves as the guard.

----------------------------------------------------------------------------

virtual
void
osmutex::EnterSection   // waits until safe to enter critical section
();

   Checks if the <osmutex> is connected to a physical mutex.  If so, uses
the method <EnterSection> of that physical mutex to enter the critical code
section.  If not (which is an error), osmutex::EnterSection does nothing.

----------------------------------------------------------------------------

virtual
void
osmutex::LeaveSection   // indicates critical section has been left
();

   Checks if the <osmutex> is connected to a physical mutex.  If so, uses
the method <LeaveSection> of that physical mutex to leave the critical code
section.  If not (which is an error), osmutex::LeaveSection does nothing.

----------------------------------------------------------------------------

osmutex::osmutex    // dummy private copy constructor
(osmutex const &)   //    prevents copying
{}

   If an <osmutex> would be copied from a source that is connected to a
physical mutex, the copy would have to be connected to the same physical
mutex.  Theoretically, connecting the copy to that (existing) physical mutex
might fail, even if it will not fail in the current implementation of
<mutex>.  Failing to connect would mean that the copy constructor would
fail; this is forbidden by the Coding Standard.  Therefore, using a copy
constructor is disabled.

----------------------------------------------------------------------------

osmutex const &       // dummy private assignment operator
osmutex::operator =   //    prevents assignment
(osmutex const &)
const
{ return *this; }

   If an <osmutex> would be copied from a source that is connected to a
physical mutex, the copy would have to be connected to the same physical
mutex.  Theoretically, connecting the copy to that (existing) physical mutex
might fail, even if it will not fail in the current implementation of
<mutex>.  Failing to connect would mean that the assignment operator would
fail.  It is not clear what the consequences of failing to assign would have
to be.  Therefore, using the assignment operator is disabled.

----------------------------------------------------------------------------

priv_mutex *           // actual mutex _before_ target mutex
osmutex::SearchMutex   // searches an existing actual mutex
(osmutexid  id)        // I: ID of the target actual mutex
const;

   Searches the list of physical mutexes <mutexlist> for a physical mutex
with the ID <id>.  Returns a pointer to the physical mutex _before_ the
physical mutex with ID <id> if the physical mutex with ID <id> is in the
list.  Returns static_cast<physmutex *>(OSNULL) if the list does not contain
a physical mutex with ID <id>.
   If the list does contain a physical mutex with ID <id>, there is always a
physical mutex before that physical mutex, because the static physical mutex
<access> is always the first physical mutex in the list.  The search for ID
<id> starts at the second physical mutex in the list, so the ID of <access>
will not clash with any user-defined ID.
   The caller of osmutex::SearchMutex must guard the call by using <access>.

----------------------------------------------------------------------------

physmutex *       // physical mutex to which <osmutex> is connected
osmutex::mutex;

   The physical mutex to which the <osmutex> is connected.  This is
static_cast<physmutex *)(OSNULL) as long as the <osmutex> is not connected
to a physical mutex.
   If the <osmutex> is connected to a physical mutex, that physical mutex is
guaranteed to be in the list of physical mutexes osmutex::mutexlist .

----------------------------------------------------------------------------

class guard_osrtl_tasking_mutex   // guards this module
{
   // ...
};  // guard_osrtl_tasking_mutex

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

guard_osrtl_tasking_mutex::~guard_osrtl_tasking_mutex   // performs exit
();                                                     //    checks on
                                                        //    the module

   This destructor checks the static global variables that are maintained by
the access mutexes for anomalies.  In debugging mode, it warns the developer
if an anomaly is found.  If it discovers that some physical mutexes have not
been destroyed, it destroys these physical mutexes.  In debugging mode, it
warns the developer about this fact.

----------------------------------------------------------------------------

static
physmutex                   // mutex to access mutex administration
access(MUTEX_MUTEXADMIN);

   The access mutexes use this global variable to guard their own blocks of
code by making these block of code critical sections.  <access> also serves
as the first element of the list of physical mutexes, <mutexlist>.  This
makes sure that <mutexlist> is never empty, which facilitates list
management.
   <access> is a global variable instead of a static data member of
<osmutex> because the global variable <mutexlist> must be able to access it.

----------------------------------------------------------------------------

static
physmutex *   // list of all physical mutexes
mutexlist
= &access;

   The access mutexes use this global variable to maintain a linked list of
physical mutexes.  <guard_osrtl_tasking_mutex> uses this variable to check
if all physical mutexes have been destroyed properly.
   <mutexlist> is a global variable instead of a static data member of
<osmutex> to allow <guard_osrtl_tasking_mutex> to access it.
<guard_osrtl_tasking_mutex> cannot be a friend of <osmutex> because that
would unnecessarily expose the existence of <guard_osrtl_tasking_physmux> to
the outside world.

----------------------------------------------------------------------------

static
int      // total number of existing access mutexes
alive
= 0;

   The access mutexes use this global variable to count the total number
of access mutexes that are alive.  <guard_osrtl_tasking_mutex> uses this
variable to check if all access mutexes have been destroyed properly.
   <alive> is a global variable instead of a static data member of
<osmutex> to allow <guard_osrtl_tasking_mutex> to access it.
<guard_osrtl_tasking_mutex> cannot be a friend of <osmutex> because that
would unnecessarily expose the existence of <guard_osrtl_tasking_mutex> to
the outside world.

----------------------------------------------------------------------------

static
guard_osrtl_tasking_mutex    // guards this module
guard;

   Instantiates a global <guard_osrtl_tasking_mutex>.  This ensures that
the destructor of <guard_osrtl_tasking_mutex> is activated when the module
<mutex> is closed down.  Refer to the description of that destructor for
details on the checks that are performed when <mutex> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl\windows\tasking\mutex\mutex.d .
----------------------------------------------------------------------------


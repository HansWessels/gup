
----------------------------------------------------------------------------
---  File:      osrtl\windows\tasking\tasking.d
---  Contents:  Task control.
---  Component: <osrtl><tasking>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl\windows\tasking\tasking.d  -  Task control
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:42:02  erick
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
 1.1.000  First issue of this component documentation file.
19970710
19970723
EAS
----------------------------------------------------------------------------
 1.1.001  <MUTEX_APPLICATION> has been added to chapter 3.  This
19970823  incorporates the changes that were made in revision 1.1.001 of
19970823  <osrtl><tasking><mutexid>.
EAS
----------------------------------------------------------------------------
 1.2.000  Identical to revision 1.1.001.  Formalised revision.
19970826
19970826
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the component <osrtl><tasking>
----------------------------------------------------------------------------

   The component <osrtl><tasking> provides services that are needed in
multi-tasking or multi-threading environments.
   <osrtl><tasking> uses the heap.  Yet, <osrtl><heap> is positioned above
<osrtl><tasking>.  This was felt to be the more generic approach: the
probability of <osrtl><heap> needed access to <osrtl><tasking> is higher
than the probability of <osrtl><tasking> needing access to <osrtl><heap>.
The consequence of this approach is that <osrtl><tasking> has implemented
its own version of the heap functions.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><tasking>
----------------------------------------------------------------------------

   To use <osrtl><tasking><mutexid>, the user must understand the concept of
access mutexes and the underlying, physical mutexes.
   The access mutexes, <osmutex>, are just a gateway that connects to a
physical mutex.  This is necessary to allow each task to have its own,
private access mutex.  The inter-task communication that is necessary inside
the functions of a mutex is performed by the underlying physical mutex; this
physical mutex is shared by the access mutexes of all involved tasks.
   Each physical mutex has an ID.  This ID must be supplied by the user; it
is important to use different IDs when different physical mutexes are to be
used.
   The user, though, does not deal at all with the physical mutexes.  He
just instantiates an <osmutex>, telling it to which physical mutex to
connect by passing the ID of the physical mutex.  The <osmutex> checks if
a physical mutex with the given ID already exists.  If so, the <osmutex>
connects to that physical mutex.  If not, the <osmutex> tries to create a
new physical mutex and set the ID of that physical mutex to the given ID.
If the new physical mutex can be created, the <osmutex> connects to it; if
there was not enough memory to create a new physical mutex, the <osmutex>
indicates failure to its caller.
   This means that a user must perform the following steps:

      1. Instantiate an <osmutex>.
      2. Connect the <osmutex> to a physical mutex.  The physical mutex is
         identified by a user-supplied ID that must be unique throughout
         all threads and/or processes of the entire program.
      3. Use osmutex::EnterSection to enter a critical code section.
      4. Use osmutex::LeaveSection to leave a critical code section.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the component <osrtl><tasking>
----------------------------------------------------------------------------

   Because <osrtl><tasking> is nothing but a higher-level view of the
modules <osrtl><tasking><mutexid> and <osrtl><tasking><mutex> (not
of <osrtl><tasking><physmux> - that module is used internally by
<osrtl><tasking>), refer to the documentation files for these modules
for a detailed explanation of the component <osrtl><tasking>.
   To give an overview of the component <osrtl><tasking>, here follows a
list of all items provided by <osrtl><tasking><mutex> that are exported to
the rest of the program.

Type:     mutexid
Use:      ID for a physical mutex to which an <osmutex> must connect.

Constant: MUTEX_APPLICATION
Use:      A constant of type <mutexid>.  It contains the lowest possible
          value for a mutex ID that may be used by the application.  Mutex
          IDs that are less than <MUTEX_APPLICATION> are reserved for
          internal use by <osrtl>.

Type:     osmutex
Use:      Access mutex.  Must be connected to a physical mutex.  Must be
          used to guard critical code sections.

Function: osmutex::osmutex()
Use:      Initialises an access mutex.

Function: osmutex::~osmutex
Use:      Closes down an access mutex.  If necessary, disconnects the access
          mutex from the physical mutex to which it is connected.

Function: osmutex::Connect
Use:      Connects an access mutex to a physical mutex.

Function: osmutex::EnterSection
Use:      Waits until it is safe to enter a critical code section.

Function: osmutex::LeaveSection
Use:      Signals that the code has left a critical code section.


----------------------------------------------------------------------------
---  End of file osrtl\windows\tasking\tasking.d .
----------------------------------------------------------------------------


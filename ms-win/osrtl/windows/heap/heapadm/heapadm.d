
----------------------------------------------------------------------------
---  File:     osrtl/windows/heap/heapadm/heapadm.d
---  Contents: Heap administration.
---  Module:   <osrtl><heap><heapadm>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/heap/heapadm/heapadm.d  -  Heap administration
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:37  erick
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
19970717
19970806
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><heap><heapadm>
----------------------------------------------------------------------------

   The module <osrtl><heap><heapadm> maintains the administration that is
needed to implement a safe (checked) heap.  It is used by <safeheap>, which
implements such a safe heap.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><heap><heapadm>
----------------------------------------------------------------------------

   <safeheap> must instantiate a <heapadministrator> whenever it allocates
or de-allocates data.  It must use <MarkAllocation> to mark each successful
allocation, and it must use <MarkDeallocation> to mark each successful de-
allocation.  It may use <Allocated> to check if a given block of memory is
(still) allocated.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><heap><heapadm> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <heapadm> uses basic types, such as <osbool> and <osobject>.

      Component <osrtl><error>.
         <heapadm> uses entities for error handling, such as <osresult>.

      Component <osrtl><tasking>.
         <heapadm> uses mutexes to guard its operations.

      Module <osrtl><heap><heap>.
         <heapadm> uses the basic (unchecked) heap to allocate and de-
         allocate elements of its internal data structures.

      Module <osrtl><heap><heapblck>.
         <heapadm> uses <osheapblock>s to keep track of the blocks of
         memory that are currently allocated.

      Module <osrtl><heap><heapadm>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><heap><heapadm>
----------------------------------------------------------------------------

class heapadministrator:   // heap administrator
   public osobject         //    generic object
{
   //...
};  // heapadministrator

   Performs the administration of allocations and de-allocations.  This
administration is intended for use by <safeheap>, which uses it to implement
a safe (checked) heap.
   Refer to the descriptions of the components of <heapadministrator> for an
explanation of the workings of this class.

----------------------------------------------------------------------------

inline
heapadministrator::heapadministrator   // initialises a <heapadministrator>
();

   Initialises a <heapadministrator> by settings its name.  This facilitates
debugging.

----------------------------------------------------------------------------

osbool                              // block could be marked as allocated?
heapadministrator::MarkAllocation   // marks a block as being allocated
(void const *     block,            // I: block to mark
 wchar_t const *  descr);           // I: description of the block

   Marks a block of memory as having been allocated on the heap.  This
is done by allocating a <heapblock> using the basic, unchecked heap
<osbasicheap>, and using this <heapblock> to store the address <block> and
the description <descr> of the allocated block.  It then inserts the new
<heapblock> in the list of <heapblock>s <allocated>.

----------------------------------------------------------------------------

osbool                                // block could be marked?
heapadministrator::MarkDeallocation   // marks a block as being de-allocated
(void const *  block);                // I: block to mark

   Marks a block of memory as having been de-allocated from the heap.  This
is done by using <Allocated> to check if the block <block> is actually
allocated.  If it is, the <heapblock> about <block> is removed from the list
of <heapblock>s <allocated>, and de-allocated using the basic, unchecked
heap <osbasicheap>.

----------------------------------------------------------------------------

osbool                         // block is allocated?
heapadministrator::Allocated   // checks if a block is allocated
(void const *  block);         // I: block to check

osbool                         // block is allocated?
heapadministrator::Allocated   // checks if a block is allocated
(void const *     block,       // I: block to check
 osheapblock * *  prev);       // O: info block before info block of <block>

   Checks if the block of memory <block> is (still) allocated on the heap.
The first version of this function is intended for use by <safeheap>; it
enters a critical section and calls the second version, with <prev> set to
static_cast<osheapblock * *>(OSNULL) .  The second version of this function
assumes that the critical section has been entered by its caller; it con-
tains the actual functionality.  The rest of this explanation will focus on
the second version.
   <Allocated> scans the list of allocated blocks <allocated> for a
<heapblock> about the target block of memory <block>.  If it does not find
such a <heapblock>, it returns <OSFALSE> to indicate that <block> is not
allocated.  If it does find a <heapblock> about <block>, and if <prev> is
not static_cast<osheapblock * *>(OSNULL) , it stores the <heapblock> before
the <heapblock> about <block> in *prev .  It then returns <OSTRUE> to indic-
ate that <block> is allocated.

----------------------------------------------------------------------------

class guard_osrtl_heap_heapadm   // guards this module
{
public:
   //...
};  // guard_osrtl_heap_heapadm

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

guard_osrtl_heap_heapadm::~guard_osrtl_heap_heapadm  // performs exit checks
();                                                  //    on the module

   This destructor scans the list of allocated blocks <allocated>.  For each
<heapblock> that is still present in that list, it performs the following
actions:

      1. It removes the <heapblock> from the list of allocated blocks
         <allocated>.
      2. In debugging mode, it uses the description of the allocated block
         (as set by heapadministrator::MarkAllocation ) to tell the
         developer that a block of memory has not been de-allocated.
      3. It de-allocates the <heapblock>, using the basic (unchecked) heap
         <osbasicheap>.

Ideally, guard_osrtl_heap_heapadm::~guard_osrtl_heap_heapadm should also
de-allocate the allocated block of memory.  However, lacking information
about the actual type of the allocated block, this is not possible.

----------------------------------------------------------------------------

osheapblock *
allocated                               // list of allocated blocks
= static_cast<osheapblock *>(OSNULL);

   This global static variable contains the head of the list of allocated
blocks of memory.  It is a global variable instead of a static data member
of <heapadministrator> to allow <guard_osrtl_heap_heapadm> to access it.

----------------------------------------------------------------------------

static
guard_osrtl_heap_heapadm                // guards this module
guard;

   Instantiates a global <guard_osrtl_heap_heapadm>.  This ensures that the
destructor of <guard_osrtl_heap_heapadm> is activated when the module
<heapadm> is closed down.  Refer to the description of that destructor for
details on the actions that are performed when <heapadm> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/heap/heapadm/heapadm.d .
----------------------------------------------------------------------------



----------------------------------------------------------------------------
---  File:     osrtl/windows/heap/heap/heap.d
---  Contents: Heap management.
---  Module:   <osrtl><heap><heap>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/heap/heap/heap.d  -  Heap management
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:33  erick
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
19970708
19970818
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><heap><heap>
----------------------------------------------------------------------------

   The module <osrtl><heap><heap> supplies access to the heap.  It allows
user code to allocate and de-allocate data objects.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><heap><heap>
----------------------------------------------------------------------------

   A function that wishes to allocate objects on the heap, or to de-allocate
objects from the heap, must instantiate a heap of the appropriate type.
The function can then use the member <HeapAllocate> of this heap object to
perform the allocation.  If an object has already been allocated on the
heap, a function can use the member <HeapDeallocate> of the heap object to
perform the de-allocation.  De-allocation of an allocated object may be done
from a different heap instance than the instance that allocated the object.
   The following example instantiates a heap for type <mytype>, allocates an
object <myobj> and an array of objects <myarr>, and finally de-allocates
<myobj> and <myarr>.  To enhance the example, <myobj> is de-allocated from
a different heap object than the one that allocated <myobj>.

      {
         osbasicheap<mytype>  heap;    // heap for objects of type <mytype>
         mytype *             myobj;   // a single object
         mytype *             myarr;   // an array of 5 objects

         // Allocate <myobj> and <myarr>.
         myobj = heap.HeapAllocate();      // allocate a single object;
                                           //    identical to
                                           //    myheap.HeapAllocate(1);
         myarr = heap.HeapAllocate(5UL);   // allocate an array of 5 objects

         // Perform some useful actions.

         // De-allocate <myobj> from a different heap.
         {
            osbasicheap<mytype> anotherheap;

            anotherheap.HeapDeallocate(myobj);
         }

         // De-allocate <myarr>.
         heap.HeapDeallocate(myarr);
      }

Note that all error checking has been left out of this example to keep it
easily understandable.
   As already briefly indicated, it is allowed to instantiate a heap for a
particular type, and to use this heap for allocation only, that is, not de-
allocating the allocated objects before the end of the life of the heap
object.  It is also allowed to instantiate a heap for a particular type, and
to use this heap for de-allocation only, provided that the objects the are
de-allocated have been allocated earlier in the program using a (different)
heap of the same type.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><heap><heap> depends on the following units, components
and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>
         <heap> uses basic entities, such as <OSNULL>.

      Component <osrtl><error>
         <heap> returns <osresult>s.

      Module <osrtl><heap><heap>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><heap><heap>
----------------------------------------------------------------------------

template <class datatype>
class osbasicheap:                // basic heap manager
   public priv_osbasicheapadmin   //    administrator
{
   //...
};  // osbasicheap<datatype>

   A basic heap manager for objects of type <datatype>.  Allocates and de-
allocates objects or arrays of objects.  Refer to the descriptions of the
components of osbasicheap<datatype>, below, for an explanation of the
workings of the heap manager.

----------------------------------------------------------------------------

template <class datatype>
inline
osbasicheap<datatype>::osbasicheap   // initialises an <osbasicheap>
();

   Sets the name of the heap object to "<osbasicheap>" to facilitate
debugging.

----------------------------------------------------------------------------

template <class datatype>
datatype *                            // allocated data, or <OSNULL>
osbasicheap<datatype>::HeapAllocate   // allocates data on the heap
(unsigned long  n = 1UL);             // I: #data elements to allocate

   Allocates an array of <n> elements of type <datatype> on the heap.  By
default, <n> is one, which results in a single element being allocated.
   If there is sufficient free memory on the heap to perform the allocation,
the return value points to the (first) element that was allocated.  If there
was not sufficient memory, the return value is
static_cast<datatype *>(OSNULL) .
   Use osbasicheap<datatype>::HeapDeallocate (with the same <datatype>) to
de-allocate the memory that was allocated by
osbasicheap<datatype>::HeapAllocate .

----------------------------------------------------------------------------

template <class datatype>
osresult                                // result code
osbasicheap<datatype>::HeapDeallocate   // de-allocates data from the heap
(datatype *  data)                      // I: data to de-allocate

   De-allocates a single element of type <datatype>, or an array of elements
of type <datatype>, that was allocated earlier using
osbasicheap<datatype>::HeapAllocate .  It is allowed to "de-allocate" a null
pointer; in that case, osbasicheap<datatype>::HeapDeallocate will do nothing
but simply return <OSRESULT_OK>.
   It is not allowed to de-allocate

      - an element that is not of the appropriate type
      - an element that was not allocated on the heap using
        osheap<datatype>::HeapAllocate using the same <datatype>
      - an element that has already been de-allocated

In all these cases, the result of such a forbidden de-allocation is
undefined.

----------------------------------------------------------------------------

class priv_osbasicheapadmin:   // administrator for the basic heap manager
   public osobject             //    generic object
{
   //...
};  // priv_osbasicheapadmin

   Heap administrator.  This class counts the total number of objects that
have been allocated on the heap and that have not yet been de-allocated.  It
is active both in debugging mode and in release mode, even though in release
mode nothing is done with the count.  The overhead, both in time and in code
size, of maintaining the count is negligible.
   <priv_osbasicheapadmin> has been made a separate class instead of making
its functionality an integrated part of <osbasicheap>; this has been done
because it is safer: because it makes sure that only a single instance of
the static variable <allocations> exists.  If <allocations> would have been
a static data member of osbasicheap<datatype>, each instance of the template
might end up having its own copy of <allocations>.

----------------------------------------------------------------------------

inline
void
priv_osbasicheapadmin::MarkAllocation   // marks a successful allocation
();

   Marks one successful allocation by incrementing the count of allocated
objects.

----------------------------------------------------------------------------

inline
void
priv_osbasicheapadmin::MarkDeallocation  // marks a successful de-allocation
();

   Marks one successful de-allocation by decrementing the count of allocated
objects.

----------------------------------------------------------------------------

inline
unsigned long                        // total number of allocated blocks
priv_osbasicheapadmin::Allocations   // obtains total #allocated blocks
();

   Returns the current count of allocated objects.

----------------------------------------------------------------------------

static
unsigned long                         // total number of allocated blocks
priv_osbasicheapadmin::allocations;

   The count of allocated objects that have not yet been de-allocated.

----------------------------------------------------------------------------

class guard_osrtl_heap_heap:      // guards this module
   private priv_osbasicheapadmin  //    administrator for basic heap manager
{
public:
   // ...
};  // guard_osrtl_heap_heap

   This data type guards this module.  Refer to the destructor of this class
for a description of the guarding process.
   The module is guarded through a single static global variable of this
class, <guard>.  <guard> causes the destructor of this class to be executed
at program termination.

----------------------------------------------------------------------------

virtual
guard_osrtl_heap_heap::~guard_osrtl_heap_heap   // performs exit checks
()                                              //    on the module

   This destructor checks the count of allocated objects that have not yet
been de-allocated.  In debugging mode, it warns the developer if this count
is not zero.

----------------------------------------------------------------------------

static
guard_osrtl_heap_heap   // guards this module
guard;

   Instantiates a global <guard_osrtl_heap_heap>.  This ensures that the
destructor of <guard_osrtl_heap_heap> is activated when the module <heap> is
closed down.  Refer to the description of that destructor for details on the
checks that are performed when <heap> is closed down.
   <guard> is the last static global variable to ensure that all other
static global variables are still valid when its destructor is activated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.

----------------------------------------------------------------------------
---  End of file osrtl/windows/heap/heap/heap.d .
----------------------------------------------------------------------------



----------------------------------------------------------------------------
---  File:     osrtl/windows/heap/safeheap/safeheap.d
---  Contents: Checked heap.
---  Module:   <osrtl><heap><safeheap>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/heap/safeheap/safeheap.d  -  Checked heap
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:42  erick
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
19970818
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><heap><safeheap>
----------------------------------------------------------------------------

   The module <osrtl><heap><safeheap> implements a safe heap.  A safe heap
is a heap that is checked for the consistency of its allocations and de-
allocations.  It does not allow de-allocation of an object that is not (or
no longer) allocated, and, when the program terminates, it de-allocates all
blocks that were allocated but not de-allocated.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><heap><safeheap>
----------------------------------------------------------------------------

   User code must instantiate an <osheap> of the appropriate data type.  The
user code can then use <Allocate> to allocate memory, and de-allocate to de-
allocate memory.
   It is allowed to instantiate an <osheap> to allocate data, and then to
destroy that <osheap> without de-allocating the allocated data first.  Later
in the program, a new <osheap> of the same data type can be instantiated to
de-allocate the allocated data.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><heap><safeheap> does not depend on any other part of
the program.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><heap><safeheap>
----------------------------------------------------------------------------

template <class datatype>
class osheap:                      // checked heap
   private osbasicheap<datatype>   //    basic heap
{
   //...
};  // osheap<datatype>

   This class implements a checked heap that does not allow de-allocation of
objects that are not allocated, and that warns the developer about memory
leaks (in debugging mode only).  Refer to the components of this class for
a decription of its functionality.

----------------------------------------------------------------------------

template <class datatype>
osheap<datatype>::osheap    // initialises an <osheap>
();

   Initialises the <osheap> by setting its name to "<osheap>".  This
facilitates debugging.

----------------------------------------------------------------------------

template <class datatype>
inline
datatype *                      // allocated data
osheap<datatype>::Allocate      // allocates data on the heap
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,     // I: description of the data elements
    unsigned long    n = 1UL);  // I: number of data elements to allocate
#else
   (wchar_t const *  ,          // -: description of the data elements
    unsigned long    n = 1UL);  // I: number of data elements to allocate
#endif

   Allocates an array of <n> elements of type <datatype>, and returns a
pointer to the first of the allocated elements if successful.  Returns
static_cast<datatype *>(OSNULL) if there was not enough memory on the heap
to do the allocation.  In debugging mode, the allocated elements are given
a description <descr>.  This description is used to inform the developer
about problems on the heap, such as not de-allocating allocated elements.
   <Allocate> is an <inline> function that only calls <SafeAllocate>.  This
approach allows the compiler to optimise away the string literal <descr> in
the caller when the program is compiled in release mode.
   Refer to the description of <SafeAllocate> for an explanation of the
workings of the allocation process.

----------------------------------------------------------------------------

template <class datatype>
osresult                       // result code
osheap<datatype>::Deallocate   // de-allocates data from the heap
(datatype *  data);            // I: data to de-allocate

   De-allocates the element <data> or the array of elements <data>.  If
<data> is static_cast<datatype *>(OSNULL) , this function does nothing but
return <OSRESULT_OK>.
   If <data> is not static_cast<datatype *>(OSNULL) , <Deallocate> uses a
<heapadministrator> to check if <data> is actually allocated.  If it is, it
de-allocates <data> using the basic, unchecked heap <osbasicheap>.  It then
uses the <heapadministrator> to mark <data> as having been de-allocated.

----------------------------------------------------------------------------

template <class datatype>
datatype *                       // allocated data
osheap<datatype>::SafeAllocate   // allocates data on the heap
(wchar_t const *  descr,         // I: description of the data elements
 unsigned long    n = 1UL);      // I: number of data elements to allocate

   <SafeAllocate> is called by <Allocate> as described under <Allocate>,
above.  It performs the actions that are described under <Allocate>.
   It first uses the basic, unchecked heap <osbasicheap> to allocate the
data.  It then uses a <heapadministrator> to mark the fact that <data> has
been allocated.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/heap/safeheap/safeheap.d .
----------------------------------------------------------------------------


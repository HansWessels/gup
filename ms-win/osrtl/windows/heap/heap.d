
----------------------------------------------------------------------------
---  File:      osrtl/windows/heap/heap.d
---  Contents:  Heap.
---  Component: <osrtl><heap>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/heap/heap.d  -  Heap
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:30  erick
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
19970708
19970724
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the component <osrtl><heap>
----------------------------------------------------------------------------

   The component <osrtl><heap> provides access to the heap, or free store.
It allows objects to be allocated and de-allocated in a way that allows
detection of illegal operations (both in debugging mode and in release mode)
and detection of memory leaks (in debugging mode only).


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><heap>
----------------------------------------------------------------------------

   The component <osrtl><heap> consists of the modules <heap>, <heapblck>,
<heapadm>, and <safeheap>.  Though, looking from a language perspective, it
exports all four modules, from a user perspective it exports <safeheap>
only.
   User code must instantiate an <osheap> of the appropriate data type.  The
user code can then use <Allocate> to allocate memory, and de-allocate to de-
allocate memory.
   It is allowed to instantiate an <osheap> to allocate data, and then to
destroy that <osheap> without de-allocating the allocated data first.  Later
in the program, a new <osheap> of the same data type can be instantiated to
de-allocate the allocated data.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the component <osrtl><heap>
----------------------------------------------------------------------------

   Because, from a user perspective, <osrtl><heap> is nothing but a higher-
level view of the module <osrtl><heap><safeheap>, refer to the documentation
file for that module for a detailed explanation of the component
<osrtl><heap>.
   To give an overview of the component <osrtl><heap>, here follows a
list of all items provided by <osrtl><heap><safeheap> that are exported to
the rest of the program.

Class:    osheap<datatype>
Use:      Type safe, checked heap manager for objects of type <datatype>.

Function: osheap<datatype>::Allocate
Use:      Allocates an element of type <datatype>, or an array of such
          elements, on the heap.

Function: osheap<datatype>::Deallocate
Use:      De-allocates an element of type <datatype>, or an array of such
          elements, from the heap.


----------------------------------------------------------------------------
---  End of file osrtl/windows/heap/heap.d .
----------------------------------------------------------------------------


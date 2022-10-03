
----------------------------------------------------------------------------
---  File:     osrtl/windows/heap/heapblck/heapblck.d
---  Contents: Heap blocks.
---  Module:   <osrtl><heap><heapblck>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/heap/heapblck/heapblck.d  -  Heap blocks
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:39  erick
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
Chapter 1  -  Purpose of the module <osrtl><heap><heapblck>
----------------------------------------------------------------------------

   <osrtl><heap><heapblck> implements a linked list that contains addresses
of memory blocks.  This module is a support module for the module
<osrtl><heap><heapadm>.  It allows <heapadm> to perform the administration
that is necessary to implement a safe (checked) heap.  Refer to the
documentation of <osrtl><heap><heapadm> and <osrtl><heap><safeheap> for a
description of this administration.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><heap><heapblck>
----------------------------------------------------------------------------

   The module <osrtl><heap><heapblck> is used by instantiating objects of
type <osheapblock> and directly accessing their data fields to implements
a linked list.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><heap><heapblck> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <heapblck> uses basics types, like <osobject>.

      Component <osrtl><error>.
         <heapblck> uses error-handling functions, like
         <OsDebugErrorMessage>.

      Module <osrtl><heap><heapblck>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><heap><heapblck>
----------------------------------------------------------------------------

----------------------------------------------------------------------------

class osheapblock:   // data on an allocated block
   public osobject   //    generic object
{
   //...
};  // osheapblock

   Data on an allocated object on the heap.  Refer to the descriptions of
the components of <osheapblock>, below, for an explanation of the workings
of this class.

----------------------------------------------------------------------------

osheapblock::osheapblock   // initialises an <osheapblock>
();

   Initialises an <osheapblock> by setting the name of the object, and by
initialising its data members to <OSNULL>.

----------------------------------------------------------------------------

void const *          // address of the allocated block
osheapblock::block;

   The address of the allocated block of memory.  It is the responsibility
of <heapadm> to maintain this data member.

----------------------------------------------------------------------------

osheapblock *        // next <osheapblock> in the list
osheapblock::next;

   The next <osheapblock> in the linked list of <osheapblock>s.  It is the
responsibility of <heapadm> to maintain this data member.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/heap/heapblck/heapblck.d .
----------------------------------------------------------------------------



----------------------------------------------------------------------------
File:   confctrl/pack.cc
Author: E.A. Silkens
----------------------------------------------------------------------------


Introduction
------------

   This file is the configuration-control file for the Windows shell for
GNU Pack.  It covers all human-generated files of the entire shell.  For
requirements on this configuration-control file, refer to section 30 of
the Coding Standard for this project, doc/coding.d and doc/coding.gen .


RCS information
---------------

@(#) confctrl/pack.cc  -  Configuration-control file

$RCSFile:$
$Author: erick $
$Revision: 84 $
$Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
$Log$
Revision 1.1  1997/12/30 13:40:42  erick
First version of the MS-Windows shell (currently only osrtl).



Configuration-control information
---------------------------------

Project:     Windows shell for GNU Pack
Author:      E.A. Silkens
Revision:    1.2     
Date:        19970826
Elements:    Document     doc/coding.gen
             Document     doc/coding.d
             Settings     global/settings
             Unit         <osrtl>

Document:    doc/coding.gen  -  Generic Coding Standard, EPG
Author:      E.A. Silkens
Revision:    1.1
Date:        19970821
Elements:    -

Document:    doc/coding.d  -  Coding Standard for GNU Pack
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    -

Settings:    global/settings  -  Global settings for GNU Pack
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    Document     global/settings.d
             Header file  global/settings.h

Document:    global/settings.d  -  Global settings for GNU Pack
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    -

Header file: global/settings.h  -  Global settings for GNU Pack
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    -

Unit:        osrtl  -  NIL between program and (OS and RTL)
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/osrtl.d
             Header file  osrtl/osrtl.h
             Unit         <osrtl> for PC Windows

Document:    osrtl/osrtl.d  -  NIL between program and (OS and RTL)
Author:      E.A. Silkens
Revision:    1.1
Date:        19970825
Elements:    -

Header file: osrtl/osrtl.h  -  NIL between program and (OS and RTL)
Author:      E.A. Silkens
Revision:    1.1
Date:        19970825
Elements:    -

Unit:        <osrtl> for PC Windows  -  NIL between program and (OS and RTL)
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/windows/osrtl.d
             Header file  osrtl/windows/osrtl.h
             Settings     osrtl/windows/settings
             Component    <osrtl><basics>
             Component    <osrtl><error>
             Component    <osrtl><hardware>
             Component    <osrtl><startup>
             Component    <osrtl><tasking>
             Component    <osrtl><heap>
             Component    <osrtl><text>
             Component    <osrtl><sound>
             Component    <osrtl><scrnobj>
             Component    <osrtl><drawing>
             Component    <osrtl><window>

Document:    osrtl/windows/osrtl.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970825
Elements:    -

Header file: osrtl/windows/osrtl.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970825
Elements:    -

Settings:    osrtl/windows/settings  -  Global settings for unit <osrtl>
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    Document     osrtl/windows/settings.d
             Header file  osrtl/windows/settings.h

Document:    osrtl/windows/settings.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    -

Header file: osrtl/windows/settings.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    -

Component:   <osrtl><basics>  -  Basic definitions
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    Document     osrtl/windows/basics/basics.d
             Header file  osrtl/windows/basics/basics.h
             Module       <osrtl><basics><constant>
             Module       <osrtl><basics><type>
             Module       <osrtl><basics><convert>
             Module       <osrtl><basics><object>

Document:    osrtl/windows/basics/basics.d
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    -

Header file: osrtl/windows/basics/basics.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970714
Elements:    -

Module:      <osrtl><basics><constant>  -  Constants
Author:      E.A. Silkens
Revision:    1.1
Date:        19970723
Elements:    Document     osrtl/windows/basics/constant/constant.d
             Header file  osrtl/windows/basics/constant/constant.h

Document:    osrtl/windows/basics/constant/constant.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970723
Elements:    -

Header file: osrtl/windows/basics/constant/constant.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970716
Elements:    -

Module:      <osrtl><basics><type>  -  Types
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    Document     osrtl/windows/basics/type/type.d
             Header file  osrtl/windows/basics/type/type.h

Document:    osrtl/windows/basics/type/type.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    -

Header file: osrtl/windows/basics/type/type.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    -

Module:      <osrtl><basics><convert>  -  Type conversions
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    Document     osrtl/windows/basics/convert/convert.d
             Header file  osrtl/windows/basics/convert/convert.h
             Source file  osrtl/windows/basics/convert/convert.cpp

Document:    osrtl/windows/basics/convert/convert.d
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    -

Header file: osrtl/windows/basics/convert/convert.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    -

Source file: osrtl/windows/basics/convert/convert.cpp
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    Function     RoundFloatToIntegral
             Function     RoundFloatToIntegral
             Function     CharConvert
             Function     CharConvert
             Function     StringConvert
             Function     StringConvert

Module:      <osrtl><basics><object>  -  Generic object
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    Document     osrtl/windows/basics/object/object.d
             Header file  osrtl/windows/basics/object/object.h
             Source file  osrtl/windows/basics/object/object.cpp

Document:    osrtl/windows/basics/object/object.d
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    -

Header file: osrtl/windows/basics/object/object.h
Author:      E.A. Silkens
Revision:    1.2
Date:        19970825
Elements:    Function     osobject::osobject
             Function     osobject::SetName
             Function     osobject::ObjectName

Source file: osrtl/windows/basics/object/object.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     osobject::~osobject
             Function     osobject::DefineName
             Function     osobject::RetrieveName
             Function     osobject::KillName

Component:   <osrtl><error>  -  Error handling
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/windows/error/error.d
             Header file  osrtl/windows/error/error.h
             Module       <osrtl><error><rescode>
             Module       <osrtl><error><errmsg>

Document:    osrtl/windows/error/error.d
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    -

Header file: osrtl/windows/error/error.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970717
Elements:    -

Module:      <osrtl><error><rescode>  -  Result codes
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/windows/error/rescode/rescode.d
             Header file  osrtl/windows/error/rescode/rescode.h

Document:    osrtl/windows/error/rescode/rescode.d
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    -

Header file: osrtl/windows/error/rescode/rescode.h
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    -

Module:      <osrtl><error><errmsg>  -  Error messages
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    Document     osrtl/windows/error/errmsg/errmsg.d
             Header file  osrtl/windows/error/errmsg/errmsg.h
             Source file  osrtl/windows/error/errmsg/errmsg.cpp

Document:    osrtl/windows/error/errmsg/errmsg.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    -

Header file: osrtl/windows/error/errmsg/errmsg.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    Function:     OsErrorMessage
             Function:     OsErrorMessage
             Function:     OsDebugErrorMessage
             Function:     OsDebugErrorMessage
             Function:     OsDebugErrorMessage

Source file: osrtl/windows/error/errmsg/errmsg.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    Function     OsErrorMessage
             Function     AddMessage
             Function     Convert

Component:   <osrtl><hardware>  -  Hardware characteristics
Author:      E.A. Silkens
Revision:    1.1
Date:        19970801
Elements:    Document     osrtl/windows/hardware/hardware.d
             Header file  osrtl/windows/hardware/hardware.h
             Module       <osrtl><hardware><screen>
             Module       <osrtl><hardware><mouse>

Document:    osrtl/windows/hardware/hardware.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970801
Elements:    -

Header file: osrtl/windows/hardware/hardware.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970801
Elements:    -

Module:      <osrtl><hardware><screen>  -  Screen characteristics
Author:      E.A. Silkens
Revision:    1.1
Date:        19970801
Elements:    Document     osrtl/windows/hardware/screen/screen.d
             Header file  osrtl/windows/hardware/screen/screen.h
             Source file  osrtl/windows/hardware/screen/screen.cpp

Document:    osrtl/windows/hardware/screen/screen.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    -

Header file: osrtl/windows/hardware/screen/screen.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970801
Elements:    -

Source file: osrtl/windows/hardware/screen/screen.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970801
Elements:    Function     ScreenWidthPixels
             Function     ScreenHeightPixels

Module:      <osrtl><hardware><mouse>  -  Mouse hardware
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    Document     osrtl/windows/hardware/mouse/mouse.d
             Header file  osrtl/windows/hardware/mouse/mouse.h
             Source file  osrtl/windows/hardware/mouse/mouse.cpp

Document:    osrtl/windows/hardware/mouse/mouse.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    -

Header file: osrtl/windows/hardware/mouse/mouse.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    -

Source file: osrtl/windows/hardware/mouse/mouse.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    Function     MouseButtons

Component:   <osrtl><startup>  -  Start-up
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    Document     osrtl/windows/startup/startup.d
             Header file  osrtl/windows/startup/startup.h
             Module       <osrtl><startup><startup>

Document:    osrtl/windows/startup/startup.d
Author:      E.A. Silkens
Revision:    1/1
Date:        19970723
Elements:    -

Header file: osrtl/windows/startup/startup.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970723
Elements:    -

Module:      <osrtl><startup><startup>  -  Program start-up
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    Document     osrtl/windows/startup/startup/startup.d
             Header file  osrtl/windows/startup/startup/startup.h
             Source file  osrtl/windows/startup/startup/startup.cpp

Document:    osrtl/windows/startup/startup/startup.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    -

Header file: osrtl/windows/startup/startup/startup.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    -

Source file: osrtl/windows/startup/startup/startup.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    Function     WinMain
             Function     ProgramName
             Function     ProgramInstance
             Function     WindowExists

Component:   <osrtl><tasking>  -  Task control
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/windows/tasking/tasking.d
             Header file  osrtl/windows/tasking/tasking.h
             Module       <osrtl><tasking><mutexid>
             Module       <osrtl><tasking><physmux>
             Module       <osrtl><tasking><mutex>

Document:    osrtl/windows/tasking/tasking.d
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    -

Header file: osrtl/windows/tasking/tasking.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970716
Elements:    -

Module:      <osrtl><tasking><mutexid>  -  Mutex IDs
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/windows/tasking/mutexid/mutexid.d
             Header file  osrtl/windows/tasking/mutexid/mutexid.h

Document:    osrtl/windows/tasking/mutexid/mutexid.d
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    -

Header file: osrtl/windows/tasking/mutexid/mutexid.h
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    -

Module:      <osrtl><tasking><physmux>  -  Physical mutexes
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    Document     osrtl/windows/tasking/physmux/physmux.d
             Header file  osrtl/windows/tasking/physmux/physmux.h
             Source file  osrtl/windows/tasking/physmux/physmux.cpp

Document:    osrtl/windows/tasking/physmux/physmux.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970723
Elements:    -

Header file: osrtl/windows/tasking/physmux/physmux.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    Function     physmutex::Id
             Function     physmutex::Connected
             Function     physmutex::SetNextElement
             Function     physmutex::NextElement

Source file: osrtl/windows/tasking/physmux/physmux.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    Function     physmutex::physmutex
             Function     physmutex::~physmutex
             Function     physmutex::Connect
             Function     physmutex::Disconnect
             Function     physmutex::EnterSection
             Function     physmutex::LeaveSection
             Function     guard_osrtl_tasking_physmux::
                             ~guard_osrtl_tasking_physmux

Module:      <osrtl><tasking><mutex>  -  Mutex semaphores
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    Document     osrtl/windows/tasking/mutex/mutex.d
             Header file  osrtl/windows/tasking/mutex/mutex.h
             Source file  osrtl/windows/tasking/mutex/mutex.cpp

Document:    osrtl/windows/tasking/mutex/mutex.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    -

Header file: osrtl/windows/tasking/mutex/mutex.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970719
Elements:    -

Source file: osrtl/windows/tasking/mutex/mutex.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970727
Elements:    Function     osmutex::osmutex
             Function     osmutex::~osmutex
             Function     osmutex::Connect
             Function     osmutex::EnterSection
             Function     osmutex::LeaveSection
             Function     osmutex::SearchMutex
             Function     guard_osrtl_tasking_mutex::
                             ~guard_osrtl_tasking_mutex

Component:   <osrtl><heap>  -  Heap
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/heap/heap.d
             Header file  osrtl/windows/heap/heap.h
             Module       <osrtl><heap><heap>
             Module       <osrtl><heap><heapblck>
             Module       <osrtl><heap><heapadm>
             Module       <osrtl><heap><safeheap>

Document:    osrtl/windows/heap/heap.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970724
Elements:    -

Header file: osrtl/windows/heap/heap.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970717
Elements:    -

Module:      <osrtl><heap><heap>  -  Heap management
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/heap/heap/heap.d
             Header file  osrtl/windows/heap/heap/heap.h
             Source file  osrtl/windows/heap/heap/heap.cpp

Document:    osrtl/windows/heap/heap/heap.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/heap/heap/heap.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     osbasicheap<datatype>::osbasicheap
             Function     osbasicheap<datatype>::HeapAllocate
             Function     osbasicheap<datatype>::HeapDeallocate
             Function     priv_osbasicheapadmin::MarkAllocation
             Function     priv_osbasicheapadmin::MarkDeallocation
             Function     priv_osbasicheapadmin::Allocations

Source file: osrtl/windows/heap/heap/heap.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     guard_osrtl_heap_heap::~guard_osrtl_heap_heap

Module:      <osrtl><heap><heapblck>  -  Heap blocks
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Document     osrtl/windows/heap/heapblck/heapblck.d
             Header file  osrtl/windows/heap/heapblck/heapblck.h
             Source file  osrtl/windows/heap/heapblck/heapblck.cpp

Document:    osrtl/windows/heap/heapblck/heapblck.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    -

Header file: osrtl/windows/heap/heapblck/heapblck.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    -

Source file: osrtl/windows/heap/heapblck/heapblck.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     osheapblock::osheapblock

Module:      <osrtl><heap><heapadm>  -  Heap administration
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/heap/heapadm/heapadm.d
             Header file  osrtl/windows/heap/heapadm/heapadm.h
             Source file  osrtl/windows/heap/heapadm/heapadm.cpp

Document:    osrtl/windows/heap/heapadm/heapadm.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    -

Header file: osrtl/windows/heap/heapadm/heapadm.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     heapadministrator::heapadministrator

Source file: osrtl/windows/heap/heapadm/heapadm.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     heapadministrator::MarkAllocation
             Function     heapadministrator::MarkDeallocation
             Function     heapadministrator::Allocated
             Function     heapadministrator::Allocated
             Function     guard_osrtl_heap_heapadm::
                             ~guard_osrtl_heap_heapadm

Module:      <osrtl><heap><safeheap>  -  Checked heap
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/heap/safeheap/safeheap.d
             Header file  osrtl/windows/heap/safeheap/safeheap.h

Document:    osrtl/windows/heap/safeheap/safeheap.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/heap/safeheap/safeheap.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     osheap<datatype>::osheap
             Function     osheap<datatype>::Allocate
             Function     osheap<datatype>::Deallocate
             Function     osheap<datatype>::SafeAllocate

Component:   <osrtl><text>  -  Textual data
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/windows/text/text.d
             Header file  osrtl/windows/text/text.h
             Module       <osrtl><text><char>
             Module       <osrtl><text><string>
             Module       <osrtl><text><number>

Document:    osrtl/windows/text/text.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970723
Elements:    -

Header file: osrtl/windows/text/text.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970714
Elements:    -

Module:      <osrtl><text><char>  -  Characters
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Document     osrtl/windows/text/char/char.d
             Header file  osrtl/windows/text/char/char.h
             Source file  osrtl/windows/text/char/char.cpp

Document:    osrtl/windows/text/char/char.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970723
Elements:    -

Header file: osrtl/windows/text/char/char.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970714
Elements:    -

Source file: osrtl/windows/text/char/char.cpp
Author:      E.A. Silkens
Revision:    1.2
Date:        19970826
Elements:    Function     IsCharWhiteSpace

Module:      <osrtl><text><string>  -  Text strings
Author:      E.A. Silkens
Revision:    1.1
Date:        19970822
Elements:    Document     osrtl/windows/text/string/string.d
             Header file  osrtl/windows/text/string/string.h
             Source file  osrtl/windows/text/string/string.cpp

Document:    osrtl/windows/text/string/string.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970822
Elements:    -

Header file: osrtl/windows/text/string/string.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     StringDuplicate
             Function     StringDuplicate
             Function     StringDuplicate
             Function     StringDifferent

Source file: osrtl/windows/text/string/string.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     StringLength
             Function     StringLength
             Function     StringCopy
             Function     StringEqual
             Function     priv_StringDuplicate

Module:      <osrtl><text><number>  -  Converts string <-> number
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/text/number/number.d
             Header file  osrtl/windows/text/number/number.h
             Source file  osrtl/windows/text/number/number.cpp

Document:    osrtl/windows/text/number/number.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/text/number/number.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Source file: osrtl/windows/text/number/number.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     StringLength
             Function     StringLength
             Function     StringLength
             Function     StringLength
             Function     ValueToString
             Function     ValueToString
             Function     ValueToString
             Function     ValueToString

Component:   <osrtl><sound>  -  Sounds
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    Document     osrtl/windows/sound/sound.d
             Header file  osrtl/windows/sound/sound.h
             Module       <osrtl><sound><sound>

Document:    osrtl/windows/sound/sound.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970723
Elements:    -

Header file: osrtl/windows/sound/sound.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    -

Module:      <osrtl><sound><sound>  -  Sounds
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    Document     osrtl/windows/sound/sound/sound.d
             Header file  osrtl/windows/sound/sound/sound.h
             Source file  osrtl/windows/sound/sound/sound.cpp

Document:    osrtl/windows/sound/sound/sound.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    -

Header file: osrtl/windows/sound/sound/sound.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    -

Source file: osrtl/windows/sound/sound/sound.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970725
Elements:    Function     OsSound

Component:   <osrtl><scrnobj>  -  Screen objects
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/scrnobj/scrnobj.d
             Header file  osrtl/windows/scrnobj/scrnobj.h
             Module       <osrtl><scrnobj><mouse>

Document:    osrtl/windows/scrnobj/scrnobj.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/scrnobj/scrnobj.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Module:      <osrtl><scrnobj><mouse>  -  Mouse cursors
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/scrnobj/mouse/mouse.d
             Header file  osrtl/windows/scrnobj/mouse/mouse.h
             Source file  osrtl/windows/scrnobj/mouse/mouse.cpp

Document:    osrtl/windows/scrnobj/mouse/mouse.
Author:      E.A. Silkens
Revision:    1.1
Date:        19970726
Elements:    -

Header file: osrtl/windows/scrnobj/mouse/mouse.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Source file: osrtl/windows/scrnobj/mouse/mouse.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     SelectMouseCursor

Component:   <osrtl><drawing>  -  Attributes for drawing
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    Document     osrtl/windows/drawing/drawing.d
             Header file  osrtl/windows/drawing/drawing.h
             Module       <osrtl><drawing><color>
             Module       <osrtl><drawing><font>

Document:    osrtl/windows/drawing/drawing.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/drawing/drawing.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Module:      <osrtl><drawing><color>  -  Colors
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    Document     osrtl/windows/drawing/color/color.d
             Header file  osrtl/windows/drawing/color/color.h
             Source file  osrtl/windows/drawing/color/color.cpp

Document:    osrtl/windows/drawing/color/color.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    -

Header file: osrtl/windows/drawing/color/color.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     oscolor::R
             Function     oscolor::G
             Function     oscolor::B

Source file: osrtl/windows/drawing/color/color.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     oscolor::oscolor
             Function     oscolor::oscolor
             Function     oscolor::operator =
             Function     oscolor::Define

Module:      <osrtl><drawing><font>  -  Fonts
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/drawing/font/font.d
             Header file  osrtl/windows/drawing/font/font.h
             Source file  osrtl/windows/drawing/font/font.cpp

Document:    osrtl/windows/drawing/font/font.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/drawing/font/font.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     osfont::Height
             Function     osfont::Width
             Function     osfont::Zero
             Function     osfont::FontName
             Function     osfont::Handle

Source file: osrtl/windows/drawing/font/font.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     osfont::osfont
             Function     osfont::~osfont
             Function     osfont::Create
             Function     osfont::Create
             Function     osfont::Create
             Function     osfont::Destroy
             Function     osfont::Transfer
             Function     DefaultFontName
             Function     CountFont
             Function     guard_osrtl_drawing_font::
                             ~guard_osrtl_drawing_font

Component:   <osrtl><window>  -  Window management
Author:      E.A. Silkens
Revision:    1.1
Date:        19970825
Elements:    Document     osrtl/windows/window/window.d
             Header file  osrtl/windows/window/window.h
             Module       <osrtl><window><winpaint>
             Module       <osrtl><window><winfont>
             Module       <osrtl><window><admin>
             Module       <osrtl><window><window>
             Module       <osrtl><window><safewin>

Document:    osrtl/windows/window/window.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    -

Header file: osrtl/windows/window/window.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    -

Module:      <osrtl><window><winpaint>  -  Window painter
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/window/winpaint/winpaint.d
             Header file  osrtl/windows/window/winpaint/winpaint.h

Document:    osrtl/windows/window/winpaint/winpaint.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/window/winpaint/winpaint.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Module:      <osrtl><window><winfont>  -  Window font
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Document     osrtl/windows/window/winfont/winfont.d
             Header file  osrtl/windows/window/winfont/winfont.h
             Source file  osrtl/windows/window/winfont/winfont.cpp

Document:    osrtl/windows/window/winfont/winfont.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    -

Header file: osrtl/windows/window/winfont/winfont.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     oswindowfont::Destroy
             Function     oswindowfont::Height
             Function     oswindowfont::HorizontalSize

Source file: osrtl/windows/window/winfont/winfont.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970818
Elements:    Function     oswindowfont::oswindowfont
             Function     oswindowfont::Create
             Function     oswindowfont::TextWidth
             Function     oswindowfont::TextWidthIdeal
             Function     oswindowfont::TextWidth
             Function     fontadmin::~fontadmin
             Function     fontadmin::Metrics
             Function     fontadmin::ActualSize
             Function     fontadmin::GetMetricFont
             Function     fontadmin::Destroy
             Function     GetCharMetrics
             Function     GetFontMetrics
             Function     guard_osrtl_window_winfont::
                             ~guard_osrtl_window_winfont

Module:      <osrtl><window><admin>  -  Window administration
Author:      E.A. Silkens
Revision:    1.1
Date:        19970819
Elements:    Document     osrtl/windows/window/admin/admin.d
             Header file  osrtl/windows/window/admin/admin.h
             Source file  osrtl/windows/window/admin/admin.cpp

Document:    osrtl/windows/window/admin/admin.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970819
Elements:    -

Header file: osrtl/windows/window/admin/admin.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     windowadmin::Windows
             Function     windowadmin::Ever
             Function     windowadmin::ClassName

Source file: osrtl/windows/window/admin/admin.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970806
Elements:    Function     windowadmin::windowadmin
             Function     windowadmin::Create
             Function     windowadmin::Destroy
             Function     guard_osrtl_window_admin::
                             ~guard_osrtl_window_admin

Module:      <osrtl><window><window>  -  Basic window
Author:      E.A. Silkens
Revision:    1.1
Date:        19970825
Elements:    Document     osrtl/windows/window/window/window.d
             Header file  osrtl/windows/window/window/window.h
             Source file  osrtl/windows/window/window/window.cpp

Document:    osrtl/windows/window/window/window.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    -

Header file: osrtl/windows/window/window/window.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970824
Elements:    Function     osbasicwindow::DestroyWindowFont
             Function     osbasicwindow::Width
             Function     osbasicwindow::Height
             Function     osbasicwindow::Name
             Function     osbasicwindow::Visible

Source file: osrtl/windows/window/window/window.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970825
Elements:    Function     WordToInt
             Function     osbasicwindow::osbasicwindow
             Function     osbasicwindow::~osbasicwindow
             Function     osbasicwindow::Create
             Function     osbasicwindow::Destroy
             Function     osbasicwindow::CreateWindowFont
             Function     osbasicwindow::FontHeight
             Function     osbasicwindow::TextWidth
             Function     osbasicwindow::Update
             Function     osbasicwindow::priv_MessageHandler
             Function     osbasicwindow::Write
             Function     osbasicwindow::Line
             Function     osbasicwindow::OutlineRectangle
             Function     osbasicwindow::FilledRectangle
             Function     osbasicwindow::WindowCreation
             Function     osbasicwindow::WindowDestruction
             Function     osbasicwindow::WindowPosSize
             Function     osbasicwindow::WindowActivation
             Function     osbasicwindow::WindowDeactivation
             Function     osbasicwindow::MousePosition
             Function     osbasicwindow::MouseLeftSelect
             Function     osbasicwindow::MouseLeftActivate
             Function     osbasicwindow::MouseLeftRelease
             Function     osbasicwindow::MouseRightSelect
             Function     osbasicwindow::MouseRightActivate
             Function     osbasicwindow::MouseRightRelease
             Function     osbasicwindow::CanBeClosed
             Function     osbasicwindow::SizeLimits
             Function     osbasicwindow::Paint
             Function     osbasicwindow::priv_Create
             Function     osbasicwindow::Write
             Function     masterwindow::Create
             Function     masterwindow::WindowDestruction
             Function     masterwindow::Paint
             Function     GenericHandler
             Function     guard_osrtl_window_window::
                             ~guard_osrtl_window_window
             Function     NamedLeftBorder
             Function     NamedRightBorder
             Function     NamedTopBorder
             Function     NamedBottomBorder
             Function     NamelessLeftBorder
             Function     NamelessRightBorder
             Function     NamelessTopBorder
             Function     NamelessBottomBorder
             Function     PositionAStoSP
             Function     PositionSPtoAS
             Function     PositionSPtoWA
             Function     PositionWAtoWP
             Function     PositionWPtoWA
             Function     SizeAStoSP
             Function     SizeFStoSP
             Function     SizeWAtoFS
             Function     SizeFStoWA
             Function     GetMouseData

Module:      <osrtl><window><safewin>  -  Generic window
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    Document     osrtl/windows/window/safewin/safewin.d
             Header file  osrtl/windows/window/safewin/safewin.h
             Source file  osrtl/windows/window/safewin/safewin.cpp

Document:    osrtl/windows/window/safewin/safewin.d
Author:      E.A. Silkens
Revision:    1.1
Date:        19970819
Elements:    -

Header file: osrtl/windows/window/safewin/safewin.h
Author:      E.A. Silkens
Revision:    1.1
Date:        19970820
Elements:    -

Source file: osrtl/windows/window/safewin/safewin.cpp
Author:      E.A. Silkens
Revision:    1.1
Date:        19970819
Elements:    Function     oswindow::oswindow
             Function     oswindow::~oswindow
             Function     oswindow::Create
             Function     oswindow::WindowDestruction
             Function     oswindow::WindowActivation
             Function     oswindow::WindowDeactivation
             Function     oswindow::WindowActivation
             Function     oswindow::WindowDeactivation
             Function     oswindow::MarkDestruction
             Function     guard_osrtl_window_safewin::
                             ~guard_osrtl_window_safewin


----------------------------------------------------------------------------
End of file confctrl/pack.cc .
----------------------------------------------------------------------------


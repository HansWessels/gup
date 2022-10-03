
----------------------------------------------------------------------------
---  File:     osrtl/windows/osrtl.d
---  Contents: NIL between program and (OS and RTL) - PC Windows
---  Unit:     <osrtl>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/osrtl.d  -  NIL between program and (OS and RTL) - PCWindows
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:40:56  erick
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
 1.1.000  First issue of this unit documentation file.
19970531
19970825
EAS
----------------------------------------------------------------------------


   This file contains the specific items that apply to the version of
<osrtl> that is targeted for PC Windows.  The generic documentation file for
all versions of <osrtl> (for any platform) is osrtl/osrtl.d .
   The PC Windows version of <osrtl> fully complies to the generic,
platform-independent version of <osrtl>.  The only specific thing about the
PC Windows version of <osrtl> that is worth mentioning is the set of
identifiers that are defined in the global namespace.  The PC Windows
version of <osrtl> is set up in such a way that these identifiers had to
appear in the global namespace, even if <osrtl> does not formally export
these identifiers.  The consequence of the fact that these identifiers are
in the global namespace is that other units must take care not to interfere
with these names.  The best way not to interfere is not to use these names
at all.
   The following list contains all preprocessor variables that are defined
using <#define> or un-defined using #undef in the header files of <osrtl>.
Other units must not define or un-define these preprocessor variables, and
they must not rely on their value or, even, on their existence.

      OSRTL
      OSRTL_BASICS
      OSRTL_BASICS_CONSTANT
      OSRTL_BASICS_CONVERT
      OSRTL_BASICS_OBJECT
      OSRTL_BASICS_TYPE
      OSRTL_DEBUGGING
      OSRTL_DRAWING
      OSRTL_DRAWING_COLOR
      OSRTL_DRAWING_FONT
      OSRTL_ERROR
      OSRTL_ERROR_ERRMSG
      OSRTL_ERROR_RESCODE
      OSRTL_HARDWARE
      OSRTL_HARDWARE_MOUSE
      OSRTL_HARDWARE_SCREEN
      OSRTL_HEAP
      OSRTL_HEAP_HEAP
      OSRTL_HEAP_HEAPADM
      OSRTL_HEAP_HEAPBLCK
      OSRTL_HEAP_SAFEHEAP
      OSRTL_PCWINDOWS
      OSRTL_SCRNOBJ
      OSRTL_SCRNOBJ_MOUSE
      OSRTL_SETTINGS
      OSRTL_SOUND
      OSRTL_SOUND_SOUND
      OSRTL_STARTUP
      OSRTL_STARTUP_STARTUP
      OSRTL_TASKING
      OSRTL_TASKING_MUTEX
      OSRTL_TASKING_MUTEXID
      OSRTL_TASKING_PHYSMUX
      OSRTL_TEXT
      OSRTL_TEXT_CHAR
      OSRTL_TEXT_NUMBER
      OSRTL_TEXT_STRING
      OSRTL_UNICODE
      OSRTL_WINDOW
      OSRTL_WINDOW_SAFEWIN
      OSRTL_WINDOW_WINDOW
      OSRTL_WINDOW_WINFONT
      OSRTL_WINDOW_WINPAINT

The following list contains all constants that are defined by <osrtl> but
that are not intended to be used by other parts of the program.

      MUTEX_FONTADMIN
      MUTEX_FONTMANAGE
      MUTEX_HEAPADMIN
      MUTEX_MUTEXADMIN
      MUTEX_SAFEWINDOW
      MUTEX_WINDOWADMIN
      MUTEX_WINDOWMANAGE

The following list contains all types that are defined by <osrtl> but that
are not intended to be used by other parts of the program.

      heapadministrator
      osbasicheap
      osbasicwindow
      osfont
      osheapblock
      physmutex
      priv_bordersize
      priv_osbasicheapadmin
      priv_windowarea
      programinstance

The following list contains all globally defined functions that are defined
by <osrtl> but that are not intended to be used by other parts of the
progam.

      priv_StringDuplicate
      ProgramInstance
      WindowExists


----------------------------------------------------------------------------
---  End of file osrtl/windows/osrtl.d .
----------------------------------------------------------------------------


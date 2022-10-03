
----------------------------------------------------------------------------
---  File:      osrtl/windows/settings.d
---  Contents:  Global settings for unit <osrtl>.
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/settings.d  -  Global settings for unit <osrtl>
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
 1.1.000  First issue of this documentation file.
19970820
19970820
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the global settings for <osrtl>
----------------------------------------------------------------------------

   To force all components of the unit <osrtl> to be compiled using the same
settings, a global settings file osrtl/windows/settings.h was created.  In
that file, all global settings for <osrtl> are combined.  Each component and
each module of <osrtl> includes that file.
   Refer to the Coding Standard for requirements on the global unit settings
file.


----------------------------------------------------------------------------
Chapter 2  -  Using the global settings for <osrtl>
----------------------------------------------------------------------------

   All components and all modules of the unit <osrtl> must include the
global settings file for <osrtl>, osrtl/windows/settings.h .


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the global settings for <osrtl>
----------------------------------------------------------------------------

OSRTL_DEBUGGING

   This symbol is defined if <osrtl> must be compiled in debugging mode.  It
is not defined if <osrtl> must be compiled in release mode.

----------------------------------------------------------------------------

OSRTL_UNICODE

   This symbol is defined if <osrtl> must be compiled to run on an operating
system that uses UNICODE strings.  This system is not defined if <osrtl>
must be compiled to run on an operating system that uses ANSI strings.


----------------------------------------------------------------------------
---  End of file osrtl/windows/settings.d .
----------------------------------------------------------------------------



----------------------------------------------------------------------------
---  File:      global/settings.d
---  Contents:  Global settings for GNU Pack.
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) global/settings.d  -  Global settings for GNU Pack
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:40:49  erick
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
Chapter 1  -  Purpose of the global settings for GNU Pack
----------------------------------------------------------------------------

   To force all units of GNU Pack to be compiled using the same global
settings, a global settings file global/settings.h was created.  In that
file, all global settings for GNU Pack are combined.  Each unit, each
component, and each module of GNU Pack includes that file.
   Refer to the Coding Standard for requirements on the global settings
file.


----------------------------------------------------------------------------
Chapter 2  -  Using the global settings for GNU Pack
----------------------------------------------------------------------------

   All units, all components, and all modules of GNU Pack must include the
global settings file for GNU Pack, global/settings.h .


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the global settings for GNU Pack
----------------------------------------------------------------------------

FORCE_DEBUGGINGMODE
FORCE_RELEASMODE

   These two symbols determine in which mode each unit, component, and
module of GNU Pack is compiled.  Each of these two symbols is either defined
or undefined.  The four possible combinations have the following results:

1. FORCE_DEBUGGINMODE: undefined
   FORCE_RELEASEMODE:  undefined
   Each unit determines its own compilation mode (debugging or release).
   This allows, for example, to compile one unit in debugging mode while all
   other units are compiled in release mode.

2. FORCE_DEBUGGINGMODE: undefined
   FORCE_RELEASEMODE:   defined
   Each unit is compiled in release mode, regardless of the local mode
   settings of the units.

3. FORCE_DEBUGGINGMODE: defined
   FORCE_RELEASEMODE:   undefined
   Each unit is compiled in debugging mode, regardless of the local mode
   settings of the units.

4. FORCE_DEBUGGINGMODE: defined
   FORCE_RELEASEMODE:   defined
   This combination is illegal.  It results in a compilation error.


----------------------------------------------------------------------------
---  End of file globa/settings.d .
----------------------------------------------------------------------------


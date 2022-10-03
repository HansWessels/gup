
----------------------------------------------------------------------------
---  File:     osrtl/windows/sound/sound/sound.d
---  Contents: Sounds.
---  Module:   <osrtl><sound><sound>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/sound/sound/sound.d  -  Sounds
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:53  erick
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
19970720
19970824
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><sound><sound>
----------------------------------------------------------------------------

   The module <osrtl><sound><sound> allows user code to generate sounds.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><sound><sound>
----------------------------------------------------------------------------

   The types and functions that are provided by <osrtl><sound><sound> can be
used directly by all other parts of the program.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><sound><sound> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Module <osrtl><basics>.
         <sound> uses basic types, such as <osbool>.

      Module <osrtl><sound><sound>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><sound><sound>
----------------------------------------------------------------------------

enum
ossound                // sound type
{
   OSSOUND_ALERT,      // alert user: a nominal (expected) event occurred
   OSSOUND_QUESTION,   // user must answer a question
   OSSOUND_PROBLEM     // notify user of a problem
};  // ossound

   <ossound> is used to indicate a type of sound.

----------------------------------------------------------------------------

osbool              // sound could be played?
OsSound             // plays a sound
(ossound  type,     // I: type of sound to play
 int      n = 1);   // I: number of times to play the sound

   Plays a sound of the type <type>.  The sound is played <n> times.  The
return value is <OSFALSE> if and only if either <type> is invalid, or if the
sound hardware refuses to play the sound.
   If <n> is zero or negative, no sound is played, and the function returns
<OSTRUE>.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/sound/sound/sound.d .
----------------------------------------------------------------------------


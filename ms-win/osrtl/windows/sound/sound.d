
----------------------------------------------------------------------------
---  File:      osrtl/windows/sound/sound.d
---  Contents:  Sounds.
---  Component: <osrtl><sound>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/sound/sound.d  -  Sounds
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:50  erick
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
19970720
19970723
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the component <osrtl><sound>
----------------------------------------------------------------------------

   The component <osrtl><sound> allows to play sounds.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><sound>
----------------------------------------------------------------------------

   The component <osrtl><sound> consists of but a single module,
<osrtl><sound><sound>; it is nothing but a higher-level view of
that module.  Therefore, refer to the documentation of the module
<osrtl><sound><sound> for a description of how to use the component
<osrtl><sound>.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the component <osrtl><sound>
----------------------------------------------------------------------------

   Because <osrtl><sound> is nothing but a high-level view of the module
<osrtl><sound><sound>, refer to the documentation file for that module for
a detailed explanation of the module <osrtl><sound>.
   To give an overview of the component <osrtl><sound>, here follows a list
of all items provided by <osrtl><sound><sound>.

Type:     ossound
Use:      Indicates a type of sound.  Three types of sound are supported:

             Type of sound       Meaning
             -------------------------------------------------------------
             OSSOUND_ALERT       Alert user of a nominal (expected) event.
             OSSOUND_QUESTION    User must answer a question.
             OSSOUND_PROBLEM     User is notified of a problem.


Function: OsSound
Use:      Plays a sound of a given type.  The sound is played a given number
          of times.


----------------------------------------------------------------------------
---  End of file osrtl/windows/sound/sound.d .
----------------------------------------------------------------------------



----------------------------------------------------------------------------
---  File:      osrtl/windows/basics/basics.d
---  Contents:  Basic definitions.
---  Component: <osrtl><basics>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/basics/basics.d  -  Basic definitions
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:40:59  erick
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
19970628
19970821
EAS
----------------------------------------------------------------------------
 1.1.001  osobject::DefineName , osobject::RetrieveName , and
19970822  osobject::KillName are no longer listed as being overridable by
19970822  derived classes.  This change brings this documentation file in
EAS       line with revision 1.1.001 of
          osrtl/windows/basics/object/object.h .
----------------------------------------------------------------------------
 1.2.000  Identical to revision 1.1.001.  Formalised revision.
19970825
19970825
EASl
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the component <osrtl><basics>
----------------------------------------------------------------------------

   The component <osrtl><basics> supplies all basic constants, types,
functions, and objects that are needed by almost any program.  Normally,
such basic things are provided by the RTL or the OS.  The purpose of
<osrtl><basics> is to provide these basic things in a platform-independent
way.


----------------------------------------------------------------------------
Chapter 2  -  Using the component <osrtl><basics>
----------------------------------------------------------------------------

   The component <osrtl><basics> consists of the modules <constant>, <type>,
<convert>, and <object>.  It only joins these modules.  Therefore, refer to
the documentation files for these modules for a description of how to use
the component <osrtl><basics>.


----------------------------------------------------------------------------
Chapter 3  -  Detailed explanation of the component <osrtl><basics>
----------------------------------------------------------------------------

   Because <osrtl><basics> is nothing but an aggregate of the modules
<constant>, <type>, <convert>, and <object>, refer to the documentation
files for these modules for a detailed explanation of the component
<osrtl><basics>.
   To give an overview of the component <osrtl><basics>, here follows a list
of all items provided by the modules that make up <osrtl><basics>.

Constant: OSNULL
Use:      The platform-independent equivalent of <NULL>.

Type:     int8
Use:      A signed integer type that contains at least 8 bits.  Its range is
          at least -127..+127 .

Type:     uint8
Use:      An unsigned integer type that contains at least 8 bits.  Its range
          is at least 0..255 .

Type:     int16
Use:      A signed integer type that contains at least 16 bits.  Its range
          is at least -32767..+32767 .

Type:     uint16
Use:      An unsigned integer type that contains at least 16 bits.  Its
          range is at least 0..65535 .

Type:     int32
Use:      A signed integer type that contains at least 32 bits.  Its range
          is at least -2147483647..+2147483647 .

Type:     uint32
Use:      An unsigned integer type that contains at least 32 bits.  Its
          range is at least 0..4294967295 .

Type:     osbool
Use:      The platform-independent equivalent of <bool>.  Possible values
          are <OSTRUE> and <OSFALSE>.

Function: RoundFloatToIntegral
Use:      Rounds a floating-point value to the nearest integral value.
          Rounding is done mathematically.  Returns <OSTRUE> if the
          floating-point value can be converted to an integral value, or
          <OSFALSE> if the floating-point value is outside the range of the
          provided integral type.

Function: CharConvert
Use:      Converts an ANSI character to its UNICODE equivalent, or vice
          versa.  Returns <OSTRUE> if the source character can be converted,
          or <OSFALSE> if the target character set does not contain an
          equivalent of the source character, resulting in a failure to
          convert the source character.

Function: StringConvert
Use:      Converts an ANSI string to its UNICODE equivalent, or vice versa.
          Returns <OSTRUE> if the source string can be converted (that is,
          if all characters of the source string can be converted to their
          UNICODE equivalents), or <OSFALSE> if the source string can not be
          converted (that is, if one or more of the characters of the source
          string can not be converted  because the target character set does
          not contain equivalents for those source characters).

Class:    osobject
Use:      The root of the class hierarchy.

Function: osobject::SetName
Use:      Assigns a name to the <osobject>.
          Functional in debugging mode only.

Function: osobject::ObjectName
Use:      Returns the name of the <osobject>.
          Functional in debugging mode only.

Function: osobject::DefineName
Use:      Implements <SetName>.

Function: osobject::RetrieveName
Use:      Implements <ObjectName>.

Function: osobject::KillName
Use:      Kills the current name of the <osobject>, if any.


----------------------------------------------------------------------------
---  End of file osrtl/windows/basics/basics.d .
----------------------------------------------------------------------------


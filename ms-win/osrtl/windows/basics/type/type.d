
----------------------------------------------------------------------------
---  File:     osrtl/windows/basics/type/type.d
---  Contents: Types.
---  Module:   <osrtl><basics><type>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/basics/type/type.d  -  Types
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:10  erick
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
19970627
19970727
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><basics><type>
----------------------------------------------------------------------------

   The module <osrtl><basics><type> supplies basic types that are needed by
almost any program.  It supplies these types in a platform-independent way.
It supplies only those types that are normally provided by the OS or the
C++ RTL.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><basics><type>
----------------------------------------------------------------------------

   All other parts of the project can directly use the types that are
provided by <type>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><basics><type> does not depend on any other part of the
program.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><basics><type>
----------------------------------------------------------------------------

Type: int8

   <int8> is a signed integer type that contains exactly 8 bits.  Values of
this type have a range of exactly -128..+127 .

----------------------------------------------------------------------------

Type: uint8

   <uint8> is an unsigned integer type that contains exactly 8 bits.  Values
of this type have a range of exactly 0..255 .

----------------------------------------------------------------------------

Type: int16

   <int16> is a signed integer type that contains exactly 16 bits.  Values
of this type have a range of exactly -32768..+32767 .

----------------------------------------------------------------------------

Type: uint16

   <uint16> is an unsigned integer type that contains exactly 16 bits.
Values of this type have a range of exactly 0..65535 .

----------------------------------------------------------------------------

Type: int32

   <int32> is a signed integer type that contains exactly 32 bits.  Values
of this type have a range of exactly -2147483648..+2147483647 .

----------------------------------------------------------------------------

Type: uint32

   <uint32> is an unsigned integer type that contains exactly 32 bits.
Values of this type have a range of exactly 0..4294967295 .

----------------------------------------------------------------------------

Type:   osbool
Values: OSFALSE, OSTRUE

   <osbool> is a platform-independent boolean type.  It replaces any other
boolean type that may be provided by a specific RTL.  Its values <OSFALSE>
and <OSTRUE> replace the values <FALSE> and <TRUE> that are normally
provided by the RTL.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   Defining <OSFALSE> as (0 == 1) and defining <OSTRUE> as (0 == 0) allows
direct assignment of boolean expressions to <osbool>s, as required by the
Coding Standard.  So, if <a> is an <int>, the following is legal for all
values of <a>:

      osbool  islarge = a > 1000;


----------------------------------------------------------------------------
---  End of file osrtl/windows/basics/type/type.d .
----------------------------------------------------------------------------


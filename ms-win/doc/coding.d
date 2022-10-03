

                        --  CODING STANDARD  --


                             FOR GNU PACK



0  -  TABLE OF CONTENTS

 0  -  Table of contents  . . . . . . . . . . . . . . . . . . . . . .  10
 1  -  Revision record  . . . . . . . . . . . . . . . . . . . . . . .  34
       1.1  -  RCS-compatible revision record . . . . . . . . . . . .  36
       1.2  -  Detailed revision record . . . . . . . . . . . . . . .  43
 2  -  Scope  . . . . . . . . . . . . . . . . . . . . . . . . . . . .  71
 4  -  Programming language . . . . . . . . . . . . . . . . . . . . .  92
 6  -  Directory tree . . . . . . . . . . . . . . . . . . . . . . . . 111
17  -  Constructors and destructors . . . . . . . . . . . . . . . . . 138
28  -  Change control . . . . . . . . . . . . . . . . . . . . . . . . 158
       28.10  -  Generating new revisions of the program  . . . . . . 160
Appendix A  -  RCS-compatible revision record . . . . . . . . . . . . 170

   The numbers at the right side of the table of contents are the line
numbers of the titles of the corresponding sections.  Line numbers are used
instead of page numbers because the EASy Programming Group has a policy not
to print anything.  This saves natural resources, which are becoming as
scarce as resources in Windows.
   Despite the fact that we at the EASy Programming Group are all C++
programmers, the first line is assigned line number 1, not 0.  This was done
to comply with the convention of almost all the editors and viewers we use.


1  -  REVISION RECORD

1.1  -  RCS-Compatible Revision Record

   This section lists the revision history of this document as the RCS
sees it.
   The actual RCS-compatible revision record is given in appendix A.


1.2  -  Detailed Revision Record

   This section lists the detailed revision history of this document.
Unfortunately, the environment that is used to generate and maintain this
document does not allow to automatically generate change bars.  Therefore,
this revision record is the only place where changes to this document are
recorded.

.----------.---------------------------------------------------------------.
| Revision | Concise description of the changes                            |
| Dates    |                                                               |
| Author   |                                                               |
|----------+---------------------------------------------------------------|
|  1.1.000 | First issue of this document.                                 |
| 19970608 |                                                               |
| 19970710 |                                                               |
| EAS      |                                                               |
|----------+---------------------------------------------------------------|
|  1.1.001 | The requirements in section 6 (Directory Tree) have been      |
| 19970820 | updated.  A platform directory shall not be created inside    |
| 19970820 | each unit directory and each unit binary directory.  Platform |
| EAS      | directories are only needed in the unit directory and the     |
|          | unit binary directory for <osrtl>.  The very existence of     |
|          | <osrtl> enables all other units to be platform-independent,   |
|          | so all other units don't need a directory for each platform.  |
`----------'---------------------------------------------------------------'


2  -  SCOPE

   This Coding Standard tailors the generic Coding Standard of the EASy
Programming Group for use in the project GNU Pack.
   The generic Coding Standard doc\coding.gen shall be applicable to the
project GNU Pack, with exceptions as indicated in this specific Coding
Standard.
   Because this specific Coding Standard is based on the generic Coding
Standard, the section numbers in this document are identical to the
corresponding section numbers in the generic Coding Standard.  As is evident
from the table of contents in section 0, the consequence of this approach is
that not all section numbers are used.
   If a section number that is present in the generic Coding Standard is not
present in this tailored Coding Standard, the section of the generic Coding
Standard shall be applicable.  If a section number is present in this
tailored Coding Standard, the section in this tailored Coding Standard shall
override the corresponding section of the generic Coding Standard, unless
explicitly indicated otherwise in that section in this tailored Coding
Standard.


4  -  PROGRAMMING LANGUAGE

   All code shall be written in ANSI C++, except in cases where the use of
ANSI C++ is not possible because of restrictions imposed by the run-time
environment.  An example of this is a restriction that is imposed by the
Windows API: some of the API functions require the use of calling
conventions (<PASCAL>, <CALLBACK>, ...).
   If ANSI C++ cannot be used, the code shall adhere as much as possible to
the ANSI C++ language.
   Exceptions shall not be used.  This means that code written for GNU Pack
shall not throw exceptions, and that this code shall not handle exceptions,
except as indicated in the next paragraph.
   Whenever a library function is needed that may throw an exception, a
wrapper function shall be designed that insulates this library function and
catches all exceptions thrown by that library function.  This insulation
shall be coded in the NIL to the library.  Refer to section 11 for a
description of NILs.


6  -  DIRECTORY TREE

   Section 6 of the generic Coding Standard shall be applicable, with the
exception of the lay-out of the directory tree.

      a. Component directories shall be created in each unit directory as
         indicated by point 3 of the generic Coding Standard, with the
         exception of the unit directory for the unit <osrtl>, which is a
         NIL between the program and [the OS and the RTL].  In the unit
         directory for <osrtl>, one directory shall be created for each of
         the platforms that are supported by GNU Pack.  The component
         directories of the components of <osrtl> shall be created inside
         the relevant platform directory.
      b. Component binary directories shall be created in each unit binary
         directory as indicated by point 7 of the generic Coding Standard,
         with the exception of the unit binary directory of the unit
         <osrtl>, which is a NIL between the program and [the OS and the
         RTL].  In the unit binary directory for <osrtl>, one binary
         directory shall be created for each of the platforms that are
         supported by GNU Pack.  The component binary directories for
         <osrtl> shall be created inside the relevant platform binary
         directories.

Reason: The above points are agreements made in the development team of GNU
        Pack.


17  -  CONSTRUCTORS AND DESTRUCTORS

   Section 17 of the generic Coding Standard shall be applicable.  The
following requirements must be added to that section 17.
   A constructor must not perform any operations that may fail.  Its only
task must be to bring the object under construction in a well-defined state.
Operations that may fail must be deferred to a member function that opens,
creates, initialises, ... the object.
   A destructor must not perform any operations that may fail.  Its only
task must be to close down the object.  Operations that may fail must be
performed by a member function that closes, destroys, shuts down, ... the
object.

Reason: Constructors and destructors are unable to return error codes to
        their callers.  Because error conditions are handled by returning
        an error code rather than by throwing an exception (section 4),
        constructors and destructors must not perform operations that may
        fail.


28  -  CHANGE CONTROL

28.10  -  Generating New Revisions Of The Program

   Section 28.10 of the generic Coding Standard shall be applicable, with
the exception that it shall be the development team of GNU Pack, not just
the EASy Programming Group, who shall decide when to generate a new revision
of the program.  It shall also be the development team of GNU Pack, not just
the EASy Programming Group, who shall decide about the revision numbers to
use for the program.


APPENDIX A  -  RCS-COMPATIBLE REVISION RECORD

$RCSfile$
$Author: erick $
$Revision: 84 $
$Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
$Log$
Revision 1.2  1997/12/30 13:40:45  erick
First version of the MS-Windows shell (currently only osrtl).



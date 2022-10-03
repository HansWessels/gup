
----------------------------------------------------------------------------
---  File:     osrtl/windows/basics/constant/constant.d
---  Contents: Constants.
---  Module:   <osrtl><basics><constant>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/basics/constant/constant.d  -  Constants
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:01  erick
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
19970723
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><basics><constant>
----------------------------------------------------------------------------

   The module <osrtl><basics><constant> supplies basic constants that are
needed by almost any program.  It supplies these constants as platform-
independent entities.  It supplies only those constants that are normally
provided by the OS or the C++ RTL.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><basics><constant>
----------------------------------------------------------------------------

   All other parts of the project can directly use the constants that are
provided by <constant>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><basics><constant> does not depend on any other part of
the program.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><basics><constant>
----------------------------------------------------------------------------

Constant: OSNULL

<OSNULL> is the platform-independent equivalent of the constant <NULL> that
is provided by ANSI C++.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/basics/constant/constant.d .
----------------------------------------------------------------------------


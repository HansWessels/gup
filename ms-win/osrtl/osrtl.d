
----------------------------------------------------------------------------
---  File:     osrtl/osrtl.d
---  Contents: NIL between program and (OS and RTL).
---  Unit:     <osrtl>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/osrtl.d  -  NIL between program and (OS and RTL)
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:40:53  erick
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


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the unit <osrtl>
----------------------------------------------------------------------------

   The unit <osrtl> supplies a NIL (Neutral Interface Layer) between the
program on the one hand, and the OS and the RTL on the other hand.  It hides
all OS- and RTL-specific things from the program.  This allows the program
to be written using the NIL only.  In doing so, the program becomes easily
portable: in order to port the program to a different platform, only <osrtl>
must be ported.


----------------------------------------------------------------------------
Chapter 2  -  Using the unit <osrtl>
----------------------------------------------------------------------------

   In order to properly use <osrtl> and live to its philosophy, applications
must NOT use any functionality provided by either the OS or the RTL.  The
functionality provided by <osrtl> must be used instead.  This is the only
way the program can remain portable.
   Note that using <osrtl> in favour of the OS and the RTL includes using
constants and types that are defined by <osrtl>.  For example, applications
must use the constant <OSNULL> rather than the constant <NULL> that is
defined by the RTL.  And applications must use the type <osbool> with the
associated values <OSFALSE> and <OSTRUE> rather than the type <bool> and the
associated values <false> and <true> or <FALSE> and <TRUE> that are defined
by some (but not all!) RTLs.

   When building programs on top of <osrtl>, the developer has to have a
thorough understanding of the basic concepts upon which <osrtl> is built.
These concepts are described in the following chapters:

      2.1  - Elementary concepts in using <osrtl>
      2.2  - Initialising, running, and closing down the application

The functionality of <osrtl> is described in the following chapters:

      2.3  - Basic constants and types
      2.4  - Result codes
      2.5  - Displaying error messages
      2.6  - Base class
      2.7  - Numeric conversions
      2.8  - Characters and strings
      2.9  - Numbers and strings
      2.10 - Heap management
      2.11 - Obtaining the name of the application
      2.12 - Semaphores
      2.13 - Playing sounds
      2.14 - Modifying the shape of the mouse cursor
      2.15 - Colors
      2.16 - The window painter
      2.17 - Fonts
      2.18 - Window management

The following chapter contains a diagram of the class tree of all classes
that are provided by <osrtl>:

      2.19 - Class tree

Unfortunately, <osrtl> has to define a number of identifiers in the global
namespace that are not meant to be used by other parts of the program.
Which identifiers are defined in the global namespace depends on the target
platform for which <osrtl> is used.  Therefore, these identifiers are listed
in the documentation files for the various platforms that <osrtl> supports.

   If you look inside the header files that define the external interface of
<osrtl>, you will find more public classes, class members, functions, types,
and variables than are listed in this documentation file.  From a language
point of view, <osrtl> exports these items.  However, other units must not
use them even if they could, because future versions may not support these
items.  Other units should only use the items that are mentioned in this
documentation file, because only those items are meant to be exported by
<osrtl>.  Only if an application uses only the items that <osrtl> officially
exports, can the application be expected to be truely portable and maintain-
able.


----------------------------------------------------------------------------
2.1 - Elementary concepts in using <osrtl>
----------------------------------------------------------------------------

   The purpose of <osrtl> is to hide all details of the OS and the RTL from
the program.  If <osrtl> would fully conform to this purpose, it should hide
everything - even a basic type like <int>.
   It is clear that it would not be practical to hide things like <int> -
these items can be considered so very standard that every platform supports
them in the same way (except, in the case of <int>, the range of values that
can be represented).  Therefore, <osrtl> takes the practical approach not to
hide language elements that can be considered extremely standard.
   Other language elements, however, are hidden by <osrtl>.  These are the
language elements that are not so standard as <int>.  For example, <bool> is
a relatively new data type that has made it to the Standards only recently.
This causes <bool> not to be so widely available as <int>: not all compilers
support <bool>, and those that do may differ in their implementation.
Therefore, such language elements are hidden by <osrtl>: <osrtl> provides
its own implementation.
   Refer to chapter 2.3 (Basic constants and types) for a description of the
constants and types that <osrtl> provides.  Programs that use <osrtl> must
use the constants and types of chapter 2.3 rather than their equivalents of
the RTL.  Only types that are not listed in chapter 2.3, like <int>, may be
used directly from the RTL.

   To allow programs maximum flexibility when using text, <osrtl> uses
UNICODE text (which uses 16-bit characters) rather than ANSI text (which
uses 8-bit characters).  All interfaces of <osrtl> use <wchar_t> instead of
<char>.  Programs that use <osrtl> should also use UNICODE text whenever
possible.
   If a program that uses <osrtl> has to use ANSI text for some reason, it
can use conversion functions to convert ANSI characters or strings to their
UNICODE equivalents, and vice versa.  These conversion functions are
described in chapter 2.8 (Characters and strings).

   <osrtl> uses result codes (error codes) to indicate the results of its
actions.  Only in some cases, <osrtl> uses special ("magic") values to
indicate failure; an example is allocation of memory on the heap, where
<osrtl> returns a null pointer if the allocation fails.
   <osrtl> does not use (throw) exceptions.  If a function from the OS or
the RTL throws an exception, <osrtl> catches the exception and transforms it
into a result code.  Programs may assume that <osrtl> never throws any
exception at all.  Ideally, programs should adhere to the approach used in
<osrtl> and use result codes too, because this results in a consistent
approach to error handling in the entire program.  Using result codes
instead of exceptions is not enforced, though.


----------------------------------------------------------------------------
2.2 - Initialising, running, and closing down the application
----------------------------------------------------------------------------

   The program is started by <osrtl>.  To this end, the program must
implement the following three functions:

      wchar_t const *         // name of the application
      ApplicationName         // obtains the name of the application
      ();

      int                     // status code
      ApplicationInitialise   // initialises the program
      ();

      int                     // status code
      ApplicationCloseDown    // closes down the program
      ();

   <osrtl> first calls <ApplicationName> to ask the application for its
name.  <ApplicationName> must return a string that contains the name of the
application, or it must return static_cast<wchar_t const *>(OSNULL) to
indicate that the application does not have a name.  <osrtl> makes a copy
of the application's name.  This copy can be retrieved by calling
<ProgramName> (chapter 2.11 (Obtaining the name of the application)).
   Next, <osrtl> calls <ApplicationInitialise> to allow the program to
initialise.  The initialisation process that <ApplicationInitialise>
performs depends on the application.  <ApplicationInitialise> should at
least create one or more windows through which the user can interact with
the program.
   <ApplicationInitialise> must return a status code.  If this status code
indicates success, <osrtl> continues normally - see below.  If the status
code indicates a problem, <osrtl> calls <ApplicationCloseDown> and
terminates execution; it passes the status code that <ApplicationInitialise>
returned to the operating system.
   If <ApplicationInitialise> indicated successful initialisation, <osrtl>
checks if any windows were created (it does not matter if these windows are
visible or invisible).  If not, there is not way for the user to interact
with the program; this is considered a problem, and <osrtl> will act as if
<ApplicationInitialise> indicated failure.  If one or more windows were
created, <osrtl> "runs" the windows.  This means that the actual function-
ality of the program must be in the code of the windows.  This window code
is driven by <osrtl>; refer to chapter 2.18 (Window management) for a
description of the window functionality that <osrtl> provides, and the
window functionality that <osrtl> expects the window to implement.
   When the program terminates (usually on request of the user through
interaction with one of the application's windows), <osrtl> calls
<ApplicationCloseDown>.  <ApplicationCloseDown> must perform any close-down
actions that are required for an orderly termination of execution.  These
actions depend, of course, on the application's code.

   Summarising, the sequence of events during the process of initialising,
running and closing down the program is:

      1. Call <ApplicationName> to obtain the name of the application, and
         make a copy of that name if the name is not null.
      2. If a failure occurred in step 1, terminate the program and return
         an appropriate status code to the operating system.
         End of sequence.
      3. Call <ApplicationInitialise> to allow the application to
         initialise.
      4. If a failure occurred in step 3, call <ApplicationCloseDown> to
         allow the program to orderly close down, then terminate the program
         and return the status code of <ApplicationInitialise> to the
         operating system.
         End of sequence.
      5. Check if the application created at least one window.  If not, call
         <ApplicationCloseDown> to allow the program to orderly close down,
         then terminate the program and return an appropriate status code to
         the operating system.
         End of sequence.
      6. "Run" the window(s).  This means that the normal functionality of
         the window(s) is executed.  "Running" the window(s) continues until
         the user indicates that the program must terminate, or until the
         last window is destroyed.
      7. Call <ApplicationCloseDown> to allow the program to orderly close
         down.
      8. Destroy the copy of the application's name that was made in step 1.

The status code that <osrtl> returns to the operating system is the first
status code that indicates a problem.  For example, if
<ApplicationInitialise> indicates a problem, and <ApplicationCloseDown>
indicates another problem, the status code that was returned by
<ApplicationInitialise> is returned to the operating system, not the status
code that is returned by <ApplicationCloseDown>.
   Note that <ApplicationCloseDown> is called even if
<ApplicationInitialise> failed:

      - If <ApplicationInitialise> is not called, <ApplicationCloseDown> is
        not called either.
      - If <ApplicationInitialise> is called, <ApplicationCloseDown> is
        called too, regardless of the return code of
        <ApplicationInitialise>.


----------------------------------------------------------------------------
2.3 - Basic constants and types
----------------------------------------------------------------------------

   If you wish to use a certain basic constant or type, first check if that
constant or type is present in this chapter, or if an equivalent constant or
type is present in this chapter.  If it is, use the constant or type that is
mentioned in this chapter instead of the constant or type as it is defined
by the OS or the RTL.  Only if the constant or type is not mentioned in this
chapter, you may use the constant or type as it is defined by the OS or the
RTL.

----------------------------------------------------------------------------

#define  OSNULL  ( static_cast<void *>(0) )

   <OSNULL> is the platform-independent equivalent of the constant <NULL>
that is defined by the RTL.  Use <OSNULL> instead of <NULL>.

----------------------------------------------------------------------------

enum
osbool
{
   OSFALSE = static_cast<int>(0 == 1),
   OSTRUE  = static_cast<int>(0 == 0)
};

   The platform-independent equivalent of <bool>.  Possible values are
<OSTRUE> and <OSFALSE>; these values are described below.  <osbool>s must be
used like ordinary boolean variables:

      osbool  big;   // whether <number> is 100 or greater

      big = number >= 100;
      if (big)
         //...

Use <osbool> instead of <bool> or, heaven forbid, <int>.  Do not use <bool>
even if your OS or your RTL supports it.
   The constant <OSFALSE> can be assigned to variables of type <osbool>, and
it can be used in conditional expressions (though literal use of <OSFALSE>
in conditional expressions is neither neccesary nor advised).
   The constant <OSTRUE> can be assigned to variables of type <osbool>, and
it can be used in conditional expressions (though literal use of <OSTRUE> in
conditional expressions is neither neccesary nor advised).

----------------------------------------------------------------------------

typedef  signed char  int8;

   A signed integer type that contains at least 8 bits.  Its range is at
least -127..+127 .

----------------------------------------------------------------------------

typedef  unsigned char  uint8;

   An unsigned integer type that contains at least 8 bits.  Its range is at
least 0..255 .

----------------------------------------------------------------------------

typedef  signed short  int16;

   A signed integer type that contains at least 16 bits.  Its range is at
least -32767..+32767 .

----------------------------------------------------------------------------

typedef  unsigned short  uint16;

   An unsigned integer type that contains at least 16 bits.  Its range is at
least 0..65535 .

----------------------------------------------------------------------------

typedef  signed int  int32;

   A signed integer type that contains at least 32 bits.  Its range is at
least -2147483647..+2147483647 .

----------------------------------------------------------------------------

typedef  unsigned int  uint32;

   An unsigned integer type that contains at least 32 bits.  Its range is at
least 0..4294967295 .


----------------------------------------------------------------------------
2.4 - Result codes
----------------------------------------------------------------------------

   As indicated in chapter 2.1 (Elementary concepts in using <osrtl>),
<osrtl> uses result codes rather than exceptions to indicate the results of
its actions.  All result codes are gathered into a single <enum> type,
<osresult>.
   Each of the possible values of an <osresult> has, of course, a
corresponding integer value.  These integer values will not change in future
revisions of <osrtl>.  If new values are added in the future, they will be
added at the end of the appropriate category.  This means that the integer
values of the actual result codes will not change; only the integer values
of the end markers of a category (see below) will change.

----------------------------------------------------------------------------

enum
osresult                     // result code
{
   // OK code.
   OSRESULT_OK,              // no error

   // Errors.
   OSERROR_FIRST = 1000,     // (marker: first error code)
   OSERROR_VALUEOUTOFRANGE,  // numeric value out of range
   OSERROR_NOCHARCONVERT,    // cannot convert char between UNICODE and ANSI
   OSERROR_NOMEMORY,         // not enough memory
   OSERROR_NORSRC_MCURSOR,   // not enough resources to set a mouse cursor
   OSERROR_NORSRC_FONT,      // not enough resources to create a font
   OSERROR_NORSRC_WINDOW,    // not enough resources to create a window
   OSERROR_NORSRC_DRAW,      // not enough resources to draw to a window
   OSERROR_NORSRC_WRITE,     // not enough resources to write text to window
   OSERROR_NODEFAULTFONT,    // no suitable default font present
   OSERROR_LAST,             // (marker: last error code)

   // Internal errors.
   OSINTERNAL_FIRST = 2000,  // (marker: first internal error code)
   OSINTERNAL_MUTEXADMIN,    // mutex administration
   OSINTERNAL_HEAPADMIN,     // heap administration
   OSINTERNAL_MOUSECURSOR,   // setting the mouse cursor
   OSINTERNAL_FONTADMIN,     // font administration
   OSINTERNAL_WINDOWADMIN,   // window administration
   OSINTERNAL_DRAW,          // drawing to a window
   OSINTERNAL_WRITE,         // writing text to a window
   OSINTERNAL_LAST,          // (marker: last internal error code)

   OSRESULT_LAST             // (marker: last result code)

};  // osresult

   An <osresult> is used to indicate the result of an action or an
operation.
   The marker values are never returned or used by any part of <osrtl>.
They are intended to be used when determining the category of a result code.
They are also intended to be used when, for example, an array is needed that
is indexed with result codes.  Using the marker values makes sure that the
size of the array is adapted automatically to possible new result codes in
future revisions of <osrtl>.


----------------------------------------------------------------------------
2.5 - Displaying error messages
----------------------------------------------------------------------------

   <osrtl> allows programs to display simple error messages.  These error
messages are meant as a back-up method of displaying information, to be used
when all other methods have failed.  The functions of <osrtl> that display
error messages are limited, and they display the messages in a very simple,
basic format.  The length of the message text and the number of lines in the
message text are limited.  The exact limits, and the exact format of the
display depend on the operating system for which <osrtl> is used.
   Error messages are displayed using one of the three forms of either
<OsErrorMessage> or <OsDebugErrorMessage>.  Both functions are described in
detail below.  <OsErrorMessage> displays its message in a named message box.
The behaviour of <OsDebugErrorMessage> depends on the compilation mode of
<osrtl>.  In debugging mode, it displays its message in a named message box
(by calling <OsErrorMessage>); the name of the message box is "Debug error".
In release mode, it expands to a null statement.

----------------------------------------------------------------------------

inline
void
OsErrorMessage                // displays an error message
(wchar_t const *  message,    // I: error message
 wchar_t const *  title);     // I: title of the message box

inline
void
OsErrorMessage                // displays an error message
(wchar_t const *  message1,   // I: error message, part 1
 wchar_t const *  message2,   // I: error message, part 2
 wchar_t const *  title);     // I: title of the message box

void
OsErrorMessage                // displays an error message
(wchar_t const *  message1,   // I: error message, part 1
 wchar_t const *  message2,   // I: error message, part 2
 wchar_t const *  message3,   // I: error message, part 3
 wchar_t const *  title);     // I: title of the message box

   <OsErrorMessage> displays an error message in a message box that has a
name <title>.  The three forms of <OsErrorMessage> function identical; the
only difference is the number of text parameters.  This allows the caller to
supply the message text in one, two, or three parts.  <OsErrorMessage> adds
the parts of the message to each other without inserting any characters; it
doesn't even insert a whitespace or a newline character.
   The first two forms of <OsErrorMessage> expand to calls to the third form
of <OsErrorMessage>, supplying static_cast<wchar_t const *>(OSNULL) for the
missing parameters.

----------------------------------------------------------------------------

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message);   // I: error message
#else
   (wchar_t const *  );          // -: error message
#endif

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2);  // I: error message, part 2
#else
   (wchar_t const *  ,           // -: error message, part 1
    wchar_t const *  );          // -: error message, part 2
#endif

inline
void
OsDebugErrorMessage              // displays an error message
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  message1,   // I: error message, part 1
    wchar_t const *  message2,   // I: error message, part 2
    wchar_t const *  message3);  // I: error message, part 3
#else
   (wchar_t const *  ,           // -: error message, part 1
    wchar_t const *  ,           // -: error message, part 2
    wchar_t const *  );          // -: error message, part 3
#endif

   In debugging mode, each of the three forms of <OsDebugErrorMessage>
expands to a call to the corresponding form of <OsErrorMessage>, specifying
the text L"Debug error" as the title of the message box.  Refer to the
detailed description of <OsErrorMessage>, above, for an explanation of the
actions and properties of <OsErrorMessage>.
   In release mode, each of the three forms of <OsDebugErrorMessage> expands
to the null statement; as a consequence, <OsDebugErrorMessage> does nothing
when <osrtl> is compiled in release mode.


----------------------------------------------------------------------------
2.6 - Base class
----------------------------------------------------------------------------

   <osrtl> supplies a class <osobject> that is intended to be the root class
of all classes of a class hierarchy.  By publicly deriving all classes from
the class <osobject>, each object may be accessed as if it were an
<osobject>.  This makes it easy to create lists and arrays of objects of any
kind.  It also makes it easy to destroy an object of any kind.
   All other classes in the project should be publicly derived from
<osobject>:

      class derivedclass:    // any class in the project
         public osobject     //    generic object
      {
         // ...              // contents of <derivedclass>
      };  // derivedclass

   In debugging mode, <osobject> variables may be assigned a name.  This is
an easy-to-use method to identify objects, thereby facilitating debugging.
A name is assigned to an <osobject> through the member function <SetName>.
The current name of an object can be obtained through <ObjectName>.  In
release mode, <SetName> expands to a null statement, and <ObjectName>
returns static_cast<wchar_t const *>(OSNULL) .  If an <osobject> is
initialised, its name is static_cast<wchar_t const *>(OSNULL) by default.

----------------------------------------------------------------------------

inline
osobject::osobject   // initialises an <osobject>
()

   Initialises an <osobject>.  In debugging mode, this means that the name
of the object is set to static_cast<wchar_t const *>(OSNULL) .  In release
mode, osobject::osobject expands to a null statement.

----------------------------------------------------------------------------

osobject::~osobject   // closes down an <osobject>
()

   Closes down an <osobject>.  In debugging mode, this means that some
administrative chores are performed.  In release mode, osobject::~osobject
expands to a null statement.

----------------------------------------------------------------------------

inline
osbool                          // object name could be set?
osobject::SetName               // sets the name of the object
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  newname)   // I: new name of the object
#else
   (wchar_t const *  )          // -: new name of the object
#endif

   Assigns a name to the <osobject>.  The return value indicates if the name
was successfully assigned.
   In debugging mode, a copy is made of <newname>, and the copy is stored as
the name of the <osobject>.  If the function succeeds, and if the <osobject>
already had a name, the previous name is destroyed.  To remove the current
name from an osobject, use SetName(static_cast<wchar_t const *>(OSNULL)) .
If <SetName> fails, the <osobject> keeps its original name.
   In release mode, <SetName> does nothing.  It just returns <OSTRUE> to
simulate successful assignment of the name to the <osobject>.
   Both in debugging mode and in release mode, <SetName> may be used any
number of times to assign a (new) name to the <osobject>.
   Use osobject::ObjectName to retrieve the name of the <osobject>.

----------------------------------------------------------------------------

inline
wchar_t const *        // name of the object
osobject::ObjectName   // retrieves the name of the object
()
const

   Obtains the current name of the <osobject>.  In debugging mode, the last
name that was successfully set by <SetName> is returned.  In release mode,
static_cast<wchar_t const *>(OSNULL) is returned.
   Use osobject::SetName to set the name of the <osobject>.


----------------------------------------------------------------------------
2.7 - Numeric conversions
----------------------------------------------------------------------------

   Numeric conversions convert a numeric value from one numeric format to
another numeric format.  Currently, <osrtl> only supplies a function to
convert a <float> to an <int> or a <long>.
   To convert an <int> to a <float>, an application should use the standard
conversion

      f = static_cast<float>(i);

The meaning of this statement is clear, and its behaviour is defined on all
platforms.
   To convert a <float> to an <int>, an application might use

      i = static_cast<int>(f);

However, the behaviour of this statement is not identical on all platforms
or on all compilers.  If, for example, f==-6.5 , some compilers on some
systems assign -6 to <i>, while other compilers on other systems assign -7
to <i>.  On all systems, the statement fails horribly if <f> is not within
the allowable range for an <int>.
   Therefore, applications should use <RoundFloatToIntegral> instead of the
above cast expression.  <RoundFloatToIntegral> tells its caller if (the
rounded value of) the <float> was inside the allowable range for the
integral value (<int> or <long>).  If it was, it rounds the <float> to the
integral value in a way which is well defined and (therefore) identical on
all machines and all compilers.
   The <float> is rounded mathematically:

       6.499 -->  6
       6.500 -->  7
       6.501 -->  7
      -6.499 --> -6
      -6.500 --> -6   (Yes, that's mathematical rounding!)
      -6.501 --> -7

The return value indicates if the conversion could be done:

      OSTRUE:  The conversion has been performed successfully.  *i has been
               defined (see below).
      OSFALSE: It was not possible to perform the conversion because <f> was
               too large (for positive values) or too small (for negative
               values) to be converted to the integral representation.  *i
               has not been changed (see below).

----------------------------------------------------------------------------

osbool                  // value can be converted?
RoundFloatToIntegral    // converts a <float> to an <int>, rounding it
(float  f,              // I: value to convert
 int *  i);             // O: converted value

osbool                  // value can be converted?
RoundFloatToIntegral    // converts a <float> to a <long>, rounding it
(float   f,             // I: value to convert
 long *  i);            // O: converted value

   Converts <f> to *i as described above.  This function is the error-safe
and platform-independent version of *i=static_cast<int>(f) or
*i=static_cast<long>(f) .


----------------------------------------------------------------------------
2.8 - Characters and strings
----------------------------------------------------------------------------

   <osrtl> allows to convert characters or strings from ANSI to UNICODE and
vice versa.  Normally, such conversions will not be necessary, because
<osrtl> is completely based on the use of UNICODE characters and strings.
Some applications may, however, need to work with old-style ANSI characters.
To support such applications, conversions from ANSI to UNICODE and vice
versa are provided.
   <osrtl> also allows applications to work with strings in an easy way.
To this end, it supplies such standard functions as checking if a character
is a whitespace character, determining the length of a string, comparing two
strings for equality or inequality, copying one string to another, and
creating a new duplicate of a string on the heap.

----------------------------------------------------------------------------

osbool            // character can be converted?
CharConvert       // converts an ANSI character to a UNICODE character
(char       a,    // I: ANSI character
 wchar_t *  u);   // O: UNICODE character

osbool            // character can be converted?
CharConvert       // converts a UNICODE character to an ANSI character
(wchar_t  u,      // I: UNICODE character
 char *   a);     // O: ANSI character

   The first form of this function converts an ANSI (8-bit) character to a
UNICODE (16-bit) character; the second form performs the inverse conversion.
Both forms convert the null character of the source character set into the
null character of the target character set.  Both forms indicate their
success or failure by means of their return value.  There is only one
possible reason for a conversion failure: the target character set does not
contain a character that is equivalent to the source character.  If the
conversion cannot be made, the target character is not modified.
   Use <StringConvert> (below) to convert an entire string.

----------------------------------------------------------------------------

osbool                  // string can be converted?
StringConvert           // converts an ANSI string to a UNICODE string
(char const *  a,       // I: ANSI string
 wchar_t *     u);      // O: UNICODE string

osbool                  // string can be converted?
StringConvert           // converts a UNICODE string to an ANSI string
(wchar_t const *  u,    // I: UNICODE string
 char *           a);   // O: ANSI string

   The first form of this function converts an ANSI (8-bit) string to its
UNICODE (16-bit) equivalent; the second form performs the inverse
conversion.  Both forms assume that the target string is large enough to
hold the converted copy of the source string, including the terminating null
character (the null character of the source character set which terminates
the source string is converted into the null character of the target
character set that terminates the target string).  Both forms indicate their
success or failure by means of their return value.  There is only one
possible reason for a conversion failure: the target character set does not
contain a character that is equivalent to the source character.
   The characters of the source string are converted one by one, starting at
the beginning of the strings.  The conversion process proceeds until the
terminating null character has been converted, or until a character is
encountered that cannot be converted to the target character set.  This
means that, if the function indicates that it cannot convert the source
string, part of the target string has been overwritten by those characters
that could be converted.  The partly defined target string is not terminated
by a null character.  Effectively, this means that the target string cannot
be used if the function fails.

----------------------------------------------------------------------------

osbool             // whether the character is whitespace
IsCharWhiteSpace   // checks if a character is whitespace
(wchar_t  ch);     // I: character to check

   Checks if <ch> is a whitespace character.  This function is the platform-
independent version of <isspace>.

----------------------------------------------------------------------------

unsigned long           // number of characters in a string
StringLength            // determines the number of chars in a string
(wchar_t const *  s);   // I: string

unsigned long           // number of characters in a string
StringLength            // determines the number of chars in a string
(char const *  s);      // I: string

   These functions count the number of characters in a string.  The string
must be null-terminated.  The terminating null character is not included in
the character count.  The "length" of a null string
( static_cast<wchar_t const *>(OSNULL) or
static_cast<char const *>(OSNULL) ) is 0 .
   These functions are the platform-independent versions of <strlen>.
   <osrtl> also supplies functions called <StringLength> that determine the
length that a numeric value would have if it were converted to a string
representation.  These functions are described in chapter 2.9 (Numbers and
strings).

----------------------------------------------------------------------------

wchar_t *                 // destination string
StringCopy                // copies a string to another string
(wchar_t *        dst,    // O: destination string
 wchar_t const *  src);   // I: source string

   Copies a source string to a destination string.  The source string must
be null-terminated.  The terminating null will also be copied to the
destination string.  The function assumes that the destination string is
large enough to contain all characters of the source string, including the
terminating null character.  The return value is <dst>.
   This function is the platform-independent version of <strcpy>.

----------------------------------------------------------------------------

inline
wchar_t *                     // duplicate of the string, on the heap
StringDuplicate               // creates a copy of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the duplicate
    wchar_t const *  s);      // I: string to copy
#else
   (wchar_t const *  ,        // -: description of the duplicate
    wchar_t const *  s);      // I: string to copy
#endif

inline
osresult                      // result code
StringDuplicate               // creates a converted duplicate of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the converted duplicate
    char const *     a,       // I: string to duplicate and convert
    wchar_t * *      u);      // O: converted duplicate of the string
#else
   (wchar_t const *  ,        // -: description of the converted duplicate
    char const *     a,       // I: string to duplicate and convert
    wchar_t * *      u);      // O: converted duplicate of the string
#endif

inline
osresult                      // result code
StringDuplicate               // creates a converted duplicate of a string
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,   // I: description of the converted duplicate
    wchar_t const *  u,       // I: string to duplicate and convert
    char * *         a);      // O: converted duplicate of the string
#else
   (wchar_t const *  ,        // -: description of the converted duplicate
    wchar_t const *  u,       // I: string to duplicate and convert
    char * *         a);      // O: converted duplicate of the string
#endif

   These functions allocate space on the heap for a copy of the source
string.  If the necessary space could be allocated, they copy the source
string to the allocated space.  The last two forms of this function convert
the string format from ANSI to UNICODE (second form) or vice versa (third
form) while copying.  In all cases, the source string must be terminated by
a null character; this terminating null character is copied to the newly
allocated destination string, possibly after conversion (second and third
form of this function).
   If the source string is a null pointer, no memory will be allocated on
the heap; instead, the output string will also be a null pointer.
   In debugging mode, the functions take an argument that contains a
description of the copy of the source string.  This description can be used
to facilitate debugging.
   The return value indicates the result of the operations.  In the first
form of the function, the return value has the following meaning:

      Source string   Return value   Meaning
      ----------------------------------------------------------------------
      non-null        non-null       Success.  Return value points to a copy
                                     of the source string on the heap.
      non-null        null           Failure.  Not enough free space on the
                                     heap to allocate a copy of the string.
      null            non-null       (impossible)
      null            null           Success.  There was no string to copy,
                                     so no copy was allocated on the heap.

In the second and third form of the function, the return value has the
following meaning:

      Return value            Meaning
      ----------------------------------------------------------------------
      OSRESULT_OK             Success.  The copy could be created (if the
                              string was not a null pointer - see below)
                              and the conversion could be done.
      OSERROR_NOMEMORY        There was not enough free space on the heap to
                              allocate a copy of the source string.
      OSERROR_NOCHARCONVERT   The source string could not be converted.
                              Refer to <StringConvert>, above, for possible
                              reasons why a string cannot be converted.  No
                              copy is allocated on the heap.

If the function is successful, the caller is responsible for the eventual
de-allocation of the copy of the string.
   If the source string of the second and third versions of this function
was a null pointer, no memory will be allocated on the heap; instead, the
destination string will be a null pointer too, and the functions will return
<OSRESULT_OK>.
   These functions are the platform-independent and extended equivalents of
the non-ANSI, but well-known, function <strdup>.

----------------------------------------------------------------------------

osbool                   // strings are equal?
StringEqual              // checks if two strings are equal
(wchar_t const *  s1,    // I: string to compare
 wchar_t const *  s2);   // I: string to compare

   Checks if the two strings are equal.  They are if both pointers are
static_cast<wchar_t const *>(OSNULL) , or if the pointers point to strings
that contain the same characters up to and including the L'\0' .
   Both string must be terminated by a null character.
   This function is the platform-independent equivalent of the expression

      strcmp(s1, s2) == 0

----------------------------------------------------------------------------

inline
osbool                   // strings are different?
StringDifferent          // checks if two strings are different
(wchar_t const *  s1,    // I: string to compare
 wchar_t const *  s2);   // I: string to compare

   Checks if the two strings are different.  This function expands to

      !StringEqual(s1, s2)

Refer to <StringEqual>, above, for a description of the implemented
functionality.
   Both string must be terminated by a null character.
   This function is the platform-independent equivalent of the expression

      strcmp(s1, s2) != 0


----------------------------------------------------------------------------
2.9 - Numbers and strings
----------------------------------------------------------------------------

   <osrtl> allows to convert numbers to their string representations.  To
facilitate such conversions, it supplies two functions:

      <StringLength> computes the length of the string that is needed to
         convert a number to its string representation.  For example,
         StringLength(999)==3 , StringLength(1000)==4 ,
         StringLength(0)==1 , StringLength(-42)==3 .
      <ValueToString> converts a numeric value to its string representation.

   <osrtl> also supplies functions called <StringLength> that count the
number of characters in an ordinary text string.  These functions are
described in chapter 2.8 (Characters and strings).

----------------------------------------------------------------------------

unsigned long             // length of string
StringLength              // finds length of string representation of number
(int  value);             // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(unsigned int  value);    // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(long  value);            // I: number

unsigned long             // length of string
StringLength              // finds length of string representation of number
(unsigned long  value);   // I: number

   These functions determine the length, in characters, that <value> would
have if it were converted to its string representation.  The terminating
null character of the string representation of <value> is not included in
the string length.
   <StringLength> assumes

      - that positive values are represented without a leading plus sign
        L'+' ;
      - that negative values are represented with a single leading minus
        sign L'-' ;
      - that the string representation of <value> does not contain any
        spaces;
      - that the string representation of <value> does not contain any
        separators (like thousands separators).

Examples:

      StringLength(0)    = StringLength("0")    = 1
      StringLength(1)    = StringLength("1")    = 1
      StringLength(999)  = StringLength("999")  = 3
      StringLength(1000) = StringLength("1000") = 4
      StringLength(42)   = StringLength("42")   = 2
      StringLength(-0)   = StringLength(0)
      StringLength(-1)   = StringLength("-1")   = 2
      StringLength(-42)  = StringLength("-42")  = 3
      StringLength(+5)   = StringLength(5)

   <osrtl> also supplies functions called <StringLength> that count the
number of characters in an ordinary text string.  These functions are
described in chapter 2.8 (Characters and strings).
   Use <ValueToString>, below, to convert <value> to its string
representation.

----------------------------------------------------------------------------

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(int        value,        // I: number to convert
 wchar_t *  s);           // O: resulting string

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(unsigned int  value,     // I: number to convert
 wchar_t *     s);        // O: resulting string

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(long       value,        // I: number to convert
 wchar_t *  s);           // O: resulting string

wchar_t *                 // resulting string
ValueToString             // converts a number to its string representation
(unsigned long  value,    // I: number to convert
 wchar_t *      s);       // O: resulting string

   Converts <value> to its string representation and stores the result in
<s>.  Returns <s>.  <s> will be terminated by a null character.
   <ValueToString> assumes that <s> is large enough to hold the string and
the terminating null character.
   Use <StringLength>, above, to determine the length of the string that is
required to hold the string representation of <value>.


----------------------------------------------------------------------------
2.10 - Heap management
----------------------------------------------------------------------------

   <osrtl> provides a heap class <osheap> that allows to allocate objects of
any type (including the built-in standard types like <int>) on the heap
(free store), and to de-allocate these objects.  Both single objects and
arrays of any size can be allocated, as long as there is sufficient free
space on the heap.
   <osheap> implements a safe heap.  It is not possible to de-allocate an
object that was never allocated, or to de-allocate an object more than once.
Just before the program terminates, <osrtl> checks if all objects that were
allocated have properly been de-allocated.  If not, it de-allocates those
objects that are still allocated.  If <osrtl> was compiled in debugging
mode, it warns the developer about each object that was allocated but not
de-allocated.

----------------------------------------------------------------------------

template <class datatype>
class osheap                // checked heap
{
   //...
};  // osheap<datatype>

   Implements a heap for objects of type <datatype>.  The heap behaves as
indicated above.
   Though not indicated in the above definition, an <osheap> is derived from
an <osobject> (refer to chapter 2.6 (Base class)).  By default, the
constructor of each <osheap> sets the name of the <osheap> to L"<osheap>"
using osobject::SetName; this facilitates debugging.  Functions that
instantiate an <osheap> may use <SetName> to change the name of the
<osheap>.

----------------------------------------------------------------------------

template <class datatype>
inline
datatype *                       // allocated data
osheap<datatype>::Allocate       // allocates data on the heap
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  descr,      // I: description of the data elements
    unsigned long    n = 1UL);   // I: number of data elements to allocate
#else
   (wchar_t const *  ,           // -: description of the data elements
    unsigned long    n = 1UL);   // I: number of data elements to allocate
#endif

   Allocates an array of <n> elements of type <datatype>, and returns a
pointer to the first of the allocated elements if successful.  Returns
static_cast<datatype *>(OSNULL) if there was not enough memory on the heap
to do the allocation.  In debugging mode, the allocated elements are given
a description <descr>.  This description is used to inform the developer
about problems on the heap, such as not de-allocating allocated elements.
   <Allocate> is an inline function; it expands to a call to another
function that implements the functionality of <Allocate>.  Inlining
<Allocate> allows the compiler to optimise away the parameter <descr> and
the literal string that is assigned to <descr> if <osrtl> is compiled in
release mode.
   It is allowed to instantiate a heap of a given type, to allocate some
data using the instantiated heap object, and then to destroy the heap
object without having de-allocated the objects that were allocated.  Of
course, the allocated objects must be de-allocated before the program
terminates; this can be done by instantiating another <osheap>.  This
approach is is illustrated in the following example (without error handling
to keep the example as concise and clear as possible):


      void
      InitSomething
      (int * &  one,
       int * &  ten)

      {  // InitSomething

         osheap<int>  heap;   // heap is used solely for allocation

         one = heap.Allocate(L"One element.");
         ten = heap.Allocate(L"Ten elements.", 10);

      }  // InitSomething


      void
      CloseDownSomething
      (int *  one,
       int *  ten)

      {  // CloseDownSomething

         osheap<int>  heap;   // heap is used solely for de-allocation

         heap.Deallocate(one);
         heap.Deallocate(ten);

      }  // CloseDownSomething


      void
      DoEverything
      ()

      {  // DoEverything

         int *  single;    // pointer to a single <int>
         int *  tenfold;   // pointer to an array of ten <int>s

         InitSomething(one, ten);
         // Do useful things.
         CloseDownSomething(one, ten);

      }  // DoEverything


The heap in <InitSomething> is used solely for allocation: the objects that
are allocated using that heap are not de-allocated before the end of the
lifetime of the heap object.  In <CloseDownSomething>, another heap object
is instantiated; that heap object is used solely for de-allocation, and the
objects that are de-allocated using that heap object were never allocated
using that same heap object.  Therefore, we must say that objects are
allocated _via_, or _using_, an <osheap>, and not _on_ an <osheap>.  In
other words, all <osheap> objects use a single, shared heap to perform their
allocations and de-allocations.
   Use osheap<datatype>::Deallocate to de-allocate objects that were
allocated using osheap<datatype>::Allocate .  The data types <datatype> of
<Allocate> and <Deallocate> must be identical.

----------------------------------------------------------------------------

template <class datatype>
osresult                       // result code
osheap<datatype>::Deallocate   // de-allocates data from the heap
(datatype *  data);            // I: data to de-allocate

   De-allocates the element <data> or the array of elements <data>.  If
<data> is static_cast<datatype *>(OSNULL) , this function does nothing but
return <OSRESULT_OK>.
   Refer to osheap<datatype>::Allocate for a detailed explanation of the
allocation and de-allocation process.


----------------------------------------------------------------------------
2.11 - Obtaining the name of the application
----------------------------------------------------------------------------

   As explained in chapter 2.2 (Initialising, running, and closing down the
application), the application calls <ApplicationName> to obtain the name of
the application; programs that are built on top of <osrtl> _must_ provide an
implementation of <ApplicationName>.
   As a service to the application, <osrtl> allows the application to obtain
the application's name as defined by <ApplicationName>.  This is done by
calling the function <ProgramName>:

      wchar_t const *   // name of the program
      ProgramName       // obtains the name of the program
      ();

<ProgramName> returns a pointer to the copy of the application's name that
was made during the initialisation of <osrtl> (refer to chapter 2.2).  If
<ProgramName> is called by <ApplicationName>, <ProgramName> returns
static_cast<wchar_t const *>(OSNULL) .


----------------------------------------------------------------------------
2.12 - Semaphores
----------------------------------------------------------------------------

   The current version of <osrtl> allows only mutex semaphores.  Mutex
stands for mutual exclusion.  Mutex semaphores, or mutexes, are semaphores
that allow at most one process to be in the critical code section that is
guarded by the mutex.
   To use the mutexes that are supplied by <osrtl>, the user must understand
the concept of access mutexes and the underlying, physical mutexes.
   The access mutexes, <osmutex>, are just a gateway that connects to a
physical mutex.  This is necessary to allow each task to have its own,
private access mutex.  The inter-task communication that is necessary inside
the functions of a mutex is performed by the underlying physical mutex; this
physical mutex is shared by the access mutexes of all involved tasks.
   Each physical mutex has an ID of type <mutexid>.  This ID must be
supplied by the user; it is important to use different IDs when different
physical mutexes are to be used.
   The user, though, does not deal at all with the physical mutexes.  He
just instantiates an <osmutex>, telling it to which physical mutex to
connect by passing the ID of the physical mutex.  The <osmutex> checks if
a physical mutex with the given ID already exists.  If so, the <osmutex>
connects to that physical mutex.  If not, the <osmutex> tries to create a
new physical mutex and set the ID of that physical mutex to the given ID.
If the new physical mutex can be created, the <osmutex> connects to it; if
there was not enough memory to create a new physical mutex, the <osmutex>
indicates failure to its caller.
   This means that a user must perform the following steps:

      1. Instantiate an <osmutex>.
      2. Connect the <osmutex> to a physical mutex.  The physical mutex is
         identified by a user-supplied ID that must be unique throughout
         all threads and/or processes of the entire program.
      3. Use osmutex::EnterSection to enter a critical code section.
      4. Use osmutex::LeaveSection to leave a critical code section.

   Note that the mutex IDs that the user code supplies must be
<MUTEX_APPLICATION> or higher.

----------------------------------------------------------------------------

typedef  unsigned long  mutexid;

   The ID of a physical mutex.  Must be supplied to osmutex::Connect
(described below) when connecting to (and maybe first creating) a physical
mutex.  Must have a value of at least <MUTEX_APPLICATION> (described below).

----------------------------------------------------------------------------

const  mutexid  MUTEX_APPLICATION  = 1000UL;   // first mutex ID for use by
                                               //    the application

   The lowest ID value for a physical mutex that may be used by the
application.  ID values that are lower than <MUTEX_APPLICATION> are reserved
for internal use by <osrtl>.

----------------------------------------------------------------------------

class osmutex:       // mutex semaphore
   public osobject   //    generic object
{
   //...
};  // osmutex

   An access mutex; access mutexes and their relation to physical mutexes
are described at the beginning of this chapter.
   An <osmutex> is publicly derived from an <osobject>.  This means that all
services provided by <osobject> may be used on an <osmutex>.  The services
that are provided by <osobject> are described in chapter 2.6 (Base class).
   The functionality of <osmutex> is described below, under the various
members of <osmutex>.

----------------------------------------------------------------------------

virtual
osmutex::~osmutex   // closes down an <osmutex>
();

   Closes down an access mutex.  If necessary, disconnects the access mutex
from the physical mutex to which it was connected by osmutex::Connect
(described below).

----------------------------------------------------------------------------

virtual
osresult           // result code
osmutex::Connect   // connects the <osmutex> to a physical mutex
(mutexid  id);     // I: ID of the physical mutex

   Connects the access mutex to a physical mutex.  The physical mutex to
which the access mutex must connect is identified by its ID, which must be
at least <MUTEX_APPLICATION>.
   osmutex::Connect checks if a physical mutex with ID <id> already exists.
If a physical mutex with that ID already exists, osmutex::Connect connects
the access mutex to that physical mutex and returns <OSRESULT_OK>.  If no
physical mutex with that ID exists, osmutex::Connect attempts to create a
new physical mutex.  If this fails, osmutex::Connect returns
<OSERROR_NOMEMORY>.  If a new physical mutex can be created,
osmutex::Connect assigns the ID <id> to the new physical mutex, connects the
access mutex to it, and returns <OSRESULT_OK>.
   Once an access mutex has connected to a physical mutex through
osmutex::Connect , it is not possible to connect the same access mutex to a
different physical mutex.  If reconnecting to a different physical mutex is
tried by calling osmutex::Connect again, osmutex::Connect returns
<OSINTERNAL_MUTEXADMIN>.  This happens even if osmutex::Connect is called
with the ID of the physical mutex to which the access mutex is already
connected.
   It is not possible to disconnect the access mutex from the physical mutex
to which it has been connected by osmutex::Connect .

----------------------------------------------------------------------------

virtual
osresult                // result code
osmutex::EnterSection   // waits until safe to enter critical section
();

virtual
osresult                // result code
osmutex::LeaveSection   // indicates: critical section has been left
();

   The function pair osmutex::EnterSection and osmutex::LeaveSection guards
a critical code section.  <EnterSection> is called at the beginning of the
critical section; <LeaveSection> is called at the end.  Together, these two
functions ensure that at most one process can be inside a critical code
section that is guarded by the physical mutex to which the <osmutex> was
connected using osmutex::Connect .
   If <EnterSection> is called while no other process is inside a critical
code section that is guarded by the physical mutex to which the <osmutex>
has been connected, <EnterSection> returns immediately, allowing the caller
to enter its critical section.  If, on the other hand, another process is
inside a critical code section that is guarded by the physical mutex to
which the <osmutex> has been connected, <EnterSection> waits until the other
process has left its critical section (by calling <LeaveSection>); only
then, <EnterSection> returns.  If more than one process is waiting for
<EnterSection> to return, the <EnterSection> that was called by the process
that called <EnterSection> first returns first: <EnterSection> maintains a
FIFO queue of waiting processes.
   If <EnterSection> or <LeaveSection> is called while the <osmutex> has not
been connected to a physical mutex, they return <OSINTERNAL_MUTEXADMIN>.
If the <osmutex> has been connected to a physical mutex, <EnterSection>
always returns <OSRESULT_OK>, regardless of whether <EnterSection> had to
wait until another process left the critical code section.  If
<LeaveSection> is called after <EnterSection> has been successfully called,
<LeaveSection> returns <OSRESULT_OK>.  If <LeaveSection> is called before
<EnterSection> has been called, or if <LeaveSection> is called more than
once without calling <EnterSection>, <LeaveSection> returns
<OSINTERNAL_MUTEXADMIN>.
   The function pair <EnterSection>/<LeaveSection> may be called an
unlimited number of times.


----------------------------------------------------------------------------
2.13 - Playing sounds
----------------------------------------------------------------------------

   <osrtl> allows user code to generate simple sounds.  Only basic sounds
are supported; playing melodies, tunes, or music is not possible.
   To generate a sound, <OsSound> must be called with appropriate
parameters.
   <osrtl> does not generate sounds itself, except if a specific internal
error is detected that cannot be reported to the user via the screen.

----------------------------------------------------------------------------

enum
ossound                // sound type
{
   OSSOUND_ALERT,      // alert user: a nominal (expected) event occurred
   OSSOUND_QUESTION,   // user must answer a question
   OSSOUND_PROBLEM     // notify user of a problem
};  // ossound

   <ossound> is used to indicate a type of sound.  The meanings of the three
possible values of <ossound> are indicated in the comment lines above.
<ossound> is used by the function <OsSound>, which is described below.

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
   The type <ossound> is described above.


----------------------------------------------------------------------------
2.14 - Modifying the shape of the mouse cursor
----------------------------------------------------------------------------

   It is possible to change the shape of the mouse cursor.  This allows user
code to let the mouse cursor look like a hourglass when the program is busy,
or like an arrow when the user is expected to select an item in a window.
Currently, <osrtl> supports two types of mouse cursor:

      - An arrow-like cursor.
        This mouse cursor is typically used when the user is expected to
        make a selection, or to point to something.
      - A cursor that indicates that the system is busy.
        This mouse cursor indicates that the system is busy, and that the
        user must wait until the system has finished its operations.
        Typically, this cursor looks like a hourglass.

----------------------------------------------------------------------------

enum
mousecursortype          // type of mouse cursor
{
   MOUSE_ARROW,          // standard mouse arrow
   MOUSE_BUSY            // "system is busy"
};  // mousecursortype

   This type is used to tell <SelectMouseCursor> which type of mouse cursor
must be selected.  The comments indicate the meanings of each of the
possible values of <mousecursortype>.

----------------------------------------------------------------------------

osresult                   // result code
SelectMouseCursor          // selects a mouse cursor
(mousecursortype  type);   // I: type of the mouse cursor to select

   Selects a mouse cursor of type <type>.
   <SelectMouseCursor> loads the indicated mouse cursor from the system (if
<type> indicates one of the system's mouse cursors) or from the executable
file of the program (if <type> indicates a custom mouse cursor).
   <SelectMouseCursor> is functional only if a mouse device is present.
If no mouse device (or other pointing device) is present,
<SelectMouseCursor> does nothing but return <OSRESULT_OK>.
   Possible return values of <SelectMouseCursor> are:

      - OSRESULT_OK
        The mouse cursor now has the look that was indicated by <type>.
      - OSINTERNAL_MOUSECURSOR
        <type> did not have a valid value, or <type> indicated that one of
        the system's mouse cursors should be used but the system was not
        able to select the indicated system cursor.
      - OSERROR_NORSRC_MCURSOR
        <type> indicated that a custom mouse cursor should be used, and the
        system was not able to load the indicated custom cursor from the
        program's executable file.


----------------------------------------------------------------------------
2.15 - Colors
----------------------------------------------------------------------------

   A color is a supporting entity.  A color as a stand-alone item does not
have any purpose; colors are useful only when applied to "physical" objects
that the user can see, like lines, rectangles, texts, and backgrounds.
   In <osrtl>, colors are defined by specifying the amounts of each of the
primary colors red, green, and blue in the resulting color.  Each of these
components has a certain weight in the resulting color; the weight of each
of the primary colors is specified as a number from 0.0 (inclusive) to 1.0
(inclusive).  The following table indicates the colors that result from
certain combinations of the three primary colors.  rrr stands for the amount
of red, ggg for the amount of green, and bbb for the amount of blue in the
resulting color.

      (rrr, ggg, bbb)   Resulting color
      ------------------------------------
      (0.0, 0.0, 0.0)   Black.
      (1.0, 1.0, 1.0)   Saturated white.
      (0.5, 0.5, 0.5)   Grey.
      (0.8, 0.8, 0.8)   Light grey.
      (0.2, 0.2, 0.2)   Dark grey.
      (1.0, 0.0, 0.0)   Saturated red.
      (0.5, 0.1, 0.1)   Dark greyish red.
      (1.0, 0.2, 0.2)   Light greyish red.

----------------------------------------------------------------------------

class oscolor:       // color
   public osobject   //    generic object
{
   //...
};  // oscolor

   A color.  Refer to the introduction of this chapter, above, and to the
descriptions of the components of <oscolor>, below, for an explanation of
the functionality of <oscolor>.
   <oscolor> is publicly derived from <osobject>.  This means that all
services provided by <osobject> may be used on an <oscolor>.  The services
that are provided by <osobject> are described in chapter 2.6 (Base class).

----------------------------------------------------------------------------

oscolor::oscolor   // initialises an <oscolor>
(float  red,       // I: amount of red   (0..1)
 float  green,     // I: amount of green (0..1)
 float  blue);     // I: amout of blue   (0..1)

   Initialises a color.  Uses osobject::SetName to set the name of the color
object to "<oscolor>"; this facilitates debugging.  Uses oscolor::Define to
sets its color components to <red>, <green> and <blue>.
   The color that results from certain combinations of values of <red>,
<green>, and <blue> is defined in the introduction of this chapter, and in
the description of <Define>, below.

----------------------------------------------------------------------------

oscolor::oscolor          // initialises an <oscolor>
(oscolor const &  src);   // I: source for the <oscolor>

oscolor const &           // copy of the source <oscolor>
oscolor::operator =       // copies a source color
(oscolor const &  src);   // I: source color

   Copies one color to another.  The contents of the color (its "value") are
copied.  The destination color will be identical to the source color.

----------------------------------------------------------------------------

void
oscolor::Define   // defines an <oscolor>
(float  red,      // I: amount of red   (0..1)
 float  green,    // I: amount of green (0..1)
 float  blue);    // I: amout of blue   (0..1)

   Defines a color, setting its color components to <red>, <green>, and
<blue>.  The previous color of the <oscolor> will be lost.
   Before setting the color components of <oscolor>, the input values <red>,
<green>, and <blue> are normalised as follows:

      1. If a component is negative, it is replaced with its absolute value.
      2. If the largest of the three components is larger than one, all
         three components are divided by the value of the largest component.

The table of resulting colors in the introduction of this chapter can be
expanded with the following information:

      (rrr, ggg, bbb)   Resulting color
      -------------------------------------------
      (2.0, 2.0, 2.0)   Same as (1.0, 1.0, 1.0) .
      (5.0, 0.0, 0.0)   Same as (1.0, 0.0, 0.0) .
      (2.0, 0.2, 0.2)   Same as (1.0, 0.1, 0.1) .
      (1.8, 3.0, 0.3)   Same as (0.6, 1.0, 0.1) .

----------------------------------------------------------------------------

inline
float        // amount of red (0..1)
oscolor::R   // obtains the amount of red in the color
()
const;

inline
float        // amount of green (0..1)
oscolor::G   // obtains the amount of green in the color
()
const;

inline
float        // amount of blue (0..1)
oscolor::B   // obtains the amount of blue in the color
()
const;

   These functions obtain the amount of the respective primary colors in the
<oscolor>.  The amounts that are returned are the amounts that result after
possible corrections as defined under oscolor::Define , above, have been
applied.  This means that the returned values are guaranteed to be in the
range from 0.0 (inclusive) to 1.0 (inclusive).


----------------------------------------------------------------------------
2.16 - The window painter
----------------------------------------------------------------------------

   To display information in a window, an application must use a window
painter.  A window painter is an abstract object that is not used or
modified directly by the application's code.  It contains information about
the window that must be painted.
   Whenever (part of) a window must be painted, the window manager supplies
the application with a reference to a window painter.  The applications's
code must specify that window painter when it commands the window manager to
paint the window.  This procedure is explained in detail in chapter 2.18
(Window management).

----------------------------------------------------------------------------

typedef  void *  ospainter;    // window painter

   An <ospainter> is an abstract window painter.  It exists to avoid
exposing the true nature (type) of a window painter to other units.


----------------------------------------------------------------------------
2.17 - Fonts
----------------------------------------------------------------------------

   Text is displayed using a certain font.  Before a text can be displayed,
a font must be created; then, the text can be displayed using the font.
   A (text) font is an abstract object that is not used or modified directly
by the user code.  Instead, a font is managed exclusively through the window
manager that is described in chapter 2.18 (Window management).

----------------------------------------------------------------------------

class oswindowfont:   // window font
   public osfont      //    generic font
{
   //...
};  // oswindowfont

   A text font.  This is an abstract data type that is managed exclusively
through the interface that <oswindow> provides, as described in the
introduction of this chapter (<oswindow> is described in chapter 2.18
(Window management)).
   <oswindowfont> is publicly derived from <osobject>.  This means that
osobject::SetName may be used to assign a name to the <oswindowfont> to
facilitate debugging.  By default, each <oswindowfont> assigns the name
L"<oswindowfont>" to itself.


----------------------------------------------------------------------------
2.18 - Window management
----------------------------------------------------------------------------

   All graphical interaction between the user and the application is routed
through windows on the screen.  A window is a rectangular area on the screen
that has borders on all sides.  The edges (or borders) of a window are
horizontal or vertical.  These characteristics of the windows that <osrtl>
supports imply, for example, that it is not possible to create a circular
window, or a rectangular window with sides that are not parallel to the
edges of the screen.
   A window may or may not have a name, or title.  If a window has a name,
the name is displayed inside the top border of the window.  In general, this
means that the top border of a window that has a name may be higher than the
top border of a window that doesn't have a name.  In general, however,
heights and widths of window borders are determined by the operating system.
   An application has access to the window area only.  The window area is
the part of the window inside the window's borders, not including the
borders themselves.  The window area is where the application displays its
information; the borders are controlled by the operating system.
   An application that wishes to display a window on the screen must
instantiate an <oswindow>.  It then uses oswindow::Create to create the
window and display it on the screen.  It may use the other member functions
of <oswindow> to perform other actions, like creating a window font in order
to write text to the window.
   In order to properly and effectively use <oswindow>s, a programmer must
understand the philosophy of <oswindow>s and their interaction with the
underlying operating system.  One basic concept is that sometimes the
application controls the window, for example when the application uses
<Create> to create a new window and display it, while at other times the
window controls the application, for example when requesting the application
to redraw (part of) the contents of the window.
   Normally, the application controls the window.  The window controls the
application in three cases only:

      - To command the window to perform a certain action.
        To do this, the operating system calls one of the command functions.
      - To inform the window about events that occurred.
        To do this, the operating system calls one of the signal functions.
      - To ask the window to supply information.
        To do this, the operating system calls one of the inquiry functions.

The command functions, the signal functions, and the inquiry functions are
called by the operating system, or, if you like, by the <oswindow>.  It is
important to understand that the application's code must NOT call any of
these three categories of functions directly.  The application's code must
implement these functions in such a way that they respond correctly when
they are called by the operating system.
   Bearing this philosophy in mind, we can now list all functionality of an
<oswindow>.  The following table supplies an overview of the categories of
member functions.  In the headers of the table, the caller is indicated as
either AC, which means that the Application's Code calls the member
functions of that category, or as OS, which means that the Operating System
calls the member functions of that category.

      Life cycle of the <oswindow>; AC
         Create
         Destroy

      Handling text; AC
         CreateWindowFont
         DestroyWindowFont
         FontHeight
         TextWidth

      Controlling the window; AC
         Update

      Obtaining information about the <oswindow>; AC
         Width
         Height
         Name
         Visible

      Paint functions; AC, but only in response to a call to <Paint>
         Write
         Line
         OutlineRectangle
         FilledRectangle

      Signal functions; OS
         WindowCreation
         WindowDestruction
         WindowPosSize
         WindowActivation
         WindowDeactivation
         MousePosition
         MouseLeftSelect
         MouseLeftActivate
         MouseLeftRelease
         MouseRightSelect
         MouseRightActivate
         MouseRightRelease

      Inquiry functions; OS
         CanBeClosed
         SizeLimits

      Command functions; OS
         Paint

   The application must never directly draw to the window.  Instead, if the
application wishes to update the contents of (part of) the window area, it
must call <Update> to inform the operating system that the contents of (part
of) the window area must be repainted.  After calling <Update>, the
application's code simply continues its normal activities.  In the meantime,
the operating system calls <Paint>, indicating which part of the window area
must be painted.  Note that the window area that is given to <Paint> may
differ from the window area that was handed to <Update>, because the
operating system may combine several areas that were handed to <Update>, and
because the operating system may decide that part of the window area needs
repainting even if the application did not explicitly indicate so (for
example, because a window from another application covered an <oswindow>,
and the other window was moved or destroyed so that the <oswindow> became
visible after having been covered).
   When the operating system calls <Paint>, the application's code is
required to repaint at least the area that was handed to <Paint>.  The
application may repaint more than the area that was handed to <Paint>, but
this is not recommended since it takes more time than necessary.  To paint
the area, the application may call any of the paint functions <Write>,
<Line>, <OutlineRectangle>, and <FilledRectangle>.  The paint functions may
be called only in response to a call to <Paint>, and they must be called
before returning from <Paint>.  The paint functions must be passed a window
painter (refer to chapter 2.16 (The window painter)); this must be the
window painter that was given to <Paint> in the current call of <Paint>; it
must not be a window painter that was given to <Paint> in a previous call;
it must also not be a window painter that was obtained in any other way.
   If painting part of the window area involves writing text to the window,
the application must call <CreateWindowFont> to create a font for writing
the text.  A font that is created by <CreateWindowFont> must eventually be
destroyed by calling <DestroyWindowFont>.  <FontHeight> and <TextWidth> may
be used to obtain information about the font or the texts that are written
using the font.
   As opposed to window painters, window fonts may be "remembered" by a
window.  That is, it is allowed to create a font in response to a call of
<Paint>, for example, in response to the first call of <Paint> (note that
a font can only be created in response to a call of <Paint> because
<CreateWindowFont> needs a window painter in order to be able to create the
font), and then to store the font for later re-use.  When text must be
written to the window in response to a later call of <Paint>, it is not
necessary to create the font again; instead, the "remembered" font can be
supplied in the call to <Write>.
   When "remembering" fonts, applications should trade speed against usage
of system resources.  Depending on the underlying operating system, creating
a font may be a slow process, and a font may consume a large amount of
system resources.  An application that creates a font only when it needs to
write text to a window and that destroys the font as soon as the text has
been written may be slow, especially if it writes several texts using
different fonts, but it needs few resources.  An application that creates
its fonts the first time they are needed and that remembers those fonts
until the window is destroyed may be fast, but it may require huge amounts
of system resources, especially if lots of windows exist at the same time
and if each of those windows remembers a number of fonts.  It is even
possible that some of the windows are not able to create the fonts they need
because the system runs out of resources.
   Each of the application's windows must be ready to receive inputs from
the operating system at any time, that is, asynchronously with the
application's normal activities.  The operating system may call one of the
signal functions to inform the <oswindow> that something has happened; it
may call one of the inquiry functions to request the window to hand some
information to the operating system; and it may call one of the command
functions to command the window to perform some action.  The response that
is expected from a window when a signal function, an inquiry function, or a
command function is called, is described in the detailed explanation of the
member functions, below.

   However, before we can discuss the members of <oswindow>, we must explain
the co-ordinate systems that <oswindow> uses.  These co-ordinate systems
play a central role in indicating positions and sizes of objects within a
window, or positions and sizes of windows on the screen.
   <osrtl> uses the following co-ordinate systems:

      Code  Meaning             Range
      ------------------------------------
      AS    Available Screen    0.0 to 1.0
      FS    Full Screen         0.0 to 1.0
      WA    Window Area         0.0 to 1.0

In each of these co-ordinate systems, the co-ordinates are expressed as
floating-point numbers, which normally range from 0.0 (inclusive) to
1.0 (exclusive).  Lower and higher co-ordinate values are possible too; they
indicate positions that are outside the normal area that is covered by the
respective co-ordinate system.
   As already explained, a window consists of the so-called window area,
surrounded by the window borders.  The window area is the part of the window
that is available to the application.  Here, all the window's information is
displayed.  The window borders are used by the system to indicate the extent
of the window.  They can be used by the user to move and resize the window.

----------------------------------------------------------------------------

AS    Available Screen    0.0 to 1.0
------------------------------------

   This co-ordinate system covers the part of the screen that is available
to the window area when the borders of the window are entirely within the
confines of the screen.  In other words, given the widths of the left and
rights borders and the heights of the top and bottom borders, the AS system
covers the following area:

      Left:   A distance L to the right of the left edge of the screen,
              where L is equal to the width of the left border of the
              window.
      Right:  A distance R to the left of the right edge of the screen,
              where R is equal to the width of the right border of the
              window.
      Top:    A distance T below the top edge of the screen, where T is
              equal to the height of the top border of the window.
      Bottom: A distance B above the bottom edge of the screen, where B is
              equal to the height of the bottom border of the window.

   The origin of the AS system is in the upper-left corner of this area.
The x axis extends to the right, the y axis extends to the bottom.  The
point x==y==0.0 , the origin of the AS system, is the point that lies a
distance L to the right of the left edge of the screen (L is described
above) and a distance T below the top edge of the screen (T is described
above).  The point x==y==1.0 is the point that lies a distance R to the left
edge of the screen (R is described above) and a distance B above the bottom
edge of the screen (B is described above).  The point x==y==0.0 lies just
inside the available area of the screen (that is, 0.0 is inclusive for both
axes); the point x==y==1.0 lies just outside the available area of the
screen (that is, 1.0 is exclusive for both axes).

----------------------------------------------------------------------------

FS    Full Screen    0.0 to 1.0
-------------------------------

   This co-ordinate system covers the entire screen.  The origin of the FS
system is the upper-left corner of the screen.  The x axis extends to the
right, the y axis extends to the bottom.  The point x==y==0.0 , the origin
of the FS system, corresponds to the upper-left corner of the screen; this
point lies just inside the boundaries of the screen, that is, 0.0 is
inclusive for both axes.  The point x==y==1.0 corresponds to the lower-right
corner of the screen; this point lies just outside the boundaries of the
screen, that is, 1.0 is exclusive for both axes.

----------------------------------------------------------------------------

WA    Window Area    0.0 to 1.0
-------------------------------

   This co-ordinate system covers the so-called window area, that is, the
part of the window that is available to the application.  The window area
is the part of the window between the window's borders.
   The origin of the WA system is the upper-left corner of the window area.
The x axis extends to the right, the y axis extends to the bottom.  The
point x==y==0.0 , the origin of the WA system, corresponds to the upper-left
corner of the window area.  This point lies just inside the window area: 0.0
is inclusive for both axes.  The point x==y==1.0 corresponds to the lower-
right corner of the window area.  This point lies just outside the window
area; it corresponds to the point where the left edge of the right border
and the top edge of the bottom border meet.  This means that 1.0 is
exclusive for both axes.
   Note that the WA system is a relative rather than an absolute system.
The position of the origin of the WA system depends on the position of the
window on the screen.  If an object has a size of, say, 0.5 in the WA
system, its absolute size (relative to the screen) depends on the size of
the window.

----------------------------------------------------------------------------

class oswindow    // generic window
{
   //...
};  // oswindow

   A generic window.  <oswindow> provides the functionality that is
described in the introductory part of this chapter, above.  Refer to the
descriptions of the components of <oswindow> for a detailed explanation of
the functionality of these components and the interrelationship between the
components.
   Though not explicitly indicated in the definition of <oswindow> above,
<oswindow> is publicly derived from <osobject>.  That means that an
<oswindow> may be used as if it were an <osobject>, and that a name can be
assigned to an <oswindow> using osobject::SetName (debugging mode only).

----------------------------------------------------------------------------

virtual
osresult                    // result code
oswindow::Create            // creates an <oswindow>
(float            left,     // I: x co-ordinate of left   edge, AS
 float            right,    // I: x co-ordinate of right  edge, AS
 float            top,      // I: y co-ordinate of top    edge, AS
 float            bottom,   // I: y co-ordinate of bottom edge, AS
 wchar_t const *  name,     // I: name of the window
 osbool           vis);     // I: must the window be visible?

Category: Life cycle of the <oswindow>.

   Creates a new <oswindow>.  The four co-ordinates <left>, <right>, <top>
and <bottom> are the co-ordinates of the window area.  If <name> specifies a
name (that is, if it is not static_cast<wchar_t const *>(OSNULL) ), the
window will have a top border that is high enough to contain the name <name>
(the top border is not necessarily wide enough to contain <name>; the width
of the top border depends on the values of <left> and <right>).  If <name>
does not specify a name (that is, if it is
static_cast<wchar_t const *>(OSNULL) ), the window will have a top border
that may be so small that it cannot display a name, depending on the under-
lying operating system.  If name==L"" (the empty string), the window will
have a top border that is high enough to be used later to display a name,
but the top border will initially not display a name.
   If <Create> is used on a window that already exists, it returns an error.

----------------------------------------------------------------------------

virtual
osresult                     // result code
oswindow::Destroy            // destroys an <oswindow>
(osbool  forced = OSFALSE);  // I: force destruction?

Category: Life cycle of the <oswindow>.

   Destroys an <oswindow>.
   <forced> indicates if the window is destroyed even if it indicates that
is should not be destroyed.  Normally, if forced==OSFALSE , <Destroy> uses
<CanBeClosed> to ask the window if it is OK to destroy that window; the
window is destroyed only if <CanBeClosed> allows this.  If, however,
forced==OSTRUE , <CanBeClosed> is not asked for consent, and the window is
destroyed unconditionally.
   In both cases, <Destroy> calls <WindowDestruction> to inform the window
that it is about to be destroyed; <WindowDestruction> is called before the
destruction is initiated.
   It is allowed to use <Destroy> on a window that has already been des-
troyed.  The reason for allowing to "destroy" a non-existing window is that
some other process that is running on the system may have destroyed the
window without the program being aware of this fact.

----------------------------------------------------------------------------

osresult                       // result code
oswindow::CreateWindowFont     // creates a window font
(wchar_t const *    name,      // I: name of the font
 float              vsize,     // I: vertical size, WA
 float              hsize,     // I: horizontal size; 1.0==normal
 ospainter const &  painter,   // I: window painter
 oswindowfont &     font)      // O: window font
const;

Category: Handling text.

   Creates a window font.  The window font may be used by the text-handling
functions of <oswindow>.
   The looks of the font are primarily determined by the font name <name>.
If name==static_cast<wchar_t const *>(OSNULL) , a default font is selected.
   The font has a height of zero if vsize==0.0 ; the font is as large as
the height of the window if vsize==1.0 .
   The width-to-height ratio of the font is normal if hsize==1.0 ; the font
is condensed (narrow and tall) if 0.0<hsize<1.0 ; the font is expanded (wide
and low) if hsize>1.0 .
   The caller must supply a window painter for <Create> to perform its
tasks.  This means that a window font can be created only in response to a
call of <Paint> (refer to the introduction of this chapter, above, and to
the description of <Paint>, below).
   It is allowed to create a font more than once without destroying it.
This does not consume system resources other than processing time.
   Use <DestroyWindowFont> to destroy the window font.

----------------------------------------------------------------------------

inline
osresult                      // result code
oswindow::DestroyWindowFont   // destroys a window font
(oswindowfont &  font)        // I: window font
const;

Category: Handling text.

   Destroys a window font that was created by <CreateWindowFont>.
   It is allowed to destroy a font several times without re-creating it.
<DestroyWindowFont> will not do anything when the font has already been
destroyed; it will just return <OSRESULT_OK>.

----------------------------------------------------------------------------

float                          // height of the font, WA
oswindow::FontHeight           // obtains the height of a font
(oswindowfont const &  font)   // I: font
const;

Category: Handling text.

   Returns the height of the <oswindowfont>.  This is the actual height (not
the commanded height) of the font.  The font height is defined as the height
of the top of the font above the baseline of the font.  The height is
usually positive, which means that the top of the highest character of the
font is above the baseline.
   <Height> returns 0.0 if the top of the highest character of the font is
on the baseline; 1.0 if the top of the highest character of the font is at
the top of the window if the baseline is at the bottom of the window; and
-1.0 if the top of the highest character of the font is at the bottom of the
window if the baseline is at the top of the window.

----------------------------------------------------------------------------

osresult                         // result code
oswindow::TextWidth              // obtains the widths of a text string
(oswindowfont const &  font,     // I: font
 ospainter const &     painter,  // I: window painter
 wchar_t const *       text,     // I: text
 float &               left,     // O: width of left margin, WA
 float &               middle,   // O: width of text without margins, WA
 float &               full)     // O: width of text plus margins, WA
const;

Category: Handling text.

   Obtains the three significant widths that a text string would have if it
were written to the screen using the given font and window painter.  A text
string consists of three parts, each of which has its own width:

      1. The left marging (<left>).
         Before drawing the first character of the string, the "pen"
         advances a certain distance relative to its current position.
         This distance is the width of the left margin.  The width of the
         left margin may be positive, zero, or negative.
      2. The width of the text string itself, not including margins
         (<middle>).
         This is the actual width of the text string itself, not taking any
         margin into account.  It is the distance between the leftmost point
         the "pen" will reach while writing the string, and the rightmost
         point the "pen" will reach.
         The width of the text string is zero if the string is empty, or
         positive if the string is not empty.
      3. The right margin.
         After drawing the last character of the string, the "pen" advances
         a certain distance to the right of the rightmost part of the image
         of the string.  This distance is the width of the right margin.
         The width of the right margin may be positive, zero, or negative.k

<full> is identical to <left> plus <middle> plus the width of the right
margin.
   Each of the three text widths is 0.0 if it is zero, and 1.0 if it is as
wide as the entire screen.

----------------------------------------------------------------------------

osresult           // result code
oswindow::Update   // tells the window to update an area
(float  left,      // I: left   edge of the area to update, WA
 float  right,     // I: right  edge of the area to update, WA
 float  top,       // I: top    edge of the area to update, WA
 float  bottom);   // I: bottom edge of the area to update, WA

Category: Controlling the window.

   <Update> must be used when the application wants to write new, updated
information to the window.  It is forbidden to directly paint to the window.
Instead, an application must wait until the operating system calls <Paint>
to indicate that part of the window area must be repainted (refer to the
introductory part of this chapter, above).  To tell the operating system to
call <Paint> soon (not necessarily immediately), an application must use
<Update> to inform the operating system that a certain area of the window
must be regarded as a candidate for repainting.

----------------------------------------------------------------------------

inline
float              // width of the window, AS
oswindow::Width    // obtains the current width of the window
()
const;

inline
float              // height of the window, AS
oswindow::Height   // obtains the current height of the window
()
const;

Category: Obtaining information about the <oswindow>.

   These two functions return the current width and height, respectively, of
the window.  Initially, the width and height are (almost) identical to the
commanded width and height (the width and height may deviate a bit from the
commanded width and height due to rounding effects when computing the
position and dimensions of the window on the physical screen).  Later, the
width and height of the window may change, either as commanded by the
program, or because the user changed the dimensions of the window.

----------------------------------------------------------------------------

inline
wchar_t const *  // the name of the window
oswindow::Name   // obtains the name of the window
()
const;

Category: Obtaining information about the <oswindow>.

   Obtains the current name of the window.  Initially, this is the name that
was passed to <Create>.
   If the name that was passed to <Create> was
static_cast<wchar_t const *>(OSNULL) , the window does not have a name and
will never have a name.  In that case, <Name> will always return
static_cast<wchar_t const *>(OSNULL) .

----------------------------------------------------------------------------

inline
osbool              // is the window visible?
oswindow::Visible   // checks if the window is currently visible
()
const;

Category: Obtaining information about the <oswindow>.

   Checks if the window is currently visible.  Note that both the program
itself and other programs running on the same system may change the window
from visible to invisible and vice versa.
   Initially, <Visible> will return the visibility state as passed to
<Create> when the window was created.

----------------------------------------------------------------------------

virtual
osresult                       // result code
oswindow::Write                // writes text
(float                 x,      // I: x co-ordinate of the text, WA
 float                 y,      // I: y co-ordinate of the text, WA
 wchar_t const *       text,   // I: text to write
 oswindowfont const &  font,   // I: font to use
 oscolor const &       color,  // I: color of the text
 ospainter const &     p);     // I: window painter

Category: Paint functions.

   Writes the text <text> to the window, using the font <font> and the color
<color>.  The text will be written at position (x,y) .  This position is the
left of the left edge of the left margin of the text, on the baseline of the
text.
   <Write> handles single lines only.  If <text> contains newline characters
or other non-printable characters, the result of <Write> is not defined.  If
<text> is so long that it does not fit in the window, or if the co-ordinates
are such that part of the text (or the entire text) is outside the window
area, the text will be clipped at the edges of the window area.
   <Write> must be used only if <Paint> indicates that (part of) the window
needs repainting.  It must be given the window painter (<ospainter>) that
<Paint> received, and it must be called before <Paint> returns.

----------------------------------------------------------------------------

virtual
osresult                     // result code
oswindow::Line               // draws a straight line
(float              x0,      // I: x co-ord of start point (incl.), WA
 float              y0,      // I: y co-ord of start point (incl.), WA
 float              x1,      // I: x co-ord of end   point (excl.), WA
 float              y1,      // I: y co-ord of end   point (excl.), WA
 oscolor const &    color,   // I: color of the line
 ospainter const &  p);      // I: window painter

Category: Paint functions.

   Draws a straight line from (x0,y0) (inclusive) to (x1,y1) (exclusive).
The line will have the color <color>.  It will be as thin as the display
allows without becoming invisibly thin.
   If (part of) the line is outside the window area, the line will be
clipped at the edges of the window area.
   <Line> must be used only if <Paint> indicates that (part of) the window
needs repainting.  It must be given the window painter (<ospainter>) that
<Paint> received, and it must be called before <Paint> returns.

----------------------------------------------------------------------------

virtual
osresult                     // result code
oswindow::OutlineRectangle   // draws the outline of a rectangle
(float              left,    // I: left   edge of the rectangle, WA
 float              right,   // I: right  edge of the rectangle, WA
 float              top,     // I: top    edge of the rectangle, WA
 float              bottom,  // I: bottom edge of the rectangle, WA
 oscolor const &    color,   // I: color of the rectangle
 ospainter const &  p);      // I: window painter

Category: Paint functions.

   Draws the outline of a rectangle, that is, draws only the bounding edge
of a rectangle without touching the rectangle's interior.
   If (part of) the rectangle is outside the window area, the rectangle will
be clipped at the edges of the window area.
   <OutlineRectangle> must be used only if <Paint> indicates that (part of)
the window needs repainting.  It must be given the window painter
(<ospainter>) that <Paint> received, and it must be called before <Paint>
returns.

----------------------------------------------------------------------------

virtual
osresult                      // result code
oswindow::FilledRectangle     // draws a filled rectangle
(float              left,     // I: left   edge of the rectangle, WA
 float              right,    // I: right  edge of the rectangle, WA
 float              top,      // I: rop    edge of the rectangle, WA
 float              bottom,   // I: bottom edge of the rectangle, WA
 oscolor const &    color,    // I: color of the rectangle
 ospainter const &  p);       // I: window painter

Category: Paint functions.

   Draws a filled rectangle, that is, draws both the bounding edge of a
rectangle and the rectangle's interior.  The bounding edge and the interior
will have the same color.
   If (part of) the rectangle is outside the window area, the rectangle will
be clipped at the edges of the window area.
   <FilledRectangle> must be used only if <Paint> indicates that (part of)
the window needs repainting.  It must be given the window painter
(<ospainter>) that <Paint> received, and it must be called before <Paint>
returns.

----------------------------------------------------------------------------

virtual
osresult                   // result code
oswindow::WindowCreation   // the window is being created
(long   ,                  // -: time of window creation
 float  ,                  // -: x position of the mouse, WA
 float  );                 // -: y position of the mouse, WA

Category: Signal functions.

   Informs the window that it is about to be created.  This function is
intended to be overridden by derived classes.  The window will actually be
created only if <WindowCreation> returns <OSRESULT_OK>.
   oswindow::WindowCreation returns <OSRESULT_OK>.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowDestruction   // the window is being destroyed
(long   ,                     // -: time of window destruction
 float  ,                     // -: x position of the mouse, WA
 float  );                    // -: y position of the mouse, WA

Category: Signal functions.

   Informs the window that it is about to be destroyed.  This function is
intended to be overridden by derived classes.  If this function is called,
the window will be destroyed unconditionally, even if it does not want to
be destroyed.  Refer to the introduction to this chapter, above, and to the
description of <Destroy>, below, for an explanation of the destruction
process.
   oswindow::WindowDestruction does nothing.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowPosSize   // position and size of window have changed
(long   ,                 // -: time of change in position and size
 float  ,                 // -: x position of the mouse, WA
 float  );                // -: y position of the mouse, WA

Category: Signal functions.

   Informs the window that either its position or its size has changed, or
that both its position and its size have changed.  This function is intended
to be overridden by derived classes.
   <WindowPosSize> may also be called if neither the position nor the size
of the window have changed.
   oswindow::WindowPosSize does nothing.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowActivation   // the window is being activated
(oswindow *  ,               // I: window that is being de-activated
 long        ,               // I: time of activation
 float       ,               // I: x position of the mouse, WA
 float       );              // I: y position of the mouse, WA

Category: Signal functions.

   Informs the window that it is being activated.  If the window that is
being de-activated is a window of this program, the first parameter
is the window that is being de-activated.  If the window that is being
de-activated belongs to a different application, the first parameter is
static_cast<oswindow *>(OSNULL) .
   This function is intended to be overridden by derived classes.
   oswindow::WindowActivation does nothing.

----------------------------------------------------------------------------

virtual
void
oswindow::WindowDeactivation   // the window is being de-activated
(oswindow *  ,                 // I: window that is being activated
 long        ,                 // I: time of activation
 float       ,                 // I: x position of the mouse, WA
 float       );                // I: y position of the mouse, WA

Category: Signal functions.

   Informs the window that it is being de-activated.  If the window that is
being activated is a window of this program, the first parameter is the
window that is being activated.  If the window that is being de-activated
belongs to a different application, the first parameter is
static_cast<oswindow *>(OSNULL) .
   This function is intended to be overridden by derived classes.
   oswindow::WindowDeactivation does nothing.

----------------------------------------------------------------------------

virtual
void
oswindow::MousePosition  // the position of the mouse has changed
(long    ,               // -: time of the change of the mouse position
 float   x,              // I: new x position of the mouse, WA
 float   y,              // I: new y position of the mouse, WA
 osbool  ,               // -: left  mouse button is down?
 osbool  ,               // -: right mouse button is down?
 osbool  ,               // -: shift key 1 is down?
 osbool  ,               // -: shift key 2 is down?
 osbool  );              // -: shift key 3 is down?

Category: Signal functions.

   Informs the window that the position of the mouse has changed.  This
function is intended to be overridden by derived classes.
   oswindow::MousePosition checks if the new position of the mouse is inside
the window area.  If it is, the mouse cursor is set to the standard arrow by
calling SelectMouseCursor(MOUSE_ARROW) .  Selecting a mouse cursor is
described in chapter 2.14 (Modifying the shape of the mouse cursor).

----------------------------------------------------------------------------

virtual
void
oswindow::MouseLeftSelect   // the left mouse button was used to select
(long    ,                  // -: time of selection with left button
 float   ,                  // -: new x position of the mouse, WA
 float   ,                  // -: new y position of the mouse, WA
 osbool  ,                  // -: left  mouse button is down?
 osbool  ,                  // -: right mouse button is down?
 osbool  ,                  // -: shift key 1 is down?
 osbool  ,                  // -: shift key 2 is down?
 osbool  );                 // -: shift key 3 is down?

Category: Signal functions.

   Informs the window that the left mouse button was used to select a
position in the window.  This function is intended to be overridden by
derived classes.
   Before <MouseLeftSelect> is called, <oswindow> calls <MousePosition>.
   oswindow::MouseLeftSelect behaves as if the left mouse button was used to
select a position in a non-special area of the window's top border.

----------------------------------------------------------------------------

virtual
void
oswindow::MouseLeftActivate   // left mouse button was used to activate
(long    ,                    // -: time of activation with left button
 float   ,                    // -: new x position of the mouse, WA
 float   ,                    // -: new y position of the mouse, WA
 osbool  ,                    // -: left  mouse button is down?
 osbool  ,                    // -: right mouse button is down?
 osbool  ,                    // -: shift key 1 is down?
 osbool  ,                    // -: shift key 2 is down?
 osbool  );                   // -: shift key 3 is down?

Category: Signal functions.

   Informs the window that the left mouse button was used to activate a
position in the window.  This function is intended to be overridden by
derived classes.
   Before <MouseLeftActivate> is called, <oswindow> calls <MousePosition>.
   oswindow::MouseLeftActivate behaves as if the left mouse button was used
to activate a position in a non-special area of the window's top border.

----------------------------------------------------------------------------

virtual
void
oswindow::MouseLeftRelease   // the left mouse button was released
(long    ,                   // -: time of release of left mouse button
 float   ,                   // -: new x position of the mouse, WA
 float   ,                   // -: new y position of the mouse, WA
 osbool  ,                   // -: left  mouse button is down?
 osbool  ,                   // -: right mouse button is down?
 osbool  ,                   // -: shift key 1 is down?
 osbool  ,                   // -: shift key 2 is down?
 osbool  );                  // -: shift key 3 is down?

Category: Signal functions.

   Informs the window that the left mouse button was released.  This
function is intended to be overridden by derived classes.
   Before <MouseLeftRelease> is called, <oswindow> calls <MousePosition>.
   oswindow::MouseLeftRelease behaves as if the left mouse button was
released on a non-special position in the window's top border.

----------------------------------------------------------------------------

virtual
void
oswindow::MouseRightSelect   // the right mouse button was used to select
(long    ,                   // -: time of selection with right button
 float   ,                   // -: new x position of the mouse, WA
 float   ,                   // -: new y position of the mouse, WA
 osbool  ,                   // -: left  mouse button is down?
 osbool  ,                   // -: right mouse button is down?
 osbool  ,                   // -: shift key 1 is down?
 osbool  ,                   // -: shift key 2 is down?
 osbool  );                  // -: shift key 3 is down?

Category: Signal functions.

   Informs the window that the right mouse button was used to select a
position in the window.  This function is intended to be overridden by
derived classes.
   Before <MouseRightSelect> is called, <oswindow> calls <MousePosition>.
   oswindow::MouseRightSelect behaves as if the right mouse button was used
to select a position in a non-special area of the window's top border.

----------------------------------------------------------------------------

virtual
void
oswindow::MouseRightActivate   // right mouse button was used to activate
(long    ,                     // -: time of activation with right button
 float   ,                     // -: new x position of the mouse, WA
 float   ,                     // -: new y position of the mouse, WA
 osbool  ,                     // -: left  mouse button is down?
 osbool  ,                     // -: right mouse button is down?
 osbool  ,                     // -: shift key 1 is down?
 osbool  ,                     // -: shift key 2 is down?
 osbool  );                    // -: shift key 3 is down?

Category: Signal functions.

   Informs the window that the right mouse button was used to activate a
position in the window.  This function is intended to be overridden by
derived classes.
   Before <MouseRightActivate> is called, <oswindow> calls <MousePosition>.
   oswindow::MouseRightActivate behaves as if the right mouse button was
used to activate a position in a non-special area of the window's top
border.

----------------------------------------------------------------------------

virtual
void
oswindow::MouseRightRelease   // the right mouse button was released
(long    ,                    // -: time of release of right mouse button
 float   ,                    // -: new x position of the mouse, WA
 float   ,                    // -: new y position of the mouse, WA
 osbool  ,                    // -: left  mouse button is down?
 osbool  ,                    // -: right mouse button is down?
 osbool  ,                    // -: shift key 1 is down?
 osbool  ,                    // -: shift key 2 is down?
 osbool  );                   // -: shift key 3 is down?

Category: Signal functions.

   Informs the window that the right mouse button was released.  This
function is intended to be overridden by derived classes.
   Before <MouseRightRelease> is called, <oswindow> calls <MousePosition>.
   oswindow::MouseRightRelease behaves as if the right mouse button was
released on a non-special position in the window's top border.

----------------------------------------------------------------------------

virtual
osbool                  // window may be closed?
oswindow::CanBeClosed   // checks if it is OK to close the window
(long   ,               // -: time of the closure request
 float  ,               // -: x position of the mouse, WA
 float  );              // -: y position of the mouse, WA

Category: Inquiry functions.

   Asks the window if it is OK to close the window.  This function is called
if <Destroy> is called with its parameter <forced> set to <OSFALSE>.  The
window must indicate if it can safely be destroyed.  It may ask the user for
permission to close the window.  Refer to the description of <Destroy>,
above, for a detailed explanation of the destruction process.
   This function is intended to be overridden by derived classes.
   oswindow::CanBeClosed returns <OSTRUE>.

----------------------------------------------------------------------------

virtual
void
oswindow::SizeLimits    // ask for size limits for the window
(float &  minwidth,     // O: minimum width  of the window, AS
 float &  maxwidth,     // O: maximum width  of the window, AS
 float &  minheight,    // O: minimum height of the window, AS
 float &  maxheight);   // O: maximum height of the window, AS

Category: Inquiry functions.

   Asks the window to supply limits to its size.  This function is intended
to be overridden by derived classes.
   oswindow::SizeLimits sets the minimum width and height to 0.0 , and the
maximum width and height to 1.0 .

----------------------------------------------------------------------------

virtual
osresult                      // result code
oswindow::Paint               // paints an area of the window
(float              left,     // I: L edge of area that needs painting, WA
 float              right,    // I: R edge of area that needs painting, WA
 float              top,      // I: T edge of area that needs painting, WA
 float              bottom,   // I: B edge of area that needs painting, WA
 long               ,         // -: time of the paint request
 float              ,         // -: x position of the mouse, WA
 float              ,         // -: y position of the mouse, WA
 ospainter const &  p)        // I: window painter
= 0;                          // derived class _must_ provide this function

Category: Command functions.

   Informs the window that (part of) the window area needs repainting.
Derived classes _must_ override this function in order to be useful.  If
<Paint> is called, the indicated area _must_ be repainted for the window to
look normal.  To paint, only the paint functions may be used.  The paint
functions must be called before returning from <Paint>.  When calling a
paint function, <Paint> _must_ pass <p> to the paint function.
   oswindow::Paint makes the entire rectangle black.  It does not take the
function result of <Paint> into account.


----------------------------------------------------------------------------
2.19 - Class tree
----------------------------------------------------------------------------

   The class tree in <osrtl> is built using single inheritance only.  This
makes the class diagram simple and easy to understand.
   As explained at the end of chapter 2 (Using the unit <osrtl>), not all
items of which <osrtl> consists are meant to be exported.  A similar
statement applies to the class tree: the full tree consists of more classes
than are shown in the class diagram.  The classes that are present but that
are not shown in the class diagram are, however, not meant to be used by
other parts of the program.  Therefore, they are irrelevant to the users of
<osrtl>.
   In the class diagram, the base class is drawn at the top.  Derived
classes are drawn below the class from which they are derived.  A line
between two classes indicates a derivation relationship.


                                  osobject
                                     |
           .-----------.----------.--^----------.--------------.
           |           |          |             |              |
        osmutex     osheap     oscolor     oswindowfont     oswindow


All class relationships in this diagram are public; in other words, all
classes at the bottom line are publicly derived from <osobject>.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The header file for the unit <osrtl>, osrtl/osrtl.h , depends on but a
single other file: the "actual" header file for <osrtl> for the current
target platform.  For example, if the version of <osrtl> that is targeted
for platform "pletvorm" is to be used, osrtl/osrtl.h includes only the file
osrtl/pletvorm/osrtl.h .  Refer to the documentation file of the current
target platform ( osrtl/pletvorm/osrtl.d ) for the dependencies of the file
osrtl/pletvorm/osrtl.h .


----------------------------------------------------------------------------
Chapter 4  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/osrtl.d .
----------------------------------------------------------------------------


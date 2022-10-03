
----------------------------------------------------------------------------
---  File:     osrtl/windows/basics/object/object.d
---  Contents: Generic object.
---  Module:   <osrtl><basics><object>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/basics/object/object.d  -  Generic object
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 84 $
---  $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:41:08  erick
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
19970626
19970806
EAS
----------------------------------------------------------------------------
 1.1.001  0. Minor typographic corrections have been made in chapter 2.
19970822  1. osobject::~osobject is no longer described as being a pure
19970822     virtual function.  This corrects a mistake in revision 1.1.000
EAS          of this documentation file.
          2. osobject::DefineName , osobject::RetrieveName , and
             osobject::KillName are no longer described as being virtual
             functions.  This brings this documentation file in line with
             revision 1.1.001 of osrtl/windows/basics/object/object.h . 
----------------------------------------------------------------------------
 1.2.000  Identical to revision 1.1.001.  Formalised revision.
19970825
19970825
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><basics><object>
----------------------------------------------------------------------------

   The module <osrtl><basics><object> supplies a class <osobject> that is
intended to be the root class of all classes of a class hierarchy.  By
publicly deriving all classes from the class <osobject>, each object may
be accessed as if it were an <osobject>.  This makes it easy to create lists
and arrays of objects of any kind.  It also makes it easy to destroy an
object of any kind.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><basics><object>
----------------------------------------------------------------------------

   All other classes in the project should be publicly derived from
<osobject>:

      class derivedclass:    // any class in the project
         public osobject     //    generic object
      {
         // ...              // contents of <derivedclass>
      };  // derivedclass

In debugging mode, each <osobject> (and, hence, each object that publicly or
protectedly derives from <osobject>) may be assigned a name to distinguish
it from other <osobject>s; this name is typically used to generate clear
messages during debugging.  <SetName> assigns a name to an <osobject>,
<ObjectName> returns the name of the <osobject>.  If there is not enough
memory to allocate storage space for the name, or if unit <osrtl> is
compiled in release mode, <SetName> does nothing and <ObjectName> returns
static_cast<wchar_t const *>(OSNULL) .  The object's name is typically used
for debugging purposes: it allows the program to display the object where
a problem was detected.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><basics><object> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Module <osrtl><basics><constant>.
         <object> uses the constant <OSNULL>.

      Module <osrtl><basics><type>.
         <object> uses the type <osbool>.

      Module <osrtl><basics><object>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><basics><object>
----------------------------------------------------------------------------

class osobject   // basic type
{
   //...
};  // osobject

   The root of the hierarchy of classes.  Refer to the descriptions of the
components of <osobject>, below, for an explanation of the workings of this
class type.

----------------------------------------------------------------------------

inline
osobject::osobject   // initialises an <osobject>
();

   In debugging mode, this constructor initialises the name of the object
to static_cast<wchar_t *>(OSNULL) to indicate that the object doesn't have
a name (yet).  In release mode, this constructor does nothing.

----------------------------------------------------------------------------

virtual
osobject::~osobject   // closes down an <osobject>
();

   In debugging mode, this destructor destroys the name of the object, if
the object has a name.  In release mode, this destructor does nothing.

----------------------------------------------------------------------------

inline
osbool                           // object name could be set?
osobject::SetName                // sets the name of the object
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  newname);   // I: new name of the object
#else
   (wchar_t const *  );          // -: new name of the object
#endif

   In debugging mode, <SetName> attempts to assign the name <newname> to the
object.  If the object already has a name, the old name is killed first.
The return value indicates if the new name could be assigned to the object.
If not, the object retains its original name.
   To remove the name of an object, use
SetName(static_cast<wchar_t const *>(OSNULL)) .
   A copy of <newname> is allocated on the heap.  Refer to chapter 5 for the
reasons for doing this, and for a description of the consequences of doing
it this way.  If the allocation fails, the object has no name and <SetName>
returns <OSFALSE>.
   <SetName> merely calls <DefineName> to do the work.  Refer to chapter 5
for a description of the reasons for this set-up.
   In release mode, <SetName> does nothing; it returns <OSTRUE> to indicate
that it successfully did nothing, and the "name" of the object remains
undefined.  Returning <OSTRUE> is done through the static data member
<truetrue> instead of simply returning <OSTRUE> directly.  This is necessary
to prevent some compilers from warning "condition is always true" when they
reach a place where the return value of <SetName> is used in a conditional
expression.

- Use <ObjectName> to obtain the current name of the object.

----------------------------------------------------------------------------

virtual
wchar_t const *        // name of the object
osobject::ObjectName   // retrieves the name of the object
()
const;

   In debugging mode, <ObjectName> returns the name of the object as set by
<SetName>.  In release mode, <SetName> did not set the name of the object,
so <ObjectName> returns static_cast<wchar_t const *>(OSNULL) .
   <ObjectName> merely calls <RetrieveName> to do the work.  Refer to
chapter 5 for a description of the reasons for this set-up.

- Use <SetName> to define the name of the object.

----------------------------------------------------------------------------

osbool                           // object name could be defined?
osobject::DefineName             // defines the name of the object
#ifdef OSRTL_DEBUGGING
   (wchar_t const *  newname);   // I: new name of the object
#else
   (wchar_t const *  );          // -: new name of the object
#endif

   <DefineName> actually performs the actions that <SetName> must perform.
Refer to the description of <SetName> for a description of these actions.
Refer to chapter 5 for the reasons for letting <SetName> just call
<DefineName>.

----------------------------------------------------------------------------

wchar_t const *          // name of the object
osobject::RetrieveName   // retrieves the name of the object
()
const;

   <RetrieveName> actually performs the actions that <ObjectName> must
perform.  Refer to the description of <ObjectName> for a description of
these actions.  Refer to chapter 5 for the reasons for letting <ObjectName>
just call <RetrieveName>.

----------------------------------------------------------------------------

osbool               // whether the name could be killed
osobject::KillName   // kills the name of the object
();

   In debugging mode, <KillName> checks if the object has a name.  If so,
it de-allocates the name from the heap and sets the name of the object to
static_cast<wchar_t *>(OSNULL) .  The return value of <KillName> indicates
if the name of the object was successfully killed.
   In release mode, the object doesn't have a name.  Therefore, <KillName>
does nothing except return <OSTRUE>.

----------------------------------------------------------------------------

#ifdef OSRTL_DEBUGGING
   wchar_t *         // name of the object
   osobject::name;
#endif

   This data member points to the name of the object.  If the object doesn't
have a name, its value is static_cast<wchar_t *>(OSNULL) .


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   <SetName> cannot be sure of the lifetime of the name <newname>.  There-
fore, it must make a local copy of the name.  To be as flexible as possible,
the copy is allocated on the heap.
   Because <osobject> is meant to be the root class of a class hierarchy, it
cannot use the services of other classes.  Therefore, it has to manage the
heap by itself.  Not being able to use other classes also means that it is
not possible to do sophisticated error handling.

   <SetName> uses a string as input.  This string is useful in debugging
mode only.  To allow the compiler not to generate this string in release
mode (that is, to optimise it away), <SetName> must be <inline>.  However,
inlining a large function (<SetName> is quite large in debugging mode) is
not nice.  Therefore, <SetName> is <inline>.  In debugging mode, it merely
calls <DefineName> to do the actual work.  In release mode, it returns
immediately; this allows the compiler to optimise away the entire call to
<SetName>, and to optimise away the (possibly large) constant string that
is passed to <SetName>.

   A similar rationale applies to the set-up of <ObjectName> and
<RetrieveName>.


----------------------------------------------------------------------------
---  End of file osrtl/windows/basics/object/object.d .
----------------------------------------------------------------------------


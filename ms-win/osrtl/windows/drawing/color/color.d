
----------------------------------------------------------------------------
---  File:     osrtl/windows/drawing/color/color.d
---  Contents: Colors.
---  Module:   <osrtl><drawing><color>
----------------------------------------------------------------------------


----------------------------------------------------------------------------
---  RCS information
---
---  @(#) osrtl/windows/drawing/color/color.d  -  Colors
---
---  $RCSfile$
---  $Author: erick $
---  $Revision: 85 $
---  $Date: 1997-12-30 14:49:19 +0100 (Tue, 30 Dec 1997) $
---  $Log$
---  Revision 1.1  1997/12/30 13:49:14  erick
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
19970722
19970824
EAS
----------------------------------------------------------------------------


----------------------------------------------------------------------------
Chapter 1  -  Purpose of the module <osrtl><drawing><color>
----------------------------------------------------------------------------

   The module <osrtl><drawing><color> provides colors.


----------------------------------------------------------------------------
Chapter 2  -  Using the module <osrtl><drawing><color>
----------------------------------------------------------------------------

   All parts of the program can directly use the class <oscolor> that is
provided by the module <osrtl><drawing><color>.  They can freely set the
color by specifying its components in terms of red, green, and blue, and
they can read the current settings of these color components.


----------------------------------------------------------------------------
Chapter 3  -  Dependencies
----------------------------------------------------------------------------

   The module <osrtl><drawing><color> depends on the following units,
components and modules for the indicated reasons:

      Global settings for the project.
         This settings file contains the global settings that apply to all
         units, components and modules of the project.

      Settings file for unit <osrtl>.
         This settings file contains the settings that apply to all
         components and modules in the unit <osrtl>.

      Component <osrtl><basics>.
         <color> uses basic entities such as <osobject>.

      Component <osrtl><error>.
         <color> uses error-handling entities such as <OsDebugErrorMessage>.

      Module <osrtl><drawing><color>.
         The Coding Standard requires every module to include its own
         header file.


----------------------------------------------------------------------------
Chapter 4  -  Detailed explanation of module <osrtl><drawing><color>
----------------------------------------------------------------------------

class oscolor:       // color
   public osobject   //    generic object
{
   //...
};  // oscolor

   A color.  Refer to the descriptions of the components of this class to
understand the functionality of <oscolor>.

----------------------------------------------------------------------------

oscolor::oscolor   // initialises an <oscolor>
(float  red,       // I: amount of red   (0..1)
 float  green,     // I: amount of green (0..1)
 float  blue);     // I: amout of blue   (0..1)

   Initialises a color.  Sets its color components to <red>, <green> and
<blue>.  Sets the name of the object to "<oscolor>" to facilitate debugging.

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
<blue>.  Before setting the color components of <oscolor>, the input values
<red>, <green>, and <blue> are normalised as follows:

      1. If a component is negative, it is replaced with its absolute value.
      2. If the largest of the three components is larger than one, all
         three components are divided by the value of the largest component.

Observe the following examples:

      (rrr, ggg, bbb)   Resulting color
      -------------------------------------------
      (0.0, 0.0, 0.0)   Black.
      (1.0, 1.0, 1.0)   Saturated white.
      (0.5, 0.5, 0.5)   Grey.
      (0.8, 0.8, 0.8)   Light grey.
      (0.2, 0.2, 0.2)   Dark grey.
      (2.0, 2.0, 2.0)   Same as (1.0, 1.0, 1.0) .
      (1.0, 0.0, 0.0)   Saturated red.
      (5.0, 0.0, 0.0)   Same as (1.0, 0.0, 0.0) .
      (0.5, 0.1, 0.1)   Dark greyish red.
      (1.0, 0.2, 0.2)   Light greyish red.
      (2.0, 0.2, 0.2)   Same as (1.0, 0.1, 0.1) .
      (1.8, 3.0, 0.3)   Same as (0.6, 1.0, 0.1) .

----------------------------------------------------------------------------

inline
float        // amount of red (0..1)
oscolor::R   // obtains the amount of red in the color
()
const;

   Returns the amount of red (after correction as described under <Define>,
above) in the color.  This amount is guaranteed to be in the range 0..1 ,
where both limits are inclusive.

----------------------------------------------------------------------------

inline
float        // amount of green (0..1)
oscolor::G   // obtains the amount of green in the color
()
const;

   Returns the amount of green (after correction as described under
<Define>, above) in the color.  This amount is guaranteed to be in the range
0..1 , where both limits are inclusive.

----------------------------------------------------------------------------

inline
float        // amount of blue (0..1)
oscolor::B   // obtains the amount of blue in the color
()
const;

   Returns the amount of blue (after correction as described under <Define>,
above) in the color.  This amount is guaranteed to be in the range 0..1 ,
where both limits are inclusive.

----------------------------------------------------------------------------

oscolor::oscolor   // dummy private default constructor
()                 //    prevents default construction
{}

   Construction of a default color is prevented.  This forces user code to
always explicitly define the color of a <oscolor>.

----------------------------------------------------------------------------

float  color::r;    // amount of red   in the color
float  color::g;    // amount of green in the color
float  color::b;    // amount of blue  in the color

   These data members contain the amounts of each of the three color
components of the <oscolor>.


----------------------------------------------------------------------------
Chapter 5  -  Design and implementation considerations
----------------------------------------------------------------------------

   There are no design and implementation considerations.


----------------------------------------------------------------------------
---  End of file osrtl/windows/drawing/color/color.d .
----------------------------------------------------------------------------


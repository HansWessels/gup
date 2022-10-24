
/**************************************************************************
 * File:     osrtl/windows/drawing/font/font.cpp                          *
 * Contents: Fonts.                                                       *
 * Document: osrtl/windows/drawing/font/font.d                            *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/drawing/font/font.cpp  -  Fonts

// $RCSfile$
// $Author: erick $
// $Revision: 85 $
// $Date: 1997-12-30 14:49:19 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:49:18  erick
// First version of the MS-Windows shell (currently only osrtl).
//


/**************************************************************************
 * Detailed history.                                                      *
 **************************************************************************/

// -------------------------------------------------------------------------
// Revision  Concise description of the changes
// Dates
// Author
// -------------------------------------------------------------------------
//  1.1.000  First issue of this module source file.
// 19970729
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// The unit <osrtl> is the only unit that is allowed to include files that
// have to do with the operating system and the RTL.
#define STRICT
#include <windows.h>   // header file of the OS

// Global settings for the project.
#include "global/settings.h"

// Global settings file for the unit <osrtl>.
#include "osrtl/windows/settings.h"

// Other components of the unit <osrtl>.
#include "osrtl/windows/basics/basics.h"       // basic definitions
#include "osrtl/windows/error/error.h"         // error handling
#include "osrtl/windows/hardware/hardware.h"   // hardware characteristics
#include "osrtl/windows/tasking/tasking.h"     // task control
#include "osrtl/windows/heap/heap.h"           // heap
#include "osrtl/windows/text/text.h"           // textual data

// Header file of the current module.
#include "osrtl/windows/drawing/font/font.h"


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class guard_osrtl_drawing_font   // guards this module
{
public:
   // Life cycle.

   virtual
   ~guard_osrtl_drawing_font     // performs exit checks on the module
   ();

};  // guard_osrtl_drawing_font


/**************************************************************************
 * Local functions.                                                       *
 **************************************************************************/

static
wchar_t const *    // name of the default font
DefaultFontName    // determines the name of the default font
();

static
osresult           // result code
CountFont          // counts creation of a font
(int  n);          // I: number of fonts that were created


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

static
int                        // total number of existing fonts
fonts
= 0;

static
guard_osrtl_drawing_font   // guards this module
guard;


/**************************************************************************
 * Function: osfont::osfont                                               *
 **************************************************************************/

osfont::osfont   // initialises an <osfont>
()

{  // osfont::osfont

   if (!SetName(L"<osfont>"))
      OsDebugErrorMessage(L"Cannot set the name of an <osfont>.");

   handle    = reinterpret_cast<osfonthandle>(static_cast<HFONT>(OSNULL));
   fname     = static_cast<wchar_t const *>(OSNULL);
   zero      = OSFALSE;
   fcheight  = 0;
   fwidth    = 0;
   cmdheight = 0.0;
   cmdwidth  = 0.0;

}  // osfont::osfont


/**************************************************************************
 * Function: osfont::~osfont                                              *
 **************************************************************************/

osfont::~osfont   // closes down an <osfont>
()

{  // osfont::~osfont

   if (reinterpret_cast<HFONT>(handle) != static_cast<HFONT>(OSNULL))
   {
      OsDebugErrorMessage(L"Attempt to close down a font\n"
                          L"that was not properly destroyed.");
      Destroy();
   }

}  // osfont::~osfont



/**************************************************************************
 * Function: osfont::Create                                               *
 **************************************************************************/

osresult                     // result code
osfont::Create               // creates a font
(wchar_t const *  name,      // I: name of the font
 float            cheight)   // I: character height, screen relative

{  // osfont::Create

   osresult  result;   // result code
   int       vsize;    // line distance, pixels

   if (RoundFloatToIntegral
          (cheight * static_cast<float>(ScreenHeightPixels()),  &vsize))
   {
      if (vsize != 0)
      {
         zero   = OSFALSE;
         result = Create(name, vsize, 0);
      }
      else
      {
         zero   = OSTRUE;
         result = OSRESULT_OK;
      }

      if (result == OSRESULT_OK)
      {
         cmdheight = cheight;
         cmdwidth  = 0.0;
      }
   }
   else
      result = OSERROR_VALUEOUTOFRANGE;

   return result;

}  // osfont::Create


/**************************************************************************
 * Function: osfont::Create                                               *
 **************************************************************************/

osresult                     // result code
osfont::Create               // creates a font
(wchar_t const *  name,      // I: name of the font
 float            cheight,   // I: character height,        screen relative
 float            width)     // I: average character width, screen relative

{  // osfont::Create

   osresult  result;   // result code
   int       vsize;    // character height,        pixels
   int       hsize;    // average character width, pixels

   // Convert the screen-relative input parameters to device units.
   if (    RoundFloatToIntegral
              (cheight*static_cast<float>(ScreenHeightPixels()), &vsize)
       &&  RoundFloatToIntegral
              (width  *static_cast<float>(ScreenWidthPixels() ), &hsize))
   {
      if (vsize != 0  &&  hsize != 0)
      {
         zero   = OSFALSE;
         result = Create(name, vsize, hsize);
      }
      else
      {
         zero   = OSTRUE;
         result = OSRESULT_OK;
      }

      if (result == OSRESULT_OK)
      {
         cmdheight = cheight;
         cmdwidth  = width;
      }
   }
   else
      result = OSERROR_VALUEOUTOFRANGE;

   return result;

}  // osfont::Create


/**************************************************************************
 * Function: osfont::Create                                               *
 **************************************************************************/

osresult                   // result code
osfont::Create             // creates a font
(wchar_t const *  name,    // I: name of the font
 int              vsize,   // I: character height,        pixels
 int              hsize)   // I: average character width, pixels

{  // osfont::Create

   osresult  result = OSRESULT_OK;   // result code

   // Correct the input parameters, if necessary.
   if (vsize < 0) vsize = -vsize;
   if (hsize < 0) hsize = -hsize;

   // Check if the font already exists.
   if (reinterpret_cast<HFONT>(handle) != static_cast<HFONT>(OSNULL))
      // Check if the font that must be created is different from the
      // font that already exists.  Only if the new font is different
      // from the existing one, destroy the existing font.
      if (    StringDifferent(name, fname)
          ||  hsize != fwidth
          ||  vsize != fcheight)
         result = Destroy();

   if (    result == OSRESULT_OK
       &&  reinterpret_cast<HFONT>(handle) == static_cast<HFONT>(OSNULL))
   {
      wchar_t const *  allocated;   // allocated name

      allocated = static_cast<wchar_t const *>(OSNULL);

      // Obtain the name of the font.
      if (name == static_cast<wchar_t const *>(OSNULL))
         name = allocated = DefaultFontName();

      if (name != static_cast<wchar_t const *>(OSNULL))
      {
         DWORD  charset;   // character set of the font

         #ifdef OSRTL_UNICODE
            wchar_t *  fontname;   // same as <name>
         #else
            char *     fontname;   // ANSI equivalent of <name>
         #endif

         #ifdef OSRTL_UNICODE
            charset  = UNICODE_CHARSET;
            fontname = name;
            result   = OSRESULT_OK;
         #else
            charset  = ANSI_CHARSET;
            result   = StringDuplicate(L"ANSI equivalent of a font name",
                                       name,
                                       &fontname);
         #endif

         if (result == OSRESULT_OK)
         {
            HFONT  font;   // the font being created

            font = CreateFontA
                      (-vsize,                // character height
                       hsize,                 // average character width
                       0,                     // angle of escapement
                       0,                     // baseline orientation angle
                       FW_DONTCARE,           // default font weigth
                       FALSE,                 // not italic
                       FALSE,                 // not underline
                       FALSE,                 // not strikeout
                       charset,               // character set
                       OUT_TT_PRECIS,         // force use of truetype fonts
                       CLIP_DEFAULT_PRECIS,   // default clipping precision
                       PROOF_QUALITY,         // best output quality
                       0x04                   // truetype font
                          | FF_DONTCARE,      // don't care which one
                       fontname);             // name of the font

            if (font == static_cast<HFONT>(OSNULL))
               result = OSERROR_NORSRC_FONT;

            #ifndef OSRTL_UNICODE
               // De-allocate <fontname>.
               {
                  osheap<char>  heap;      // heap used to de-allocate
                  osresult      dealloc;   // result of the de-allocation

                  dealloc = heap.Deallocate(fontname);
                  if (result == OSRESULT_OK)
                  {
                     if ((result = dealloc) != OSRESULT_OK)
                        if (!DeleteObject(static_cast<HGDIOBJ>(font)))
                           OsDebugErrorMessage(L"Cannot delete font after "
                                               L"failure\nto de-allocate "
                                               L"the ANSI equivalent\nof "
                                               L"a font name.");
                  }
                  else
                     OsDebugErrorMessage(L"Cannot de-allocate the ANSI "
                                         L"equivalent\nof a font name.");
               }
            #endif

            if (result == OSRESULT_OK)
            {
               // We have successfully created the font.

               if (allocated == static_cast<wchar_t const *>(OSNULL))
                  fname = const_cast<wchar_t const *>
                             (StringDuplicate(L"Name of a font", name));
               else
                  fname = allocated;

               if (fname == static_cast<wchar_t const *>(OSNULL))
               {
                  if (!DeleteObject(static_cast<HGDIOBJ>(font)))
                     OsDebugErrorMessage(L"Cannot delete font after "
                                         L"failure to make\na duplicate "
                                         L"of the name of the font.");
                  result = OSERROR_NOMEMORY;
               }

               if (result == OSRESULT_OK)
               {
                  handle   = reinterpret_cast<osfonthandle>(font);
                  fwidth   = hsize;
                  fcheight = vsize;

                  if ((result = CountFont(+1)) != OSRESULT_OK)
                     if (Destroy() != OSRESULT_OK)
                        OsDebugErrorMessage(L"Cannot destroy a font after "
                                            L"failure\nto count it.");
               }
            }
         }
      }
      else
         result = OSERROR_NODEFAULTFONT;
   }

   return result;

}  // osfont::Create


/**************************************************************************
 * Function: osfont::Destroy                                              *
 **************************************************************************/

osresult          // result code
osfont::Destroy   // destroys a font
()

{  // osfont::Destroy

   osresult  result = OSRESULT_OK;   // result code

   // Allow destruction of non-existent fonts.
   if (reinterpret_cast<HFONT>(handle) != static_cast<HFONT>(OSNULL))
   {
      if (DeleteObject
             (static_cast<HGDIOBJ>(reinterpret_cast<HFONT>(handle))))
         result = CountFont(-1);
      else
         result = OSINTERNAL_FONTADMIN;

      // De-allocate <fname>.
      {
         osresult  dealloc;   // result of de-allocating <fname>

         // Do the de-allocation.
         {
            osheap<wchar_t>  heap;   // heap for de-allocating <fname>

            dealloc = heap.Deallocate(const_cast<wchar_t *>(fname));
         }

         if (result == OSRESULT_OK)
            result = dealloc;
         else
            if (dealloc != OSRESULT_OK)
               OsDebugErrorMessage(L"Cannot de-allocate "
                                   L"the name of a font.");
      }

      handle    = reinterpret_cast<osfonthandle>(static_cast<HFONT>(OSNULL));
      fname     = static_cast<wchar_t const *>(OSNULL);
      zero      = OSFALSE;
      fwidth    = 0;
      fcheight  = 0;
      cmdheight = 0.0;
      cmdwidth  = 0.0;
   }

   return result;

}  // osfont::Destroy


/**************************************************************************
 * Function: osfont::Transfer                                             *
 **************************************************************************/

osresult             // result code
osfont::Transfer     // transfers font information
(osfont &  target)   // O: target for the transfer

{  // osfont::Transfer

   osresult  result;   // result code

   // Destroy <target>.
   result = target.Destroy();

   if (result == OSRESULT_OK)
   {
      // Transfer the information to <target>.
      {
         target.handle    = handle;
         target.fname     = fname;
         target.zero      = zero;
         target.fcheight  = fcheight;
         target.fwidth    = fwidth;
         target.cmdheight = cmdheight;
         target.cmdwidth  = cmdwidth;
      }

      // Destroy the original information.
      {
         handle    = reinterpret_cast<osfonthandle>
                        (static_cast<HFONT>(OSNULL));
         fname     = static_cast<wchar_t const *>(OSNULL);
         zero      = OSFALSE;
         fcheight  = 0;
         fwidth    = 0;
         cmdheight = 0.0;
         cmdwidth  = 0.0;
      }
   }

   return result;

}  // osfont::Transfer


/**************************************************************************
 * Function: DefaultFontName                                              *
 **************************************************************************/

static
wchar_t const *   // name of the default font
DefaultFontName   // determines the name of the default font
()

{  // DefaultFontName

   static
   wchar_t const * const      // names of default fonts,
   defaultfontname[]          //    in decreasing order of preference
   = {
        L"Arial Narrow",
        L"Arial",
        L"Times New Roman",
        L"Courier New",
        L"System",
        L""
     };

   osfont     font;   // a default font
   wchar_t *  name;   // name of the default font
   int        i;      // index into <defaultfontname>

   name = static_cast<wchar_t *>(OSNULL);
   i = 0;
   do
   {
      // Attempt to create a font with the name defaultfontname[i] ,
      // using any line distance.
      if (font.Create(defaultfontname[i], 1.0) == OSRESULT_OK)
      {
         // A font with the name defaultname[i] is present in the system.
         // Use that font as the default font.
         if (font.Destroy() == OSRESULT_OK)
            name = StringDuplicate(L"Name of the default font",
                                   defaultfontname[i]);
         else
            OsDebugErrorMessage(L"Cannot destroy a default font.");
      }
   } while (    name == static_cast<wchar_t *>(OSNULL)
            &&  ++i < sizeof(defaultfontname) / sizeof(defaultfontname[0]));

   return const_cast<wchar_t const *>(name);

}  // DefaultFontName


/**************************************************************************
 * Function: CountFont                                                    *
 **************************************************************************/

static
osresult    // result code
CountFont   // counts creation of a font
(int  n)    // I: number of fonts that were created

{  // CountFont

   osresult  result;   // result code
   osmutex   mutex;    // mutex for doing font administration

   if (    (result = mutex.Connect(MUTEX_FONTADMIN)) == OSRESULT_OK
       &&  (result = mutex.EnterSection())           == OSRESULT_OK)
   {
      fonts += n;

      result = mutex.LeaveSection();
   }

   return result;

}  // CountFont


/**************************************************************************
 * Function: guard_osrtl_drawing_font::~guard_osrtl_drawing_font          *
 **************************************************************************/

guard_osrtl_drawing_font::~guard_osrtl_drawing_font  // performs exit checks
()                                                   //    on the module

{  // guard_osrtl_drawing_font::~guard_osrtl_drawing_font

   if (fonts != 0)
      OsDebugErrorMessage(L"Some fonts were created but never destroyed.");

}  // guard_osrtl_drawing_font::~guard_osrtl_drawing_font


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/drawing/font/font.cpp .                           *
 **************************************************************************/



/**************************************************************************
 * File:     osrtl/windows/window/winfont/winfont.cpp                     *
 * Contents: Window font.                                                 *
 * Document: osrtl/windows/window/winfont/winfont.d                       *
 **************************************************************************/


/**************************************************************************
 * RCS information.                                                       *
 **************************************************************************/

// @(#) osrtl/windows/window/winfont/winfont.cpp  -  Window font

// $RCSfile$
// $Author: erick $
// $Revision: 84 $
// $Date: 1997-12-30 14:42:55 +0100 (Tue, 30 Dec 1997) $
// $Log$
// Revision 1.1  1997/12/30 13:42:41  erick
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
// 19970801
// 19970818
// EAS
// -------------------------------------------------------------------------


/**************************************************************************
 * Imports.                                                               *
 **************************************************************************/

// The unit <osrtl> is the only unit that is allowed to include files that
// have to do with the operating system and the RTL.
#include <limits.h>    // ranges of basic types
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
#include "osrtl/windows/drawing/drawing.h"     // attributes for drawing

// Other modules of the component <window>.
#include "osrtl/windows/window/winpaint/winpaint.h"   // window painter

// Header file of the current module.
#include "osrtl/windows/window/winfont/winfont.h"


/**************************************************************************
 * Local types.                                                           *
 **************************************************************************/

class fontadmin     // font administrator
{
public:
   // Life cycle.

   virtual
   ~fontadmin    // closes down a <fontadmin>
   ();

   // Actual functionality.

   static
   osresult                     // result code
   Metrics                      // determines important font metrics
   (wchar_t const *    name,    // I: name of the font
    ospainter const &  p,       // I: painter for the font
    float &            hv,      // O: H/V size ratio, screen sizes, of the
                                //    font when the H size matches the V
                                //    size
    float &            height,  // O: actual height of the font as a
                                //    fraction of the commanded vertical
                                //    font size; >0 if characters extend
                                //    above baseline
    float &            depth);  // O: actual depth of the font as a fraction
                                //    of the commanded vertical font size;
                                //    >0 if characters extend below baseline

   static
   osresult                     // result code
   ActualSize                   // determines the actual size of a font
   (osfont const &     f,       // I: font to measure
    ospainter const &  p,       // I: painter for the font
    float &            h,       // O: height above baseline, screen relative
    float &            th,      // O: total height, screen relative
    float &            tw);     // O: total width, screen relative

   static
   osresult                     // result code
   GetMetricFont                // obtains font used to obtain font metrics
   (wchar_t const *   name,     // I: font name
    osfont const * &  font);    // O: font used to obtain font metrics

protected:
   // Supporting functionality.

   static
   void
   Destroy    // destroys all administration data
   ();

private:
   // Types.

   struct fontmetrics
   {
      wchar_t const *  name;    // name of the font
      float            hv;      // H/V size ratio, screen sizes, of the font
                                //    when the H size matches the V size
      float            height;  // actual height of the font as a fraction
                                //    of the commanded vertical font size;
                                //    >0 if characters extend above baseline
      float            depth;   // actual depth of the font as a fraction
                                //    of the commanded vertical font size;
                                //    >0 if characters extend below baseline
      osfont           font;    // font used to obtain font metrics
      fontmetrics *    next;    // next <fontmetrics> in the list
   };  // fontmetrics

   // Data.

   static  fontmetrics *  metricdata;   // data on the metrics of fonts

};  // fontadmin


class guard_osrtl_window_winfont:   // guards this module
   private fontadmin                //    font administrator
{
public:
   ~guard_osrtl_window_winfont      // performs exit checks on the module
   ();

};  // guard_osrtl_window_winfont


/**************************************************************************
 * Local data.                                                            *
 **************************************************************************/

fontadmin::fontmetrics *                           // data on the metrics
fontadmin::metricdata                              //    of all fonts
= static_cast<fontadmin::fontmetrics *>(OSNULL);   //    that were ever used

static
guard_osrtl_window_winfont    // guards this module
guard;


/**************************************************************************
 * Local functions.                                                       *
 **************************************************************************/

static
osresult                           // result code
GetCharMetrics                     // obtains metrics about a character
#ifdef OSRTL_UNICODE
   (HDC      dc,                   // I: device context
    wchar_t  ch,                   // I: subject character
    int &    a,                    // O: A width of the character
    int &    b,                    // O: B width of the character
    int &    c);                   // O: C width of the character
#else
   (HDC      dc,                   // I: device context
    char     ch,                   // I: subject character
    int &    a,                    // O: A width of the character
    int &    b,                    // O: B width of the character
    int &    c);                   // O: C width of the character
#endif

static
osresult                           // result code
GetFontMetrics                     // obtains metrics about a font
(osfont const &         font,      // I: subject font
 ospainter const &      painter,   // I: window painter
 OUTLINETEXTMETRIC * &  metric);   // O: metrics about the font


/**************************************************************************
 * Function: oswindowfont::oswindowfont                                   *
 **************************************************************************/

oswindowfont::oswindowfont   // initialises an <oswindowfont>
()

{  // oswindowfont::oswindowfont

   exists   = OSFALSE;
   horsize  = 0.0;
   vertsize = 0.0;
   above    = 0.0;
   if (!SetName(L"<oswindowfont>"))
      OsDebugErrorMessage(L"Cannot set the name of an <oswindowfont>.");

}  // oswindowfont::oswindowfont


/**************************************************************************
 * Function: oswindowfont::Create                                         *
 **************************************************************************/

osresult                       // result code
oswindowfont::Create           // creates a window font
(wchar_t const *    name,      // I: name of the font
 float              vsize,     // I: vertical size of font, screen rel.
 float              hsize,     // I: horizontal size of font; 1.0==normal
 ospainter const &  painter)   // I: window painter

{  // oswindowfont::Create

   osresult   result;   // result code
   fontadmin  admin;    // font administrator
   float      hv;       // H/V size ratio, screen sizes, of the font
                        //    when the H size matches the V size
   float      height;   // actual height of the font as a fraction
                        //    of the commanded vertical font size;
                        //    >0 if characters extend above baseline
   float      depth;    // actual depth of the font as a fraction
                        //    of the commanded vertical font size;
                        //    >0 if characters extend below baseline

   result = admin.Metrics(name, painter, hv, height, depth);
   if (result == OSRESULT_OK)
   {
      float  v;   // character height,        screen relative
      float  h;   // average character width, screen relative

      {
         float  mul;   // multiplier to translate font sizes

         mul = 1.0 / (height + depth);
         v = vsize * mul;
         h = hsize * vsize * mul * hv;
      }

      {
         float  delta;         // change in requested font height or width
         float  totalheight;   // total height of the font, screen relative
         float  totalwidth;    // total width of the font, screen relative

         delta = 1.0 / static_cast<float>(ScreenHeightPixels());

         // Create a font.  As long as the actual height of the font is less
         // than the desired height, increase the commanded height of the
         // font by one pixel.
         do
         {
            result = inherited::Create(name, v, h);
            if (result == OSRESULT_OK)
            {
               if (!inherited::Zero())
                  result = admin.ActualSize
                              (*dynamic_cast<inherited const *>
                                   (const_cast<oswindowfont const *>(this)),
                               painter,
                               above, totalheight, totalwidth);
               else
               {
                  above       = inherited::Height();
                  totalheight = inherited::Height();
                  totalwidth  = inherited::Width ();
               }

               if (result == OSRESULT_OK)
                  if (totalheight < vsize)
                     v += delta;
                  else
                     break;
            }
         } while (result == OSRESULT_OK);

         // As long as the actual height of the font exceeds the desired
         // height, decrease the commanded height of the font by one pixel.
         // Due to the loop above, it is expected that decreasing the
         // commanded height of the font must be done at most once.
         while (totalheight >= vsize + delta*0.5  &&  result == OSRESULT_OK)
         {
            if ((v -= delta) < 0.0)
               v = 0.0;
            result = inherited::Create(name, v, h);
            if (result == OSRESULT_OK)
               if (!inherited::Zero())
                  result = admin.ActualSize
                              (*dynamic_cast<osfont const *>
                                   (const_cast<oswindowfont const *>(this)),
                               painter,
                               above, totalheight, totalwidth);
               else
               {
                  above       = inherited::Height();
                  totalheight = inherited::Height();
                  totalwidth  = inherited::Width ();
               }
         }

         // As long as the actual width of the font exceeds the desired
         // width, decrease the commanded width of the font by one pixel.
         // It is expected that this must be done at most once.
         {
            float  hcorr = h;   // corrected version of <h>

            while (totalwidth > h  &&  result == OSRESULT_OK)
            {
               hcorr -= 1.0 / static_cast<float>(ScreenWidthPixels());
               if (hcorr < 0.0)
                  hcorr = 0.0;
               result = inherited::Create(name, v, hcorr);
               if (result == OSRESULT_OK)
                  if (!inherited::Zero())
                     result = admin.ActualSize
                                 (*dynamic_cast<osfont const *>
                                      (const_cast<oswindowfont const *>
                                          (this)),
                                  painter,
                                  above, totalheight, totalwidth);
                  else
                  {
                     above       = inherited::Height();
                     totalheight = inherited::Height();
                     totalwidth  = inherited::Width ();
                  }
            }
         }

         if (result == OSRESULT_OK)
         {
            horsize  = hsize;
            vertsize = vsize;
            exists   = OSTRUE;
         }
      }
   }

   return result;

}  // oswindowfont::Create


/**************************************************************************
 * Function: oswindowfont::TextWidth                                      *
 **************************************************************************/

osresult                      // result code
oswindowfont::TextWidth       // obtains widths of text string, scrn rel
(wchar_t const *    text,     // I: text
 ospainter const &  painter,  // I: window painter
 float &            left,     // O: width of left margin
 float &            middle,   // O: width of text without margins
 float &            full)     // O: width of text including both margins
const

{  // oswindowfont::TextWidth

   osresult  result;   // result code

   if (!inherited::Zero())
      result = TextWidth(text,
                         painter,
                         *dynamic_cast<inherited const *>(this),
                         left, middle, full);
   else
      result = TextWidthIdeal(text, painter, left, middle, full);

   return result;

}  // oswindowfont::TextWidth


/**************************************************************************
 * Function: oswindowfont::TextWidthIdeal                                 *
 **************************************************************************/

osresult                        // result code
oswindowfont::TextWidthIdeal    // obtains widths of text string, scrn rel
(wchar_t const *    text,       // I: text
 ospainter const &  painter,    // I: window painter
 float &            left,       // O: width of left margin
 float &            middle,     // O: width of text without margins
 float &            full)       // O: width of text including both margins
const

{  // oswindowfont::TextWidthIdeal

   osresult        result;   // result code
   fontadmin       admin;    // font administrator
   osfont const *  font;     // font used to obtain font metrics

   result = admin.GetMetricFont(FontName(), font);
   if (result == OSRESULT_OK)
   {
      result = TextWidth(text, painter, *font, left, middle, full);
      if (result == OSRESULT_OK)
      {
         float  adminheight;   // height of the administration font

         {
            float  hv;       // H/V size ratio, screen sizes, of the font
                             //    when the H size matches the V size
            float  height;   // actual height of the font as a fraction
                             //    of the commanded vertical font size;
                             //    >0 if characters extend above baseline
            float  depth;    // actual depth of the font as a fraction
                             //    of the commanded vertical font size;
                             //    >0 if characters extend below baseline

            result = admin.Metrics(FontName(), painter, hv, height, depth);
            if (result == OSRESULT_OK)
               adminheight = font->Height() * (height + depth);
         }

         if (result == OSRESULT_OK)
         {
            float  corr;   // correction factor for text widths

            corr = horsize * vertsize / adminheight;

            // Correct the text widths.
            {
               left   *= corr;
               middle *= corr;
               full   *= corr;
            }
         }
      }
   }

   return result;

}  // oswindowfont::TextWidthIdeal


/**************************************************************************
 * Function: oswindowfont::TextWidth                                      *
 **************************************************************************/

osresult                       // result code
oswindowfont::TextWidth        // obtains widths of text string, scrn rel
(wchar_t const *    text,      // I: text
 ospainter const &  painter,   // I: window painter
 osfont const &     font,      // I: font to use for writing the string
 float &            left,      // O: width of left margin
 float &            middle,    // O: width of text without margins
 float &            full)      // O: width of text including both margins
const

{  // oswindowfont::TextWidth

   osresult  result;   // result code
   int       length;   // length of <text>

   if (exists)
      // Obtain <length>.
      {
         unsigned long  len;   // length of <text>

         len = StringLength(text);
         if (len <= static_cast<unsigned long>(INT_MAX))
         {
            length = static_cast<int>(len);
            result = OSRESULT_OK;
         }
         else
            result = OSINTERNAL_WRITE;
      }
   else
      result = OSINTERNAL_WRITE;

   if (length > 0  &&  result == OSRESULT_OK)
   {
      HDC    dc;        // device context
      HFONT  orgfont;   // original font of the device context
      SIZE   psize;     // full text size,            pixels
      int    pleft;     // width of the left  margin, pixels
      int    pright;    // width of the right margin, pixels

      dc = reinterpret_cast<HDC>(painter);

      // Select the <oswindowfont> into the device context.
      orgfont = static_cast<HFONT>
                   (SelectObject
                       (dc, static_cast<HGDIOBJ>
                               (reinterpret_cast<HFONT>(font.Handle()))));

      // Obtain <psize>.
      {
         #ifdef OSRTL_UNICODE
            wchar_t const *  t = text;   // text
         #else
            char *  t;   // text

            result = StringDuplicate(L"Copy of a string of which the\n"
                                     L"widths must be determined.",
                                     text,
                                     &t);
         #endif

         if (result == OSRESULT_OK)
         {
            if (!GetTextExtentPointA(dc, t, static_cast<int>(length),
                                                                 &psize))
               result = OSINTERNAL_WRITE;

            #ifndef OSRTL_UNICODE
               // De-allocate <t>.
               {
                  osresult  dealloc;   // result of the de-allocation

                  {
                     osheap<char>  heap;

                     dealloc = heap.Deallocate(t);
                  }

                  if (result == OSRESULT_OK)
                     result = dealloc;
                  else
                     if (dealloc != OSRESULT_OK)
                        OsDebugErrorMessage
                           (L"Cannot de-allocate the copy of the string\n"
                            L"of which the widths had to be determined.");
               }
            #endif
         }
      }

      // Obtain <pleft> and <pright>.
      if (result == OSRESULT_OK)
      {
         #ifdef OSRTL_UNICODE
            wchar_t  first;   // first character of <text>
            wchar_t  last;    // last  character of <text>

            first = text[0];
            last  = text[length-1];
         #else
            char  first;   // first character of <text>
            char  last;    // last  character of <text>

            if (    !CharConvert(text[0       ], &first)
                ||  !CharConvert(text[length-1], &last ))
               result = OSERROR_NOCHARCONVERT;
         #endif

         if (result == OSRESULT_OK)
         {
            int  dummy;   // (no need to obtain all character widths)

            result = GetCharMetrics(dc, first, pleft, dummy, dummy);
         }

         if (result == OSRESULT_OK)
         {
            int  dummy;   // (no need to obtain all character widths)

            result = GetCharMetrics(dc, first, dummy, dummy, pright);
         }
      }

      // Select the original font into the device context, thereby
      // de-selecting the <oswindowfont> from the device context.
      if (static_cast<HFONT>
                        (SelectObject(dc, static_cast<HGDIOBJ>(orgfont)))
            != reinterpret_cast<HFONT>(font.Handle()))
         if (result == OSRESULT_OK)
            result = OSINTERNAL_WRITE;
         else
            OsDebugErrorMessage(L"Cannot de-select a font from a device "
                                L"context\nafter determining the widths "
                                L"of a text string.");

      if (result == OSRESULT_OK)
      {
         float  swidth;   // screen width, pixels

         swidth = static_cast<float>(ScreenWidthPixels());
         left   = static_cast<float>(pleft                    ) / swidth;
         middle = static_cast<float>(psize.cx - pleft - pright) / swidth;
         full   = static_cast<float>(psize.cx                 ) / swidth;
      }
   }

   return result;

}  // oswindowfont::TextWidth


/**************************************************************************
 * Function: fontadmin::~fontadmin                                        *
 **************************************************************************/

fontadmin::~fontadmin   // closes down a <fontadmin>
()

{  // fontadmin::~fontadmin

   // This is a dummy destructor that is provided only because
   // the coding standard requires that every class have a
   // virtual destructor, even if such a destructor is empty.

   (void)0;

}  // fontadmin::~fontadmin


/**************************************************************************
 * Function: fontadmin::Metrics                                           *
 **************************************************************************/

osresult                     // result code
fontadmin::Metrics           // determines important font metrics
(wchar_t const *    name,    // I: name of the font
 ospainter const &  p,       // I: painter for the font
 float &            hv,      // O: H/V size ratio, screen sizes, of the font
                             //    when the H size matches the V size
 float &            height,  // O: actual height of the font as a fraction
                             //    of the commanded vertical font size;
                             //    >0 if characters extend above baseline
 float &            depth)   // O: actual depth of the font as a fraction
                             //    of the commanded vertical font size;
                             //    >0 if characters extend below baseline

{  // fontadmin::Metrics

   osresult       result;   // result code
   fontmetrics *  curr;     // element of the list <metricdata>

   // Check if the metrics of font <name> are already known.
   {
      curr = metricdata;
      while (    curr != static_cast<fontmetrics *>(OSNULL)
             &&  StringDifferent(curr->name, name))
         curr = curr->next;
   }

   if (curr != static_cast<fontmetrics *>(OSNULL))
   {
      hv     = curr->hv;
      height = curr->height;
      depth  = curr->depth;
      result = OSRESULT_OK;
   }
   else
   {
      // The metrics of font <name> are not yet known.  Let's find out what
      // the metrics are...

      osfont  font;   // test font

      // Create a font.  The size is not important.
      if ((result = font.Create(name, 1.0)) == OSRESULT_OK)
      {
         OUTLINETEXTMETRIC *  metric;   // metrics about the font

         result = GetFontMetrics(font, p, metric);
         if (result == OSRESULT_OK)
         {
            // metric->otmEMSquare is the design size of the font.
            // Creating a font that has this design size as its
            // vertical size allows to obtain the _exact_ value of
            // the font's average character width.

            float  dheight;   // height of the font, screen relative

            dheight = metric->otmEMSquare
                         / static_cast<float>(ScreenHeightPixels());

            // Create a font that is as high as the font's design size.
            result = font.Create(name, dheight);

            if (result == OSRESULT_OK)
            {
               OUTLINETEXTMETRIC *  metric;   // metrics about the font

               result = GetFontMetrics(font, p, metric);
               if (result == OSRESULT_OK)
               {
                  TEXTMETRIC *  tm;   // metrics about the font

                  tm = &metric->otmTextMetrics;
                  {
                     float  dwidth;   // average char width, screen rel.

                     dwidth = static_cast<float>(tm->tmAveCharWidth)
                                 / static_cast<float>(ScreenWidthPixels());
                     hv = dwidth / dheight;
                  }
                  height = static_cast<float>(tm->tmAscent)
                              / static_cast<float>(metric->otmEMSquare);
                  depth  = static_cast<float>(tm->tmDescent + 1)
                              / static_cast<float>(metric->otmEMSquare);

                  // De-allocate <metric>.
                  {
                     osheap<char>  heap;

                     result = heap.Deallocate
                                 (reinterpret_cast<char *>(metric));
                  }
               }
            }

            // De-allocate <metric>.
            {
               osresult      dealloc;   // result of the de-allocation
               osheap<char>  heap;

               dealloc = heap.Deallocate(reinterpret_cast<char *>(metric));
               if (result == OSRESULT_OK)
                  result = dealloc;
               else
                  if (dealloc != OSRESULT_OK)
                     OsDebugErrorMessage(L"Cannot de-allocate the data "
                                         L"structure\nfor text metrics "
                                         L"after a failure occurred.");
            }
         }
      }

      if (result == OSRESULT_OK)
      {
         // The metrics of the font have been determined.  Save them.

         osheap<fontmetrics>  heap;
         fontmetrics *        newmetrics;   // new element of <metricdata>

         newmetrics = heap.Allocate(L"Font metrics");
         if (newmetrics != static_cast<fontmetrics *>(OSNULL))
         {
            newmetrics->name = const_cast<wchar_t const *>
                                  (StringDuplicate
                                      (L"font name in a <fontmetrics>",
                                       name));
            if (    newmetrics->name != static_cast<wchar_t const *>(OSNULL)
                ||  name == static_cast<wchar_t const *>(OSNULL))
            {
               result = font.Transfer(newmetrics->font);
               if (result == OSRESULT_OK)
               {
                  osmutex  mutex;   // mutex for doing font administration

                  newmetrics->hv     = hv;
                  newmetrics->height = height;
                  newmetrics->depth  = depth;

                  if (    (result = mutex.Connect(MUTEX_FONTMANAGE))
                                                             == OSRESULT_OK
                      &&  (result = mutex.EnterSection()) == OSRESULT_OK)
                  {
                     newmetrics->next = metricdata;
                     metricdata       = newmetrics;

                     if ((result = mutex.LeaveSection()) != OSRESULT_OK)
                        metricdata = metricdata->next;
                  }
               }

               if (result != OSRESULT_OK)
                  if (font.Destroy() != OSRESULT_OK)
                     OsDebugErrorMessage(L"Cannot destroy the font that is"
                                         L"used to\nobtain font metrics "
                                         L"after a failure in\nobtaining "
                                         L"font metrics.");
            }
            else
               result = OSERROR_NOMEMORY;

            if (result != OSRESULT_OK)
               if (heap.Deallocate(newmetrics) != OSRESULT_OK)
                  OsDebugErrorMessage(L"Cannot de-allocate a <fontmetrics>"
                                      L"\nafter failure to make a copy\n"
                                      L"of the name of the font.");
         }
         else
            result = OSERROR_NOMEMORY;
      }
   }

   return result;

}  // fontadmin::Metrics


/**************************************************************************
 * Function: fontadmin::ActualSize                                        *
 **************************************************************************/

osresult                 // result code
fontadmin::ActualSize    // determines the actual size of a font
(osfont const &     f,   // I: font to measure
 ospainter const &  p,   // I: painter for the font
 float &            h,   // O: height above baseline, screen relative
 float &            th,  // O: total height, screen relative
 float &            tw)  // O: total width,  screen relative

{  // fontadmin::ActualSize

   osresult    result;   // result code
   HDC         dc;       // device context of <p>
   HFONT       org;      // font that was originally selected into <dc>
   TEXTMETRIC  m;        // metrics about <f>

   dc = reinterpret_cast<HDC>(p);
   org = static_cast<HFONT>
            (SelectObject(dc, static_cast<HGDIOBJ>
                                 (reinterpret_cast<HFONT>(f.Handle()))));
   if (GetTextMetrics(dc, &m))
   {
      float  sheight;   // total height of the screen, pixels

      sheight = static_cast<float>(ScreenHeightPixels());
      h  = static_cast<float>(m.tmAscent                  ) / sheight;
      th = static_cast<float>(m.tmAscent + m.tmDescent + 1) / sheight;
      tw =   static_cast<float>(m.tmAveCharWidth   )
           / static_cast<float>(ScreenWidthPixels());
      result = OSRESULT_OK;
   }
   else
      result = OSINTERNAL_FONTADMIN;

   SelectObject(dc, static_cast<HGDIOBJ>(org));

   return result;

}  // fontadmin::ActualSize


/**************************************************************************
 * Function: fontadmin::GetMetricFont                                     *
 **************************************************************************/

osresult                   // result code
fontadmin::GetMetricFont   // obtains font used to obtain font metrics
(wchar_t const *   name,   // I: font name
 osfont const * &  font)   // O: font used to obtain font metrics

{  // fontadmin::GetMetricFont

   osresult       result;   // result code
   fontmetrics *  curr;     // element of the list <metricdata>

   // Check if the metrics of font <name> are already known.
   {
      curr = metricdata;
      while (    curr != static_cast<fontmetrics *>(OSNULL)
             &&  StringDifferent(curr->name, name))
         curr = curr->next;
   }

   if (curr != static_cast<fontmetrics *>(OSNULL))
   {
      font   = const_cast<osfont const *>(&curr->font);
      result = OSRESULT_OK;
   }
   else
      result = OSINTERNAL_FONTADMIN;

   return result;

}  // fontadmin::GetMetricFont


/**************************************************************************
 * Function: fontadmin::Destroy                                           *
 **************************************************************************/

void
fontadmin::Destroy    // destroys all administration data
()

{  // fontadmin::Destroy

   osmutex  mutex;   // mutex for doing font administration

   if (mutex.Connect(MUTEX_FONTMANAGE) == OSRESULT_OK)
      if (mutex.EnterSection() == OSRESULT_OK)
      {
         // Destroy the entire list <metricdata>.
         while (metricdata != static_cast<fontmetrics *>(OSNULL))
         {
            fontmetrics *  victim;   // element to be destroyed

            victim     = metricdata;
            metricdata = metricdata->next;

            // Destroy victim->font .
            if (victim->font.Destroy() != OSRESULT_OK)
               OsDebugErrorMessage(L"Cannot destroy a font that was\n"
                                   L"used to obtain font metrics.");

            // De-allocate victim->name .
            if (victim->name != static_cast<wchar_t const *>(OSNULL))
            {
               osheap<wchar_t>  heap;

               if (heap.Deallocate(const_cast<wchar_t *>(victim->name))
                      != OSRESULT_OK)
                  OsDebugErrorMessage(L"Cannot de-allocate the name of a "
                                      L"font\nwhile cleaning up the font "
                                      L"administration.");
            }

            // De-allocate <victim>.
            {
               osheap<fontmetrics>  heap;

               if (heap.Deallocate(victim) != OSRESULT_OK)
                  OsDebugErrorMessage(L"Cannot de-allocate data on a font\n"
                                      L"while cleaning up the font "
                                      L"administration.");
            }
         }

         if (mutex.LeaveSection() != OSRESULT_OK)
            OsDebugErrorMessage(L"Cannot leave a critical section "
                                L"after\nhaving cleaned up the "
                                L"font administration.");
      }
      else
         OsDebugErrorMessage(L"Cannot enter a critical section to\n"
                             L"clean up the font administration.");
   else
      OsDebugErrorMessage(L"Cannot connect to the mutex for\n"
                          L"cleaning up font administration.");

}  // fontadmin::Destroy


/**************************************************************************
 * Function: GetCharMetrics                                               *
 **************************************************************************/

static
osresult                // result code
GetCharMetrics          // obtains metrics about a character
#ifdef OSRTL_UNICODE
   (HDC      dc,        // I: device context
    wchar_t  ch,        // I: subject character
    int &    a,         // O: A width of the character
    int &    b,         // O: B width of the character
    int &    c)         // O: C width of the character
#else
   (HDC      dc,        // I: device context
    char     ch,        // I: subject character
    int &    a,         // O: A width of the character
    int &    b,         // O: B width of the character
    int &    c)         // O: C width of the character
#endif

{  // GetCharMetrics

   osresult  result;   // result code
   ABC       abc;      // A, B and C widths of the character

   if (GetCharABCWidths(dc, static_cast<UINT>(ch), static_cast<UINT>(ch),
                        &abc))
   {
      a = abc.abcA;
      b = abc.abcB;
      c = abc.abcC;

      result = OSRESULT_OK;
   }
   else
      if (GetCharWidth(dc, static_cast<UINT>(ch), static_cast<UINT>(ch),
                       &b))
      {
         a = 0;
         c = 0;

         result = OSRESULT_OK;
      }
      else
         result = OSINTERNAL_WRITE;

   return result;

}  // GetCharMetrics


/**************************************************************************
 * Function: GetFontMetrics                                               *
 **************************************************************************/

static
osresult                           // result code
GetFontMetrics                     // obtains metrics about a font
(osfont const &         font,      // I: subject font
 ospainter const &      painter,   // I: window painter
 OUTLINETEXTMETRIC * &  metric)    // O: metrics about the font

{  // GetFontMetrics

   osresult  result;    // result code
   UINT      size;      // required size of *metric
   HFONT     orgfont;   // original font in <painter>

   // Select <font> into <painter>.
   orgfont = static_cast<HFONT>
                (SelectObject(reinterpret_cast<HDC>(painter),
                              static_cast<HGDIOBJ>
                                 (reinterpret_cast<HFONT>(font.Handle()))));

   size = GetOutlineTextMetrics(reinterpret_cast<HDC>(painter),
                                0,
                                static_cast<OUTLINETEXTMETRIC *>(OSNULL));
   if (size > 0)
   {
      osheap<char>  heap;

      // Allocate <metric>.
      metric = reinterpret_cast<OUTLINETEXTMETRIC *>
                (heap.Allocate(L"Text metrics",
                 static_cast<unsigned long>(size)));
      if (metric != static_cast<OUTLINETEXTMETRIC *>(OSNULL))
      {
         if (GetOutlineTextMetrics(reinterpret_cast<HDC>(painter),
                                   size,
                                   metric))
            result = OSRESULT_OK;
         else
         {
            result = OSINTERNAL_FONTADMIN;

            // De-allocate <metric>.
            if (heap.Deallocate(reinterpret_cast<char *>(metric))
                                                            != OSRESULT_OK)
               OsDebugErrorMessage(L"Cannot de-allocate space for text "
                                   L"metrics\nafter failure to obtain "
                                   L"the text metrics.");
         }
      }
      else
         result = OSERROR_NOMEMORY;
   }
   else
      result = OSINTERNAL_FONTADMIN;

   // Select <font> out of <painter>; re-select the font that was originally
   // selected into <painter> into <painter>.
   SelectObject(reinterpret_cast<HDC>(painter),
                static_cast<HGDIOBJ>(orgfont));

   return result;

}  // GetFontMetrics


/**************************************************************************
 * Function: guard_osrtl_window_winfont::~guard_osrtl_window_winfont      *
 **************************************************************************/

guard_osrtl_window_winfont::~guard_osrtl_window_winfont   // performs exit
()                                                        //    checks on
                                                          //    the module

{  // guard_osrtl_window_winfont

   Destroy();

}  // guard_osrtl_window_winfont


/*************************************************************************
 *****   This module was written by E.A. Silkens, head of the EASy   *****
 *****   Programming Group.  You may freely use this code for your   *****
 *****   own purposes without paying any fee.  However, if you use   *****
 *****   this code, either literally or modified, you are obliged    *****
 *****   to mention the name of the original author, E.A. Silkens.   *****
 *************************************************************************/


/**************************************************************************
 * End of osrtl/windows/window/winfont/winfont.cpp .                      *
 **************************************************************************/



#ifndef __CGL_INTERNAL_H
#define __CGL_INTERNAL_H

#include "cgl_appl.h"
#include "cgl_dial.h"

#define FALSE			0
#define TRUE			1

/*
 * Flags for cgld_form_button().
 */

#define FMD_FORWARD		0
#define FMD_BACKWARD	1
#define FMD_DEFLT		2

/*
 * Constants for 3D-buttons.
 */

#define AES3D_1			0x200
#define AES3D_2			0x400

/*
 * Extended object types (4 - 63)
 */

#define CGLD_EMODECOUNT	64							/* Number of extended types. */

#define __XD_IS_ELEMENT			0x01
#define __XD_IS_SPECIALKEY	0x02
#define __XD_IS_SELTEXT			0x04				/* element that may employ 'shortcuts' */
#define __XD_IS_NOTYETDEF		0x08				/* element type that's not yet defined (how sad ;-)) */

/* 4 is reserved for special functions. */

#define CGLD_DRAGBOX		5								/* Button in upper right corner to drag the dialog box. */
#define CGLD_ROUNDRB		6								/* Round radiobutton. */
#define CGLD_RECTBUT		7								/* Rectangular button. */
#define CGLD_BUTTON			8								/* Ordinary button with keyboard control. */
#define CGLD_RBUTPAR		9								/* Rectangle with title. */
#define CGLD_TITLE			10							/* IA: Underlined title */
#define CGLD_RECTBUTTRI	11							/* IA: rectangle button: tri-state!
																				   this means cycle between:
																				   NORMAL / SELECTED / CHECKED */
#define CGLD_CYCLBUT		12							/* IA: cycling button. used with pop-ups mostly. */
#define CGLD_UP					52							/* Codes for control with cursor keys. */
#define CGLD_DOWN				53
#define CGLD_LEFT				54
#define CGLD_RIGHT			55
#define CGLD_SUP				56
#define CGLD_SDOWN			57
#define CGLD_SLEFT			58
#define CGLD_SRIGHT			59
#define CGLD_CUP				60
#define CGLD_CDOWN			61
#define CGLD_CLEFT			62
#define CGLD_CRIGHT			63

/*
 * Constants for tri-state button states.
 */

#define TRISTATE_MASK	(CROSSED | CHECKED)
#define TRISTATE_0		CROSSED
#define TRISTATE_1		CHECKED
#define TRISTATE_2		(CROSSED | CHECKED)

/*
 * Defines for internal fill style.
 */

#define FIS_HOLLOW		0
#define FIS_SOLID		1
#define FIS_PATTERN		2
#define FIS_HATCH		3
#define FIS_USER		4

/*
 * Structures and other type definitions.
 */

typedef struct
{
	int fnt_id;
	int fnt_type;
	int fnt_height;
	int fnt_chw;
	int fnt_chh;
} CGL_FONT;

typedef struct xuserblk
{
	int (*ub_code)(PARMBLK *parmblock);
	struct xuserblk *ub_parm;							/* Pointer to itself. */
	int ob_type;													/* Original object type. */
	int ob_flags;													/* Original object flags. */
	long ob_spec;													/* Original object specifier. */
} XUSERBLK;


typedef struct xdobjdata
{
	struct xdobjdata *next;
} CGLOBJDATA;

/*
 * Global variabels.
 */

extern int cgl_vdi_handle;							/* VDI handle. */
extern int cgl_ncolors;									/* Number of colors. */
extern int cgl_nplanes;									/* Number of planes. */
extern int cgl_fonts_loaded;						/* Fonts loaded flags. */
extern CGL_FONT cgl_regular_font;				/* Info of regular font. */
extern CGL_FONT cgl_small_font;					/* Info of small font. */

extern int cgl_apid;										/* AES application id. */
extern int cgl_aes4_0;									/* AES 4.0 flag (MultiTOS extensions). */
extern int cgl_draw_3d;									/* 3D buttons. */
extern int cgl_min_timer;								/* Minimum time passed to event_multi(). */
extern GRECT cgl_desk;									/* Size of desktop. */

extern APPLICATION *cgl_app;

extern int cgl_local_button_event;

extern cgl_dialog *cgl_dialogs;
extern unsigned char cgld_emode_specs[CGLD_EMODECOUNT];

extern CGLOBJDATA *cgl_objdata;

template<class T> inline T min(T x, T y)	{ return (x < y) ? x : y; }
template<class T> inline T max(T x, T y)	{ return (x > y) ? x : y; }

/*
 * Inline function to check if an extended user-defined structure is used.
 */

static inline int IS_XUSER(USERBLK *userblk)	\
	{ return (userblk->ub_parm == (long)userblk); }

/*
 * Functions from util.cc.
 */

extern int cgl_clip_desk(GRECT *r);
extern void cgl_xuserdef(OBJECT *object, XUSERBLK *userblk,
												 int (*code) (PARMBLK *parmblock));
extern void cgl_reset_mouse_update(void);

/*
 * Functions from window.cc.
 */

extern int cw_hndlbutton(int x, int y, int n, int bstate, int kstate);
extern int cw_hndlkey(int scancode, int keystate);
extern int cw_hndlmessage(int *message);

#endif /* __CGL_INTERNAL_H */

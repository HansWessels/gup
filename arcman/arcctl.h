/*
 * Archive manager.
 *
 *     Archive control function codes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:48:06 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 210 $
 * $Log$
 * Revision 1.2  2000/09/03 12:48:06  wout
 * Added control code ARC_MV_MODE. Added hardlink supported flag.
 *
 * Revision 1.1  1998/12/28 14:58:08  klarenw
 * Updated to new compression engine. Cleanups. LHA support.
 */

/*
 * General archive controls.
 */

#define ARC_FILE_TYPES		0			/* Return a bitmap with supported file types. */
#define ARC_MV_MODE			1			/* Multiple volume support. */

/*
 * LHA archive controls.
 */

#define ARC_LHA_HDR_LEVEL	0x100		/* Set LHA header level. */

/*
 * Flags for ARC_FILE_TYPES.
 */

#define ARC_FT_REG_FILE		0x01		/* Archive supports regular files. */
#define ARC_FT_DIR			0x02		/* Archive supports directories. */
#define ARC_FT_SYMLINK		0x04		/* Archive supports symbolic links. */
#define ARC_FT_HARDLINK		0x08		/* Archive supports hard links. */

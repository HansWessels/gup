/********************************************************************
 *                                                                  *
 * ARJ Shell. Copyright (c) 1995, W. Klaren.                        *
 *                                                                  *
 ********************************************************************
 *
 * $Author: klarenw $
 * $Date: 1998-03-18 19:49:04 +0100 (Wed, 18 Mar 1998) $
 * $Revision: 112 $
 * $Log$
 * Revision 1.3  1998/03/18 18:49:03  klarenw
 * *** empty log message ***
 *
 * Revision 1.2  1996/08/21 16:51:32  wklaren
 * Versie 0.1 van de shell.
 *
 * Revision 1.1  1996/08/07 11:59:32  wklaren
 * Eerste versie van de ARJ shell. Deze versie maakt nog gebruik
 * van de xdialog++ bibliotheek.
 *
 ********************************************************************/

#include <aesbind.h>
#include <mintbind.h>
#include <stdlib.h>
#include <string.h>
#include <vdibind.h>

#include "arjshell.h"
#include "cgl_dial.h"
#include "resource.h"
#include "version.h"

#define FALSE		0
#define TRUE		1

#define RSRCNAME	"arjshell.rsc"

OBJECT *menu,
		   *maindialog,
		   *infobox,
		   *progressdialog,
		   *newarchive,
		   *comp_opt_dialog;

int rsc_init(void)
{
	int result;

	(void)Pdomain(0);
	result = rsrc_load(RSRCNAME);
	(void)Pdomain(1);

	if (!result)
		return 1;

	cgld_gaddr(R_TREE, ARJMENU, &menu);
	cgld_gaddr(R_TREE, MAINDIAL, &maindialog);
	cgld_gaddr(R_TREE, INFOBOX, &infobox);
	cgld_gaddr(R_TREE, PROGDIAL, &progressdialog);
	cgld_gaddr(R_TREE, NEWARCHV, &newarchive);
	cgld_gaddr(R_TREE, PACKOPTS, &comp_opt_dialog);

	((TEDINFO *) infobox[INFOVERS].ob_spec)->te_ptext = INFO_VERSION;
	((TEDINFO *) infobox[COPYRGHT].ob_spec)->te_ptext = INFO_COPYRIGHT;

	/*
	 * Align title and close box of directory listbox.
	 */

	maindialog[DIRCLOSE].ob_y = maindialog[DIRTEXT].ob_y - maindialog[DIRCLOSE].ob_height - 1;
	maindialog[DIRTITLE].ob_x = maindialog[DIRCLOSE].ob_x + maindialog[DIRCLOSE].ob_width + 1;
	maindialog[DIRTITLE].ob_y = maindialog[DIRTEXT].ob_y - maindialog[DIRTITLE].ob_height - 1;

	/*
	 * Align title and close box of archive listbox.
	 */

	maindialog[ARJCLOSE].ob_y = maindialog[ARJTEXT].ob_y - maindialog[ARJCLOSE].ob_height - 1;
	maindialog[ARJTITLE].ob_x = maindialog[ARJCLOSE].ob_x + maindialog[ARJCLOSE].ob_width + 1;
	maindialog[ARJTITLE].ob_y = maindialog[ARJTEXT].ob_y - maindialog[ARJTITLE].ob_height - 1;

	return 0;
}

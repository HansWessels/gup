/********************************************************************
 *                                                                  *
 * ARJ Shell. Copyright (c) 1996, W. Klaren.                        *
 *                                                                  *
 ********************************************************************
 *
 * $Author: klarenw $
 * $Date: 1998-03-18 19:49:04 +0100 (Wed, 18 Mar 1998) $
 * $Revision: 112 $
 * $Log$
 * Revision 1.2  1998/03/18 18:49:03  klarenw
 * *** empty log message ***
 *
 * Revision 1.1  1996/08/07 12:00:12  wklaren
 * Eerste versie van de ARJ shell. Deze versie maakt nog gebruik
 * van de xdialog++ bibliotheek.
 *
 ********************************************************************/

#include "arjshell.h"

extern OBJECT *menu,
				*maindialog,
				*infobox,
				*progressdialog,
				*newarchive,
				*comp_opt_dialog;

int rsc_init(void);

/*
 * Include files.
 *
 *     Archive manager.
 *
 * $Author: klarenw $
 * $Date: 1998-12-28 16:17:33 +0100 (Mon, 28 Dec 1998) $
 * $Revision: 140 $
 * $Log$
 * Revision 1.1  1998/12/28 15:17:33  klarenw
 * *** empty log message ***
 *
 */

#ifndef __ARCMAN_H__
#define __ARCMAN_H__

#include "sysdep/arcman.h"
#include "../arcman/header.h"
#include "../arcman/archive.h"
#include "../arcman/arcctl.h"
#include "../arcman/os.h"


extern int compress(OPTIONS* opt);
extern int decompress(OPTIONS* opt);
extern int list_arj(OPTIONS* opt);

#endif

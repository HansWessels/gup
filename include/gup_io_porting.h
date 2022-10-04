/*
 * GNU Universal packer.
 *
 *     General porting includes.
 */

#ifndef __GUP_IO_PORTING_H__
#define __GUP_IO_PORTING_H__



#ifdef O_BINARY
#define OPEN_RD_FLAGS	(O_RDONLY | O_BINARY)
#define OPEN_WR_FLAGS	(O_RDWR | O_CREAT | O_TRUNC | O_BINARY)
#else
#define OPEN_RD_FLAGS	O_RDONLY
#define OPEN_WR_FLAGS	(O_RDWR | O_CREAT | O_TRUNC)
#endif

#ifdef S_IWGRP
#define FMODE	(S_IWUSR | S_IWGRP | S_IWOTH | S_IRUSR | S_IRGRP | S_IROTH)
#else
#define FMODE	(S_IREAD | S_IWRITE)
#endif


#endif

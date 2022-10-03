/*
  Include file voor ARJBETA.C
  (c) 1994 Mr Ni! (the Great)
  Voor testen inpak routines
  
  1997-11-08
  HWS: added NEVER_USE(x) macro to surpress the warning: 
       never used parameter x in functions
  
*/

#ifndef __ARJBETA_H__
#define __ARJBETA_H__

#include "compress.h"

#define NEVER_USE(x) (void) x

typedef struct
{
  int handle;                   /* file handle van source file */
  uint32 (*crc_func)(uint8 *str, long len, uint32, void *crc_table); /* crc_functie */
  uint32 crc; /* crc som */
  void* crc_table; /* crc_table */
} read_crc_struct;

typedef struct
{
  char* arj_naam;                /* pointer naar archive naam */
  unsigned long main_header;     /* start van de main header in de file */
  int add_mode;                  /* 1=add files, 0=create new archive */
  int extension;                 /* 1 = force arj extension */
  int mv_nr;                     /* volume nummer */
  int mv_drive;                  /* drive waarop de multiple volumes worden gezet */
  int path;                      /* store 1 complete path, 0 only filenames */
  unsigned long mv_size;         /* grootte van volume in bytes */
                                 /* als mv aanstaat en mv_size == 0 dan wordt
                                    de gebruikte drive als removable beschouwd
                                    en wordt hij vol gepropt waarna om het
                                    volgende volume wordt gevraagd. */
  long (*headerlen)(const char *naam, packstruct *com); /* calculates the header length */
  uint32 (*init_crc)(void); /* initialiseert de crc variabele */
  void *(*make_crc_table)(void* propagator); /* build crc_table */
  union
  {
    uint32 poly32; /* crc plolynomal */
    uint16 poly16; /* crc plolynomal */
  }crc;
  read_crc_struct rc; /* crc, crc_table, crc_func, handle voor fread crc */
  void (*close_and_create_volume)(packstruct *com); /* maakt volume aan */
  gup_result (*make_header)(const char *naam, long origsize, long packedsize,
                      long filestart, int mode, int attrib, uint16 datum, uint16 tijd, 
                      uint8 flag, packstruct *com); /* maakt de fileheader */
  unsigned long (* make_main_header)(char *naam, uint8 flags, packstruct *com); /* maakt de archive header */
  gup_result (*close_archive)(packstruct *com); /* sluit archive af */
} commandstruct;

void showheader(void);
extern commandstruct xcommand;

gup_result buf_write(long count, buf_fhandle_t* buff, int* handle);

#define strend(s)   (s + strlen(s))

#endif
  
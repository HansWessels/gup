/*
 * Sliding dictionary code voor ARJ
 *
 * (c) 1995 Hans "Mr Ni! the Great" Wessels
 *
*/

/*
 * Tree for sliding dictionary
 *
 * (c) Copyright 1995 Hans Wessels
 *
 * Wat is het idee van deze tree?
 *
 * De nieuwste nodes zitten altijd boven aan, de beide childs van een node
 * zijn altijd ouder dan de node zelf. Dit heeft tot gevolg dat de leaves
 * van de tree de oudste nodes zijn.
 *
 * Hierdoor vind je bij het matchen altijd de dichtbijzijndste match. Een
 * tweede voordeel is dat de oudste nodes altijd helemaal onderaan liggen,
 * op deze manier kun je ze makkelijk verwijderen, gewoon van de parent
 * afknippen.
 *
*/

/*-
  De sliding dictionary en verscheidene HASH functies.

  configuratie:
  arjbeta -m7 -jm
  file: calgary corpus
  alle tijden in seconden
  de packed size was voor alle gevallen: 1024206 bytes
  rle detectie stond voor alle gevallen aan
  indien mogelijk is er een delta hash gebruikt

  HASH functie:                             | Tijd
  ------------------------------------------+-----
  0                                         | 1301
  x[0]                                      | 1074
  x[2]                                      | 1052
  (byte)((x[0]<<6)^(x[1]<<3)^x[2])          |  928
  (x[0]<<8)+x[1]                            |  901
  (x[1]<<8)+x[2]                            |  883
  ((x[1]&31)<<5)^((x[0]&31)<<10)^(x[2]&31)  |  797
  (x[0]<<12)^(x[1]<<6)^x[2]                 |  793
  (x[0]<<14)^(x[1]<<7)^(x[2])               |  828
  ((x[0]^x[1])<<8)^(x[1]^x[2])              |  794 ***
  CRC16 (van LZH)                           |  822

  Conclusie:
  *De hash functie heeft wel degelijk invloed op de pack snelheid.
  *Een hash die alle drie de characters gebruikt heeft de beste
    resultaten.
  *De nu gebruikte HASH functie (gemerkt met ***) is de beste. Deze
    heeft geen aparte rle detectie nodig en is daarom 4 seconden
    sneller dan hier staat aangegeven.
*/

#if 0

/* obsolete */

#endif

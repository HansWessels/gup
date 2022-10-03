
#ifndef __COMPR_IO_H__
#define __COMPR_IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct 
{
  uint8 * start;    /* (R) start of buffer, unsigned long aligned */
  uint8 * current;  /* (R/W) position of read/write pointer */
  uint8 * end;      /* (R) end of buffer, unsigned long aligned */
} buf_fhandle_t;

/*

Hoe moet de file buffering geconstrueerd worden opdat deze 
zo optimaal mogelijk is voor de compressie en decompressie 
routines.

Er zijn voor deze routines vier verschillende gevallen die 
alle op een andere manier behandeld moeten worden.

decode:

	gup_result (*buf_read)(buf_fhandle_t* br_buf, void* br_propagator);
	buf_fhandle_t* br_buf;
	void* br_propagator;

    De buffer voor deze routine moet zo groot mogelijk zijn. 
    Hij bevat de file die gedecodeerd moet worden. De header 
    read routines kunnen ook gebruik maken van deze buffer 
    om headers te lezen.
  
    De gfs_fhandle_t is gedefinieerd in de gfs file buffering 
    routines. De struct bevat pointers die gebruikt kunnen worden 
    om data uit de buffer te lezen. Als de decoder met current_pos 
    bij buf_end is aangekomen wordt deze routine aangeroepen om de 
    buffer opnieuw te vullen. Na aanroep moet current_pos niet meer 
    op buf_end staan.
    
    De br_propagator kan gebruikt worden om extra data aan buf_read() 
    mee te geven. 


	gup_result (*buf_write_crc)(long count, const void *buf, void* bwc_propagator);
	void* bwc_propagator;
	
	  buf_write_crc() wordt gebruikt om de gedepackte data te saven. 
	  Indien het compressie formaat dat vereist kan deze functie 
	  een CRC over de data uit voeren. De data moet het liefst met zo min 
	  mogelijk tussen buffering naar file worden geschreven. Data in de 
	  buffer mag niet veranderd worden. count mag nul zijn.
	  
	  De bwc_propagator kan gebruikt worden om extra data aan buf_write_crc() 
	  mee te geven.

encode:

  gup_result (*buf_write_announce)(long count, gfs_fhandle_t* bw_buf, void* bwa_propagator);
  gfs_fhandle_t* bw_buf
  void* bwa_propagator;
  
    buf_write_announce() wordt door de compressie engine gebruikt om 
    aan te geven dat hij count bytes naar de buffer wil schrijven. De 
    routine moet er voor zorgen dat er tussen current_pos en buf_end 
    tenminste count bytes zitten. Omdat de buffer door de compressie 
    engine wordt gealloceerd moet dit altijd kunnen. De header 
    routines mogen ook gebruik maken van de buffer om data in te 
    schrijven.
    
	  De bwa_propagator kan gebruikt worden om extra data aan buf_write_announce() 
	  mee te geven.
  
  long (*buf_read_crc)(long count, const void *buf, void* brc_propagator);
  void* brc_propagator;

    De compressie routine gebruikt deze routine om z'n compressie data 
    buffer te vullen met te comprimeren data. De data moet het liefst 
    met zo min mogelijk tussen buffering weg geschreven worden. Indien 
    het compressie formaat het vereist kan de routine een crc over de 
    gelezen data uitvoeren. Er moegen niet meer dan count bytes  gelezen 
    worden. Resultaat is het aantal gelezen bytes. Zowel het resultaat 
    als count kunnen 0 zijn.

	  De brc_propagator kan gebruikt worden om extra data aan buf_read_crc() 
	  mee te geven.
	  
	Noot: De callback functie kunnen _alleen_ de members van hun eigen 
	handle struct wijzigen.

*/


#ifdef __cplusplus
}
#endif

#endif

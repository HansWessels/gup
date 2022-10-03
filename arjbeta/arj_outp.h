/*
  ARJ BETA output functies headerfile 
*/

#ifndef __ARJ_OUTPUT_H__
#define __ARJ_OUTPUT_H__

char* pnprint(int width, const char* name, char* buf);
void init_progres(unsigned long size);
void print_progres(unsigned long delta, void* propagator);
void init_progres_size(unsigned long size);
void print_progres_size(unsigned long delta, void* propagator);
void init_message(gup_result err, void* propagator);
void display_error(gup_result err);



#endif 
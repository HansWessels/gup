/*-
 * sliding dictionary evaluator
 * (c) 1996 Mr Ni! (the Great) of the TOS-crew
 */


#ifndef GUP__EVALUATOR_H__
#define GUP__EVALUATOR_H__

#ifdef __cplusplus
extern "C"
{
#endif
gup_result encode_big(packstruct *com);

int backmatch_big(long pos, uint16 ptr, int hist, packstruct* com);

void init_dictionary(packstruct *com);
c_codetype find_dictionary(long pos, packstruct *com);
c_codetype find_dictionary_fast(long pos, packstruct *com);


#ifdef __cplusplus
}

#endif
#endif

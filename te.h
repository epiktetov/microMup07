/*------------------------------------------------------+----------------------
// МикроМир07    te = Text editor - Редактор текста     | (c) Epi MG, 2007,2016
//------------------------------------------------------+--------------------*/
#ifndef TE_H_INCLUDED           /* Old "te.h" (c) Attic 1989, (c) EpiMG 2001 */
#define TE_H_INCLUDED

void TeInit(void);
bool tesetxy(int x, long y);
void qsety         (long y); /* <- exception E_SETY if cannot move to that Y */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teIL(),              /* некоторые команды (используются в clip.cpp etc) */
     teCR();
void tesmark();  /* set "temp" marker at edited text, or before "long" jumps */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern tchar *spa, *rpa, *soptfiles; /* current search and replaces patterns */
extern int    spl,  rpl,  soptfilen; /* and their length, all calculated by: */
void tesParse();                     /* search/etc patterns parsing function */
/*---------------------------------------------------------------------------*/
#endif                                                      /* TE_H_INCLUDED */

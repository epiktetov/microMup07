/*------------------------------------------------------+----------------------
// МикроМир07     UNIX-specific stuff and tmSyncPos     | (c) Epi MG, 2007,2011
//------------------------------------------------------+--------------------*/
#ifndef UNIX_H_INCLUDED      /* Old tm.h (c) Аттик 1989, (c) EpiMG 1999,2001 */
#define UNIX_H_INCLUDED
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void x2enter(void);           /* command line enter (2) ввести команду shell */
void tmshell(int kcode);
int tmGrep  (int kcode);
int tmSyncPos(void);
/*---------------------------------------------------------------------------*/
#endif                                                    /* UNIX_H_INCLUDED */

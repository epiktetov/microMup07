/*------------------------------------------------------+----------------------
// МикроМир07       Shell commands and tmSyncPos        | (c) Epi MG, 2007-2014
//------------------------------------------------------+--------------------*/
#ifndef UNIX_H_INCLUDED      /* Old tm.h (c) Аттик 1989, (c) EpiMG 1999,2001 */
#define UNIX_H_INCLUDED
extern QString MiApp_shell;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void x2enter(void);           /* command line enter (2) ввести команду shell */
void tmshell(int kcode);
int tmGrep  (int kcode);
int tmSyncPos(void);
void tmLoadXeq(txt *t); /* load text by executing command from t->file->name */
/*---------------------------------------------------------------------------*/
#endif                                                    /* UNIX_H_INCLUDED */

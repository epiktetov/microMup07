/*------------------------------------------------------+----------------------
// МикроМир07              Откатка - Undo               | (c) Epi MG, 2007,2011
//------------------------------------------------------+--------------------*/
#ifndef UD_H_INCLUDED                             /* Old ud.h (c) Attic 1989 */
#define UD_H_INCLUDED 

void tundoload  (txt *t);
void tundounload(txt *t); BOOL udcut(txt *t);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define UT_IL    1 /* тип записи: строка всталена */
#define UT_DL    2 /*             строка удалена  */
#define UT_REP   3 /*             строка заменена */
#define UT_LOAD  4 /* - загрузка редактора строки */
#define UL_InLE  5 /* - inside Line Editor        */
#define U_MARK   0100
#define REPLACE 32000  /* used for dx in lundoadd */

void leundo(), lesundo(), leunundo(), lesunundo();
extern BOOL UdMark;
void tundo1add(txt *t, small typ,          char *a, small len);
void tundo2add(txt *t, char *ao, small lo, char *an, small ln);
void lundoadd (txt *t, small xl, small xr, small dx, tchar *os, tchar *ns);
/*---------------------------------------------------------------------------*/
#endif                                                      /* UD_H_INCLUDED */

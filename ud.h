/*------------------------------------------------------+----------------------
// МикроМир07              Откатка - Undo               | (c) Epi MG, 2007
//------------------------------------------------------+--------------------*/
#ifndef UD_H_INCLUDED                             /* Old ud.h (c) Attic 1989 */
#define UD_H_INCLUDED 

void UdInit(); void tundoload  (txt *t);
               void tundounload(txt *t); BOOL udcut(txt *t);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define UT_IL    0 /* тип записи: строка всталена */
#define UT_DL    1 /*             строка удалена  */
#define UT_REP   2 /*             строка заменена */
#define UT_LOAD  3 /* - загрузка редактора строки */
#define UL_GEN   4
#define U_MARK   0100
#define REPLACE 32000 /* used for dx in lundoadd */

extern BOOL UdMark;
extern small undodir;

void tundo1add (txt *t, small typ,          char *a, small len);
void tundo2add (txt *t, char *ao, small lo, char *an, small ln);
void lundoadd  (txt *t, small xl, small xr, small dx, tchar *os, tchar *ns);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teundo(), tesundo(), teunundo(), tesunundo(), 
     leundo(), lesundo(), leunundo(), lesunundo();
/*---------------------------------------------------------------------------*/
#endif                                                      /* UD_H_INCLUDED */

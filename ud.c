/*------------------------------------------------------+----------------------
// МикроМир07              Откатка - Undo               | (c) Epi MG, 2007,2011
//------------------------------------------------------+--------------------*/
#include "mic.h"                  /* Old ud.c (c) Attic 1989, (c) EpiMG 1998 */
#include "ccd.h"
#include "twm.h"
#include "vip.h" /* uses Lwd, Ltxt and Ttxt */
#include "dq.h"
#include "le.h"
#include "te.h"
#include "tx.h"
#include "ud.h"
#include <stdio.h>
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*                 Undo буфер - дек, растущий только с конца.                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
typedef struct { /* UT_LOAD -- загрузка редактора строки */
 small    utyp;
 small _unused;
 large  uytext;
} textlundo;

typedef struct { /* UT_IL / UT_DL -- добавленные / удаленные строки (ascii) */
 small utyp;
 small uslen;
 large uytext;
/* old/new string */
} text1undo;

typedef struct { /* UT_REP -- измененные строки (ascii) */
 small utyp;
 small unslen;
 small uoslen;
 large uytext;
 small udindex;
/* new string */
/* old string */
} text2undo;

typedef struct { /* UL_InLE -- замена внутри строки (tchar) */
 small utyp;
 small ulnsl, ulosl;
 small ulx;
 small ulxl, ulxr, uldx;
 small _unused;
 /* Новая  подстрока */
 /* Старая подстрока */
} lineundo;

static BOOL undo_blocked =  FALSE;
static char ubuf[UBUFSIZ], *eubuf;
BOOL UdMark = FALSE;
/*-----------------------------------------------------------------------------
 *               Добавить сформированную запись откатки к файлу
 */
static void undogadd (txt *t, small typ)
{
  ((text1undo *)ubuf)->utyp = ( UdMark ? typ|U_MARK : typ ); UdMark = FALSE;
  large len = t->txudcptr;
  if (typ <= UT_LOAD) {                       /* началось изменение текста - */
    if (t->txudlptr < len) len = t->txudlptr; /*     удалим строчную откатку */
                           t->txudlptr = len;
  }
  DqCutE_toX(t->txudeq, len);
  DqAddE (t->txudeq, ubuf, eubuf-ubuf);
  t->txudcptr = len = DqLen(t->txudeq); 
  if (typ < UT_LOAD) t->txudlptr = len;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tundoload (txt *t)                         /* Загрузка редактора строки */
{
  if (undo_blocked || t->txudeq == NULL) return;
  ((textlundo*)ubuf)->uytext =   t->txy;
  eubuf =      ubuf + sizeof(textlundo); undogadd(t, UT_LOAD);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tundounload (txt *t)                                /* Покидание строки */
{
  UdMark = TRUE; if (undo_blocked || t->txudeq == NULL) return;
  DqCutE_toX(t->txudeq, t->txudlptr);
  t->txudcptr =         t->txudlptr;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                       Учесть операцию внутри строки
 */
void lundoadd (txt *t,
             small xl, small xr, /* Измененная или ролируемая подстрока      */
             small dx,           /* Величина сдвига, REPLACE в случае замены */
             tchar *os,
             tchar *ns) /* Старая и новая подстроки (NIL для одних пробелов) */
{
  if (undo_blocked || !t || t->txudeq == NULL) return;
  small slen = (dx == REPLACE) ? xr-xl : my_abs(dx);
  char *p = ubuf;
  lineundo *pl = (lineundo*)p;
  pl->ulx  = Lx;
  pl->ulxl = xl;
  pl->ulxr = xr;
  pl->uldx = dx;
  pl->ulnsl = xl = lstrlen(slen, ns);
  pl->ulosl = xr = lstrlen(slen, os);
  p += sizeof(lineundo);
      p = blkmov(ns, p, xl*sizeof(tchar));
  eubuf = blkmov(os, p, xr*sizeof(tchar)); undogadd(t, UL_InLE);
}
/*-----------------------------------------------------------------------------
 *                   Учесть удаление/вставку строки в текст
 */
void tundo1add (txt  *t, small typ, /* Тип записи - UT_IL или UT_DL */
                char *a, small len) /* Строка и ее длина            */
{
  if (undo_blocked || t->txudeq == NULL) return;
  text1undo *pt = (text1undo*)ubuf;
  pt->uytext = t->txy;
  pt->uslen = len;
  eubuf = lblkmov(a, ubuf+sizeof(text1undo), len); undogadd(t, typ);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Выделить минимальные различные подстроки
 * вых: индекс подстрок, *plo - длина первой подстроки, *pln - длина второй
 *      0,0,0 если строки равны
 */
static small midind (char *ao, small *plo, char *an, small *pln)
{
  char *p1, *p2;
  small i, j;
  for (p1 = ao, p2 = an, j = my_min(*plo, *pln); j; j--)
    if (*p1++ != *p2++) { p1--; break; }

  if (j == 0 && *plo == *pln) { *plo = 0;
                                *pln = 0; return 0; }  /* строки равны */
  else
    i = p1 - ao; /* индекс первого различного символа */

  for (p1 = ao + (*plo), p2 = an + (*pln); j; j--)
    if (*(--p1) != *(--p2)) { p1++; p2++; break; }
    
  *plo = (p1 - ao) - i;
  *pln = (p2 - an) - i; return i;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                       Учесть замену строки в тексте
 */
void tundo2add (txt *t, char *ao, small lo, /* Старая строка */
                        char *an, small ln) /* Новая  строка */
{
  if (undo_blocked || t->txudeq == NULL) return;
  small k = midind(ao, &lo, an, &ln);
  text2undo *pt = (text2undo*)ubuf;
  pt->uytext = t->txy;
  pt->udindex = k;
  pt->unslen = ln;
  pt->uoslen = lo;
  char *p = ubuf+sizeof(text2undo);
      p = lblkmov(an+k, p, ln);
  eubuf = lblkmov(ao+k, p, lo); undogadd(t, UT_REP);
}
/*---------------------------------------------------------------------------*/
static void genundo (int undodir)
{
  int undo_typ = ((textlundo *)ubuf)->utyp & (~U_MARK);  undo_blocked = TRUE;
  if (undo_typ == UT_LOAD) {
    textlundo *pt = (textlundo *)ubuf;
    qsety(pt->uytext);
  }
  else if (undo_typ == UL_InLE) {
    lineundo *pl = (lineundo *)ubuf;
    tchar *utsptr = (tchar *)(ubuf + sizeof(lineundo));
    small dx = pl->uldx, len = pl->ulnsl;
    small maxlen = (dx == REPLACE) ? (pl->ulxr - pl->ulxl) : my_abs(dx);
    Lx = pl->ulx;
    if (undodir < 0) {  if (dx != REPLACE)    dx = -dx;
                        utsptr += len; len = pl->ulosl;  
    }
    if (len < maxlen) blktspac(utsptr+len, maxlen-len);
                llmove(pl->ulxl, pl->ulxr, dx, utsptr); Lchange = TRUE;
  }
  else if ((undo_typ == UT_IL && undodir > 0) ||
           (undo_typ == UT_DL && undodir < 0)) {
    text1undo *pt1 = (text1undo *)ubuf;
    qsety(pt1->uytext);
    TxIL(Ttxt, ubuf+sizeof(text1undo), pt1->uslen);
  }
  else if ((undo_typ == UT_DL && undodir > 0) ||
           (undo_typ == UT_IL && undodir < 0)) {
    text1undo *pt1 = (text1undo *)ubuf;
    qsety(pt1->uytext);
    TxDL(Ttxt);
  }
  else if (undo_typ == UT_REP) {
    text2undo *pt2 = (text2undo *)ubuf;
    small index = pt2->udindex;
    small usl, uosl, unsl;
    char *usptr, *unsptr;
    qsety(pt2->uytext);
    usl = TxRead(Ttxt, afbuf) - index;
    unsptr = ubuf + sizeof(text2undo);
    usptr = afbuf + index;
    if (undodir > 0) { uosl = pt2->uoslen;
                       unsl = pt2->unslen; }
    else {
      uosl = pt2->unslen;
      unsl = pt2->uoslen; unsptr += uosl;
    }
    if (uosl != unsl) { usl -= uosl;   lblkmov(usptr+uosl, usptr+unsl, usl);
                        usl += unsl; } lblkmov(unsptr,     usptr,     unsl);
    TxRep(Ttxt, afbuf, usl + index);
  }
  undo_blocked = FALSE;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                         Собственно команды откатки
 */
static void tsundo (BOOL slow)
{
  txt *t = Lwnd ? Ltxt : Ttxt;
  if (!t || t->txudeq == NULL || t->txudcptr == 0) exc(E_NOUNDO);
  int undone = 0;
  while (t->txudcptr) {                        int real_len;
    DqCopyBackward(t->txudeq, t->txudcptr, ubuf, &real_len);
    small utyp = ((textlundo *)ubuf)->utyp,
      undo_typ = utyp & (~U_MARK);
//
// If we were in LE mode and first block to undo is not "in-LE" type, then exit
// that mode (but block any changes in tundounload function), we are guaranteed
// to not have line changes in this case; return if already undone something:
//
    if (Lwnd && undo_typ != UL_InLE) { if (undone) return;
      Lchange = FALSE;
      undo_blocked = TRUE; ExitLEmode();
    }
    t->txudcptr -= real_len;
    genundo(-1);   undone++;
    if (undo_typ == UT_LOAD)  continue; // never stop at tundoload() block op
    if (slow || (utyp & U_MARK)) break; // but stop unless "slow" or at marker
  }
  if (t->txudcptr == t->txudfile) t->txstat &= ~TS_CHANGED;
  else                            t->txstat |=  TS_CHANGED;
}
void leundo()  { tsundo(FALSE); }
void lesundo() { tsundo(TRUE ); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void tsunundo (BOOL slow)
{
  txt *t = Lwnd ? Ltxt : Ttxt;
  if (!t || !t->txudeq || t->txudcptr == DqLen(t->txudeq)) exc(E_NOUNDO);
  int redone = 0;
  while (t->txudcptr != DqLen(t->txudeq)) {   int real_len;
    DqCopyForward(t->txudeq, t->txudcptr, ubuf, &real_len);
    small utyp = ((textlundo *)ubuf)->utyp,
      undo_typ = utyp & (~U_MARK);                  // stop if slow or @marker
    if ((slow || (utyp & U_MARK)) && redone) break; //     (and already redone)
    t->txudcptr += real_len;
    genundo(1);    redone++;
    if (undo_typ == UT_LOAD) { undo_blocked =  TRUE;   // entering LE mode
                EnterLEmode(); undo_blocked = FALSE; } //  (temp.block changes)
  }
  if (t->txudcptr == t->txudfile) t->txstat &= ~TS_CHANGED;
  else                            t->txstat |=  TS_CHANGED;
}
void leunundo()  { tsunundo(FALSE); }
void lesunundo() { tsunundo(TRUE ); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                Укоротить буфер откатки до положенной квоты
 */
void udcut (txt *t)
{
  long len, minptr;
  if (t->txudeq && (len    = DqLen (t->txudeq))                > UDHQUOTA
                && (minptr = my_min(t->txudcptr, t->txudlptr)) > 0) {
    int real_len;
    if ((len -= UDLQUOTA) < minptr) minptr = len;
    for (len = 0; len < minptr; ) { 
      DqCopyForward(t->txudeq, len, NIL, &real_len); len += real_len;
    }
    t->txudcptr -= len;
    t->txudlptr -= len;  DqCutB_byX(t->txudeq, len);
} }
void udclear (txt *t) { DqEmpt(t->txudeq);
                        t->txudcptr = t->txudlptr = 0; }
/*---------------------------------------------------------------------------*/

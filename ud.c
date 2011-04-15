/*------------------------------------------------------+----------------------
// МикроМир07              Откатка - Undo               | (c) Epi MG, 2007,2011
//------------------------------------------------------+--------------------*/
#include "mic.h"                  /* Old ud.c (c) Attic 1989, (c) EpiMG 1998 */
#include "ccd.h"
#include "twm.h"
#include "vip.h" /* uses Ltxt and Ttxt */
#include "dq.h"
#include "le.h"
#include "te.h"
#include "tx.h"
#include "ud.h"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*                 Undo буфер - дек, растущий только с конца.                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
typedef struct {
 small    utyp;
 small _unused; /* hack: adjuct to dword boundary */
 large  uytext;
} textlundo;

typedef struct { /* добавленные / удаленные строки (ascii) */
 small utyp;
 small uslen;
 large uytext;
/* old/new string */
} text1undo;

typedef struct { /* измененные строки (ascii) */
 small utyp;
 small unslen;
 small uoslen;
 large uytext;
 small udindex;
/* new string */
/* old string */
} text2undo;

typedef struct { /* замена внутри строки (tchar) */
 small utyp;
 small ulnsl, ulosl;
 small ulx;
 small ulxl, ulxr, uldx;
 small _unused; /* hack: adjuct to dword boundary */
 /* Новая  подстрока */
 /* Старая подстрока */
} lineundo;

static char *ubuf, *eubuf;
small undodir;
BOOL  UdMark;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void UdInit (void)
{
  ubuf = GetMain(UBUFSIZ); undodir = 0;
  UdMark = FALSE;
}
/*-----------------------------------------------------------------------------
 *               Добавить сформированную запись откатки к файлу
 */
static void undogadd (txt *t, small typ)
{
  large len = t->txudcptr;
  ((text1undo *)ubuf)->utyp = ( UdMark ? typ|U_MARK : typ );
  UdMark=FALSE;
  if (typ <= UT_LOAD) {                       /* изменение текста -          */
    if (t->txudlptr < len) len = t->txudlptr; /*     удалим строчную откатку */
                           t->txudlptr = len;
  }
  DqCutE_toX(t->txudeq, len);
  DqAddE (t->txudeq, ubuf, eubuf-ubuf);
  t->txudcptr = len = DqLen(t->txudeq); 
  if (typ < UT_LOAD) t->txudlptr = len;
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
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tundoload (txt *t)                         /* Загрузка редактора строки */
{
  if (undodir == 0 && t->txudeq) {
    char *p = ubuf;
    ((textlundo *)p)->uytext = t->txy;
    eubuf = p + sizeof(textlundo);
    undogadd(t, UT_LOAD);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tundounload (txt *t)                                /* Покидание строки */
{
  if (undodir == 0 && t->txudeq) {
    DqCutE_toX(t->txudeq, t->txudlptr);
    t->txudcptr = t->txudlptr;
  }
  UdMark = TRUE;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                   Учесть удаление/вставку строки в текст
 */
void tundo1add (txt  *t, small typ, /* Тип записи - UT_IL или UT_DL */
                char *a, small len) /* Строка и ее длина            */
{
  if (undodir == 0 && t->txudeq) {
    char *p = ubuf;
    text1undo *pt = (text1undo*)p;
    pt->uytext = t->txy;
    pt->uslen = len;
    p += sizeof(text1undo);
    eubuf = lblkmov(a, p, len); undogadd(t, typ);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                       Учесть замену строки в тексте
 */
void tundo2add (txt *t, char *ao, small lo, /* Старая строка */
                        char *an, small ln) /* Новая  строка */
{
  if (undodir == 0 && t->txudeq) {
    small k = midind(ao, &lo, an, &ln);
    char *p = ubuf;
    text2undo *pt = (text2undo*)p;
    pt->uytext = t->txy;
    pt->udindex = k;
    pt->unslen = ln;
    pt->uoslen = lo;
    p += sizeof(text2undo);
        p = lblkmov(an+k, p, ln);
    eubuf = lblkmov(ao+k, p, lo); undogadd(t, UT_REP);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                       Учесть операцию внутри строки
 */
void lundoadd (txt *t,
             small xl, small xr, /* Измененная или ролируемая подстрока      */
             small dx,           /* Величина сдвига, REPLACE в случае замены */
             tchar *os,
             tchar *ns) /* Старая и новая подстроки (NIL для одних пробелов) */
{
  if (undodir == 0 && t && t->txudeq) {
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
    eubuf = blkmov(os, p, xr*sizeof(tchar)); undogadd(t, UL_GEN);
} }
/*---------------------------------------------------------------------------*/
static void genundo (void) 
{
  small undo_typ = ((textlundo *)ubuf)->utyp & (~U_MARK);
  if (undo_typ == UL_GEN) {
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
  undodir=0;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                         Собственно команды откатки
 */
static void tsundo (BOOL slow, BOOL isLtxt)
{
  txt *t = isLtxt ? Ltxt : Ttxt;
  if (t && t->txudcptr && t->txudeq) {
    while (t->txudcptr) {
      small utyp, undo_typ;
      int real_len;
      DqCopyBackward(t->txudeq, t->txudcptr, ubuf, &real_len);
      utyp = ((textlundo *)ubuf)->utyp;
      undo_typ = utyp & (~U_MARK);
      if (isLtxt && undo_typ != UL_GEN) { Lchange = FALSE; exc(E_UP); }
      t->txudcptr -= real_len; 
      undodir = -1; genundo();
      if (undo_typ != UT_LOAD && (slow || (utyp & U_MARK))) break;
    }
  } else exc(E_NOUNDO);
}
void teundo()  { tsundo(FALSE, FALSE); }
void tesundo() { tsundo(TRUE,  FALSE); }
void leundo()  { tsundo(FALSE, TRUE ); }
void lesundo() { tsundo(TRUE,  TRUE ); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void tsunundo (BOOL slow, BOOL isLtxt)
{
  txt *t = isLtxt ? Ltxt : Ttxt;
  if (t && t->txudeq && t->txudcptr != DqLen(t->txudeq)) {
    small ctr = 0;
    while (t->txudcptr != DqLen(t->txudeq)) {
      small utyp, undo_typ;
      int real_len;
      DqCopyForward(t->txudeq, t->txudcptr, ubuf, &real_len);
      utyp = ((textlundo *)ubuf)->utyp;
      undo_typ = utyp & (~U_MARK);
      if (ctr && undo_typ != UT_LOAD && (slow || (utyp & U_MARK))) break;
      if (!isLtxt && undo_typ == UL_GEN) exc(E_DOWN);
      t->txudcptr += real_len; 
      undodir = 1;  genundo(); ctr++;
    }
  } else exc( E_NOUNDO );
}
void teunundo()  { tsunundo(FALSE, FALSE); }
void tesunundo() { tsunundo(TRUE,  FALSE); }
void leunundo()  { tsunundo(FALSE, TRUE ); }
void lesunundo() { tsunundo(TRUE,  TRUE ); }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                Укоротить буфер откатки до положенной квоты
 */
BOOL udcut (txt *t)
{
  long len, minptr;
  if (t->txudeq == NIL || (len = DqLen(t->txudeq)) <= UDHQUOTA
                       || (minptr = my_min(t->txudcptr, t->txudlptr)) <= 0) 
    return FALSE;
  else {
    int real_len;
    if ((len -= UDLQUOTA) < minptr) minptr = len;
    for (len = 0; len < minptr; ) { 
      DqCopyForward(t->txudeq, len, NIL, &real_len); len += real_len;
    }
    t->txudcptr -= len;
    t->txudlptr -= len;  DqCutB_byX(t->txudeq, len);     return TRUE;
} }
/*---------------------------------------------------------------------------*/

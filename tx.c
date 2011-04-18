/*------------------------------------------------------+----------------------
// МикроМир07              Texts - Тексты               | (c) Epi MG, 2007,2011
//------------------------------------------------------+--------------------*/
#include "mic.h"             /* Old tx.c (c) Attic 1989, (c) EpiMG 1998,2001 */
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
#include "dq.h"
#include "tx.h"
#include "le.h" /* uses Lebuf etc (in TxInfo) */
#include "ud.h"
#include <string.h>
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
txt  *texts;
char *txbuf;
char *afbuf;
static tchar *tcbuf;

local txt *TxNewBuf (void)
{
  txt *t = (txt*) GetMain(MAXFILES * sizeof(txt)), *tprev = NIL;
  int i;
  for (t += MAXFILES, i = MAXFILES; i--; tprev = t) {
    --t;
    t->txnext = tprev;
    t->txstat = t->txlructr = 0; t->file = 0;
  }
  return t;
}
/*-----------------------------------------------------------------------------
 * Каждый текст представляет собой Л2-список, реализованный на двух деках
 * (фактически стеках):
 *                      ustk - верхний стек, записи добавляются в конец
 *                      dstk - нижний  стек, записи добавляются в начало
 *
 * Внимание, при непрерывной реализации деков верхний стек должен располагаться
 * ниже по адресам чем нижний (whatever that means - Epi.)
 */
void TxInit() 
{
  texts = TxNewBuf();                               txbuf = GetMain(MAXLUP+2);
  tcbuf = (tchar*) GetMain(MAXLPAC*sizeof(tchar));  afbuf = GetMain(MAXLUP);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
txt *TxNew (BOOL qundo)                        /* Создание / удаление текста */
{
  txt *t, *tprev;
  for (tprev = NIL, t = texts; t && t->txstat != 0; t = t->txnext) tprev = t;
  if (t == NIL)
    tprev->txnext = t = TxNewBuf();

  t->txustk = DqNew(DT_ASC, 0, STKEXT); TxMarks0(t);
  t->txdstk = DqNew(DT_ASC, STKEXT, 0);
  t->txudeq = qundo ? DqNew(DT_BIN, 0, UNDOEXT) : NIL;
  t->txudcptr = t->txudlptr = 0;    t->txwndptr = NIL;
  t->txlructr = 0;   t->txy = 0;    t->txlm = 0;
             t->cx = t->tcx = 0;    t->txrm =    MAXTXRM;
             t->cy = t->tcy = 0;    t->txstat |= TS_BUSY;  return t;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxMarks0(txt *t) { int i;
                        for (i=0; i<TXT_MARKS; i++) { t->txmarkx[i] = 0;
                                                      t->txmarky[i] = 0; }}
void TxDel (txt *t)
{
  t->txstat = 0; DqDel(t->txustk); t->txustk = NIL;
                 DqDel(t->txdstk); t->txdstk = NIL; QfsClear(t->file);
  if (t->txudeq) DqDel(t->txudeq); t->txudeq = NIL;          t->file = NIL;
}
/*---------------------------------------------------------------------------*/
BOOL qTxUp  (txt *t) { return qDqEmpt(t->txustk); } /* Перемещения по тексту */
BOOL qTxDown(txt *t) { return qDqEmpt(t->txdstk); }
void TxUp   (txt *t)
{
  if (qTxUp(t))       exc(E_MOVUP);
  DqMoveEtoB(t->txustk, t->txdstk); t->txy--;
}
void TxTop (txt *t) { while(!qTxUp(t)) TxUp(t); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDown (txt *t)
{
  if (qTxDown(t))   exc(E_MOVDOWN);
  DqMoveBtoE(t->txdstk, t->txustk); t->txy++;
}
void TxBottom (txt *t) { while (!qTxDown(t)) TxDown(t); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
BOOL TxSetY (txt *t, large y)
{
  while (t->txy < y) { if (qTxDown(t)) return FALSE; TxDown(t); }
  while (t->txy > y) { if (qTxUp(t))   return FALSE; TxUp(t);   } return TRUE;
}
/*--------------------------------------------------- Модификации текста ----*/
void TxDL(txt *t)
{
  if (qTxDown(t)) exc(E_MOVDOWN);
  else {
    small len = DqGetB(t->txdstk, txbuf);   tundo1add(t, UT_DL, txbuf, len);
    int i;
    for (i=0; i<TXT_MARKS; i++) if (t->txmarky[i] > t->txy) t->txmarky[i]--; 
    wndop(TW_DL, t);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDEL_beg(txt *t)
{
  int i, num_deleted = 0;
  if (t->txy < 1) return; /* already in the first line: nothing to delete */

  while (!qDqEmpt(t->txustk)) {
    small len = DqGetE(t->txustk, txbuf);
    t->txy--;
    tundo1add(t, UT_DL, txbuf, len); num_deleted++;
  }
  for (i=0; i<TXT_MARKS; i++) {
    if (t->txmarky[i] > num_deleted) t->txmarky[i] -= num_deleted;
    else                             t->txmarky[i]  = 0;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDEL_end(txt *t)
{
  int i;
  while (!qDqEmpt(t->txdstk)) {
    small len = DqGetB(t->txdstk, txbuf);
    tundo1add(t, UT_DL, txbuf, len);
  }
  for (i=0; i<TXT_MARKS; i++)
    if (t->txmarky[i] > t->txy) t->txmarky[i] = 0;  /* deleting end-of-text  */
}                                                   /* may delete some marks */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxEmpt (txt *t)
{
  deq *d = t->txudeq; DqEmpt(t->txdstk);
                      DqEmpt(t->txustk); if (d) DqEmpt(d);
  t->txy = 0;
  t->txudcptr = 0;   t->cx = t->tcx = 0;      TxMarks0(t);
  t->txudlptr = 0;   t->cy = t->tcy = 0;  wndop(TW_EM, t);
}
/*- - - - - - - - - - - - - - - - - - - - Convert Ascii file string to tchar */
#define isUTFcont(x) ((x & 0xC0) == 0x80)

small aftotc (const char *orig, int len, tchar *dest_buf)
{
  const char *orig_end = orig + ((len < 0) ? (int)strlen(orig) : len);
  tchar attr = 0;                               unsigned /*w,*/ x,y,z;
  small ltc  = 0;
  while (ltc < MAXLPAC && orig < orig_end) {
    char c = *orig++;
    if (c == ACPR && orig < orig_end) {
      attr = (tchar)(AT_IN_FILE & (*orig++ << 16));  continue;
    }
    else if (c == TAB) {
     /* 
      * Replace TAB with proper number of spaces and mark the first space with 
      * AT_TAB attribute:
      */
      *dest_buf++ = (tchar)' ' | AT_TAB | attr;
      for (ltc++; (ltc % TABsize) != 0
                && ltc < MAXLPAC; ltc++) *dest_buf++ = (tchar)' ' | attr;
      continue;
    }
#ifdef notdef
    else if (orig <= orig_end-3 && (c & 0xF8) == 0xF0 && isUTFcont(orig[0])
                                && isUTFcont(orig[1]) && isUTFcont(orig[2])) {
      w = c       & 0x0F;
      x = *orig++ & 0x3F; /* 4-byte:     11110www 10xxxxxx 10yyyyyy 10zzzzzz */
      y = *orig++ & 0x3F; /*                   -> 000wwwxx xxxxyyyy yyzzzzzz */
      z = *orig++ & 0x3F; /*        (not supported due to QChar limitations) */
      *dest_buf++ = ((tchar)w << 18) | 
                    ((tchar)x << 12) | ((tchar)y << 6) | z | attr; }
#endif
    else if (orig <= orig_end-2 && (c & 0xF0) == 0xE0 && isUTFcont(orig[0])
                                                      && isUTFcont(orig[1])) {
      x = c       & 0x0F; 
      y = *orig++ & 0x3F; /* 3-byte:              1110xxxx 10yyyyyy 10zzzzzz */
      z = *orig++ & 0x3F; /*                            -> xxxxyyyy yyzzzzzz */
      *dest_buf++ = ((tchar)x << 12) | ((tchar)y << 6) | z | attr;
    }
    else if (orig <= orig_end-1 && (c & 0xE0) == 0xC0 && isUTFcont(orig[0])) {
      y = c       & 0x1F;
      z = *orig++ & 0x3F; /* 2-byte:  110yyyyy 10zzzzzz -> 00000yyy yyzzzzzz */
      *dest_buf++ = ((tchar)y << 6) | z | attr;
    }
    else if ((c & 0x80) != 0)
         { *dest_buf++ = (ctotc(c) + 0x60) | attr | AT_ITALIC; }
    else { *dest_buf++ =  ctotc(c)         | attr;             }     
    ltc++;
  }
  return ltc;
}
/* - - - - - - - - - - - - - - - - Convert tchar string to ascii file string */

small tctoaf (tchar *orig, int len, char *dest_buf)
{
  char attr = 0, cattr; int laf, itc, j;
  tchar  c, cna;
  for (itc = laf = 0; itc < len && laf < MAXLUP; itc++, laf++) {
    c = orig[itc];
    if ((cattr = (char)((c & AT_IN_FILE) >> 16)) != attr) {
      attr = cattr;
      if (laf < MAXLUP-2) { *dest_buf++ = ACPR;
                            *dest_buf++ = ACPR2 | attr; laf += 2; }
    }
    if (c & AT_TAB) {
     /*
      * T-char has AT_TAB attributes - probably it is the head of the sequence
      * of spaces, if all t-chars until the next tab-stop are spaces, then
      * replace them with single TAB.
      */
      for (j = itc+1; j < len && (j % TABsize) != 0; j++)
        if ((char)orig[j] != ' ') goto vanilla_char;

      *dest_buf++ = TAB;
      itc = j - 1; continue;
    }
vanilla_char:
    cna = c & AT_CHAR;
#ifdef notdef
    if (cna > 0xFFFF && laf < MAXLUP-3) {     /* 000wwwxx xxxxyyyy yyzzzzzz */
      laf += 3;                                              /* plus 3 byte */ 
      *dest_buf++ = (char)(0xF0 | ((cna & 0x1C0000) >> 18)); /* -> 11110www */
      *dest_buf++ = (char)(0x80 | ((cna & 0x03F000) >> 12)); /*    10xxxxxx */
      *dest_buf++ = (char)(0x80 | ((cna & 0x000FC0) >>  6)); /*    10yyyyyy */
      *dest_buf++ = (char)(0x80 |  (cna & 0x00003F)       ); /*    10zzzzzz */
    }
#endif
    if (cna > 0x7FF && laf < MAXLUP-2) {          /* 0000 xxxxyyyy yyzzzzzz */
      laf += 2;                                              /* plus 2 byte */ 
      *dest_buf++ = (char)(0xE0 | ((cna & 0x00F000) >> 12)); /* -> 1110xxxx */
      *dest_buf++ = (char)(0x80 | ((cna & 0x000FC0) >>  6)); /*    10yyyyyy */
      *dest_buf++ = (char)(0x80 |  (cna & 0x00003F)       ); /*    10zzzzzz */
    }
    else if (cna > 0x7F && laf < MAXLUP-1) {           /* 00000yyy yyzzzzzz */
      laf += 1;                                              /* plus 1 byte */ 
      *dest_buf++ = (char)(0xC0 | ((cna & 0x0007C0) >>  6)); /* -> 110yyyyy */
      *dest_buf++ = (char)(0x80 |  (cna & 0x00003F)       ); /*    10zzzzzz */
    }
    else *dest_buf++ = (char)cna;
  }
  return laf;
}
/*---------------------------------------------------------------------------*/
void TxIL(txt *t, char *text, small len)
{
  DqAddB   (t->txdstk, text, len); int i;
  tundo1add(t,  UT_IL, text, len); 
  for (i=0; i<TXT_MARKS; i++)
    if (t->txmarky[i] >= t->txy) t->txmarky[i]++;  wndop(TW_IL, t);
}
void TxTIL (txt *t, tchar *tp, small len)
{
  TxIL(t, afbuf, tctoaf(tp, len, afbuf));
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxRep (txt *t, char *text, small len)
{
  if (!qTxDown(t)) {
    small olen = DqGetB(t->txdstk, txbuf);
    tundo2add(t,  txbuf, olen, text, len);
    DqAddB   (t->txdstk,       text, len); wndop(TW_RP, t);
} }
void TxTRep (txt *t, tchar *tp, small len)
{
  TxRep(t, afbuf, tctoaf(tp, len, afbuf));
}
void TxFRep (txt *t, tchar *tp) { TxTRep(t, tp, lstrlen(MAXLPAC, tp)); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
small TxRead (txt *t, char *tp)
{
  return qTxDown(t) ? 0 : DqCopyB(t->txdstk, tp);
}
small TxTRead (txt *t, tchar *tp)
{
  if (qTxDown(t)) {
    small  len = aftotc        (MSG_EOF, -1, tp);
    return len + QfsFullName2tc(t->file, tp+len);
  } 
  else { small laf =  TxRead(t, txbuf);
         return aftotc(txbuf, laf, tp); }
}
small TxFRead (txt *t, tchar *tp)
{
  int len =  TxTRead(t, tp);
  blktspac(tp+len, MAXLPAC-len); return len;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
tchar *TxInfo (wnd *w, large y, int *pl)      /* used for repaint in vip.cpp */
{
  txt *t = w->wtext;
  if (!t || TxSetY(t, (y += w->wty)) == FALSE) { *pl = 0; return NIL; }
  else {
    tchar *tc;
    if (w == Lwnd && y == Ly) tc = Lebuf;
    else {       
      small len = TxFRead(t, tc = tcbuf); int i;
      if (y > 0 && len < MAXLPAC-3) {
        for (i=1; i<TXT_MARKS; i++)
          if (t->txmarky[i] == y) { tc[len+1] = (i+'0')| AT_MARKFLG;
                                    tc[len+2] =    ' ' | AT_MARKFLG; break; }
    } }
    tc +=         w->wtx;
    *pl = MAXLPAC-w->wtx; return tc;
} }
/*---------------------------------------------------------------------------*/

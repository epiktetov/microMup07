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
#include "synt.h"
#include <string.h>
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
txt *texts  =  NIL;  static tchar tcbuf[MAXLPAC]; int tcbuflen = 0;
char afbuf[MAXLUP];
char txbuf[MAXLUP+2];
/*-----------------------------------------------------------------------------
 * Каждый текст представляет собой Л2-список, реализованный на двух деках
 * (фактически стеках):
 *                      ustk - верхний стек, записи добавляются в конец
 *                      dstk - нижний  стек, записи добавляются в начало
 *
 * Внимание, при непрерывной реализации деков верхний стек должен располагаться
 * ниже по адресам чем нижний (whatever that means - Epi.)
 */
void TxInit() { blktspac(tcbuf, MAXLPAC); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
txt *TxNew (BOOL qundo)                        /* Создание / удаление текста */
{
  txt *t, *tprev;
  for (tprev = NIL, t = texts; t && t->txstat != 0; t = t->txnext) tprev = t;
  if (t == NIL) {
    t = (txt*)GetMain(sizeof(txt)); /* если не нашли свободного дескриптора: */
    t->txstat = 0;     t->file = 0; /* сделаем новый, добавим в конец списка */
    t->txnext = NIL;                /* (не было предыдущего = список пустой) */
    if (tprev) tprev->txnext = t;
    else               texts = t;
  }
  t->txustk = DqNew(DT_TEXT, 0, MAXLUP); t->txudfile = -1; TxMarks0(t);
  t->txdstk = DqNew(DT_TEXT, MAXLUP, 0);
  t->txudeq = qundo ? DqNew(DT_UNDO, 0, UDLQUOTA) : NIL; t->txlm = 0;
  t->txudcptr = t->txudlptr  = 0;     t->txwndptr = NIL; t->txrm =    MAXTXRM;
  t->clustk   = t->cldstk    = 0;     t->txlructr = 0;   t->txstat |= TS_BUSY;
  t->clang       = 0; t->txy = 0;
  t->cx = t->tcx = 0; memset(t->thisSynts, 0xDE, sizeof(int) * MAXSYNTBUF);
  t->cy = t->tcy = 0; memset(t->prevSynts,    0, sizeof(int) * MAXSYNTBUF);
  t->maxTy       = 0; memset(t->lastSynts,    0, sizeof(int) * MAXSYNTBUF);
  return t;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxEnableSynt (txt *t, small clang)       /* add deqs for syntax checker */
{
  if (( t->clang = clang ) && t->clustk == NIL && t->cldstk == NIL) {
    t->clustk = DqNew(DT_SYNT, 0, MAXLPAC);
    t->cldstk = DqNew(DT_SYNT, MAXLPAC, 0);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxMarks0(txt *t) { int i;
                        for (i=0; i<TXT_MARKS; i++) { t->txmarkx[i] =  0;
                                                      t->txmarky[i] = -1; }}
void TxDel (txt *t)
{
  t->txstat = 0; DqDel(t->txustk); t->txustk = NIL; QfsClear(t->file);
  t->maxTy  = 0; DqDel(t->txdstk); t->txdstk = NIL;          t->file = NIL;
  if (t->txudeq) DqDel(t->txudeq); t->txudeq = NIL;
  if (t->clustk) DqDel(t->clustk); t->clustk = NIL;
  if (t->cldstk) DqDel(t->cldstk); t->cldstk = NIL; t->clang = 0;
}
/*---------------------------------------------------------------------------*/
BOOL qTxTop   (txt *t) { return qDqEmpt(t->txustk); }
BOOL qTxBottom(txt *t) { return qDqEmpt(t->txdstk); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxUp (txt *t)                                  /* Перемещения по тексту */
{
  if (qTxTop(t)) exc(E_MOVUP);
  else {                    /* now inline: DqMoveEtoB(t->txustk, t->txdstk); */
    int len, real_len;
    char *pb;  extgap(t->txdstk, MAXLUP, TRUE);
    pb = DqLookupBack(t->txustk, DqLen(t->txustk), &len, &real_len);
    DqAddB(t->txdstk, pb, len);        t->txustk->dend -= real_len;
    t->txy--;
    if (t->clustk && !qDqEmpt(t->clustk)) {
      len = DqGetE(t->clustk, (char*)(t->thisSynts));
            DqAddB(t->cldstk, (char*)(t->thisSynts), len);
      if (qDqEmpt(t->clustk)) t->prevSynts[0] = 0;
      else
        DqCopyBackward(t->clustk, DqLen(t->clustk), (char*)(t->prevSynts), 0);
} } }
void TxTop (txt *t) { while(!qTxTop(t)) TxUp(t); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDown (txt *t)
{
  if (qTxBottom(t)) exc(E_MOVDOWN); /* was DqMoveBtoE(t->txdstk, t->txustk); */
  else {
    int len, real_len;
    char *pb;  extgap(t->txustk, MAXLUP, FALSE);
    pb = DqLookupForw(t->txdstk, 0, &len, &real_len);
    DqAddE(t->txustk, pb, len); t->txdstk->dbeg += real_len;
    t->txy++;
    if (t->clustk) {
      if (qDqEmpt(t->cldstk)) 
           len = SyntParse (t, pb, len,    t->prevSynts) * sizeof(int);
      else len = DqGetB(t->cldstk, (char*)(t->prevSynts));
      DqAddE(t->clustk, (char*)(t->prevSynts), len);
      if (qTxBottom(t)) {    t->thisSynts[0] = 0xE0;
        blkmov(t->prevSynts, t->lastSynts, len);
      }
      else if (qDqEmpt(t->cldstk))        t->thisSynts[0] = 0xDE;
      else DqCopyForward(t->cldstk, 0, (char*)(t->thisSynts), 0);
} } }
void TxBottom (txt *t) { while (!qTxBottom(t)) TxDown(t); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
BOOL TxSetY (txt *t, large y)
{
  while(t->txy < y) { if (qTxBottom(t)) return FALSE; TxDown(t); }
  while(t->txy > y) { if (qTxTop(t))    return FALSE; TxUp(t);   } return TRUE;
}
/*--------------------------------------------------- Модификации текста ----*/
void TxDL(txt *t)
{
  if (qTxBottom(t)) exc(E_MOVDOWN);
  else {
    small len = DqGetB(t->txdstk, txbuf);   tundo1add(t, UT_DL, txbuf, len);
    int i;
    for (i=0; i<TXT_MARKS; i++) if (t->txmarky[i] > t->txy) t->txmarky[i]--; 
    t->maxTy--;                                             wndop(TW_DL, t);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDEL_beg(txt *t)
{
  int i, num_deleted = 0;
  if (t->txy < 1) return; /* already in the first line: nothing to delete */

  while (!qDqEmpt(t->txustk)) {
    small len = DqGetE(t->txustk, txbuf);
    t->txy--;
    tundo1add(t, UT_DL, txbuf, len); num_deleted++; t->maxTy--;
  }
  for (i=0; i<TXT_MARKS; i++) {
    if (t->txmarky[i] > num_deleted) t->txmarky[i] -= num_deleted;
    else                             t->txmarky[i]  = -1;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDEL_end(txt *t)
{
  t->maxTy = t->txy;
  int i;
  while (!qDqEmpt(t->txdstk)) {
    small len = DqGetB(t->txdstk, txbuf);
    tundo1add(t, UT_DL, txbuf, len);
  }
  for (i=0; i<TXT_MARKS; i++)
    if (t->txmarky[i] > t->txy) t->txmarky[i] = -1; /* deleting end-of-text  */
}                                                   /* may delete some marks */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxEmpt (txt *t)
{                    if (t->txudeq) DqEmpt(t->txudeq);
  DqEmpt(t->txdstk); if (t->clustk) DqEmpt(t->clustk);
  DqEmpt(t->txustk); if (t->cldstk) DqEmpt(t->cldstk);
  t->maxTy = t->txy = 0;
  t->txudcptr = t->txudlptr = 0; t->cx = t->tcx = 0;
  t->txudfile = -1;              t->cy = t->tcy = 0;     TxMarks0(t);
  memset(t->lastSynts, 0, sizeof(int) * MAXSYNTBUF); wndop(TW_EM, t);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxIL(txt *t, char *text, small len)
{
  DqAddB   (t->txdstk, text, len); t->maxTy++;
  tundo1add(t,  UT_IL, text, len); 
  int i;
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
  if (!qTxBottom(t)) {
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
  return qTxBottom(t) ? 0 : DqCopyForward(t->txdstk,0, tp,0);
}
small TxTRead (txt *t, tchar *tp)
{
  if (qTxBottom(t)) {
    small  len = aftotc(AF_DIRTY "^^" AF_NONE " end of ", -1, tp);
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
/*- - - - - - - - - - - - - - - - - - - - Convert Ascii file string to tchar */
// Some attributes are saved in file:
//
//   U+281 latin letter small capital inverted R -- starts ʁboldʀ text
//   U+282 latin small letter S with hook        -- starts ʂdark blueʀ (prompt)
//   U+284 - - - dotless J with stroke and hook  -- starts ʄdark redʀ (regex)
//   U+288 - - - T with retroflex hook           -- starts ʈsky blueʀ (special)
//   U+290 - - - Z with retroflex hook           -- starts AT_DIRTY attribute
//   U+280 latin letter small capital R          -- indicates end of attributes
//
// Unicode character from U+280 to U+29F (UTF-8 from \xCA\x80 to \xCA\x9F) thus
// cannot appear in µMup07 text (they all belong to rarely used IPA extensions)
//
small aftotc (const char *orig, int len, tchar *dest_buf)
{
  const char *orig_end = orig + ((len < 0) ? (int)strlen(orig) : len);
  tchar attr = 0;                                      unsigned x,y,z;
  small ltc  = 0;
  while (ltc < MAXLPAC && orig < orig_end) {
    char c = *orig++;
    if (c == ACPR && orig < orig_end && (*orig & 0xC0) == 0x80) {
      attr = (tchar)(AT_IN_FILE & (*orig++ << 16));    continue;
    }
    else if (c == TAB) { /* Replace TAB with proper number of spaces and mark 
                          * the first space with AT_TAB attribute (+keep attr)
                          */
      *dest_buf++ = (tchar)' ' | AT_TAB | attr;
      for (ltc++; (ltc % TABsize) != 0
                && ltc < MAXLPAC; ltc++) *dest_buf++ = (tchar)' ' | attr;
      continue;
    }
#define isUTFcont(x) ((x & 0xC0) == 0x80)
#ifdef notdef
    else if (orig <= orig_end-3 && (c & 0xF8) == 0xF0 && isUTFcont(orig[0])
                                && isUTFcont(orig[1]) && isUTFcont(orig[2])) {
      w = c       & 0x0F;
      x = *orig++ & 0x3F; /* 4-byte:     11110www 10xxxxxx 10yyyyyy 10zzzzzz */
      y = *orig++ & 0x3F; /*                   -> 000wwwxx xxxxyyyy yyzzzzzz */
      z = *orig++ & 0x3F; /*           (unsupported due to QChar limitation) */
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
     { if (c & 0x40) { *dest_buf++ = (ctotc(c) + 0x350) | attr | AT_BADCHAR; }
       else          { *dest_buf++ = (ctotc(c) +  0x60) | attr | AT_BADCHAR; }}
    else             { *dest_buf++ =  ctotc(c)          | attr;               }
    ltc++;
  }
  return ltc;
}
/* - - - - - - - - - - - - - - - - Convert tchar string to ascii file string */
//
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
    cna = c & AT_CHAR; /* = character sans attributes */
    if (c & AT_TAB) { 
      for (j = itc+1; j < len && (j % TABsize) != 0; j++)
        if ((char)orig[j] != ' ') goto vanilla_char;
      /*  ^
       * Replacing space with AT_TAB back to TAB only works if all tchars up to
       *  the next tab-stop are spaces, otherwise attribute is silently ignored
       */
      *dest_buf++ = TAB; itc = j - 1; continue;
    }
    if (c & AT_BADCHAR) {
           if ( 0xE0 <= cna && cna <= 0x11F) *dest_buf++ = (char)(cna -  0x60);
      else if (0x410 <= cna && cna <= 0x44F) *dest_buf++ = (char)(cna - 0x350);
      else goto vanilla_char;                                         continue;
    }
vanilla_char: 
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
  while (laf && *(--dest_buf) == ' ') laf--; /* remove trailing spaces */
  return laf;
}
/*---------------------------------------------------------------------------*/
tchar txFlags[TXT_MARKS] = { 0, AT_MARKFLG + AT_BG_RED + 0xB9,     /* red  ¹ */
                                AT_MARKFLG             + 0xB2,     /* brown² */
                                AT_MARKFLG + AT_BG_BLU + 0xB3,     /* blue ³ */
                                AT_MARKFLG + AT_BG_GRN + 0x2074 }; /* green⁴ */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
tchar *TxInfo (wnd *w, large y, int *pl)      /* used for repaint in vip.cpp */
{                                             /* to get current char mim.cpp */
  txt *t = w->wtext;                          /* (return buf may be changed) */
  int len = 0;
  if (tcbuf[0] & AT_DIRTY) { blktspac(tcbuf, MAXLPAC); tcbuflen = 0; }
  if (t && TxSetY(t, y)) {
    if (w == Lwnd && y == Ly) blktmov(Lebuf, tcbuf, len = Lleng);
    else                                 len = TxTRead(t, tcbuf);
    if (t->maxTy < y) t->maxTy = y;
    if (y > 0 && len < MAXLPAC-3) {
      int i;
      for (i=1; i<TXT_MARKS; i++) {
        if (t->txmarky[i] == y) {
          if (len < w->wsw-1) tcbuf[len++] = ' ';
                              tcbuf[len++] =      txFlags[i];
                              tcbuf[len++] = ' '|(txFlags[i] & AT_ALL); break;
  } } } }
  if (len < tcbuflen) blktspac(tcbuf+len, tcbuflen - len);
  *pl = tcbuflen = len;                      return tcbuf;
}
void TxRecalcMaxTy (txt *t) { TxBottom(t); t->maxTy = t->txy; }
/*---------------------------------------------------------------------------*/

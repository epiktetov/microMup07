/*------------------------------------------------------+----------------------
// МикроМир07              Texts - Тексты               | (c) Epi MG, 2007-2022
//------------------------------------------------------+--------------------*/
#include "mic.h"             /* Old tx.c (c) Attic 1989, (c) EpiMG 1998,2001 */
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
txt *TxNew(bool qundo) /* Создание текста / удаление оного, точнее - очистка */
{                      /* (структура переиспользуется, никогда не удаляется) */
  txt *t, *tprev;
  for (tprev = NIL, t = texts; t && t->txstat != 0; t = t->txnext) tprev = t;
  if (t == NIL) {
    t = (txt*)xmalloc(sizeof(txt)); /* если не нашли свободного дескриптора: */
    memset(t,   0,    sizeof(txt)); /* сделаем новый, добавим в конец списка */
    t->txstat = 0;     t->file = 0; /* (не было предыдущего = список пустой, */
    t->txnext = NIL;                /*         добавим ссылку на этот текст) */
    if (tprev) tprev->txnext = t;
    else               texts = t;
  }
  t->txustk = DqNew(DT_TEXT, 0, MAXLUP, t); t->txudfile = -1; TxMarks0(t);
  t->txdstk = DqNew(DT_TXTD, MAXLUP, 0, t);
  t->txudeq = qundo ? DqNew(DT_UNDO, 0, UDLQUOTA, t) : NIL;
  t->txlm = 0;
  t->txrm = MAXTXRM;
  t->txudcptr = t->txudlptr = 0; t->txwndptr = NIL;  t->txstat |= TS_BUSY;
  t->clustk   = t->cldstk   = 0; t->txlructr = 0;
  t->clang  = 0;
  t->vp_ctx = t->vp_wtx = 0; t->txy = t->maxTy = 0;
  t->vp_cty = t->vp_wty = 0;
  memset(t->thisSynts, 0xDE, sizeof(int) * MAXSYNTBUF); t->luaTxid = 0;
  memset(t->prevSynts,    0, sizeof(int) * MAXSYNTBUF);
  memset(t->lastSynts,    0, sizeof(int) * MAXSYNTBUF);       return t;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxEnableSynt (txt *t, short clang)       /* add deqs for syntax checker */
{
  if (( t->clang = clang ) && t->clustk == NIL && t->cldstk == NIL) {
    t->clustk = DqNew(DT_SYNT, 0, MAXLPAC, t);
    t->cldstk = DqNew(DT_SYNT, MAXLPAC, 0, t);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxMarks0 (txt *t)        /* clear (set to zero) all marks in given text */
{
  for (int i = 0; i < TXT_MARKS; i++) TxUnmark(i, t);
}
void TxUnmark (int i, txt *t) {            t->txmarkx[i] =   0;
                                           t->txmarky[i] =  -1;
  if (t->txmarks[i]) xfree(t->txmarks[i]); t->txmarks[i] = NIL;
}
void TxUnmarkY (txt *t, long y)
{
  for (int i = 1; i < TXT_MARKS; i++) if (t->txmarky[i] == y) TxUnmark(i, t);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDel (txt *t) /* Delete the contents (do not de-allocate 'txt' itself) */
{
  t->txstat = 0; DqDel(t->txustk); t->txustk = NIL; QfsClear(t->file);
  t->maxTy  = 0; DqDel(t->txdstk); t->txdstk = NIL; t->file =  NIL;
  if (t->txudeq) DqDel(t->txudeq); t->txudeq = NIL; t->clang =   0;
  if (t->clustk) DqDel(t->clustk); t->clustk = NIL; t->luaTxid = 0;
  if (t->cldstk) DqDel(t->cldstk); t->cldstk = NIL;    TxMarks0(t);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDiscard (txt *t) /* Discard the contents of the file, and UNDO & Synt */
{
  DqEmpt(t->txdstk); if (t->clustk) DqEmpt(t->clustk);
  DqEmpt(t->txustk); if (t->cldstk) DqEmpt(t->cldstk);
                     if (t->txudeq) DqEmpt(t->txudeq);
  t->txudfile = -1;
  t->txudcptr = t->txudlptr = 0; t->vp_ctx = t->vp_wtx = 0;
  t->maxTy    = t->txy      = 0; t->vp_cty = t->vp_wty = 0;
  t->txstat  &= ~TS_FILE;
  memset(t->thisSynts, 0, sizeof(int) * MAXSYNTBUF);
  memset(t->prevSynts, 0, sizeof(int) * MAXSYNTBUF);
  memset(t->lastSynts, 0, sizeof(int) * MAXSYNTBUF); TxMarks0(t);
}
void TxEmpt(txt *t) { TxDiscard(t); wndop(TW_EM,t); } /* make the text Empty */
/*---------------------------------------------------------------------------*/
bool qTxBottom(txt *t) { return qDqEmpt(t->txdstk); }
bool qTxTop   (txt *t)
{                           // cleanup prevSynts and upper stack for Synts when
  if (qDqEmpt(t->txustk)) { // at the top of text - just in case we lost tracks
    t->prevSynts[0] = 0;
    if (t->clustk) DqEmpt(t->clustk); return  TRUE;
  }                              else return FALSE;
}
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
bool TxSetY (txt *t, long y)
{
  while(t->txy < y) { if (qTxBottom(t)) return FALSE; TxDown(t); }
  while(t->txy > y) { if (qTxTop(t))    return FALSE; TxUp(t);   } return TRUE;
}
/*--------------------------------------------------- Модификации текста ----*/
void TxDL(txt *t)
{
  if (qTxBottom(t)) exc(E_MOVDOWN);
  else {
    short len = DqGetB(t->txdstk, txbuf); tundo1add(t, UT_DL, txbuf, len);
    if (t->cldstk && !qDqEmpt(t->cldstk))
                       DqGetB(t->cldstk, (char*)(t->thisSynts));
    int i;
    for (i=0; i<TXT_MARKS; i++) {
      if (t->txmarky[i] < t->txy) continue;
      if (t->txmarky[i] > t->txy) t->txmarky[i]--;
      else                        TxUnmark (i, t);
    }
    t->maxTy--; wndop(TW_DL, t);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDEL_beg(txt *t)
{
  int i, num_deleted = 0;
  if (t->txy < 1) return; /* already in the first line: nothing to delete */

  while (!qDqEmpt(t->txustk)) {
    short len = DqGetE(t->txustk, txbuf);
    t->txy--;
    tundo1add(t, UT_DL, txbuf, len); num_deleted++; t->maxTy--;
  }
  if (t->clustk && !qDqEmpt(t->clustk)) DqEmpt(t->clustk);
  for (i=0; i<TXT_MARKS; i++) {
    if (t->txmarky[i] > num_deleted) t->txmarky[i] -= num_deleted;
    else                                           TxUnmark(i, t);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxDEL_end(txt *t)
{
  t->maxTy = t->txy;
  while (!qDqEmpt(t->txdstk)) {
    short len = DqGetB(t->txdstk, txbuf);
    tundo1add(t, UT_DL, txbuf, len);
  }
  if (t->cldstk && !qDqEmpt(t->cldstk)) DqEmpt(t->cldstk);
  for (int i=0; i<TXT_MARKS; i++)
    if (t->txmarky[i] > t->txy) t->txmarky[i] = -1; /* deleting end-of-text  */
}                                                   /* may delete some marks */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxIL(txt *t, char *text, short len)
{
  DqAddB   (t->txdstk, text, len); t->maxTy++;
  tundo1add(t,  UT_IL, text, len); 
  if (t->cldstk) {
    int clen = SyntParse(t, text, len, t->thisSynts) * sizeof(int);
    DqAddB (t->cldstk,         (char*)(t->thisSynts),        clen);
  }
  for (int i=0; i<TXT_MARKS; i++)
    if (t->txmarky[i] >= t->txy) t->txmarky[i]++;  wndop(TW_IL, t);
}
void TxTIL (txt *t, tchar *tp, short len)
{
  TxIL(t, afbuf, tctoaf(tp, len, afbuf));
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void TxRep (txt *t, char *text, short len)
{
  if (!qTxBottom(t)) {                  // delete mark (4+) in the edited line
    for (int i = 4; i < TXT_MARKS; i++) //      (likely not applicable anymore)
      if (t->txmarky[i] == t->txy) TxUnmark(i,t);
    short olen = DqGetB(t->txdstk, txbuf);
    tundo2add(t,  txbuf, olen, text, len);
    DqAddB   (t->txdstk,       text, len); wndop(TW_RP, t);
} }
void TxTRep (txt *t, tchar *tp, short len)
{
  TxRep(t, afbuf, tctoaf(tp, len, afbuf));
}
void TxFRep (txt *t, tchar *tp) { TxTRep(t, tp, lstrlen(MAXLPAC, tp)); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
char *TxGetLn(txt *t, int *len) { return DqLookupForw(t->txdstk,0, len,0); }
short TxRead (txt *t, char *tp)
{
  return qTxBottom(t) ? 0 : DqCopyForward(t->txdstk,0, tp,0);
}
short TxTRead (txt *t, tchar *tp)
{
  if (qTxBottom(t)) {
    short  len = aftotc("^^ end of ",-1, tp);
    return len + QfsFullName2tc(t->file, tp+len);
  } 
  else { short laf =  TxRead(t, txbuf);
         return aftotc(txbuf, laf, tp); }
}
short TxFRead (txt *t, tchar *tp)
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
short aftotc (const char *orig, int len, tchar *dest_buf)
{
  const char  *orig_end = orig + ((len < 0) ? (int)strlen(orig) : len);
  tchar attr = 0, *dest = dest_buf;                     unsigned x,y,z;
  short ltc  = 0;
  while (ltc < MAXLPAC && orig < orig_end) {
    char c = *orig++;
    if (c == ACPR && orig < orig_end && (*orig & 0xC0) == 0x80) {
      attr = (tchar)(AT_IN_FILE & (*orig++ << 16));    continue;
    }
    else if (c == CR) *dest++ = (tchar)';' | AT_TAB | attr;
    //
    // Unfold '\t' from file: Replace TAB with proper number of spaces and mark
    // the first space with AT_TAB attribute (keeping other axisting attrs too)
    //
    else if (c == TAB) {
      *dest++ = (tchar)' ' | AT_TAB | attr;
      for (ltc++; (ltc % TABsize) != 0  && ltc < MAXLPAC; ltc++)
                             *dest++ = (tchar)' ' | attr; continue;
    }
    // Convert "X\bY" into bold 'Y', provided 'Y' is one-byte ASCII character,
    // to make t/n/groff output to look nice (so the «man xxx» is readable)
    //
    else if (c == '\b') {
      if (dest > dest_buf) {
        if (orig < orig_end && ' ' < *orig && *orig <= '~')
             dest[-1] = (tchar)(*orig++) | AT_BOLD;
        else dest--; //
      }    continue; // if 'Y' not in the string or not ASCII, just remove last
    }                //                      element in dest buffer (if exists)
#define isUTFcont(x) ((x & 0xC0) == 0x80)
#ifdef notdef
    else if (orig <= orig_end-3 && (c & 0xF8) == 0xF0 && isUTFcont(orig[0])
                                && isUTFcont(orig[1]) && isUTFcont(orig[2])) {
      w = c       & 0x0F;
      x = *orig++ & 0x3F; /* 4-byte:     11110www 10xxxxxx 10yyyyyy 10zzzzzz */
      y = *orig++ & 0x3F; /*                   -> 000wwwxx xxxxyyyy yyzzzzzz */
      z = *orig++ & 0x3F; /*           (unsupported due to QChar limitation) */
      *dest++ = ((tchar)w << 18) |
                ((tchar)x << 12) | ((tchar)y << 6) | z | attr; }
#endif
    else if (orig <= orig_end-2 && (c & 0xF0) == 0xE0 && isUTFcont(orig[0])
                                                      && isUTFcont(orig[1])) {
      x = c       & 0x0F; 
      y = *orig++ & 0x3F; /* 3-byte:              1110xxxx 10yyyyyy 10zzzzzz */
      z = *orig++ & 0x3F; /*                            -> xxxxyyyy yyzzzzzz */
      *dest++ = ((tchar)x << 12) | ((tchar)y << 6) | z | attr;
    }
    else if (orig <= orig_end-1 && (c & 0xE0) == 0xC0 && isUTFcont(orig[0])) {
      y = c       & 0x1F;
      z = *orig++ & 0x3F; /* 2-byte:  110yyyyy 10zzzzzz -> 00000yyy yyzzzzzz */
      *dest++ = ((tchar)y << 6) | z | attr;
    }
    else if ((c & 0x80) != 0)
     { if (c & 0x40)  { *dest++ = (ctotc(c)+0x350) | attr | AT_BADCHAR; }
       else           { *dest++ = (ctotc(c)+ 0x60) | attr | AT_BADCHAR; }}
    else if (c < ' ') { *dest++ = (ctotc(c)+ '@' ) | attr | AT_TAB;      }
    else              { *dest++ =  ctotc(c)        | attr;               }
    ltc++;
  }
  return ltc;
}
/* - - - - - - - - - - - - - - - - Convert tchar string to ascii file string */
//
short tctoaf (tchar *orig, int len, char *dest_buf)
{
  char attr = 0, cattr; int laf, itc, j;
  tchar  c, csa;
  for (itc = laf = 0; itc < len && laf < MAXLUP; itc++, laf++) {
    c = orig[itc];
    if ((cattr = (char)((c & AT_IN_FILE) >> 16)) != attr) {
      attr = cattr;
      if (laf < MAXLUP-2) { *dest_buf++ = ACPR;
                            *dest_buf++ = ACPR2 | attr; laf += 2; }
    }
    csa = c & AT_CHAR; /* = character sans attributes */
    if (c & AT_TAB) { 
      switch (csa) {
       default: *dest_buf++ = (char)(csa - '@'); break; /* control character */
      case ';':
        if (itc == len-1) *dest_buf++ = CR; /* convert CR at the end of line */
        else       laf--;            break; /* remove it from text elsewhere */
      case ' ':
        for (j = itc+1; j < len && (j % TABsize) != 0
                                && (char)orig[j] == ' '; j++) ;
        *dest_buf++ = TAB;
        itc = j-1; /* skip all spaces up to next TAB stop or first non-space */
    } }
    else if (c & AT_BADCHAR) {
           if ( 0xE0 <= csa && csa <= 0x11F) *dest_buf++ = (char)(csa -  0x60);
      else if (0x410 <= csa && csa <= 0x44F) *dest_buf++ = (char)(csa - 0x350);
    }
#ifdef notdef
    if (csa > 0xFFFF && laf < MAXLUP-3) {     /* 000wwwxx xxxxyyyy yyzzzzzz */
      laf += 3;                                              /* plus 3 byte */ 
      *dest_buf++ = (char)(0xF0 | ((csa & 0x1C0000) >> 18)); /* -> 11110www */
      *dest_buf++ = (char)(0x80 | ((csa & 0x03F000) >> 12)); /*    10xxxxxx */
      *dest_buf++ = (char)(0x80 | ((csa & 0x000FC0) >>  6)); /*    10yyyyyy */
      *dest_buf++ = (char)(0x80 |  (csa & 0x00003F)       ); /*    10zzzzzz */
    }
#endif
    else if (csa > 0x7FF && laf < MAXLUP-2) {     /* 0000 xxxxyyyy yyzzzzzz */
      laf += 2;                                              /* plus 2 byte */ 
      *dest_buf++ = (char)(0xE0 | ((csa & 0x00F000) >> 12)); /* -> 1110xxxx */
      *dest_buf++ = (char)(0x80 | ((csa & 0x000FC0) >>  6)); /*    10yyyyyy */
      *dest_buf++ = (char)(0x80 |  (csa & 0x00003F)       ); /*    10zzzzzz */
    }
    else if (csa > 0x7F && laf < MAXLUP-1) {           /* 00000yyy yyzzzzzz */
      laf += 1;                                              /* plus 1 byte */ 
      *dest_buf++ = (char)(0xC0 | ((csa & 0x0007C0) >>  6)); /* -> 110yyyyy */
      *dest_buf++ = (char)(0x80 |  (csa & 0x00003F)       ); /*    10zzzzzz */
    }
    else *dest_buf++ = (char)csa;
  }
  while (laf && *(--dest_buf) == ' ') laf--; /* remove trailing spaces */
  return laf;
}
/*---------------------------------------------------------------------------*/
tchar TxChar (txt *t, int x, long y) /* get one tchar for given text and pos */
{
  if (x >= 0 && TxSetY(t, y)) { int     len = TxTRead(t, tcbuf);
                                return (len > x) ? tcbuf[x] : 0; }
  else return 0;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
tchar *TxInfo(wnd *w, long y, int *pl) /* TxInfo used for repaint in vip.cpp */
{                                      /*    and to get current char mim.cpp */
  int len = 0, i, x;                    /* (return buffer may be changed)    */
  txt *t = w->wtext;
  if (t && TxSetY(t, y)) { if (t->maxTy < y) // NOTE: must call TxSetY even if
                               t->maxTy = y; //  using Lwnd (for SyntColorize)
    //
    if (w == Lwnd && y == Ly) blktmov(Lebuf, tcbuf, len = Lleng );
    else                                  len = TxTRead(t, tcbuf);
  }
  if (len < tcbuflen) blktspac(tcbuf+len, tcbuflen-len);
  if (t) {
    for (i = 1; i < TXT_MARKS; i++) { if (t->txmarky[i] != y) continue;
                                      x = t->txmarkx[i];
    //
    // Marker character temporarily replaces space in the text, but not shown
    // if that position is non-blank (but attributes are copied in any case):
    //
       tchar attr = tATTR(t->txmarkt[i]);
       if (tcharIsBlank(tcbuf[x])) tcbuf[x] = t->txmarkt[i];
       else                        tcbuf[x] = (tcbuf[x] & AT_CHAR)|attr; x++;
       if (x < MAXLPAC-1)        { tcbuf[x] = (tcbuf[x] & AT_CHAR)|attr; x++; }
       if (len < x) len = x;
  } }
  *pl = tcbuflen = len; return tcbuf;
}
void TxRecalcMaxTy (txt *t) { TxBottom(t); t->maxTy = t->txy; }
/*---------------------------------------------------------------------------*/

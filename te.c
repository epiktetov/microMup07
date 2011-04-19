/*------------------------------------------------------+----------------------
// МикроМир07    te = Text editor - Редактор текста     | (c) Epi MG, 2006-2011
//------------------------------------------------------+--------------------*/
#include "mic.h"          /* Old te.c (c) Attic 1989-96, (c) EpiMG 1998,2001 */
#include "ccd.h"
#include "twm.h"
#include "vip.h"
#include "clip.h"
#include "le.h"
#include "te.h"
#include "tx.h"
#include "ud.h"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd  *Twnd = NIL;                     /* Окно, в котором редактируется текст */
txt  *Ttxt;                           /* Редактируемый текст                 */
small Tx;                             /* X курсора в тексте                  */
large Ty;                             /* Y курсора в тексте                  */
/*---------------------------------------------------------------------------*/
void qsety (large y)
{
  BOOL q = TxSetY(Ttxt, y); Ty = Ttxt->txy;
  if (!q) exc(E_SETY);
}
BOOL tesetxy (small x, large y)
{
  BOOL q = TxSetY(Ttxt,y); Tx = x;
                           Ty = Ttxt->txy; return q;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tesmark() { Ttxt->txmarkx[0] = Tx;
                 Ttxt->txmarky[0] = Ty; }
void tecmark() 
{
  int tmp = Ttxt->txmarkx[0]; Ttxt->txmarkx[0] = Tx; Tx = tmp;
      tmp = Ttxt->txmarky[0]; Ttxt->txmarky[0] = Ty; qsety(tmp);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tesmarkN()                       /* Set/clear mark N | NOTE: codes must */
{                                     /* and go to mark N |  be TE_S/CMARK+N */
  int N = KbCode - TE_SMARK;
  if (Ttxt->txmarky[N] != Ty) { Ttxt->txmarkx[N] = Tx;
                                Ttxt->txmarky[N] = Ty; wndop(TW_ALL, Ttxt); }
  else { Ttxt->txmarkx[N] = 0;
         Ttxt->txmarky[N] = 0; wndop(TW_RP, Ttxt); }
}
void tecmarkN()
{
  int N = KbCode - TE_CMARK;
  if (Ttxt->txmarky[N] > 0) { tesmark(); Tx =  Ttxt->txmarkx[N];
                                         qsety(Ttxt->txmarky[N]); }
}
/*---------------------------------------------------------------------------*/
void tedown() { Ty++; }  void tetbeg() { tesetxy(Tx,          0); }
void teup  () { Ty--; }  void tetend() { tesetxy(Tx, 2147483647); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tecentr()             /* Esc NN Ctrl+E => В строку с номером KbCount-1  */
{                          /*          else => Текущую строку в центр экрана */
  small whh;
  large y;
  if (KbRadix != 0) qsety(KbCount ? KbCount-1 : 0); 
  else {
    wadjust(Twnd, Tx, Ty); whh = (Twnd->wsh-1) >> 1; y = my_max(Ty-whh, 0);
    wadjust(Twnd, Tx,  y); while (!qTxDown(Ttxt) && whh--) TxDown(Ttxt);
    wadjust(Twnd, Tx, Ttxt->txy);
} }
/*---------------------------------------------------------------------------*/
void teCR()                    /* dirty hack for micros.dir: if in the right */
{                              /* (filename) field, return back to comment   */  
  if ((Ttxt->txstat & TS_MCD)  /* field, otherwise just go to the beginning  */
      && Tx > MCD_LEFT) {      /* of new line                                */
    Ttxt->txlm = 0;
    Ttxt->txrm = MCD_LEFT; Tx = 0;
  }
  else { Ty++; Tx = Ttxt->txlm; }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teRCR() 
{            
  int Txrm; Lleng = TxTRead(Ttxt, Lebuf);
  Txrm = my_min(Lleng, Ttxt->txrm); 
  for (Tx = Ttxt->txlm; Tx < Txrm && tcharIsBlank(Lebuf[Tx]); Tx++);
  if (Tx == Ttxt->txrm) Tx = Ttxt->txlm;                      Ty++;   
} 
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static small mcdpat (tchar *buf, small i)           /* micros.dir paste line */
{
  if ((Ttxt->txstat & TS_MCD) && i < MCD_LEFT) {
    tchar *p;
    for (p = buf+i; i < MCD_LEFT; i++) *p++ = (tchar)' ';
                                  i++; *p++ = LDS_MCD;
  } 
  return i; // returns length of filled up lfbuf
}
void teIL() { TxTIL(Ttxt, lfbuf, mcdpat(lfbuf,0)); }
void teDL() { if (BlockMark) teclrblock();
              else           TxDL  (Ttxt); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teclrbeg(void) { TxDEL_beg(Ttxt); qsety(0); wndop(TW_ALL, Ttxt); }
void teclrend(void) { TxDEL_end(Ttxt);           wndop(TW_ALL, Ttxt); }
/*---------------------------------------------------------------------------*/
static void teslice (BOOL vert)
{
  int len; Lleng = TxTRead(Ttxt, Lebuf);
  if (Tx < Lleng) {
    if (vert) { blktspac(lfbuf, Tx);      len = Lleng;
                blktmov(Lebuf+Tx, lfbuf+Tx, Lleng-Tx); }
    else {
      len = my_min(Lleng, Ttxt->txrm); blktmov(Lebuf+Tx, lfbuf, len); 
      len = mcdpat(lfbuf, len);
    }
    TxTRep(Ttxt, Lebuf, Tx); TxDown(Ttxt);
    TxTIL(Ttxt, lfbuf, len);
  }
  else { TxDown(Ttxt);  TxTIL(Ttxt, lfbuf, mcdpat(lfbuf, 0)); }
}
void teblin() { teslice(FALSE); }
void teslin() { teslice(TRUE);  }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tenblin()                               /* merge |like█  --> |like█this */
{                                            /*       |this       |          */
  int len;  Lleng = TxFRead(Ttxt, Lebuf);
  TxDown(Ttxt); if (qTxDown(Ttxt)) exc(E_LPASTE);
  len = TxTRead(Ttxt, lfbuf);
  len = lstrlen(my_min(len, Ttxt->txrm), lfbuf);
  if (Tx + len > Ttxt->txrm) exc(E_LPASTE);
  TxDL(Ttxt);
  TxUp(Ttxt); blktmov(lfbuf, Lebuf+Tx, len);
  TxFRep(Ttxt, Lebuf);
}
void tenslin()  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{                                        /* merge |like█      --> |like█that */
  if (Tx == 0) teDL();                   /*       |     that      |          */
  else {
    Lleng = TxFRead(Ttxt, Lebuf);
    TxDL(Ttxt);
    if (qTxDown(Ttxt)) {  TxTIL(Ttxt, Lebuf, Lleng); exc(E_LPASTE); }
    TxFRead(Ttxt, lfbuf); blktmov(Lebuf, lfbuf, Tx);
    TxFRep (Ttxt, lfbuf);
} }
/*---------------------------------------------------------------------------*/
void temovup()  /* moves either 1-line block or right-of-cursor part of line */
{ 
  int x0,x1; if (Block1size(&x0, &x1)) BlockTy--;  /* exc(E_BLOCKOUT) if not */
             else { x0 = Tx;                       /*           1-line block */
                    x1 = Ttxt->txrm; } 
  Ty--;
  TxFRead(Ttxt, Lebuf); blktmov (Lebuf+x0, lfbuf, x1-x0);
                        blktspac(Lebuf+x0,        x1-x0); TxFRep(Ttxt, Lebuf);
  TxUp(Ttxt);
  TxFRead(Ttxt, Lebuf); blktmov (lfbuf, Lebuf+x0, x1-x0); TxFRep(Ttxt, Lebuf);
}                 
void temovdown()  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{ 
  int x0,x1; if (Block1size(&x0, &x1)) BlockTy++;
             else { x0 = Tx;
                    x1 = Ttxt->txrm; } 
  Ty++;
  Lleng = TxFRead(Ttxt, Lebuf); blktmov (Lebuf+x0, lfbuf+x0, x1-x0);
                                blktspac(Lebuf+x0,           x1-x0);
  TxFRep(Ttxt, Lebuf);
  TxDown(Ttxt);
  if (qTxDown(Ttxt)) { blktspac(lfbuf, x0);
                       mcdpat  (lfbuf, x1); TxTIL(Ttxt, lfbuf, Lleng); }
  else {
    TxFRead(Ttxt, Lebuf); blktmov(lfbuf+x0, Lebuf+x0, x1-x0);
    TxFRep (Ttxt, Lebuf);
} }
/*---------------------------------------------------------------------------*/
void teformat()
{
  int txw = my_min(Twnd->wsw, Ttxt->txrm)-1; tchar *Lepos = Lebuf;
  int len;
  if (Ttxt->txstat & TS_MCD) exc(E_FORMAT);
  Lleng = TxFRead(Ttxt, Lebuf); TxDL(Ttxt);
//
// Folding Very Long Lines (VLL): if the line has continuation mark at the end,
// merge it with the next and repeat merging for the all following line having
// that mark (AT_SUPER'»'), removing the mark between lines:
//
  if (Lleng > 0 && Lebuf[Lleng-1] == TeSCH_CONTINUE) {
    while (Lebuf[Lleng-1] == TeSCH_CONTINUE) {
      if (qTxDown(Ttxt)) Lleng--;
      else {
        len = TxTRead(Ttxt, lfbuf); TxDL(Ttxt);
        blktmov(lfbuf, Lebuf + Lleng - 1, len); Lleng += len - 1;
  } } }
//
// Just the opposite: if the line does not fit into suggested width (minimum of
// screen width and text right margin), unfold it to several lines, splitting
// by word boundary (but only if that does not make a line too short):
//
  else if (Lleng > txw) {
    while (Lleng > txw) {
      small wbeg;
      Lx = txw-1; leNword(&wbeg, NIL, NIL);
      Lx = Tx;
      len =  (wbeg > txw - txw/10) ? wbeg : txw;
      blktmov(Lepos, lfbuf, len);  Lepos += len; Lleng -= len;
      lfbuf[len++] = TeSCH_CONTINUE;
      TxTIL(Ttxt, lfbuf, len); 
      TxDown(Ttxt);      Ty++;
  } }
//
// Otherwise, try to merge the line with the next one (only if the result fits
// into suggested width; when empty line added it always does). In this case,
// DO NOT repeat the operation - let user do if s/he wants.
//
  else if (!qTxDown(Ttxt)) {
    len = TxFRead(Ttxt, lfbuf);
    if (len == 0) TxDL(Ttxt);
    else {
      small tbeg = 0;
      while(tbeg < len && tcharIsBlank(lfbuf[tbeg])) tbeg++;
      if (Lleng+1+len-tbeg > txw) Ty++;
      else {
        TxDL(Ttxt);
        if (tbeg < len) { Lebuf[Lleng++] = (tchar)' ';
          blktmov (lfbuf+tbeg, Lebuf+Lleng, len-tbeg); Lleng += len-tbeg; }
  } } }
  TxTIL(Ttxt, Lepos, Lleng);
} 
/*---------------------------------------------------------------------------*/
static tchar spatbuf[MAXLPAC];                    /* search pattern buffer   */
static int spatlen = 0;                           /* search pattern length   */
static int tesFlag = 0;
static tchar *tesHistory[LE_HISTORY_SIZE] = { 0 };
#define LSPROMPT 12
#define  SPROMPT "‹" AF_SUPER "st·ic" AF_NONE "›Find:"
#define SfPROMPT                      AF_LIGHT "Find:"
#define SgPROMPT                      AF_LIGHT "Grep:"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
tchar    *spa = NIL, *rpa = NIL,      /* current search and replace patterns */
                     *soptfiles;      /* search options (for grep only)      */
int spl = 0, rpl = 0, soptfilen = 0;

void TeInit (void) { spatlen = aftotc(SPROMPT, -1, spatbuf); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tesentr (void)                       /* te: Search / Grep pattern ENTeR */
{
  aftotc(SfPROMPT,7, spatbuf+7);
  LenterARG(spatbuf, &spatlen, LSPROMPT, tesHistory,
                     &tesFlag, TE_SDOWN, TE_SUP, 
    LeARG_STANDARD + LeARG_REGEXP + LeARG_WILDCARD + LeARG_IGNORECASE);
}
void tegentr (void)
{
  aftotc(SgPROMPT,7, spatbuf+7);
  LenterARG(spatbuf, &spatlen, LSPROMPT, tesHistory,
                     &tesFlag, TM_GREP,  TM_GREP2,
    LeARG_STANDARD + LeARG_REGEXP + LeARG_IGNORECASE); /* no LeARG_WILDCARD */
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static tchar *tesFindDelim (tchar *ptn, 
                            tchar *ptnend, tchar pstart, int *found_len)
{ tchar *p;
  if (pstart) {
    while (ptn < ptnend && *ptn != pstart) ptn++;
    if (ptn < ptnend) ptn++;
    else {
      *found_len = 0; return NIL; /* start of subpattern not found */
  } }
  *found_len = ptnend - ptn;
  for (p = ptn; 
       p < ptnend; p++) if (*p & AT_SUPER) { *found_len = p - ptn; break; }

  return ptn;
}
void tesParse() /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
  tchar *spatend = spatbuf+spatlen;
  spa       = tesFindDelim(spatbuf+LSPROMPT, spatend, 0, &spl);
  rpa       = tesFindDelim(spa, spatend, LeSCH_REPL_BEG, &rpl);
  soptfiles = tesFindDelim(spa, spatend, LeSCH_REPL_END, &soptfilen);
}
void tesPrepare()
{
  if (tesFlag) { tesParse(); vipPrepareSearch(); tesFlag = 0; }
}
/*---------------------------------------------------------------------------*/
void tesdown (void)
{
  int x = Tx; tesPrepare();
  for(;;) {
    Lleng = TxTRead(Ttxt, Lebuf); x = vipFind(Lebuf, Lleng, x+1, FALSE);
    if (x < 0) {  
      if (qkbhin())     exc(E_KBREAK); TxDown(Ttxt);
      if (qTxDown(Ttxt)) exc(E_SFAIL); // x == -1 here, exactly start of line
    } 
    else { Ty = Ttxt->txy;
           Tx = x; return; }
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tesup(void) 
{
  int x = Tx; tesPrepare();
  for(;;) {
    if (x < 1) { 
      if (qTxUp(Ttxt)) exc(E_SFAIL); TxUp(Ttxt);
      if (qkbhin())   exc(E_KBREAK);
    }
    Lleng = TxTRead(Ttxt, Lebuf); 
    x = vipFind(Lebuf, Lleng, (x < 0) ? Lleng : x-1, TRUE);
    if (x != -1) {
      Ty = Ttxt->txy;
      Tx = x; return;
} } }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int tereplace()
{
  int len, x0, x1; tesPrepare();
  if (rpa == NIL) exc(E_NORPAT);
  Lleng = TxFRead (Ttxt, Lebuf);
  len = vipFindReplace(Lebuf, Lleng, Tx, lfbuf, &x0, &x1);
  if (len < 0) return -1;
  if (len+Lleng+x0-x1 > MAXLPAC) exc(E_ROUT);
  if (len < x1-x0) {
    int lendiff = x1-x0-len;
    blktmov(Lebuf+x1, Lebuf+x0+len, MAXLPAC-x1);
    blktspac(Lebuf+MAXLPAC-lendiff, lendiff);
  }
  else if (len > x1-x0) 
    blktmov(Lebuf+x1, Lebuf+x0+len, MAXLPAC-x0-len);
//
  blktmov(lfbuf, Lebuf+x0, len); 
  TxFRep(Ttxt, Lebuf); return 0;
}
void terdown() { if (                 tereplace() < 0) tesdown(); }
void terup()   { if (qTxDown(Ttxt) || tereplace() < 0) tesup();   }
/*---------------------------------------------------------------------------*/
void teswidth (void)                              /* <- currently not mapped */
{
  small x = KbCount;
  if (KbRadix == 0 || x < 0 || x > MAXLPAC) exc(E_BADPRM);
  Ttxt->txrm = x;
  if (Tx > x) Tx = x;
}
void tesro() { if (Ttxt->txredit == TXED_YES) Ttxt->txredit = TXED_NO; }
void tesrw() { if (Ttxt->txredit == TXED_NO) Ttxt->txredit = TXED_YES; }
/*---------------------------------------------------------------------------*/
comdesc tecmds[] =
{
  { TE_UP,     teup,     CA_NBEG        }, /* курсор вверх                   */
  { TE_DOWN,   tedown,          CA_NEND }, /* курсор вниз                    */
  { TE_TBEG,   tetbeg,   CA_RPT         }, /* в начало текста                */
  { TE_TEND,   tetend,   CA_RPT         }, /* в конец текста                 */
  { TE_SMARK,  tesmark,  CA_RPT         }, /* установить маркер              */
  { TE_CMARK,  tecmark,  0              }, /* обменять курсор и маркер       */
  { TE_CENTR,  tecentr,  CA_RPT         }, /* текущую строку в середину окна */
  { TE_SMARK1, tesmarkN, CA_RPT },
  { TE_CMARK1, tecmarkN, CA_RPT },
  { TE_SMARK2, tesmarkN, CA_RPT }, { TE_SMARK4, tesmarkN, CA_RPT },
  { TE_CMARK2, tecmarkN, CA_RPT }, { TE_CMARK4, tecmarkN, CA_RPT },
  { TE_SMARK3, tesmarkN, CA_RPT }, { TE_SMARK5, tesmarkN, CA_RPT },
  { TE_CMARK3, tecmarkN, CA_RPT }, { TE_CMARK5, tecmarkN, CA_RPT },
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { TE_CR,     teCR,                       CA_NEND        }, /*       Enter  */
  { TE_RCR,    teRCR,                      CA_NEND        }, /* Shift+Enter  */
  { TE_IL,     teIL,     CA_CHANGE                        }, /* вставить стр */
  { TE_DL,     teDL,     CA_CHANGE|        CA_NEND        }, /* удалить стр. */
  { TE_CLRBEG, teclrbeg, CA_CHANGE|CA_NBEG|CA_NEND|CA_RPT }, /* - начало     */
  { TE_CLREND, teclrend, CA_CHANGE|CA_NBEG|CA_NEND|CA_RPT }, /* - конец EOF  */
  { TE_BLIN,   teblin,   CA_CHANGE|        CA_NEND|CA_RPT }, /* разрезать    */
  { TE_SLIN,   teslin,   CA_CHANGE|        CA_NEND|CA_RPT }, /* -вертикально */
  { TE_NBLIN,  tenblin,  CA_CHANGE|        CA_NEND|CA_RPT }, /* склеить      */
  { TE_NSLIN,  tenslin,  CA_CHANGE|        CA_NEND        }, /* -вертикально */
  { TE_MOVUP,  temovup,  CA_CHANGE|CA_NBEG|CA_NEND        }, /* сдвинуть     */
  { TE_MOVDOWN,temovdown,CA_CHANGE|        CA_NEND        }, /* - вниз       */
  { TE_FORMAT, teformat, CA_CHANGE|        CA_NEND        },
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { TE_SENTR, tesentr,  0                 }, /* ввод строки поиска/замены    */
  { TE_GENTR, tegentr,  0                 }, /* ввод строки поиска для grep  */
  { TE_SDOWN, tesdown,           CA_NEND  }, /* поиск вниз                   */
  { TE_SUP,   tesup,    0                 }, /* поиск вверх                  */
  { TE_RUP,   terup,    CA_CHANGE         }, /* замена и поиск вверх         */
  { TE_RDOWN, terdown,  CA_CHANGE|CA_NEND }, /* замена и поиск вниз          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { TE_SWIDTH, teswidth, CA_RPT }, /* установить ширину текста               */
  { TE_SRO,    tesro,    CA_RPT }, /* режим просмотра текста                 */
  { TE_SRW,    tesrw,    CA_RPT }, /* режим редактирования текста            */
/*
 * Implemented in clip.cpp (declaration in "clip.h"):
 */
  { LE_CHAR,   teicharblk, CA_BLOCK|CA_CHANGE|CA_NEND }, /* tall cursor mode */
  { LE_IC,     teicblock,  CA_BLOCK|CA_CHANGE|CA_NEND },
  { LE_DC,     tedcblock,  CA_BLOCK|CA_CHANGE|CA_NEND },
  { LE_CCHAR,  tecsblock,  CA_BLOCK          |CA_RPT  }, /* запомнить блок   */
  { LE_CDCHAR, tesdblock,  CA_BLOCK|CA_CHANGE|CA_RPT  }, /* - и очистить     */
  { TE_CLIN,   teclin,     CA_LCUT |          CA_NEND }, /* запомнить строку */
  { TE_CDLIN,  tecdlin,    CA_LCUT |CA_CHANGE|CA_NEND }, /* - с удалением    */
  { LE_PASTE,  cpaste,              CA_CHANGE         }, /* вспомнить        */
/*
 * Implemented in ud.c (declaration in "ud.h"), the same functions as in le.c
 */
  { TE_UNDO,    leundo,    0 }, /* откатка (may set TS_CHANGE) */
  { TE_UNUNDO,  leunundo,  0 }, /* откатка откатки             */
  { TE_SUNDO,   lesundo,   0 }, /* "медленная" откатка         */
  { TE_SUNUNDO, lesunundo, 0 }, /* "медленная" откатка откатки */
  { 0,0,0 }
};
comdesc *Tdecode (int kcode)  /*- - - - - - - - - - - - - - - - - - - - - - -*/
{
  if (Mk_IsCHAR(kcode)) kcode = LE_CHAR;
  comdesc *cp;
  for (cp = tecmds; cp->cfunc; cp++)
    if ((int)cp->mi_ev == kcode)
      return (BlockMark || !(cp->attr & CA_BLOCK)) ? cp : NIL;
/*
 * ^ for TE, attribute CA_BLOCK marks commands that are hadnled here only when
 * block is present (otherwise LE takes care of them), the rest's unconditional
 */
  return NIL;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int TeCommand (comdesc *cp)
{
  jmp_buf *nextexc, teenv; /* TODO: remove nextexc (no recursion anymore) */
  large rpt;
  small x;

  if (cp->attr & CA_LCUT && Ttxt == LCtxt) return E_LCUT;
  if (cp->attr & CA_CHANGE) {
    if (Ttxt->txredit != TXED_YES) return E_CHANGE;
    UdMark = TRUE;
  }
  for (rpt = (cp->attr & CA_RPT) ? 1 : KbCount; rpt; rpt--) {
    TxSetY(Ttxt, Ty);
    if (qkbhin())                              return E_KBREAK;
    if ((cp->attr & CA_NBEG) && qTxUp  (Ttxt)) return E_MOVUP;
    if ((cp->attr & CA_NEND) && qTxDown(Ttxt)) return E_MOVDOWN;
    if ((cp->attr & CA_EXT)  && qTxDown(Ttxt)) {
      teIL(); 
      Ttxt->txstat |= TS_CHANGED; TxSetY(Ttxt, Ty);
    }
    nextexc = excptr; 
              excptr = & teenv; if ((x = setjmp(teenv)) == 0) (*cp->cfunc)();
              excptr = nextexc; if  (x)                             return x;

    if (cp->attr & CA_CHANGE) Ttxt->txstat |= TS_CHANGED;
  }
  return E_OK;
}
/*---------------------------------------------------------------------------*/

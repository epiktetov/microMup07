/*------------------------------------------------------+----------------------
// МикроМир07    te = Text editor - Редактор текста     | (c) Epi MG, 2006-2022
//------------------------------------------------------+--------------------*/
#include "mic.h"          /* Old te.c (c) Attic 1989-96, (c) EpiMG 1998,2001 */
#include "ccd.h"
#include "twm.h"
#include "vip.h"
#include "clip.h"
#include "synt.h"
#include "le.h"
#include "te.h"
#include "tx.h"
#include "ud.h"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd  *Twnd = NIL;                     /* Окно, в котором редактируется текст */
txt  *Ttxt;                           /* Редактируемый текст                 */
int   Tx;                             /* X курсора в тексте                  */
long  Ty;                             /* Y курсора в тексте                  */
/*---------------------------------------------------------------------------*/
bool tesetxy (int x, long y) { bool ok = TxSetY(Ttxt, y);
                               Tx = x;
                               Ty = Ttxt->txy; return ok; }
void qsety (long y)
{
  bool ok = TxSetY(Ttxt, y);   Ty = Ttxt->txy;   if (!ok) exc(E_SETY);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tesmark()           /* "temp" marker: "временный" маркера, ставится сам */
{                        /*  в tleunload(le.c) при de jure изменениях текста */
  Ttxt->txmarkx[0] = Tx;
  Ttxt->txmarky[0] = Ty; Ttxt->txmarkt[0] = 0;
}
void tecmark()  /* в другой угол блока, или вернуться туда, где меняли текст */
{
  if (BlockMark) { long tmp = BlockTx; BlockTx = Tx; Tx = tmp;
                        tmp = BlockTy; BlockTy = Ty; Ty = tmp; }
  else {
    short Mx = Ttxt->txmarkx[0];
    long  My = Ttxt->txmarky[0]; if (My < 0) exc(E_MOVUP); tesetxy(Mx, My);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
tchar txMarkT[TXT_MARKS] = { 0, /* mark type, attributes + character to show */
    AT_MARKFLG|AT_BG_RED| 0xB9, /* red   ¹ |                                 */
    AT_MARKFLG|AT_BG_GRN| 0xB2, /* green ² | used in TxInfo(), file tx.c     */
    AT_MARKFLG|AT_BG_BLU| 0xB3, /* blue  ³ |                                 */
    AT_MARKFLG|AT_BG_RED|  ' ', /* red (no char) mark for '!'  TXT_MARK_ERR  */
    AT_MARKFLG|AT_BG_GRN|  ' ', /* green mark for '@' info     TXT_MARK_INFO */
    AT_MARKFLG|AT_BG_BLU|  ' ', /*  blue mark for '#' whatever TXT_MARK_SMTH */
    AT_MARKFLG|            ' ', /* brown mark for warnings     TXT_MARK_WARN */
};
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tesmarkN()                       /* Set/clear mark N | NOTE: codes must */
{                                     /* and go to mark N | be TE_S/CMARK0+N */
  int N = KbCode - TE_SMARK0;
  if (Ttxt->txmarky[N] == Ty) { Ttxt->txmarkx[N] =  0;
                                Ttxt->txmarky[N] = -1; wndop(TW_RP, Ttxt); }
  else {
    TxUnmarkY(Ttxt,Ty); // <- remove any other markers from the current line
    Ttxt->txmarkx[N] = Tx;
    Ttxt->txmarky[N] = Ty; Ttxt->txmarkt[N] = txMarkT[N]; wndop(TW_ALL, Ttxt);
} }
void tecmarkN() // TE_CMARKN/_CMARKP => go to next/prev mark down/up (fail if
{               //                   or go to mark N                not found)
  long  My = -1;         int i, N;
       if (KbCode  < TE_CMARKN) N = KbCode - TE_CMARK0;
  else if (KbCode == TE_CMARKN) {
    for (i = 1; i < TXT_MARKS; i++)
      if (Ttxt->txmarky[i] > Ty && (My < 0 || My > Ttxt->txmarky[i]))
                                            { My = Ttxt->txmarky[i]; N = i; }
    if (My < 0) exc(E_SFAIL);
  }
  else if (KbCode == TE_CMARKP) {
    for (i = 1; i < TXT_MARKS; i++)
      if (Ttxt->txmarky[i] >= 0 &&
          Ttxt->txmarky[i] < Ty && My < Ttxt->txmarky[i])
                                 { My = Ttxt->txmarky[i]; N = i; }
    if (My < 0) exc(E_SFAIL);
  }
  else return;
      My  =                        Ttxt->txmarky[N];
  if (My >= 0 && My != Ty) tesetxy(Ttxt->txmarkx[N], My);
  else { KbCode = TE_SMARK0+N; //
                   tesmarkN(); // for convenience, if requested mark is not set
} }                            // (or is set to the current line) => tesmark
/*---------------------------------------------------------------------------*/
void tedown() { Ty++; }  void tetbeg() { tesetxy(Tx,          0); }
void teup  () { Ty--; }  void tetend() { tesetxy(Tx, 2147483647); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tecentr()             /* Esc NN Ctrl+E => В строку с номером KbCount-1  */
{                          /*          else => Текущую строку в центр экрана */
  short whh;
  long y;
  if (KbRadix != 0) qsety(KbCount ? KbCount-1 : 0); 
  else {
    wadjust(Twnd, Tx, Ty); whh = (Twnd->wsh-1) >> 1; y = my_max(Ty-whh, 0);
    wadjust(Twnd, Tx,  y); while (!qTxBottom(Ttxt) && whh--)  TxDown(Ttxt);
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
static short mcdpat (tchar *buf, short i)           /* micros.dir paste line */
{
  if ((Ttxt->txstat & TS_MCD) && i < MCD_LEFT) {
    if (qTxBottom(Ttxt) && i == 0)
                         return aftotc(AF_PROMPT "incdirs»" AF_NONE, -1, buf);
    else {
      blktspac(buf+i, MCD_LEFT-i); buf[MCD_LEFT] = LDS_MCD; return MCD_LEFT+1;
  } }
  return i; // returns length of filled up lfbuf
}
void teIL() { TxTIL(Ttxt, lfbuf, mcdpat(lfbuf,0)); }
void teDL() { if (BlockMark) teclrblock();
              else           TxDL  (Ttxt); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teclrbeg(void) { TxDEL_beg(Ttxt); qsety(0); wndop(TW_ALL, Ttxt); }
void teclrend(void) { TxDEL_end(Ttxt);           wndop(TW_ALL, Ttxt); }
/*---------------------------------------------------------------------------*/
static void teslice (bool vert)
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
  int len;   Lleng = TxFRead(Ttxt, Lebuf);
  TxDown(Ttxt); if (qTxBottom(Ttxt)) exc(E_LPASTE);
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
    if (qTxBottom(Ttxt)) { TxTIL(Ttxt, Lebuf, Lleng); exc(E_LPASTE); }
    TxFRead(Ttxt, lfbuf);  blktmov(Lebuf, lfbuf, Tx);
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
  if (qTxBottom(Ttxt)) { blktspac(lfbuf, x0);
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
      if (qTxBottom(Ttxt)) Lleng--;
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
      int wbeg;
      Lx = txw-1; if (tcharIsBlank(Lebuf[Lx])) wbeg = Lx;
                  else          leNword(&wbeg, NIL, NIL);
      Lx = Tx;
      len =  (wbeg > txw - txw/10) ? wbeg : txw;
      blktmov(Lepos, lfbuf,  len);
      lfbuf[len] = TeSCH_CONTINUE;
      TxTIL (Ttxt, lfbuf,  len+1);
      TxDown(Ttxt); Ty++;
      blktmov (Lebuf+len, Lebuf, ( Lleng -= len ));
  } }
//
// Otherwise, try to merge the line with the next one (only if the result fits
// into suggested width; when empty line added it always does). In this case,
// DO NOT repeat the operation - let the user do that, if s/he wants.
//
  else if (!qTxBottom(Ttxt)) {
    len = TxFRead(Ttxt, lfbuf);
    if (len == 0) TxDL(Ttxt);
    else {
      int   tbeg = 0;
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
#define  SPROMPT "‹" AF_SUPER "st·ic" AF_NONE  "›Find:"
#define SfPROMPT                      AF_PROMPT "Find:"
#define SgPROMPT                      AF_PROMPT "Grep:"
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
  if (pstart) { while (ptn < ptnend && *ptn != pstart) ptn++;
                if    (ptn < ptnend)                   ptn++;
                else { *found_len = 0; return NIL; }
  }
  for (p = ptn;  p < ptnend; p++)
    if (*p == LeSCH_REPL_BEG ||
        *p == LeSCH_REPL_END) { *found_len = p      - ptn; return ptn; }
                                *found_len = ptnend - ptn; return ptn;
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
  int x = Tx, len; tesPrepare();
  for(;;) {
    char *str = TxGetLn(Ttxt, &len); x = vipFind(str,len, x+1, false);
    if (x < 0) {  
      if (qkbhin())       exc(E_KBREAK); TxDown(Ttxt);
      if (qTxBottom(Ttxt)) exc(E_SFAIL);       x = -1;
    } 
    else { Ty = Ttxt->txy; if (x > Ttxt->txrm) exc(E_EDTEND);
           Tx = x;                                    return; }
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tesup(void) 
{
  int x = Tx, len; tesPrepare();
  for(;;) {
    if (x < 1) { 
      if (qTxTop(Ttxt)) exc(E_SFAIL); TxUp(Ttxt);
      if (qkbhin())    exc(E_KBREAK);     x = -1;
    }
    char *str = TxGetLn(Ttxt, &len); x = vipFind(str,len, (x<0)?len:x-1, true);
    if (x >= 0) {
      Ty = Ttxt->txy; if (x > Ttxt->txrm) exc(E_EDTEND);
      Tx = x;                                    return;
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
void terdown() { if (                   tereplace() < 0) tesdown(); }
void terup()   { if (qTxBottom(Ttxt) || tereplace() < 0) tesup();   }
/*---------------------------------------------------------------------------*/
void teswidth (void)
{
  if (KbRadix) {
         if (KbCount <=      0) Ttxt->txrm = MAXTXRM;
    else if (KbCount > MAXLPAC) Ttxt->txrm = MAXLPAC;
    else                        Ttxt->txrm = KbCount;
  } else                        Ttxt->txrm = Tx;
}
void tesro() { if (Ttxt->txredit == TXED_YES) Ttxt->txredit = TXED_NO; }
void tesrw() { if (Ttxt->txredit == TXED_NO) Ttxt->txredit = TXED_YES; }
/*---------------------------------------------------------------------------*/
comdesc tecmds[] =
{
  { TE_UP,     teup,     CA_NBEG          }, /* курсор вверх    */
  { TE_DOWN,   tedown,            CA_NEND }, /* курсор вниз     */
  { TE_TBEG,   tetbeg,   CA_RPT           }, /* в начало текста */
  { TE_TEND,   tetend,   CA_RPT           }, /* в конец текста  */
  { TE_CENTR,  tecentr,  CA_RPT },
  { TE_CMARK0, tecmark,  CA_RPT }, { TE_SMARK0, tesmarkN, CA_RPT },
  { TE_CMARK1, tecmarkN, CA_RPT }, { TE_SMARK1, tesmarkN, CA_RPT },
  { TE_CMARK2, tecmarkN, CA_RPT }, { TE_SMARK2, tesmarkN, CA_RPT },
  { TE_CMARK3, tecmarkN, CA_RPT }, { TE_SMARK3, tesmarkN, CA_RPT },
  { TE_CMARKN, tecmarkN, CA_RPT }, { TE_CMARKP, tecmarkN, CA_RPT },
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { TE_CR,     teCR,                    CA_NEND        }, /*       Enter     */
  { TE_RCR,    teRCR,                   CA_NEND        }, /* Shift+Enter     */
  { TE_IL,     teIL,     CA_MOD                        }, /* вставить строку */
  { TE_DL,     teDL,     CA_MOD|        CA_NEND        }, /* удалить строку  */
  { TE_CLRBEG, teclrbeg, CA_MOD|CA_NBEG|CA_NEND|CA_RPT }, /* - начало        */
  { TE_CLREND, teclrend, CA_MOD|CA_NBEG|CA_NEND|CA_RPT }, /* - конец EOF     */
  { TE_BLIN,   teblin,   CA_MOD|        CA_NEND|CA_RPT }, /* разрезать       */
  { TE_SLIN,   teslin,   CA_MOD|        CA_NEND|CA_RPT }, /* - вертикально   */
  { TE_NBLIN,  tenblin,  CA_MOD|        CA_NEND|CA_RPT }, /* склеить         */
  { TE_NSLIN,  tenslin,  CA_MOD|        CA_NEND        }, /* - вертикально   */
  { TE_MOVUP,  temovup,  CA_MOD|CA_NBEG|CA_NEND        }, /* сдвинуть        */
  { TE_MOVDOWN,temovdown,CA_MOD|        CA_NEND        }, /* - вниз          */
  { TE_FORMAT, teformat, CA_MOD|        CA_NEND        },
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { TE_SENTR,  tesentr,  0              }, /* ввод строки поиска/замены      */
  { TE_GENTR,  tegentr,  0              }, /* ввод строки поиска для grep    */
  { TE_SDOWN,  tesdown,         CA_NEND }, /* поиск вниз                     */
  { TE_SUP,    tesup,    0              }, /* поиск вверх                    */
  { TE_RUP,    terup,    CA_MOD         }, /* замена и поиск вверх           */
  { TE_RDOWN,  terdown,  CA_MOD|CA_NEND }, /* замена и поиск вниз            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { TE_SWIDTH, teswidth, CA_RPT }, /* установить ширину текста               */
  { TE_SRO,    tesro,    CA_RPT }, /* режим просмотра текста                 */
  { TE_SRW,    tesrw,    CA_RPT }, /* режим редактирования текста            */
/*
 * Implemented in clip.cpp (declaration in "clip.h"):
 */
  { LE_CCHAR,  tecsblock,  CA_BLOCK       |CA_RPT  }, /* запомнить блок   */
  { LE_CDCHAR, tesdblock,  CA_BLOCK|CA_MOD|CA_RPT  }, /* - и очистить     */
  { LE_DLWORD, tedelblock, CA_BLOCK|CA_MOD|CA_RPT  }, /* схлопнуть блок   */
  { TE_CLIN,   teclin,     CA_LCUT |       CA_NEND }, /* запомнить строку */
  { TE_CDLIN,  tecdlin,    CA_LCUT |CA_MOD|CA_NEND }, /* - с удалением    */
  { LE_PASTE,  cpaste,              CA_MOD         }, /* вспомнить        */
  { TE_CLPOS,  teclpos,             0              },
  { TE_TOCLIP, clipToCB,            0              },
  { TE_FROMCB, clipFromCB,          CA_MOD         },
  { TE_LD1UP,  teldpgUp,            CA_MOD         }, /* ┐  line-drawing  */
  { TE_LD1LT,  teldpgLeft,          CA_MOD         }, /* │ pseudo-graphic */
  { TE_LD1DN,  teldpgDown,          CA_MOD         }, /* │  (single line) */
  { TE_LD1RT,  teldpgRight,         CA_MOD         }, /* ┘                */
  { TE_LD2UP,  teldpgDblUp,         CA_MOD         }, /* ╗              ↑ */
  { TE_LD2LT,  teldpgDblLeft,       CA_MOD         }, /* ║ line-drawing ← */
  { TE_LD2DN,  teldpgDlbDown,       CA_MOD         }, /* ║  double line ↓ */
  { TE_LD2RT,  teldpgDlbRight,      CA_MOD         }, /* ╝              → */
/*
 * Implemented in ud.c (declaration in "ud.h"), the same functions as in le.c
 */
  { TE_UNDO,    leundo,    0 }, /* откатка (may set TS_CHANGE) */
  { TE_UNUNDO,  leunundo,  0 }, /* откатка откатки             */
  { TE_SUNDO,   lesundo,   0 }, /* "медленная" откатка         */
  { TE_SUNUNDO, lesunundo, 0 }, /* "медленная" откатка откатки */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { TE_SHBRAK,  SyntBrakToggle, CA_RPT }, /* toggle ShowBrak mode - synt.cpp */
  { TE_BRAKOFF, SyntLangOff,    CA_RPT }, /* выключить раскраску             */
  { TE_BRAKON,  SyntLangOn,     CA_RPT }, /* включить (снова) раскраску      */
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
  int x, rpt;
  if (cp->attr & CA_LCUT && Ttxt == LCtxt) return E_LCUT;
  if (cp->attr & CA_MOD) {
    if (Ttxt->txredit == TXED_YES) UndoMark = TRUE;
    else                           return E_CHANGE;
  }
  for (rpt = (cp->attr & CA_RPT) ? 1 : KbCount; rpt; rpt--) {
    TxSetY(Ttxt, Ty);
    if ((cp->attr & CA_NBEG) && qTxTop   (Ttxt)) return E_MOVUP;
    if ((cp->attr & CA_NEND) && qTxBottom(Ttxt)) return E_MOVDOWN;
//
//  if ((cp->attr & CA_EXT)  && qTxBottom(Ttxt)) { -- not used in te.c anymore
//    teIL();
//    Ttxt->txstat |= TS_CHANGED; TxSetY(Ttxt, Ty);
//  }
    nextexc = excptr; 
              excptr = & teenv; if ((x = setjmp(teenv)) == 0) (*cp->cfunc)();
              excptr = nextexc; if  (x)                             return x;

    if (cp->attr & CA_MOD) Ttxt->txstat |= TS_CHANGED;
    if (qkbhin())                     return E_KBREAK;
  }
  return E_OK;
}
/*---------------------------------------------------------------------------*/

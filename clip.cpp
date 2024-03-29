//------------------------------------------------------+----------------------
// МикроМир07  Clipboard (using wxClipboard) & CS/LS op | (c) Epi MG, 2007-2022
//------------------------------------------------------+----------------------
#include <QApplication>   /* Old le.c (c) Attic 1989,    (c) EpiMG 1996-2003 */
#include <QClipboard>     /* old te.c (c) Attic 1989-96, (c) EpiMG 1998,2001 */
#include <QStringList>    /* old cp.c (c) Attic 1989     (c) EpiMG 1998-99   */
#include "mim.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
#include "clip.h"
extern "C" {
#include "le.h"
#include "te.h"
#include "tx.h"
#include "ud.h"
}
QClipboard  *theClipboard; dummyClip *theDummyClip;
QClipboard::Mode clipMode;
static void toClipboard();
static int  cb_new_data   = 0;
static bool cb_refocusing = false;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void clipStart()
{
  theClipboard = QApplication::clipboard();
  clipMode = theClipboard->supportsSelection() ? QClipboard::Selection
                                               : QClipboard::Clipboard;
  theDummyClip = new dummyClip;
  QObject::connect(theClipboard, SIGNAL(dataChanged()),
                   theDummyClip,      SLOT(changed1()));
  QObject::connect(theClipboard, SIGNAL(selectionChanged()),
                   theDummyClip,           SLOT(changed2()));
  ldpgInit();
}
// - - - - - - - - - - - - - - - - - - -
void clipChanged (QClipboard::Mode mode)
{
  if (mode == clipMode) cb_new_data++;
}
void dummyClip::changed1() { clipChanged(QClipboard::Clipboard); }
void dummyClip::changed2() { clipChanged(QClipboard::Selection); }
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void clipRefocus () {      cb_refocusing =         true; }
void clipFocusOff() { if (!cb_refocusing) toClipboard(); }
/*---------------------------------------------------------------------------*/
// Внутренний cut/paste буфер: запомненные символы собираятся в ccbuf, строки
// сразу кладутся в LCtxt (в начало добавляется специальный символ по которому
// при paste вставляется новая строка)
//
txt *LCtxt = NULL;           /* Текст-хранилище строк (и всего прочего тоже) */
static tchar ccbuf[MAXLPAC]; /* Буфер запомненных символов (неполная строка) */
static int  cclen  = 0;      /* Мощность буфера запомненных символов/слов    */
static bool cpopen = false;  /* Буфер "открыт" (идет запоминание, добавлять) */
static bool cpnocl = false;  /* Запрещено сохранение строк (запоминаем char) */
static bool cpempt = true;   /* Буфер пустой -- гарантировано ничего там нет */
static char clbuf[MAXLUP];
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int                                    ldpgMode = 0;
void teldpgMod1() { if (ldpgMode == 1) ldpgMode = 0; else ldpgMode = 1; }
void teldpgMod2() { if (ldpgMode == 2) ldpgMode = 1; else ldpgMode = 2; }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void cptinit (bool needContent)
{
  if (LCtxt == NULL) {
    if ((LCtxt = tmDesc(CLIPFILNAM, false)) == NULL) exc(E_CPFER);
    if (needContent) tmLoad(LCtxt);
    else   LCtxt->txstat |= TS_NEW;
           LCtxt->txstat |= (TS_FILE|TS_PERM);
} }
QString clipStatus()  /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
  return QString::fromUtf8(ldpgMode ?    (ldpgMode == 2 ? "═" : "─")
                           : cpopen ? "»" : cb_new_data ? "«" : "·");
}
/*---------------------------------------------------------------------------*/
static void CPempty (void)
{
  cptinit(false); TxEmpt(LCtxt);
  cclen = 0;
  cpopen = cpempt = true; cpnocl = false;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void CSsave1 (int X, int len, bool trim)  /* Save 1 line at given pos */
{                                                /* & len in Lebuf, trimming */
  if (!cpopen) CPempty();                        /* spaces if requested      */
  cb_refocusing =  false;
  int lns = 0;
  while (len-- && cclen < MAXLPAC) {
    tchar tc = Lebuf[X++];
    ccbuf[cclen++] = tc;
    if (!tcharIsBlank(tc)) lns = cclen;
  }                                                cpnocl = true;
  if (trim && lns && lns < cclen-1) cclen = lns+1; cpempt = false;
}
void lecchar()  { CSsave1(Lx++, 1, false);         }  /* le "copy character" */
void lecdchar() { CSsave1(Lx,   1, false); leDC(); }  /* le "copy & delete"  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int CSsaveW()                                  /* Save word at cursor */
{
  int xr; if (! leNword(&Lx, NULL, &xr)) exc(E_NOOP);
          CSsave1(Lx, xr-Lx, true);        return xr;
}
void lecword()  { Lx = CSsaveW();            }    /* le "copy word"          */
void lecdword() {      CSsaveW(); ledword(); }    /* le "copy & delete word" */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void CSclose ()
{
  cptinit(false);
  if (cclen) { TxBottom(LCtxt);
    TxTIL(LCtxt, ccbuf, cclen); cclen = 0; LCtxt->txstat |= TS_CHANGED;
} }
void cpsave() // flush everything we saved so far into disk file (used at exit
{             // and for Calculator feature - feeding .micro7.clip to the 'bc')
  CSclose();  //
  if (LCtxt && (LCtxt->txstat & TS_CHANGED)) tmsave(LCtxt);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Защита от случайного нажатия на F3 вместо F4: если последнее запоминаемое */
/* было символы, отказать (но только 1 раз, выполнить затребованную операцию */
/*                                             если пользователь настаивает) */
static void tecutlin (int len)
{
       if (!cpopen) CPempty();
  else if (!cpnocl) CSclose();
  else    { cpnocl =  false; exc(E_LCUT); }
  clbuf[0] = CpSCH_NEW_LINE;
  TxBottom(LCtxt);
  TxIL(LCtxt, clbuf, len+1); LCtxt->txstat |= TS_CHANGED; cpempt = false;
}
static int pos_to_clbuf1 (txt *t, long ty)
{
  char *p = scpy(t->file->full_name.uStr(), clbuf+1);
       *p++ = ':';                 p = ltod(p, ty+1);
       *p++ = ':';          return p -     (clbuf+1);
}
void teclin()  { tecutlin( TxRead(Ttxt, clbuf+1) ); Ty++;       }
void tecdlin() { tecutlin( TxRead(Ttxt, clbuf+1) ); TxDL(Ttxt); }
void teclpos() { tecutlin(pos_to_clbuf1(Ttxt, Ty));             }
/*---------------------------------------------------------------------------*/
void cpclose() { CSclose(); cpopen = false; }
void cpreopen() 
{ 
  cptinit(false);   TxBottom(LCtxt);
  if (!qTxTop(LCtxt)) { TxUp(LCtxt);
    int len = TxRead(LCtxt, clbuf);
    if (len > 0 && clbuf[0] != CpSCH_NEW_LINE)
      cclen = aftotc(clbuf, len, ccbuf);
  }
  cpopen = true;  
}                                  
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/          
static void toClipboard()
{
  CSclose();  TxTop(LCtxt); bool add_EOL = false; QString cbData; int len;
  while (!qTxBottom(LCtxt)) {
    if (add_EOL) cbData += (QChar)clipEOL;
    char *tp = TxGetLn(LCtxt, &len);
    int dx = (*tp == CpSCH_NEW_LINE) ? 1 : 0;
    cbData += QString::fromUtf8(tp+dx, len-dx);
    if (dx) cbData += (QChar)clipEOL;
    else              add_EOL = true; TxDown(LCtxt);
  }
  theClipboard->setText(cbData, clipMode); 
  cb_new_data = 0;         cpopen = false;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static bool fromClipboard()                          /* Get data from system */
{                                                    /* Clipboard to our own */
  QString cbData = theClipboard->text(clipMode);     /* storage adding ^L in */
  cb_new_data = 0;                                   /* front of full lines  */
  if (cbData.isEmpty()) return false;
  cptinit(false);
  TxEmpt (LCtxt);
  QRegExp eol("([^\r\n]*)(?:\r?\n|\r(?!\n))(.*)");
  while (!cbData.isEmpty()) {           int lflen;
    if (eol.exactMatch(cbData)) {       lfbuf[0] = (tchar)CpSCH_NEW_LINE;
           lflen = qstr2tcs(eol.cap(1), lfbuf+1)+1;  cbData = eol.cap(2); }
    else { lflen = qstr2tcs(cbData, lfbuf);          cbData = "";         }
    TxTIL (LCtxt, lfbuf, lflen);
    TxDown(LCtxt);
  }
  cpempt = false; LCtxt->txstat |= TS_CHANGED; return true;
}
static void clearClipboard() { theClipboard->clear(); cb_new_data = 0; }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void clipToCB()                       /* forced to/from clipboard operations */
{
  if (BlockMark) tecsblock(); // save the block only if one currently selected
               toClipboard(); // but copy c/p buffer into clipboard regardless
}
void clipFromCB() { fromClipboard(); cpaste(); }
/*---------------------------------------------------------------------------*/
static void pasteFromLCtxt (bool in_Ttxt) /* paste into Ttxt or current line */
{
  int len = TxRead(LCtxt, clbuf); TxDown(LCtxt);  if (!len) return;
  char *clpos = clbuf;
  if (clbuf[0] == CpSCH_NEW_LINE) { clpos++; len--;
    if (in_Ttxt) {
      if (Lwnd) ExitLEmode();
      TxSetY(Ttxt, Ty);
      TxIL  (Ttxt, clpos, len); Ttxt->txstat |= TS_CHANGED; return;
  } }
       if (Lx <  Lxle) exc(E_EDTBEG);
  else if (Lx >= Lxre) exc(E_EDTEND);
  else {
    TxSetY(Ttxt, Ty);                 //  Have to take care of inserting empty
    if (in_Ttxt && qTxBottom(Ttxt)) { // line at the end-of-text (usually this
      if (Lwnd)   ExitLEmode();       // is done by LeCommand automatically),
      TxSetY(Ttxt, Ty); teIL();       // make sure to position in Ttxt first!
    }
    if (Lwnd == NULL) EnterLEmode();                  // inserting one-by-one
    cclen = aftotc(clpos, len, ccbuf);                // to enable slow undo
    for (int i = 0; i < cclen; i++) leLLCE(ccbuf[i]); // inside the line and
    cclen = 0;                        Lchange = TRUE; // cleanup in leARGmode
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void pasteFromClbuf()      /* called from cpaste through tallblockop, */
{                                 /*   data already loaded into clbuf[cclen] */
  leic20(ccbuf, cclen); Lx += cclen;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void cpaste()                              /* Paste: called either from te.c */
{                                          /*                   or from le.c */
  if (!cpopen || cpempt) {                 /* so have to work in any LE mode */
    if (cb_new_data) fromClipboard();
  }
  else   clearClipboard(); //<- clear system Clipboard to avoid inconsistency
  if (cpempt) exc(E_NOOP); //   (when 1st paste resets cpopen, and second one
  int oldTx = Lwnd? Lx:Tx; //       gets data from Clipboard instead of LCtxt)
  cptinit(true);
  cpclose(); TxTop(LCtxt); if (qTxBottom(LCtxt)) return;
//
// In entering-argument mode, paste only the content of first line (disabling
// full-line paste) and silently ignore the rest; the same when block selected
// (only when it is 1-character wide, "tall cursor" mode)
//
       if (leARGmode) pasteFromLCtxt(false);
  else if (BlockMark) {
    if (BlockTx != Tx) exc(E_BLOCKOUT);
    else {
      static comdesc cpPaste = { LE_PASTE, pasteFromClbuf, 0 };
      int len = TxRead(LCtxt, clbuf);
      if (len == 0) return;
      if (clbuf[0] == CpSCH_NEW_LINE) cclen = aftotc(clbuf+1, len-1, ccbuf);
      else                            cclen = aftotc(clbuf,   len,   ccbuf);
      tallblockop(&cpPaste);
  } }
  else {
    pasteFromLCtxt(true);     // First stored line may be pasted either as line
    while(!qTxBottom(LCtxt)){ // or as characters, exit LE mode for the rest of
      if (Lwnd) ExitLEmode(); // save buffer (if paste isn't finished) and move
      Tx = oldTx;     Ty++;   // cursor to saved position one line below, rinse
      pasteFromLCtxt(true);   // and repeat pasting.
} } }
/*---------------------------------------------------------------------------*/
static int blkXmin, blkXmax, blkXsize, // NOTE: for 1-line blocks with cursor
           blkYmin, blkYmax, blkYsize; // @ right corner, right-most position
                                       // is NOT included into block (to make
static void make_blkXYsize()           // selection of words more usable)
{
  if (BlockTy < Ty) { blkYmin = BlockTy; blkYmax = Ty; }
  else              { blkYmin = Ty; blkYmax = BlockTy; }
  if (BlockTx < Tx)
       { blkXmin = BlockTx; blkXmax = (blkYmin == blkYmax) ? Tx-1 : Tx; }
  else { blkXmin = Tx;      blkXmax = BlockTx;                          }
  blkXsize = blkXmax-blkXmin+1; //
  blkYsize = blkYmax-blkYmin+1; // both blkXsize and blkYsize always > 0
}
int Block1size (int *x0, int *x1) /*- - - - - - - - - - - - - - - - - - - - -*/
{                                                       // does not return any
  if (BlockMark) {  if (BlockTy != Ty) exc(E_BLOCKOUT); // value when block is
    make_blkXYsize(); *x0 = blkXmin;                    // taller than one line
                      *x1 = blkXmax+1; return blkXsize;
  } else                               return        0;
}
int Block1move (tchar *buf, int maxLen)
{
  int x0,x1,len = Block1size(&x0,&x1); if (len > maxLen) len = maxLen;
  TxSetY (Ttxt, Ty);
  TxFRead(Ttxt, Lebuf); blktmov(Lebuf+x0, buf, len);       return len;
}
bool BlockXYsize (int *dx, int *dy) /*- - - - - - - - - - - - - - - - - - - -*/
{
  if (BlockMark) { make_blkXYsize(); *dx = blkXsize;
                                     *dy = blkYsize; return TRUE; }
  else return FALSE;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tallblockop (comdesc *cp)             /* "tall cursor" block operations */
{
  bool umark = UndoMark; if (Lwnd) ExitLEmode();
  int cTx = Tx;                make_blkXYsize();
  int cTy = Ty;
  for (Ty = blkYmin; Ty <= blkYmax; Ty++) { TxSetY(Ttxt, Ty); Tx = cTx;
    tleload();                               // ^
    if (cp->mi_ev == LE_CHAR) leic2(KbCode); // make sure text positioned to Y,
    else                     (*cp->cfunc)(); // and X is the same for all lines
    UndoMark = umark;
    umark  = false;               // set UndoMark to stored value on first line
    Lchange = true;  tleunload(); // only (so the operation is undone as whole,
  }                               // whether it has repetition or not): we need
  if (cp->mi_ev == LE_CHAR) Tx++; // this trick because tleload resets the mark
  Ty = cTy;         BlockTx = Tx;
}
/*---------------------------------------------------------------------------*/
#define C_NONE   0 /* Запоминание блоков (и очистка): - просто запомнить     */
#define C_DELETE 1 /* - запомнить с удалением (схлопыванием)                 */
#define C_CLEAR  2 /* - запомнить с очисткой                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void gblockcut (int op)
{
  int y, trm = my_min(Ttxt->txrm, MAXTXRM); make_blkXYsize();
  for (y = blkYmin; y <= blkYmax; y++) {
    TxSetY(Ttxt, y);
    Lleng = TxFRead(Ttxt, Lebuf);
    CSsave1(blkXmin, blkXsize, false);
    if (y < blkYmax) CSclose();
    switch (op) {
    case C_NONE:                                   continue;
    case C_CLEAR:  blktspac(Lebuf+blkXmin, blkXsize); break;
    case C_DELETE:
      blktmov (Lebuf+blkXmax+1,  Lebuf+blkXmin, trm-blkXmax-1);
      blktspac(Lebuf+(trm-blkXsize), blkXsize);          break;
    }
    TxFRep(Ttxt, Lebuf);
} }
void tecsblock() { gblockcut(C_NONE);                          scblkoff(); }
void tesdblock() { gblockcut(LeInsMode ? C_DELETE : C_CLEAR);  scblkoff(); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teclrblock()    /* clear the block (text not moved even in insert mode) */
{
  make_blkXYsize();
  for (int y = blkYmin; y <= blkYmax; y++) {
    TxSetY(Ttxt, y);
    Lleng = TxFRead(Ttxt, Lebuf); blktspac(Lebuf+blkXmin, blkXsize);
            TxFRep (Ttxt, Lebuf);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tedelblock()  /* delete / collapse the block (even in replacement mode) */
{
  int y, trm = my_min(Ttxt->txrm, MAXTXRM); make_blkXYsize(); Tx = blkXmin;
  for (y = blkYmin; y <= blkYmax; y++) {
    TxSetY(Ttxt, y);
    Lleng = TxFRead(Ttxt, Lebuf);
    blktmov (Lebuf+blkXmax+1,  Lebuf+blkXmin, trm-blkXmax-1);
    blktspac(Lebuf+(trm-blkXsize), blkXsize);
    TxFRep(Ttxt, Lebuf);
  }
  scblkoff(); // unlike "clear" command this one always unmark the block (since
}             // the text is collapsed, old selection does not make much sense)
/*---------------------------------------------------------------------------*/
txt *DKtxt = NULL;   /* text for deck (saved in DECKFILNAM "~/.micro7.deck") */
#define DKsize 52
//
void saveToDeck (txt *t, long ty)
{
  QString fname = t->file->full_name;          char *pline = clbuf+1;
  int len, name_len = fname.length(), pos_len = pos_to_clbuf1(t, ty);
  if (DKtxt == NULL) {
    if ((DKtxt = tmDesc(DECKFILNAM, false)) == NULL) return;
    if (!tmLoad(DKtxt))                              return;
  }
  for (TxTop(DKtxt); !qTxBottom(DKtxt); TxDown(DKtxt)) {
    char *tline = TxGetLn(DKtxt, &len);
    if (len > name_len         &&
        tline[name_len] == ':' &&
        memcmp(tline, pline, name_len) == 0) { TxDL(DKtxt); break; }
  }
  TxSetY(DKtxt,     DKsize-1);
  if (DKtxt->txy >= DKsize-1) TxDEL_end(DKtxt);
  TxTop(DKtxt);    TxIL(DKtxt, pline, pos_len);
  DKtxt->txstat |= TS_CHANGED;   tmsave(DKtxt);
}
/*---------------------------------------------------------------------------*/
// Line drawing with pseudo-graphic Unicode characters:
//
static const char ldpgC[] = "┌───┬┐└┴┘│││├┼┤╒╤╕╘╧╛╞╪╡╓╥╖╙╨╜╟╫╢╔═══╦╗╚╩╝║║║╠╬╣";
static unsigned  ldpgDM[] =
{ // ┌    ─    ─    ─    ┬    ┐    └    ┴    ┘    │    │    │    ├    ┼    ┤
  0x05,0x03,0x01,0x02,0x07,0x06,0x09,0x0B,0x0A,0x0C,0x04,0x08,0x0D,0x0F,0x0E,
  // ╒                   ╤    ╕    ╘    ╧    ╛                   ╞    ╪    ╡
  0x14,               0x34,0x24,0x18,0x38,0x28,               0x1C,0x3C,0x2C,
  // ╓                   ╥    ╖    ╙    ╨    ╜                   ╟    ╫    ╢
  0x41,               0x43,0x42,0x81,0x83,0x82,               0xC1,0xC3,0xC2,
  // ╔    ═    ═    ═    ╦    ╗    ╚    ╩    ╝    ║    ║    ║    ╠    ╬    ╣
  0x50,0x30,0x10,0x20,0x70,0x60,0x90,0xB0,0xA0,0xC0,0x40,0x80,0xD0,0xF0,0xE0
};                                         //
#define NLDPG (sizeof(ldpgDM)/sizeof(int)) // directions bitmask: 0xUDLRudlr

static tchar ldpgTC    [NLDPG]; // all line-drawing pseudo-graphic tchars
static tchar ldpgDMtoTC[ 256 ]; // direction mask to LDPG tchar conversion
void ldpgInit()
{
  aftotc(ldpgC,  -1,             ldpgTC );
  memset(ldpgDMtoTC,0,sizeof(ldpgDMtoTC));
  for (unsigned i = 0; i < NLDPG; i++) ldpgDMtoTC[ ldpgDM[i] ] = ldpgTC[i];
}
static unsigned DM (tchar tc, int shift, int mask) // get Direction Mask from
{                                                  //              neighbours
  for (unsigned i = 0; i < NLDPG; i++) if (ldpgTC[i] == tc)
    { unsigned dm = (shift < 0) ? ldpgDM[i] << 1 : ldpgDM[i] >> 1;
                                               return   dm & mask; }
  /* not found => */                           return           0;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void LDPG (int dx, int dy, int dbl)        /* dbl = 0:single,4:double */
{
  unsigned mop = 0, // mask forced by operation
          madd = 0; // addition from neighbours
  txt *t = Ttxt;
  if (qTxBottom(t)) exc(E_MOVEND);
  if (dx == +1) mop = 0x01 << dbl; else madd |= DM(TxChar(t,Tx+1,Ty),+1,0x11);
  if (dx == -1) mop = 0x02 << dbl; else madd |= DM(TxChar(t,Tx-1,Ty),-1,0x22);
  if (dy == +1) mop = 0x04 << dbl; else madd |= DM(TxChar(t,Tx,Ty+1),+1,0x44);
  if (dy == -1) mop = 0x08 << dbl; else madd |= DM(TxChar(t,Tx,Ty-1),-1,0x88);
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  short newx = my_max(Tx+dx, t->txlm);  if (newx >= t->txrm) newx = t->txrm-1;
  long  newy = my_max(Ty+dy,       0);
  tchar           newtc = ldpgDMtoTC[ mop|madd ];
  if (newtc == 0) newtc = ldpgDMtoTC[(mop|madd)&(dbl?0xF0:0x0F)];
  if (newtc == 0)                                    newtc = '?';
  EnterLEmode(); llchar(newtc); Lchange = TRUE;
   ExitLEmode(); tesetxy(newx, newy); vipReady();
//                          vipGotoXY(newx,newy);
  ldpgMode = dbl ? 2 : 1;
}
void teldpgLeft() { LDPG(-1,0,(ldpgMode == 2) ? 4 : 0); }
void teldpgRight(){ LDPG(+1,0,(ldpgMode == 2) ? 4 : 0); }
void teldpgUp()   { LDPG(0,-1,(ldpgMode == 2) ? 4 : 0); }
void teldpgDown() { LDPG(0,+1,(ldpgMode == 2) ? 4 : 0); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teldpgIL()
{
  blktspac(lfbuf,( Lleng = TxTRead(Ttxt, Lebuf) ));
  int    len = 0;
  for (int i = 0; i < Lleng; i++) {
    int mc = DM(Lebuf[i],           +1,0x44),
       mup = DM(TxChar(Ttxt,i,Ty-1),-1,0x88);
    tchar tc;
    if (mc && mup && ( tc = ldpgDMtoTC[mc|mup] )) { lfbuf[i] = tc; len = i+1; }
  }                         // ^
  TxSetY(Ttxt, Ty);         // only insert LDPG char if both current and upper
  TxTIL (Ttxt, lfbuf, len); // position connect to each other, keep space else
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void leldpgIC()             /* <-- called in LE mode (current line in Lebuf) */
{
  int mc = DM(Lebuf[Lx],  +1,0x11),
     mrt = DM(Lebuf[Lx+1],-1,0x22);
  tchar tc = (mc && mrt) ? ldpgDMtoTC[mc|mrt] : 0;    leic2(tc ? tc : ' ');
}
/*---------------------------------------------------------------------------*/

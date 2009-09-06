//------------------------------------------------------+----------------------
// МикроМир07  Clipboard (using wxClipboard) & CS/LS op | (c) Epi MG, 2007-2008
//------------------------------------------------------+----------------------
#include <QApplication>   /* Old le.c (c) Attic 1989,    (c) EpiMG 1996-2003 */
#include <QClipboard>     /* old te.c (c) Attic 1989-96, (c) EpiMG 1998,2001 */
#include <QStringList>    /* old cp.c (c) Attic 1989     (c) EpiMG 1998-99   */
#include "mim.h"
#include "ccd.h"
#include "twm.h"
#include "vip.h"
#include "clip.h"
extern "C" {
#include "le.h"
#include "te.h"
#include "tx.h"
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
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void clipChanged (QClipboard::Mode mode)
{
  if (mode == clipMode) cb_new_data++;
}
void dummyClip::changed1() { clipChanged(QClipboard::Clipboard); }
void dummyClip::changed2() { clipChanged(QClipboard::Selection); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void clipRefocus () { cb_refocusing = true; }
void clipFocusOff()
{
  if (cb_refocusing) cb_refocusing = false;
  else toClipboard();
}
/*---------------------------------------------------------------------------*/
// Внутренний cut/paste буфер: запомненные символы собираятся в ccbuf, строки
// сразу кладутся в LCtxt (в начало добавляется специальный символ по которому
// при paste вставляется новая строка)
//
txt *LCtxt = NULL;           /* Текст-хранилище строк (и всего прочего тоже) */
static tchar ccbuf[MAXLPAC]; /* Буфер запомненных символов (неполная строка) */
static small cclen = 0;      /* Мощность буфера запомненных символов/слов    */
static bool cpopen = false;
static bool cpempt = true;
static char clbuf[MAXLUP];
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void cptinit (bool needContent)
{
  if (LCtxt == NULL) {
    if ((LCtxt = tmDesc(SAVFILNAM, false)) == NULL) exc(E_CPFER);
    if (needContent) tmLoad(LCtxt);
    else   LCtxt->txstat |= TS_NEW;
           LCtxt->txstat |= (TS_FILE|TS_PERM);
} }
void cpsave()
{
  if (LCtxt && (LCtxt->txstat & TS_CHANGED)) tmsave(LCtxt, FALSE);
}
QString clipStatus()  /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
  return QString::fromUtf8(cpopen ? "»" : cb_new_data ? "«" : "·");
}
/*---------------------------------------------------------------------------*/
static void CPempty (void)
{
  cptinit(false); TxEmpt(LCtxt);
  cclen = 0;
  cpopen = cpempt = true;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void CSsave1 (int X, int len, bool trim)  /* Save 1 line at given pos */
{                                                /* & len in Lebuf, trimming */
  if (!cpopen) CPempty();                        /* spaces if requested      */
  int lns = 0;
  while (len-- && cclen < MAXLPAC) {
    tchar tc = Lebuf[X++];
    ccbuf[cclen++] = tc;
    if (!tcharIsBlank(tc)) lns = cclen;
  }
  if (trim && lns && lns < cclen-1) cclen = lns+1; cpempt = false;
}
void lecchar()  { CSsave1(Lx++, 1, false);         }  /* le "copy character" */
void lecdchar() { CSsave1(Lx,   1, false); leDC(); }  /* le "copy & delete"  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int CSsaveW()                                  /* Save word at cursor */
{
  small xl, xr; if (!leNword(&xl, NULL, &xr)) exc(E_NOOP);
                CSsave1(xl, xr-xl, true);       return xr;
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
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/          
static void tecutlin() 
{
  if (cpopen) CSclose();
  else        CPempty();
  clbuf[0] = CpSCH_NEW_LINE; int len = TxRead(Ttxt, clbuf+1);
  TxBottom(LCtxt);
  TxIL(LCtxt, clbuf, len+1); LCtxt->txstat |= TS_CHANGED; cpempt = false;
}
void teclin()  { tecutlin(); Ty++;       }
void tecdlin() { tecutlin(); TxDL(Ttxt); }
/*---------------------------------------------------------------------------*/
void cpclose() { CSclose(); cpopen = false; }
void cpreopen() 
{ 
  cptinit(false);  TxBottom(LCtxt);
  if (!qTxUp(LCtxt)) { TxUp(LCtxt);
    int len = TxRead(LCtxt, clbuf);
    if (len > 0 && clbuf[0] != CpSCH_NEW_LINE)
      cclen = aftotc(clbuf, len, ccbuf);
  }
  cpopen = true;  
}                                  
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/          
#define clipEOL LF

static void toClipboard()
{
  CSclose(); TxTop(LCtxt); bool add_EOL = false; QString cbData;
  while (! qTxDown(LCtxt)) {
    if (add_EOL) cbData += (QChar)clipEOL;
    Lleng = TxTRead(LCtxt, Lebuf); 
    int dx = (Lebuf[0] == (tchar)CpSCH_NEW_LINE) ? 1 : 0;
    cbData += tcs2qstr(Lebuf+dx, Lleng-dx); 
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
/*---------------------------------------------------------------------------*/
static void pasteFromLCtxt (bool in_Ttxt) /* paste into Ttxt or current line */
{
  int len = TxRead(LCtxt, clbuf); TxDown(LCtxt);  if (!len) return;
  char *clpos = clbuf;
  if (clbuf[0] == CpSCH_NEW_LINE) { clpos++; len--;
    if (in_Ttxt) {
      if (Lwnd) ExitLEmode(E_NOCOM);
      TxSetY(Ttxt, Ty);
      TxIL  (Ttxt, clpos, len); Ttxt->txstat |= TS_CHANGED; return;
  } }
       if (Lx <  Lxle) exc(E_EDTBEG);
  else if (Lx >= Lxre) exc(E_EDTEND);
  else {
    if (in_Ttxt && qTxDown(Ttxt)) {  //  Have to take care of inserting empty
      if (Lwnd) ExitLEmode(E_NOCOM); // line at the end-of-text (usually this
      TxSetY(Ttxt, Ty);      teIL(); // is done by LeCommand automatically)
    }
    if (Lwnd == NULL) EnterLEmode(E_NOCOM);           // inserting one-by-one
    cclen = aftotc(clpos, len, ccbuf);                // to enable slow undo
    for (int i = 0; i < cclen; i++) leLLCE(ccbuf[i]); // inside the line and
    cclen = 0;                        Lchange = TRUE; // cleanup in leARGmode
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void cpaste()                              /* Paste: called either from te.c */
{                                          /*                   or from le.c */
  if (!cpopen || cpempt) {                 /* so have to work in any LE mode */
    if (cb_new_data) fromClipboard();
  }
  else   clearClipboard(); // clear system Clipboard to avoid inconsistency
  if (cpempt) exc(E_NOOP); // (first paste resets ccopen, so the second one
  int oldLx = Lx;          // gets data from Clipboard instead of LCtxt)
  cptinit(true);
  cpclose(); TxTop(LCtxt); if (qTxDown(LCtxt)) return;
//
// In entering-argument mode, paste only the content of first line (disabling
// full-line paste) and silently ignore the rest:
//
  if (leARGmode) pasteFromLCtxt(false);
  else {
    pasteFromLCtxt(true);            // First line may be pasted either as line
    while (!qTxDown(LCtxt)) {        // or as characters, exit LE mode for next
      if (Lwnd) ExitLEmode(E_NOCOM); // lines and move cursor to saved position
      Tx = oldLx;     Ty++;          // one line down.   TODO: word "line" used
      pasteFromLCtxt(true);          // above in 3 different meanings, fix that
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
  blkXsize = blkXmax-blkXmin+1;
  blkYsize = blkYmax-blkYmin+1;
}
int Block1size (int *x0, int *x1) /*- - - - - - - - - - - - - - - - - - - - -*/
{
  if (BlockMark) {
    if (BlockTy != Ty) exc(E_BLOCKOUT);
    else {
      make_blkXYsize(); *x0 = blkXmin;
                        *x1 = blkXmax+1; return blkXsize;
  } }                                    return        0;
}
void BlockXYsize (int *dx, int *dy) /*- - - - - - - - - - - - - - - - - - - -*/
{
  if (BlockMark) { make_blkXYsize(); *dx = blkXsize;
                                     *dy = blkYsize; }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *   Block insert/delete character, microMir style:
 */
#define C_IC  1    /* Insert character in block mode (сдвинуть текст влево)  */
#define C_DC -1    /* Delete character in block mode (сдвинуть текст вправо) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void gblockicdc (int op)
{
  int y, trm = my_min(Ttxt->txrm, MAXTXRM); make_blkXYsize();
  tchar *LebufX0 = Lebuf+blkXmin,
        *LebufX1 = Lebuf+blkXmin+1;  int len = trm-blkXmin-1;
  if (op == C_IC) {
    for (y = blkYmax; y >= blkYmin; y--) {
      TxSetY(Ttxt, y);  Lleng = TxTRead(Ttxt, Lebuf); // make sure we can do IC
      if (!tcharIsBlank(Lebuf[trm-1])) exc(E_EDTEND); // operation in all lines
  } }
  for (y = blkYmin; y <= blkYmax; y++) {
    TxSetY(Ttxt, y);
    Lleng = TxFRead(Ttxt, Lebuf);
    switch (op) {
    case C_IC: blktmov(LebufX0, LebufX1, len); *LebufX0     = ' '; break;
    case C_DC: blktmov(LebufX1, LebufX0, len); Lebuf[trm-1] = ' '; break;
    }
    TxFRep(Ttxt, Lebuf);
} }
void teicblock() { gblockicdc(C_IC); }
void tedcblock() { gblockicdc(C_DC); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *   Запоминание блоков:
 */
#define C_NONE   0 /* Просто запомнить                                       */
#define C_DELETE 1 /* Запомнить с удалением (схлопыванием)                   */
#define C_CLEAR  2 /* Запомнить с очисткой                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void gblockcut (int op)
{ 
  int y, trm = my_min(Ttxt->txrm, MAXTXRM);  make_blkXYsize();
  if (blkXsize == 0) exc(E_NOOP);
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
/*---------------------------------------------------------------------------*/

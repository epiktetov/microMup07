//------------------------------------------------------+----------------------
// МикроМир07           Text & Window Manager           | (c) Epi MG, 2004-2014
//------------------------------------------------------+----------------------
#include <QCoreApplication> // need QCoreApplication::arguments
#include <QRegExp>
#include "mim.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "unix.h"
#include "vip.h"
#include "clip.h"
#include "synt.h"
#include "luas.h"
extern "C" {
#include "dq.h"
#include "le.h" // uses Lebuf for dirlist generation
#include "te.h"
#include "tx.h"
#include "ud.h"
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmInitialize (void)
{
  long memsize;
  char *membuf = MemInit(&memsize); DqInit(membuf, memsize);
  TxInit();
  TeInit(); LeStart(); clipStart(); luasInit();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool tmStart (QString param)
{
  QRegExp ptn("(.+):(\\d+):$");
  if (ptn.exactMatch(param)) {
    QString filename = ptn.cap(1);
    QString init_pos = ptn.cap(2);
    long ipos = init_pos.toLong();
    if (!filename.isEmpty() && ipos > 0) return twStart(filename, ipos);
  }                                      return twStart(param,       1);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twStart (QString filename, long ipos)
{
  Twnd = vipNewWindow(NULL, -1, -1, TM_VFORK);
  if (twEdit(Twnd, filename)) {
#ifndef Q_OS_LINUX // force cursor on Mac and Windows (not needed on Linux, as
    vipReady();    // the application window gets focus / raise automatically)
#endif
    Twnd->sctw->mf->raise ();
    Ty = ipos-1; return true; // set initial pos after twEdit (which reset it)
  }
  vipFreeWindow(Twnd); return false;
}
void twExit() { if (vipFreeWindow(Twnd)) mimExit(); }
/*---------------------------------------------------------------------------*/
void wattach (txt *t, wnd *w)
{
  scblkoff(); w->wtext = t;
              w->wnext = t->txwndptr;
                         t->txwndptr = w;
  w->ctx = t->vp_ctx; w->wtx = t->vp_wtx;
  w->cty = t->vp_cty; w->wty = t->vp_wty; t->txstat |= TS_WND;
}
void wupdate (txt *t, wnd *vp)
{
  t->vp_ctx = vp->ctx; t->vp_wtx = vp->wtx;
  t->vp_cty = vp->cty; t->vp_wty = vp->wty;
}
void wdetach (txt *t) /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{                                     /* assuming Twnd is currently attached */
  if (t->txwndptr == NULL) return;
  if (t->txwndptr == Twnd) t->txwndptr = Twnd->wnext;
  else {
    for (wnd *vp = t->txwndptr; vp; vp = vp->wnext)
      if (vp->wnext == Twnd) { vp->wnext = Twnd->wnext; break; }
  }
  if (t->txwndptr == NULL) t->txstat &= ~TS_WND;
  Twnd->wtext = NULL;
  Twnd->wnext = NULL; t->vp_wtx = Twnd->wtx; t->vp_ctx = Tx;
                      t->vp_wty = Twnd->wty; t->vp_cty = Ty;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void twRedraw (txt *t)    /* redraw all attached windows after rename */
{
  for (wnd *w = t->txwndptr; w; w = w->wnext) { vipUpdateWinTitle(w); 
                                                vipRedrawWindow  (w); }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twEdit (wnd *wind, QString filename, txt *referer, bool isNew)
{
  txt *tfound = tmDesc(filename, true, referer);
  if (!tfound || !tmLoad(tfound))  return false;
  Ttxt = tfound;
  Tx = Ttxt->vp_ctx; wattach(Ttxt, wind);
  Ty = Ttxt->vp_cty;
  if (isNew) { Twnd = NULL; vipActivate(wind); }
                      vipUpdateWinTitle(wind);
                      vipRedrawWindow  (wind); return true;
}
/*---------------------------------------------------------------------------*/
void twDirPop (void)
{
  wdetach(Ttxt);
  while (Twnd->dirsp > Twnd->stack) {
    Twnd->dirsp--;
    Twnd->wtx = Twnd->dirsp->dswtx; Tx = Twnd->dirsp->dsx;
    Twnd->wty = Twnd->dirsp->dswty; Ty = Twnd->dirsp->dsy;
    bool ok = twEdit (Twnd, Twnd->dirsp->file->full_name);
    QfsClear(Twnd->dirsp->file);           if (ok) return;
  }
  twExit();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void twDirPush (QString filename, txt *referer)
{
  wdetach(Ttxt);
//
// Do not save pseudo-text used for shell command execution in the stack (since
// such text is unique and does not have proper name anyway), also check space
//
  if (Ttxt->file->ft == QftNOFILE ||
         Twnd->dirsp == Twnd->stack + MAXDIRSTK) vipBell();
  else {
    Twnd->dirsp->dswtx = Twnd->wtx; Twnd->dirsp->dsx = Tx;
    Twnd->dirsp->dswty = Twnd->wty; Twnd->dirsp->dsy = Ty;
    Twnd->dirsp->file = QfsDup(Ttxt->file); Twnd->dirsp++;
  }
  Twnd->wtx = 0;
  Twnd->wty = 0; 
  if (!twEdit(Twnd, filename, referer ? referer : Ttxt)) twDirPop();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void twDirCopy (wnd *from, wnd *to)
{
  dirstk *fromDS, *toDS;
  for (fromDS = from->stack, toDS = to->stack; 
       fromDS < from->dirsp; toDS++, fromDS++) {
    toDS->file = QfsDup(fromDS->file);
    toDS->dswtx = fromDS->dswtx; toDS->dsx = fromDS->dsx;
    toDS->dswty = fromDS->dswty; toDS->dsy = fromDS->dsy;
  }
  to->dirsp = toDS;
}
/*--------------------------------------------------------- Text manager ----*/
static void checkfile (txt *t)
{
  bool isOk = (t->file->ft < QftREAL) || QfsIsUpToDate(t->file);
  bool editable = t->file->writable && !(t->txstat & TS_RDONLY);
  t->txredit = editable ? TXED_YES : TXED_NO;  if (isOk) return;
/*
 * Not ok - have newer file on disk... but if our older version has any unsaved
 * modifications, warn and preserve it (otherwise discard the file contents):
 */
  if (t->txstat & TS_CHANGED) vipBell(); // TODO: ask question!
  else                     TxDiscard(t);
}
void tmCheckFiles (void) // - - - - - - - - - - - - - - - - - - - - - - - - - -
{ 
  for (txt *t = texts; t; t = t->txnext) {
    if ((t->txstat & TS_BUSY) && !(t->txstat & TS_PSEUDO)) checkfile(t);
  }
  for (wnd *w = windows; w; w = w->wdnext) //! make sure that contents for all
    if (w->wtext) tmLoad(w->wtext);        // windows exist in memory - reload
}                                          // texts for all active windows
/*---------------------------------------------------------------------------*/
struct cmdmap { const char *key, *cmd; int txstat; };
cmdmap vcs_commands[] =
{
  { "git:",     "«git show %2:%1»",                           TS_RDONLY },
  { ":",        "«git show %2:%1»",                           TS_RDONLY },
  { "gitlog",   "«git log --pretty='%h (%aN %ad) %s' %2 %1»", TS_GITLOG },
  { "blame",    "«git blame --date short %2 %1»",             TS_GITLOG },
  { "hg:",      "«hg cat -r %2 %1»",                          TS_RDONLY },
  { "annotate", "«hg annotate -n -dq %2 %1»",                 TS_GITLOG },
  { "hglog",    "«hg log --template '{rev} ({author|user}"
                " {date|date}) {desc|firstline}\\n' %2 %1»",  TS_GITLOG },
  { 0,0,0 }
};
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
txt *tmDesc (QString filename, bool needUndo, txt *referer)
{
  QRegExp ptn("([^:]+):(git:|:|gitlog|blame|hg:|hglog|annotate)(.*)");
  short force_txstat = 0;
  short force_clang  = 0;
  if (filename.compare(QfsELLIPSIS) == 0) { // PSEUDO text used by Unix & Lua
    txt *Ctxt;                              //
    for (Ctxt = texts; Ctxt; Ctxt = Ctxt->txnext)
      if ((Ctxt->file && Ctxt->file->ft == QftNOFILE) &&
          (Ctxt->txstat & TS_PSEUDO) &&
         !(Ctxt->txstat & (TS_WND|TS_PERM)) ) break;
//   ^^
// Look for unattached PSEUDO text, clear existing one or create new (no UNDO),
// update the status (just in case), recreate the name (with proper directory):
//
    if  (Ctxt)  TxEmpt(Ctxt);
    else Ctxt = TxNew(FALSE);     Ctxt->txstat |=   TS_PSEUDO |TS_FILE;
    Ctxt->txredit = TXED_YES;     Ctxt->txstat &= ~(TS_CHANGED|TS_RDONLY);
    qfile *new_file = QfsNew(QfsELLIPSIS, referer ? referer->file : NULL);
    QfsClear(Ctxt->file);
    Ctxt->file = new_file; return Ctxt;
  }
  else if (ptn.exactMatch(filename)) {
    for (cmdmap *p = vcs_commands; p->key; p++)
      if (ptn.cap(2) == p->key) {
        filename = Utf8(p->cmd).arg(ptn.cap(1)).arg(ptn.cap(3));
        force_txstat = p->txstat;
        force_clang  = SyntKnownLang(ptn.cap(1)); break;
  }   }
  qfile *fd = QfsNew(filename, referer ? referer->file : NULL);
  filename = fd->full_name;
       if (fd->ft == QftDIR) force_txstat |= TS_DIRLST;
  else if (fd->ft == QftTEXT) force_clang  = SyntKnownLang(fd->name);
  txt *t;
  for (t = texts; t; t = t->txnext) {
    if ((t->txstat & TS_BUSY) && t->file &&
        filename.compare(t->file->full_name, QfsIGNORE_CASE) == 0) {
      QfsClear(fd);
      checkfile(t); return t; // found existing text
  } }
  t = TxNew(needUndo ? TRUE : FALSE); // not found => create new one
  t->file = fd;
  TxEnableSynt(t,force_clang);
  t->txstat  |=  force_txstat; t->txredit = TXED_YES;
  return t;                    // ^
}                              // make QftNOFILE editable (no tmLoad follows)
/*---------------------------------------------------------------------------*/
#ifdef UNIX
# include <errno.h>
# define TwmBACKUP_EXT "~"
# define QStingFERROR QString("[%1] %2 in %3").arg(errno).arg(strerror(errno))
#else
# define TwmBACKUP_EXT ".bkup"
# define QStingFERROR QString("Error in %1")
#endif
bool teferr (txt *t) { QString msg = QStingFERROR.arg(t->file->full_name);
                       vipError(msg);                        return false; }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void tmDirLST (txt *t)
{
  QDir *dir = QfsDir(t->file);
  QStringList filters;   filters << (t->file->name);
  QFileInfoList filist = dir->entryInfoList(filters, 
         QDir::AllEntries|QDir::Hidden|QDir::System, QDir::Name);

  for (QFileInfoList::const_iterator it  = filist.constBegin();
                                     it != filist.constEnd(); ++it) {
    QString line;
    QFile::Permissions attr = it->permissions();
    ushort ftattr = it->isDir() ? 0x281 : 0x280;
    char type = QFile::symLinkTarget(it->filePath()).isEmpty()
                       ? (it->isDir() ? 'd' : '-')   : 'l';
    // Note:
    // unlike the standard 'ls -l' command, the list shows file permissions of
    // the TARGET file (but with first character changed to 'l' for symlinks)
    //
    line.sprintf("%lc%c%c%c%c%c%c%c%c%c%c%lc%12d %s %lc%ls", ftattr, type,
         attr & QFile::ReadOwner  ? 'r' : '-',
         attr & QFile::WriteOwner ? 'w' : '-',
         attr & QFile::ExeOwner   ? 'x' : '-',
         attr & QFile::ReadGroup  ? 'r' : '-',
         attr & QFile::WriteGroup ? 'w' : '-',
         attr & QFile::ExeGroup   ? 'x' : '-',
         attr & QFile::ReadOther  ? 'r' : '-',
         attr & QFile::WriteOther ? 'w' : '-',
         attr & QFile::ExeOther   ? 'x' : '-',  0x280,
         int(it->size()),  QfsModDateText(*it).cStr(),
         ftattr, (wchar_t *)(it->fileName().utf16()));
    TxTIL(t, Lebuf, qstr2tcs(line, Lebuf)); TxDown(t);
  } 
  if (t->txudeq) udclear(t); //<- reset undo start to this point (initial blank
  t->vp_ctx = t->txlm = DIRLST_FNPOS; //         state is not very interesting)
  t->txstat |= TS_DIRLST | TS_RDONLY;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void check_MCD (txt *t) // TODO: check file contents instead
{
  if (t->file->name.compare(QfsROOTFILE, QfsIGNORE_CASE) == 0) {
    t->txstat |= TS_MCD; t->txlm = 0;
                         t->txrm = MCD_LEFT;
} }
/*---------------------------------------------------------------------------*/
void tmDumpList() /* dumps the list of all text objects (for debug purposes) */
{
  for (txt *tx = texts; tx; tx = tx->txnext) {
    fprintf(stderr, " %04x:%s,%s\n", tx->txstat,
           tx->file ? tx->file->     name.cStr() : "Null",
           tx->file ? tx->file->full_name.cStr() : "Null");
} }
bool tmDoLoad (txt *t)  /*- - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
  switch (t->file->ft) {       // NOFILE file always considered "loaded" (since
  case QftNOFILE:              // there is no place to reload them from anyway)
  case QftNIL:    return true; //-
  case QftPSEUDO: tmLoadXeq(t); break; // - load by executing command: unix.cpp
  case QftDIR:    tmDirLST( t); break; // - load directory listing (see above)
  case QftTEXT:
    {
      int oc = QfsOpen(t->file, FO_READ); // oc = -1:fail,0:empty,1:trunc,2:ok
      if (oc < 0) return false;           //
      if (oc > 0) oc = DqLoad(t->txdstk, t->file, t->file->size);
      QfsClose(t->file);
      if (oc <  0) return false; // zero result (empty file) is acceptable,
      if (oc == 1)               // truncated file too (make 'em read-only,
         t->txstat |= TS_RDONLY; // as won't be possible to save correctly)
  } }
  bool editable = t->file->writable && !(t->txstat & TS_RDONLY);
  t->txredit = editable ? TXED_YES : TXED_NO;
  t->txstat &= ~TS_CHANGED;     check_MCD(t);  // double-check file type
  t->txstat |=  TS_FILE;                       //
  if (t->clang == CLangNONE) TxEnableSynt(t, SyntSniffText(t));
  luasNtxt(t);
  if (t->txudeq != NIL) { t->txstat  |= TS_UNDO;
                          t->txudfile = t->txudcptr; } TxRecalcMaxTy(t);
  return true;
}
bool tmLoad (txt *t)  /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
//  fprintf(stderr, "tmLoad(%s)\n", t->file ? t->file->name.cStr() : "Null");
//  tmDumpList();
//+
  bool ok;
  if (t->txstat & TS_FILE) ok = true;
  else              ok = tmDoLoad(t);
  if (ok) {                                  t->txlructr = 0;
    for (t = texts; t != NIL; t = t->txnext) t->txlructr++;
    return true;
  }
  return teferr(t);
}
#ifdef Q_OS_MAC /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmLoadIn (txt *t, QString  filename) /* only for MacEvents::eventFilter */
{
  QfsClear(t->file); t->file = QfsNew(filename, NULL);
  tmLoad(t);
  if (Twnd) { vipUpdateWinTitle(Twnd);
}             vipRedrawWindow  (Twnd); vipReady(); }
#endif
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool tmReLoad (txt *t)   // Forced reload -- check the status of real file (and
{                        // discard outdated contents if necessary), but always
  switch (t->file->ft) { // re-create directory listings and PSEUDO files anew:
  case QftNOFILE:        //
  default:                                         return false;
  case QftTEXT:   checkfile(Ttxt); tmLoad  (Ttxt); return  true;
  case QftPSEUDO: TxDiscard(Ttxt); tmLoad  (Ttxt); return  true;
  case QftDIR:    TxDiscard(Ttxt); tmDoLoad(Ttxt); return  true;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmUnlink (txt *t)       /* unlink text from the file (change to PSEUDO) */
{
  qfile *pseudo = QfsNew(QfsELLIPSIS, t->file);
  QfsClear(t->file);          t->file = pseudo;
  t->txstat |=  TS_PSEUDO;
  t->txredit =   TXED_YES;
}
/*---------------------------------------------------------------------------*/
bool tmsave (txt *t, bool needBackup)
{
  if (t->file == NULL || (t->txstat & (TS_PSEUDO|TS_DIRLST)) ||
                        !(t->txstat &  TS_CHANGED)) return TRUE;
  else
  if (t->file->ft != QftTEXT) return FALSE; // use twSave() for other types
  if (needBackup) {
    QString backup_name = t->file->name + TwmBACKUP_EXT;
    qfile *bfile = QfsNew(backup_name, t->file);
    QfsDelete(bfile);
    if (QfsRename(t->file, backup_name)) t->txstat |= TS_NEW;
    else                                    return teferr(t);
  }
  TxTop(t); long deqsize = DqLen(t->txdstk);
  if (deqsize) {
    int omode = ((t->txstat & TS_NEW) || t->file->size > deqsize) ? FO_NEW
                                                                  : FO_WRITE;
    if (QfsOpen(t->file, omode) < 0) return teferr(t);
    bool ok = DqSave(t->txdstk, t->file);
    QfsClose(t->file); if (ok) QfsUpdateInfo(t->file);
                       else          return teferr(t); t->txstat &= ~TS_NEW;
  }
  else if (! (t->txstat & TS_NEW)) QfsDelete(t->file);
  t->txudfile = t->txudcptr;
  t->txstat  &= ~(TS_CHANGED|TS_SAVERR);  return true;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool tmSaveAs (txt *t, QString  fn)
{
  if (fn.compare(t->file->full_name, QfsIGNORE_CASE) == 0) return tmsave(t, 0);
  else {
    for (txt *to = texts; to; to = to->txnext) {
      if ((to->txstat & TS_BUSY) && to->file &&
          fn.compare(to->file->full_name, QfsIGNORE_CASE) == 0) {
//
// Found other text with matching name: check if this is a ghost (has no active
// windows, delete it), or some active/unsaved text (then abort the operation):
//
        if (to->txstat & (TS_CHANGED|TS_PERM|TS_WND)) { vipBell(); 
                                                        return false; }
        else TxDel(to);
    } }
    qfile *fd = QfsNew(fn, t->file);
    if (fd->ft != QftTEXT) { delete fd; vipBell(); return false; }
    else {
      QfsClear(t->file);     t->file = fd; check_MCD(t);
      t->txstat &= ~(TS_DIRLST|TS_PSEUDO);
      t->txstat |= TS_CHANGED;
      t->txredit =   TXED_YES; tmsave(t, FALSE); twRedraw(t); return true;
} } }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twSave (txt *t, wnd *vp, bool needBackup)
{
  if (t->txstat & (TS_PSEUDO|TS_DIRLST)) return true;
  else if (t->file->ft == QftTEXT)       return tmsave(t, needBackup);
  else if (t->txstat & TS_CHANGED) {
    QString new_name = vp->sctw->mf->saveAsDialog(t);
    if (!new_name.isEmpty()) return tmSaveAs(t, new_name);
  }                          return true;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmSaveAll (void)                    /* сохранить ВСЕ изменения на диске */
{
  for (txt *t = texts; t; t = t->txnext) {
    if (t->txstat & TS_CHANGED) {
      if (t->txwndptr) twSave(t, t->txwndptr, false); // <- saves QftNOFILE too
      else             tmsave(t,              FALSE); //    (will ask for name)
} } }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twSafeClose (txt *t, wnd *vp)
{
  if ((t->txstat & TS_CHANGED) && !(t->txstat & (TS_PSEUDO|TS_DIRLST))
                               && t->txwndptr->wnext == NULL) return false;
  Twnd =  vp;
  wdetach(t); twExit(); return true; /* NOTE: directory stack is NOT checked */
}
/*---------------------------------------------------------------------------*/
#define  FPROMPT AF_PROMPT "File:"
#define LFPROMPT 5
static tchar filebuf[MAXLPAC];
static int   filebuflen = 0;
static int  filebufFlag = 0;
static tchar *fileHistory[LE_HISTORY_SIZE] = { 0 };
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmFentr (void)                   /* file name ENTeR -- ввести имя файла */
{
  if (! filebuflen) filebuflen = aftotc(FPROMPT, -1, filebuf);
  LenterARG(filebuf, &filebuflen, LFPROMPT, fileHistory,
                     &filebufFlag, TM_F1ENTR, TM_F2ENTR, 0);
}
QString tmFentrFN (void)
{
  QString fn = tcs2qstr(filebuf+LFPROMPT, filebuflen-LFPROMPT);
  fn.replace(QString::fromUtf8("•"), Ttxt->file->name);
  return fn; //                 ^
}            // attrs stripped already, but this char not common in filenames
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmDoFentr (void) { if (filebuflen > LFPROMPT)  twDirPush(tmFentrFN()); }
void tmDoFentr2(void) { if (filebuflen > LFPROMPT) twShowFile(tmFentrFN()); }
void twShowFile(QString name)
{
  wnd *wind = vipSplitWindow(Twnd, TM_VFORK);
  if  (wind)  twEdit(wind, name, NULL, true);
}
void twOpenALEStx() { twShowFile (ALESFILNAM); }
void twOpenBLAHtx() { twShowFile (BLAHFILNAM); }
void twNewLuaText() { twShowFile(":/new.lua");   // open file, then unlink it
     tmUnlink(Ttxt);  vipUpdateWinTitle(Twnd); } // (and update window title)
/*---------------------------------------------------------------------------*/
static void tmExtractIncs (txt *mcd, QStringList& incList)
{
  QRegExp re("(?:\\((macx|unix|win32)\\))?\\s*([^(].+)");
  TxBottom(mcd);
  while (!qTxTop(mcd)) {  TxUp(mcd); // scan upward all 'inc:' lines
        Lleng = TxTRead(mcd, Lebuf); //
    if (Lleng < 1 || Lebuf[0] != (AT_PROMPT|'i')) return;
    tlesniff(mcd);
    QString line = tcs2qstr(Lebuf+Lxle, Lleng-Lxle);
    if (re.exactMatch(line) && (re.cap(1).isEmpty() || re.cap(1) == QtPLATF))
      incList.prepend(re.cap(2));
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void tmFnewSearchIncs (QString foundName)
{
       if (foundName.isEmpty ())                         return;
  else if (QfsExists(foundName)) { twDirPush(foundName); return; }
  else {
    QStringList::const_iterator  it;
    QStringList incList, searchList;
    for (txt *t = texts; t; t = t->txnext)
      if ((t->txstat & TS_MCD) &&
          (t->file->path == Ttxt->file->path)) tmExtractIncs(t, incList);
#ifdef UNIX
    incList.append(QString("/usr/local/include,/usr/include"));
#endif
    for (it = incList.constBegin(); it != incList.constEnd(); it++)
#if (QT_VERSION >= 0x040500)
      searchList.append(it->split(QRegExp(",\\s*")));
#else
      { QStringList subList = it->split(QRegExp(",\\s*"));
        QStringList::const_iterator st;
        for (st = subList.constBegin(); st != subList.constEnd(); st++)
          searchList.append(*st);
      }
#endif
    for (it = searchList.constBegin(); it != searchList.constEnd(); it++) {
      QString filename = (*it) + "/" + foundName;
      if (QfsExists(filename)) { twDirPush(filename); return; }
    }
    twDirPush(foundName); // enter that file even if it is not know to exist
} }                       // (create empty text)
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmFnewByTtxt (void)                 /* войти в новый файл (имя из Ttxt) */
{
  if (!tleread()) return;
  int lm, rm;                      // sets lm/rm, nothing more to do
       if (Block1size(&lm, &rm)) ; //-
  else if ((Ttxt->txstat & TS_MCD) && !(Lebuf[0] & AT_PROMPT)) {
#define USE_MCD_MARGIN(LEFTorRIGHT) \
  if (Lleng > LEFTorRIGHT &&        \
  (char)Lebuf[LEFTorRIGHT] == LDS_MCD) { lm = LEFTorRIGHT+1; rm = Lleng; }
         USE_MCD_MARGIN(MCD_RIGHT) //
    else USE_MCD_MARGIN(MCD_LEFT)  // allow both double-delimiter (64+69th pos)
    else return;                   // and single-delimiter (64th pos only) fmts
  }
  else if (Ttxt->txstat & TS_DIRLST) { lm = DIRLST_FNPOS; rm = Lleng; }
  else if (Ttxt->txstat & TS_GITLOG) { tmSyncPos();           return; }
  else if (Tx > Lleng) return;
  else {
                 for (lm = Tx; lm >= 0    && (uchar)Lebuf[lm]   != 0xAB;) lm--;
    if (lm >= 0) for (rm = Tx; rm < Lleng && (uchar)Lebuf[rm-1] != 0xBB;) rm++;
    else {
      lm = rm = Tx;
      while (lm > 0     && !strchr(" \'\",(<\xBB)", (char)Lebuf[lm-1])) lm--;
      while (rm < Lleng && !strchr(" \'\",>)",      (char)Lebuf[rm]  )) rm++;
      tmFnewSearchIncs(tcs2qstr(Lebuf+lm, rm-lm));                    return;
  } }
  if (lm < rm) twDirPush(tcs2qstr(Lebuf+lm, rm-lm), Ttxt);
}
/*---------------------------------------------------------------------------*/
static void infilnam (void)      /* return by Enter, see teCR() in file te.c */
{
  if (! (Ttxt->txstat & TS_MCD)) { vipBell(); return; }
  Ttxt->txrm = MAXTXRM;
  Ttxt->txlm = MCD_LEFT+1; 
  if (tleread() && (char)Lebuf[MCD_RIGHT] == LDS_MCD) Tx = MCD_RIGHT+1;
  else                                                Tx = MCD_LEFT +1;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int TmCommand (int kcode)
{
  wnd *wind;
  jmp_buf tmenv;  int x; excptr = & tmenv ;
                         x = setjmp(tmenv); if (x) return x;
  switch (kcode) {
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  case TM_UPDATE: tmSaveAll(); break;        /* сохранить изменения на диске */
  case TM_EXIT:                              /* выйти из файла с сохранением */
    if (Ttxt != LCtxt &&
             !twSave(Ttxt, Twnd, false)) return E_SFAIL;
    twDirPop();
    break;
  case TM_EXBACK:                   /* выйти, сохранив новую и старую версию */
    if (Ttxt != LCtxt &&
             !twSave(Ttxt, Twnd, true)) return E_SFAIL;
    twDirPop();
    break;
  case TM_EXOLD:                            /* выйти из файла без сохранения */
    if (Ttxt != LCtxt && 
        (Ttxt->txstat & TS_CHANGED)) { DqEmpt(Ttxt->txustk);
                                       DqEmpt(Ttxt->txdstk);
      Ttxt->txy = 0;    Ttxt->txudcptr = Ttxt->txudlptr = 0;
      if (Ttxt->txudeq) DqEmpt(Ttxt->txudeq);
      Ttxt->txstat &=  ~(TS_FILE|TS_CHANGED);
    }
    twDirPop();
    break;
  case TM_QUIT:                                      /* выйти из окна совсем */
    Twnd->sctw->mf->safeClose(Twnd->sctw);
    break;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  case TM_FNEW:   tmFnewByTtxt(); break; /* войти в новый файл (имя из Ttxt) */
  case TM_FENTR:  tmFentr  ();    break; /* ввести имя файла...              */
  case TM_F1ENTR: tmDoFentr ();   break; /* ...войти в этот файл (тут же)    */
  case TM_F2ENTR: tmDoFentr2();   break; /* ...войти в файл (в новом окне)   */
  case TM_SHELL:  x2enter();      break; /* ввести и выполнить команду shell */
  case TM_FEXEC:
  case TM_F2EXEC:
    tmSaveAll(); /* save current changes - just in case */
    tmshell(kcode);
    break;
  case TM_INFILE: infilnam(); break; /* go to filename field in micros.dir   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  case TM_HFORK:                     /* горизонтальный fork и к нижнему окну */
  case TM_VFORK:                     /* create new window и к правому окну   */
    wind = vipSplitWindow(Twnd, KbCode);
    if (wind) {
      wupdate  (Ttxt, Twnd);
      wattach  (Ttxt, wind); vipUpdateWinTitle(wind);
      twDirCopy(Twnd, wind); vipActivate      (wind);
                             vipRedrawWindow  (Twnd); return E_OK;
    }
    else return E_SFAIL;

  case TM_UWINDOW: case TM_LWINDOW: case TM_RWINDOW:
  case TM_DWINDOW:
    wind = vipFindWindow(Twnd, KbCode);
    if (wind) {
      vipActivate(wind);
      vipFocus   (wind); return E_OK;
    }
    else return E_SFAIL;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  case TM_RELOAD: if (tmReLoad(Ttxt)) twRedraw(Ttxt); /* ^R = forced re-load */
                  else                return E_SFAIL;
    break;
  case TW_GRAD1: case TW_GRAD3:
  case TW_GRAD2: case TW_GRAD4: Twnd->sctw->SetGradFromPool(kcode - TW_GRAD1);
    break;
  case TM_GREP:
  case TM_GREP2:   return (tmGrep(kcode) < 0) ? E_SFAIL : E_OK;
  case TM_SYNCPOS: return (tmSyncPos()   < 0) ? E_SFAIL : E_OK;
  case TM_LUAF:    return luasExec(Ttxt,false);
  case TM_LUAS:    return luasExec(Ttxt, true);
  case TM_LUAN:    twNewLuaText(); return E_OK;
  case TM_LUAA:    twOpenALEStx(); return E_OK;
  case TM_GOBLAH:  twOpenBLAHtx(); return E_OK;
  default:
    return E_NOCOM;
  } return E_OK;
}
/*---------------------------------------------------------------------------*/

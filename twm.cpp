//------------------------------------------------------+----------------------
// МикроМир07           Text & Window Manager           | (c) Epi MG, 2004-2011
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
  TeInit(); LeStart(); clipStart();
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
#ifdef Q_OS_WIN
    vipReady();
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
  t->txstat |= TS_WND;
  w->wcx = t->cx; w->wtx = t->tcx;
  w->wcy = t->cy; w->wty = t->tcy;
}
void wdetach (txt *t, wnd *w) /*- - - - - - - - - - - - - - - - - - - - - - -*/
{
  if (t->txwndptr == NIL) return;
  if (t->txwndptr == w) t->txwndptr = w->wnext;
  else {
    wnd *w1;
    for (w1 = t->txwndptr; w1->wnext != w; w1 = w1->wnext)
      if (w1->wnext == NIL) return;
    w1->wnext = w->wnext;
  }
  t->cx = Tx; t->tcx = w->wtx;  w->wtext = NULL;
  t->cy = Ty; t->tcy = w->wty;  w->wnext = NULL;
  if (t->txwndptr == NULL) t->txstat &= ~TS_WND;
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
  Tx = Ttxt->cx; wattach(Ttxt, wind);
  Ty = Ttxt->cy;
  if (isNew) { Twnd = NULL; vipActivate(wind); }
                      vipUpdateWinTitle(wind);
                      vipRedrawWindow  (wind); return true;
}
/*---------------------------------------------------------------------------*/
void twDirPop (void)
{
  wdetach(Ttxt, Twnd);
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
  wdetach(Ttxt, Twnd);
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
static void discardfile (txt *t)
{
  if (t->txstat & TS_FILE) {
    DqEmpt(t->txustk); t->txy = 0;
    DqEmpt(t->txdstk); t->txstat &= ~TS_FILE;
  }
  if (t->txstat & TS_UNDO) { DqEmpt(t->txudeq); 
                                    t->txudcptr = t->txudlptr = 0; }
}
static void checkfile (txt *t) // - - - - - - - - - - - - - - - - - - - - - - -
{
  bool ok = (t->file->ft < QftREAL) || QfsIsUpToDate(t->file);
  t->txredit = t->file->writable ? TXED_YES : TXED_NO; if (ok) return;
/*
 * Not ok - have newer file on disk... but if our older version has any unsaved
 * modifications, warn and preserve it (otherwise discard the file contents):
 */
  if (t->txstat & TS_CHANGED) vipBell(); // TODO: ask question!
  else                   discardfile(t);
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
  { "git",    "«git show %2:%1»",                           TS_RDONLY },
  { "",       "«git show %2:%1»",                           TS_RDONLY },
  { "hg",     "«hg cat -r %2 %1»",                          TS_RDONLY },
  { "blame",  "«git blame %2 %1»",                          TS_GITPL  },
  { "gitlog", "«git log --pretty='%h (%aN %ad) %s' %2 %1»", TS_GITPL  },
  { 0,0,0 }
};
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
txt *tmDesc (QString filename, bool needUndo, txt *referer)
{
  QRegExp ptn("(.+):(git|gitlog|blame||hg):(.*)");
  small force_txstat = 0;
  small force_clang  = 0;
       if (filename.compare(":clip") == 0) filename = QString(SAVFILNAM);
  else if (filename.compare(":help") == 0) {
    filename = QCoreApplication::arguments().at(0);
#ifdef Q_OS_MAC
    filename.replace(QRegExp("/MacOS/[^/]+$"), "/Resources/micro.keys");
#else
    filename.replace(QRegExp("[^/]+$"), "micro.keys");
#endif
    force_txstat = TS_RDONLY;
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
    line.sprintf("%lc%c%c%c%c%c%c%c%c%c%c%lc%12d %s %lc%ls", ftattr,
         it->isDir()              ? 'd' : '-',
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
  if (t->txudeq)      udclear(t); //<- reset undo start to this point (initial
  t->cx = t->txlm = DIRLST_FNPOS; //   blank state is not very not interesting)
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void check_MCD (txt *t) // TODO: check file contents instead
{
  if (t->file->name.compare(QfsROOTFILE, QfsIGNORE_CASE) == 0) {
    t->txstat |= TS_MCD; t->txlm = 0;
                         t->txrm = MCD_LEFT;
  }
  else { t->txstat &= ~TS_MCD; t->txlm = 0;
                               t->txrm = MAXTXRM; }
}
bool tmDoLoad (txt *t)  /*- - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
//  fprintf(stderr, "tmLoad(%s)\n", t->file ? t->file->name.cStr() : "Null");
//  for (txt *tx = texts; tx; tx = tx->txnext) {
//    fprintf(stderr, " %04o%s\n", tx->txstat,
//                                 tx->file ? tx->file->name.cStr() : "Null");
//  }
//+
  int oc = 0;
  switch (t->file->ft) {       // NOFILE file always considered "loaded" (since
  case QftNOFILE:              // there is no place to reload them from anyway)
  case QftNIL:    return true; //-
  case QftPSEUDO: tmLoadXeq(t); break; // - load by executing command: unix.cpp
  case QftDIR:    tmDirLST( t); break; // - load directory listing (see above)
  case QftTEXT:
      oc = QfsOpen(t->file, FO_READ);  // DqLoad: -1:fail,0:empty,1:trunc,2:ok
      if (oc < 0) return false;        //
      if (oc > 0) oc = DqLoad(t->txdstk, t->file, t->file->size);
      QfsClose(t->file);
      if (oc < 0) return false; // zero result (empty file) is also acceptable
  }
  bool editable = !(t->txstat & TS_RDONLY) && t->file->writable && (oc != 1);
  t->txredit = editable ? TXED_YES : TXED_NO;
  t->txstat &= ~TS_CHANGED;     check_MCD(t); TxRecalcMaxTy(t);
  t->txstat |=  TS_FILE;
  if (t->txudeq != NIL) { t->txstat  |= TS_UNDO;
                          t->txudfile = t->txudcptr; } return true;
}
bool tmLoad (txt *t)  /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
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
}             vipRedrawWindow  (Twnd); }
#endif
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool tmReLoad (txt *t)   // Forced reload -- check the status of real file (and
{                        // discard outdated contents if necessary), but always
  switch (t->file->ft) { // re-create directory listings and PSEUDO files anew:
  case QftNOFILE:        //
  default:                                           return false;
  case QftTEXT:   checkfile  (Ttxt); tmLoad  (Ttxt); return  true;
  case QftPSEUDO: discardfile(Ttxt); tmLoad  (Ttxt); return  true;
  case QftDIR:         TxEmpt(Ttxt); tmDoLoad(Ttxt); return  true;
} }
/*---------------------------------------------------------------------------*/
BOOL tmsave (txt *t, BOOL needBackup)
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
    BOOL ok = DqSave(t->txdstk, t->file);
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
    if (new_name.isEmpty()) return false;
    else                    return tmSaveAs(t, new_name);
  } else                    return true;
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
  if ((t->txstat & TS_CHANGED) && t->txwndptr->wnext == NULL) return false;
  wdetach(t, vp);
  Twnd = vp; twExit(); return true; /* NOTE: directory stack is NOT checked */
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
void tmDoFentr (void) { if (filebuflen > LFPROMPT)
                          twDirPush(tcs2qstr(filebuf   +LFPROMPT,
                                             filebuflen-LFPROMPT)); }
void tmDoFentr2 (void)
{ 
  if (filebuflen > LFPROMPT) {
    wnd *wind = vipSplitWindow(Twnd, TM_VFORK);
    if (wind) twEdit(wind, tcs2qstr(filebuf   +LFPROMPT,
                                    filebuflen-LFPROMPT), NULL, true);
} }
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
  QStringList::const_iterator  it; if (foundName.isEmpty()) return;
  QStringList incList, searchList;
  for (txt *t = texts; t; t = t->txnext)
    if (t->txstat & TS_MCD) tmExtractIncs(t, incList);

#ifndef Q_OS_WIN
  incList.append(QString("/usr/local/include,/usr/include"));
#endif
  for (it = incList.constBegin(); it != incList.constEnd(); it++)
    searchList.append(it->split(QRegExp(",\\s*")));

  for (it = searchList.constBegin(); it != searchList.constEnd(); it++) {
    QString filename = (*it) + "/" + foundName;
    if (QfsExists(filename)) { twDirPush(filename); return; }
  }
  twDirPush(foundName);
}
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
    else return;                   // and single-delimiter (64th posd only) fmt
  }
  else if (Ttxt->txstat & TS_DIRLST) { lm = DIRLST_FNPOS; rm = Lleng; }
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
#ifdef UNIX
  case TM_SHELL: x2enter(); break;       /* ввести и выполнить команду shell */
  case TM_FEXEC:
  case TM_F2EXEC:
    tmSaveAll(); /* save current changes - just in case */
    tmshell(kcode);
    break;
#endif
  case TM_INFILE: infilnam(); break; /* go to filename field in micros.dir   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  case TM_HFORK:                     /* горизонтальный fork и к нижнему окну */
  case TM_VFORK:                     /* create new window и к правому окну   */
    wind = vipSplitWindow(Twnd, KbCode);
    if (wind) {
      wattach  (Ttxt, wind); Ttxt->tcx = Twnd->wtx; wind->wcy = Ty;
      twDirCopy(Twnd, wind); Ttxt->tcy = Twnd->wty; wind->wcx = Tx;
      vipActivate(wind);
      vipUpdateWinTitle(wind);
      vipRedrawWindow  (wind); return E_OK;
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
#ifdef UNIX
  case TM_CALC: tmCalcBC(); break;
  case TM_GREP:
  case TM_GREP2:   return (tmGrep(kcode) < 0) ? E_SFAIL : E_OK;
  case TM_SYNCPOS: return (tmSyncPos()   < 0) ? E_SFAIL : E_OK;
#endif
  default:
    return E_NOCOM;
  } return E_OK;
}
/*---------------------------------------------------------------------------*/

//------------------------------------------------------+----------------------
// МикроМир07           Text & Window Manager           | (c) Epi MG, 2004-2011
//------------------------------------------------------+----------------------
#include <QCoreApplication> // need QCoreApplication::arguments
#include <QRegExp>
#include "mim.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#ifdef UNIX
# include "unix.h"
#endif
#include "vip.h"
#include "clip.h"
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
  TeInit(); UdInit();  clipStart();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool tmStart (QString param)
{
  QRegExp ptn("(.+):(\\d+):$");
  if (ptn.exactMatch(param)) {
    QString filename = ptn.cap(1);
    QString init_pos = ptn.cap(2);
    long ipos = init_pos.toLong ();
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
    Ty = ipos-1; return true; // set initial pos after wmedin (which reset it)
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
                                                          wredraw(w); }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twEdit (wnd *wind, QString filename, txt *referer)
{
  txt *tfound = tmDesc(filename, true, referer);
  if (!tfound || !tmLoad(tfound))  return false;
  Ttxt = tfound;
  Tx = Ttxt->cx;
  Ty = Ttxt->cy;     wattach(Ttxt, wind);
  vipUpdateWinTitle(wind); wredraw(wind); return true;
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
  if ( ((Ttxt->txstat & TS_PSEUDO) && !(Ttxt->txstat & TS_DIRLST))
                        || Twnd->dirsp == Twnd->stack + MAXDIRSTK ) vipBell();
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
  bool ok = (t->file->ft == QftPSEUDO) || QfsIsUpToDate(t->file);
  t->txredit = t->file->writable ? TXED_YES : TXED_NO;   if (ok) return;
/*
 * Not ok - have newer file on disk... but if our older version has any unsaved
 * modifications, warn and preserve it.
 */
  if (t->txstat & TS_CHANGED) { vipBell(); return; } // TODO: ask question!
  if (t->txstat & TS_FILE) {
    DqEmpt(t->txustk); t->txy = 0;
    DqEmpt(t->txdstk); t->txstat &= ~TS_FILE;
  }
  if (t->txstat & TS_UNDO) {
    DqEmpt(t->txudeq); t->txudcptr = t->txudlptr = 0;
    // t->txstat &= ~TS_UNDO; keeping UNDO bit since txudeq still here
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmCheckFiles()
{ 
  for (txt *t = texts; t; t = t->txnext) 
    if ((t->txstat & TS_BUSY) && !(t->txstat & TS_PSEUDO)) checkfile(t);

  for (wnd *w = windows; w; w = w->wdnext) //! make sure that contents for all
    if (w->wtext) tmLoad(w->wtext);        // windows exist in memory - reload
}                                          // texts for all active windows
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void check_MCD (txt *t)
{
  // TODO; better check:
  //
  if (t->file->name.compare(QfsROOTFILE, QfsIGNORE_CASE) == 0) {
    t->txstat |= TS_MCD; t->txlm = 0;
                         t->txrm = MCD_LEFT;
  }
  else { t->txstat &= ~TS_MCD; t->txlm = 0;
                               t->txrm = MAXTXRM; }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
txt *tmDesc (QString filename, bool needUndo, txt *referer)
{
  small force_txstat = 0;
  if (filename.compare(":clip") == 0) filename = QString(SAVFILNAM);
  if (filename.compare(":help") == 0) {
    filename = QCoreApplication::arguments().at(0);
#ifdef Q_OS_MAC
    filename.replace(QRegExp("/MacOS/[^/]+$"), "/Resources/micro.keys");
#else
    filename.replace(QRegExp("[^/]+$"), "micro.keys");
#endif
    force_txstat = TS_RDONLY;
  }
  qfile *fd = QfsNew(filename, referer ? referer->file : NULL);
  filename = fd->full_name;
  txt *t;
  for (t = texts; t; t = t->txnext)
    if ((t->txstat & TS_BUSY) && t->file &&
        filename.compare(t->file->full_name, QfsIGNORE_CASE) == 0) {
      QfsClear(fd);
      checkfile(t); return t; // found existing text
    }
  while ((t = TxNew(needUndo ? TRUE : FALSE)) == NIL) tmswap(TRUE);
  t->file = fd;         t->txstat |= force_txstat;
  if (fd->ft == QftDIR) t->txstat |= TS_DIRLST;
  else /*just-in-case*/ t->txredit =  TXED_YES; return t;
}
/*---------------------------------------------------------------------------*/
#ifdef UNIX
# include <errno.h>
# define TwmBACKUP_EXT "~"
#else
# define TwmBACKUP_EXT ".bkup"
#endif
bool teferr (txt *t)
{
#ifdef UNIX
  QString msg = QString("[%1] %2 in %3").arg(errno).arg(strerror(errno))
#else
  QString msg = QString("Error in %1")
#endif
                                               .arg(t->file->full_name);
  vipError(msg); return false;
}
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
    line.sprintf("%c%c%c%c%c%c%c%c%c%c%12d %s %ls",
         it->isDir()              ? 'd' : '-',
         attr & QFile::ReadOwner  ? 'r' : '-',
         attr & QFile::WriteOwner ? 'w' : '-',
         attr & QFile::ExeOwner   ? 'x' : '-',
         attr & QFile::ReadGroup  ? 'r' : '-',
         attr & QFile::WriteGroup ? 'w' : '-',
         attr & QFile::ExeGroup   ? 'x' : '-',
         attr & QFile::ReadOther  ? 'r' : '-',
         attr & QFile::WriteOther ? 'w' : '-',
         attr & QFile::ExeOther   ? 'x' : '-', 
         int(it->size()), QfsModDateText(*it).cStr(), it->fileName().utf16());

    Lleng = qstr2tcs(line, Lebuf); TxTIL(t, Lebuf, Lleng); 
                                   TxDown(t);
  } t->cx = t->txlm = DIRLST_FNPOS;
}
bool tmDoLoad (txt *t)  /*- - - - - - - - - - - - - - - - - - - - - - - - - -*/
{
  switch (t->file->ft) {
  case QftPSEUDO:     return true; // PSEUDO file always considered "loaded"
  case QftDIR: tmDirLST(t); break;
  case QftTEXT:
    {
      int oc = QfsOpen(t->file, FO_READ);
      if (oc < 0) return false;
      if (oc > 0 && !DqLoad(t->txdstk, t->file, t->file->size)) oc = -2;
      QfsClose(t->file);
      if (oc < 0) return false; // zero result (empty file) is also acceptable
  } }
  bool editable = !(t->txstat & TS_RDONLY) && t->file->writable;
  t->txredit = editable ? TXED_YES : TXED_NO;
  t->txstat &= ~TS_CHANGED;     check_MCD(t);
  t->txstat |=  TS_FILE;
  if (t->txudeq != NIL) t->txstat |= TS_UNDO; return true;
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
void tmLoadIn (txt *t, QString  filename) /*- - - - - - - - - - - - - - - - -*/
{
  QfsClear(t->file); t->file = QfsNew (filename, NULL);
  tmLoad(t);         if (Twnd) vipUpdateWinTitle(Twnd);
}
/*---------------------------------------------------------------------------*/
BOOL tmsave (txt *t, BOOL needBackup)
{
  if (t->file == NULL || (t->txstat & TS_PSEUDO+TS_DIRLST) ||
                        !(t->txstat & TS_CHANGED)) return TRUE;
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
  t->txstat &= ~TS_CHANGED;               return true;
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
        // Found other text with matching name - check if this is a ghost (then
        // delete it), or some active/unsaved text (then abort the operation):
        //
        if (to->txstat & TS_CHANGED+TS_PERM+TS_WND){ vipBell(); return false; }
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
  if (t->txstat & TS_PSEUDO+TS_DIRLST) return true;
  else if (t->file->ft == QftTEXT)     return tmsave(t, needBackup);
  else if (t->txstat & TS_CHANGED) {
    QString new_name = vp->sctw->mf->saveAsDialog(t);
    if (new_name.isEmpty()) return false;
    else                    return tmSaveAs(t, new_name);
  } else                    return true;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmSaveAll (void)                    /* сохранить ВСЕ изменения на диске */
{
  for (txt *t = texts; t; t = t->txnext)
    if (t->txstat & TS_CHANGED) { if (t->txwndptr) 
                                       twSave(t, t->txwndptr, false);
                                  else tmsave(t,              FALSE); }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twSafeClose (txt *t, wnd *vp)
{
  if ((t->txstat & TS_CHANGED) && t->txwndptr->wnext == NULL) return false;
  wdetach(t, vp);
  Twnd = vp; twExit(); return true; /* NOTE: directory stack is NOT checked */
}
/*---------------------------------------------------------------------------*/
#define  FPROMPT AF_LIGHT "File:"
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
                     &filebufFlag, TM_F2ENTR, TM_F2ENTR, 0);
}
void tmDoFentr (void) 
{ 
  if (filebuflen > LFPROMPT) twDirPush(tcs2qstr(filebuf   +LFPROMPT,
                                                filebuflen-LFPROMPT)); 
}                                                                       
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmFnewByTtxt (void)                 /* войти в новый файл (имя из Ttxt) */
{
  if (!tleread()) return;
/*++
  TxSetY(Ttxt, Ty); if (qTxDown(Ttxt)) return;
  Lleng  = TxTRead(Ttxt, Lebuf);
*/
  int lm, rm;
       if (Block1size(&lm, &rm)); // sets lm/rm, nothing more to do
  else if (Ttxt->txstat & TS_MCD) { 
#define USE_MCD_MARGIN(LEFTorRIGHT) \
  if (Lleng > LEFTorRIGHT &&        \
  (char)Lebuf[LEFTorRIGHT] == LDS_MCD) { lm = LEFTorRIGHT+1; rm = Lleng; }
         USE_MCD_MARGIN(MCD_RIGHT) //
    else USE_MCD_MARGIN(MCD_LEFT)  // allow both double-delimiter (64+69th pos)
    else return;                   // and single-delimiter (64th posd only) fmt
/*++
                                    if (Lleng < (lm = MCD_RIGHT+1)) return;
                                    else         rm = Lleng;                
*/
  }
  else if (Ttxt->txstat & TS_DIRLST) { lm = DIRLST_FNPOS; rm = Lleng; }
  else if (Tx > Lleng) return;
  else {
    lm = rm = Tx;
    while (lm > 0     && !strchr(" \'\"<", (char)Lebuf[lm-1])) lm--;
    while (rm < Lleng && !strchr(" \'\">", (char)Lebuf[rm]  )) rm++;
    /*
     * TODO: find the include file using some search path
     */
  }
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
  case TM_F2ENTR: tmDoFentr();    break; /* ...войти в этот файл             */
#ifdef UNIX
  case TM_SHELL: x2enter(); break;       /* ввести и выполнить команду shell */
  case TM_FEXEC:
  case TM_F2EXEC:
    tmSaveAll(); /* save current changes - just in case */
    tmshell(kcode);
    break;
#endif
  case TM_INFILE: infilnam(); break; /* enter filename field in micros.dir   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  case TM_HFORK:                     /* горизонтальный fork и к нижнему окну */
  case TM_VFORK:                     /* create new window и к правому окну   */
    wind = vipSplitWindow(Twnd, KbCode);
    if (wind) {
      wattach  (Ttxt, wind); Ttxt->tcx = Twnd->wtx; wind->wcy = Ty;
      twDirCopy(Twnd, wind); Ttxt->tcy = Twnd->wty; wind->wcx = Tx;
      vipActivate(wind);
      vipUpdateWinTitle(wind);   wredraw(wind);        return E_OK;
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
#ifdef UNIX
  case TM_GREP:
  case TM_GREP2:   if (tmGrep(kcode) < 0) vipBell(); break;
  case TM_SYNCPOS: if (tmSyncPos()   < 0) vipBell(); break;
#endif
  default:
    return E_NOCOM;
  }
  return E_OK;
}
/*---------------------------------------------------------------------------*/

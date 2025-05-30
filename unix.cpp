/*------------------------------------------------------+----------------------
// МикроМир07       Shell commands and tmSyncPos        | (c) Epi MG, 2007-2025
//------------------------------------------------------+--------------------*/
#include <QString>        /* Old tm.c (c) Attic 1989-91, (c) EpiMG 1997-2003 */
#include <QRegExp>
#include <QProcess>
#if (QT_VERSION >= 0x040600)
# include <QProcessEnvironment>
# define GetENV(var,default) \
         QProcessEnvironment::systemEnvironment().value(var,default);
#else
# define GetENV(var,default) default
#endif
#include "mic.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
#include "clip.h"
#include "luas.h"
#include "synt.h"
#include "unix.h"
extern "C" {
#include "dq.h"
#include "le.h"
#include "te.h"
#include "tx.h"
#include "ud.h"
}
#ifdef Q_OS_WIN
# define MiDEFAULT_SHELL "C:/Windows/system32/cmd.exe"
#else
# define MiDEFAULT_SHELL "/bin/sh"
#endif
QString MiApp_shell;        /* set by mimReadPreferences(), see file mim.cpp */
/*---------------------------------------------------------------------------*/
static tchar tcmdbuffer[MAXLPAC];
static int   tcmdbuflen = 0,
          tcmdpromptlen = 0;
static int  tcmdbufFlag = 0;
static tchar *tcmdHistory[LE_HISTORY_SIZE] = { 0 };
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void x2enter (void)           /* command line enter (2) ввести команду shell */
{
  if (Twnd->wsibling && !(Twnd->wsibling->wtext->txstat & TS_PSEUDO)
                     && !(Twnd->wtext->txstat           & TS_PSEUDO))
    exc(E_SETY);
  QString prompt = QfsShortDirName(Ttxt->file);
  int new_prompt_len  = prompt.length()+2;
  if (new_prompt_len != tcmdpromptlen) {
    int len = my_max(tcmdpromptlen, new_prompt_len);
    blktmov(tcmdbuffer + tcmdpromptlen,
            tcmdbuffer + new_prompt_len, MAXLPAC-len);
    tcmdbuflen += new_prompt_len - tcmdpromptlen;
    tcmdpromptlen = new_prompt_len;
  }
  qstr2tcs(prompt,           tcmdbuffer,      AT_PROMPT);
  aftotc(AF_PROMPT "->", -1, tcmdbuffer+tcmdpromptlen-2);
  LenterARG(tcmdbuffer, &tcmdbuflen, tcmdpromptlen, tcmdHistory,
                        &tcmdbufFlag, TM_FEXEC, TM_F2EXEC, 0);
}
/*---------------------------------------------------------------------------*/
static void appendText(txt *Stxt, int& Sx, char *text, char *tend)
{
//  if (qTxBottom(Ttxt)) { teIL(); Lleng = 0; } // insert new empty line here,
//  else       Lleng = TxTRead(Ttxt, Lebuf);    // or get partially filled one
//  Lleng += aftotc(text, tend-text, Lebuf+Lleng);
//  if (Lleng > Twnd->wsw) {
//    int width = Twnd->wsw-1;
//    tchar tmp = Lebuf[width];     Lebuf[width] = TeSCH_CONTINUE;
//    TxTRep(Ttxt, Lebuf, width+1); Lebuf[width] = tmp;
//    TxDown(Ttxt);
//    blktmov(Lebuf+width, Lebuf, Lleng -= width); teIL();
//  }
//  TxTRep(Ttxt, Lebuf, Tx = Lleng);
//
  int             len = my_min(tend-text, MAXLUP);
  if (qTxBottom(Stxt)) TxIL(Stxt, text, Sx = len);
  else {
    int olen =         TxRead(Stxt, afbuf);
    if (olen+len > MAXLUP) len = MAXLUP-Sx;
    if (len > 0) {
      lblkmov(text, afbuf + olen,   len);
      TxRep  (Stxt, afbuf,  olen += len);
    }                          Sx = olen;
  }
  if (Sx > Stxt->txrm) Sx = Stxt->txrm;
}
static void appendNL(txt *Stxt, int& Sx, long& Sy)
{
  TxDown(Stxt); Sx = Stxt->txlm; Sy++;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void shellexec (txt *Stxt, int  &Sx,
                                  long &Sy, const char *cmd, wnd *Swnd = NULL)
{
  QString shell = MiApp_shell;
  if (shell.isEmpty()) shell = GetENV("SHELL", MiDEFAULT_SHELL);
  if (shell.isEmpty()) { vipError("no shell");           return; }
  QObject *parent = NULL;
  QProcess *proc = new QProcess(parent);
  QStringList args; args << "-c" << cmd;
  QfsChDir(Stxt->file); proc->start(shell,args);
  // - - - - - - - - - - - - - - - - - - - - - -
  char *p, *pst;    int len, k, break_count = 0;
//                              ^
// terminate/close is *supposed* to kill the process, but that's not for sure;
// also may need to read the rest of data from buffer after process terminated
//
  short oldTXED = Stxt->txredit;           // mark text as "read only"..
                  Stxt->txredit = TXED_NO; // mostly to force red cursor
  EnterOSmode();
  int gotCR = 0;
  do {
    if (proc->state() == QProcess::NotRunning) break_count++;
    QByteArray                       bytes = proc->readAllStandardError();
    if ((len = bytes.length()) <= 0) bytes = proc->readAllStandardOutput();
    if ((len = bytes.length())  > 0) {                    TxSetY(Stxt, Sy);
      for (p = pst = bytes.data(); len--; p++) {
        switch (*p) {
        case '\r': gotCR = 1; break;
        case '\n': gotCR = 0;
          appendText(Stxt,Sx,     pst,   p); // add text before NL + NL itself,
          appendNL  (Stxt,Sx,Sy); pst = p+1; // move pst to the char after '\n'
          break;
        default:                                           // gotCR => add text
          if (gotCR) {                                     // with CR included,
            if (p != pst) appendText(Stxt,Sx,     pst, p); // if possible, move
                          appendNL  (Stxt,Sx,Sy); pst = p; // pst to a new char
            gotCR = 0;
        } }
      } if (p != pst) appendText(Stxt,Sx, pst, p);
    }
    while ((k = kbhin()) != 0) { // Process all user input from KBH queue
      char ascii = k;            //  (but do not wait for anything here)
      switch (k) {
      case    3: proc->terminate(); break_count++; break; /* ^C */
      case    4: proc->close();     break_count++; break; /* ^D */
      default:
        if (k == '\n') appendNL(Stxt,Sx,Sy);
        else {
          p = &ascii; appendText(Stxt,Sx, p, p+1);
        }
        proc->write(&ascii, 1);
    } }
    if (Swnd) vipOnFocus (Swnd); vipYield(); // <- QCoreApp::processEvents();
    if (Swnd) vipFocusOff(Swnd); TxSetY(Stxt, Sy);
  }
  while (break_count < 2);     if (!proc->waitForFinished(300)) proc->kill();
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ExitOSmode(); Stxt->txstat |= TS_CHANGED;
                Stxt->txredit =    oldTXED;
}
/*---------------------------------------------------------------------------*/
static QString tcmd2qstr (tchar *p, tchar *pend)
{
  char cmdbuffer[MAXLPAC], *pc = cmdbuffer; tchar tp;
  while (p < pend) {
    switch (tp = *p++) {
    case TmSCH_THIS_OBJ:
    case LeSCH_REPL_BEG: pc = scpy(Ttxt->file->name.cStr(), pc); break;
    case LeSCH_REPL_END: pc = scpy(Ttxt->file->path.cStr(), pc); break;
    default:
      *pc++ = (char)tp;
  } } *pc = 0;
  return QString(cmdbuffer);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int shelltext (int kcode) /* select new Ttxt (and Twnd) for shell cmd */
{                                /* returns 0 on success, non-zero for error */
  txt *Ctxt;
  wnd *wind;
  if (Ttxt->txstat & TS_PSEUDO) { TxEmpt(Ctxt = Ttxt); wind = Twnd; }
  else
  if (Twnd->wsibling && (Twnd->wsibling->wtext->txstat & TS_PSEUDO)) {
    wind = Twnd->wsibling;
    Ctxt = Twnd->wsibling->wtext; TxEmpt(Ctxt);
  }
  else {
    Ctxt = tmDesc(QfsELLIPSIS, false, Ttxt);
    wind = vipSplitWindow(Twnd, kcode);
    if (wind) wattach(Ctxt, wind);
    else    { vipBell(); return 1; }   // the content is re-created => register
  }                                    //                  as new text for Lua
  if (wind != Twnd) vipActivate(wind); luasNtxt(Ctxt);
              vipUpdateWinTitle(wind); vipFocus(wind);
  return 0;
}
/*---------------------------------------------------------------------------*/
void tmshell (int kcode)
{
  QString cmd = tcmd2qstr(tcmdbuffer + tcmdpromptlen, tcmdbuffer + tcmdbuflen);
  if (shelltext(kcode == TM_F2EXEC ? TM_VFORK : TM_HFORK))              return;
//
// Now insert the command (along with the prompt = current directory) into the
// text and execute UNIX shell command... then check if any files were changed
//
  TxTIL(Ttxt, tcmdbuffer, tcmdbuflen); Ty++;
  TxDown(Ttxt);
  shellexec(Ttxt, Tx, Ty, cmd.cStr(), Twnd); tmCheckFiles();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - grep - - */
int tmGrep (int kcode)
{
  if (leOptMode & LeARG_WILDCARD) return -1; // <- neither POSIX nor GNU grep
  tesParse();       if (spl <= 0) return -1; //     support wildcard searches
//
  QString grep_cmd = QString("grep -%1%2nH '%3' %4")
    .arg((leOptMode & LeARG_REGEXP)     ? "E" : "F")
    .arg((leOptMode & LeARG_IGNORECASE) ? "i" :  "")
    .arg(tcs2qstr(spa, spl))
    .arg(soptfilen ? tcmd2qstr(soptfiles, soptfiles+soptfilen) : QString("*"));
//
  if (shelltext(kcode == TM_GREP ? TM_HFORK : TM_VFORK)) return -1;
  TxIL(Ttxt, (char*)grep_cmd.cStr(), grep_cmd.length());      Ty++;
  TxDown(Ttxt);
  shellexec(Ttxt, Tx, Ty, grep_cmd.cStr(), Twnd); return 0;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void append_disrstk_item (qfile *file, long ypos)
{
  QString fn = QString("%1:%2:").arg(file->full_name).arg(ypos);
  TxIL(Ttxt,(char*)fn.cStr(),fn.length());  Ty++;  TxDown(Ttxt);
}
int tmShowDirst (void) // show dirstack = history of files entered by Esc,Down
{                      //  (or equivalent) commands in the current text/window
  txt *Ctxt = Ttxt;    //
  wnd *Cwnd = Twnd;                         if (shelltext(TM_HFORK)) return -1;
  dirstk   *pds   = Cwnd->dirsp; append_disrstk_item(Ctxt->file, Ctxt->vp_cty);
  while ((--pds) >= Cwnd->stack) append_disrstk_item( pds->file,     pds->dsy);
  return 0;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmLoadXeq (txt *t) /* load text by executing command from t->file->name */
{
  t->txstat |= TS_PSEUDO|TS_FILE; // txt might be created with UDNO => kill it
  TxEmpt(t);                      //
  if (t->txudeq) { DqDel(t->txudeq); t->txudeq  =     NULL;
                                     t->txstat &= ~TS_UNDO; }
  int  Sx = 0;
  long Sy = 0; QString cmd =  t->file->name; // <- copy the value first, since
               cmd.remove(0,1).remove(-1,1); //  remove() changes the argument
  shellexec(t, Sx, Sy, cmd.cStr());
}
/*---------------------------------------------------------------------------*/
void tmDirLST (txt *t)             /* load directory listing into given text */
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
/*---------------------------------------------------------------------------*/
static int scan_lines_up (char c1st, char *line_buffer)       /* SyncPos(tm) */
{
  int len, steps = 0;
  do {                        if (qTxTop(Ttxt)) return -1;
    TxUp(Ttxt);
    if ((len = TxRead(Ttxt, line_buffer)) <= 0) return -1;
    steps++;
    line_buffer[len] = 0;                         // scans line up from current
  }                                               // position (copying them to
  while (line_buffer[0] == c1st); return steps-1; // the supplied buffer) while
}                                                 // first char matches c1st
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int tmSyncPos (int marks)  /* SyncPos(tm) -- only works with ASCII filenames */
{
  char c,  line_buffer[MAXLPAC]; long file_pos = 0, line_pos = 0, foo;
  wnd *sibling = Twnd->wsibling;
  wnd *other_wnd = NULL;
  short len,  steps = 0;                       // gcc "unix.cpp:277: error:..."
  QString filename;                            // grep "unix.cpp:273:int tm..."
  QRegExp gnuFileLine("([+./0-9A-z-]+):(\\d+):.*");
  QRegExp unifiedDiff("@@ [^@]+\\+(\\d+),[^@]+ @@.*");
  QRegExp lineKeyword("\"?([^ \",]+)\"?, line (\\d+)(?:.(\\d+))?");
//        ^
// [some-prefix] ["]{file}["], line {line}[.{pos}][:] {some-text}
//
//   "nm.h", line 155: Error: large is not defined (Sun WorkShop Compilers)
//   cc: "nm.h", line 155: error 1000: Unexpected symbol: "large" (HP ANSI C)
//   Error 419: "nm.h", line 155 # 'large' is used as a type... (HP ANSI C++)
//   "nm.h", line 155.51: 1506-046 (S) Syntax error (VisualAge C++ for AIX)
//   cc: Error: nm.h, line 155: Ill-formed parameter type list.. (Digital C++)
//
        TxSetY(Ttxt,          Ty); if (qTxBottom(Ttxt)) return -1;
  len = TxRead(Ttxt, line_buffer); if (len <= 0)        return -1;
  line_buffer[len] = 0;
//
// Mercurial/git log handling - actually, we only care whether each line starts
// with (probably, abbreviated) revision SHA-1 (both «git log --pretty=oneline»
// and µMup-specific 'gitlog' qualify, as well as «git blame» output), or short
// rev number (Hg only) - the value just must be suitable for 'name:git|hg:rev'
// special name understood by tmDesc (filename of the original file taken from
// the last word of Ttxt name, VCS name obtained from the 1st word of the name)
//
  if (Ttxt->txstat & TS_GITLOG) {
    filename =  Ttxt->file->name.section(' ', -1).remove(-1,1) + ":" +
                Ttxt->file->name.section(' ',0,0).remove( 0,1) + ":" +
            QString(line_buffer).section(' ',0,0);
    //
    // Cannot specify absolute path in the 'git show rev:file' command (because
    // of "Path 'file' exists on disk, but not in 'rev'" message) - so changing
    // current working directory to one stored in the file->path of Ttxt:
    //
    if (!Ttxt->file->path.isEmpty()) QfsChDir(Ttxt->file);
  }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {line-other}a{line1}[,{line2}]         {line-other1}[,{line-other2}]d{line}
// > ...                                  < ...
// > {some-text}                          < {some-text}
//
// {line-other1}[,{line-other2}]c{line1}[,{line2}]
// < ...
// ---                                            « ʁTraditional diff formatʀ »
// > ...
// > {some-text}                  (parsed using traditional methods, of course)
//
  else if ((c = line_buffer[0]) == '<' || c == '>') {
    if (sibling == NULL) return -1;                      // only works if file
    else {                                               // is already opened
      int offset = scan_lines_up(c, line_buffer);        // in sibling window
      if (offset < 0) return -1;                         // (nowhere to obtain
      if (memcmp(line_buffer, "---", 3) == 0             // the name otherwise)
          && (scan_lines_up('-', line_buffer) < 0 ||
              scan_lines_up('<', line_buffer) < 0)) return -1;

      char          *p = dtol(line_buffer, &foo);
      if (*p == ',') p = dtol(p+1,         &foo);
                     p = dtol(p+1,    &file_pos); file_pos += offset;
    }
    goto diff_sync_to_sibling_wnd; // obviously, "traditional methods" are
  }                                // not authentic without goto statement
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @@ -{line-other},{k} +{line},{n} @@ [comment]      « ʁUnified diff formatʀ »
//  {context}
//  ...                 Like for traditional diff, the file being compared must
// -{deleted-line}      be opened already in "other" windows & that file should
// +{added-line}     <- correspond to "newer" version, and tmSyncPos is invoked
//  ...                 with cursor on hunk header or one of the added lines
//
  else if (line_buffer[0] == '@' || line_buffer[0] == '+') {
    if (sibling == NULL) return -1;
    else {
      bool found = (line_buffer[0] == '@');
      while (!found && !qTxTop(Ttxt)) {         TxUp(Ttxt);
        line_buffer[ len = TxRead(Ttxt, line_buffer) ] = 0;
        switch (line_buffer[0]) {
          case '-':            continue;
          case '@': found = true; break;
          default:       steps++; break;
    } } }
    if (unifiedDiff.exactMatch(line_buffer)) {
      file_pos = unifiedDiff.cap(1).toLong() + steps;
diff_sync_to_sibling_wnd:
      vipActivate(sibling);
      vipFocus   (sibling); tesetxy(Tx, file_pos-1); return  0;
    }                                                return -1;
  }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  else if (gnuFileLine.exactMatch(line_buffer)) {
    filename = gnuFileLine.cap(1);
    file_pos = gnuFileLine.cap(2).toLong();
  }
  else if (lineKeyword.indexIn(line_buffer) >= 0) {
    filename = lineKeyword.cap(1);
    file_pos = lineKeyword.cap(2).toLong();
    line_pos = lineKeyword.cap(3).toLong();
  }
  else return -1; /* unknown format */
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Filename found, open that file in the other window, if not opened already in
// the sibling window (upper pane). Note: only filename is checked for sibling,
//                                      but for others we do check path as well
  txt  *prev_txt = Ttxt;
  if (sibling && filename == sibling->wtext->file->name) other_wnd = sibling;
  else {
    for (other_wnd = windows; other_wnd; other_wnd = other_wnd->wdnext) {
      txt *t =                                       other_wnd->wtext;
      if (t && t->file &&
        ((t->file->name == filename && t->file->path == Ttxt->file->path) ||
                                       t->file->full_name == filename) ) break;
  } }
  if (other_wnd) { vipActivate(other_wnd);   // if "other window" already exist
                   vipFocus   (other_wnd); } // then just activate, else create
  else {                                     // a new one, forked from sibling,
    if (sibling) { vipActivate(sibling);     // to preserve original dir stack
                   vipFocus   (sibling);
      if ((other_wnd = twFork(TM_VFORK)) == NULL) return -1;     twDirPush();
    }
    else if ((other_wnd = vipSplitWindow(Twnd, TM_VFORK)) == NULL) return -1;
    if (!twEdit(other_wnd, filename, NULL, true))                  return -1;
  }
  if (marks) luas2txtProc("MkSyncMarks", prev_txt, Ttxt);
  tesetxy(line_pos, file_pos-1);                return 0;
}
/*---------------------------------------------------------------------------*/

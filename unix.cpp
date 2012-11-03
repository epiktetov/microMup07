/*------------------------------------------------------+----------------------
// МикроМир07     UNIX-specific stuff and tmSyncPos     | (c) Epi MG, 2007-2012
//------------------------------------------------------+--------------------*/
#include <QString>        /* Old tm.c (c) Attic 1989-91, (c) EpiMG 1997-2003 */
#include <QRegExp>
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
#include "synt.h"
#include "unix.h"
extern "C" {
#include "dq.h"
#include "le.h"
#include "te.h"
#include "tx.h"
}
/*-----------------------**--------------------------------------------------*/
# include <stdio.h>      /*                                                  */
# include <errno.h>      /*               Execute Shell Command              */
# include <fcntl.h>      /*                                                  */
# include <signal.h>     /*--------------------------------------------------*/
# include <stdlib.h>
# include <unistd.h>     
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/wait.h>
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
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
  if (qTxBottom(Stxt)) TxIL(Stxt, text, Sx = tend-text);
  else {
    Lleng = TxTRead(Stxt, Lebuf);
    Lleng += aftotc(text, tend-text, Lebuf+Lleng);
    TxTRep(Stxt, Lebuf, Sx = Lleng);
} }
static void appendCR(txt *Stxt, int& Sx, long& Sy)
{
  TxDown(Stxt); Sx = Stxt->txlm; Sy++;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void shellexec (txt *Stxt, int  &Sx,
                                  long &Sy, const char *cmd, wnd *Swnd = NULL)
{
  int fdsin[2], fdsout[2];
  int pid;
#define close_both(_fdx) close(_fdx[0]); close(_fdx[1]);

  if (pipe(fdsin)    < 0)                                          return;
  if (pipe(fdsout)   < 0) { close_both(fdsin);                     return; }
  if ((pid = fork()) < 0) { close_both(fdsin); close_both(fdsout); return; }
  else if (pid > 0) {
    char line_buffer[MAXLPAC], *p, *pst;
    int child_status, fds_in = fdsin[1],
                      fds_out = fdsout[0], flags, break_count = 0;
    close(fdsin[0]);
    close(fdsout[1]);
    flags = fcntl(fds_out, F_GETFL, 0); flags |= O_NONBLOCK;
            fcntl(fds_out, F_SETFL, flags);

    short oldTXED = Stxt->txredit;           // mark text as "read only"..
                    Stxt->txredit = TXED_NO; // mostly to force red cursor
    EnterOSmode();
    do {
      struct timeval timeout; int k;
      int selres;
      fd_set read_mask; FD_ZERO(&read_mask);         timeout.tv_sec  = 0;
                        FD_SET(fds_out, &read_mask); timeout.tv_usec = 10000;
// Wait for child:
//
      selres = select(fds_out+1, &read_mask, NULL, NULL, &timeout);
      if (selres < 0 && errno != EINTR) break;
      if (selres > 0) {
        int max_len = MAXLPAC-Tx-1;
        int len = read(fds_out, pst = line_buffer, max_len);
        if (len <= 0) break;               TxSetY(Stxt, Sy);
        for (p = pst; len--; p++) {
          if (*p == '\r' || *p == '\n') { appendText(Stxt,Sx,     pst,   p);
                                          appendCR  (Stxt,Sx,Sy); pst = p+1; }
        }
        if (p != pst) appendText(Stxt,Sx, pst, p);
      }
      while ((k = kbhin()) != 0) { // Process all user input from KBH queue
        unsigned char ascii = k;   //  (but do not wait for anything here)
        switch (k) {
        case    3: kill(pid, SIGINT);          break_count++; break; /* ^C */
        case    4: close(fds_in); fds_in = -1; break_count++; break; /* ^D */
        default:                               // ^
          if (fds_in < 0)  continue;           // kill/close is *supposed* to
          if (k == '\n') appendCR(Stxt,Sx,Sy); // break the loop, but may not
          else {
            p = (char*)&ascii; appendText(Stxt,Sx, p, p+1);
          }
          write(fds_in, &ascii, 1);
      } }
      if (Swnd) vipOnFocus (Swnd); vipYield(); // <- QCoreApp::processEvents();
      if (Swnd) vipFocusOff(Swnd); TxSetY(Stxt, Sy);
    }
    while (break_count < 2);     if (fds_in != -1) close(fds_in);
                                                  close(fds_out);
    if (!waitpid(pid, &child_status, WNOHANG)) {
      kill(pid, SIGKILL);  //
      wait(&child_status); // if child does not want to die, try harder...
    }
    ExitOSmode(); Stxt->txstat |= TS_CHANGED;
                  Stxt->txredit =    oldTXED;
  }
  else { /* ------------------------ child process ------------------------- */

    QfsChDir(Stxt->file);  close(fdsin[1]);   dup2(fdsin[0],  0);  /* stdin  */
                                              dup2(fdsout[1], 1);  /* stdout */
                           close(fdsout[0]);  dup2(fdsout[1], 2);  /* stderr */

    QString shell = GetENV("SHELL", "/bin/sh");
    execl(shell.cStr(),
          shell.cStr(), "-c", cmd, NULL); exit(1);
} }
/*---------------------------------------------------------------------------*/
void tmshell (int kcode)
{
  tchar tp, *p = tcmdbuffer + tcmdpromptlen, *pend = tcmdbuffer + tcmdbuflen;
  char cmdbuffer[MAXLPAC],                     *pc = cmdbuffer;
  txt *Ctxt;
  wnd *wind;
  if (p == pend) return; // empty command line => should not do anything
  while (p < pend) {
    switch (tp = *p++) {
    case TmSCH_THIS_OBJ:
    case LeSCH_REPL_BEG: pc = scpy(Ttxt->file->name.cStr(), pc); break;
    case LeSCH_REPL_END: pc = scpy(Ttxt->file->path.cStr(), pc); break;
    default:
      *pc++ = (char)tp;
  } } *pc = 0;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if (Ttxt->txstat & TS_PSEUDO) { TxEmpt(Ctxt = Ttxt); wind = Twnd; }
  else
  if (Twnd->wsibling && (Twnd->wsibling->wtext->txstat & TS_PSEUDO)) {
    wind = Twnd->wsibling;
    Ctxt = Twnd->wsibling->wtext; TxEmpt(Ctxt);
  }
  else {
    Ctxt = tmDesc(QfsELLIPSIS, false, Ttxt);
    wind = vipSplitWindow(Twnd, kcode == TM_F2EXEC ? TM_VFORK : TM_HFORK);
    if (wind) wattach(Ctxt, wind);
    else    { vipBell();   return; }
  }
  if (wind != Twnd) vipActivate(wind);
              vipUpdateWinTitle(wind); vipFocus(wind);
//
// Removed Sun Oct 21 12:12:00 2012 - all positions are cleared by TxEmpt or
//                                                     when creating new text
//   wind->ctx = wind->wtx = 0;
//   wind->cty = wind->wty = 0;
//   if (Twnd == wind) Tx = Ty = 0;
//   else        vipActivate(wind); vipFocus(wind); vipUpdateWinTitle(wind);
//--
// Now insert the command (along with the prompt == current directory) into the
// text and execute UNIX shell command... then check if any files were changed
//
  TxTIL (Ttxt, tcmdbuffer, tcmdbuflen); Ty++;
  TxDown(Ttxt);
  shellexec(Ttxt, Tx, Ty, cmdbuffer, Twnd); tmCheckFiles();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int tmGrep (int kcode)                                               /* grep */
{
  QString grep_command = "grep ";                     tesParse();
       if (leOptMode & LeARG_STANDARD) grep_command.append("-F");
  else if (leOptMode & LeARG_REGEXP)   grep_command.append("-E");
  else return -1;
  if (leOptMode & LeARG_IGNORECASE) grep_command.append("i");
                                    grep_command.append("n '");
  if (spl <= 0) return -1;
  tchar *tcmd = tcmdbuffer + tcmdpromptlen;
  tcmd += qstr2tcs(grep_command, tcmd);
  tcmd  = blktmov (spa,  tcmd, spl);
  tcmd += qstr2tcs("' ", tcmd);
  if (soptfilen) tcmd  = blktmov(soptfiles, tcmd, soptfilen);
  else           tcmd += qstr2tcs("*",      tcmd);

  tcmdbuflen = tcmd - tcmdbuffer;
  tmshell(kcode == TM_GREP ? TM_FEXEC : TM_F2EXEC); return 0;
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
int tmSyncPos (void) /* SyncPos(tm) -- NOTE: only works with ASCII filenames */
{
  char c, line_buffer[MAXLPAC]; long file_pos = 0, line_pos = 0, foo;
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
    if ((other_wnd = Twnd->wsibling) == NULL) return -1; // only works if file
    else {                                               // is already opened
      int offset = scan_lines_up(c, line_buffer);        // in "other" window
      if (offset < 0) return -1;                         // (nowhere to obtain
      if (memcmp(line_buffer, "---", 3) == 0             // the name otherwise)
          && (scan_lines_up('-', line_buffer) < 0 ||
              scan_lines_up('<', line_buffer) < 0)) return -1;

      char          *p = dtol(line_buffer, &foo);
      if (*p == ',') p = dtol(p+1,         &foo);
                     p = dtol(p+1,    &file_pos); file_pos += offset;
    }
    goto diff_sync_to_other_wnd; // obviously, "traditional methods" are
  }                              // not authentic without goto statement
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @@ -{line-other},{k} +{line},{n} @@ [comment]      « ʁUnified diff formatʀ »
//  {context}
//  ...                 Like for traditional diff, the file being compared must
// -{deleted-line}      be opened already in "other" windows & that file should
// +{added-line}     <- correspond to "newer" version, and tmSyncPos is invoked
//  ...                 with cursor on hunk header or one of the added lines
//
  else if (line_buffer[0] == '@' || line_buffer[0] == '+') {
    if ((other_wnd = Twnd->wsibling) == NULL) return -1;
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
diff_sync_to_other_wnd:
      vipActivate(other_wnd);
      vipFocus   (other_wnd); tesetxy(Tx, file_pos-1); return  0;
    }                                                  return -1;
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
// Filename found, open that file in the other window (if not opened already)
//
  if ((other_wnd = Twnd->wsibling) != NULL) {
    vipActivate(other_wnd);
    vipFocus   (other_wnd);
    if (filename != other_wnd->wtext->file->name) twDirPush(filename, Ttxt);
  }
  else if ((other_wnd = vipSplitWindow(Twnd, TM_VFORK)) != NULL) {
    vipFocusOff(Twnd);
    if (!twEdit(other_wnd, filename, NULL, true)) return -1;
  }
  tesetxy(line_pos, file_pos-1); return 0;
}
/*---------------------------------------------------------------------------*/

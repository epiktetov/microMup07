/*------------------------------------------------------+----------------------
// МикроМир07     UNIX-specific stuff and tmSyncPos     | (c) Epi MG, 2007-2011
//------------------------------------------------------+--------------------*/
#include <QString>        /* Old tm.c (c) Attic 1989-91, (c) EpiMG 1997-2003 */
#include "mic.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
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
  qstr2tcs(prompt,          tcmdbuffer,      AT_PROMPT);
  aftotc(AF_LIGHT "->", -1, tcmdbuffer+tcmdpromptlen-2);
/*
 * If invoked from command window, copy the current line (excluding old prompt)
 * as initial value for new command:
 */
  if ((Ttxt->txstat & TS_PSEUDO) && tleread()) {
    tchar      *p, *pend;
    for (p = Lebuf, pend = p+Lleng; p < pend && (*p & AT_PROMPT); p++) ;
    if (p < pend) {
      blktmov(p, tcmdbuffer + tcmdpromptlen, pend-p);
      tcmdbuflen = tcmdpromptlen + (pend-p);
  } }
  LenterARG(tcmdbuffer, &tcmdbuflen, tcmdpromptlen, tcmdHistory,
                        &tcmdbufFlag, TM_FEXEC, TM_F2EXEC, 0);
}
/*---------------------------------------------------------------------------*/
txt *Stxt;  /* text and window for shellexec, the latter may be NULL if that */
wnd *Swnd;  /* windows is not opened yet, e.g. when called from tmLoadXeq()  */

static void appendText (char *text, char *tend)
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
  if (qTxBottom(Ttxt)) TxIL(Ttxt, text, Tx = tend-text);
  else {
    Lleng = TxTRead(Ttxt, Lebuf);
    Lleng += aftotc(text, tend-text, Lebuf+Lleng);
    TxTRep(Ttxt, Lebuf, Tx = Lleng);
} }
static void appendCR() { TxDown(Ttxt); Tx = Ttxt->txlm; Ty++; }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void shellexec (const char *cmd)
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
        if (len <= 0) break;               TxSetY(Ttxt, Ty);
        for (p = pst; len--; p++)
          switch (*p) {
          case '\r':
          case '\n': appendText(pst, p); appendCR(); pst = p+1; break;
          case '\b': if (p > pst) p--;                          break;
          }
        if (p != pst) appendText(pst, p);
      }
      while ((k = kbhin()) != 0) { // Wait for user input
        unsigned char ascii = k;
        switch (k) {
        case    3: kill(pid, SIGINT);          break_count++; break; /* ^C */
        case    4: close(fds_in); fds_in = -1; break_count++; break; /* ^D */
        default:
          if (fds_in < 0)  continue;
          if (k == '\n') appendCR();
          else {
            p = (char*)&ascii; appendText(p, p+1);
          }
          write(fds_in, &ascii, 1);
      } }
      if (Swnd) vipOnFocus (Swnd); vipYield ();
      if (Swnd) vipFocusOff(Swnd); TxSetY(Ttxt, Ty);
    }
    while (break_count < 2);     if (fds_in != -1) close(fds_in);
                                                  close(fds_out);
    if (!waitpid(pid, &child_status, WNOHANG)) {
      kill(pid, SIGKILL);
      wait(&child_status); /* if child does not want to die, try harder */
  } }
  else { /* ------------------------ child process ------------------------- */

    QfsChDir(Ttxt->file);  close(fdsin[1]);   dup2(fdsin[0],  0);  /* stdin  */
                                              dup2(fdsout[1], 1);  /* stdout */
                           close(fdsout[0]);  dup2(fdsout[1], 2);  /* stderr */

    execl("/bin/sh", "sh", "-c", cmd, NULL);  exit(1);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void unixShell_in_Ttxt (const char *cmd)
{
  small oldTXED = Ttxt->txredit;
        Ttxt->txredit = TXED_NO; EnterOSmode();             shellexec(cmd);
        Ttxt->txredit = oldTXED;  ExitOSmode(); Ttxt->txstat |= TS_CHANGED;
}
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
  else { for (Ctxt = texts; Ctxt; Ctxt = Ctxt->txnext)
           if ((Ctxt->txstat & TS_PSEUDO) &&
              !(Ctxt->txstat & (TS_WND|TS_PERM)) ) break;
//       ^^
// Look for unattached PSEUDO text, clear existing one or create new (no UNDO),
// and also create new window for the text by splitting current one (and attach
// it to the text - vipBell is just for safety, we already checked possibility)
//
    if  (Ctxt)  TxEmpt(Ctxt);
    else Ctxt = TxNew(FALSE);
    wind = vipSplitWindow(Twnd, kcode == TM_F2EXEC ? TM_VFORK : TM_HFORK);
    if (wind) wattach(Ctxt, wind);
    else    { vipBell();   return; }
  }
  Ctxt->txstat |= TS_PSEUDO|TS_FILE; Ctxt->txstat &= ~(TS_CHANGED|TS_RDONLY);
  Ctxt->txredit = TXED_YES;
  qfile *new_file = QfsNew(QfsELLIPSIS, Ttxt->file); // Ctxt may be the same as
  QfsClear(Ctxt->file);       Ctxt->file = new_file; // Ttxt here (& that's Ok)
  wind->wcx = wind->wtx = 0;
  wind->wcy = wind->wty = 0; 
  if (Twnd == wind) Tx = Ty = 0;
  else        vipActivate(wind); vipUpdateWinTitle( Swnd = wind );
//
// Now insert the command (along with the prompt == current directory) into the
// text and execute UNIX shell command... then check if any files were changed
//
  TxTIL (Ttxt, tcmdbuffer, tcmdbuflen);   Ty++;
  TxDown(Ttxt);
  unixShell_in_Ttxt(cmdbuffer); tmCheckFiles();
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
  txt  *oldTtxt = Ttxt; Ttxt = t; //
  small oldTx   = Tx;   Tx   = 0; // instead of making shellexec() independent
  large oldTy   = Ty;   Ty   = 0; // of Ttxt just replace the text temporarily
  Swnd = NULL;
  QString cmd = t->file->name;
          cmd.remove(0,1).chop(1); unixShell_in_Ttxt(cmd.cStr());
  Tx = oldTx;
  Ty = oldTy; Ttxt = oldTtxt;
}
/*---------------------------------------------------------------------------*/
static int scan_lines_up (char c1st, char *line_buffer)       /* SyncPos(tm) */
{
  int len, steps = 0;
  do {                        if (qTxTop(Ttxt)) return -1;
    TxUp(Ttxt);
    if ((len = TxRead(Ttxt, line_buffer)) <= 0) return -1;
    steps++;
    line_buffer[len] = 0;
  }
  while (line_buffer[0] == c1st); return steps-1;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int tmSyncPos (void)
{
  char line_buffer[MAXLPAC], c, *p, *pline, filename[MAXPATH];
  small len;               large file_pos, line_pos =  0, foo;
  wnd *other_wnd = NULL;

        TxSetY(Ttxt,          Ty); if (qTxBottom(Ttxt)) return -1;
  len = TxRead(Ttxt, line_buffer); if (len <= 0)        return -1;
  line_buffer[len] = 0;
  filename[0]      = 0;
/*                                     ! NOTE: only works with ASCII filenames
 * Currently supported formats:        +---------------------------------------
 *
 * {line-other}a{line1}[,{line2}]    {line-other1}[,{line-other2}]d{line}
 * > ...                             < ...
 * > {some-text}                     < {some-text}
 *
 * {line-other1}[,{line-other2}]c{line1}[,{line2}]                -- diff
 * < ...
 * ---
 * > ...
 * > {some-text}
 *
 * {file}:{line}: {some-text}                                     -- GNU cc
 *
 * [some-prefix] ["]{file}["], line {line}[.{pos}][:] {some-text} -- others
 *
 *   "nm.h", line 155: Error: large is not defined (Sun WorkShop Compilers)
 *   cc: "nm.h", line 155: error 1000: Unexpected symbol: "large" (HP ANSI C)
 *   Error 419: "nm.h", line 155 # 'large' is used as a type... (HP ANSI C++)
 *   "nm.h", line 155.51: 1506-046 (S) Syntax error (VisualAge C++ for AIX)
 *   cc: Error: nm.h, line 155: Ill-formed parameter type list.. (Digital C++)
 */
  if ((c = line_buffer[0]) == '<' || c == '>') {
/*
 * diff works only if file is already opened in "other" window:
 */
    if ((other_wnd = Twnd->wsibling) == NULL) return -1;
    else {
      int offset = scan_lines_up(c, line_buffer);
      if (offset < 0) return -1;

      if (memcmp(line_buffer, "---", 3) == 0
          && (scan_lines_up('-', line_buffer) < 0 ||
              scan_lines_up('<', line_buffer) < 0)) return -1;

                     p = dtol(line_buffer, &foo);
      if (*p == ',') p = dtol(p+1,         &foo);
                     p = dtol(p+1,    &file_pos); file_pos += offset;
    }
    vipActivate(other_wnd);
    vipFocus   (other_wnd);
  } 
  else if ((pline = strstr(line_buffer+1, ", line ")) != NULL) {

         p =       dtol(pline+7, &file_pos);
    if (*p == '.') dtol(p+1,     &line_pos);

    if (pline[-1] == '\"') pline--;
    for (p = pline-1; 
         p > line_buffer && *p != ' ' && *p != '\"'; p--) ; p++;

    *sncpy(p, filename, pline-p) = 0; /* copy filename and terminate by 0 */
  }
  else if ((p = strchr(line_buffer, ':')) != NULL) {
    *p++ = 0;
    strcpy(filename, line_buffer); dtol(p, &file_pos);
  }
  else return -1; /* unknown format */
/*
 * If filename is present, open it in the other window (if not opened already)
 */
  if (*filename) {
    if ((other_wnd = Twnd->wsibling) != NULL) {
      vipActivate(other_wnd);
      vipFocus   (other_wnd);
      if (strcmp(filename, other_wnd->wtext->file->name.cStr()) != 0) 
        twDirPush(QString::fromAscii(filename), Ttxt);
    }
    else if ((other_wnd = vipSplitWindow(Twnd, TM_VFORK)) != NULL) {
       bool ok = twEdit(other_wnd, QString::fromAscii(filename), NULL, true);
       if (!ok) return -1;
  } }
  else if (other_wnd == NULL) return -1; /* should not come here */

  tesetxy(line_pos, file_pos-1); /* inside MIM line numbers start with 0 */
  return 0;
}
/*---------------------------------------------------------------------------*/

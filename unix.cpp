/*------------------------------------------------------+----------------------
// МикроМир07     UNIX-specific stuff and tmSyncPos     | (c) Epi MG, 2007-2011
//------------------------------------------------------+--------------------*/
#include <QString>        /* Old tm.c (c) Attic 1989-91, (c) EpiMG 1997-2003 */
#include "mic.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
#include "unix.h"
extern "C" {
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
static tchar tcmdbuffer[MAXLPAC]; txt *CmdTxt = NULL;
static int   tcmdbuflen = 0,
          tcmdpromptlen = 0;
static int  tcmdbufFlag = 0;
static tchar *tcmdHistory[LE_HISTORY_SIZE] = { 0 };
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void x2enter (void)           /* command line enter (2) ввести команду shell */
{
  if (Twnd->wsibling && Twnd->wsibling->wtext != CmdTxt
                     && Twnd->wtext           != CmdTxt) exc(E_SETY);
  QString prompt = QfsShortDirName(Ttxt->file);
  int new_prompt_len  = prompt.length()+2;
  if (new_prompt_len != tcmdpromptlen) {
    int len = my_max(tcmdpromptlen, new_prompt_len);
    blktmov(tcmdbuffer + tcmdpromptlen,
            tcmdbuffer + new_prompt_len, MAXLPAC-len);
    tcmdbuflen += new_prompt_len - tcmdpromptlen;
    tcmdpromptlen = new_prompt_len;
  }
  qstr2tcs(prompt,          tcmdbuffer,       AT_LIGHT);
  aftotc(AF_LIGHT "->", -1, tcmdbuffer+tcmdpromptlen-2);
/*
 * If invoked from command window, copy the current line (excluding old prompt)
 * as initial value for new command:
 */
/*++
  if ((Ttxt->txstat & TS_PSEUDO) && !qTxDown(Ttxt)) {
    tchar  *p, *pend;
    TxSetY(Ttxt, Ty); Lleng = TxTRead(Ttxt, Lebuf);
*/
  if ((Ttxt->txstat & TS_PSEUDO) && tleread()) {
    tchar      *p, *pend;
    for (p = Lebuf, pend = p+Lleng; p < pend && (*p & AT_LIGHT); p++);
    if (p < pend) {
      blktmov(p, tcmdbuffer + tcmdpromptlen, pend-p);
      tcmdbuflen = tcmdpromptlen + (pend-p);
  } }
  LenterARG(tcmdbuffer, &tcmdbuflen, tcmdpromptlen, tcmdHistory,
                        &tcmdbufFlag, TM_FEXEC, TM_F2EXEC, 0);
}
/*---------------------------------------------------------------------------*/
static void appendText (char *text, char *tend)
{
  if (qTxDown(Ttxt)) TxIL(Ttxt, text, Tx = tend-text);
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
      vipOnFocus (Twnd); vipYield ();
      vipFocusOff(Twnd); TxSetY(Ttxt, Ty);
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
void unixShell_in_Ttxt (const char *cmd)
{
  small oldTXED = Ttxt->txredit;
                  Ttxt->txredit = TXED_NO; EnterOSmode(); shellexec(cmd);
                  Ttxt->txredit = oldTXED;  ExitOSmode();
                  Ttxt->txstat |= TS_CHANGED;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tmshell (int kcode)
{
  tchar tp, *p = tcmdbuffer + tcmdpromptlen, *pend = tcmdbuffer + tcmdbuflen;
  char cmdbuffer[MAXLPAC],                     *pc = cmdbuffer;
  wnd *wind = NULL;
  if (p == pend) return; // empty command line => should not do anything
  while (p < pend) {
    switch (tp = *p++) {
    case LeSCH_REPL_BEG: pc = scpy(Ttxt->file->name.cStr(), pc); break;
    case LeSCH_REPL_END: pc = scpy(Ttxt->file->path.cStr(), pc); break;
    default:
      *pc++ = (char)tp;
  } } *pc = 0;
//+
//  fprintf(stderr, "tmshell(plen=%d,len=%d,cmd=%s)\n", tcmdpromptlen,
//                                                      tcmdbuflen, cmdbuffer);
//-
  if (CmdTxt) TxEmpt(CmdTxt); // After preparing command line, find text and
  else CmdTxt = TxNew(FALSE); // windows where the command will be executed:
  CmdTxt->txredit = TXED_YES;
  CmdTxt->txstat |= TS_PSEUDO|TS_FILE;
  if (Twnd->wsibling && Twnd->wsibling->wtext == CmdTxt) wind = Twnd->wsibling;
  else if                               (Ttxt == CmdTxt) wind = Twnd;
  else {
    wind = vipSplitWindow(Twnd, kcode == TM_F2EXEC ? TM_VFORK : TM_HFORK);
    if (wind) wattach(CmdTxt, wind);
    else { vipBell(); return; }
  }
  qfile *new_file = QfsNew(QfsELLIPSIS, Ttxt->file); // CmdTxt may be the
  QfsClear(CmdTxt->file);   CmdTxt->file = new_file; // same as Ttxt here
  wind->wcx = wind->wtx = 0;
  wind->wcy = wind->wty = 0; 
  if (Twnd == wind) Tx = Ty = 0;
  else        vipActivate(wind); vipUpdateWinTitle(wind);
//
// Now insert the command (along with the prompt == current directory) into the
// text and execute UNIX shell command... then check if any files were changed
//
  TxTIL (Ttxt, tcmdbuffer, tcmdbuflen);   Ty++;
  TxDown(Ttxt);
  unixShell_in_Ttxt(cmdbuffer); tmCheckFiles();
}
/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
static int scan_lines_up (char c1st, char *line_buffer)       /* SyncPos(tm) */
{
  int len, steps = 0;
  do {                         if (qTxUp(Ttxt)) return -1;
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

        TxSetY(Ttxt,          Ty); if (qTxDown(Ttxt)) return -1;
  len = TxRead(Ttxt, line_buffer); if (len <= 0)      return -1;
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
         p > line_buffer && *p != ' ' && *p != '\"'; p--); p++;

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
    else if ((other_wnd = vipSplitWindow(Twnd, TM_VFORK)) == NULL
             || ! twEdit(other_wnd, QString::fromAscii(filename))) return -1;
  }
  else if (other_wnd == NULL) return -1; /* should not come here */

  tesetxy(line_pos, file_pos-1); /* inside MIM line numbers start with 0 */
  return 0;
}

/*---------------------------------------------------------------------------*/

#ifdef UNIX_notdef

static char modfile_buf[MAXPATH+15];
static char modfile_rep[MAXPATH+15];

BOOL tm_modfile()    /* изменение имени и/или атрибутов файла */
{
   BOOL repeat;
   char *p, *p1;
   tchar *pt;
   struct stat statbuf, *d;
   small attr, new_attr, len, i, nmlen, mask, llen;

   /* получить имя файла из строки; */
   if ( ( p = fenter() ) == NIL ) return FALSE;
   /* получить каноническое имя файла; */
   if ( !FsParse( p, modfile_buf ) || Fsiswild ) return FALSE;
   /* получить атрибуты файла; */
   d = &statbuf;
   if ( Fsddlen == 1 ) { /* корневой каталог */
			if ( access( "/", 02 ) ) return FALSE;
   } else {
      p = modfile_buf + Fsddlen - 1;
      *p = 0;
      if ( access( modfile_buf, 02 ) ) return FALSE;
      *p = '/';
   }
   if ( stat( modfile_buf, d ) ) return FALSE;
   new_attr = (attr = d->st_mode);
   nmlen = Fsnmlen;

   repeat = TRUE;
   while ( repeat ) {
      repeat = FALSE;
      /* провести диалог с пользователем; */
      p = modfile_buf + Fsddlen + nmlen;
      for ( i=nmlen; i <= 14 ; i++ ) *p++ = ' ';
      *attrtoa( p, new_attr) = 0;
      for ( p=modfile_buf, pt=Lebuf; *p; ) *pt++ = ctotc( *p++ );
      len = (llen = p - modfile_buf);
      Lenter( NIL, &llen, Fsddlen, std_terms, FALSE );
      if ( KbCode == TK_BREAK ) return FALSE;

      p1 = modfile_rep + llen;
      for ( p=modfile_rep, pt=Lebuf; p < p1; ) {
         *p++ = (*pt++) & 0xFF;
      }
      while ( p < modfile_rep + len ) *p++ = ' ';
      *p = 0;

      if ( *(modfile_rep+Fsddlen) == ' ' ) { /*  имя файла пропало */
         /* удалить файл */
         *(modfile_buf + Fsddlen + nmlen) = 0;
         return FsDelete( modfile_buf );
      } else { /* переименование и/или изменение атрибутов */
         p = modfile_buf + Fsddlen;
         for ( i=13; p[i] == ' '; i-- );
         p[i+1] = 0;
         p1 = modfile_rep + Fsddlen;
         for ( i=13; p1[i] == ' '; i-- );
         p1[i+1] = 0;
         if ( !scmp( p, p1) ) {  /*  имя файла изменилось */
            /* переименовать файл; */
            if ( !FsRename( modfile_buf, modfile_rep ) ) return FALSE;
            *scpy( p1, p ) = 0;
            nmlen = slen( p );
         }
         /* расшифровка полученых атрибутов */
         p = modfile_rep + Fsddlen + 14 + 1;
         new_attr = 0;
         if ( *p++ != *(modfile_buf + Fsddlen + 14 + 1) ) repeat = TRUE;

         /* переустановка ид. пользователя  */
         if ( *p == 's' ) new_attr |= 04000;
         else if ( *p != '-' ) {
            repeat = TRUE;
            new_attr |= attr & 04000; 
         }
         p++;
         if ( *p == 's' && *(p+7) == 'x' ) {
            /* переустановка ид. группы */
            new_attr |= 02010;
         } else {
            if ( *p == 'l' && *(p+7) == '-' ) {
               /* учет блокировки доступа  */
               new_attr |= 02000; 
            } else {
               if ( *p != '-' ) {
                  repeat = TRUE;
                  new_attr |= attr & 02000;
                  *(p+7) = '?';
               }
            }
         }
         p++;
         /* бит навязчивости */
         if ( *p == 't' ) new_attr |= 01000;
         else if ( *p != '-' ) {
            repeat = TRUE;
            new_attr |= attr & 01000; 
         }
         p++;
	 for ( mask = 0400, p1 = "rwxrwxrwx"; mask; mask >>= 1, p++, p1++ ) {
            if ( *p == *p1 ) new_attr |= mask;
            else if ( *p != '-' ) {
               repeat = TRUE;
               new_attr |= attr & mask;
            }
         }
         if ( !repeat && ( new_attr != (attr & 07777) ) ) {
            /* ошибок нет - правим */
            return ( chmod( modfile_buf, (int)new_attr ) == 0 );
         }
      }  /* конец работы с атрибутами */
      if (repeat) vipBell();
   }
   return TRUE;
}

#endif
/*---------------------------------------------------------------------------*/

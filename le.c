/*------------------------------------------------------+----------------------
// МикроМир07    le = Line Editor -- Редактор строки    | (c) Epi MG, 2006-2025
//------------------------------------------------------+--------------------*/
#include "mic.h"             /* Old le.c (c) Attic 1989, (c) EpiMG 1996-2003 */
#include "ccd.h"
#include "twm.h"
#include "vip.h"
#include "clip.h"
#include "synt.h"
#include "le.h"
#include "te.h" // teIL(), tesmark();
#include "tx.h"
#include "ud.h"
#include <stdlib.h>       /* calloc() used in tpstrdup()                     */
#include <string.h>       /* strchr() used in leccase(), plus strlen()       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void LeStart() { blktspac(Lebuf, MAXLPAC); }

tchar Lebuf[MAXLPAC];     /* Буфер строки                                    */
tchar lfbuf[MAXLPAC];     /* альтернативный буфер                            */
int   Lleng;              /* Длина строки без хвостовых пробелов             */
int   Lxlm, Lxrm;         /* левая/правая граница для перемещения            */
int   Lxle, Lxre;         /* левая/правая граница для редактирования         */
long  Ly;                 /* Y строки в тексте (для окна)                    */
int   Lx;                 /* X курсора в строке (а не в окне!)               */
bool  LeInsMode;          /* Режим вставки (если 0, то режим замены)         */
bool  Lclang;             /* Язык редактируемого текста (из Ttxt->clang)     */
bool  Lredit;             /* Можно менять строку                             */
bool  Lchange;            /* Строка изменялась                               */
txt  *Ltxt;               /* Текст, в который сливается откатка              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd  *Lwnd = NIL;         /* Окно, в котором редактируется строка            */
tchar Lattr = 0;          /* Текущие атрибуты ввода (used in LenterARG now)  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int leARGmode = 0;        /* Argument Enter mode (1 = first char, > 1 later) */
static void LenterARGcomplete(int rcode);
/*---------------------------------------------------------------------------*/
void leright()    { Lx++; }
void leleft()     { Lx--; }
void lebeg() { Lx = Lxlm; }
void leend() 
{
  for (Lx = Lxrm; Lx > Lxlm; Lx--) if (! tcharIsBlank(Lebuf[Lx-1])) return;
}
void lewleft()  { Lx = Lwnd->wtx;                 }
void lewright() { Lx = Lwnd->wtx + Lwnd->wsw - 1; }
void letab()
{ 
  int x = (Lx + TABsize)/TABsize*TABsize;         Lx = (x < Lxrm) ? x : Lxrm-1;
}
void leltab() { int x = (Lx - 1)/TABsize*TABsize; Lx = (x > Lxlm) ? x : Lxlm; }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void lecentr()            /* Esc NN Ctrl+H => В позицию с номером KbCount-1  */
{                         /*          else => Текущую позицию в центр экрана */
  if (KbRadix) {
    int x = KbCount ? KbCount-1 : 0; if (Lxlm <= x && x <= Lxrm) Lx = x;
                                     else                 exc(E_MOVEND); }
  else {
    int dx = (Lwnd->wsw - 1) / 2; if (Lx < dx) wadjust(Lwnd, 0,     Ly);
                                  else {       wadjust(Lwnd, Lx+dx, Ly);
                                               wadjust(Lwnd, Lx-dx, Ly); }}
}
/*---------------------------------------------------------------------------*/
void tleload (void)                                /* load LE line from text */
{
  Ltxt = Ttxt; Lx = Tx; Lleng = TxFRead(Ttxt, Lebuf);
  Lwnd = Twnd; Ly = Ty;
  Lclang = Ttxt->clang;
  Lredit = qTxBottom(Ttxt) ? FALSE : (Ttxt->txredit == TXED_YES);
  Lchange = FALSE;
  tundoload(Ttxt); tlesniff(Ttxt);
}
void tlesniff(txt *tx)
{
  Lxlm = Lxle = tx->txlm;
  Lxrm = Lxre = tx->txrm;
  if (Lebuf[Lxlm] & AT_PROMPT) { // no PROMPT editing (but unset right margin)
    Lxrm = Lxre = MAXTXRM;       //
    while (Lxle < Lxre && Lxle < Lleng && (Lebuf[Lxle] & AT_PROMPT)) Lxle++;
} }
void EnterLEmode (void) { TxSetY(Ttxt, Ty); tleload(); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tleunload (void)                              /* unload LE line to text */
{
  if (Lwnd) {
    Lwnd = NIL; tundounload(Ttxt);
    Tx = Lx;
    if (Lchange) { TxSetY(Ttxt, Ty); TxTRep(Ttxt, Lebuf, Lleng);
                                     Ttxt->txstat |= TS_CHANGED; tesmark(); }
    else
      if (Lclang) vipRedrawLine(Twnd, Ty - Twnd->wty);
} }
void ExitLEmode (void) { if (leARGmode) LenterARGcomplete(0);
                         else { UndoMark = true; tleunload(); } }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool tleread(void)    /* read Lebuf from text (read-only), true if non-empty */
{
           TxSetY (Ttxt, Ty); if (qTxBottom(Ttxt)) return FALSE;
  Lleng  = TxTRead(Ttxt, Lebuf);                   return (Lleng > 0);
}
/*---------------------------------------- Базовый уровень строчных операций */
void blktspac (tchar *p, int len)
{
  if (len) do { *p++ = (tchar)' '; } while(--len);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int lstrlen (int lmax, tchar *p0)
{
  if (p0 == NIL) return 0;
  else {
    tchar *p; for (p = p0+lmax; p > p0 && p[-1] == (tchar)' '; p--);
    return p-p0;
} }
static int leleng() { return lstrlen(MAXLPAC, Lebuf); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void lefill (tchar* dest, int len, tchar *src)
{
  for (; len; len--) *dest++ = (src != NIL) ? *src++ : (tchar)' ';
}
void llmove (int xl, int xr, int dx, tchar *ns) /* Move: xl - start position */
{                                               /*       xr - end position   */
  if (dx == REPLACE) {                          /*       dx - move direction */
    lundoadd(Ltxt, xl, xr, dx, Lebuf+xl, ns);   /*       ns - inserted chars */
    lefill(Lebuf+xl, xr-xl, ns);
  } 
  else {
    if (dx == 0) return;
    if (dx  > 0) {
      lundoadd(Ltxt, xl, xr, dx, Lebuf+xr-dx, ns);
      blktmov (Lebuf+xl, Lebuf+xl+dx, xr-xl-dx);
      lefill  (Lebuf+xl, dx, ns);
    } 
    else {
      lundoadd(Ltxt, xl, xr, dx, Lebuf+xl, ns);
      blktmov (Lebuf+xl-dx, Lebuf+xl, xr-xl+dx);
      lefill  (Lebuf+xr+dx, -dx, ns);
  } }
  Lleng = leleng();
  if (Lclang)
       vipRedrawLine(Lwnd,                 Ly - Lwnd->wty          );
  else vipRedraw    (Lwnd, xl - Lwnd->wtx, Ly - Lwnd->wty, xr-xl, 1);
}
void ledeol();    /* == llmove(Lx, Lxre, REPLACE, NIL); used in leLLCE below */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void llchar (tchar lchar)    /* replace current character (and force redraw) */
{
  int x = Lx; lundoadd(Ltxt, x, x+1, REPLACE, Lebuf+x, &lchar);

  if ((Lebuf[x++] = lchar) != ' ') { if (Lleng <  x) Lleng = x;        }
  else                             { if (Lleng == x) Lleng = leleng(); }
  if (Lclang) 
       vipRedrawLine(Lwnd,                 Ly - Lwnd->wty     );
  else vipRedraw    (Lwnd, Lx - Lwnd->wtx, Ly - Lwnd->wty, 1,1);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void leic2 (tchar lchar)        /* low-level character insert and char entry */
{
  if (tcharIsBlank(Lebuf[Lxre-1])) llmove(Lx, Lxre, 1, &lchar);
  else                                           exc(E_EDTEND);
}
void leic20 (tchar *buf, int len)
{
  if (lstrlen(len, Lebuf+Lxre-len) == 0) llmove(Lx, Lxre, len, buf);
  else                                                exc(E_EDTEND);
}
void leLLCE (tchar lchar)
{
  if (leARGmode == 1) { Lx = Lxle;   /* clear the line if fist key / command */
      leARGmode++;      ledeol ();   /* for LenterARG is regular character   */
  }
  if (lchar == ACPR || lchar == TAB || lchar == CR 
                                    || lchar == LF) exc(E_ILCHAR);
  // Insert char (even in replace
  // mode) when cursor is over special character (with AT_SUPER attribute):
  //
  if (LeInsMode || (Lebuf[Lx] & AT_SUPER)) leic2 (lchar);
  else                                     llchar(lchar); Lx++;
}
/*---------------------------------------------------------------------------*/
void leIC() { if (ldpgMode && Lx > 0) leldpgIC();
              else                    leic2(' '); }
//- - - - - - - - - - - - - - - - - - - - - - - - -
void leDC()   { llmove(Lx, Lxre,      -1, NIL); }
void ledeol() { llmove(Lx, Lxre, REPLACE, NIL); }
void ledbgol()
{                
  if (LeInsMode || (Lebuf[Lx] & AT_SUPER)) 
       { llmove(Lxle, Lxre, Lxle-Lx, NIL); Lx = Lxle; }
  else { llmove(Lxle,   Lx, REPLACE, NIL);            }
}
void lebs() { if (LeInsMode || (Lebuf[Lx] & AT_SUPER)) { Lx--; leDC();      }
              else                                     { Lx--; llchar(' '); }}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void lespchar() { if (KbRadix == 0) exc (E_ILCHAR);
                  else             leLLCE(KbCount); } // character by code
void lechar()   {             leLLCE(KbCode|Lattr); } // regular character
void lecrchar() { if (!dosEOL)  leLLCE(';'|AT_TAB); } // CR for mixed EOL
void letabchr() 
{ 
  static tchar TABchars[] = { ' '|AT_TAB,' ',' ',' ',' ',' ',' ',' ' };
  if (TABsize > 8) TABsize = 8;
  int dx = TABsize - (Lx % TABsize);
  llmove(Lx, Lxre, dx, TABchars); letab();
}
void lehchar2() { leLLCE(LeSCH_REPL_BEG); } /* Ctrl+.('>') ʈ>ʀ */
void lehchar1() { leLLCE(LeSCH_REPL_END); } /* Ctrl+/('/') ʈ/ʀ */
void lehchar0() { leLLCE(TmSCH_THIS_OBJ); } /* Ctrl+,('<')   */
/*---------------------------------------------------------------------------*/
bool leNword (int *cwbeg,   /* Найти (unless ptr=0): начало текущего слова   */
              int *cwend,   /*                       конец текущего слова    */
              int *nwbeg)   /* (return "на слове?")  начало следующего слова */
{
  bool onWord = TRUE;
  int x = Lx;
  if (tcharIsBlank(Lebuf[x])) onWord = FALSE;
  else {
    if (cwbeg) {                    // NOTE: logic for cwbeg/cwend is different
      for (x = Lx; x > Lxlm; x--) { //      (despite code looking very similar)
        int pc = Lebuf[x-1] & AT_CHAR;
        if (pc == ' ' ||   SyntType(pc) == ';'
                      || ( SyntType(pc) != '(' &&
                           SyntType(Lebuf[x] & AT_CHAR) == '(' )) break;
      } *cwbeg = x;
    }
    if (cwend || nwbeg) {
      for (x = Lx; x < Lxrm-1; x++) {
        int tc = Lebuf[x] & AT_CHAR, nc = Lebuf[x+1] & AT_CHAR;
        if (nc == ' ' ||   SyntType(tc) == ';'
                      || ( SyntType(tc) != '(' &&
                           SyntType(nc) == '(' )) break;
    } }
    if (cwend) *cwend = x;
  }
  if (nwbeg) { for (*nwbeg = ++x; x < Lxrm; x++)
                 if (!tcharIsBlank(Lebuf[x])) { *nwbeg = x; break; }
  }
  return onWord;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void lenword() { leNword(NIL, NIL, &Lx); }     /* в начало следующего  слова */
void lepword()                                 /* в начало предыдущего слова */
{ 
  if (Lx > Lxlm) {
    for (Lx--; Lx > Lxlm && tcharIsBlank(Lebuf[Lx]); Lx--);
    leNword(&Lx, NIL, NIL);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void lemark_word()    /* выделить текущее слово (или слово слева от курсора) */
{
  while (Lx > Lxlm && tcharIsBlank(Lebuf[Lx])) Lx--;
  if (leNword(&BlockTx, &Lx, NIL)) {
                  BlockMark = TRUE;
    BlockTy = Ty; BlockTemp = TRUE; Lx++;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void ledword()   /* удалить слово и пробелы за ним (всегда схлопывает текст) */
{
  int xr; if (Lx < Lxle) Lx = Lxle; leNword(NIL, NIL, &xr);
          if (xr > Lxre) xr = Lxre; llmove(Lx, Lxre, Lx-xr, NIL);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void ledlword()        /* удалить слово влево (очистить или схлопнуть текст) */
{                      /*         (если выделен блок => схлопнуть его, te.c) */
  int saved_Lx = Lx; lepword();
  if (Lx < Lxle) Lx = Lxle;
  if (Lx < saved_Lx) {
    if (LeInsMode) llmove(Lx, Lxre, Lx-saved_Lx, NIL);
    else           llmove(Lx, saved_Lx, REPLACE, NIL);
} }
/*---------------------------------------------------------------------------*/
void lecentrx()    /* центрировать текст в текущей строке (по размерам окна) */
{
  int xll = my_max(Lxle, Lxlm), xl = xll, mvlen,
      xrr = my_min(Lxre, Lxrm), xr = xrr;
  if (xrr > Lwnd->wsw)          xr = xrr = Lwnd->wsw;

  do { if (xl >= xrr) return; } while (tcharIsBlank(Lebuf[xl++]));
  xl = xl - xll - 1;
  do { if (xr <= xll) return; } while (tcharIsBlank(Lebuf[--xr]));
  xr = xrr - xr - 1;
  mvlen = (xr-xl)/2;      if (mvlen) llmove(xll, xrr, mvlen, NIL);
}
void lerins() { LeInsMode = TRUE;  } /* режим вставки */
void lerrep() { LeInsMode = FALSE; } /* режим замены  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void leconvert (int type) /* convert upper <-> lower case for letters */
{                                /* and decimal <-> other bases for numbers  */
  int x0, x1, len;
  if (! Block1size(&x0, &x1)) x1 = (x0 = Lx) + 1;
  len = vipConvert(Lebuf+x0, x1-x0, type, lfbuf); 
  if (len < 0) exc(E_ILCHAR);
       if (len < x1-x0) llmove(x0+len, Lxre, len+x0-x1, NIL);
  else if (len > x1-x0) llmove(x1,     Lxre, len+x0-x1, NIL);
  llmove(x0, Lx = x0+len, REPLACE, lfbuf);
  if (BlockMark) BlockTx = x0;
}
void leccup()  { leconvert(cvTO_UPPER);   }
void leccdwn() { leconvert(cvTO_LOWER);   }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void leconvert_word (int type)
{
  if (!BlockMark) { scblkon(TRUE); lepword(); Tx = Lx; }
  leconvert(type);
}
void lecwdec() { leconvert_word(cvTO_DECIMAL); }
void lecwhex() { leconvert_word(cvTO_RADIX);   }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void  make_tchar_bold(tchar *tc) { *tc |= AT_BOLD; }
static void clear_tchar_attr(tchar *tc) { *tc &= AT_CHAR; }
static void leupdate_attr(void (*fn)(tchar*))
{
  int x0,x1; if (! Block1size(&x0,&x1)) x1 = (x0 = Lx) + 1;
  for (Lx = x0; Lx < x1; Lx++)  (*fn)(&Lebuf[Lx]);
  if (BlockMark) { BlockTx = x0;                       // just to make sure
                   llmove(x0,x1, REPLACE, Lebuf+x0); } // window is updated
}
void lecbold() { leupdate_attr( make_tchar_bold); }
void leclear() { leupdate_attr(clear_tchar_attr); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void lemovleft() 
{
  int x0,x1; if (Block1size(&x0, &x1)) BlockTx--;
             else { x0 = Lx;
                    x1 = Lxre; } Lx--; llmove(x0-1, x1, -1, NIL);
}
void lemovright() /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
{ 
  int x0,x1; if (Block1size(&x0, &x1)) BlockTx++;
             else { x0 = Lx;
                    x1 = Lxre-1; } Lx++; llmove(x0, x1+1, +1, NIL);
}
/*---------------------------------------------------------------------------*/
static tchar *tpstrdup (tchar *tline, int len) /* duplicate as Pascal string */
{
  tchar *tpline = calloc(len+1, sizeof(tchar));
  *tpline = len;
  blktmov(tline, tpline+1, len); return tpline;
}
static int tpstrcmp (tchar *tp1, tchar *tp2) /* the ordering (by lentgh then */
{                                            /* by content) is not used now  */
  int len1 = *tp1++,
      len2 = *tp2++;     if (len1 != len2) return len1-len2;
  while (len1--) {
    tchar diff = (*tp1++) - (*tp2++); if (diff) return diff;
  }                                             return    0;
}
/*---------------------------------------------------------------------------*/
static tchar  *leResARG, **leHistory;       /* LenterARG: ввод аргумента для */
static int *leResARGlen,  *leRetCode;       /* поиска/замены и других команд */
static int leKBCode[2] = { TE_CR, TE_RCR }; /*-------------------------------*/
int leOptFlags, leOptMode;
txt *Atxt = NIL;                            /* Псевдо-текст для ввода строки */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void laSetOption (int option)
{
  const char *smode = "..", *imode = NIL; int i;
  switch (option) {
  case LeARG_STANDARD:   Lattr = 0;         smode = "st"; break;
  case LeARG_WILDCARD:   Lattr = AT_PROMPT; smode = "wc"; break;
  case LeARG_REGEXP:     Lattr = AT_REGEX;  smode = "re"; break;
  case LeARG_IGNORECASE:
    if (leOptMode & LeARG_IGNORECASE) imode = "cs";
    else                              imode = "ic";
    leOptMode ^= LeARG_IGNORECASE;
    Lebuf[4] = tATTR(Lebuf[4]) + imode[0];
    Lebuf[5] = tATTR(Lebuf[5]) + imode[1]; return;
  }
  leOptMode = option | (leOptMode & LeARG_IGNORECASE);
  Lebuf[1] = tATTR(Lebuf[1]) + smode[0];
  Lebuf[2] = tATTR(Lebuf[2]) + smode[1];
  for (i = Lxlm; i < Lleng; i++)
    if (!(Lebuf[i] & AT_SUPER))
      Lebuf[i] = Lattr | (Lebuf[i] & ~(AT_PROMPT|AT_REGEX));
}
static void redrawLwnd() { wadjust  (Lwnd, 0, Ly);
                           vipRedraw(Lwnd, 0, Ly - Lwnd->wty, Lwnd->wsw, 1); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void LenterARG(tchar *buf, int *bufLen, /* buffer for argument & its length  */
                           int promLen, /* len of prompt (in buffer already) */
          tchar **history, int *kbcode, /* place for history and return code */
          int do_on_CR,  int do_on_RCR, /* commands to execute on CR and RCR */
                         int opt_flag)  /* allowed options (st/wc/re//ic/cs) */
{
  if (Atxt == NIL) { Atxt = TxNew(TRUE); Atxt->txstat |= TS_PERM+TS_PSEUDO; }
  TxEmpt(Ltxt = Atxt);
  Lwnd    = Twnd;                Ly = Ty;      leResARG    = buf;
  Lredit  = TRUE;  Lxlm = Lxle = Lx = promLen; leResARGlen = bufLen;
  Lchange = FALSE; Lxrm = Lxre = MAXTXRM;      leKBCode[0] = do_on_CR;
                          leARGmode =  1;      leKBCode[1] = do_on_RCR;
                          leHistory = history; leRetCode   = kbcode;
  if (BlockMark) {
    if (BlockTy == Ty) { int len = Block1move(buf+promLen, MAXLPAC-promLen);
                         *bufLen = Lx =       len+promLen;      leARGmode++; }
    scblkoff();                                          // ^
  }                                                      // block selected =>
  Lleng = *bufLen; blktmov (buf,  Lebuf,         Lleng); //  copy the contents
                   blktspac(Lebuf+Lleng, MAXLPAC-Lleng); // (only when 1-line)
  if (opt_flag) {                                        //  + always unselect
    switch (Lebuf[1] & AT_CHAR) {
    case 's': leOptMode = LeARG_STANDARD; break;
    case 'r': leOptMode = LeARG_REGEXP;   break;
    case 'w': leOptMode = LeARG_WILDCARD; break;
    }
    leOptFlags = opt_flag;           laSetOption(leOptMode);
    if ((Lebuf[4] & AT_CHAR) == 'i') laSetOption(LeARG_IGNORECASE);
  }
  else { leOptFlags = 0; leOptMode = LeARG_STANDARD; }
  redrawLwnd();                       exc(E_LENTER);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void laToggle (int option)
{
  if (leARGmode && (leOptFlags & option)) { laSetOption(option);
                                                   redrawLwnd(); }
  else exc(E_ATTR);
}
void lastdmode() { laToggle(LeARG_STANDARD);   }
void laremode()  { laToggle(LeARG_REGEXP);     }
void lawcmode()  { laToggle(LeARG_WILDCARD);   }
void laignorec() { laToggle(LeARG_IGNORECASE); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int LeHistorySave()
{
  tchar *cLine = tpstrdup(Lebuf + Lxle, Lleng < Lxle ? 0 : Lleng - Lxle);
  int i;
  for (i = 0; i < LE_HISTORY_SIZE && leHistory[i]; i++) {
    if (tpstrcmp(leHistory[i], cLine) == 0) {
      free(cLine);
      return i; /* return position of the string in the history */
  } }
  if (leHistory[i = LE_HISTORY_SIZE-1]) free(leHistory[i]);
  while (i) {
    leHistory[i] = leHistory[i-1]; i--; /* No such string yet - move history */
  }                                     /* one position down, add new string */
  leHistory[0] = cLine;  return 0;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void LeHistorySelect (int delta)
{
  int cPos = LeHistorySave() + delta;

  if (0 <= cPos && cPos < LE_HISTORY_SIZE && leHistory[cPos]) {
    int len = *leHistory[cPos];
    Lleng = Lxle + len;                          // Restore search mode from
    blktmov(leHistory[cPos]+1, Lebuf+Lxle, len); // history string only when
    blktspac(Lebuf+Lleng, MAXLPAC-Lleng);        // modes are allowed (Find)
    if (leOptFlags && Lleng > Lxle) {            //
           if (Lebuf[Lxle] & AT_PROMPT) laSetOption(LeARG_WILDCARD);
      else if (Lebuf[Lxle] & AT_REGEX)  laSetOption(LeARG_REGEXP);
      else                              laSetOption(LeARG_STANDARD);
  } }
  else vipBell();
}
void leup()   { if (leHistory) LeHistorySelect(+1); redrawLwnd(); }
void ledown() { if (leHistory) LeHistorySelect(-1); redrawLwnd(); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static void LenterARGcomplete (int rcode)
{
  if (leRetCode) *leRetCode   = rcode;  leARGmode = 0;
                 *leResARGlen = Lleng;  Lattr     = 0;
  blktmov (Lebuf, leResARG, Lleng);
  if (leHistory) LeHistorySave();
  Lwnd = NIL;
  vipRedraw(Twnd, 0, Ly - Twnd->wty, Twnd->wsw, 1);

  KbCode = (rcode == TE_CR)  ? leKBCode[0] :
           (rcode == TE_RCR) ? leKBCode[1] : rcode;
}
void lecr()  { LenterARGcomplete(TE_CR);  exc(E_LEXIT); }
void lercr() { LenterARGcomplete(TE_RCR); exc(E_LEXIT); }
void lequit() 
{
  switch (KbCode) {
  case TE_SDOWN: LenterARGcomplete(TE_CR);  exc(E_LEXIT);
  case TE_SUP:   LenterARGcomplete(TE_RCR); exc(E_LEXIT);
  case TE_NPAGE:
  case TK_BREAK:
    LenterARGcomplete(0); exc(E_LEXIT);
  default:                exc(E_NOCOM);
} }
/*---------------------------------------------------------------------------*/
comdesc lecmds[] =
{
  { LE_CHAR,   lechar,   CA_EXT|CA_LinMOD|CA_NEND }, /* обычный символ (1st) */
  { LE_RIGHT,  leright,                   CA_NEND }, /* курсор вправо        */
  { LE_LEFT,   leleft,                    CA_NBEG }, /* курсор влево         */
  { LE_BEG,    lebeg,    CA_RPT                   }, /* - в начало строки    */
  { LE_END,    leend,    CA_RPT                   }, /* - в конец строки     */
  { LE_WLEFT,  lewleft,  CA_RPT                   }, /* в самую левую/правую */
  { LE_WRIGHT, lewright, CA_RPT                   }, /*       позицию в окне */
  { LE_TAB,    letab,                     CA_NEND }, /* в следующую TAB      */
  { LE_LTAB,   leltab,                    CA_NBEG }, /* в предыдущую TAB     */
  { LE_CENTR,  lecentr,  0                        }, /* курсор в центр окна  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { LE_IC,     leIC,            CA_LinMOD|CA_NEND }, /* вставить пробел      */
  { LE_DC,     leDC,            CA_LinMOD|CA_NEND }, /* удалить за курсором  */
  { LE_DEOL,   ledeol,          CA_LinMOD         }, /* - конец строки       */
  { LE_DBGOL,  ledbgol,         CA_LinMOD         }, /* - начало строки      */
  { LE_BS,     lebs,            CA_LinMOD|CA_NBEG }, /* удалить до курсора   */
  { LE_SPCHAR, lespchar, CA_EXT|CA_LinMOD|CA_NEND|CA_RPT },
  { LE_TABCHR, letabchr, CA_EXT|CA_LinMOD|CA_NEND }, /* insert TAB           */
  { LE_CRCHAR, lecrchar, CA_EXT|CA_LinMOD|CA_NEND }, /* insert CR, mixed EOL */
  { LE_HCHAR0, lehchar0, CA_EXT|CA_LinMOD|CA_NEND }, /* ins: TmSCH_THIS_OBJ  */
  { LE_HCHAR1, lehchar1, CA_EXT|CA_LinMOD|CA_NEND }, /* ins: LeSCH_REPL_END  */
  { LE_HCHAR2, lehchar2, CA_EXT|CA_LinMOD|CA_NEND }, /* ins: LeSCH_REPL_BEG  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  { LE_NWORD,  lenword,                 CA_NEND }, /* на следующее слово     */
  { LE_PWORD,  lepword,                 CA_NBEG }, /* на предыдущее слово    */
  { LE_MWORD,  lemark_word,      CA_RPT         }, /* выделить текущее слово */
  { LE_DWORD,  ledword,       CA_LinMOD|CA_NEND }, /* удалить слово справа   */
  { LE_DLWORD, ledlword,CA_BLOCK|CA_MOD|CA_NBEG }, /* удалить слово влево    */
  { LE_CENTRX, lecentrx,  CA_RPT|CA_MOD         }, /* центрировать строку    */
  { LE_RINS,   lerins,    0                     }, /* режим вставки          */
  { LE_RREP,   lerrep,    0                     }, /* режим замены           */
  { LE_CCUP,   leccup,           CA_MOD|CA_NEND }, /* -> прописная           */
  { LE_CCDWN,  leccdwn,          CA_MOD|CA_NEND }, /* -> строчная            */
  { LE_CWDEC,  lecwdec,   CA_RPT|CA_MOD         }, /* word -> decimal        */
  { LE_CWHEX,  lecwhex,   CA_RPT|CA_MOD         }, /* word -> hex            */
  { LE_CBOLD,  lecbold,          CA_MOD|CA_NEND }, /* сделать жирным         */
  { LE_CLEAR,  leclear,          CA_MOD|CA_NEND }, /* очистить аттрибуты     */
  { LE_MOVRIGHT, lemovright,     CA_MOD|CA_NEND }, /* сдвинуть вправо        */
  { LE_MOVLEFT,  lemovleft,      CA_MOD|CA_NBEG }, /* сдвинуть влево         */
/*
 * Only works for leARGmode (do not process unless leARGmode):
 */
  { LA_STDMODE, lastdmode, CA_LEARG|CA_RPT }, /* Режим поиска: стандартный   */
  { LA_REMODE,  laremode,  CA_LEARG|CA_RPT }, /*       regular expressions   */
  { LA_WCMODE,  lawcmode,  CA_LEARG|CA_RPT }, /*       wildcards             */
  { LA_IGNOREC, laignorec, CA_LEARG|CA_RPT }, /* "ignore case" toggle        */
  { TE_UP,      leup,      CA_LEARG },    /* up in argument history list     */
  { TE_DOWN,    ledown,    CA_LEARG },    /* down in history list            */
  { TE_CR,      lecr,      CA_LEARG },    /* normal termination of LenterARG */
  { TE_RCR,     lercr,     CA_LEARG },    /* - alternative termination       */
/*
 * Implemented in clip.cpp (declaration in "clip.h"):
 */
  { LE_CCHAR,  lecchar,  CA_BLOCK|       CA_NEND }, /* запомнить символ   */
  { LE_CDCHAR, lecdchar, CA_BLOCK|CA_MOD|CA_NEND }, /* - с удалением      */
  { LE_CWORD,  lecword,                  CA_NEND }, /* запомнить слово    */
  { LE_CDWORD, lecdword,          CA_MOD|CA_NEND }, /* - с удалением      */
  { LE_PASTE,  cpaste,            CA_MOD|CA_NEND }, /* вспомнить          */
  { LE_CPCLOS, cpclose,  0                       }, /* запоминать сначала */
  { LE_CPOPEN, cpreopen, 0                       }, /* переоткрыть буфер  */
/*
 * Implemented in ud.c (declaration in "ud.h"), the same functions as in te.c
 */
  { TE_UNDO,    leundo,    CA_MOD }, /* откатка (always set Lchange) */
  { TE_UNUNDO,  leunundo,  CA_MOD }, /* откатка откатки              */
  { TE_SUNDO,   lesundo,   CA_MOD }, /* "медленная" откатка          */
  { TE_SUNUNDO, lesunundo, CA_MOD }, /* "медленная" откатка откатки  */
  { 0,0,0 }
};
comdesc lequitcmd[] =
{
  { TK_NONE, lequit, CA_LEARG }       /* any non-LE command in ArgEnter mode */
};
comdesc *Ldecode (int kcode)  /*- - - - - - - - - - - - - - - - - - - - - - -*/
{                                        // for LE, attribute CA_BLOCK marks
  if (Mk_IsCHAR(kcode)) kcode = LE_CHAR; // commands that are NOT processed
  comdesc *cp;                           // here when block is selected (but
  for (cp = lecmds; cp->cfunc; cp++) {   // should be passed to TE instead)
    if ((int)cp->mi_ev == kcode) {       //
      if ((cp->attr & CA_BLOCK) && BlockMark ) return NIL;
      if ((cp->attr & CA_LEARG) && !leARGmode) return NIL;
      else                                     return cp;
  } }
  return leARGmode ? lequitcmd : NIL;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int LeCommand (comdesc *cp)
{
  jmp_buf *nextexc, leenv;
  int x, rpt;
  if ((cp->attr & CA_EXT) && !Lredit && Ttxt->txredit == TXED_YES) {
    UndoMark = true;       tleunload();
    TxSetY(Ttxt,Ty); teIL(); tleload(); // the only case Lredit may be false
  }                                     // for editable text is when current
  if (cp->attr & CA_MOD) {              // line is end-of-text, insert empty
    if (Lredit) UndoMark = true;
    else        return E_CHANGE;
  }
  for (rpt = (cp->attr & CA_RPT) ? 1 : KbCount; rpt; rpt--) {
    if (cp->attr & (CA_NEND|CA_NBEG)) {
      x = Lx;
      if ((cp->attr & CA_NBEG) && Lx <= Lxlm) return E_MOVBEG;
      if ((cp->attr & CA_NEND) && Lx >= Lxrm) return E_MOVEND;
      if (cp->attr & CA_MOD) {
        x = (cp->attr & CA_NBEG) ? Lx-1 : Lx; // for NBEG check pos to the left
        if (x >= Lxre)              return E_EDTEND;
        if (x <  Lxle) { Lx = Lxle; return E_EDTBEG; }
    } }
    nextexc = excptr; excptr = &leenv;           // "tall cursor" works only if
    if ((x = setjmp(leenv)) == 0) {              // block is 1 char wide + make
      if ((cp->attr & CA_LINEAR) && BlockMark) { // sure we're not in leARGmode
        if (leARGmode || BlockTx != Tx ||        //
                         BlockTy == Ty) exc(E_BLOCKOUT);
        tallblockop(cp);
      }
      else {        (*cp->cfunc)(); // <-- perform requested operation here;
        if (leARGmode) leARGmode++; // any command makes next char not first
        else Tx = Lx;
        if (cp->attr & CA_MOD) Lchange = TRUE; // mark text as "changed" (tall
    } }                                        // block ops also do that mark)
    excptr = nextexc;
    if (x)  return x; if (qkbhin()) return E_KBREAK;
  }
  return E_OK;
}
/*---------------------------------------------------------------------------*/
int char2tcharsX(const char *orig, tchar *buffer, tchar attr)
{
  int len; for (len = 0; *orig && len < MAXLPAC; len++)
              *buffer++ = attr | ctotc(*orig++);
  return len;
}
int char2tchars(const char *orig, tchar *buffer)
{
  return char2tcharsX(orig, buffer, 0);
}
void tchar2chars(const tchar *orig, int len, char *buffer)
{
  while (len--) *buffer++ = (char)*orig++; *buffer = 0;
}
/*---------------------------------------------------------------------------*/

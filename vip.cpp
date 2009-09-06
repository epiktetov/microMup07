//------------------------------------------------------+----------------------
// МикроМир07 ViewPort (interface Qt/C++ • legacy code) | (c) Epi MG, 2006-2007
//------------------------------------------------------+----------------------
#include <QApplication>
#include <QPainter>
#include <QString>
#include <QFileInfo>
#include <QMessageBox>
#include "mic.h"
#include "mim.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
#include "clip.h" // clipRefocus()
extern "C" {
#include "le.h"
#include "te.h"
}
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
BOOL BlockMark = FALSE; QRect BlockMarkRect;   wnd *windows = NULL;
BOOL BlockTemp = FALSE;
int  BlockTx,  BlockTy; /* block 1st corner (another is cursor) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd *vipNewWindow (wnd *wbase, int sw, int sh, int kcode)
{
  wnd *w = (wnd*)malloc(sizeof(wnd));
  w->wtext = NULL;
  w->dirsp = w->stack;
  L2_INSERT(windows, w, wdprev, wdnext);
  w->wsw = (sw < 0) ? MiApp_defWidth  : sw;  w->cx  = w->cy = -1;
  w->wsh = (sh < 0) ? MiApp_defHeight : sh;  w->wty = w->wtx = 0;

  MiFrame *basef = wbase ? wbase->sctw->mf : 0;
  MiFrame *frame;
  if (kcode == TM_VFORK) {
    if (wbase && wbase->wsibling) w->wsh += wbase->wsibling->wsh;
    w->wsibling = NULL;                           w->sib_pos = 0;
    frame = new MiFrame(basef);
    frame->NewScTwin(w); frame->show();
  }
  else if (wbase) {
    w->wsibling = wbase; wbase->wsh -= w->wsh; wbase->sib_pos = 0;
    wbase->wsibling = w; basef->NewScTwin (w);     w->sib_pos = 1;
  }
  else { free(w);  // incorrect function call - for TE_HFORK
         w = NULL; // case the base window must be specified
  }
  if (wbase) vipFocusOff(wbase); return w;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd *vipSplitWindow (wnd *wbase, int kcode)   /* TE_VFORK creates new window */
{
  if (kcode == TM_VFORK) return vipNewWindow(wbase, wbase->wsw,
                                                    wbase->wsh, kcode);
  else if (wbase->wsibling) return NULL;
  else {
    int h = wbase->wsh / 2; // ... and TE_HFORK splits existing one
    if (h < 3) return NULL;
    wnd *wind = vipNewWindow(wbase, wbase->wsw, h, kcode);
    if (wind) wredraw(wbase);                 return wind;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool vipFreeWindow (wnd *vp) /* returns true if last window has been deleted */
{
  wnd *next_active     = NULL;  L2_DELETE(windows, vp, wdprev, wdnext);
  if (Twnd == vp) Twnd = NULL;
  if (vp->wsibling) {
    vp->wsibling->wsh  += vp->wsh;
    vp->wsibling->wsibling = NULL; next_active = vp->wsibling;
  }
  MiFrame *frame = vp->sctw->mf;
  frame->DeleteScTwin(vp->sctw);
  free(vp);
  if (next_active) { vipActivate(next_active);
                     frame->updateWinTitle (); return false; }
  else {
    delete frame; return (windows == NULL);
} }
/*---------------------------------------------------------------------------*/
inline void vipGetPosition (wnd *vp, int& x, int& y)
{
  MiFrame *frame = vp->sctw->mf; x = frame->x(); // TODO: lower pane?
                                 y = frame->y();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd *vipFindWindow (wnd *wbase, int kcode) /* rel.direction TM_U/L/D/RWINDOW */
{
  if (wbase->wsibling) {
    if ((kcode == TM_DWINDOW && wbase->sib_pos == 0)
     || (kcode == TM_UWINDOW && wbase->sib_pos == 1)) return wbase->wsibling;
  }
  int dx = 0, dy = 0, xb, yb, x, y, xt, yt, delta;
  wnd *w, *wt = NULL;
  vipGetPosition(wbase, xb, yb);
  switch (kcode) {
  case TM_UWINDOW: dx =  0; dy = -1; break;
  case TM_DWINDOW: dx =  0; dy = +1; break;
  case TM_LWINDOW: dx = -1; dy =  0; break;
  case TM_RWINDOW: dx = +1; dy =  0; break;
  }
  xt = yt = 2147483647; /* infinity */
  if (dx) {
    for (w = windows; w != NULL; w = w->wdnext) {
      vipGetPosition(w, x, y);
      if ((delta = dx*(x - xb)) > 0 && delta < xt) { wt = w; xt = delta; }
  } }
  if (dy) {
    for (w = windows; w != NULL; w = w->wdnext) {
      vipGetPosition(w, x, y);
      if ((delta = dy*(y - yb)) > 0 && delta < yt) { wt = w; yt = delta; }
  } }
  return wt;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipFocus (wnd *vp) { vp->sctw->mf->activateWindow();
                          vp->sctw->mf->raise(); // just in case
                          vp->sctw->setFocus();           
}
void vipUpdateWinTitle (wnd *vp) { vp->sctw->mf->updateWinTitle(); }
/*---------------------------------------------------------------------------*/
void vipRepaint(wnd *vp, QPainter& dc, MiScTwin *sctw, int x0, int x1,
                                                       int y0, int y1)
{
//+
// fprintf(stderr, "vipRepaint(%d-%d,%d-%d) ", x0,x1, y0,y1);
//-
  int wt = my_min(x1-x0+1, MAXLPAC);
  QRect cline;
  for (int y = y0; y <= y1; y++) {
    tchar buf[MAXLPAC];
    int len;
    tchar *pt_line = TxInfo(vp, y, &len) + x0;
                                    len -= x0;
         if (len < 0)  len = 0;
    else if (len > wt) len = wt;
    //
    // Check if need to draw cursor and/or block mark -- to simplify the logic,
    // just copy text and add cursor / block attributes (INVERT and appropriate 
    // color) to the buffer:
    //
    cline = QRect(x0, y, wt, 1);
    if ((BlockMark && cline.intersects(BlockMarkRect))
                   || cline.contains(vp->cx, vp->cy) ) {
      if (len > 0) memcpy(buf, pt_line, len * sizeof(tchar));
      if (len < wt) {
        memset(buf+len, 0x20, (wt-len) * sizeof(tchar)); len = wt;
      }
      if (cline.contains(vp->cx, vp->cy)) buf[vp->cx - x0] ^= vp->ctc;
      if (BlockMark && cline.intersects(BlockMarkRect)) {
        int xfrom = my_max(x0, BlockMarkRect.left()),
              xto = my_min(x1, BlockMarkRect.right());
        tchar attr = BlockTemp ? AT_BG_RED : AT_BG_BLU;
        for (int x = xfrom; x <= xto; x++) buf[x - x0] |= attr;
      }
      pt_line = buf;
    }
    if (len > 0 ) sctw->Text (dc, x0,     y, pt_line, len);
    if (len < wt) sctw->Erase(dc, x0+len, y,       wt-len);
} }
/*---------------------------------------------------------------------------*/
inline void win_repaint (wnd *vp, int x0, int x1, int y0, int y1)
{
  vp->sctw->Repaint(x0, y0, x1-x0, y1-y0);
}
inline void BlockMark_repaint (wnd *vp)
{
  win_repaint(vp, BlockMarkRect.left(), BlockMarkRect.right() +1,
                   BlockMarkRect.top(), BlockMarkRect.bottom()+1);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void scblkon (BOOL isTemp) { BlockMark = TRUE;   BlockTx = Tx;
                             BlockTemp = isTemp; BlockTy = Ty; }
void scblkoff() 
{ 
  if (BlockMark) { BlockMark = FALSE; BlockMark_repaint(Twnd); 
                                      BlockMarkRect = QRect(0,0,0,0); }
}
/*-----------------------------------------------------------------------------
 * Переизобразим текст после модификации на всех прикрепленных к нему окнах.
 */
void wndop (small op, txt *t)
{
  int y = t->txy, dy;
  wnd *w;
  for (w = t->txwndptr; w != NULL; w = w->wnext) {
    wxmin = 0; wxmax = w->wsw; wdx = 0;
    wymin = 0; wymax = w->wsh; wdy = 0;
    switch (op) {
    case TW_EM: w->wty = 0;
                w->wtx = 0; wrdrw(w);   continue;
    case TW_ALL:            wredraw(w); continue;
    case TW_RP:
      if (w->wty <= y && y < w->wty + w->wsh) {
        wymin = y - w->wty;
        wymax = wymin +  1; wrdrw(w); /* Current line */
      }
      continue;
    case TW_IL: dy = +1; break;
    case TW_DL: dy = -1; break;
    default:          continue;
    }
         if (y <  w->wty) w->wty += dy;
    else if (y >= w->wty+w->wsh); // nothing to do
    else {
      wdy += dy;
      wymin = y - w->wty; wroll(w);
  } }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int wxmin, wxmax, wdx, /*  Перемещение внутри (wxmin..wxmax - wymin..wymax)  */
    wymin, wymax, wdy; /*    на (wdx, wdy) в окне (w)                        */

void wroll (wnd *vp)
{
  if (wxmin < 0) wxmin = 0; if (wxmax > vp->wsw) wxmax = vp->wsw;
  if (wymin < 0) wymin = 0; if (wymax > vp->wsh) wymax = vp->wsh;
  int rollH = wymax-wymin - my_abs(wdy);
  if (MiApp_useDIAGRAD || wdx || rollH <= 0) win_repaint(vp, wxmin, wxmax, 
                                                             wymin, wymax);
  else { // ^ scrolling only possible with horizontal
         // gradient (not diagonal one) and only in vertical direction
         //
    if (wdy > 0) { vp->sctw->Scroll (wxmin, wymin, wxmax-wxmin, rollH, wdy);
                   vp->sctw->Repaint(wxmin, wymin, wxmax-wxmin,        wdy); }
    else {
      vp->sctw->Scroll (wxmin, wymin-wdy, wxmax-wxmin, rollH, wdy);
      vp->sctw->Repaint(wxmin, wymax+wdy, wxmax-wxmin,        wdy);
} } }
void wrdrw  (wnd *vp) { win_repaint(vp, wxmin, wxmax, wymin, wymax); }
void wredraw(wnd *vp) { win_repaint(vp, 0,   vp->wsw, 0,   vp->wsh); }
/*---------------------------------------------------------------------------*/
void wpos (wnd *vp, int x, int y)        /* repaint cursor at given position */
{
  if (vp->wtext) {
    if (vp->wtext->txredit == TXED_YES) {
      if (vp->wtext->txstat & TS_CHANGED) vp->ctc = AT_INVERT | AT_BG_BLU;
      else                                vp->ctc = AT_INVERT | AT_BG_GRN;
    } else                                vp->ctc = AT_INVERT | AT_BG_RED;
    if (LeInsMode) vp->ctc |= AT_SUPER;
  }
  vp->sctw->Repaint(vp->cx = x, vp->cy = y, 1,1);
  if (vp->sctw->info.isVisible()) vp->sctw->info.updateInfo();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void wpos_off (wnd *vp)
{
  int ocx = vp->cx; vp->cx = -1;
  int ocy = vp->cy; vp->cy = -1; vp->sctw->Repaint(ocx, ocy, 1,1);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void wadjust (wnd *w, int x, int y) // "Натянуть" окно на точку (x,y) в тексте
{
  int ddx = x - w->wtx; int moved = 0; // +1: can roll, -1: cannot
  int ddy = y - w->wty;
  if (ddy < 0 || (ddy -= w->wsh-1) > 0) { w->wty += ddy; moved = +1; }
  if (ddx < 0 || (ddx -= w->wsw-1) > 0) { w->wtx += ddx; moved = -1; }
  if (BlockMark) {
    int x0 = my_min(x - w->wtx, BlockTx - w->wtx),
        x1 = my_max(x - w->wtx, BlockTx - w->wtx),
        y0 = my_min(y - w->wty, BlockTy - w->wty),
        y1 = my_max(y - w->wty, BlockTy - w->wty);
    if (x0 < 0) x0 = 0;  if (x1 > w->wsw) x1 =  w->wsw;
    if (y0 < 0) y0 = 0;  if (y1 > w->wsh) y1 =  w->wsh;
    QRect block(x0, y0, x1-x0+1, y1-y0+1);
    if (BlockMarkRect != block) {
                             BlockMark_repaint(w);
      BlockMarkRect = block; BlockMark_repaint(w);
  } }
  if (moved) {
    if (moved > 0 && my_abs(ddy) < w->wsh-1) {
      wxmin = 0; wxmax = w->wsw; wdx = 0;
      wymin = 0; wymax = w->wsh; wdy = -ddy; wroll(w);
    }
    else win_repaint(w, 0, w->wsw, 0, w->wsh);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipReady() 
{                 
       if (Lwnd) { wadjust(Lwnd,Lx,Ly); wpos(Lwnd,Lx-Lwnd->wtx,Ly-Lwnd->wty); }
  else if (Twnd) { wadjust(Twnd,Tx,Ty); wpos(Twnd,Tx-Twnd->wtx,Ty-Twnd->wty); }
}
/*---------------------------------------------------------------------------*/
void vipOnFocus (wnd *vp) /* if possible, goto window (cursor is not Ok yet) */
{ 
  if (vp != Twnd && Ttxt && vp->wtext) vipActivate(vp); vipReady();
}
void vipFocusOff (wnd *vp)
{
  if (Lwnd) ExitLEmode(E_NOCOM);
  if (vp == Twnd) { 
    vp->wcx = Tx; // save the last text position when un-focusing the current
    vp->wcy = Ty; // editor's window... just in case
  }
  scblkoff(); wpos_off(vp);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipActivate (wnd *vp)
{
  if (!tmLoad(vp->wtext)) return;
  Ttxt = vp->wtext;
  if (Twnd != NIL) { Twnd->wcx = Tx;
                     Twnd->wcy = Ty; }
  Tx = vp->wcx;
  Ty = vp->wcy; clipRefocus(); Twnd = vp; // no vipFocus(Twnd) here!
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipGotoXY (int x, int y)           /* click in the active window (Twnd) */
{
  vipFocusOff(Twnd); tesetxy((small)(Twnd->wtx+x), Twnd->wty+y);
  vipOnFocus (Twnd);
}
/*---------------------------------------------------------------------------*/
int vipOnRegCmd (wnd *vp, int kcode)
{
  int rc =  E_OK; comdesc *cp; wpos_off(vp);
  KbCode = kcode;
  while (KbCode) {
    if (Lwnd) {
      if ((cp = Ldecode(KbCode)) == NULL) { ExitLEmode(E_NOCOM); continue; }
      switch (rc = LeCommand(cp)) {
      case E_UP:    ExitLEmode(E_UP); // and FALL THROUGH
      case E_LEXIT: continue;
      case E_OK:   break;
      default: vipBell();
      }            break;
    }
    else if ((cp = Tdecode(KbCode)) != NULL) rc = TeCommand(cp);
    else if ((cp = Ldecode(KbCode)) == NULL) rc = TmCommand(KbCode);
    else {   
      EnterLEmode(E_NOCOM); continue;
    }
    switch (rc) {
    case E_DOWN: EnterLEmode(E_DOWN); continue;
    case E_LENTER: rc = E_OK;  break;
    case E_OK:                 break;
    default: vipBell();
    }                       
    KbCode  = 0;
  } KbCount = 1;
    KbRadix = 0; return rc;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
inline int vip1sixth (wnd *vp)  /* calculate medium jump size (1/6th of wsh) */
{
  int N = (vp->wsh + 2) / 6; return (N > 0) ? N : 1;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef notdef
static int vipWinScroll (wnd *vp, int dx, int dy)
{
  wxmin = 0; wxmax = vp->wsw; if (dx < 0 && vp->wtx+dx < 0) dx = -vp->wtx;
  wymin = 0; wymax = vp->wsh; if (dy < 0 && vp->wty+dy < 0) dy = -vp->wty;
  vp->wtx += dx;   wdx = -dx;
  vp->wty += dy;   wdy = -dy; if (dx || dy) wroll(vp);        return E_OK;
}
#endif
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int vipCmdScroll (wnd *vp, int kcode, int dx, int dy)
{
  int rc  = vipOnRegCmd(vp, kcode);
  if (rc != E_OK) return rc;
  wxmin = 0; wxmax = vp->wsw; if (dx < 0 && vp->wtx+dx < 0) dx = -vp->wtx;
  wymin = 0; wymax = vp->wsh; if (dy < 0 && vp->wty+dy < 0) dy = -vp->wty;
  vp->wtx += dx;   wdx = -dx;
  vp->wty += dy;   wdy = -dy; if (dx || dy) wroll(vp);        return E_OK;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipOnTwxCmd (wnd *vp, int kcode)
{
#ifdef notdef
  switch (kcode) {
  case TW_SCROLUP: return vipWinScroll(vp, 0, -KbCount);
  case TW_SCROLDN: return vipWinScroll(vp, 0,  KbCount);
  case TW_SCROLLF: return vipWinScroll(vp, -KbCount, 0);
  case TW_SCROLRG: return vipWinScroll(vp,  KbCount, 0);
  }
#endif
  switch (kcode) {
  case TW_SCROLLF: return vipCmdScroll(vp, LE_LEFT, -KbCount, 0);
  case TW_SCROLRG: return vipCmdScroll(vp, LE_RIGHT, KbCount, 0);
  }
  if (leARGmode) return vipOnRegCmd(vp, kcode); // no vert.scroll in leARGmode
  switch (kcode) {
  case TW_SCROLUPN: KbCount = vip1sixth(vp); kcode = TE_UP;   break;
  case TW_SCROLDNN: KbCount = vip1sixth(vp); kcode = TE_DOWN; break;
  case TE_PPAGE:    KbCount = vp->wsh-1;     kcode = TE_UP;   break;
  case TE_NPAGE:    KbCount = vp->wsh-1;     kcode = TE_DOWN; break;
  case TW_SCROLUP:                           kcode = TE_UP;   break;
  case TW_SCROLDN:                           kcode = TE_DOWN; break;
  case TW_DOWN:
              KbCount = vip1sixth(vp);  return vipOnRegCmd(vp, TE_DOWN);
  case TW_UP: KbCount = vip1sixth(vp);  return vipOnRegCmd(vp, TE_UP);
  default:                              return vipOnRegCmd(vp, kcode);
  }
  return vipCmdScroll(vp, kcode, 0, (kcode==TE_UP) ? -KbCount : KbCount);
}
/*---------------------------------------------------------------------------*/
struct KeyTuple { int ev, ca, count, radix; };
KeyTuple *pMacro,      macroBuf[TK_SMX-TK_SM0][MAXTXRM];
int enteringMacro = 0, macroLen[TK_SMX-TK_SM0] ={ 0,0 };
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int vipExecuteMacro (wnd *vp, int N)
{
  int count = KbCount, rc;
  while (count--) {
    KeyTuple *pM = macroBuf[N];
    int       nM = macroLen[N];
    while (nM-- && !qkbhin()) {
      int ev = pM->ev, ca = pM->ca; KbCount = pM->count;
                                    KbRadix = pM->radix;
      pM++;
      if ((rc = vipOnMimCmd(vp, ev, ca)) != E_OK) return   rc;
  } }                                             return E_OK;
} 
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipOnMimCmd (wnd *vp, int ev, int ca) // temp block converted to permanent
{                                         // by F5 and cleared by keys without
  if (BlockMark) {                        // KxBLK/TMP/SEL bits (TODO: fix bit)
    if (BlockTemp) { 
      if (ev == TX_MCBLOCK) { BlockTemp = FALSE;
                              BlockMark_repaint(Twnd); return E_OK;
      }
      else if (!(ca & KxTMP)) scblkoff();
    }
    else { if (!(ca & KxBLK)) scblkoff();
           if (  ev == TX_MCBLOCK) return E_OK; }
  }
  else { if (ca &  KxTS)         scblkon(TRUE);
    else if (ev == TX_MCBLOCK) { scblkon(FALSE); return E_OK; }
  }
  if (TK_EM0 <= ev && ev <= TK_EMX) { int N = ev - TK_EM0;
    if (macroLen[N] > 0) return vipExecuteMacro(vp, N);
    else { vipBell();    return E_FINISHED; }
  }
  return vipOnTwxCmd(vp, ev);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipOnKeyCode (wnd *vp, int ev, int ca)
{
       if (ev == TK_LINFO) vp->sctw->info.updateInfo(MitLINE_BLOCK);
  else if (ev == TK_CHARK) vp->sctw->info.updateInfo(MitCHARK);
  else if (ev == TW_CDOWN) { wpos_off(vp);
    if (Lwnd)  ExitLEmode (E_NOCOM); // set Ty to the last line in the window,
    tesetxy (Tx, vp->wty+vp->wsh-1); // to avoid scrolling to the empty screen
    vp->sctw->repeatCmd(TW_SCROLDN); // in case when started with cursor on top
  }
  else if (ev == TW_CUP) vp->sctw->repeatCmd(TW_SCROLUP);
  else {
    int N;
    if (enteringMacro) { N = enteringMacro - TK_EM0;
      if ((TK_SM0 <= ev && ev <= TK_SMX) ||
                           ev == enteringMacro) enteringMacro = 0;
      else {
        pMacro->ev = ev; pMacro->count = KbCount;
        pMacro->ca = ca; pMacro->radix = KbRadix;
        pMacro++;
        if (++macroLen[N] >= MAXTXRM) { vipBell(); enteringMacro = 0; }
        else vipOnMimCmd(vp, ev, ca);
    } }
    else if (TK_SM0 <= ev && ev <= TK_SMX) { N = ev - TK_SM0;
      macroLen[N] = 0;
      pMacro = macroBuf[N]; enteringMacro = TK_EM0 + N; 
    }
    else vipOnMimCmd(vp, ev, ca);
} }
/*---------------------------------------------------------------------------*/
static QRegExp Qspre; /* Qt presentation of search pattern (regexp/wildcard) */
static QString Qsstr; /* Qt search string                                    */
static QString Repst; /* Qt replace string (may include \1,\2,.. for regexp) */
static int find_Mode; /* copy of the leOptMode (can be changed w/o re-parse) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipPrepareSearch()             /* prepare Qspre/Qsstr for the vipFind() */
{
  find_Mode = leOptMode; if (rpa) Repst = tcs2qstr(rpa, rpl);
  if (find_Mode & LeARG_STANDARD) Qsstr = tcs2qstr(spa, spl);
  else                           
    Qspre = QRegExp(tcs2qstr(spa, spl), 
      (find_Mode & LeARG_IGNORECASE) ? Qt::CaseInsensitive : Qt::CaseSensitive,
      (find_Mode & LeARG_WILDCARD)   ? QRegExp::Wildcard   : QRegExp::RegExp);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipFind (tchar *str, int st_len, int from_pos, BOOL backward)
{
  QString line = tcs2qstr(str, st_len);
  Qt::CaseSensitivity cs = (find_Mode & LeARG_IGNORECASE) ? Qt::CaseInsensitive
                                                          : Qt::CaseSensitive;
  if (backward) {
    int from = my_min(from_pos-st_len, 0); // negative, from end of the string
    if (find_Mode & LeARG_STANDARD) 
         return line.lastIndexOf(Qsstr, from-1, cs);
    else return line.lastIndexOf(Qspre, from-1);
  }
  else {
    if (find_Mode & LeARG_STANDARD) return line.indexOf(Qsstr, from_pos, cs);
    else                            return line.indexOf(Qspre, from_pos);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipFindReplace (tchar *str, int st_len, int at_pos,  /* "find a replace" */
                    tchar *out, int *st_x0, int *st_x1)
{
  QString line = tcs2qstr(str + at_pos, st_len - at_pos);
  Qt::CaseSensitivity cs = (find_Mode & LeARG_IGNORECASE) ? Qt::CaseInsensitive
                                                          : Qt::CaseSensitive;
  if (find_Mode & LeARG_STANDARD) {
    if (line.startsWith(Qsstr, cs)) { // Unfortunately, cannot use replace() as
      int len = qstr2tcs(Repst, out); // it replaces every occurence, we need 1
      *st_x0 = at_pos;
      *st_x1 = at_pos + Qsstr.length(); return len;
    }                                   return  -1;
  } 
// It is even more tricky for regexp replace, because of \1,\2,.. substitutions
// (we cannot easily predict the length of the result), so have to obtain total
// match first and then do the replace() on it:
  else {
    int mpos = Qspre.indexIn(line); if (mpos != 0) return -1;
    QString orig = Qspre.cap(0);
    *st_x0 = at_pos;
    *st_x1 = at_pos + orig.length();
    return  qstr2tcs (orig.replace(Qspre, Repst), out);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipConvert (tchar *str, int str_len, int cvt_type, tchar *out)
{
  QString line = tcs2qstr(str, str_len); bool ok; int radix;
  long N;
  switch (cvt_type) {
  case cvTO_UPPER: return qstr2tcs(line.toUpper(), out);
  case cvTO_LOWER: return qstr2tcs(line.toLower(), out);
  case cvTO_DECIMAL:
    if (KbRadix && 2 <= KbCount && KbCount <= 36) radix = KbCount;
    else 
         if (line.startsWith("0x")) { radix = 16;line = line.remove(0, 2); }
    else if (line.startsWith("0b"))   radix =  2;
    else if (line.startsWith('0'))    radix =  8;
    else                              radix = 16;
    N = line.toLong(&ok, radix);
    if (ok) return qstr2tcs(QString::number(N, 10), out);
    else    return -1;
  case cvTO_RADIX:
    radix = (KbRadix && 2 <= KbCount && KbCount <= 36) ? KbCount : 16;
    N = line.toLong(&ok);
    if (ok) return qstr2tcs(QString::number(N, radix), out);
    else    return -1;
  }         return -1; // unknown conversion type
}
/*---------------------------------------------------------------------------*/
#define MAX_KEY_QUEUE 64
BOOL vipOSmode;
static int keyQueue[MAX_KEY_QUEUE], *kqHead = keyQueue, *kqTail = keyQueue;
static quint64 last_kbhin = 0;

void setkbhin(int key) 
{ 
  last_kbhin = pgtime(); 
  if (key) {
    if (kqTail == keyQueue+MAX_KEY_QUEUE) {
      int len = kqTail - kqHead;
      if (kqHead == keyQueue) { vipBell(); return; }
      blkmov(kqHead, keyQueue, len*sizeof(int));
      kqHead = keyQueue;
      kqTail = keyQueue+len;
    }
    *kqTail++ = key;
} }
int   kbhin(void) { return (kqHead < kqTail) ? *kqHead++ : 0; }
BOOL qkbhin(void)
{
  return (kqHead < kqTail) || (pgtime() - last_kbhin) > 32768;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void EnterOSmode()
{
  Twnd->sctw->setWindowModality(Qt::ApplicationModal);
  kqHead = kqTail = keyQueue;        vipOSmode = TRUE;
}
void ExitOSmode()
{
  Twnd->sctw->setWindowModality  (Qt::NonModal);
  kqHead = kqTail = keyQueue; vipOSmode = FALSE;
}
void vipYield() { QCoreApplication::processEvents(); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipError (const char *msg) { vipError(QString::fromUtf8(msg)); }
void vipError (QString msg)
{
  QMessageBox::warning(NULL, QString("µMup07 error"), msg);
}
void vipBell()                                { QApplication::beep();       }
void vipTrace1(const char *fmt, int arg)      { fprintf(stderr, fmt,  arg); }
void vipTrace2(const char *fmt, int x, int y) { fprintf(stderr, fmt, x, y); }
/*---------------------------------------------------------------------------*/
QString vipQstrX (QString str)
{
  QString Xstr; Xstr.sprintf("[%d]'", str.length());
  for (int i = 0; i < str.length(); i++)
    if (str.at(i).unicode() < ' ') 
         Xstr += QString("^")+QChar(str.at(i).unicode()+'@');
    else Xstr += str.at(i);          return Xstr+QChar('\'');
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
QString tcs2qstr (tchar *tcs, int len)
{
  QChar tbuf_tmp[MAXLPAC];
  for (int i = 0; i < len; i++) tbuf_tmp[i] = int(tcs[i] & AT_CHAR);
  return QString(tbuf_tmp, len);
}
int qstr2tcs (QString qstr, tchar *buffer, tchar attr)
{
  int len = my_min(qstr.length(), MAXLPAC-1);
  for (int i = 0; i < len; i++) buffer[i] = (tchar)qstr.at(i).unicode()+attr;
  return len;
}
/*---------------------------------------------------------------------------*/

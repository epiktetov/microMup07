//------------------------------------------------------+----------------------
// МикроМир07 ViewPort (interface Qt/C++ • legacy code) | (c) Epi MG, 2006-2023
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
#include "macs.h"
#include "synt.h"
extern "C" {
#include "le.h"
#include "te.h"
}
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
bool BlockMark = FALSE; QRect BlockMarkRect;   wnd *windows = NULL;
bool BlockTemp = FALSE;
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
  w->wspace = 0;
  MiScTwin *baset = wbase ? wbase->sctw : NULL;
  MiFrame  *basef = baset ? baset->mf   : NULL;
  MiFrame  *frame;
  if (kcode == TM_VFORK) {
    if (wbase && wbase->wsibling) w->wsh += wbase->wsibling->wsh;
    w->wsibling = NULL;                           w->sib_pos = 0;
    frame = new MiFrame(basef);
    frame->NewScTwin(w, baset); frame->show();
  }
  else if (wbase) {
    w->wsibling = wbase; wbase->wsh -= w->wsh;   wbase->sib_pos = 0;
    wbase->wsibling = w; basef->NewScTwin(w, baset); w->sib_pos = 1;
  }
  else { free(w);  // incorrect function call - for TE_HFORK
         w = NULL; // case the base window must be specified
  }
  if (wbase) vipFocusOff(wbase); return w;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd *vipSplitWindow (wnd *wbase, int kcode)   /* TM_VFORK creates new window */
{
  if (kcode == TM_VFORK) return vipNewWindow(wbase, wbase->wsw,
                                                    wbase->wsh, kcode);
  else if (wbase->wsibling) return NULL;
  else {
    int h = wbase->wsh / 2; // ... and TE_HFORK splits existing one
    if (h < 3) return NULL;
    wnd *wind = vipNewWindow (wbase, wbase->wsw, h, kcode);
    if (wind) vipRedrawWindow(wbase);          return wind;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool vipFreeWindow (wnd *vp) /* returns true if last window has been deleted */
{
  wnd *sibling = vp->wsibling;               // if the window had a sibling,
  if (sibling) { sibling->wsh  += vp->wsh;   // update its height to adjust
                 sibling->wsibling = NULL; } // and unlink from this window
  MiFrame *frame = vp->sctw->mf;
  frame->DeleteScTwin(vp->sctw); // deleting MiScTwin triggers vipCleanupWin()
  if (sibling) {
    vipActivate(sibling);                  // activate the sibling window, and
    frame->updateWinTitle(); return false; // update frame title to reflect it
  }
  else { delete frame; return (windows == NULL); } // returns true if was last
}                                                  // window, twExit will exit
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipCleanupWindow (wnd *vp)     /* called when MiScTwin deleted from GUI */
{                                   /* should cleanup Twnd  (and wnd struct) */
  if (Twnd == vp) Twnd = NULL;
  if (vp->wsibling)  vp->wsibling->wsibling = NULL;
  L2_DELETE(windows, vp, wdprev, wdnext); free(vp);
}
/*---------------------------------------------------------------------------*/
inline void vipGetPosition (wnd *vp, int& x, int& y)
{
  x = vp->sctw->mf->x();
  y = vp->sctw->mf->y();
  if (vp->sib_pos && vp->wsibling) y += vp->sctw->Th2qtH(vp->wsh);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
wnd *vipFindWindow (wnd *wbase, int kcode) /* rel.direction TM_U/L/D/RWINDOW */
{
  if (wbase->wsibling) {
    if ((kcode == TM_DWINDOW && wbase->sib_pos == 0)
     || (kcode == TM_UWINDOW && wbase->sib_pos == 1)) return wbase->wsibling;
  }
  int dx = 0, dy = 0, xb, yb, x, y, xt, yt, delta;
  wnd *w, *wt = NULL;        xt = yt = 2147483647; /* infinity */
  macs_update_all_wspaces();
  vipGetPosition(wbase, xb, yb);
  switch (kcode) {
  case TM_UWINDOW: dx =  0; dy = -1; break;
  case TM_DWINDOW: dx =  0; dy = +1; break;
  case TM_LWINDOW: dx = -1; dy =  0; break;
  case TM_RWINDOW: dx = +1; dy =  0; break;
  }
  for (w = windows; w != NULL; w = w->wdnext) {
    if (w->wspace != wbase->wspace) continue;
    vipGetPosition(w, x, y);
    if (dx && (delta = dx*(x - xb)) > 0 && delta < xt) { wt = w; xt = delta; }
    if (dy && (delta = dy*(y - yb)) > 0 && delta < yt) { wt = w; yt = delta; }
  }
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
// fprintf(stderr, "  repaint(x=%d:%d,y=%d:%d,H=%d)\n",
//                              x0,x1,  y0,y1,  y1-y0+1);
//+
  int width = my_min(x1-x0+1, MAXLPAC);
  QRect cline;
  for (int y = y0; y <= y1; y++) {         int len;
    tchar *pt_line = TxInfo(vp, y + vp->wty, &len);  // get the text line...
    if (vp->wtext && vp->wtext->clang)               // colorize if required
             SyntColorize(vp->wtext, pt_line, len);  // & discard out-of-win
    pt_line += vp->wtx;                              //     part (before wtx)
        len -= vp->wtx; cline = QRect(x0,y,width,1);
//
// NOTE: TxInfo returns len of the filled up buffer, but the buffer itself is
// larger - tcbuf[MAXLPAC+MAXTXRM] + it's discardable, so no problem to write
// past the end-of-pt_line (we only use len because Erase is faster than Text)
//
         if (len < x0) len = x0;   // Draw BlockMark and cursor into buffer
    else if (len > x1) len = x1+1; // (block first, as area may contain both,
    bool spoiled = false;          //                    keeping cursor color)
    if (BlockMark && cline.intersects(BlockMarkRect)) {
      tchar attr = BlockTemp ? AT_BG_RED : AT_BG_BLU;
      int xfrom = my_max(x0, BlockMarkRect.left()),
           xend = my_min(x1, BlockMarkRect.right());
      if (len <= xend) len = xend+1;
      for (int x = xfrom; x <= xend; x++) pt_line[x] |= attr;  spoiled = true;
    }
    if (cline.contains(vp->cx, vp->cy)) {  // strip all attrs from under cursor
      if (len <= vp->cx) len = vp->cx+1;   //
      pt_line[vp->cx] = (pt_line[vp->cx] & AT_CHAR) | vp->ctc; spoiled = true;
    }
    if (len <= x1) sctw->Erase(dc, len, y,           x1-len+1);
    if (len  > x0) sctw->Text (dc,  x0, y, pt_line+x0, len-x0);
    if (spoiled)
      blktspac(pt_line+x0, x1-x0+1); // if spoiled (by cursor or BlockMark),
} }                                  // cleanup everything to avoid ghosts
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// MiScTwin::Repaint does not call vipRepaint directly, but instead "updates"
// (invalidates) specified region, so Qt will schedule the repaint for that
//
void vipRedraw (wnd *vp, int tx, int ty, int width, int height)
{
  vp->sctw->Repaint(tx, ty, width, height); // repaint the text area...
  vp->sctw->RepaintVpPB();                  // ...and VP position bar aws well
}
void vipRedrawWindow(wnd *vp)         { vipRedraw(vp, 0,0, vp->wsw, vp->wsh); }
void vipRedrawLine  (wnd *vp, int ty) { vp->sctw->Repaint(0, ty, vp->wsw, 1); }
/*---------------------------------------------------------------------------*/
void wpos (wnd *vp, int x, int y)        /* repaint cursor at given position */
{
  txt *t = vp->wtext;
  if (t) { if (t->txredit == TXED_YES) {
             if (t->txstat & TS_CHANGED) vp->ctc = AT_INVERT | AT_BG_BLU;
             else                        vp->ctc = AT_INVERT | AT_BG_GRN;
           } else                        vp->ctc = AT_INVERT | AT_BG_RED;
           if (LeInsMode)                vp->ctc             |= AT_SUPER;
  }
  vp->cx = x;
  vp->cy = y;                     vp->sctw->Repaint(x,y, 1,1);
  if (vp->sctw->info.isVisible()) vp->sctw->info.updateInfo();
}
void wpos_off (wnd *vp) //- - - - - - - - - - - - - - - - - - - - - - - - - - -
{
  int ocx = vp->cx; vp->cx = -1;
  int ocy = vp->cy; vp->cy = -1; vp->sctw->Repaint(ocx, ocy, 1,1);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void BlockMark_repaint (wnd *vp)
{
  vp->sctw->Repaint(BlockMarkRect.left(),  BlockMarkRect.top(),
                    BlockMarkRect.width(), BlockMarkRect.height());
}
void scblkon (bool isTemp) { BlockMark = TRUE;   BlockTx = Tx;
                             BlockTemp = isTemp; BlockTy = Ty; }
void scblkoff() 
{ 
  if (BlockMark) { BlockMark = FALSE; BlockMark_repaint       (Twnd);
                                      BlockMarkRect = QRect(0,0,0,0); }
}
/*---------------------------------------------------------------------------*/
void vipRoll (wnd *vp, int wymin, int wymax, int dy) /* rolls region up/down */
{
  int rollH = wymax - wymin - my_abs(dy);        vp->sctw->RepaintVpPB();
  if (rollH < 1)     vp->sctw->Repaint(0, wymin, vp->wsw, wymax - wymin);
  else if (dy > 0) { vp->sctw->Scroll (0, wymin, vp->wsw, rollH,     dy);
                     vp->sctw->Repaint(0, wymin, vp->wsw,            dy); }
  else {
    vp->sctw->Scroll (0, wymin-dy, vp->wsw, rollH, dy);
    vp->sctw->Repaint(0, wymax+dy, vp->wsw,       -dy);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Переизобразим текст после модификации на всех прикрепленных к нему окнах
//
void wndop (short op, txt *text)
{
  int y = text->txy, dy;
  for (wnd *vp = text->txwndptr; vp; vp = vp->wnext) {
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    switch (op) {                                    // Simple redraw operation
    case TW_EM: vp->wty = 0;
                vp->wtx = 0; // and FALL THROUGH
    case TW_ALL:
      vipRedrawWindow(vp); continue;
    case TW_RP:
      if (vp->wty <= y  &&   y < vp->wty + vp->wsh)
        vp->sctw->Repaint(0, y - vp->wty,  vp->wsw, 1); // current line
      continue;
    case TW_DWN:
      if (y < vp->wty + vp->wsh) {         // all lines down from current one
        int y1  =  my_max(0, y - vp->wty); //
        vp->sctw->Repaint(0, y1, vp->wsw, vp->wsh - y1);
      }
      continue;
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case TW_IL: dy = +1; break;                  // Rolling some text up / down
    case TW_DL: dy = -1; break;                  // (may be faster than redraw)
    default:          continue;
    }
         if (y <  vp->wty)  vp->wty += dy;
    else if (y >= vp->wty + vp->wsh); // nothing to do
    else
      vipRoll(vp, y - vp->wty, vp->wsh, dy);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void wadjust (wnd *vp, int x, int y) // "Натянуть" окно на точку (x,y) в тексте
{
  int ddx = x - vp->wtx; int moved = 0; // +1: can roll, -1: cannot
  int ddy = y - vp->wty;
  if (ddy < 0 || (ddy -= vp->wsh-1) > 0) { vp->wty += ddy; moved = +1; }
  if (ddx < 0 || (ddx -= vp->wsw-1) > 0) { vp->wtx += ddx; moved = -1; }
  if (BlockMark) {
    int x0 = my_min(x - vp->wtx, BlockTx - vp->wtx),
        x1 = my_max(x - vp->wtx, BlockTx - vp->wtx),
        y0 = my_min(y - vp->wty, BlockTy - vp->wty),
        y1 = my_max(y - vp->wty, BlockTy - vp->wty);
    if (x0 < 0) x0 = 0; if (x1 > vp->wsw) x1 = vp->wsw;
    if (y0 < 0) y0 = 0; if (y1 > vp->wsh) y1 = vp->wsh;
    QRect block(x0, y0, x1-x0+1, y1-y0+1);
    if (BlockMarkRect != block) { BlockMark_repaint(vp);
        BlockMarkRect =  block;   BlockMark_repaint(vp); }
  }
  if (moved) {
    if (moved > 0 && my_abs(ddy) < vp->wsh-1) vipRoll(vp, 0, vp->wsh, -ddy);
    else vipRedrawWindow(vp);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipReady() 
{                 
       if (Lwnd) { wadjust(Lwnd,Lx,Ly); wpos(Lwnd,Lx-Lwnd->wtx,Ly-Lwnd->wty); }
  else if (Twnd) { wadjust(Twnd,Tx,Ty); wpos(Twnd,Tx-Twnd->wtx,Ty-Twnd->wty);
//
    if (Ttxt == NIL || Twnd->sctw == NIL) return;
    for (int i = 1; i < TXT_MARKS; i++) {
      if (Ttxt->txmarky[i] == Ty && Ttxt->txmarks[i]) {
        Twnd->sctw->diag.update    (Ttxt->txmarks[i], Ttxt->txmarkt[i]);
        Twnd->sctw->diag.show();                                 return;
    } } Twnd->sctw->diag.hide(); //
} }                              // hide mark window if line does not match
/*---------------------------------------------------------------------------*/
void vipOnFocus (wnd *vp) /* if possible, goto window (cursor is not Ok yet) */
{ 
  if (vp != Twnd && Ttxt && vp->wtext) vipActivate(vp); vipReady();
}
void vipFocusOff (wnd *vp) // save current text position (and sync with Ttxt)
{                          // before un-focusing active editor window
  if (Lwnd) ExitLEmode();  //
  if (vp == Twnd) { Twnd->ctx = Tx;
                    Twnd->cty = Ty; if (Ttxt) wupdate(Ttxt, Twnd); }
  scblkoff();
  wpos_off(vp); vp->sctw->diag.hide();
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipActivate (wnd *vp)    // save current cursor position and sync active
{                             //  Ttxt and Twnd before activating new window
  if (Twnd) { Twnd->ctx = Tx; //
              Twnd->cty = Ty; if (Ttxt && Ttxt == Twnd->wtext)
                                  wupdate(Ttxt,   Twnd);     }
  if (tmLoad(vp->wtext)) {
    Ttxt = vp->wtext; Tx = vp->ctx;
    if (TxSetY (Ttxt, Ty = vp->cty) == FALSE) Ty = Ttxt->txy;
    Twnd = vp; //
} }            // no vipFocus(Twnd) here!
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipGotoXY (int x, int y)           /* click in the active window (Twnd) */
{
  vipFocusOff(Twnd); tesetxy((short)(Twnd->wtx+x), Twnd->wty+y);
  vipOnFocus (Twnd);
}
/*---------------------------------------------------------------------------*/
void vipOnKeyCode (wnd *vp, int ev)   /* called from MkMimXEQ (file ccd.cpp) */
{
       if (ev == TK_LINFO) vp->sctw->info.updateInfo(MitLINE_BLOCK);
  else if (ev == TK_PINFO) vp->sctw->info.updateInfo(MitLINE_POSXY);
  else if (ev == TK_CHARK) vp->sctw->info.updateInfo(MitCHARK);
  else if (ev == TK_KBKEY) vp->sctw->info.updateInfo(MitKBKEY);
  else if (ev == TW_CDOWN) { wpos_off(vp);
    if (Lwnd) ExitLEmode();          // set Ty to the last line in the window,
    tesetxy (Tx, vp->wty+vp->wsh-1); // to avoid scrolling into an empty screen
    vp->sctw->repeatCmd(TW_SCROLDN); // in case when started with cursor on top
  }
  else if (ev == TW_CUP) { wpos_off(vp); // similar things for scrolling up...
    if (Lwnd) ExitLEmode();              //
    tesetxy (Tx,  vp->wty); vp->sctw->repeatCmd(TW_SCROLUP);
  }
  else vipOnMimCmd(vp, ev);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
inline int vip1sixth (wnd *vp)  /* calculate medium jump size (1/6th of wsh) */
{
  int N = vp->wsh / 6; return (N > 0) ? N : 1;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int vipCmdScroll (wnd *vp, int kcode, int dx, int dy)
{
  int rc  = vipOnRegCmd(vp, kcode);
  if (rc != E_OK) return rc;
  if (dx < 0 && vp->wtx+dx < 0) dx = -vp->wtx; vp->wtx += dx;
  if (dy < 0 && vp->wty+dy < 0) dy = -vp->wty; vp->wty += dy;
       if (dx) vipRedrawWindow(vp);
  else if (dy) vipRoll(vp, 0, vp->wsh, -dy);
  else {       vipBell();     return E_SETY; }   return E_OK;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipOnMimCmd (wnd *vp, int ev) /* handling block selections and scrolling */
{
  last_MiCmd_time = pgtime();
  int ca = 0;
  if (Mk_IsCHAR(ev)) ca = KxSEL; // regular character should keep any selection
  else switch (ev & 0xf0000) {           //
  case 0xc0000: ca = 0;          break;  // NOTE: text/line editors do not care
  case 0xd0000: ca =      KxBLK; break;  // about KxTS (same command's used for
  case 0xf0000: ca =      KxSEL; break;  // Left and Shift+Left), so we have to
  case 0xe0000: ca = KxTS|KxSEL;         //   convert the keycode appropriately
    ev = (micom_enum)((ev & ~KxTMP) | KxBLK);
  }
  switch (20*BlockMark+BlockTemp) {
  case 21:
    if (ev == TX_MCBLOCK) { BlockTemp = FALSE;
                            BlockMark_repaint(Twnd); return E_OK; }
    else {                   //    ^
      if (ca & KxTMP) break; // "temporary" block converted to permanent one by
      scblkoff();     break; // TX_MCBLOCK (bound to F5 by default) and cleared
    }                        // by any command without KxBLK/TMP/SEL bits
  case 20:
    if (ca & KxBLK) break; scblkoff(); // NOTE: TX_MCBLOCK by itself should not
    if (ev == TX_MCBLOCK) return E_OK; // have any Kx bits (F5..F5 = clear blk)
    break;
  default: if (ca &  KxTS)         scblkon(TRUE);
      else if (ev == TX_MCBLOCK) { scblkon(FALSE); return E_OK; }
  }
  switch (ev) {
  case TW_SCROLLF: return vipCmdScroll(vp, LE_LEFT, -KbCount, 0);
  case TW_SCROLRG: return vipCmdScroll(vp, LE_RIGHT, KbCount, 0);
  }
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if (leARGmode) return vipOnRegCmd(vp, ev); // no vert.scroll in leARGmode
  int dy;
  switch (ev) {     // NOTE: scoll up/down key codes differs in 2nd LSB:
  case TW_SCROLUPN: //                   ..1. (0xd00fa or 0xd1413) scroll up
  case TW_SCROLDNN: //                   ..0. (0xd00fd or 0xd1415) scroll down
    KbCount = vip1sixth(vp);
  case TW_SCROLUP:
  case TW_SCROLDN:
    if (ev & 2) { ev = TE_UP;   dy = -KbCount; }
    else        { ev = TE_DOWN; dy =  KbCount; }
    return vipCmdScroll(vp, ev, 0, dy);
  case TW_UP:
    dy = vip1sixth(vp);
    if (vp->wty < dy) { vp->sctw->repeatCmd(TE_UP,dy); return E_OK; }
    else { KbCount = dy; return vipOnRegCmd(vp,TE_UP);              }
  case TW_DOWN:
    dy = vip1sixth(vp);
    if (vp->wty+dy > vp->wsh) { vp->sctw->repeatCmd(TE_DOWN,dy); return E_OK; }
    else         { KbCount = dy; return vipOnRegCmd(vp,TE_DOWN);              }
  case TE_PPAGE:
  case TE_NPAGE:
    dy = vip1sixth(vp);
    if (dy < 2)             ev = (ev & 1) ? TW_SCROLDN  : TW_SCROLUP;
    else { dy = vp->wsh/dy; ev = (ev & 1) ? TW_SCROLDNN : TW_SCROLUPN; }
    vp->sctw->repeatCmd(ev, dy);
    return E_OK;
  default: return vipOnRegCmd(vp, ev);
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipOnRegCmd (wnd *vp, int kcode)                     /* regular commands */
{
  int rc =  E_OK; comdesc *cp; wpos_off(vp);
  KbCode = kcode;
  while (KbCode) {
    if (Lwnd) {
      if ((cp = Ldecode(KbCode)) == NULL) { ExitLEmode(); continue; }
      switch (rc = LeCommand(cp)) {
      case E_LEXIT: continue; // <-- re-try the command on TE and/or TM level
      case E_OK:       break;
      case E_KBREAK: vipBell();              break;
      default: vipErrEvent(rc); setkbhin(1); break; // we've done here, break
      }                                      break; // the while(KbCode) loop
    }
    else if ((cp = Tdecode(KbCode)) != NULL) rc = TeCommand(cp);
    else if ((cp = Ldecode(KbCode)) == NULL) rc = TmCommand(KbCode);
    else {                     // ^
      EnterLEmode(); continue; // if this is a LE-level command, enter LE mode
    }
    switch (rc) {                    // TE/TM command return codes:
    case E_LENTER: rc = E_OK; break; // - LenterARG called
    case E_OK:                break; // - everything all right
    case E_KBREAK: vipBell(); break; // - KB break (kbhin already set)
    default: vipErrEvent(rc);        // - some error encountered => place
                 setkbhin(1); break; //       an interript into kbhin queue
    }                       
    KbCode  = 0; // cleanup KbStuff (and force end-of-loop as well)
  } KbCount = 1; //
    KbRadix = 0; return rc; // return code is not really used (vipOnKeyCode
}                           // discards it), used only for "return vipXxxCmd"
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
inline int vipDoFind (QString line, int st_len, int from_pos, bool backward)
{
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
int vipFind (char *str, int st_len, int from_pos, bool backward)
{
  QString line = QString::fromUtf8(str, st_len);   st_len = line.size();
  int pos, i;
  for (pos = i = 0; i < st_len && pos < from_pos; pos++,i++)
    if (line.at(i).unicode() == 9) pos += TABsize - (pos % TABsize) - 1;
  from_pos = i;
// ^
// workaround for TABs (and, below, text attributes) in the search string, fix
// the position difference between packed as-in-file line and unpacked tstring
//
  int X = vipDoFind(line, st_len, from_pos, backward);
  if (X > 0) {
    for (pos = i = 0; i < X; pos++,i++) {
      int t = line.at(i).unicode();
      if ((t & 0x0FFE0) == 0x280) pos--;
      else if (t == 9) pos += TABsize - (pos % TABsize) - 1;
    }                                               X = pos;
  } return X;
}
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
         if (line.startsWith("0x")) { radix = 16; line = line.remove(0, 2); }
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
static int keyQueue[MAX_KEY_QUEUE], *kqHead = keyQueue, *kqTail = keyQueue;
static quint64 last_kbhin = 0;                      bool vipOSmode = false;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void setkbhin (int key)  // KB queue for hardware (vipOSmode) input, also used
{                        // for detecting interrupt/error (key=1) and timeouts
  last_kbhin = pgtime(); // (32 secs without real KB input => something wrong)
  if (key) {
    if (kqTail == keyQueue+MAX_KEY_QUEUE) {
      int len = kqTail - kqHead;
      if (len == MAX_KEY_QUEUE) { QApplication::beep(); return; }
      blkmov(kqHead,
             keyQueue, len * sizeof(int)); kqHead = keyQueue;
                                           kqTail = keyQueue + len;
    } *kqTail++ = key;
  }                                // clear the queue (remove error indicator)
  else kqHead = kqTail = keyQueue; // if key==0 (MiScTwin::keyPress/wheelEvent)
}
int   kbhin(void) { return (kqHead < kqTail) ? *kqHead++ : 0; }
bool qkbhin(void)
{                                         // returns true when:
  return (kqHead < kqTail)                // - queue not empty (error queued)
      || (pgtime() - last_kbhin) > 32000; // - last event more than 32sec ago
}
void vipBell() { QApplication::beep(); }
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
void vipFileTooBigError (qfile *f, long size)
{
  vipError(QString("File %1 is too big (size: %2), truncated")
                         .arg(QfsShortName(f)).arg(size));
}
void vipErrEvent (int /*E_code*/)
{
  vipBell(); // if (Twnd) Twnd->sctw->DisplayInfo(QString("e:%1").arg(E_code));
}
/*---------------------------------------------------------------------------*/
QString vipQstrX (QString str)                /* currently not used anywhere */
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

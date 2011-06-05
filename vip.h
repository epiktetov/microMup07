/*------------------------------------------------------+----------------------
// МикроМир07 ViewPort (interface wx/C++ / legacy code) | (c) Epi MG, 2006-2011
//------------------------------------------------------+--------------------*/
#ifndef VIP_H_INCLUDED  /* Old "nm.h" (c) Attic 1989-90, (c) EpiMG 1998-2001 */
#define VIP_H_INCLUDED  /* old "wd.h" (c) Attic 1989, then (c) EpiMG 1996,98 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef __cplusplus
#ifdef QSTRING_H
  void    vipError (QString msg); // show error (actually Qt::warning) dialog
  QString vipQstrX (QString str); // convert non-printables in the given Qstr
#endif
wnd *vipNewWindow  (wnd *wbase, int sw, int sh, int kcode_XFORK);
wnd *vipSplitWindow(wnd *wbase,                 int kcode_XFORK);
bool vipFreeWindow (wnd *wnd);
wnd *vipFindWindow (wnd *wbase, int kcode_XWINDOW);
void vipUpdateWinTitle(wnd *w);
void vipCleanupWindow (wnd *w);        /* <- called from MiScTwin destructor */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern "C" {
#endif
void vipFocus(wnd *w);
void vipOnFocus (wnd *w);     /* to be called when the window gets focus     */
void vipFocusOff(wnd *w);     /* to be called when focus goes off the window */
void vipActivate(wnd *w);     /* make this window current (set Ttxt/Twnd)    */
void vipGotoXY(int x, int y); /*        click in the active window (Twnd)    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern BOOL LeInsMode;  /* Режим вставки (если FALSE, то режим замены), le.c */
extern BOOL BlockMark;  /* block mark is on, with BlockMarkRect coordinates  */
void scblkon(BOOL tmp);
void scblkoff();
extern int BlockTx, BlockTy; /* block 1st corner (another one is at cursor)  */
extern BOOL BlockTemp;       /* block is "temporary"                         */
extern wnd *windows;  /* <-- used in tm.c to loop through all active windows */
#ifdef MIM_H_INCLUDED /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* 
 * Repaint client area in text coordinates (inclusive, i.e. y0=y1 => one line):
 */
void vipRepaint(wnd *vp, QPainter& dc, MiScTwin *sctw, int x0, int x1,
                                                       int y0, int y1);
void TxRecalcMaxTy(txt *t);
extern QRect BlockMarkRect;
#endif
/* namoMir was initially developed for very slow devices, special optimization
 * was deployed in TxIL/TxDL/TxRep to redraw only portion that really changed:
 */
extern void wndop (short op, txt *t); /* Показать изменение текста:     */
#define TW_EM  0                      /* - text empty                   */
#define TW_IL  1                      /* - insert line                  */
#define TW_DL  2                      /* - delete line                  */
#define TW_RP  3                      /* - replace current line         */
#define TW_DWN 4                      /* - redraw down the current line */
#define TW_ALL 5                      /* redraw all (too many changes)  */

extern void wadjust (wnd *vp, int tx, int ty);
void vipRedrawLine  (wnd *vp,         int ty);
void vipRedrawWindow(wnd *vp);
void vipRedraw      (wnd *vp, int tx, int ty, int width, int height);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipReady(), vipBell();
BOOL qkbhin();
int  kbhin();
void setkbhin(int kc); /* called on real keyboard events, resets kbhin timer */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern BOOL vipOSmode; /* working in OS mode: current Twnd is modal and keys */
void EnterOSmode();    /* are converted back to raw Ascii, used only on UNIX */
void ExitOSmode(), vipYield();
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipOnKeyCode(wnd *w, int ev, int ca); /* macro entering (TK_SMn,TW_CUP) */
int  vipOnMimCmd (wnd *w, int ev, int ca); /* - TK_EMn and block selections  */
int  vipOnTwxCmd (wnd *w, int kcode);      /* - TW_xx commands               */
int  vipOnRegCmd (wnd *w, int kcode); /* process other (LE/TE/TM_x) commands */
void EnterLEmode(void);
void ExitLEmode(void);
extern int leARGmode;     /* Argument Enter mode (1 = first char, > 1 later) */
#ifdef CCD_H_INCLUDED
comdesc *Ldecode(int kcode); int LeCommand(comdesc *cp);  /* line editor cmd */
comdesc *Tdecode(int kcode); int TeCommand(comdesc *cp);  /* text editor cmd */
                             int TmCommand(int   kcode);
#endif /*--------------------------------------------------------------------*/
extern wnd * Lwnd; /* Окно, в котором редактируется строка (used in le.c)    */
extern wnd * Twnd; /* Окно, в котором редактируется текст (used in te.c)     */
extern txt * Ttxt; /* Редактируемый текст                                    */
extern long  Ty;   /* Y курсора в тексте      (real programmers use global   */
extern short Tx;   /* X курсора в тексте      vars for most important stuff) */
/*
 * Positioning in text (returns FALSE if could not reach specified line) and
 * principal method of getting text info (used for window re-paint):
 */
BOOL   TxSetY(txt *t, long y);
tchar *TxInfo(wnd *w, long y, int *len);                  /* defined in tx.c */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void vipPrepareSearch(); /* using patterns prepared by tesParse() - see te.h */
int  vipFind(char  *str,
             int st_len, int from_pos, BOOL backward);
/*
 *  Next function verifies that 'str' matches previously set pattern at given
 *  position and provides the replacement string (and also positions of match
 *  in the original string):
 */
int vipFindReplace (tchar *str, int st_len, int at_pos,  /* return 'out' len */
                    tchar *out, int *st_x0, int *st_x1); /* or -1 on failure */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int vipConvert (tchar *str, int str_len, int cvt_type, tchar *out_buf);
#define cvTO_UPPER    1 /* converting 'str' to upper case |                  */
#define cvTO_LOWER    2 /*                  to lower case | result placed to */
#define cvTO_DECIMAL 10 /* number to decimal              | 'out_buf' length */
#define cvTO_RADIX   16 /* number to any base (default hex)         returned */
/*---------------------------------------------------------------------------*/
void vipError (const char *message);                              /* vip.cpp */
#ifdef QFS_H_INCLUDED
  void vipFileTooBigError(qfile *f, long size);
#endif
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif                                                     /* VIP_H_INCLUDED */

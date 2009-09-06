//------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 1998-2007
//------------------------------------------------------+----------------------
#include <Qt>
#include "mic.h"
#include "ccd.h"
//+
#include <stdio.h>
//-
#ifdef Q_OS_MAC
#  define Mk_UP    (Qt::Key_Up    | Qt::KeypadModifier)
#  define Mk_DOWN  (Qt::Key_Down  | Qt::KeypadModifier)
#  define Mk_LEFT  (Qt::Key_Left  | Qt::KeypadModifier)
#  define Mk_RIGHT (Qt::Key_Right | Qt::KeypadModifier)
#else
#  define Mk_UP     Qt::Key_Up
#  define Mk_DOWN   Qt::Key_Down
#  define Mk_LEFT   Qt::Key_Left
#  define Mk_RIGHT  Qt::Key_Right
#endif
#define Mk_HOME     Qt::Key_Home
#define Mk_END      Qt::Key_End
#define Mk_PAGEUP   Qt::Key_PageUp
#define Mk_PAGEDOWN Qt::Key_PageDown

#define Mk_PAD_PLUS  (Qt::KeypadModifier | Qt::Key_Plus)
#define Mk_PAD_MINUS (Qt::KeypadModifier | Qt::Key_Minus)
#define Mk_PAD_SLASH (Qt::KeypadModifier | Qt::Key_Slash)
#define Mk_PAD_STAR  (Qt::KeypadModifier | Qt::Key_Asterisk)
#define Mk_PAD_ENTER (Qt::KeypadModifier | Qt::Key_Enter)

#define Mk_ESCAPE Qt::Key_Escape
#define Mk_ENTER  Qt::Key_Return // Qt::Key_Enter ?
#define Mk_TAB    Qt::Key_Tab
#define Mk_BkTAB  Qt::Key_Backtab
#define Mk_BACK   Qt::Key_Backspace

#ifdef Q_OS_MAC                  // Insert key is called "Help" on Mac keyboard
#  define Mk_INSERT Qt::Key_Help // and the name sticks even when Microsoft k/b
#else                            // attached; restore the God-given name here
#  define Mk_INSERT Qt::Key_Insert
#endif
#define Mk_DELETE Qt::Key_Delete
#define Mk_F1     Qt::Key_F1
#define Mk_F2     Qt::Key_F2
#define Mk_F3     Qt::Key_F3
#define Mk_F4     Qt::Key_F4
#define Mk_F5     Qt::Key_F5
#define Mk_F6     Qt::Key_F6
#define Mk_F7     Qt::Key_F7
#define Mk_F8     Qt::Key_F8
#define Mk_F9     Qt::Key_F9
#define Mk_F10    Qt::Key_F10
#define Mk_F11    Qt::Key_F11
#define Mk_F12    Qt::Key_F12

#define MAKE_TRANS_TABLE
#include "ccd.h"
micom NoCMD = { TK_NONE, 0, KxSEL };
//-----------------------------------------------------------------------------
static int kbMode = 0;
int KbCode;                            /* код запроса                        */
int KbCount = 1;                       /* повторитель, если не вводился то 1 */
int KbRadix = 0;                       /* основание,   если не вводился то 0 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static micom *translate_by_tbl (int key)
{
  for (micom *pt = trans1; pt->ev; pt++) if (pt->kcode == key) return   pt;
                                                               return NULL;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static micom *key_translate (int key_mask)
{
//+
//fprintf(stderr, "trans(%x,mode=%x)", key_mask, kbMode);
//-
  micom *res;
  if (kbMode == TK_ESC) {
    if (Mk_IsCHAR(key_mask)) {
      int digit = (key_mask > '@') ? ((key_mask&0xDF)-'A'+10) : (key_mask-'0');
      if (KbRadix) {
        if (0 <= digit && digit < KbRadix) {
          KbCount = KbRadix * KbCount + digit; return &NoCMD;
      } }
      else { if (digit ==  0) { KbRadix =  8; return &NoCMD; }
        else if (digit == 33) { KbRadix = 16; return &NoCMD; }
        else
        if (0 <= digit && digit <= 9) { KbRadix = 10;
                                        KbCount = digit; return &NoCMD; }
    } }               kbMode  = 0;
    if (KbCount <= 0) KbCount = 1;
    if (KbRadix) return key_translate(key_mask);
    else {
      res =    translate_by_tbl(mod_ESC | key_mask);
      return res ? res : translate_by_tbl(key_mask);
  } }
  if (kbMode == TK_HOME) {
    res =   translate_by_tbl(mod_HOME | key_mask); kbMode = 0;
    return res ? res : translate_by_tbl(key_mask);
  }
  else if (Mk_IsCHAR(key_mask)) return 0; // use standard translation
  else return translate_by_tbl(key_mask); // translate by prim. table
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
micom *key2mimCmd (int key_mask)   /* returns NULL if not a microMir command */
{
  micom *mk = key_translate(key_mask);
//+
//if (mk) fprintf(stderr, "ev=%x\n", mk->ev);
//else    fprintf(stderr, "ev=NULL\n");
//-
  if (mk) {
    switch (mk->ev) {
    case TK_ESC:  KbCount = KbRadix = 0;
    case TK_HOME: kbMode  = mk->ev; return &NoCMD;
    default:                        return     mk;
  } }
  return mk;
}
//-----------------------------------------------------------------------------

//------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 1998-2011
//------------------------------------------------------+----------------------
#include <stdio.h>
#include <Qt>
#include <QKeyEvent>
#include <QKeySequence>
#include "mim.h"
#include "ccd.h"
#define MAKE_TRANS_TABLE
#include "ccd.h"
micom NoCMD = { TK_NONE, 0, KxSEL };
//-----------------------------------------------------------------------------
static int kbMode = 0;
int KbCode;                            /* код запроса                        */
int KbCount = 1;                       /* повторитель, если не вводился то 1 */
int KbRadix = 0;                       /* основание,   если не вводился то 0 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int MkConvertKeyMods (QKeyEvent *event, int &modMask)
{
  QString text = event->text();
  int ev_key = event->key();
  int    key = ev_key | int(event->modifiers() &  Qt::KeypadModifier);
//
  modMask = ((event->modifiers() & Qt::ShiftModifier) ? mod_SHIFT : 0)|
            ((event->modifiers() & Qt::AltModifier)   ? mod_ALT   : 0)|
#ifdef Q_OS_MAC
            ((event->modifiers() & Qt::MetaModifier)    ? mod_CTRL : 0)|
            ((event->modifiers() & Qt::ControlModifier) ? mod_META : 0);
#else
            ((event->modifiers() & Qt::ControlModifier) ? mod_CTRL : 0)|
            ((event->modifiers() & Qt::MetaModifier)    ? mod_META : 0);
#endif
  if (MiApp_debugKB) {            fprintf(stderr, "OnKey(%x",    key);
    if (text[0].unicode() > 0x1f) fprintf(stderr, ":%s", text.cStr());
    else for (int i=0; i<text.length(); i++)
       fprintf(stderr, ".%04x", text[i].unicode());
  //
    if (Mk_IsSHIFT(key)) last_MiCmd_key = "";
    else {
      last_MiCmd_key = MkToString(modMask|ev_key);
      fprintf(stderr, ",%s", last_MiCmd_key.cStr());
    }
    fprintf(stderr, ")mods=%c%c%c%c,native=%x:%x:%x",
       (modMask & mod_META) ? 'M' : '.', (modMask & mod_CTRL)  ? 'c' : '.',
       (modMask & mod_ALT)  ? 'a' : '.', (modMask & mod_SHIFT) ? 's' : '.',
       event->nativeScanCode(),
       event->nativeModifiers(), event->nativeVirtualKey());
  }
#if defined(Q_OS_MAC) && (QT_VERSION < 0x040500)
    if (key == Qt::Key_unknown)            //----- old Qt version 4.3.3 did not
      switch (event->nativeVirtualKey()) { // convert keys F13…F15 correctly on
      case 0x69: key = Qt::Key_F13; break; // Apple new solid aluminum keyboard
      case 0x6b: key = Qt::Key_F14; break; // (model A1243), fixing that, using
      case 0x71: key = Qt::Key_F15; break; // native virtual key (which is Ok)
      }
#endif
  if (key & Qt::KeypadModifier) {          // For whatever reasons, Qt reports
#ifdef Q_OS_MAC                            // arrows key always with KeypadMod
    switch (ev_key) {                      // only on Mac, make it consistent
    case Qt::Key_Up:   case Qt::Key_Left:  //
    case Qt::Key_Down: case Qt::Key_Right: key = ev_key; break;
    }
#else
# ifdef Q_OS_WIN                       // completely ignore Alt+Numpad keys,
    if (modMask == mod_ALT) return -1; // let Windows handle them by itself
# endif
// Make MicroMir Numpad commands work the same with any NumLock state (except
//                                               for Mac, which has no NumLock)
    int keyName = (key & ~Qt::KeypadModifier);
    if (keyName == Qt::Key_Enter) keyName = Mk_PAD_ENTER; // merge numpad Enter
    switch (modMask + keyName) {                          // to Return/Enter(↵)
    case mod_CTRL+'8':
    case mod_CTRL+mod_SHIFT+'8':
    case mod_CTRL+          Qt::Key_Up:
    case mod_CTRL+mod_SHIFT+Qt::Key_Up:    key = Mk_PAD_UP;    break;
    case mod_CTRL+'2':
    case mod_CTRL+mod_SHIFT+'2':
    case mod_CTRL+          Qt::Key_Down:
    case mod_CTRL+mod_SHIFT+Qt::Key_Down:  key = Mk_PAD_DOWN;  break;
    case mod_CTRL+'4':
    case mod_CTRL+mod_SHIFT+'4':
    case mod_CTRL+          Qt::Key_Left:
    case mod_CTRL+mod_SHIFT+Qt::Key_Left:  key = Mk_PAD_LEFT;  break;
    case mod_CTRL+'6':
    case mod_CTRL+mod_SHIFT+'6':
    case mod_CTRL+          Qt::Key_Right:
    case mod_CTRL+mod_SHIFT+Qt::Key_Right: key = Mk_PAD_RIGHT; break;
    case mod_CTRL+'-': //
    case mod_CTRL+'+': //
    case mod_CTRL+'/': // these keys are already Ok, leave them alone
    case mod_CTRL+'*': //
    case mod_CTRL+Qt::Key_Return: break;
    default:
      if (modMask) {       // Force corresponding command with any modifier..
        switch (keyName) { //
        case '.': key = Mk_DELETE;   break;
        case '0': key = Mk_INSERT;   break;   case '5': key = Mk_CLEAR;  break;
        case '1': key = Mk_END;      break;   case '6': key = Mk_RIGHT;  break;
        case '2': key = Mk_DOWN;     break;   case '7': key = Mk_HOME;   break;
        case '3': key = Mk_PAGEDOWN; break;   case '8': key = Mk_UP;     break;
        case '4': key = Mk_LEFT;     break;   case '9': key = Mk_PAGEUP; break;
        default:  key = keyName;
      } }
      else key = keyName; // ..and ingnore Qt::KeypadModifier otherwise
    }
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  }
  return key;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void key2mimStart()
{
#ifdef notdef_obsolete //+
  for (micom *pt = trans1; pt->ev; pt++)
    if (( pt->attr &= KxSEL ) == KxTMP) pt->attr = KxTS|KxSEL;
#endif
}
micom_enum key2mimCheckPrefix (int mk)
{
  if (mk == TK_ESC || mk == TK_HOME
                   || mk == TK_CtrJ) { kbMode = mk; return TK_NONE;        }
  else                               { kbMode =  0; return (micom_enum)mk; }
}
int key2mimHomeEscMask()
{
  switch (kbMode) { case TK_HOME: return mod_HOME;
                    case TK_CtrJ: return mod_CtrJ;
                    default:      return           0;
                    case TK_ESC:  return KbRadix ? 0 : mod_ESC; } }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static micom *translate_by_tbl (int key)
{
  for (micom *pt = trans1; pt->ev; pt++) if (pt->kcode == key) return   pt;
                                                               return NULL;
}
static micom *key_translate (int key_mask)
{
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
    } }              kbMode  = 0;
    if (KbCount < 0) KbCount = 0;
    if (KbRadix) return key_translate(key_mask);
    else {
      res =    translate_by_tbl(mod_ESC | key_mask);
      return res ? res : translate_by_tbl(key_mask);
  } }
  if (kbMode == TK_CtrJ) {
    res =   translate_by_tbl(mod_CtrJ | key_mask); kbMode = 0;
    return res ? res : translate_by_tbl(key_mask);
  }
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
  if (mk) {
    switch (mk->ev) {
    case TK_ESC:  KbCount = KbRadix = 0; // and FALL THROUGH
    case TK_CtrJ:
    case TK_HOME: kbMode = mk->ev; return &NoCMD;
    default:                       return     mk;
  } }
  return mk;
}
//-----------------------------------------------------------------------------
int MkFromString (QString keystr)  //  Conversion from user-readable/-writable
{                                  //  string to the keycode as reported by Qt
  keystr.replace("^", "Ctrl+");
  QKeySequence SQ(keystr);
  int mask = 0, kcode;
  for (size_t i=0; i<SQ.count(); i++)
    switch (( kcode = SQ[i] )) {
    case Mk_ESCAPE:    mask = mod_ESC;  continue; //  NOTE: do not accumulate
    case Mk_HOME:      mask = mod_HOME; continue; // mask, only the last valid
    case mod_CTRL+'J': mask = mod_CtrJ; continue;
    default:       return kcode | mask;
    }
  return 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QString MkToString(int kcode)
{
  if (kcode == 0x4000041) kcode = 0x400004a;
//+
  int k1,k2;
  switch (kcode & 0x00e00000) {
  case mod_HOME: k1 = Qt::Key_Home;   k2 = kcode; break;
  case mod_CtrJ: k1 = Qt::CTRL+'J';   k2 = kcode; break;
  case mod_ESC:  k1 = Qt::Key_Escape; k2 = kcode; break;
  default:       k1 = kcode;          k2 = 0;
  }
  QKeySequence SQ(k1,k2 & ~0x00e00000);
  return SQ.toString(QKeySequence::PortableText);
}
//-----------------------------------------------------------------------------

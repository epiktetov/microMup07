//------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 1998-2011
//------------------------------------------------------+----------------------
//include <qnamespace.h>
#include <QKeyEvent>
#include <QKeySequence>
#include "mim.h"
#include "ccd.h"
#include "vip.h"
#include "lua.hpp"
#include "luas.h"
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
  bool  keypad = (event->modifiers() &  Qt::KeypadModifier);
  int      key =  event->key();
  QString text =  event->text();
//
// Revert the Qt intelligence: Ctrl should be Ctrl on any platform, I'm too old
// to re-learn the keysequences twice a day switching from Mac to Linux to Win:
//
  modMask = (event->modifiers() & (Qt::AltModifier|Qt::ShiftModifier)) |
#ifdef Q_OS_MAC
            ((event->modifiers() & Qt::MetaModifier)    ? mod_CTRL : 0)|
            ((event->modifiers() & Qt::ControlModifier) ? mod_META : 0);
#else
            ((event->modifiers() & Qt::ControlModifier) ? mod_CTRL : 0)|
            ((event->modifiers() & Qt::MetaModifier)    ? mod_META : 0);
#endif
  if (MiApp_debugKB) { fprintf(stderr, "OnKey(%s%x", keypad ? "#" : "", key);
    if (text[0].unicode() > 0x1f)        fprintf(stderr, ":%s", text.cStr());
    else for (int i=0; i<text.length(); i++)
           fprintf(stderr, ".%02x", text[i].unicode());

    fprintf(stderr, "|%c%c%c%c),native=%x:%x:%x",
        (modMask & mod_META) ? 'M' : '.', (modMask & mod_CTRL)  ? 'c' : '.',
        (modMask & mod_ALT)  ? 'a' : '.', (modMask & mod_SHIFT) ? 's' : '.',
                          event->nativeScanCode(), event->nativeModifiers(),
                                                   event->nativeVirtualKey());
    if (Mk_IsSHIFT(key)) fprintf(stderr, "\n");
  } if (Mk_IsSHIFT(key)) return -1;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if defined(Q_OS_MAC) && (QT_VERSION < 0x040500)
    if (key == Qt::Key_unknown)            //    old Qt version (4.3.3) did not
      switch (event->nativeVirtualKey()) { // convert keys F13…F15 correctly on
      case 0x69: key = Qt::Key_F13; break; // Apple new solid aluminum keyboard
      case 0x6b: key = Qt::Key_F14; break; // (model A1243), fixing that, using
      case 0x71: key = Qt::Key_F15; break; // native virtual key (which is Ok)
      }
#endif
  if (keypad) {    // For whatever reasons, Qt always reports arrow keys with
#ifdef Q_OS_MAC    // KeypadMod on Mac, remove it for consistency..
    switch (key) { //
    case Qt::Key_Up:   case Qt::Key_Left:
    case Qt::Key_Down: case Qt::Key_Right: return key;
    }
#else
# ifdef Q_OS_WIN                       // completely ignore Alt+Numpad keys,
    if (modMask == mod_ALT) return -1; // let Windows handle them by itself
# endif
    switch (key) {
    case Qt::Key_Delete:   key = '.'; break; // for consistency, ALWAYS convert
    case Qt::Key_Insert:   key = '0'; break; // base key to digit, so Linux and
    case Qt::Key_End:      key = '1'; break; // Windows behaves the same as Mac
    case Qt::Key_Down:     key = '2'; break; //
    case Qt::Key_PageDown: key = '3'; break; // (for those attached to standard
    case Qt::Key_Left:     key = '4'; break; // OS-specific behavior, feel free
    case Qt::Key_Clear:    key = '5'; break; // to remap command by Lua script)
    case Qt::Key_Right:    key = '6'; break; //
    case Qt::Key_Home:     key = '7'; break;
    case Qt::Key_Up:       key = '8'; break;
    case Qt::Key_PageUp:   key = '9'; break;
    }
#endif
    return key | mod_KyPAD;
  } return key;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
QString MkToString (int kcode)
{
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
void MkMimXEQ (int kcode, int modMask, QString text, wnd *vp)
{
  last_MiCmd_key = MkToString(kcode|modMask);
  luaP_getglobal("MkCCD");  luaX_getfield_top(last_MiCmd_key.uStr());
  if (MiApp_debugKB) fprintf(stderr, ",‹%s›", last_MiCmd_key.uStr());

  if (lua_isnumber(L,-1)) {                            // must check before str
    int mk = lua_tointeger(L,-1);        luaQn_pop(1); // (as number is string,
    if (MiApp_debugKB) fprintf(stderr, ",0x%x\n", mk); // since its convertible
    vipOnKeyCode(vp, mk, 0);                   return; // to string always)
  }
  else if (lua_isstring  (L,-1)) text = lua_tostring(L,-1);
  else if (lua_isfunction(L,-1)) {
    if (MiApp_debugKB) fprintf(stderr, ",lua\n");
    // not implemented yet
    return;
  }
  if (MiApp_debugKB) fprintf(stderr, ",'%s'\n", text.uStr());
  luaQn_pop(1);
  for (int i=0; i<text.length(); i++) {
    int k = text.at(i).unicode();
    if (Mk_IsCHAR(k)) vipOnKeyCode(vp, k, KxSEL);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MkInitCCD (void)          // move Command Codes Definitions into Lua table
{
  luaP_newtable();
  for (microCCD *pt = CCD; pt->hexcode; pt++) {
    if (pt->kseq) { luaP_pushstring (pt->kseq);
                    luaP_pushinteger(pt->hexcode); luaQQ_rawset(-3); }
  }
  luaQ_setglobal("MkCCD");
}
//-----------------------------------------------------------------------------
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
static micom trans1[] = { {0,0,0} };
//+
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

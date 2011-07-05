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
//-----------------------------------------------------------------------------
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
  if (MiApp_debugKB) { fprintf(stderr, "OnKey(%x", key);
    if (!text.isEmpty()) {
      if (text.at(0).unicode() >= ' ') fprintf(stderr, ":%s", text.cStr());
      else for (int i = 0; i < text.size(); i++)
                               fprintf(stderr, ".%02x", text[i].unicode());
    }
    fprintf(stderr, "|%c%c%c%c%c),native=%x:%x:%x",      keypad ? '#' : '.',
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
  int keypad = 0;
  int N = keystr.length() - 1;
  if (keystr.at(0).unicode() == '[' && keystr.at(N).unicode() == ']') {
      keystr = keystr.mid(1,N-1);      keypad = mod_KyPAD;
  }
  QKeySequence ks(keystr); return keypad | ks[0];
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int MkToDigit (int kcode)
{
  int key = (kcode & ~mod_KyPAD);
       if ('0' <= key && key <= '9') return key - '0';
  else if ('A' <= key && key <= 'F') return key - 'A' + 10;
  else                               return -1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QString MkToString (int kcode)
{
  bool keypad  =  kcode &  mod_KyPAD;
  QKeySequence ks(kcode & ~mod_KyPAD);
  QString st = ks.toString();
  if (keypad && st.size() > 0) { int N = st.size() - 1;
              return st.left(N) + "[" + st.at(N) + "]";
  } else      return st;
}
/*---------------------------------------------------------------------------*/
#define MkMAX_MACRO_SIZE 350 /* the only limitation is to be able editing it */
QString MkMacro;             /*  (so Mk.Fn="value" should fit under MAXTXRM) */
int MkRecording = 0;
inline QString MacroName(int N) { return QString("F%1").arg(N+6); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void MkStartRecording (int N)
{
  QString macro = MacroName( MkRecording = N );
  luaP_getglobal("Mk");
  luaP_pushstring (macro.cStr());
  luaP_pushinteger    (TK_EM0+N);
  luaQQ_rawset(-3); luaQn_pop(1);
}
void MkStopRecording (void) //- - - - - - - - - - - - - - - - - - - - - - - - -
{                                                // make sure there's no self-
  QString macro = MacroName(MkRecording);        // references in the string,
  MkMacro.replace(Utf8("‹")+macro+Utf8("›"),""); // to avoid infinite recursion
  luaP_getglobal("Mk");                          // (one such reference always
  luaP_pushstring(  macro.cStr());               //  recorded just before stop)
  luaP_pushstring(MkMacro.uStr()); MkMacro.clear();
  luaQQ_rawset(-3);  luaQn_pop(1); MkRecording = 0;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int KbCode;                  /* код запроса (устанавливается в vipOnRegCmd)  */
int KbCount =  1;            /* повторитель, если не вводился,         то 1  */
int KbRadix =  0;            /* основание,   если не было повторителя, то 0  */
QString MkPrefix;            /* текущий префикс (пустая строка если не было) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void MkMimXEQ (int kcode, int modMask, QString text, wnd *vp)
{
  int digit = MkToDigit(kcode);
  if (KbRadix && !modMask && digit >= 0) {
    KbCount = KbRadix * KbCount + digit;
    last_MiCmd_key = Utf8("×%1").arg(KbCount); return;
  }
  luaP_getglobal("Mk");
  if (MkPrefix.isEmpty()) { last_MiCmd_key = MkToString(kcode|modMask);
                            luaP_getfield(-1,last_MiCmd_key.uStr()); }
  else {
    if (MkPrefix == "Esc" && !modMask) {
      const char *radix = "";
           if (kcode == 'X') { KbRadix = 16; KbCount = 0;     radix = "hex"; }
      else if (digit ==  0 ) { KbRadix =  8; KbCount = 0;     radix = "oct"; }
      else if (digit  >  0 ) { KbRadix = 10; KbCount = digit; radix = "dec"; }
      if (KbRadix) {
        last_MiCmd_key = Utf8("(%1)×%2…").arg(radix).arg(KbCount);
        luaQn_pop(1);                            MkPrefix.clear(); return;
    } }
    QString MkStr = MkToString(kcode|modMask);
    last_MiCmd_key = MkPrefix + "," + MkStr;
    luaP_getfield(-1,last_MiCmd_key.uStr()); MkPrefix.clear();
    if (lua_isnil(L,-1)) {
      luaQn_pop(1);    last_MiCmd_key = MkStr; // Try finding CCD with prefix
      luaP_getfield(-1,last_MiCmd_key.uStr()); // 1st, ignore it if not found
  } }
  if (MiApp_debugKB)  fprintf(stderr, ",‹%s›", last_MiCmd_key.uStr());
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if (MkRecording) {
    if (KbRadix) MkMacro.append(Utf8("×")+QString::number(KbCount)+Utf8("⁝"));
    int t1 = text.isEmpty() ? 0 : text.at(0).unicode();
    if (t1 < ' ') MkMacro.append(Utf8("‹")+last_MiCmd_key+Utf8("›"));
    else switch (t1) {
      case 0x00D7: // × ← these two characters have special meaning in macros
      case 0x2039: // ‹
               MkMacro.append(Utf8("⑆")+text); break;
      default: MkMacro.append          (text); break;
    }
    if (MkMacro.size() > MkMAX_MACRO_SIZE) { vipBell(); MkStopRecording(); }
  }
  MkLuaXEQ(text, vp);   // Lua stack here: [-2] global "Mk" (not used below)
}                       //                 [-1] command code definition value
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MkLuaXEQ (QString text, wnd *vp)
{
  if (lua_isnumber(L,-1)) {                            // MUST check before str
    int mk = lua_tointeger(L,-1);        luaQn_pop(2); // as number is str too,
    if (MiApp_debugKB) fprintf(stderr, ",0x%x\n", mk); // since its convertible
    switch (mk) {                                      // always to string
    case TK_PREFIX: MkPrefix = last_MiCmd_key; return;
    case TK_ESC:    MkPrefix = "Esc";          return;
    case TK_CtrJ:   MkPrefix = "^J";           return;
    case TK_SM1:
    case TK_SM2:
    case TK_SM3: if (MkRecording) { MkStopRecording();  vipBell(); }
                                    MkStartRecording(mk - TK_SM0); return;
    case TK_EM1:
    case TK_EM2:
    case TK_EM3: MkStopRecording();    return;
    default:     vipOnKeyCode(vp, mk); return;
  } }
  else if (lua_isstring(L,-1)) text = Utf8(lua_tostring(L,-1));
  else if (lua_isfunction(L,-1)) {
    if (MiApp_debugKB) fprintf(stderr, ",lua\n");
    luasFunc();                     luaQn_pop(1); return;
  }
  if (MiApp_debugKB) fprintf (stderr, ",'%s'\n", text.uStr());
  if (!text.isEmpty()) MkStrXEQ(text,vp);        luaQn_pop(2);
}
static int luMkXEQ (lua_State *L) { luaL_checktype(L, 1, LUA_TTABLE);
                                    luaL_checkany (L, 2);
                                    MkLuaXEQ ("",  Twnd);   return 0; }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MkStrXEQ (QString text, wnd *vp)
{
  int N, len = text.length();
  for (int i = 0; i < len; i++) {
    int k = text.at(i).unicode();
    if (k == 0x2446 && i+1 < len) vipOnKeyCode(vp, text.at(++i).unicode()); //⑆
    else if (k ==  0x2039) {                                                //‹
      if ((N = text.indexOf(Utf8("›"),i+1)) > 0) {
        luaP_getglobal("Mk");
        luaP_getfield(-1,text.mid(i+1,N-i-1).cStr()); i=N; MkLuaXEQ("", vp);
    } }
    else if (k == 0xD7) { //×
      if ((N = text.indexOf(Utf8("⁝"),i+1)) > 0) {
        KbRadix = 1;                                // applied to next command
        KbCount = text.mid(i+1,N-i-1).toInt(); i=N; // (radix does not matter)
    } }
    else if (Mk_IsCHAR(k)) vipOnKeyCode(vp, k);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MkInitCCD (void)   // move Command Codes Definitions into global Lua table
{
  luaP_newtable();
  for (microCCD *pt = CCD; pt->hexcode; pt++) {
    if (pt->kseq) { luaP_pushstring (pt->kseq);
                    luaP_pushinteger(pt->hexcode); luaQQ_rawset(-3); }
  }
  luaP_pushstring     ("XEQ");
  luaP_pushCfunction(luMkXEQ); luaQQ_rawset(-3); luaQ_setglobal("Mk");
}
//-----------------------------------------------------------------------------

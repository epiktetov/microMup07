//------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 1998-2011
//------------------------------------------------------+----------------------
#include <Qt>
#include <QKeySequence>
#include "mic.h"
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
void key2mimStart()
{
  for (micom *pt = trans1; pt->ev; pt++)
    if (( pt->attr &= KxSEL ) == KxTMP) pt->attr = KxTS|KxSEL;
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

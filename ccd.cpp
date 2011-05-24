//------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 1998-2011
//------------------------------------------------------+----------------------
#include <Qt>
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
static micom *translate_by_tbl (int key)
{
  for (micom *pt = trans1; pt->ev; pt++) if (pt->kcode == key) return   pt;
                                                               return NULL;
}
void key2mimStart()
{
  for (micom *pt = trans1; pt->ev; pt++)
    if (( pt->attr &= KxSEL ) == KxTMP) pt->attr = KxTS|KxSEL;
}
int key2mimHomeEscMask() { return (kbMode == TK_HOME ? mod_HOME :
                                   kbMode == TK_ESC  ? mod_ESC  : 0); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
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
    case TK_ESC:  KbCount = KbRadix = 0;
    case TK_HOME: kbMode  = mk->ev; return &NoCMD;
    default:                        return     mk;
  } }
  return mk;
}
//-----------------------------------------------------------------------------

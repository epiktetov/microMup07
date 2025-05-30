/*------------------------------------------------------+----------------------
// МикроМир07          Embedded Lua scripting           | (c) Epi MG, 2011-2025
//------------------------------------------------------+--------------------*/
#include <QRegExp>
#include "mim.h"
#include "ccd.h"
#include "lua.hpp"
#include "luas.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
extern "C" {
#include "dq.h"
#include "le.h"
#include "te.h"
#include "tx.h"
#include "ud.h" // UndoMark
}
lua_State *L = NULL;
//-----------------------------------------------------------------------------
struct luRegExp { // Using "full" user-data to enable proper garbage collection
  QRegExp *re;    // (light userdata is a value, has no individual metatable,
};                //        and it is not collected -- as it was never created)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Object cosntructor: re = Re("..regex.."[,false]) or Re[[..regex..]]
//                                          ^
static int luasNewRe (lua_State *L) // case-sensitive? (default: true)
{
  QString ptn = luaL_checkstring(L,1);
  bool cs  =  luaX_optboolean(2,true);
  luRegExp *newRe = (luRegExp *)luaP_newuserdata(sizeof(luRegExp));
  newRe->re = new QRegExp(ptn, cs ? Qt::CaseSensitive : Qt::CaseInsensitive);
  luaP_getmetatable("re");
  luaQ_setmetatable  (-2); return 1;
}
static int luasReGC (lua_State *L) // called from garbage collector
{
  luRegExp *Re = (luRegExp*)luaL_checkudata(L,1,"re");
  if (Re->re) delete Re->re;            Re->re = NULL; return 0;
}
static int luasReTS (lua_State *L) // tostring(Re) = pattern (mostly for debug)
{
  luRegExp *Re = (luRegExp*)luaL_checkudata(L,1,"re");
  luaP_pushstring(Re->re->pattern().uStr()); return 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Re (Regular expression) methods, intentionally with different names compared
// to built-in Lua patterns to avoid confusion (args modelled after Qt lib):
//
// re:ifind("str",pos)  -- returns index of first match (nil if does not match)
// re:cap(N)            -- returns Nth capture
// re:caps()            -- all captures (to use in multi-value assignement)
// re:grepl("str","to") -- globally replace pattern (where \N = Nth capture)
//
static int luasReIndex (lua_State *L)
{
  luRegExp *Re = (luRegExp*)luaL_checkudata(L,1,"re");
  QString str = Utf8(luaL_checkstring(L,2));
  int ipos = luaL_optinteger(L,3,-1);
  int found = Re->re->indexIn(str, ipos+1); // Qt string starts indexing from 0
  if (found < 0) luaP_pushnil();            //            but Lua starts from 1
  else           luaP_pushinteger(found+1); return 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luasReCap (lua_State *L) // valid arg: 0 to captureCount() inclusive
{                                   //                     cap(0) = whole match
  luRegExp *Re = (luRegExp*)luaL_checkudata(L,1,"re");
  int N = luaL_checkinteger(L,2);
  luaP_pushstring(Re->re->cap(N).uStr()); return 1;
}
static int luasReCaps (lua_State *L)
{
#if QT_VERSION >= 0x040600
  luRegExp *Re = (luRegExp*)luaL_checkudata(L,1,"re");
  int N = Re->re->captureCount();
  for (int i=1; i<=N; i++) luaP_pushstring(Re->re->cap(i).uStr()); return N;
#else
  return luaL_error(L,"requires Qt 4.6");
#endif
}
static int luasReGRepl (lua_State *L)
{
  luRegExp *Re = (luRegExp*)luaL_checkudata(L,1,"re");
  QString str = Utf8(luaL_checkstring(L,2));
  QString tos = Utf8(luaL_checkstring(L,3));
  luaP_pushstring(str.replace(*(Re->re), tos).uStr()); return 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
luaL_Reg luReMetaFuncs[] =
{
  { "ifind", luasReIndex }, { "cap",  luasReCap  }, { "__gc",       luasReGC },
  { "grepl", luasReGRepl }, { "caps", luasReCaps }, { "__tostring", luasReTS },
  { NULL, NULL }
};
static void luaReInit (void)
{
  luaP_pushCfunction(luasNewRe); luaQ_setglobal("Re");
//
// "re" metatable (used as metatable for objects returned by Re function)
//   functions  - instance functions (methods == ifind,cap,caps,grepl,etc)
//   __index    - reference to itself
//
  luaP_newmetatable("re"); luaX_setfuncs(luReMetaFuncs);
  luaP_pushvalue(-1);
  luaQ_setfield(-2,"__index"); luaQn_pop(1);
}
//-----------------------------------------------------------------------------
struct luTxtID { // userdata for safe reference to the particular text instance
  txt *t;        // - pointer (safe to use, since txt_tag structs never freed)
  int id;        // - expected value of luaTxid in that text
};
static int atNextInst = 1;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void luasNtxt (txt *newTxt)  // "new text" hook (called from tmDoLoad, twm.cpp)
{
  luaP_getglobal("Txt"); int iTxt = lua_gettop(L);
  if (newTxt->luaTxid) {
    luaP_pushinteger(newTxt->luaTxid);
    luaP_pushnil();
    luaQQ_settable(iTxt); // Txt[oldTxid] = nil (remove the element)
  }
  luaP_pushinteger(newTxt->luaTxid = atNextInst++);
  luaP_newtable();
  luTxtID *newTxtID = (luTxtID *)luaP_newuserdata(sizeof(luTxtID));
           newTxtID->t  = newTxt;
           newTxtID->id = newTxt->luaTxid; luaQ_setfield(-2,"√id");
  luaP_getmetatable("txt");            //
  luaQ_setmetatable(-2);               // NOTE: metatable for Tx obj must be
  luaQQ_rawset   (iTxt); luaQn_pop(1); // set after Tx.id (to avoid recursion)
//+
//  fprintf(stderr, "luasNtxt(%s):%d\n", newTxt->file->full_name.cStr(),
//                                       newTxt->luaTxid);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static txt *luasN_gettext (int ix)  // returns txt from Lua reference on given
{                                   // index in the stack (verifies id match)
  luaP_getfield(ix,"√id");
  luTxtID *txtID = (luTxtID*)lua_touserdata(L,-1); luaQn_pop(1);
  if (txtID == NULL ||
      txtID->id != txtID->t->luaTxid) luaL_error(L,"bad txt reference");
//
// MicroMir usually does not bother updating Ttxt->vp_ctx/y, but for simplicity
// all luTx… functions operate with 'txt' structure only, do that update here:
//
  if (txtID->t == Ttxt) { Ttxt->vp_ctx = Tx;
                          Ttxt->vp_cty = Ty; }  return txtID->t;
}
static void luas_UpdateTxy (txt *t, bool changed = false) // reverse update
{
  if (changed) t->txstat |= TS_CHANGED; if (t == Ttxt) { Tx = Ttxt->vp_ctx;
                                                         Ty = Ttxt->vp_cty; }
}
//-----------------------------------------------------------------------------
// Txt object constructor by text name or bool flag: true = "real text" (save)
//                                                   false = not real (discard)
// Tx.open("name"/true/false) --> text object
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxOpen (lua_State *L)  // for named texts, Lua table is created by
{                                   // tmDoLoad functions, for ÷/… pseudo-files
  QString tname; bool init = false; // we must do that manually here
  if (lua_isboolean(L,1))
     { tname = lua_toboolean(L,1) ? QfsEMPTY_NAME : QfsELLIPSIS; init = true; }
  else tname = luaL_checkstring(L,1);
  vipFocusOff(Twnd); Twnd = vipSplitWindow(Twnd, TM_VFORK);
  if (Twnd && twEdit(Twnd, tname)) {
    if (init) luasNtxt(Ttxt);
    luaP_getglobal("Txt");
    luaX_rawgeti_top(Ttxt->luaTxid); return 1;
  }
  else return luaL_error(L,"cannot open %s", tname.uStr());
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxFocus (lua_State*)     // Tx:focus() = focus last opened window,
{                                     //              associated with the text
  txt *tx = luasN_gettext(1);
  wnd *vp = tx->txwndptr;
  if (vp) { vipActivate(vp);
            vipFocus   (vp);   return 0; }
  else return luaL_error(L,"no window");
}
//-----------------------------------------------------------------------------
// Methods and instance variables (members) of Txt class, text access/movements
//
//   Tx:line(N) = content of Nth line in given text (= nil if past end-of-text)
//   Tx:lines() = iterator over the text:  for N,line in Tx:lines() do ... end
//   Tx.X, Tx.Y = current cursor position  (setting new value will move cursor)
//   Tx.reX,reY = opposite corner of selection rectangle (nil if no selection)
//   Tx.maxY    = max valid value of Y (= total number of lines in given text)
//   Tx.name    = text (file) name (not including the path)
//   Tx.fname   = full canonical file name (with absolute path)
//   Tx:go(dy)
//   Tx:go(dx,dy) = convenience methods to move cursor around ('cause Lua does
//                                      not allow 'Tx.Y++' or even 'Tx.Y += k')
// NOTE:
//   attempt to move cursor out of text boundaries does not generate any
//   error, cursor just moves as far as possible in requested direction
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxLine (lua_State *L) // Tx:line(N) == Tx.line(Tx,N)
{
  txt *t = luasN_gettext(1);
  long Y = (long)luaL_optnumber(L,2,t->vp_cty+1);
  int len;
  if (TxSetY(t, Y-1)) { char *line = TxGetLn(t, &len);
                         luaP_pushlstring(line,  len); } else luaP_pushnil();
  return 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxLine_it (lua_State *L)   // line iterator function called as:
{                                       //
  txt *t = luasN_gettext(1);            //   while true do
  long N = (long)luaL_checknumber(L,2); //     local N,line = line_it(self,N)
  int len;                              //     if N == nil then break end
  if (TxSetY(t, N) && !qTxBottom(t)) {  //     ...
    char *line = TxGetLn(t, &len);      //   end
    luaP_pushnumber(N+1);
    luaP_pushlstring(line, len);
  }
  else { luaP_pushnil(); luaP_pushnil(); }  return 2;
}
static int luTxLines (lua_State *)  // optional parameter to Tx:lines specifies
{                                   // start position for iterator (default: 1)
  luasN_gettext(1);                 //
  long Y = (long)luaL_optnumber(L,2,1);
  luaP_pushCfunction(luTxLine_it);  //    function = luTxLine_it(self,N)
  luaP_pushvalue    (1);            //       state = text descriptor
  luaP_pushinteger(Y-1); return 3;  //    init.var = Y-1 (before given line)
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxIndex (lua_State *L) // Tx.X and Tx.Y - read/write access
{                                   // Tx.maxY       - read-only
  txt *t = luasN_gettext(1);
  const char *var = luaL_checkstring(L,2);
       if (strcmp(var,"id")  == 0) luaP_pushinteger(t->luaTxid);
  else if (strcmp(var,"X")   == 0) luaP_pushinteger(t->vp_ctx+1);
  else if (strcmp(var,"Y")   == 0) luaP_pushinteger(t->vp_cty+1);
  else if (strcmp(var,"reX") == 0) {
    if (t == Ttxt && BlockMark) luaP_pushinteger(BlockTx+1);
    else                        luaP_pushnil();
  }
  else if (strcmp(var,"reY") == 0) {
    if (t == Ttxt && BlockMark) luaP_pushinteger(BlockTy+1);
    else                        luaP_pushnil();
  }
  else if(strcmp(var, "maxY") == 0) luaP_pushinteger(t->maxTy);
  else if(strcmp(var, "name") == 0) luaP_pushstring(t->file->name.uStr());
  else if(strcmp(var,"fname") == 0) luaP_pushstring(t->file->full_name.uStr());
  else {
    luaP_getglobal ("Txt"); // refer to the base Txt table for everything else
    luaX_getfield_top(var); // (if field is not there, let Lua generate errors
  }                         // by itself)
  return 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void luSetBlock3 (bool setX) // X/y value on Lua stack[3], text == Ttxt
{
  long N = (long)luaL_optnumber(L,3,0);
  if (N > 0) { BlockMark = true;
               BlockTemp = false; if (setX) BlockTx = N-1;
                                  else      BlockTy = N-1; }
  else BlockMark = false;
}
static int luTxNewindex (lua_State *L)
{
  long N;       txt *t = luasN_gettext(1);
  const char *var = luaL_checkstring(L,2);
  if (strcmp(var,"X") == 0) {
    N = (long)luaL_checknumber(L,3); t->vp_ctx = my_min(my_max(N-1,0),t->txrm);
  }
  else if (strcmp(var,"Y") == 0) {             // negative value means counting
    if ((N = (long)luaL_checknumber(L,3)) < 1) // from bottom (0 == after text)
         t->vp_cty = my_max(t->maxTy-N+1,0);   //
    else t->vp_cty = my_min(my_max(N-1,0),t->maxTy);
  }
  else if (strcmp(var,"reX") == 0) { if (t == Ttxt) luSetBlock3(true);  }
  else if (strcmp(var,"reY") == 0) { if (t == Ttxt) luSetBlock3(false); }
  else { luaQQ_rawset(1); return 0; } //
  luas_UpdateTxy(t);      return 0;   // fall back to (raw) Lua settable
}                                     //      for all unknown properties
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxGo (lua_State *L) // Tx:go([dx,]dy) == Tx.go(Tx,[dx,]dy)
{                                //   if dx not specified, default to 0
  txt *t = luasN_gettext(1);
  int dx = 0, dy, k = 2;
  if (lua_gettop(L) > 2) dx = luaL_checkinteger(L,k++);
                         dy = luaL_checkinteger(L,k  );
  t->vp_ctx = my_min(my_max(t->vp_ctx + dx,0),t->txrm);
  t->vp_cty = my_min(my_max(t->vp_cty + dy,0),t->maxTy);
  luas_UpdateTxy(t);                           return 0;
}
//-----------------------------------------------------------------------------
// Methods of Txt class, text modifications:
//                                                 +-- unlike built-in MicroMir
//   Tx:IC("text") = insert text into current line | commands, move cursor past
//   Tx:IL("line") = insert given line at cursor   | inserted text
//   Tx:DC(N) = delete N characters at cursor
//   Tx:DL(N) = delete N lines at cursor (default = 1)
//   Tx:gtl() = get-text-left   (gets first chunk of non-spaces left of cursor)
//   Tx:mark(k,x,y,"text") = add/update mark k (0…19) at x,y (y<0 to remove it)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxIC (lua_State *L) // Tx:IC("text") = insert given text at cursor
{
  txt *t = luasN_gettext(1);          size_t len;
  const char *text = luaL_checklstring(L,2,&len);
  int tlen = aftotc(text, len, lfbuf);
  int ctx = t->vp_ctx;
  TxSetY(t, t->vp_cty); Lleng = TxFRead(t, Lebuf);
//
// Make room for new text, moving the tail tlen positions right (but check 1st
// that resulting line will not overflow the buffer), then copy new text there:
//
  int tail = (Lleng > ctx) ? (Lleng - ctx) : 0;
  if (ctx+tlen+tail > MAXLPAC) luaL_error(L,"line too long");
  if (tail) blktmov(Lebuf+ctx, Lebuf+ctx+tlen, tail);
            blktmov(lfbuf,     Lebuf+ctx,      tlen); TxFRep(t, Lebuf);
  t->vp_ctx += tlen;
  luas_UpdateTxy(t, true); return 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxIL (lua_State *L) // Tx:IL("text") = insert given line at cursor
{
  txt *t = luasN_gettext(1);          size_t len; // for consistency remove all
  const char *text = luaL_checklstring(L,2,&len); // trailing spaces - TxFRep()
  while (len > 0 && text[len-1] == ' ') len -= 1; // does that, TxIL() does not
  TxSetY(t, t->vp_cty++);
  TxIL  (t, (char*)text, len); luas_UpdateTxy(t, true); return 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luTxDC (lua_State *L) // Tx:DC(N) = delete N character (default = 1)
{
  txt *t = luasN_gettext(1); int N = luaL_optinteger(L,2,1);
  int ctx = t->vp_ctx;
  TxSetY(t, t->vp_cty);
  if ((Lleng = TxFRead(t, Lebuf)) < ctx) return 0;
  int tail = (Lleng > ctx+N) ?  (Lleng-ctx-N) : 0;
  if (tail) blktmov(Lebuf+ctx+N, Lebuf+ctx, tail);
            blktspac(Lebuf+ctx+tail, N);
  TxFRep(t, Lebuf);
  luas_UpdateTxy(t, true); return 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luTxDL (lua_State *L) // Tx:DL(N) = delete N lines (default = 1) at cursor
{
  txt *t = luasN_gettext(1); int N = luaL_optinteger(L,2,1);
  TxSetY(t, t->vp_cty);
  while (!qTxBottom(t) && N--) TxDL(t); luas_UpdateTxy(t, true); return 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luGetTextLeft (lua_State*) // Tx:gtl() = get first chunk of non-spaces from
{                              //           text to the left of cursor position
  txt *t = luasN_gettext(1);
  int ctx = t->vp_ctx;
  TxSetY(t, t->vp_cty); Lleng = TxFRead(t, Lebuf);
  int N = ctx;
  while (N > 0 &&  tcharIsBlank(Lebuf[N])) N--;
  while (N > 0 && !tcharIsBlank(Lebuf[N])) N--;
  if (N) N++;
  if (N < ctx) { N = tctoaf(Lebuf+N, ctx-N, afbuf);
                 luaP_pushlstring(afbuf, N); return 1; }
  else         { luaP_pushnil();             return 0; }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luTxMark (lua_State*) // Tx:mark(k,x,y,"text") =  add/update mark k (0…19)
{                         //   at given position, or remove the mark when x=y=0
  txt *t = luasN_gettext(1);
  int  k = luaL_checkinteger(L,2); if (k < 0 || k >= TXT_MARKS) return 0;
  int  x = luaL_checkinteger(L,3); if (x > MAXLPAC-3)      x = MAXLPAC-3;
  int  y = luaL_optinteger(L,4,0); if (y >  t->maxTy)      y =  t->maxTy;
//
  if (t->txmarks[k]) xfree(t->txmarks[k]); // clean up the old text in any case
  t->txmarkx[k] = x ? x-1  : 0;            // (it will either remain 0, or gets
  t->txmarky[k] =     y-1;                 //          replaced with new value)
  t->txmarks[k] = 0;
  if (x > 0 && y > 0) {                    size_t len;
    const char *text = luaL_optlstring(L,5,NULL,&len);
    if (text) {
      switch (*text) {
      case '!': text++; len--; t->txmarkt[k] = txMarkT[TXT_MARK_ERR];  break;
      case '@': text++; len--; t->txmarkt[k] = txMarkT[TXT_MARK_INFO]; break;
      case '#': text++; len--; t->txmarkt[k] = txMarkT[TXT_MARK_SMTH]; break;
      case '$': text++; len--;
      default: t->txmarkt[k] = txMarkT[TXT_MARK_WARN]; // (likely a warning)
      }
      if (len) t->txmarks[k] = xstrndup(text, len);
  } }
  wndop(TW_ALL, t); return 0; // always redraw the text (inefficient, but we're
}                             //  not going to check if mark is visible or not)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
luaL_Reg luTxFuncs[] =
{                         { "go", luTxGo }, { "gtl",  luGetTextLeft },
  { "open",  luTxOpen  }, { "IL", luTxIL }, { "mark", luTxMark      },
  { "focus", luTxFocus }, { "IC", luTxIC },
  { "line",  luTxLine  }, { "DL", luTxDL },
  { "lines", luTxLines }, { "DC", luTxDC }, { NULL, NULL   }
};
luaL_Reg luTxMetaFuncs[] =
{
  { "__index", luTxIndex }, { "__newindex", luTxNewindex }, { NULL, NULL }
};
static void luTxtInit (void)
{
// Txt table
//   open(tn/f) - object constructor, by text name or bool flag (true/false)
//   functions  - instance functions (methods == line,lines,go,IC,IL,DC,DL)
//   this       - reference to the currently active text (Ttxt), luasExec only
//   [luaTxid]  - all active texts are stored here (idexed by txt->luaTxid)
//     [k].id   -- instance id as userdata == { &txt_tag, txt_tag.luaTxid }
// metatable for instance objects
//   __index    - read access to instance variables (X,Y,maxY;ref->Txt)
//   __newindex - write access to inst variables (X,Y only)
//
  luaP_newlibtable(luTxFuncs); luaX_setfuncs(luTxFuncs); luaQ_setglobal("Txt");
  luaP_newmetatable   ("txt"); luaX_setfuncs(luTxMetaFuncs);      luaQn_pop(1);
}
static void luasN_setTxt_this (txt *Tx) //- - - - - - - - - - - - - - - - - - -
{
  luaP_getglobal  ("Txt"); // Txt.this = Txt[ Ttxt->luaTxid ] or nil (clear)
  luaP_pushstring("this"); //
  if (Tx) luaP_rawgeti(-2,Tx->luaTxid);   // to avoid adding metatable to Ttxt
  else    luaP_pushnil();                 // just to cover one rarely used ref,
          luaQQ_rawset(-3); luaQn_pop(1); // adding "this" only in the luasExec
}                                         // scope (^J,^J and Shift^J commands)
//-----------------------------------------------------------------------------
static const char abc[] = "0123456789+ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "-abcdefghijklmnopqrstuvwxyz";
static int Nabc (const char c)
{
  const char *p = strchr(abc,c); return p ? (p-abc) : 0;
}
static inline char encode1(int  N, char k) { return  abc[(N + Nabc(k)) & 63]; }
static inline long decode1(char x, char k) { return (Nabc(x) - Nabc(k)) & 63; }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luasEncode (lua_State *L)
{
  size_t tlen; const char *orig_text = luaL_checklstring(L,1,&tlen);
  size_t klen; const char       *key = luaL_checklstring(L,2,&klen);
  if (klen < 1) luaP_pushstring("bad-key");
  else {
    uchar   *text = (uchar*)malloc((tlen+2)/3*3);
    for (uchar *p = (uchar*)blkmov((void*)orig_text,text,tlen);
                                              tlen % 3; tlen++) *p++ = ' ';
    char *encoded = (char*)malloc(4*tlen/3),
             *out = encoded;
    for (size_t i = 0, k = 0; i < tlen; i += 3) {
      long N = (text[i] << 16) | (text[i+1] << 8) | text[i+2];
      *out++ = encode1((N & 0xFC0000) >> 18, key[k++ % klen]);
      *out++ = encode1((N & 0x03F000) >> 12, key[k++ % klen]);
      *out++ = encode1((N & 0x000FC0) >>  6, key[k++ % klen]);
      *out++ = encode1((N & 0x00003F),       key[k++ % klen]);
    }
    luaP_pushlstring(encoded,out-encoded);
                free(text); free(encoded);
  } return 1;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luasDecode (lua_State *L)
{
  size_t tlen; const char *text = luaL_checklstring(L,1,&tlen);
  size_t klen; const char  *key = luaL_checklstring(L,2,&klen);
       if (tlen % 4) luaP_pushstring("bad-length");
  else if (klen < 1) luaP_pushstring("bad-key");
  else {
    char *decoded = (char*)malloc(3*tlen/4),
             *out = decoded;
    for (size_t i = 0, k = 0; i < tlen; i += 4) {
      long N = decode1(text[i],   key[k++ % klen]) << 18;
      N |=     decode1(text[i+1], key[k++ % klen]) << 12;
      N |=     decode1(text[i+2], key[k++ % klen]) <<  6;
      N |=     decode1(text[i+3], key[k++ % klen]);
      *out++ = ((N >> 16) & 0xFF);
      *out++ = ((N >>  8) & 0xFF); *out++ = N & 0xFF;
    }
    luaP_pushlstring(decoded,out-decoded);
                            free(decoded);
  } return 1;
}
static void luEndecFn(void) //- - - - - - - - - - - - - - - - - - - - - - - - -
{
  luaP_pushCfunction(luasEncode); luaQ_setglobal("encode");
  luaP_pushCfunction(luasDecode); luaQ_setglobal("decode");
}
//-----------------------------------------------------------------------------
void luasInit(void)                            // Lua SCRIPTING initialization
{
  L = luaL_newstate();
      luaL_openlibs(L); MkInitCCD(); // defines "Mk" table (for Lua and itself)
                        luaReInit(); // defines "Re" function (obj constructor)
                        luTxtInit(); // defines "Txt" table
                        luEndecFn(); // defines "encode" and "decode" functions
  luaP_newtable();
  luaQ_setglobal("MkLF"); // table "MkLF" initially empty, user may add filters
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  txt *autoLua = tmDesc(":/auto.lua", false);   // do not need UNDO (read-only)
  if (autoLua) { tmLoad  (autoLua);
                 luasExec(autoLua, false); }
  // Order:
  // - built-in auto.lua (most static of them all, and should be overridable)
  // - configurable script from MicroMir preferences
  // - local host auto-load executable script (supposed to be most volatile)
  //
  if (!MiApp_autoLoadLua.isEmpty()) {
    int rc = luaL_dostring(L, MiApp_autoLoadLua.uStr());
    if (rc) fprintf(stderr, "LuaERROR: %s\n", lua_tostring(L,-1));
  }
  txt *alesLua = tmDesc(ALESFILNAM, true); // with UNDO
  if (alesLua) { tmLoad  (alesLua);
                 luasExec(alesLua, false); }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luasExec (txt *Tx, bool just1line) // load/execute given text …(tx, false),
{                                      // or current line in Ttxt …(Ttxt,true);
  const char *buffer;
  int len;
  if (just1line && Tx == Ttxt) {             TxSetY (Ttxt,   Ty);
                                    buffer = TxGetLn(Ttxt, &len); }
  else {        TxTop(Tx);
    while (!qTxBottom(Tx)) {        char *pl = TxGetLn(Tx, &len);
      if (*pl == '^' || *pl == ACPR || *pl == '\xE2') TxDL  (Tx);
      else                                            TxDown(Tx);
    }
    buffer = Tx->txustk->dbeg;
    len    = Tx->txustk->dend - buffer;
  }                                        luasN_setTxt_this(Tx);
  int rc = luaL_loadbuffer(L, buffer, len, Tx->file->name.uStr())
              || lua_pcall(L,0,0,0);     luasN_setTxt_this(NULL);
  if (rc) {
    const char *luaError = lua_tostring(L,-1);
    QRegExp re("\\[.+\\]:(\\d+):(.+)");
    if (re.exactMatch(luaError)) {
      QString mimErr = Utf8("^" AF_ERROR "%1" AF_NONE).arg(re.cap(2));
      if (just1line) TxSetY(Ttxt, Ty + 1);
      else           TxSetY(Tx, re.cap(1).toInt());
      TxIL (Tx,  mimErr.uStr(),   mimErr.length());
    }
    else vipError(QString("LuaERROR: %1").arg(lua_tostring(L,-1)));
    return E_SFAIL;
  }
  else if (Tx == Ttxt) Twnd->sctw->DisplayInfo("ok");  return E_OK;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luasFunc (void)       // executing Lua function (from the top of Lua stack)
{                         // for consistency, always run it outside of LE mode,
  if (Lwnd) ExitLEmode(); // consider it a single operation in future Undo ops
  UndoMark = true;
  luaP_getglobal("Txt"); luaX_rawgeti_top(Ttxt->luaTxid);
  if (KbRadix)
       luaP_pushinteger(KbCount);         // function(Ttxt,kbCount/nil)
  else luaP_pushnil();                    //   no return if Ok
  if (luaQX_pcall(2,0) == 0) return E_OK; //   error msg if fails
  else {                                  //
    if (!lua_isstring(L,-1)) return E_KBREAK;
    vipError(QString("LuaERROR: %1").arg(lua_tostring(L,-1)));
    luaQn_pop(1);                              return E_SFAIL;
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void luas2txtProc (const char *proc_name, txt *Ref, txt *Txt)
{
  luaP_getglobal(proc_name);
  luaP_getglobal("Txt"); luaX_rawgeti_top(Ref->luaTxid);
  luaP_getglobal("Txt"); luaX_rawgeti_top(Txt->luaTxid);
  int rc = luaQX_pcall(2,0);
  if (rc)
    vipError(QString("LuaERROR: %1").arg(lua_tostring(L,-1)));
}
//------------------------------------------------------//---------------------
static txt *tLF = NULL;                                 // --- Load Filters ---
static int luLF_addText(lua_State*)                     //---------------------
{                                     size_t len;
  const char *text = luaL_checklstring(L,1,&len);
  size_t gap = cpdiff(tLF->txdstk->dbeg, tLF->txustk->dend);
  if (len+MAXTXRM > gap)      luaL_error(L,"out-of-memory"); int nLF = 0;
  for (const char *p = text; (p = strchr(p,'\n')) != NULL; p++)    nLF++;
  DqAddE(tLF->txustk, (char*)text, len);
  tLF->txy += nLF+1;           return 0;
}
// MkLF["filter"] = function(filename,addText) -- this function will be called
//   for str in io.lines(filename) do          -- for filenames specified with
//     -- filter                               -- ":filter" suffix, shall read
//     addText(str)                            -- everything by itself and add
//   end                                       -- the content line by line via
// end                                         -- addText function provided
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luLF_read (txt *t) // function returns: -1:fail, 0:empty, 1:truncated, 2:ok
{
  luaP_getglobal("MkLF");
  luaX_getfield_top(t->file->lf.uStr()); // call MkLF["name"] with (file,addFn)
  if (lua_isfunction(L,-1)) {            //
    luaP_pushstring(t->file->full_name.uStr());                   tLF = t;
    luaP_pushcfunction(luLF_addText); extgap(t->txustk, DqFree(), false );
    int rc = luaQX_pcall(2,0);            bool empty = qDqEmpt(t->txustk);
    if (rc == 0) return empty ? 0 : 2;
    else {
      QString err = Utf8("«..%1..»").arg(lua_tostring(L,-1));
      DqAddE(t->txustk, err.uStr(),err.size()); luaQn_pop(1);
      tLF->txy++;                    return  1;
  } }
  else {
    fprintf(stderr, "MkLF[%s] not found\n", t->file->lf.uStr());
    luaQn_pop(1);                                     return -1;
} }
//-----------------------------------------------------------------------------

/*------------------------------------------------------+----------------------
// МикроМир07          Embedded Lua scripting           | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#include <QString>
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
#include "tx.h"
}
lua_State *L = NULL;
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
//+
//  fprintf(stderr, "luasNtxt(%d,file=%s,top=%d)\n", atNextInst,
//                                    newTxt->file->name.cStr(), iTxt);
//-
  luaP_pushinteger(newTxt->luaTxid = atNextInst++);
  luaP_newtable();
  luTxtID *newTxtID = (luTxtID*)luaP_newuserdata(sizeof(luTxtID));
           newTxtID->t  = newTxt;
           newTxtID->id = newTxt->luaTxid; luaQ_setfield(-2,"id");
//
// Set metatable for the new instance == Txt._mt (handles Tx.X, Tx.Y etc)
//
  luaP_pushvalue (iTxt); luaX_getfield_top("_mt");
  luaQ_setmetatable(-2);
  luaQQ_rawset   (iTxt); luaQn_pop(1);
}
static void luasN_setTxt_this (txt *Tx) //- - - - - - - - - - - - - - - - - - -
{
  luaP_getglobal  ("Txt"); // Txt.this = Txt[ Ttxt->luaTxid ] or nil (clear)
  luaP_pushstring("this"); //
  if (Tx) luaP_rawgeti(-2,Tx->luaTxid);
  else    luaP_pushnil();
          luaQQ_rawset(-3); luaQn_pop(1);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static txt *luasN_gettext (int ix)  // returns txt from Lua reference on given
{                                   // index in the stack (verifies id match)
  luaP_getfield(ix,"id");
  luTxtID *txtID = (luTxtID*)lua_touserdata(L,-1);
  if (txtID == NULL ||
      txtID->id != txtID->t->luaTxid) luaL_error(L,"bad txt reference");
  luaQn_pop(1);
  if (txtID->t == Ttxt) { Ttxt->vp_ctx = Tx;
                          Ttxt->vp_cty = Ty; }  return txtID->t;
}
static void luas_UpdateTxy (txt *t, bool changed = false)
{
  if (changed) t->txstat |= TS_CHANGED; if (t == Ttxt) { Tx = Ttxt->vp_ctx;
                                                         Ty = Ttxt->vp_cty; }
}
//-----------------------------------------------------------------------------
// Text lazy constructor by text name or bool flags: true = "real text" (save)
//                                                   false = not real (discard)
//   Tx.open("name"/true/false) --> text object
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxOpen (lua_State *L)
{
  QString tname; bool init = false;
  if (lua_isboolean(L,1))
     { tname = lua_toboolean(L,1) ? QfsEMPTY_NAME : QfsELLIPSIS; init = true; }
  else tname = luaL_checkstring(L,1);
  vipFocusOff(Twnd); Twnd = vipSplitWindow(Twnd, TM_VFORK);
  if (Twnd && twEdit(Twnd, tname)) {
    if (init) luasNtxt(Ttxt); // tmDoLoad does nothing for ÷/… files, add table
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
//   Tx.maxY    = max valid value of Y  (= total number of lines in given text)
//   Tx:go(dy)
//   Tx:go(dx,dy) = convenience methods to move cursor around ('cause Lua does
//                                      not allow 'Tx.Y++' or even 'Tx.Y += k')
// NOTE:
//   attempt to move cursor past out of text borders does not generate any
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
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxLines (lua_State *)
{
  luasN_gettext(1);
  luaP_pushCfunction(luTxLine_it); // function = luTxLine_it(self,N)
  luaP_pushvalue  (1);             //    state = text descriptor
  luaP_pushinteger(0); return 3;   // init.var = 0 (before 1st line)
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxIndex (lua_State *L) // Tx.X and Tx.Y - read/write access
{                                   // Tx.maxY       - read-only
  txt *t = luasN_gettext(1);
  const char *var = luaL_checkstring(L,2);
       if (strcmp(var,"X")    == 0) luaP_pushinteger(t->vp_ctx+1);
  else if (strcmp(var,"Y")    == 0) luaP_pushinteger(t->vp_cty+1);
  else if (strcmp(var,"maxY") == 0) luaP_pushinteger(t->maxTy);
  else {
    luaP_getglobal ("Txt"); // refer to the base Txt table for everything else
    luaX_getfield_top(var); // (if field is not there, let Lua generate errors
  }                         // by itself)
  return 1;
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
  else { luaQQ_rawset(1); return 0; } // fall back to (raw) Lua settable
  luas_UpdateTxy(t);      return 0;   //      for all unknown properties
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxGo (lua_State *L) // Tx:go([dx,]dy) == Tx.go(Tx,[dx,]dy)
{                                //   if dx not specified, default to 0
  txt *t = luasN_gettext(1);
  int dx = 0, dy, k = 2;
  if (lua_gettop(L) > 2) dx = luaL_checkinteger(L,k++);
                         dy = luaL_checkinteger(L,k  );
  t->vp_ctx = my_max(my_min(t->vp_ctx + dx,0),t->txrm);
  t->vp_cty = my_max(my_min(t->vp_cty + dy,0),t->maxTy);
  luas_UpdateTxy(t);                           return 0;
}
//-----------------------------------------------------------------------------
// Methods of Txt class, text modifications:
//                                                 +-- unlike built-in MicroMir
//   Tx:IC("text") = insert text into current line | commands, move cursor past
//   Tx:IL("line") = insert given line at cursor   | inserted text
//   Tx:DC(N) = delete N characters at cursor
//   Tx:DL(N) = delete N lines at cursor (default = 1)
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
static int luTxIL (lua_State *L) // Tx:IL("text") = insert given line at cursor
{
  txt *t = luasN_gettext(1);          size_t len;
  const char *text = luaL_checklstring(L,2,&len);
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
luaL_Reg luTxFuncs[] =
{                         { "go", luTxGo }, { "gtl", luGetTextLeft },
  { "open",  luTxOpen  }, { "IL", luTxIL },
  { "focus", luTxFocus }, { "IC", luTxIC },
  { "line",  luTxLine  }, { "DL", luTxDL },
  { "lines", luTxLines }, { "DC", luTxDC }, { NULL, NULL   }

};
luaL_Reg luTxMetaFuncs[] =
{
  { "__index", luTxIndex }, { "__newindex", luTxNewindex }, { NULL, NULL }
};
//-----------------------------------------------------------------------------
void luasInit(void)
{
  L = luaL_newstate();
      luaL_openlibs(L); MkInitCCD(); // defines "Mk" table (for Lua and itself)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Txt table
//   open(tn/f) - lazy constructor, by text name or bool flag (true/false)
//   functions  - other instance functions (line,lines,go,IC,IL,DC,DL)
//   _mt        - metatable for instance objects (class methods & inst vars)
//     __index    - read access to instance variables (X,Y,maxY;ref->Txt)
//     __newindex - write access to inst variables (X,Y only)
//   this       - reference to the currently active text (Ttxt), luasExec only
//   [luaTxid]  - active texts are stored here (idexed by txt->luaTxid)
//     [k].id   -- instance id as userdata == { &txt_tag, txt_tag.luaTxid }
//
  luaP_newtable(); luaL_register(L,NULL,luTxFuncs);
  luaP_pushstring("_mt");
  luaP_newtable(); luaL_register(L,NULL,luTxMetaFuncs); luaQQ_settable(-3);
  luaQ_setglobal("Txt");
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  txt *autoLua = tmDesc(":/auto.lua", false); // do not need UNDO (read-only)
  if (autoLua) { tmLoad  (autoLua);
                 luasExec(autoLua,false); }
//
  if (!MiApp_autoLoadLua.isEmpty()) {
    int rc = luaL_dostring(L, MiApp_autoLoadLua.uStr());
    if (rc) fprintf(stderr, "LuaERROR: %s\n", lua_tostring(L,-1));
} }
//-----------------------------------------------------------------------------
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
    QString mimErr;
    if (re.exactMatch(luaError)) {      TxSetY(Tx, re.cap(1).toInt());
      QString mimErr = Utf8("^" AF_ERROR "%1" AF_NONE).arg(re.cap(2));
      TxIL(Tx, mimErr.uStr(), mimErr.length());
    }
    else vipError(QString("LuaERROR: %1").arg(lua_tostring(L,-1)));
    return E_SFAIL;
  }
  else if (Tx == Ttxt) Twnd->sctw->DisplayInfo("ok");     return E_OK;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luasFunc (void)       // executing Lua function (from the top of Lua stack)
{                         // (for consistency always run it outside of LE mode)
  if (Lwnd) ExitLEmode();
  luaP_getglobal ("Txt"); luaX_rawgeti_top(Ttxt->luaTxid);
  if (KbRadix)
       luaP_pushinteger(KbCount);         // function(Ttxt,kbCount/nil)
  else luaP_pushnil();                    //   no return if Ok
  if (luaQX_pcall(2,0) == 0) return E_OK; //   errof msg if fails
  else {
    vipError(QString("LuaERROR: %1").arg(lua_tolstring(L,-1,0)));
    luaQn_pop(1);                                 return E_SFAIL;
} }
//-----------------------------------------------------------------------------

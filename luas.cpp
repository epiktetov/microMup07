/*------------------------------------------------------+----------------------
// МикроМир07          Embedded Lua scripting           | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#include <QString>
#include <QRegExp>
#include "mic.h"
#include "ccd.h"
#include "lua.hpp"
#include "luas.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
extern "C" {
#include "dq.h"
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
static void luasN_setTxt_this (bool set_to_Ttxt) // - - - - - - - - - - - - - -
{
  luaP_getglobal  ("Txt"); // Txt.this = Txt[ Ttxt->luaTxid ] or nil (clear)
  luaP_pushstring("this"); //
  if (set_to_Ttxt) luaP_rawgeti(-2,Ttxt->luaTxid);
  else             luaP_pushnil();
  luaQQ_rawset(-3);  luaQn_pop(1);
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
static void luas_UpdateTxy (txt *t) { if (t == Ttxt) { Tx = Ttxt->vp_ctx;
                                                       Ty = Ttxt->vp_cty; } }
//-----------------------------------------------------------------------------
// Text lazy constructor by text name or bool flags: EMPTY = "real text" (true)
//                                                   PSEUDO = not real  (false)
//   Tx.open("name"/true/false) --> text object
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxOpen (lua_State *L)
{
  QString tname; bool init = false;
  if (lua_isboolean(L,1))
     { tname = lua_toboolean(L,1) ? QfsEMPTY_NAME : QfsELLIPSIS; init = true; }
  else tname = luaL_checkstring(L,1);
  if (tmStart(tname)) {
    if (init) luasNtxt(Ttxt); // tmDoLoad does nothing for ÷/… files, add table
    luaP_getglobal("Txt");
    luaX_rawgeti_top(Ttxt->luaTxid); return 1;
  }
  else return luaL_error(L,"cannot open %s", tname.uStr());
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
  long Y = (long)luaL_optnumber(L,2,t->vp_cty);
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
  txt *t = luasN_gettext(1);
  const char *var = luaL_checkstring(L,2);
  long N =    (long)luaL_checknumber(L,3);
       if (strcmp(var,"X") == 0) t->vp_ctx = my_max(my_min(N-1,0),t->txrm);
  else if (strcmp(var,"Y") == 0) {
    if (N < 0) t->vp_cty = my_min(t->maxTy-N+1,0);
    else       t->vp_cty = my_max(my_min(N-1,0),t->maxTy);
  }
  else return luaL_error(L,"unknown property");
  luas_UpdateTxy(t);                  return 0;
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
  return luaL_error(L,"not implemented");
}
static int luTxIL (lua_State *L) // Tx:IL("text") = insert given line at cursor
{
  txt *t = luasN_gettext(1);          size_t len;
  const char *text = luaL_checklstring(L,2,&len);
  TxSetY(t, t->vp_cty++);
  TxIL  (t, (char*)text, len); luas_UpdateTxy(t); return 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luTxDC (lua_State *L) // Tx:DC(N) = delete N character (default = 1)
{
  return luaL_error(L,"not implemented");
}
int luTxDL (lua_State *L) // Tx:DL(N) = delete N lines (default = 1) at cursor
{
  txt *t = luasN_gettext(1);
  int N = luaL_optinteger(L,2,1);
  TxSetY(t, t->vp_cty);
  while (!qTxBottom(t) && N--) TxDL(t); return 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
luaL_Reg luTxFuncs[] =
{
  { "open",  luTxOpen  }, { "IL", luTxIL },
  { "line",  luTxLine  }, { "IC", luTxIC },
  { "lines", luTxLines }, { "DL", luTxDL },
  {    "go", luTxGo    }, { "DC", luTxDC }, { NULL, NULL   }
};
luaL_Reg luTxMetaFuncs[] =
{
  { "__index", luTxIndex }, { "__newindex", luTxNewindex }, { NULL, NULL }
};
//-----------------------------------------------------------------------------
void luasInit(void)
{
  L = luaL_newstate();
      luaL_openlibs(L); MkInitCCD();
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Txt table
//   open(tn/f) - lazy constructor, by text name or int flags (EMPTY/PSEUDO)
//   EMPTY      - empty unnamed text (= true)
//   PSEUDO     - empty pseudo-text (= false)
//   functions  - other instance functions (line,lines,go,IC,IL,DC,DL)
//   _mt        - metatable for instance objects (class methods & inst vars)
//     __index    - read access to instance variables (X,Y,maxY;ref->Txt)
//     __newindex - write access to inst variables (X,Y only)
//   this       - reference to the currently active text (Ttxt), luasExec only
//   [luaTxid]  - active texts are stored here (idexed by txt->luaTxid)
//     [k].id   -- instance id as userdata == { &txt_tag, txt_tag.luaTxid }
//
  luaP_newtable(); luaL_register(L,NULL,luTxFuncs);
  luaP_pushboolean(1);  luaQ_setfield(-2, "EMPTY");
  luaP_pushboolean(0);  luaQ_setfield(-2,"PSEUDO");
  luaP_pushstring("_mt");
  luaP_newtable(); luaL_register(L,NULL,luTxMetaFuncs); luaQQ_settable(-3);
  luaQ_setglobal("Txt");
}
//-----------------------------------------------------------------------------
int luasExec (void)
{
  int len;    TxTop(Ttxt);
  while (!qTxBottom(Ttxt)) {
    char *pl = TxGetLn(Ttxt, &len);
    if (*pl == '^' || *pl == ACPR || *pl == '\xE2') TxDL  (Ttxt);
    else                                            TxDown(Ttxt);
  }
  char *buffer = Ttxt->txustk->dbeg;
  len = Ttxt->txustk->dend - buffer;    luasN_setTxt_this(true);
  int rc = luaL_loadbuffer(L,buffer,len,Ttxt->file->name.uStr())
              || lua_pcall(L,0,0,0);   luasN_setTxt_this(false);
  if (rc) {
    const char *luaError = lua_tostring(L,-1);
    QRegExp re("\\[.+\\]:(\\d+):(.+)");
    QString mimErr;
    if (re.exactMatch(luaError)) {    TxSetY(Ttxt, re.cap(1).toInt());
      QString mimErr = Utf8("^" AF_ERROR "%1" AF_NONE).arg(re.cap(2));
      TxIL(Ttxt, mimErr.uStr(), mimErr.length());
    }
    else vipError(QString("LuaERROR: %1").arg(lua_tolstring(L,-1,0)));
    return E_SFAIL;
  } return E_OK;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luasFunc (void)     // executing Lua-defined function (on top of Lua stack)
{
  luaP_getglobal("Txt"); luaX_rawgeti_top(Ttxt->luaTxid);
  if (KbRadix)
       luaP_pushinteger(KbCount);         // function(Ttxt,kbCount/nil)
  else luaP_pushnil();                    //   no return if Ok
  if (luaQX_pcall(2,0) == 0) return E_OK; //   errof msg if fails
  else {
    vipError(QString("LuaERROR: %1").arg(lua_tolstring(L,-1,0)));
    luaQn_pop(1);                                 return E_SFAIL;
} }
//-----------------------------------------------------------------------------

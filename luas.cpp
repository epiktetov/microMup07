/*------------------------------------------------------+----------------------
// МикроМир07          Embedded Lua scripting           | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#include "mic.h"
#include "ccd.h"
#include <QString>
#include <QRegExp>
#include "lua.hpp"
#include "luas.h"
#include "twm.h"
#include "vip.h"
extern "C" {
#include "tx.h"
}
lua_State *L = NULL;
//-----------------------------------------------------------------------------
static int LFunc[TM_LUA_FnMAX - TM_LUA_FUNC0 + 1] = { 0 };
static int luMicomNewindex (lua_State *L)
{
  // 1st argument is a reference to Micom table itself (we don't need it here)
  //
  if (lua_isnumber(L,2) && lua_isfunction(L,3)) {
    int key = (int)lua_tonumber(L,2);
    int  mk = MicomGetMapL(key);
    if (!mk) {
      for (int  N = TM_LUA_FUNC0; N <= TM_LUA_FnMAX; N++)
        if (LFunc[N-TM_LUA_FUNC0] == 0)
          { LFunc[N-TM_LUA_FUNC0] = mk = N; break; }
    }
    if (!mk) luaL_error(L, "micom table is full");
    MicomSetMapL(key, mk);
    luaP_getglobal("Micom");
    luaP_getfield(-1, "_f");
    luaP_pushvalue(3);
    luaQ_rawseti(-2, mk-TM_LUA_FUNC0);
    luaQn_pop   (2);         return 0;
  }
  return luaL_error(L, "bad parameters");
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int luasFunc(int kcode)   // executing Lua-defined function as MicroMir command
{
  luaP_getglobal("Micom");
  luaP_getfield (-1,"_f");
  luaP_rawgeti  (-1, kcode-TM_LUA_FUNC0);
  if (KbRadix) luaP_pushinteger(KbCount);
  else         luaP_pushnil(); //
  int rc = luaQX_pcall (1, 0); // call function(count=N or nil)
  if (rc) {
    const char *luaErr = lua_tolstring(L,-1,0); luaQn_pop(1);
    fprintf(stderr, "LuaERROR: %s\n", luaErr); // TODO: handle error message
  }
  luaQn_pop(2); return rc ? E_SFAIL : E_OK;
}
//-----------------------------------------------------------------------------
//
//   Tx.line(N) = content of line N in current text (nil if past end-of-text)
//   Tx.bottom  = true if at the bottom-of-the text
//   Tx.x, Tx.y = current cursor position (setting new value will move cursor)
//
// NOTE: attempt to move cursor past end-of-text does not generate any error
// (cursor is moved to the bottom, that is, after the last line in the text)
//
//   Tx.IL("text" or nil) = insert given text or empty line at cursor position
//   Tx.DL()              = delete line at cursor position
//
static int luTxLine (lua_State *L)
{
  if (lua_gettop(L) == 1 && lua_isnumber(L,1)) {
    long Y = (long)lua_tonumber(L,1);
    int len;
    if (TxSetY(Ttxt, Y)) {
      char *line = TxGetLn(Ttxt, &len);
      luaP_pushlstring(line, len);
    }
    else luaP_pushnil(); return 1;
  }
  else return luaL_error(L,"incorrect argument");
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxIndex (lua_State *L)
{
  // 1st argument is a reference to Tx table itself (currently not used)
  //
  if (lua_isstring(L,2)) {
    const char *var = lua_tolstring(L,2,0);
         if (strcmp(var,"x") == 0) { luaP_pushinteger(Tx); return 1; }
    else if (strcmp(var,"y") == 0) { luaP_pushinteger(Ty); return 1; }
    else if (strcmp(var,"bottom") == 0) {
      TxSetY(Ttxt, Ty); if (qTxBottom(Ttxt)) luaP_pushboolean(1);
                        else                 luaP_pushnil();
      return 1;
  } }
  return luaL_error(L, "bad parameters");
}
static int luTxNewindex (lua_State *L)
{
  // 1st argument is a reference to Tx table itself (currently not used)
  //
  if (lua_isstring(L,2) && lua_isnumber(L,3)) {
    const char *var = lua_tolstring(L,2,0);
    long N = (long)lua_tonumber(L,3);
         if (strcmp(var,"x") == 0) { Tx = (short)N;  return 0; }
    else if (strcmp(var,"y") == 0) {
      TxSetY(Ttxt,N);           // NOTE: operation is successful even if given
      Ty = Ttxt->txy; return 0; // line does not exists (set to max available)
  } }
  return luaL_error(L, "bad parameters");
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int luTxIL (lua_State *L)
{
  if (lua_gettop(L) != 1) return luaL_error(L, "missing parameters");
  const char *text = "";
  size_t len;
       if (lua_isnil(L,1))                           len = 0;
  else if (lua_isstring(L,1)) text = lua_tolstring(L,1,&len);
  else                  return luaL_error(L,"bad parameter");
  TxSetY(Ttxt, Ty);
  TxIL  (Ttxt, (char*)text, len); return 0;
}
static int luTxDL (lua_State*) { if (TxSetY(Ttxt, Ty)) TxDL(Ttxt); return 0; }
//-----------------------------------------------------------------------------
void luasInit(void)
{
  L = luaL_newstate();
  luaL_openlibs(L);
  luaP_newtable();        // 'Micom' table: Micom[kcode] = function ... end
  luaP_newtable();        //
  luaQ_setfield(-2,"_f"); // (private) Micom._f[n] == reference to that func
  luaP_newtable();
  luaP_pushCfunction(luMicomNewindex);
  luaQ_setfield     (-2,"__newindex");
  luaQ_setmetatable (-2);
  luaQ_setglobal ("Micom");
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  luaP_newtable();              // 'Tx' table keeps functions to work with text
  luaP_pushCfunction(luTxLine); //   Tx.line(N)
  luaQ_setfield    (-2,"line"); //   Tx.IL(text)
  luaP_pushCfunction(luTxIL);   //   Tx.DL()
  luaQ_setfield    (-2,"IL");
  luaP_pushCfunction(luTxDL);
  luaQ_setfield    (-2,"DL");
  luaP_newtable();               // Metatable handles access to "member vars":
  luaP_pushCfunction(luTxIndex); //   Tx.x, Tx.y, Tx.bottom
  luaQ_setfield  (-2,"__index"); //
  luaP_pushCfunction(luTxNewindex);
  luaQ_setfield  (-2,"__newindex");
  luaQ_setmetatable (-2);
  luaQ_setglobal("Tx");
}
//-----------------------------------------------------------------------------
int luasExec (void)
{
  QString LS = "--\n";
  int len;
  for (TxTop(Ttxt); !qTxBottom(Ttxt); TxDown(Ttxt)) {
    char *pl = TxGetLn(Ttxt, &len);
    if (*pl == '^')                      TxDL(Ttxt);
    else LS.append(QString::fromAscii(pl,len)+"\n");
  }
  if (luaL_dostring(L, LS.cStr()) == 0) return E_OK;
  if (lua_gettop(L) > 0 && lua_isstring(L,-1)) {
    const char *luaErr = lua_tolstring(L,-1,0);
    QRegExp re("\\[.+\\]:(\\d+):(.+)");
    if (re.exactMatch(luaErr)) {            int N = re.cap(1).toInt();
      QString mimErr = Utf8("^" AF_ERROR "%1" AF_NONE).arg(re.cap(2));
      TxSetY(Ttxt, N-1);
      TxIL(Ttxt, mimErr.uStr(), mimErr.length());
  } }
  return E_SFAIL;
}
//-----------------------------------------------------------------------------

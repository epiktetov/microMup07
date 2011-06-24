/*------------------------------------------------------+----------------------
// МикроМир07          Embedded Lua scripting           | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#include "mic.h"
#include "ccd.h"
#include "luas.h"
#include "lua.hpp"
#include <QString>
#include <QRegExp>
#include "vip.h"
extern "C" {
#include "tx.h"
}
static lua_State *L = NULL;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static int lTxLine (lua_State *L)
{
  if (lua_gettop(L) == 1 && lua_isnumber(L, 1)) {
    long Y = (long)lua_tonumber(L, 1);
    if (TxSetY(Ttxt, Y)) {
      int len;
      char *line = TxGetLn(Ttxt, &len);
      lua_pushlstring(L, line, len);
    }
    else lua_pushstring(L, "<eof>"); return 1;
  }
  else { lua_pushstring(L, "incorrect argument"); return lua_error(L); }
}
//-----------------------------------------------------------------------------
static int LFunc[TM_LUA_FnMAX - TM_LUA_FUNC0 + 1] = { 0 };
static int lMicom_newindex (lua_State *L)
{
  // 1st argument is reference to Micom table itself (we don't need it)
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
    lua_getglobal(L, "Micom");
    lua_getfield (L, -1,"_f");
    lua_pushvalue(L, 3);
    lua_rawseti  (L, -2, mk-TM_LUA_FUNC0);
    lua_pop      (L, 2);         return 0;
  }
  return luaL_error(L, "bad parameters");
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int  luasFunc(int kcode)
{
  lua_getglobal(L, "Micom");
  lua_getfield (L, -1,"_f");
  lua_rawgeti  (L, -1, kcode-TM_LUA_FUNC0);
  lua_call     (L, 0, 0);    lua_pop(L, 2); return E_OK;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void luasInit(void)
{
  L = luaL_newstate();
  luaL_openlibs(L);
  lua_newtable (L); // 'Micom' table and its metatable
  lua_newtable (L);
  lua_setfield (L, -2, "_f");
  lua_newtable (L);
  lua_pushcfunction(L,  lMicom_newindex);
  lua_setfield     (L, -2, "__newindex");
  lua_setmetatable (L, -2);
  lua_setglobal    (L, "Micom");

  lua_pushcfunction(L, lTxLine); lua_setglobal(L, "TxLine");
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

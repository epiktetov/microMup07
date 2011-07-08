/*------------------------------------------------------+----------------------
// МикроМир07          Embedded Lua scripting           | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#ifndef LUAS_H_INCLUDED
#define LUAS_H_INCLUDED
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void luasInit(void);       /* initialization (also load/executes "auto.lua") */
void luasNtxt(txt*newTxt); /*  new-text hook (called from tmDoLoad, twm.cpp) */
int  luasExec(txt*, bool); /*  load/execute given text (current line) as Lua */
int  luasFunc(void); /* execute (and pop) Lua function from top of Lua stack */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef lua_h         /* the problem with Lua function names is that it's not */
extern lua_State *L; /* very clear how do they operate with the stack, fixin */
//
//  luaP_xxx  - function pushes 1 element to the stack
//  luaQ_xxx  - pops 1 element from the stack
//  luaX_xxx  - modifies element(s) on stack, but keeps the stack balanced
//  luaQQ_xxx - pops 2 elements
//  luaQn_xxx - pops multiple elements (depending on parameters)
//  luaQX_xxx - pops elements and then pushes some (complicated)
//
inline int luaQX_pcall(int nargs, int nres) // pops nargs+1 (args & function)
{                                           // pushes nres or 1 (err message)
  return lua_pcall(L,nargs,nres,0);         //
}
inline void luaP_getfield(int ix, const char *fn) { lua_getfield(L,ix,fn);  }
inline void luaP_getglobal(const char *name)      { lua_getglobal(L,name);  }
inline void luaP_getmetatable(const char *t)      { luaL_getmetatable(L,t); }
inline void luaP_newtable(void)                   { lua_newtable(L);        }
inline void *luaP_newuserdata(size_t len)  { return lua_newuserdata(L,len); }
inline int luaP_newmetatable(const char *tname)
{
  return luaL_newmetatable(L,tname); // creates metatable and adds to registry
}
inline void luaQn_pop(int n)                      { lua_pop(L,n);           }
inline void luaP_pushboolean            (int b)   { lua_pushboolean  (L,b); }
inline void luaP_pushCfunction(lua_CFunction f)   { lua_pushcfunction(L,f); }
inline void luaP_pushinteger  (lua_Integer   n)   { lua_pushinteger  (L,n); }
inline void luaP_pushnumber   (lua_Number    n)   { lua_pushnumber   (L,n); }
inline void luaP_pushnil(void)                    { lua_pushnil(L);         }
inline void luaP_pushlstring(const char *s, size_t len)
{
  lua_pushlstring(L,s,len); // makes (or reuse) an internal copy of the string
}
inline void luaP_pushstring(const char *s)        { lua_pushstring(L,s);    }
inline void luaP_pushvalue(int ix)                { lua_pushvalue(L,ix);    }
inline void luaQ_remove   (int ix)                { lua_remove   (L,ix);    }
inline void luaP_rawgeti (int ix, int n)          { lua_rawgeti  (L,ix, n); }
inline void luaQ_rawseti (int ix, int n)          { lua_rawseti  (L,ix, n); }
inline void luaQQ_rawset (int ix)                 { lua_rawset   (L,ix);    }
inline void luaQ_setfield(int ix, const char *fn) { lua_setfield (L,ix,fn); }
inline void luaQ_setglobal(const char *name)      { lua_setglobal(L, name); }
inline int luaQ_setmetatable(int ix)       { return lua_setmetatable(L,ix); }
inline void luaQQ_settable  (int ix)              { lua_settable    (L,ix); }
//
// Additional convenience methods:
//
inline bool luaX_optboolean(int ix, bool defaultValue)
{
  return lua_isboolean(L,ix) ? (bool)lua_toboolean(L,ix) : defaultValue;
}
inline void luaX_getfield_top (const char *fn)      { luaP_getfield(-1,fn);
                                                           luaQ_remove(-2); }
inline void luaX_rawgeti_top(int n) { luaP_rawgeti (-1,n); luaQ_remove(-2); }
#endif
/*---------------------------------------------------------------------------*/
#endif                                                    /* LUAS_H_INCLUDED */

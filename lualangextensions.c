#include "luaex.h"
#include<math.h>
#include "global.h"

#define lregt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)

static int lua_isnan (lua_State* l) {
lua_pushboolean(l, _isnan(luaL_checknumber(l,1)));
return 1;
}

static int lua_isinteger (lua_State* l) {
double d = luaL_checknumber(l,1);
lua_pushboolean(l, d==((int)d));
return 1;
}

static int lua_round (lua_State* l) {
double d  = luaL_checknumber(l,1);
int base = luaL_optint(l, 2, 0);
if (base) {
double x = pow(10,base);
d = round(d*x)/x;
}
else d = round(d);
lua_settop(l,0);
lua_pushnumber(l,d);
return 1;
}

static int lua_tobase (lua_State* l) {
char c[32];
long long n = luaL_checknumber(l,1);
int base = luaL_optint(l,2, 10);
lua_settop(l,0);
lua_pushstring(l, lltoa(n, c, base));
return 1;
}

static int lua_log2 (lua_State* l) {
double n = luaL_checknumber(l,1);
double base = luaL_optnumber(l, 2, 2);
lua_settop(l,0);
lua_pushnumber(l, log(n)/log(base));
return 1;
}

static int lua_intdiv (lua_State* l) {
lua_pushinteger(l, luaL_checkint(l,1) / luaL_checkint(l,2));
return 1;
}


int luaopen_langextensions  (lua_State* l) {
lua_getglobal(l, "math");
lua_pushnumber(l, 2.7182818284590452353602874713527);
lua_setfield(l, -2, "e");
lregt(l, "tobase", lua_tobase);
lregt(l, "log2", lua_log2);
lregt(l, "round", lua_round);
lregt(l, "intdiv", lua_intdiv);
lua_pop(l,1);
lua_register(l, "isnan", lua_isnan);
lua_register(l, "isinteger", lua_isinteger);
return 0;
}

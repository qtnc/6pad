#define UNICODE
#include<luaex.h>
#include<windows.h>
#include "consts.h"
#include "global.h"

#define lregt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)

static int lStrcvt (lua_State* L) {
int len=0;
const char* str = luaL_checklstring(L,1,&len);
int cpFrom = luaL_checkint(L,2);
int cpTo = luaL_optint(L,3,-1);
if (cpTo<0) {
cpTo = cpFrom;
cpFrom = guessEncoding(str);
}
if (cpFrom>=1200&&cpFrom<=1203) len/=2;
char* out = strncvt(str, len, cpFrom, cpTo, &len);
if (!out) return 0;
if (cpTo>=1200&&cpTo<=1203) len*=2;
lua_pushlstring(L, out, len);
if (abs(str-out)>4) free(out);
return 1;
}

static lGuessEncoding (lua_State* L) {
lua_pushinteger(L, guessEncoding(luaL_checkstring(L,1)));
return 1;
}

static int lStrlen (lua_State* L) {
int len=0;
const char* str = luaL_checklstring(L,1,&len);
int enc = luaL_optint(L,2,-1);
if (enc<0) enc = guessEncoding(str);

}

static int lStrchr (lua_State* l) {
const char* str = luaL_checkstring(l,1);
char ch=0;
if (lua_type(l,2)==LUA_TNUMBER) ch = luaL_checkint(l,2);
else ch = *luaL_checkstring(l,2);
int offset = luaL_optint(l,3,1) -1;
if (offset<0) offset+=strlen(str);
const char* re = strchr(str+offset,ch);
int re2 = (re&&re-str<strlen(str)? re-str : -1);
if (re2>=0) lua_pushinteger(l,re2+1);
return re2>=0? 1:0;
}

static int lStrrchr (lua_State* l) {
int len=0;
char* str = luaL_checklstring(l,1,&len);
char ch=0;
if (lua_type(l,2)==LUA_TNUMBER) ch = luaL_checkint(l,2);
else ch = *luaL_checkstring(l,2);
int offset = luaL_optint(l,3,0) -1;
if (offset<0) offset+=len;
int re = -1;
for (int i=offset; i>=0; i--) {
if (str[i]==ch) { re=i; break; }
}
if (re>=0) lua_pushinteger(l,re+1);
return re>=0? 1:0;
}

static int lStrpbrk (lua_State* l) {
const char* s1 = luaL_checkstring(l,1);
const char* s2 = luaL_checkstring(l,2);
int offset = luaL_optint(l,3,1) ;
if (offset<0) offset+=strlen(s1);
else offset--;
const char* s3 = strpbrk(s1+offset,s2);
lua_settop(l,0);
if (!s3) return 0;
lua_pushinteger(l,s3-s1+1);
return 1;
}

static int lFindFirstNotOf (lua_State* l) {
const char* s1 = luaL_checkstring(l,1);
const char* s2 = luaL_checkstring(l,2);
int offset = luaL_optint(l,3,1);
if (offset<0) offset+=strlen(s1);
else offset--;
const char* c = s1+offset;
while (*c&&strchr(s2,*c)) c++;
if (!*c) return 0;
lua_pushinteger(l, c-s1+1);
return 1;
}

static int lFindLastOf (lua_State* l) {
const char* s1 = luaL_checkstring(l,1);
const char* s2 = luaL_checkstring(l,2);
int offset = luaL_optint(l,3,-1);
if (offset<0) offset+=strlen(s1);
else offset--;
const char* c = s1+offset;
while (c>=s1&&!strchr(s2,*c)) c--;
if (c<s1) return 0;
lua_pushinteger(l, c-s1+1);
return 1;
}

static int lFindLastNotOf (lua_State* l) {
const char* s1 = luaL_checkstring(l,1);
const char* s2 = luaL_checkstring(l,2);
int offset = luaL_optint(l,3,-1);
if (offset<0) offset+=strlen(s1);
else offset--;
const char* c = s1+offset;
while (c>=s1&&strchr(s2,*c)) c--;
if (c<s1) return 0;
lua_pushinteger(l, c-s1+1);
return 1;
}

int luaopen_exstringapi (lua_State* l) {
lua_getglobal(l, "string");
lregt(l, "cfind", lStrchr);
lregt(l, "crfind", lStrrchr);
lregt(l, "findFirstOf", lStrpbrk);
lregt(l, "findLastOf", lFindLastOf);
lregt(l, "findFirstNotOf", lFindFirstNotOf);
lregt(l, "findLastNotOf", lFindLastNotOf);
lregt(l, "convert", lStrcvt);
lregt(l, "encoding", lGuessEncoding);
lua_pop(l,1);
return 0;
}

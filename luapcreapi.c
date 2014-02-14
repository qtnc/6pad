#define UNICODE
#include<luaex.h>
#include<windows.h>
#include "consts.h"
#include "global.h"
#include "pcre-functions.h"

#define OPT_RETURN_TABLE 0x01000000
#define OPT_EMPTY_OFFSET 0x02000000

#define lregt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)

static pcre* lpcre (lua_State* l, int idx, int oidx, int* options) {
char* s = luaL_checkstring(l,idx), *opts = luaL_optstring(l,oidx,NULL);
*options = PCRE_NEWLINE_ANY;
if (opts) for (; *opts; opts++) {
switch (*opts) {
case 'i' : case 'I' : *options |= PCRE_CASELESS; break;
case 'm' : case 'M' : *options |= PCRE_MULTILINE; break;
case 's' : case 'S' : *options |= PCRE_DOTALL; break;
case 'U' : *options |= PCRE_UNGREEDY; break;
case 'u' : *options |= PCRE_UTF8; break;
case 'd' : case 'D' : *options |= PCRE_DOLLAR_ENDONLY; break;
case 'a' : case 'A' : *options |= PCRE_ANCHORED; break;
case 'n' : *options |= PCRE_DUPNAMES; break;
case 'x' : *options |= PCRE_EXTENDED; break;
case 'X' : *options |= PCRE_EXTRA; break;
case 'F' : *options |= PCRE_FIRSTLINE; break;
case 'N' : case '0' : *options |= PCRE_NO_AUTO_CAPTURE; break;
case 'e' : case 'E' : *options |= PCRE_NOTEMPTY; break;
case 'b' : case 'B' : *options |= PCRE_NOTBOL; break;
case 'z' : case 'Z' : *options |= PCRE_NOTEOL; break;
case 'o' : case '1' : *options |= OPT_EMPTY_OFFSET; break;
case 't' : case 'T' : *options |= OPT_RETURN_TABLE; break;
case 'O' : case '+' : /* optimize; currently unsupported */ break;
default : luaL_argerror(l, idx, "invalid pattern options");
}}
pcre* regex = preg_compile(s, *options & PCRE_COMPILETIME_MASK | PCRE_COMPILETIME_FORCE);
if (!regex) {
char errormsg[1024]={0};
snprintf(errormsg, 1023, "syntax error in regular expression: %s at offset %d", pregerrormsg, pregerrorof+1);
luaL_argerror(l, idx, errormsg);
}
return regex;
}

static int pushresults (pcre* regex, lua_State* l, const char* str, int* match, int re, int options) {
int top = lua_gettop(l);
int nNames = 0;
pcre_fullinfo(regex, NULL, PCRE_INFO_NAMECOUNT, &nNames);
BOOL table = (options&OPT_RETURN_TABLE)!=0 || nNames>0;
if (table) lua_newtable(l);
int i; for (i=0; i<re; i++) {
int len = match[i*2+1] - match[i*2];
if (len>0 || !options&OPT_EMPTY_OFFSET) {
char* s = malloc(len+1);
memcpy(s, str+match[i*2], len);
s[len]=0;
lua_pushlstring(l, s, len);
free(s);
}
else lua_pushinteger(l, match[i*2]+1);
if (table) lua_rawseti(l, top+1, i);
}
if (nNames>0) {
int size = 0;
pcre_fullinfo(regex, NULL, PCRE_INFO_NAMEENTRYSIZE, &size);
typedef struct { unsigned char unused, index; char name[size -2]; } Name;
Name* names = NULL;
pcre_fullinfo(regex, NULL, PCRE_INFO_NAMETABLE, &names);
if (names && size>0) for (int i, j=0; j<nNames; j++) {
i = names[j].index;
int len = match[i*2+1] - match[i*2];
char* s = malloc(len+1);
memcpy(s, str+match[i*2], len);
s[len]=0;
lua_pushlstring(l, s,len);
lua_setfield(l, top+1, names[j].name);
free(s);
}}
return table? 1 : re;
}

static int pcrematch (lua_State* l) {
pcre* regex = NULL;
int options = 0, len=0;
const char* str = luaL_checklstring(l,1,&len);
if (!(regex = lpcre(l, 2, 3, &options))) return 0;
int start = luaL_optint(l,4,1);
if (start>0) start--;
else if (start<0) start += len;
int* match = 0;
int re = preg_match(regex, options, str, start, len, &match);
if (re<0 || !match) return 0;
lua_settop(l,0);
int retn = pushresults(regex, l, str, match, re, options);
pcre_free(regex);
return retn;
}

static int pcregmatch_closure (lua_State* l) {
int len=0;
const char* str = lua_tolstring(l,lua_upvalueindex(1), &len);
pcre* regex = lua_topointer(l, lua_upvalueindex(2));
int start = lua_tointeger(l,lua_upvalueindex(3));
int options = lua_tointeger(l, lua_upvalueindex(4));
int* match = 0;
int re = preg_match(regex, options, str, start, len, &match);
if (re<0 || !match) return 0;
lua_pushinteger(l, match[1]);
lua_replace(l, lua_upvalueindex(3));
lua_settop(l,0);
return pushresults(regex, l, str, match, re, 0);
}

static int pcregmatch (lua_State* l) {
pcre* regex = NULL;
int options = 0, len=0, size=0;
const char* str = luaL_checklstring(l,1,&len);
if (!(regex = lpcre(l, 2, 3, &options))) return 0;
pcre_fullinfo(regex, NULL, PCRE_INFO_SIZE, &size);
int start = luaL_optint(l,4,1);
if (start>0) start--;
else if (start<0) start += len;
lua_pushvalue(l,1);
void* regexCopy = lua_newuserdata(l, size);
memcpy(regexCopy, regex, size);
lua_pushinteger(l,start);
lua_pushboolean(l,options);
lua_pushcclosure(l, pcregmatch_closure, 4);
return 1;
}

static char* luaTableLookupReplacer   (const char* s, int* vector, int result, lua_State* l) {
int i = result>=2?1:0; 
int len = vector[i*2+1]-vector[i*2];
char* z = malloc(len+1);
memcpy(z, s+vector[i*2], len);
z[len]=0;
lua_pushstring(l,z);
free(z);
lua_gettable(l,3);
if (lua_isnoneornil(l,-1)) return NULL;
else {
const char* z = lua_tostring(l,-1);
lua_pop(l,1);
return z;
}}

static int pcrerepl (lua_State* l) {
pcre* regex = NULL;
int options = 0, len=0;
const char* str = luaL_checklstring(l,1,&len);
if (!(regex = lpcre(l, 2, 4, &options))) return 0;
const char* repl = 0;
BOOL table = FALSE;
if (lua_isstring(l,3)) repl = luaL_checkstring(l,3);
else if (lua_istable(l,3)) table = TRUE;
else if (!lua_isfunction(l,3)) luaL_typerror(l, 3, "string, table or function");
if (repl) {
int nReplaces = 0;
const char* result = preg_replace(regex, options, repl, str, 0, len, &nReplaces);
lua_pushstring(l,result);
lua_pushinteger(l, nReplaces);
pcre_free(regex);
if (result!=str) free(result);
return 2;
}
else if (table) {
int nReplaces=0;
const char* result = preg_replace_callback(regex, options, luaTableLookupReplacer, str, 0, len, &nReplaces, l);
lua_settop(l,0);
lua_pushstring(l,result);
lua_pushinteger(l,nReplaces);
if (result!=str) free(result);
pcre_free(regex);
return 2;
}
int top = lua_gettop(l);
char* f (pcre* rgx, const char* s, int* vector, int result, void* p) {
lua_settop(l,top);
lua_pushvalue(l,3);
int nParams = pushresults(rgx, l, s, vector, result, options);
lua_call(l, nParams, 1);
if (lua_isnoneornil(l,-1)) return 0;
return luaL_checkstring(l,-1);
}
int nReplaces=0;
const char* result = preg_replace_callback(regex, options, f, str, 0, len, &nReplaces, NULL);
lua_settop(l,0);
lua_pushstring(l,result);
lua_pushinteger(l,nReplaces);
if (result!=str) free(result);
pcre_free(regex);
return 2;
}

static int pcresearch (lua_State* l) {
pcre* regex = NULL;
int options = 0;
const char* str = luaL_checkstring(l,1);
if (!(regex = lpcre(l, 2, 3, &options))) return 0;
int start = luaL_optint(l,4,1);
if (start>0) start--;
else if (start<0) start += strlen(str);
int* match = 0;
int re = preg_match(regex, options, str, start, 0, &match);
pcre_free(regex);
if (re<0 || !match)  return 0;
lua_settop(l,0);
lua_pushinteger(l, match[0] +1);
lua_pushinteger(l, match[1] +1);
return 2;
}


static int pcrersearch (lua_State* l) {
pcre* regex = NULL;
int options = 0, len=0;
const char* str = luaL_checklstring(l,1,&len);
if (!(regex = lpcre(l, 2, 3, &options))) return 0;
int start = luaL_optint(l,4,-1);
if (start>0) start--;
else if (start<0) start += strlen(str);
int* match = 0;
int pos = 0, begin=-1, end=-1;
while (0<preg_match(regex, options, str, pos, 0, &match) && match && match[0]<start) {
begin = match[0];
end = match[1];
pos = match[0] +1;
}
pcre_free(regex);
if (begin<0 || end<0) return 0;
lua_settop(l,0);
lua_pushinteger(l, begin+1);
lua_pushinteger(l, end+1);
return 2;
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

int luaopen_pcreapi (lua_State* l) {
lua_getglobal(l, "string");
lregt(l, "pmatch", pcrematch);
lregt(l, "pgmatch", pcregmatch);
lregt(l, "pfind", pcresearch);
lregt(l, "prfind", pcrersearch);
lregt(l, "pgsub", pcrerepl);
lua_pop(l,1);
return 0;
}

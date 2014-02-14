#define UNICODE
#include <windows.h>
#include<luaex.h>
#include "global.h"
#include "consts.h"

#define gethandle(l,n) *((HWND*)lua_touserdata(l,n))
#define lregt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)
#define streq(a,b) (0==strcmp(a,b))

extern HWND win;
extern HINSTANCE hinst;

extern BOOL handleCloseHandler (HWND, int) ;

static int ch_index (lua_State* l) {
HWND h = gethandle(l,1);
if (lua_isnumber(l,2)) {
int pos = luaL_checkint(l,2);
if (pos==0) pos=-1;
if (pos<0) pos += SendMessage(h, LB_GETCOUNT, 0, 0);
else pos--;
int len = SendMessage(h, LB_GETTEXTLEN, pos, 0);
wchar_t* wstr = malloc(sizeof(wchar_t) * (len+1));
SendMessage(h, LB_GETTEXT, pos, wstr);
wstr[len]=0;
const char* str = strncvt(wstr, len, CP_UTF16, CP_UTF8, &len);
lua_settop(l,0);
lua_pushlstring(l, str, len);
free(str);
free(wstr);
return 1;
}
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "selectedIndex")) {
int cur = SendMessage(h, LB_GETCURSEL, 0, 0);
if (cur==LB_ERR || cur<0) lua_pushinteger(l,0);
else lua_pushinteger(l,cur+1);
return 1;
}
else if (streq(nm, "selectedItem")) {
int cur = SendMessage(h, LB_GETCURSEL, 0, 0);
if (cur==LB_ERR || cur<0) return 0;
int len = SendMessage(h, LB_GETTEXTLEN, cur, 0);
wchar_t* wstr = malloc(sizeof(wchar_t) * (len+1));
SendMessage(h, LB_GETTEXT, cur, wstr);
wstr[len]=0;
const char* str = strncvt(wstr, len, CP_UTF16, CP_UTF8, &len);
lua_settop(l,0);
lua_pushlstring(l, str, len);
free(str);
free(wstr);
return 1;
}
else if (streq(nm, "closed")) {
lua_pushboolean(l, IsWindow(h)!=0);
return 1;
}
else if (streq(nm, "onClose")) {
void* p = GetProp(h, L"onClose");
if (p) lua_pushluafunction(l,p);
else lua_pushnil(l);
return 1;
}
else if (streq(nm, "onAction")) {
void* p = GetProp(h, L"onAction");
if (p) lua_pushluafunction(l,p);
else lua_pushnil(l);
return 1;
}
else if (streq(nm, "onSelect")) {
void* p = GetProp(h, L"onSelect");
if (p) lua_pushluafunction(l,p);
else lua_pushnil(l);
return 1;
}
else if (streq(nm, "onContextMenu")) {
void* p = GetProp(h, L"onContextMenu");
if (p) lua_pushluafunction(l,p);
else lua_pushnil(l);
return 1;
}
else if (streq(nm, "hwnd")) {
lua_pushlightuserdata(l, h);
return 1;
}
//SUITE
return luaL_getmetafield(l, 1, nm);
}

static int ch_newindex (lua_State* l) {
HWND h = gethandle(l,1);
if (lua_isnumber(l,2)) {
int pos = luaL_checkint(l,2);
if (pos==0) pos=-1;
if (pos<0) pos += SendMessage(h, LB_GETCOUNT, 0, 0);
else pos--;
const char* str = luaL_checkstring(l,3);
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
int sel = SendMessage(h, LB_GETCURSEL, 0, 0);
SendMessage(h, LB_DELETESTRING, pos, 0);
SendMessage(h, LB_INSERTSTRING, pos, wstr);
SendMessage(h, LB_SETCURSEL, sel, 0);
free(wstr);
return 0;
}
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "selectedIndex")) {
int n = lua_tonumber(l,3);
if (n==0) n=-1;
else if (n<0) n += SendMessage(h, LB_GETCOUNT, 0, 0);
else n--;
SendMessage(h, LB_SETCURSEL, n, 0);
}
else if (streq(nm, "selectedItem")) {
const char* str = luaL_checkstring(l,3);
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
SendMessage(h, LB_SELECTSTRING, -1, wstr);
free(wstr);
}
else if (streq(nm, "onClose")) SetProp(h, L"onClose", lua_topointer(l,3));
else if (streq(nm, "onAction")) SetProp(h, L"onAction", lua_topointer(l,3));
else if (streq(nm, "onSelect")) SetProp(h, L"onSelect", lua_topointer(l,3));
else if (streq(nm, "onContextMenu")) SetProp(h, L"onContextMenu", lua_topointer(l,3));
//SUITE
else luaL_argerror(l, 2, "undefined property");
return 0;
}

static int ch_addmultitems (lua_State* l, int tidx, HWND h, int pos) {
int n=lua_objlen(l,tidx);
SendMessage(h, WM_SETREDRAW, FALSE, 0);
SendMessage(h, LB_INITSTORAGE, n, 64);
for (int i=1; i<=n; i++) {
lua_rawgeti(l, tidx, i);
const char* str = lua_tostring(l,-1);
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
SendMessage(h, LB_INSERTSTRING, pos, wstr);
if (pos>=0) pos++;
free(wstr);
lua_pop(l,1);
}
SendMessage(h, WM_SETREDRAW, TRUE, 0);
lua_settop(l,1);
return 1;
}

static int ch_additem (lua_State* l) {
HWND h = gethandle(l,1);
const char* str = NULL;
if (lua_type(l,2)!=LUA_TTABLE) str = luaL_checkstring(l,2);
int pos = luaL_optint(l,3,0) -1;
if (pos<-1) pos += 1 + SendMessage(h, LB_GETCOUNT, 0, 0);
if (!str) return ch_addmultitems(l, 2, h, pos);
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
SendMessage(h, LB_INSERTSTRING, pos, wstr);
free(wstr);
lua_settop(l,1);
return 1;
}

static int ch_deleteitem (lua_State* l) {
HWND h = gethandle(l,1);
int pos = luaL_optint(l,2,0);
if (pos==0) pos=-1;
if (pos<0) pos += SendMessage(h, LB_GETCOUNT, 0, 0);
else pos--;
SendMessage(h, LB_DELETESTRING, pos, 0);
lua_settop(l,1);
return 1;
}

static int ch_getlen (lua_State* l) {
HWND h = gethandle(l,1);
lua_settop(l,0);
lua_pushinteger(l, SendMessage(h, LB_GETCOUNT, 0, 0));
return 1;
}

static int ch_clear (lua_State* l) {
HWND h = gethandle(l,1);
lua_settop(l,0);
SendMessage(h, LB_RESETCONTENT, 0, 0);
return 0;
}

static int ch_find (lua_State* l) {
HWND h = gethandle(l,1);
const char* str = luaL_checkstring(l,1);
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
int pos = luaL_optint(l,2,1);
if (pos<0) pos += SendMessage(h, LB_GETCOUNT, 0, 0); 
else if (pos==0) pos = SendMessage(h, LB_GETCURSEL, 0, 0);
else pos--;
int re = SendMessage(h, LB_FINDSTRING, pos -1, wstr);
free(wstr);
if (re>=pos) {
int len = SendMessage(h, LB_GETTEXTLEN, re, 0);
wchar_t* wstr2 = malloc(sizeof(wchar_t) * (len+1));
SendMessage(h, LB_GETTEXT, re, wstr2);
wstr2[len]=0;
const char* str2 = strncvt(wstr2, len, CP_UTF16, CP_UTF8, &len);
lua_settop(l,0);
lua_pushinteger(l, pos+1);
lua_pushlstring(l, str2, len);
free(str2);
free(wstr2);
return 2;
}
return 0;
}

int luacommonhandle_setfocus (lua_State* l) {
HWND h = gethandle(l,1);
SetForegroundWindow(GetParent(h));
lua_settop(l,1);
return 1;
}

int luacommonhandle_close (lua_State* l) {
HWND h = gethandle(l,1);
HWND p = GetParent(h);
if (p) {
if (p==win) SendMessage(win, WM_DESTROY, 0, 0);
else SendMessage(p, WM_COMMAND, IDCANCEL, 0);
}}

int luaopen_choicehandleapi (lua_State* l) {
lua_newclass(l, "choicehandle");
lregt(l, "add", ch_additem);
lregt(l, "remove", ch_deleteitem);
lregt(l, "clear", ch_clear);
lregt(l, "find", ch_find);
lregt(l, "focus", luacommonhandle_setfocus);
lregt(l, "close", luacommonhandle_close);
lregt(l, "__len", ch_getlen);
lregt(l, "__index", ch_index);
lregt(l, "__newindex", ch_newindex);
lua_pop(l,1);
return 0;
}



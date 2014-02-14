#define UNICODE
#include<luaex.h>
#include<windows.h>
#include "consts.h"
#include "global.h"

#define lregt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)
#define gethandle(l,n) (*(HWND*)(lua_touserdata(l,n)))
#define streq(a,b) (0==strcmp(a,b))

extern HWND win, edit;

static DWORD prepareProgressDialog (HWND* hProgress) {
showProgressDialog(hProgress);
return 0;
}

static int ph_open (lua_State* l) {
HWND hProgress = 0;
newThread(prepareProgressDialog,&hProgress);
while (!hProgress) Sleep(1);
lua_settop(l,0);
lua_pushfulluserdata(l, &hProgress, sizeof(hProgress), "progress");
return 1;
}

static int ph_close (lua_State* l) {
HWND progress = gethandle(l,1);
SendMessage(progress, WM_USER, 1006, 1);
return 0;
}

static int ph_index (lua_State* l) {
HWND progress = gethandle(l,1);
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "title")) {
int len = GetWindowTextLength(progress);
wchar_t title[len+2];
GetWindowText(progress, title, len+1);
title[len]=0;
const char* ch = strncvt(title, len, CP_UTF16, CP_UTF8, NULL);
lua_settop(l,0);
lua_pushstring(l,ch);
free(ch);
return 1;
}
else if (streq(nm, "text")) {
int len = SendMessage(progress, WM_USER, 1003, 0);
wchar_t wch[len+2];
SendMessage(progress, WM_USER, 1003, wch);
wch[len]=0;
const char* ch = strncvt(wch, len, CP_UTF16, CP_UTF8, NULL);
lua_settop(l,0);
lua_pushstring(l,ch);
free(ch);
return 1;
}
else if (streq(nm, "value")) {
int n = SendMessage(progress, WM_USER, 1004, 0);
lua_settop(l,0);
lua_pushnumber(l, n/10000.0);
return 1;
}
else if (streq(nm, "cancelled")) {
lua_settop(l,0);
lua_pushboolean(l, SendMessage(progress, WM_USER, 1005, 0));
return 1;
}
else if (streq(nm, "hwnd")) {
lua_pushlightuserdata(l, progress);
return 1;
}
return luaL_getmetafield(l, 1, nm);
}

static int ph_newindex (lua_State* l) {
HWND progress = gethandle(l,1);
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "title")) {
const wchar_t* wc = strcvt(luaL_checkstring(l,3), CP_UTF8, CP_UTF16, NULL);
SetWindowText(progress, wc);
free(wc);
}
else if (streq(nm, "text")) {
const wchar_t* wc = strcvt(luaL_checkstring(l,3), CP_UTF8, CP_UTF16, NULL);
SendMessage(progress, WM_USER, 1001, wc);
free(wc);
}
else if (streq(nm, "value")) {
int value = 10000 * luaL_checknumber(l,3);
SendMessage(progress, WM_USER, 1002, value);
}
else luaL_argerror(l, 2, "undefined property");
return 0;
}

int __declspec(dllexport) luaopen_progressapi (lua_State* l) {
lua_newclass(l, "progress");
lregt(l, "open", ph_open);
lregt(l, "close", ph_close);
lregt(l, "__index", ph_index);
lregt(l, "__newindex", ph_newindex);
lua_pop(l,1);
return 0;
}


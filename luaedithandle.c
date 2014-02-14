#define UNICODE
#include<luaex.h>
#include<windows.h>
#include "consts.h"
#include "global.h"

#define lregt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)
#define gethandle(l,n) (*(HWND*)(lua_touserdata(l,n)))
#define streq(a,b) (0==strcmp(a,b))

extern HWND win, edit;
extern pagecontext* curPage;

extern BOOL handleCloseHandler (HWND, int) ;
extern int luacommonhandle_setfocus (lua_State* l) ;
extern int luacommonhandle_close (lua_State* l) ;

void editReplaceSel (HWND h, const wchar_t* s, int ss, int se, BOOL restoresel) {
int oldss, oldse, slen = wcslen(s);
if (h==edit) {
curPage->shouldAddUndo = TRUE;
editAboutToChange(0);
}
if (restoresel) {
SendMessage(h, EM_GETSEL, &oldss, &oldse);
if (ss>=-1) SendMessage(h, EM_SETSEL, ss, se);
}
SendMessage(h, EM_REPLACESEL, TRUE, s);
if (restoresel) {
if (oldss>ss) oldss += slen - abs(se-ss);
if (oldse>ss) oldse += slen - abs(se-ss);
SendMessage(h, EM_SETSEL, oldss, oldse);
}
SendMessage(h, EM_SCROLLCARET, 0, 0);
if (h==edit) {
SendMessage(win, WM_COMMAND, (EN_CHANGE<<16) | IDT_EDIT, edit);
updateStatusBar();
}}

static int eh_index (lua_State* l) {
HWND edit = gethandle(l,1);
if (lua_isnumber(l,2)) {
int num = luaL_checkint(l,2);
if (num==0) num = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
else if (num<0) num += SendMessage(edit, EM_GETLINECOUNT, 0, 0);
else num--;
int lidx = SendMessage(edit, EM_LINEINDEX, num, 0);
int ll = SendMessage(edit, EM_LINELENGTH, lidx, 0);
wchar_t* wc = malloc(sizeof(wchar_t) * (ll+1));
wc[0] = ll;
SendMessage(edit, EM_GETLINE, num, wc);
wc[ll]=0;
const char* mbc = strncvt(wc, ll, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,mbc);
free(mbc);
free(wc);
return 1;
}
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "text")) {
HLOCAL hEdit = SendMessage(edit, EM_GETHANDLE, 0, 0);
const wchar_t* wstr = LocalLock(hEdit);
const char* str = strcvt(wstr, CP_UTF16, CP_UTF8, NULL);
LocalUnlock(hEdit);
lua_pushstring(l,str);
free(str);
return 1;
}
else if (streq(nm,"selText") || streq(nm, "selectedText")) {
int ssel=0, esel=0;
SendMessage(edit, EM_GETSEL, &ssel, &esel);
HLOCAL hEdit = SendMessage(edit, EM_GETHANDLE, 0, 0);
const wchar_t* wstr = LocalLock(hEdit);
const char* str = strncvt(wstr+ssel, esel-ssel, CP_UTF16, CP_UTF8, NULL);
LocalUnlock(hEdit);
lua_pushlstring(l, str, esel-ssel);
free(str);
return 1;
}
else if (streq(nm, "selStart") || streq(nm, "selectionStart")) {
int k; SendMessage(edit, EM_GETSEL, &k, 0);
lua_pushinteger(l,k+1);
return 1;
}
else if (streq(nm, "selEnd") || streq(nm, "selectionEnd")) {
int k; SendMessage(edit, EM_GETSEL, 0, &k);
lua_pushinteger(l,k+1);
return 1;
}
else if (streq(nm, "currentLine")) {
lua_pushinteger(l, 1+SendMessage(edit, EM_LINEFROMCHAR, -1, 0));
return 1;
}
else if (streq(nm, "lineCount")) {
lua_pushinteger(l, SendMessage(edit, EM_GETLINECOUNT, 0, 0));
return 1;
}
else if (streq(nm, "readOnly")) {
lua_pushboolean(l, ES_READONLY&GetWindowLong(edit, GWL_STYLE));
return 1;
}
else if (streq(nm, "closed")) {
lua_pushboolean(l, IsWindow(edit)!=0);
return 1;
}
else if (streq(nm, "modified")) {
lua_pushboolean(l, 0!=SendMessage(edit, EM_GETMODIFY, 0, 0));
return 1;
}
else if (streq(nm, "onClose")) {
void* p = GetProp(edit, L"onClose");
if (p) lua_pushluafunction(l,p);
else lua_pushnil(l);
return 1;
}
else if (streq(nm, "onContextMenu")) {
void* p = GetProp(edit, L"onContextMenu");
if (p) lua_pushluafunction(l,p);
else lua_pushnil(l);
return 1;
}
else if (streq(nm, "hwnd")) {
lua_pushlightuserdata(l, edit);
return 1;
}
//SUITE
return luaL_getmetafield(l, 1, nm);
}

static int eh_newindex (lua_State* l) {
HWND hEdit = gethandle(l,1);
if (lua_isnumber(l,2)) {
int num = luaL_checkint(l,2);
if (num==0) num = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
else if (num<0) num += SendMessage(edit, EM_GETLINECOUNT, 0, 0);
else num--;
int lidx = SendMessage(edit, EM_LINEINDEX, num, 0);
int ll = SendMessage(edit, EM_LINELENGTH, lidx, 0);
const wchar_t* wc = strcvt(luaL_checkstring(l,3), CP_UTF8, CP_UTF16, NULL);
editReplaceSel(hEdit, wc, lidx, lidx+ll, TRUE);
free(wc);
return 0;
}
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "selText") || streq(nm, "selectedText")) {
const wchar_t* wc = strcvt(luaL_checkstring(l,3), CP_UTF8, CP_UTF16, NULL);
editReplaceSel(hEdit, wc, -2, -2, FALSE);
free(wc);
}
else if (streq(nm, "text")) {
const wchar_t* wc = strcvt(luaL_checkstring(l,3), CP_UTF8, CP_UTF16, NULL);
editReplaceSel(hEdit, wc, 0, -1, TRUE);
free(wc);
}
else if (streq(nm, "selStart") || streq(nm, "selectionStart")) {
int selStart, selEnd; SendMessage(hEdit, EM_GETSEL, &selStart, &selEnd);
selStart  = luaL_checkint(l,3);
if (selStart<=0) selStart += GetWindowTextLength(hEdit);
else selStart--;
SendMessage(hEdit, EM_SETSEL, selStart, selEnd);
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
if (edit==hEdit) updateStatusBar();
}
else if (streq(nm, "selEnd") || streq(nm, "selectionEnd")) {
int selStart, selEnd; SendMessage(hEdit, EM_GETSEL, &selStart, &selEnd);
selEnd = luaL_checkint(l,3);
if (selEnd<=0) selStart += GetWindowTextLength(hEdit);
else selEnd--;
SendMessage(hEdit, EM_SETSEL, selStart, selEnd);
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
if (edit==hEdit) updateStatusBar();
}
else if (streq(nm, "currentLine")) {
int num = luaL_checkint(l,3);
if (num==0) luaL_argerror(l, 3, "0 is not a correct value");
else if (num<0) num += SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
else num--;
int lidx = SendMessage(hEdit, EM_LINEINDEX, num, 0);
SendMessage(hEdit, EM_SETSEL, lidx, lidx);
SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
if (edit==hEdit) updateStatusBar();
}
else if (streq(nm, "readOnly")) {
SendMessage(hEdit, EM_SETREADONLY, luaL_checkboolean(l,3)!=0, 0);
}
else if (streq(nm, "modified")) {
SendMessage(hEdit, EM_SETMODIFY, luaL_checkboolean(l,3)!=0, 0);
}
else if (streq(nm, "onClose")) SetProp(hEdit, L"onClose", lua_topointer(l,3));
else if (streq(nm, "onContextMenu")) SetProp(hEdit, L"onContextMenu", lua_topointer(l,3));
//SUITE
else luaL_argerror(l, 2, "undefined property");
return 0;
}

static int eh_linecount (lua_State* l) {
HWND edit = gethandle(l,1);
lua_settop(l,0);
lua_pushinteger(l, SendMessage(edit, EM_GETLINECOUNT, 0, 0));
return 1;
}

static int eh_select (lua_State* l) {
HWND edit = gethandle(l,1);
int start = luaL_optint(l,2,0),
end = luaL_optint(l,3,0),
curStart, curEnd, len = GetWindowTextLength(edit);
SendMessage(edit, EM_GETSEL, &curStart, &curEnd);
if (start<0) start += len;
else if (start>0) start--;
else if (start==0) start = curStart;
if (end<0) end += len;
else if (end>0) end--;
else if (end==0) end = curEnd;
SendMessage(edit, EM_SETSEL, start, end);
lua_settop(l,0);
lua_pushinteger(l,start);
lua_pushinteger(l,end);
return 2;
}

static int eh_line2offset (lua_State* l) {
HWND edit = gethandle(l,1);
int num = luaL_optint(l,2,0);
if (num>0) num--;
else if (num<0) num += SendMessage(edit, EM_GETLINECOUNT, 0, 0);
else num = -1;
int re = SendMessage(edit, EM_LINEINDEX, num, 0);
lua_settop(l,0);
lua_pushinteger(l,re+1);
return 1;
}

static int eh_offset2line (lua_State* l) {
HWND edit = gethandle(l,1);
int num = luaL_optint(l,2,0);
if (num>0) num--;
else if (num<0) num += GetWindowTextLength(edit);
else num = -1;
int re = SendMessage(edit, EM_LINEFROMCHAR, num, 0);
lua_settop(l,0);
lua_pushinteger(l,re+1);
return 1;
}

static int eh_insert (lua_State* l) {
HWND edit = gethandle(l,1);
const char* mbs = lua_tostring(l,2);
if (!mbs) return 0;
int ss = luaL_optint(l,3,0), se = luaL_optint(l,4,0);
const wchar_t* ws = strcvt(mbs, CP_UTF8, CP_UTF16, NULL);
if (ss==0) editReplaceSel(edit, ws, -2, -2, FALSE);
else {
if (ss<0) ss += GetWindowTextLength(edit) +1;
else ss--;
if (se==0) se=ss;
else if (se<-1) se += GetWindowTextLength(edit) +1;
else se--;
editReplaceSel(edit, ws, ss, se, ss!=se);
}
free(ws);
lua_settop(l,1);
return 1;
}

static int eh_append (lua_State* l) {
HWND edit = gethandle(l,1);
const char* s = lua_tostring(l,2);
if (!s) return;
const wchar_t* ws = strcvt(s, CP_UTF8, CP_UTF16, NULL);
int k = GetWindowTextLength(edit);
editReplaceSel(edit, ws, k, k, FALSE);
free(ws);
lua_settop(l,1);
return 1;
}

int luaopen_editapi (lua_State* l) {
lua_newclass(l, "edithandle");
lregt(l, "focus", luacommonhandle_setfocus);
lregt(l, "close", luacommonhandle_close);
lregt(l, "insert", eh_insert);
lregt(l, "replace", eh_insert);
lregt(l, "append", eh_append);
lregt(l, "offsetOfLine", eh_line2offset);
lregt(l, "lineOfOffset", eh_offset2line);
lregt(l, "select", eh_select);
lregt(l, "__index", eh_index);
lregt(l, "__newindex", eh_newindex);
lregt(l, "__len", eh_linecount);
lua_pop(l,1);
return 0;
}


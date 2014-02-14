#define UNICODE
#include<luaex.h>
#include<windows.h>
#include "consts.h"
#include "global.h"

#define regt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)
#define streq(a,b) (0==strcmp(a,b))

extern BOOL callfunc (lua_State* l, void* p, int nArgs, int nRet) ;
extern pagecontext* curPage, **pages;
extern int nPages, curFlags;

extern HINSTANCE hinst;
extern HWND win, edit, status, tabctl;
extern CRITICAL_SECTION lcs;
extern lua_State* l;

extern wchar_t* globalFilenameFilter; extern int globalFilenameFilterLength;

static luafunc onBeforeOpen=0, onAfterOpen=0, onBeforeSave=0, onAfterSave=0, onBeforeTabChange=0, onAfterTabChange=0, onBeforeTabClose=0, onAfterTabClose=0, 
onEnter=0, onTab=0, onLineChange=0, onTabNew=0, onKeyDown=0, onKeyUp=0, onKeyPress=0,
onBeforeClose=0, onStatusBarChange=0, onContextMenu=0;

static pagecontext* tab_getFromArg (lua_State* l, int idx) {
lua_pushvalue(l,idx);
lua_gettable(l, LUA_REGISTRYINDEX);
pagecontext* p = lua_topointer(l,-1);
lua_pop(l,1);
return p;
}

static void tab_pushTable (lua_State* l, pagecontext* p) {
if (!p) { lua_pushnil(l); return; }
lua_pushlightuserdata(l,p);
lua_gettable(l, LUA_REGISTRYINDEX);
if (lua_isnoneornil(l,-1)) {
lua_pop(l,1);
lua_newtable(l);
lua_pushvalue(l,-1);
lua_pushlightuserdata(l,p);
lua_settable(l, LUA_REGISTRYINDEX);
lua_pushlightuserdata(l,p);
lua_pushvalue(l,-2);
lua_settable(l, LUA_REGISTRYINDEX);
lua_getfield(l, LUA_REGISTRYINDEX, "tabhandle");
lua_setmetatable(l, -2);
}}

static void tab_removeTable (lua_State* l, pagecontext* p) {
lua_pushlightuserdata(l,p);
lua_gettable(l, LUA_REGISTRYINDEX);
if (!lua_isnoneornil(l,-1)) {
lua_pushnil(l);
lua_settable(l, LUA_REGISTRYINDEX);
}
else lua_pop(l,1);
lua_pushnil(l);
lua_settable(l, LUA_REGISTRYINDEX);
}
void tablrelease (pagecontext* p) {
if (!p) return;
EnterCriticalSection(&lcs);
tab_removeTable(l,p);
#define E(x) p->x = luafixfunc(l, &(p->x), NULL);
E(onBeforeOpen) E(onAfterOpen) E(onBeforeSave) E(onAfterSave) 
E(onBeforeTabClose) E(onEnter) E(onTab) E(onLineChange) E(onKeyDown) E(onKeyPress) E(onKeyUp) E(onContextMenu) E(onStatusBarChange)
#undef E
LeaveCriticalSection(&lcs);
}

static BOOL evfunc1 (pagecontext* p, luafunc f1, luafunc f2) {
luafunc f = f1? f1 : f2;
if (!f) return TRUE;
BOOL re = TRUE;
EnterCriticalSection(&lcs);
lua_settop(l,0);
if (p) tab_pushTable(l, p);
else lua_pushnil(l);
if (callfunc(l, f, 1, LUA_MULTRET) && lua_isboolean(l,-1) && !lua_toboolean(l,1)) re=FALSE;
LeaveCriticalSection(&lcs);
return re;
}
#define E(x) BOOL ev_##x (pagecontext* p) { return evfunc1(p, p->x, x); }
#define G(x) BOOL ev_##x (pagecontext* p) { return evfunc1(p, x, NULL); }
E(onBeforeSave) E(onAfterSave) E(onBeforeOpen) E(onAfterOpen) E(onBeforeTabClose) E(onContextMenu) E(onStatusBarChange)
G(onBeforeTabChange) G(onAfterTabChange) G(onAfterTabClose) G(onTabNew) G(onBeforeClose)
#undef E
#undef G

static int ev_generalKey (pagecontext* p, int k, int kf, luafunc f1, luafunc f2) {
if (!p) return TRUE;
luafunc f = f1? f1 : f2;
if (!f) return TRUE;
int re = TRUE;
EnterCriticalSection(&lcs);
lua_settop(l,0);
lua_pushinteger(l,k);
lua_pushinteger(l,kf);
if (callfunc(l, f, 2, LUA_MULTRET)) {
switch(lua_type(l,-1)) {
case LUA_TNUMBER: re = lua_tointeger(l,-1); break;
case LUA_TBOOLEAN: re = !!lua_toboolean(l,-1); break;
case LUA_TSTRING: re = *lua_tostring(l,-1); break;
default: break;
}}
LeaveCriticalSection(&lcs);
return re;
}
#define E(x) int ev_##x (pagecontext* p, int k, int kf) { return p? ev_generalKey(p, k, kf, p->x, x) :TRUE; }
E(onKeyDown) E(onKeyUp) E(onKeyPress)
#undef E

intptr_t ev_onEnter (const wchar_t* line, int lineLen, int curline, int curchar, int lineOffset) {
luafunc f = curPage->onEnter? curPage->onEnter : onEnter;
if (!f) return 0;
intptr_t re = 0;
EnterCriticalSection(&lcs);
const char* str = strncvt(line, lineLen, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
lua_pushinteger(l,curline);
lua_pushinteger(l,curchar+1);
lua_pushinteger(l, lineOffset+1);
free(str);
if (callfunc(l, f, 4, LUA_MULTRET)) {
switch(lua_type(l,-1)) {
case LUA_TBOOLEAN: re = lua_toboolean(l,-1)? 0 : 101; break;
case LUA_TNUMBER: re = lua_tonumber(l,-1); break;
case LUA_TSTRING: re = strcvt(lua_tostring(l,-1), CP_UTF8, CP_UTF16, NULL); break;
default: break;
}}
LeaveCriticalSection(&lcs);
return re;
}

void ev_onTab (int spos, int pos, intptr_t* re1, intptr_t* re2) {
luafunc f = curPage->onTab? curPage->onTab : onTab;
if (!f) return;
int ln = SendMessage(edit, EM_LINEFROMCHAR, pos, 0);
int li = SendMessage(edit, EM_LINEINDEX, ln, 0);
int ll = SendMessage(edit, EM_LINELENGTH, pos, 0);
wchar_t wstr[ll+1];
wstr[0]=ll+1;
SendMessage(edit, EM_GETLINE, ln, wstr);
wstr[ll]=0;
const char* str = strncvt(wstr, ll, CP_UTF16, CP_UTF8, NULL);
EnterCriticalSection(&lcs);
lua_settop(l,0);
lua_pushstring(l,str);
lua_pushinteger(l,ln);
lua_pushinteger(l, spos+1);
lua_pushinteger(l,pos+1);
lua_pushinteger(l,li+1);
if (callfunc(l, f, 5, LUA_MULTRET)) {
int n = lua_gettop(l) -1;
if (lua_isboolean(l, -n)) *re1 = !!lua_toboolean(l,-n);
else if (lua_isstring(l,-n)) {
if (n>1 && lua_isboolean(l,1-n)) *re2 = !!lua_toboolean(l,1-n);
*re1 = strcvt(lua_tostring(l,-n), CP_UTF8, CP_UTF16, NULL);
}}
LeaveCriticalSection(&lcs);
free(str);
}

void ev_onLineChange (pagecontext* p, int dir) {
luafunc f = curPage->onLineChange? curPage->onLineChange : onLineChange;
if (!f) return;
EnterCriticalSection(&lcs);
lua_settop(l,0);
if (p) tab_pushTable(l, p);
else lua_pushnil(l);
lua_pushinteger(l,dir);
callfunc(l,f,2,LUA_MULTRET);
LeaveCriticalSection(&lcs);
}


BOOL ev_mapBoolEvent (const wchar_t* evt, HWND hwnd, int type) {
void* p = GetProp(hwnd, evt);
if (!p) return TRUE;
BOOL re = TRUE;
const char* typeS = (type==0? "choicehandle" : "edithandle");
EnterCriticalSection(&lcs);
lua_settop(l,0);
lua_pushfulluserdata(l, &hwnd, sizeof(hwnd), typeS);
if (callfunc(l, p, 1, LUA_MULTRET) && lua_isboolean(l,-1) && !lua_toboolean(l,-1)) re = FALSE;
LeaveCriticalSection(&lcs);
return re;
}

BOOL ev_onCloseModlessWindow (HWND hwnd, int type) { return ev_mapBoolEvent(L"onClose", hwnd, type); }
BOOL ev_onSelectInModlessWindow (HWND hwnd, int type) { return ev_mapBoolEvent(L"onSelect", hwnd, type); }
BOOL ev_onContextMenuInModlessWindow (HWND hwnd, int type) { return ev_mapBoolEvent(L"onContextMenu", hwnd, type); }

static int tab_save (lua_State* l) {
pagecontext* p = tab_getFromArg(l,1);
if (!p) return 0;
save(p);
return 0;
}

static int tab_close (lua_State* l) {
pagecontext* p = tab_getFromArg(l,1);
int idx = indexOfPage(p);
if (idx<0) { printf("ERROR tab is %d\r\n", idx); return; }
int cs = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
SendMessage(tabctl, TCM_SETCURSEL, idx, 0);
deleteCurrentTab();
if (cs!=idx) {
if (cs>idx) cs--;
SendMessage(tabctl, TCM_SETCURSEL, cs, 0);
}
return 0;
}

static int tab_focus (lua_State* l) {
pagecontext* p = tab_getFromArg(l,1);
int idx = indexOfPage(p);
SendMessage(tabctl, TCM_SETCURSEL, idx, 0);
SendMessage(tabctl, TCM_HIGHLIGHTITEM, idx, TRUE);
curPage = pages[idx];
restorePage(curPage);
ev_onAfterTabChange(curPage);
return 0;
}

static int tab__index (lua_State* l) {
pagecontext* p = tab_getFromArg(l,1);
if (!p) return 0;
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "text")) {
if (p->curText) {
const char* str = strcvt(p->curText, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
free(str);
}
else {
int len = GetWindowTextLength(edit);
const wchar_t* wstr = malloc(sizeof(wchar_t) * (len+1));
GetWindowText(edit, wstr, len+1);
const char* str = strcvt(wstr, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
free(str);
free(wstr);
}}
else if (streq(nm, "filename")) {
if (!p->curFile) return 0;
const char* str = strcvt(p->curFile, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
free(str);
}
else if (streq(nm, "name")) {
if (!p->curName) return 0;
const char* str = strcvt(p->curName, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
free(str);
}
else if (streq(nm, "indentString")) {
const char* str = strcvt(p->tabmlr+1, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
free(str);
}
else if (streq(nm, "encoding")) lua_pushinteger(l, p->curEncoding);
else if (streq(nm, "lineEnding")) lua_pushinteger(l, p->curLineEnding);
else if (streq(nm, "indentType")) lua_pushinteger(l, p->tabSpaces);
else if (streq(nm, "modified")) {
if (curPage==p) lua_pushboolean(l, !!SendMessage(edit, EM_GETMODIFY, 0, 0));
else lua_pushboolean(l, p->modified);
}
else if (streq(nm, "readOnly")) {
if (curPage==p) lua_pushboolean(l, ES_READONLY&GetWindowLong(edit, GWL_STYLE));
else lua_pushboolean(l, p->readOnly);
}
else if (streq(nm, "filenameFilter")) {
if (!p->curFilenameFilter) return 0;
const char* str = strncvt(p->curFilenameFilter, p->curFilenameFilterLength, CP_UTF16, CP_UTF8, NULL);
lua_pushlstring(l,str, p->curFilenameFilterLength);
free(str);
}
else if (streq(nm, "filenameFilterIndex")) lua_pushinteger(l, p->curFilenameFilterIndex);
else if (streq(nm, "selectionStart") || streq(nm, "selStart")) {
int k;
if (curPage==p) SendMessage(edit, EM_GETSEL, &k, NULL);
else k = p->curSelStart;
lua_pushinteger(l, k+1);
}
else if (streq(nm, "selectionEnd") || streq(nm, "selEnd")) {
int k;
if (curPage==p) SendMessage(edit, EM_GETSEL, NULL, &k);
else k = p->curSelEnd;
lua_pushinteger(l, k+1);
}
#define E(x) else if (streq(nm,#x)) { if (p->x) lua_pushluafunction(l,p->x); else lua_pushnil(l); }
E(onBeforeOpen) E(onAfterOpen) E(onBeforeSave) E(onAfterSave)
E(onBeforeTabClose) E(onEnter) E(onTab) E(onLineChange) E(onKeyDown) E(onKeyUp) E(onKeyPress) E(onContextMenu) E(onStatusBarChange)
#undef E
// other properties
else if (!luaL_getmetafield(l, 1, nm)) lua_rawget(l,1);
return 1;
}

static int tab__newindex (lua_State* l) {
pagecontext* p = tab_getFromArg(l,1);
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "text")) {
const char* str = luaL_checkstring(l,3);
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
if (!p->curText) {
SetWindowText(edit, wstr);
free(wstr);
}
else {
free(p->curText);
p->curText = wstr;
}}
else if (streq(nm, "encoding")) {
int enc = luaL_checkint(l,3);
if (p==curPage) setEncoding(enc);
else p->curEncoding=enc;
}
else if (streq(nm, "lineEnding")) {
int le = luaL_checkint(l,3);
if (le<0 || le>2) luaL_argerror(l, 3, "invalid line ending: must be between 0 and 2");
if (p==curPage) setLineEnding(le);
else p->curLineEnding=le;
}
else if (streq(nm, "indentType")) {
int it = luaL_checkint(l,3);
if (it<0) it=0; else if (it>8) it=8;
if (p==curPage) setTabSpaces(it);
else {
p->tabSpaces=it;
fillIndentString(p->tabmlr, 10, p->tabSpaces);
}}
else if (streq(nm, "modified")) {
p->modified = luaL_checkboolean(l,3);
if (curPage==p) SendMessage(edit, EM_SETMODIFY, p->modified, 0);
}
else if (streq(nm, "readOnly")) {
p->readOnly = luaL_checkboolean(l,3);
if (p==curPage) SendMessage(edit, EM_SETREADONLY, p->readOnly, 0);
}
#define E(x) else if (streq(nm,#x)) p->x = luafixfunc(l, &(p->x), lua_topointer(l,3));
E(onBeforeOpen) E(onAfterOpen) E(onBeforeSave) E(onAfterSave)
E(onBeforeTabClose)  E(onEnter)  E(onTab)  E(onLineChange) E(onKeyDown)  E(onKeyUp)  E(onKeyPress) E(onContextMenu) E(onStatusBarChange)
#undef E
else if (streq(nm, "filename")) {
if (p->curFile) free(p->curFile);
p->curFile = strcvt(luaL_checkstring(l,3), CP_UTF8, CP_UTF16, NULL);
updatePageName(p, p->curFile);
if (p==curPage) updateWindowTitle();
else updateTabName(indexOfPage(p));
}
else if (streq(nm, "name")) {
if (p->curName) free(p->curName);
p->curName = strcvt(luaL_checkstring(l,3), CP_UTF8, CP_UTF16, NULL);
if (p==curPage) updateWindowTitle();
else updateTabName(indexOfPage(p));
}
else if (streq(nm, "filenameFilter")) {
int len = 0;
const char* str = luaL_optlstring(l,3,NULL,&len);
if (p->curFilenameFilter) free(p->curFilenameFilter);
p->curFilenameFilter = str? strncvt(str, len, CP_UTF8, CP_UTF16, &len) :NULL;
p->curFilenameFilterLength = len;
}
else if (streq(nm, "filenameFilterIndex")) p->curFilenameFilterIndex = luaL_checkinteger(l,3);
else if (streq(nm, "selStart") || streq(nm, "selectionStart")) {
if (curPage==p) {
int selStart, selEnd; SendMessage(edit, EM_GETSEL, &selStart, &selEnd);
selStart  = luaL_checkint(l,3);
if (selStart<=0) selStart += GetWindowTextLength(edit);
else selStart--;
SendMessage(edit, EM_SETSEL, selStart, selEnd);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
updateStatusBar();
}
else {
int k = luaL_checkint(l,3);
p->curSelStart = k>0? k -1 : wcslen(p->curText)-k;
}}
else if (streq(nm, "selEnd") || streq(nm, "selectionEnd")) {
if (curPage==p) {
int selStart, selEnd; SendMessage(edit, EM_GETSEL, &selStart, &selEnd);
selEnd = luaL_checkint(l,3);
if (selEnd<=0) selEnd += GetWindowTextLength(edit);
else selEnd--;
SendMessage(edit, EM_SETSEL, selStart, selEnd);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
updateStatusBar();
}
else {
int k = luaL_checkint(l,3);
p->curSelEnd = k>0? k -1 : wcslen(p->curText)-k;
}}
// other properties
else if (streq(nm, "indentString")) luaL_argerror(l, 2, "read only property");
else lua_rawset(l,1);
return 0;
}

static int window__index (lua_State* l) {
const char* nm = luaL_checkstring(l,2);
if (streq(nm, "edit")) lua_pushfulluserdata(l, &edit, sizeof(edit), "edithandle");
else if (streq(nm, "currentTab")) tab_pushTable(l, curPage);
#define E(x) else if (streq(nm,#x)) { if (x) lua_pushluafunction(l,x); else lua_pushnil(l); }
E(onBeforeOpen) E(onAfterOpen) E(onBeforeSave) E(onAfterSave) E(onTabNew) E(onBeforeClose)
E(onBeforeTabChange) E(onAfterTabChange) E(onBeforeTabClose) E(onAfterTabClose)
E(onEnter) E(onTab) E(onLineChange) E(onKeyDown) E(onKeyUp) E(onKeyPress) E(onContextMenu) E(onStatusBarChange)
#undef E
else if (streq(nm, "statusbar")) {
int len = GetWindowTextLength(status);
const wchar_t* w = malloc(sizeof(wchar_t) * (len+1));
GetWindowText(status, w, len+1);
const char* s = strcvt(w, CP_UTF16, CP_UTF8, NULL);
lua_settop(l,0);
lua_pushstring(l, s);
free(w);
free(s);
return 1;
}
else if (streq(nm, "title")) {
int len = GetWindowTextLength(win);
const wchar_t* w = malloc(sizeof(wchar_t) * (len+1));
GetWindowText(win, w, len);
const char* s = strcvt(w, CP_UTF16, CP_UTF8, NULL);
lua_settop(l,0);
lua_pushstring(l, s);
free(w);
free(s);
return 1;
}
else if (streq(nm, "menubar")) {
luamenuapi_pushmainmenu(l);
lua_pushstring(l, "menubar");
lua_pushvalue(l,-2);
lua_rawset(l, 1);
return 1;
}
else if (streq(nm, "filenameFilter")) {
if (!globalFilenameFilter) return 0;
int rlen=0;
const char* str = strncvt(globalFilenameFilter, globalFilenameFilterLength, CP_UTF16, CP_UTF8, &rlen);
lua_pushlstring(l,str, rlen);
free(str);
return 1;
}
else if (streq(nm, "autoWrap")) lua_pushboolean(l, curFlags&1);
else if (streq(nm, "autoReload")) lua_pushboolean(l, curFlags&2);
else if (streq(nm, "smartHome")) lua_pushboolean(l, curFlags&4);
else if (streq(nm, "followIndent")) lua_pushboolean(l, curFlags&8);
else if (streq(nm, "showStatusBar")) lua_pushboolean(l, curFlags&16);
else if (streq(nm, "hwndEdit")) lua_pushlightuserdata(l, edit);
else lua_rawget(l,1);
return 1;
}

static int window__newindex (lua_State* l) {
const char* nm = luaL_checkstring(l,2);
if (0) {}
#define E(x) else if (streq(nm,#x)) x = luafixfunc(l, x, lua_topointer(l,3));
E(onBeforeOpen) E(onAfterOpen) E(onBeforeSave) E(onAfterSave) E(onTabNew) E(onBeforeClose)
E(onBeforeTabChange) E(onAfterTabChange) E(onBeforeTabClose) E(onAfterTabClose)
E(onEnter) E(onTab) E(onLineChange) E(onKeyDown) E(onKeyUp) E(onKeyPress) E(onContextMenu) E(onStatusBarChange)
#undef E
else if (streq(nm, "statusbar")) {
const char* s = luaL_checkstring(l,3);
const wchar_t* w = strcvt(s, CP_UTF8, CP_UTF16, NULL);
SetWindowText(status,w);
free(w);
return 0;
}
else if (streq(nm, "title")) {
const char* s = luaL_checkstring(l,3);
const wchar_t* w = strcvt(s, CP_UTF8, CP_UTF16, NULL);
SetWindowText(win,w);
free(w);
return 0;
}
else if (streq(nm, "filenameFilter")) {
int len=0;
const char* str = luaL_optlstring(l,3,NULL,&len);
if (globalFilenameFilter) free(globalFilenameFilter);
globalFilenameFilter = str? strncvt(str, len, CP_UTF8, CP_UTF16, &len) :NULL;
globalFilenameFilterLength = len;
return 0;
}
#define C(n,i,f) else if (streq(nm, n)) { if (!(curFlags&f) == luaL_checkboolean(l,3)) SendMessage(win, WM_COMMAND, i, 0); }
C("autoWrap", IDM_LINEWRAP, 1)
C("autoReload", IDM_AUTORELOAD, 2)
C("smartHome", IDM_SMARTHOME, 4)
C("followIndent", IDM_KEEPINDENT, 8)
C("showStatusBar", IDM_SHOWSTATUS, 16)
#undef C
else if (streq(nm, "edit") || streq(nm, "currentTab") || streq(nm, "menubar") || streq(nm, "hwnd") || streq(nm, "hwndEdit") || streq(nm, "hwndTabCtrl")) luaL_argerror(l, 2, "read only property");
else lua_rawset(l,1);
return 0;
}

static int tabstable__len (lua_State* l) {
lua_settop(l,0);
lua_pushinteger(l, nPages);
return 1;
}

static int tabstable__index (lua_State* l) {
int idx = -4;
if (lua_isnumber(l,2)) idx = lua_tointeger(l,2) -1;
else {
const char* str = luaL_checkstring(l,2);
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, NULL);
for (int i=0; i<nPages; i++) {
if (0==wcscmp(wstr, pages[i]->curName) || 0==wcscmp(wstr, pages[i]->curFile)) { idx=i; break; }
}}
lua_settop(l,0);
pagecontext* p = NULL;
if (idx==-1) idx = curPage - pages[0];
if (idx>=0 && idx<nPages) p = pages[idx];
if (!p) return 0;
lua_settop(l,0);
tab_pushTable(l, p);
return 1;
}

void luaopen_eventstable (lua_State* l) {
char strPath[300]={0};
GetModuleFileNameA(NULL, strPath, 300);
*strrchr(strPath, '\\')= 0;


lua_newtable(l);
lua_pushvalue(l,-1);
lua_setfield(l, LUA_REGISTRYINDEX, "tabhandle");
regt(l, "__index", tab__index);
regt(l, "__newindex", tab__newindex);
regt(l, "save", tab_save);
regt(l, "close", tab_close);
regt(l, "focus", tab_focus);
lua_pushboolean(l,TRUE); lua_setfield(l, -2, "__metatable");
lua_pop(l,1);

lua_newtable(l);
lua_newuserdata(l, sizeof(intptr_t));
lua_newtable(l);
regt(l, "__index", tabstable__index);
regt(l, "__len", tabstable__len);
lua_pushboolean(l,TRUE); lua_setfield(l, -2, "__metatable");
lua_setmetatable(l, -2);
lua_setfield(l, -2, "tabs");
lua_pushlightuserdata(l, win);
lua_setfield(l, -2, "hwnd");
lua_pushlightuserdata(l, tabctl);
lua_setfield(l, -2, "hwndTabCtrl");
lua_pushstring(l, strPath);
lua_setfield(l, -2, "basedir");
lua_newtable(l);
regt(l, "__index", window__index);
regt(l, "__newindex", window__newindex);
lua_pushboolean(l,TRUE); lua_setfield(l, -2, "__metatable");
lua_setmetatable(l, -2);
lua_setglobal(l, "window");
}




#define UNICODE
#include<luaex.h>
#include<windows.h>
#include "consts.h"
#include "list.h"
#include "global.h"

#define regt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)

extern int addCustomCommand (void*);
extern void removeCustomCommand(void*) ;

extern int luaopen_progressapi (lua_State* l) ;
extern int luaopen_processapi (lua_State* l) ;
extern int luaopen_filedir (lua_State* l) ;


INT_PTR inputDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
void printToConsole (const wchar_t* msg) ;
void showLuaError (const char* msg) ;
void handleCustomCommand (void*) ;

extern HINSTANCE hinst;
extern HWND win, edit, status, tabctl, console;
extern list* extensionList;

CRITICAL_SECTION lcs;
lua_State* l = 0;

typedef struct {
luafunc* func;
BOOL once;
} timerdata;

BOOL callfunc (lua_State* l, void* p, int nArgs, int nRet) {
lua_pushcfunction(l, lua_getbacktrace);
lua_insert(l, -1 -nArgs);
lua_pushluafunction(l,p);
lua_insert(l, -1 -nArgs);
if (lua_pcall(l, nArgs, nRet, -2 -nArgs)) {
showLuaError(lua_tostring(l,-1));
return FALSE;
}
return TRUE;
}

void* luafixfunc (lua_State* l, void* x, void* p) {
lua_pushlightuserdata(l,x);
if (p) lua_pushluafunction(l,p);
else lua_pushnil(l);
lua_settable(l, LUA_REGISTRYINDEX);
return p;
}

BOOL eval (const char* str, int* rt, void* rp) {
EnterCriticalSection(&lcs);
BOOL success = FALSE;
lua_settop(l,0);
lua_pushcfunction(l, lua_getbacktrace);
if (luaL_loadstring(l,str) || lua_pcall(l, 0, LUA_MULTRET, -2)) showLuaError(lua_tostring(l,-1));
else {
success = TRUE;
if (rt&&rp) {
int type = lua_type(l,-1);
switch(type){
case LUA_TNIL: break;
case LUA_TBOOLEAN: *rt=1; *(BOOL*)rp = lua_toboolean(l,-1); break;
case LUA_TNUMBER: *rt=2; *(double*)rp=lua_tonumber(l,-1); break;
default: *rt=3; *(char**)rp = lua_tostring(l,-1); break;
}}}
LeaveCriticalSection(&lcs);
return success;
}

void handleCustomCommand (void* p) {
if (!p) return;
else if  ( ((intptr_t)p) &1) {
void(*func)(void) = (((char*)p)-1);
func(); return;
}
EnterCriticalSection(&lcs);
callfunc(l, p, 0, 0);
LeaveCriticalSection(&lcs);
}

static __stdcall void timerProc1 (HWND hwnd, int msg, int id, int time) {
EnterCriticalSection(&lcs);
BOOL clear = TRUE;
timerdata* timer = NULL;
lua_settop(l,0);
lua_pushlightuserdata(l,id);
lua_gettable(l, LUA_REGISTRYINDEX);
if (!lua_isnoneornil(l,-1)) {
timer = lua_topointer(l,-1);
if (timer) {
if (timer->func && callfunc(l, timer->func, 0, 0))  clear = timer->once;
}}
if (clear) {
KillTimer(NULL,id);
lua_pushlightuserdata(l,id);
lua_pushnil(l);
lua_settable(l, LUA_REGISTRYINDEX);
if (timer) {
lua_pushluafunction(l,timer->func);
lua_pushnil(l);
lua_settable(l, LUA_REGISTRYINDEX);
free(timer);
}}
lua_settop(l,0);
LeaveCriticalSection(&lcs);
}

static int setTimeoutOrInterval (lua_State* l, BOOL once) {
int time = luaL_checkint(l,2);
if (!lua_isfunction(l,1)) luaL_typerror(l,1,"function");
luafunc p = lua_topointer(l,1);
int id = SetTimer(NULL, NULL, time, timerProc1);
if (!id) return 0;
timerdata* timer = malloc(sizeof(timerdata));
timer->func = p;
timer->once = once;
lua_pushlightuserdata(l,id);
lua_pushlightuserdata(l,timer);
lua_settable(l, LUA_REGISTRYINDEX);
lua_pushvalue(l,1);
lua_pushboolean(l,TRUE);
lua_settable(l, LUA_REGISTRYINDEX);
lua_settop(l,0);
lua_pushlightuserdata(l,id);
return 1;
}
static int setTimeout (lua_State* l) { return setTimeoutOrInterval(l,TRUE); }
static int setInterval (lua_State* l) { return setTimeoutOrInterval(l,FALSE); }

static int clearTimeout (lua_State* l) {
int id = lua_topointer(l,1);
if (!id) return 0;
BOOL re = KillTimer(NULL, id);
if (re) {
timerdata* timer = NULL;
lua_pushlightuserdata(l,id);
lua_gettable(l,LUA_REGISTRYINDEX);
if (!lua_isnoneornil(l,-1)) {
timer = lua_topointer(l,-1);
if (timer) {
if (timer->func) {
lua_pushluafunction(l,timer->func);
lua_pushnil(l);
lua_settable(l,LUA_REGISTRYINDEX);
}
free(timer);
}}
lua_pushlightuserdata(l,id);
lua_pushnil(l);
lua_settable(l,LUA_REGISTRYINDEX);
}
lua_settop(l,0);
lua_pushboolean(l,re);
return 1;
}

static int lOpenDoc (lua_State* l) {
const char* fn = luaL_checkstring(l,1);
const wchar_t* wfn = strcvt(fn, CP_UTF8, CP_UTF16, NULL);
open(wfn, OF_REUSEOPENEDTABS | OF_NEWTAB, 0);
free(wfn);
return 0;
}

static int lSimpleExec (lua_State* l) {
lua_pushinteger(l, system2(luaL_checkstring(l,1)));
return 1;
}

static int lShellExec (lua_State* l) {
const char
*file = luaL_checkstring(l,1),
*param = luaL_optstring(l,2,NULL),
*operation = luaL_optstring(l,3,NULL);
BOOL show = luaL_optboolean(l,4,TRUE);
int re = ShellExecuteA(win, operation, file, param, NULL, show? SW_SHOWNORMAL : SW_HIDE);
printf("ShellExec result: %d\r\n", re);
lua_settop(l,0);
lua_pushboolean(l,re>32);
return 1;
}

static int lsleep (lua_State* l) {
int n = luaL_checknumber(l,1);
if (n<=0) luaL_argerror(l,1, "sleep time must be>0");
Sleep(n);
return 0;
}

static DWORD executeInBackgroundThreadProc (void** x) {
lua_State* l = x[0];
void* p = x[1];
free(x);
EnterCriticalSection(&lcs);
lua_pushcfunction(l, lua_getbacktrace);
lua_pushluafunction(l,p);
if (lua_pcall(l, 0, 0, -2)) showLuaError(lua_tostring(l,-1));
lua_pushlightuserdata(l,p);
lua_pushnil(l);
lua_settable(l, LUA_REGISTRYINDEX);
LeaveCriticalSection(&lcs);
return 0;
}

static int executeInBackground (lua_State* l) {
if (!lua_isfunction(l,1)) luaL_typerror(l,1,"function");
void* p = lua_topointer(l,1);
lua_pushlightuserdata(l,p);
lua_pushvalue(l,1);
lua_settable(l, LUA_REGISTRYINDEX);
void** x = malloc(sizeof(void*) *2);
x[0] = l;
x[1] = p;
newThread(executeInBackgroundThreadProc, x);
return 0;
}

static int lplaysnd (lua_State* l) {
PlaySoundA(luaL_checkstring(l,1), NULL, SND_ASYNC | SND_FILENAME);
return 0;
}

static int savedlg (lua_State* l) {
const char* fn = luaL_optstring(l, 1, 0);
const char* title = luaL_optstring(l, 2, 0);
int filterlen=0, nFilter = luaL_optint(l, 4, 0);
const char* filters = luaL_optlstring(l,3,0,&filterlen);
if (nFilter<0) nFilter=0;
const wchar_t* wfn = (fn? strcvt(fn, CP_UTF8, CP_UTF16, NULL) : 0);
const wchar_t* wtitle = (title? strcvt(title, CP_UTF8, CP_UTF16, NULL) : 0);
const wchar_t* wfilters  = (filters? strncvt(filters, filterlen, CP_UTF8, CP_UTF16, NULL) : MSG_FILENAMEFILTER);
const wchar_t* result = showFileDialog(wfn, wtitle, wfilters, &nFilter, 0);
if (!result) return 0;
const char* mbresult = strcvt(result, CP_UTF16, CP_UTF8, NULL);
lua_settop(l,0);
lua_pushstring(l,mbresult);
lua_pushinteger(l,nFilter);
free(mbresult);
free(result);
if (wtitle) free(wtitle);
if (wfn) free(wfn);
if (wfilters) free(wfilters);
return 2;
}

static int opendlg (lua_State* l) {
const char* fn = luaL_optstring(l, 1, 0);
const char* title = luaL_optstring(l, 2, 0);
int filterlen=0, nFilter = luaL_optint(l, 4, 0);
const char* filters = luaL_optlstring(l, 3, 0,&filterlen);
if (nFilter<0) nFilter=0;
const wchar_t* wfn = (fn? strcvt(fn, CP_UTF8, CP_UTF16, NULL) : 0);
const wchar_t* wtitle = (title? strcvt(title, CP_UTF8, CP_UTF16, NULL) : 0);
const wchar_t* wfilters  = (filters? strncvt(filters, filterlen, CP_UTF8, CP_UTF16, NULL) : MSG_FILENAMEFILTER);
const wchar_t* result = showFileDialog(wfn, wtitle, wfilters, &nFilter, 1);
if (!result) return 0;
lua_settop(l,0);
const char* mbresult = strcvt(result, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,mbresult);
lua_pushinteger(l, nFilter);
free(mbresult);
free(result);
if (wtitle) free(wtitle);
if (wfn) free(wfn);
if (wfilters) free(wfilters);
return 2;
}

static int browsefolders (lua_State* l) {
const char *sFolder = luaL_optstring(l,1,NULL), *title = luaL_optstring(l,2,NULL);
const wchar_t* wtitle = (title? strcvt(title, CP_UTF8, CP_UTF16, NULL) : 0);
wchar_t folder[512]={0};
if (sFolder) wsprintf(folder, L"%hs", sFolder);
BOOL re = BrowseFolders(win, folder, wtitle);
if (wtitle) free(wtitle);
if (!re) return 0;
lua_settop(l,0);
const char* str = strcvt(folder, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
free(str);
return 1;
}

static int setclipboard (lua_State* l) {
const char* str = luaL_checkstring(l,1);
int len = 0;
const wchar_t* wstr = strcvt(str, CP_UTF8, CP_UTF16, &len);
setClipboardText(wstr, len);
free(wstr);
return 0;
}

static int getclipboard (lua_State* l) {
int len = 0;
const wchar_t* wstr = getClipboardText(&len);
const char* str = strncvt(wstr, len, CP_UTF16, CP_UTF8, &len);
lua_settop(l,0);
lua_pushlstring(l,str,len);
free(str);
free(wstr);
return 1;
}

static int beep (lua_State* l) {
int n1 = luaL_optint(l,1,0);
int n2 = luaL_optint(l,2,0);
if (n2>0) Beep(n1,n2);
else if (n1<=1) MessageBeep(MB_OK);
else if (n1==2) MessageBeep(MB_ICONINFORMATION);
else if (n1==3) MessageBeep(MB_ICONEXCLAMATION);
else if (n1==4) MessageBeep(MB_ICONERROR);
return 0;
}

static int choice (lua_State* l) {
const char* prompt = luaL_checkstring(l,1);
const char* title = luaL_optstring(l,3,MSG_INPUTDEFTITLE);
if (!lua_istable(l,2)) luaL_typerror(l, 2, "table");
int pos = 0, cap = 1024, index = 0;
wchar_t* wlist = malloc(sizeof(wchar_t) * cap);
wlist[0]=0;
while (1) {
lua_rawgeti(l, 2, ++index);
if (lua_isnoneornil(l,-1)) {
lua_pop(l,1);
break;
}
int len;
const char* ch = lua_tolstring(l,-1,&len);
if (pos+len+4>=cap) {
cap = cap*3/2+1;
if (cap<pos+len+4) cap = pos+len+4;
wlist = realloc(wlist, sizeof(wchar_t)*cap);
}
wsprintf(wlist+pos, L"%hs", ch);
pos += len+1;
lua_pop(l,1);
}
wlist[pos] = 0;
wlist[pos+1] = 0;
wlist[pos+2]=0;
const wchar_t* wprompt = strcvt(prompt, CP_UTF8, CP_UTF16, NULL);
const wchar_t* wtitle = strcvt(title, CP_UTF8, CP_UTF16, NULL);
int flags = 1;
if (lua_isboolean(l,4) && lua_toboolean(l,4)) flags&=~1; 
if (flags&1) { // Modal version
const wchar_t* re = showListDialog(wprompt, wlist, wtitle, flags);
if (!re) flags=0;
else {
lua_settop(l,0);
const char* x = strcvt(re+1, CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,x);
lua_pushinteger(l, re[0] +1);
free(x);
flags = 2;
}}
else { // Modless version
static BOOL choicehandleinitialized = FALSE;
HWND hList = showListDialog(wprompt, wlist, wtitle, flags);
if (!choicehandleinitialized) {
lua_settop(l,0);
luaopen_choicehandleapi(l);
choicehandleinitialized = TRUE;
}
lua_settop(l,0);
lua_pushfulluserdata(l, &hList, sizeof(hList), "choicehandle");
flags = 1;
}
free(wtitle);
free(wprompt);
free(wlist);
return flags;
}

static int output (lua_State* l) {
const char* prompt = luaL_checkstring(l,1);
const char* text = luaL_checkstring(l,2);
const char* title = luaL_optstring(l,3,MSG_H_OUTPUT);
const wchar_t* wprompt = strcvt(prompt, CP_UTF8, CP_UTF16, NULL);
const wchar_t* wtext = strcvt(text, CP_UTF8, CP_UTF16, NULL);
const wchar_t* wtitle = strcvt(title, CP_UTF8, CP_UTF16, NULL);
HWND h = showOutputWindow(wprompt, wtext, wtitle);
free(wprompt);
free(wtext);
free(wtitle);
lua_settop(l,0);
lua_pushfulluserdata(l, &h, sizeof(h), "edithandle");
return 1;
}

static int prompt (lua_State* l) {
const char* prompt = luaL_checkstring(l,1);
const char* text = luaL_optstring(l,2,"");
const char* title = luaL_optstring(l,3,MSG_INPUTDEFTITLE);
if (!lua_isnoneornil(l,4) && (!lua_istable(l,4) && !lua_isboolean(l,4))) luaL_typerror(l,4, "boolean or table");
int wlen = (strlen(text) + strlen(prompt) + strlen(title) + 8);
wchar_t* wstr = malloc(sizeof(wchar_t) * wlen);
wstr[0]=0;
int pos = 1;
pos += 1+wsprintf(wstr+pos, L"%hs", prompt);
pos += 1+wsprintf(wstr+pos, L"%hs", text);
pos += 1+wsprintf(wstr+pos, L"%hs", title);
if (lua_isboolean(l,4)) wstr[0] = !!lua_toboolean(l,4);
else if (lua_istable(l,4)) {
int itemlen; const char* item;
lua_pushnil(l);
while (lua_next(l,4)) {
item = lua_tolstring(l,-1, &itemlen);
if (item && itemlen>0) {
if (pos+itemlen+3 >= wlen) {
int k = wlen*3/2+1;
if (k<pos+itemlen+3) k = pos+itemlen+3;
wstr = realloc(wstr, sizeof(wchar_t) * k);
wlen=k;
}
pos += 1 + wsprintf(wstr+pos, L"%hs", item);
}
lua_pop(l,1);
}}
wstr[pos]=0;
int re = DialogBoxParam(hinst, wstr[0]?IDD_INPUT2:IDD_INPUT, win, inputDlgProc, wstr);
free(wstr);
if (re>= -1 && re<=1) return 0;
else {
const char* mbre = strcvt(re, CP_UTF16, CP_UTF8, NULL);
lua_settop(l,0);
lua_pushstring(l,mbre);
free(mbre);
return 1;
}}

static int alert (lua_State* l, int flg) {
int n = lua_gettop(l);
if (n<=1) lua_pushnil(l);
lua_getglobal(l,"tostring");
lua_pushvalue(l,1);
lua_call(l,1,1);
const char* msg = lua_tostring(l,-1);
if (!msg) msg = "nil";
const char* ttl = luaL_optstring(l,2, "Information");
const wchar_t* wmsg = strcvt(msg, CP_UTF8, CP_UTF16, NULL);
const wchar_t* wttl = strcvt(ttl, CP_UTF8, CP_UTF16, NULL);
MessageBox(win, wmsg, wttl, MB_OK | flg);
free(wttl); free(wmsg);
return 0;
}

static int alert1 (lua_State* l) { return alert(l, MB_ICONASTERISK); }
static int alert2 (lua_State* l) { return alert(l, MB_ICONERROR); }

static int confirm (lua_State* l) {
const char* msg = luaL_checkstring(l,1);
const char* ttl = luaL_optstring(l,2, "Question");
const wchar_t* wmsg = strcvt(msg, CP_UTF8, CP_UTF16, NULL);
const wchar_t* wttl = strcvt(ttl, CP_UTF8, CP_UTF16, NULL);
BOOL re = IDYES == MessageBox(win, wmsg, wttl, MB_YESNO | MB_ICONEXCLAMATION);
free(wttl);
free(wmsg);
lua_settop(l,0);
lua_pushboolean(l,re);
return 1;
}

static int lAddAccelerator (lua_State* l) {
int fArg=1, sArg=2;
const char* sShortcut = NULL;
if (!lua_isfunction(l,1)) { fArg=2; sArg=1; }
if (!lua_isnoneornil(l,sArg)) sShortcut = luaL_checkstring(l,sArg);
if (!lua_isfunction(l,fArg)) luaL_typerror(l, fArg, "function");
void* p = lua_topointer(l,fArg);
if (!p) return 0;
int kFlags = 0, key=0;
int command = findCustomCommand(p);
if ((!sShortcut || !parseKeyName(sShortcut, &kFlags, &key)) && command<0) return 0;
if (key && command<0) command = addCustomCommand(p);
if (command<0) return 0; 
if (key) addAccelerator(kFlags, key, command + IDM_CUSTOMCOMMAND);
else removeAccelerator(command + IDM_CUSTOMCOMMAND);
lua_pushboolean(l,TRUE);
return 1;
}

BOOL handleListClick (HWND h, int idx, const wchar_t* txt) {
void* p = GetProp(h, L"onAction");
if (!p) return TRUE;
BOOL re = FALSE;
const char* str = strcvt(txt, CP_UTF16, CP_UTF8, NULL);
EnterCriticalSection(&lcs);
lua_settop(l,0);
lua_pushfulluserdata(l, &h, sizeof(h), "choicehandle");
lua_pushstring(l,str);
lua_pushinteger(l,idx+1);
if (callfunc(l, p, 3, LUA_MULTRET) && lua_isboolean(l,-1) && lua_toboolean(l,-1)) re = TRUE;
free(str);
LeaveCriticalSection(&lcs);
return re;
}

static int luatype2 (lua_State* l) {
return lua_extype(l,1);
}

int luaEvalString (const char* code) {
EnterCriticalSection(&lcs);
lua_pushcfunction(l, lua_getbacktrace);
if (luaL_loadstring(l,code) || lua_pcall(l, 0, LUA_MULTRET, -2)) showLuaError(lua_tostring(l,-1));
LeaveCriticalSection(&lcs);
}

static int lprint2 (lua_State* l) {
int n = lua_gettop(l);
lua_getglobal(l,"tostring");
for (int i=1; i<=n; i++) {
lua_pushvalue(l,n+1);
lua_pushvalue(l,i);
lua_call(l,1,1);
const char* s = lua_tostring(l,-1);
const wchar_t* ws = strcvt(s, CP_UTF8, CP_UTF16, NULL);
printToConsole(ws);
if (i<n) printToConsole(L" ");
free(ws);
lua_pop(l,1);
}
printToConsole(L"\r\n");
}

void initializeLua (int argc, const wchar_t** argv) {
InitializeCriticalSection(&lcs);
EnterCriticalSection(&lcs);
l = lua_open();
luaL_openlibs(l);
lua_settop(l,0);
lua_register(l, "alert", alert1);
lua_register(l, "warning", alert2);
lua_register(l, "confirm", confirm);
lua_register(l, "prompt", prompt);
lua_register(l, "output", output);
lua_register(l, "choice", choice);
lua_register(l, "saveDialog", savedlg);
lua_register(l, "openDialog", opendlg);
lua_register(l, "chooseFolder", browsefolders);
lua_register(l, "sleep", lsleep);
lua_register(l, "executeInBackground", executeInBackground);
lua_register(l, "beep", beep);
lua_register(l, "playSound", lplaysnd);
lua_register(l, "setClipboardText", setclipboard);
lua_register(l, "getClipboardText", getclipboard);
lua_register(l, "setTimeout", setTimeout);
lua_register(l, "setInterval", setInterval);
lua_register(l, "clearTimeout", clearTimeout);
lua_register(l, "clearInterval", clearTimeout);
lua_register(l, "openDocument", lOpenDoc);
lua_register(l, "addAccelerator", lAddAccelerator);
lua_register(l, "removeAccelerator", lAddAccelerator);
lua_register(l, "print", lprint2);
lua_register(l, "shellExec", lShellExec);

lua_getfield(l, LUA_GLOBALSINDEX, "os");
regt(l, "execute", lSimpleExec);
lua_newtable(l);
for (int i=0; i<argc; i++) {
const char* str = strcvt(argv[i], CP_UTF16, CP_UTF8, NULL);
lua_pushstring(l,str);
lua_rawseti(l, -2, i);
free(str);
}
lua_setfield(l, -2, "argv");
lua_pop(l,1);

luaopen_pcreapi(l);
luaopen_exstringapi(l);
luaopen_langextensions(l);
luaopen_editapi(l);
luaopen_menuapi(l);
luaopen_eventstable(l);

lua_getglobal(l, "package");
lua_getfield(l, -1, "preload");
regt(l, "progress", luaopen_progressapi);
regt(l, "process", luaopen_processapi);
regt(l, "filedir", luaopen_filedir);
lua_settop(l,0);
lua_pushcfunction(l, lua_getbacktrace);
lua_getglobal(l, "require");

for (int i=0, n=extensionList?l_len(extensionList):0; i<n; i++) {
lua_settop(l,3);
lua_pushvalue(l,-1);
lua_pushstring(l, l_item(extensionList,i));
if (lua_pcall(l, 1, LUA_MULTRET, 0))  showLuaError(lua_tostring(l,-1));
}
LeaveCriticalSection(&lcs);
}

void printToConsole (const wchar_t* msg) {
wprintf(L"%s",msg);
if (!console) showConsoleWindow();
if (!console) return;
SendMessage(console, WM_COMMAND, 2000, msg);
}

void showLuaError (const char* msg) {
int len=0;
wchar_t* wmsg = strcvt(msg, CP_UTF8, CP_UTF16, &len);
wmsg = normalizeLineEnding(wmsg, len, 0, NULL, &len);
wmsg[len]=0;
MessageBeep(MB_ICONERROR);
printToConsole(wmsg);
printToConsole(L"\r\n");
free(wmsg);
}



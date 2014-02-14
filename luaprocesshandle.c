#include<luaex.h>
#define UNICODE
#include<windows.h>
#include "global.h"

#define regt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l, -2, n)

typedef struct {
HANDLE hProc, hThread, inRd, inWr, outRd, outWr;
} phandle;

static int pwrite (lua_State* l) {
phandle* p = lua_touserdata(l,1);
if (!p->hProc) return 0;
int i, n, sz, nWritten, nTotal=0;
for (i=2, n=lua_gettop(l); i<=n; i++) {
const char* s = luaL_checklstring(l,i,&sz);
WriteFile(p->inWr, s, sz, &nWritten, NULL);
nTotal += nWritten;
}
lua_settop(l,0);
lua_pushinteger(l,nTotal);
return 1;
}

static const char* preadline (phandle* p, int* pLen) {
int n=0, cap=32, nRead = 0;
char c=0, *s = malloc(cap);
while (GetFileSize(p->outRd,NULL)>0 && ReadFile(p->outRd, &c, 1, &nRead, NULL) && nRead>0 && c!='\n') {
if (c!='\r' || c!='\n') s[n++] = c;
if (n>=cap) s = realloc(s, cap = cap*3/2 +1);
}
s[n]=0;
if (nRead<=0) { free(s); s=0; }
if (pLen) *pLen = n;
return s;
}

static const char* preadall (phandle* p, int* pLen) {
int n=0, cap=1375, nRead;
char* s = malloc(cap);
while (GetFileSize(p->outRd,NULL)>0 && ReadFile(p->outRd, s+n, cap-n, &nRead, NULL) && nRead>0) {
n+=nRead;
if (n>=cap) s = realloc(s, cap = cap*3/2 +1);
else break;
}
s[n]=0;
if (pLen) *pLen=n;
return s;
}

static int pforeachline (lua_State* l) {
phandle* p = lua_touserdata(l,1);
lua_settop(l,1);
lua_getfield(l, LUA_REGISTRYINDEX,  "process");
lua_getfield(l, 2, "readLine");
lua_pushvalue(l,1);
lua_pushnil(l);
return 3;
}

static int preadline3 (lua_State* l) {
phandle* p = lua_touserdata(l,1);
int len;
const char* s = preadline(p, &len);
lua_settop(l,0);
if (!s) lua_pushnil(l);
else lua_pushlstring(l, s, len);
return 1;
}

static int pread (lua_State* l) {
phandle* p = lua_touserdata(l,1);
int nArgs = lua_gettop(l), nCount, nRe=0;
lua_checkstack(l, nArgs -1);
for (nCount=2; nCount<=nArgs; nCount++) {
if (!GetFileSize(p->outRd, NULL)) break;
if (lua_isnumber(l,nCount)) {
int n = lua_tointeger(l,nCount), nRead;
char* s = malloc(n+1);
ReadFile(p->outRd, s, n, &nRead, NULL);
s[nRead]=0;
lua_pushlstring(l, s, nRead);
free(s);
nRe++;
}
else if (lua_isstring(l,nCount)) {
const char* fmt = lua_tostring(l,nCount);
if (stricmp(fmt, "*a")==0) {
int len;
const char* s = preadall(p, &len);
lua_pushlstring(l, s, len);
free(s);
nRe++;
}
else if (stricmp(fmt, "*l")==0) {
int len;
const char* s = preadline(p, &len);
lua_pushlstring(l, s, len);
free(s);
nRe++;
}
else luaL_argerror(l, nCount, "unsupported reading format");
}
else luaL_typerror(l, nCount, "string or number");
}
return nRe;
}

static int pflush (lua_State* l) {
phandle* p = lua_touserdata(l,1);
if (!p->hProc) return 0;
FlushFileBuffers(p->inWr);
Sleep(1);
return 0;
}

static void waitfor (phandle* p) {
if (p->inWr) {
FlushFileBuffers(p->inWr);
Sleep(1);
CloseHandle(p->inWr);
p->inWr=0;
}
if (p->hProc) WaitForSingleObject(p->hProc, INFINITE);
}

static int procexit (phandle* p, int mode) {
int e = -1;
if (p->hProc) {
if (mode==0) waitfor(p);
else TerminateProcess(p->hProc, mode);
GetExitCodeProcess(p->hProc, &e);
CloseHandle(p->hProc);
CloseHandle(p->hThread);
p->hThread = p->hProc = 0;
HANDLE* h = p;
int i; for (i=0; i<6; i++) {
if (h[i]) CloseHandle(h[i]);
h[i]=0;
}}
return e;
}

static int pclose3 (lua_State* l) {
phandle* p = lua_touserdata(l,1);
int re = procexit(p, 0);
lua_settop(l,0);
lua_pushinteger(l,re);
return 1;
}

static int pdestroy3 (lua_State* l) {
phandle* p = lua_touserdata(l,1);
int re = procexit(p, luaL_optint(l,2,255));
lua_settop(l,0);
lua_pushinteger(l,re);
return 1;
}

static int pwait3 (lua_State* l) {
phandle* p = lua_touserdata(l,1);
waitfor(p);
lua_settop(l,1);
return 1;
}

static int popen3 (lua_State* l) {
const char* cmd = luaL_checkstring(l,1);
phandle p;
PROCESS_INFORMATION pi;
STARTUPINFO si;
SECURITY_ATTRIBUTES sa;
sa.nLength = sizeof(sa);
sa.bInheritHandle = TRUE;
sa.lpSecurityDescriptor = NULL;
CreatePipe(&p.inRd, &p.inWr, &sa, 0);
CreatePipe(&p.outRd, &p.outWr, &sa, 0);
if (!p.inRd || !p.inWr || !p.outRd || !p.outWr) return 0;
SetHandleInformation(p.inWr, HANDLE_FLAG_INHERIT, FALSE);
SetHandleInformation(p.outRd, HANDLE_FLAG_INHERIT, FALSE);
ZeroMemory(&si, sizeof(si));
ZeroMemory(&pi, sizeof(pi));
si.cb = sizeof(si);
si.dwFlags = STARTF_USESTDHANDLES;
si.hStdInput = p.inRd;
si.hStdOutput = si.hStdError = p.outWr;
if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
CloseHandle(p.inRd); CloseHandle(p.inWr);
CloseHandle(p.outRd); CloseHandle(p.outWr);
return 0;
}
p.hProc = pi.hProcess;
p.hThread = pi.hThread;
lua_settop(l,0);
lua_pushfulluserdata(l, &p, sizeof(p), "process");
return 1;
}

static int pgc (lua_State* l) {
phandle* p = lua_touserdata(l,1);
procexit(p, 255);
return 0;
}

int system2 (const char* cmd) {
int e = -1;
PROCESS_INFORMATION pi;
STARTUPINFO si;
SECURITY_ATTRIBUTES sa;
sa.nLength = sizeof(sa);
sa.bInheritHandle = TRUE;
sa.lpSecurityDescriptor = NULL;
ZeroMemory(&si, sizeof(si));
ZeroMemory(&pi, sizeof(pi));
si.cb = sizeof(si);
if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
}
WaitForSingleObject(pi.hProcess, INFINITE);
GetExitCodeProcess(pi.hProcess, &e);
CloseHandle(pi.hProcess);
return e;
}

int __declspec(dllexport) luaopen_processapi  (lua_State* l) {
lua_settop(l,0);
lua_newclass(l, "process");
regt(l, "read", pread);
regt(l, "readLine", preadline3);
regt(l, "write", pwrite);
regt(l, "flush", pflush);
regt(l, "lines", pforeachline);
regt(l, "close", pclose3);
regt(l, "destroy", pdestroy3);
regt(l, "wait", pwait3);
regt(l, "open", popen3);
regt(l, "__gc", pgc);
lua_pop(l,1);
return 0;
}





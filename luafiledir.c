#include<luaex.h>
#include<windows.h>
#include "global.h"
#include "consts.h"

#define regt(l,n,f) lua_pushcclosure(l,f,0); lua_setfield(l,-2,n)

typedef struct {
HANDLE h;
int state;
WIN32_FIND_DATA fd;
} fileinfo;

unsigned long filetimeTo1970 (unsigned long long) ;

static int findnext (lua_State* l) {
fileinfo* fi = lua_topointer(l,1);
switch(fi->state) {
case 0 :
fi->h = FindFirstFile(fi->fd.cFileName, &fi->fd);
if (!fi->h) goto _2;
case 1 : {
unsigned long long ft = (((unsigned long long)fi->fd.ftLastWriteTime.dwHighDateTime)<<32) | fi->fd.ftLastWriteTime.dwLowDateTime;
unsigned long long sz = ((unsigned long long)(fi->fd.nFileSizeHigh)<<32LL) | fi->fd.nFileSizeLow;
lua_settop(l,0);
lua_pushstring(l, fi->fd.cFileName);
lua_pushboolean(l, 0!=(fi->fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY));
lua_pushnumber(l, filetimeTo1970(ft));
lua_pushnumber(l, sz);
fi->state = 1+(!FindNextFile(fi->h, &fi->fd));
return 4;
}
case 2 : _2:
if (fi->h) FindClose(fi->h);
free(fi);
default :
return 0;
}}

static int findfirst (lua_State* l) {
fileinfo* fi = malloc(sizeof(fileinfo));
memset(fi, 0, sizeof(fileinfo));
strcpy(fi->fd.cFileName, luaL_optstring(l,1,"*"));
lua_settop(l,0);
lua_getfield(l, LUA_REGISTRYINDEX,  "_____findnextfile");
lua_pushlightuserdata(l,fi);
return 2;
}

static int isfile (lua_State* l) {
DWORD n = GetFileAttributes(luaL_checkstring(l,1));
lua_pushboolean(l, n!=INVALID_FILE_ATTRIBUTES && 0==(n&FILE_ATTRIBUTE_DIRECTORY));
return 1;
}

static int isdir (lua_State* l) {
DWORD n = GetFileAttributes(luaL_checkstring(l,1));
lua_pushboolean(l, n!=INVALID_FILE_ATTRIBUTES && 0!=(n&FILE_ATTRIBUTE_DIRECTORY));
return 1;
}

static int lcopyfile (lua_State* l) {
const char *src = luaL_checkstring(l,1), *dst = luaL_checkstring(l,2);
BOOL failIfExists = luaL_optboolean(l,3,FALSE);
BOOL re = CopyFileA(src,dst,failIfExists);
lua_pushboolean(l,re);
return 1;
}

static int ldeletefile (lua_State* l) {
const char* fn = luaL_checkstring(l,1);
BOOL re = DeleteFileA(fn);
lua_pushboolean(l,re);
return 1;
}

static int lmovefile (lua_State* l) {
const char *src = luaL_checkstring(l,1), *dst = luaL_checkstring(l,2);
BOOL re = MoveFileExA(src,dst, MOVEFILE_COPY_ALLOWED);
lua_pushboolean(l,re);
return 1;
}

static int lfilesize (lua_State* l) {
const char* fn = luaL_checkstring(l,1);
HANDLE hFile = CreateFileA(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
DWORD size = -1;
if (hFile&&hFile!=INVALID_HANDLE_VALUE) {
size = GetFileSize(hFile,NULL);
CloseHandle(hFile);
}
lua_pushinteger(l,size);
return 1;
}

static int lfiletime (lua_State* l, int type) {
const char* fn = luaL_checkstring(l,1);
const wchar_t* wfn = strcvt(fn, CP_UTF8, CP_UTF16, NULL);
unsigned long t = getFileTime(wfn, type);
free(wfn);
lua_pushnumber(l,t);
return 1;
}
static int lfilemtime (lua_State* l) {
return lfiletime(l, LAST_MODIFIED_TIME);
}
static int lfilectime (lua_State* l) {
return lfiletime(l, CREATED_TIME);
}
static int lfileatime (lua_State* l) {
return lfiletime(l, LAST_ACCESSED_TIME);
}

static int lmkdir (lua_State* l) {
const char* dst = luaL_checkstring(l,1);
BOOL re = CreateDirectoryA(dst,NULL);
lua_pushboolean(l,re);
return 1;
}

static int lrmdir (lua_State* l) {
const char* dst = luaL_checkstring(l,1);
BOOL re = RemoveDirectoryA(dst);
lua_pushboolean(l,re);
return 1;
}

static int lsymlink (lua_State* l) {
typedef BOOL(*CreateSymbolicLinkFunc)(const char*, const char*, DWORD);
static CreateSymbolicLinkFunc CreateSymbolicLink = NULL;
if (!CreateSymbolicLink) {
HANDLE kernel32 = LoadLibraryA("kernel32.dll");
CreateSymbolicLink = GetProcAddress(kernel32, "CreateSymbolicLinkA");
}
if (lua_gettop(l)<=0) {
lua_pushboolean(l, !!CreateSymbolicLink);
return 1;
}
if (CreateSymbolicLink) {
lua_pushboolean(l, CreateSymbolicLink(luaL_checkstring(l,2), luaL_checkstring(l,1), luaL_optboolean(l,3,0)));
return 1;
}
return 0;
}

static int lcwd (lua_State* l) {
const char* nwd = luaL_optstring(l,1,NULL);
if (nwd) lua_pushboolean(l, SetCurrentDirectoryA(nwd));
else {
char buf[300] = {0};
GetCurrentDirectoryA(300,buf);
lua_pushstring(l,buf);
}
return 1;
}

static int lAppdir (lua_State* l) {
char path[512]={0};
GetModuleFileNameA(NULL,path,511);
char* c = strrchr(path,'\\');
*++c=0;
lua_settop(l,0);
lua_pushstring(l,path);
return 1;
}

static int lRealpath (lua_State* l) {
const char* path = luaL_checkstring(l,1);
char real[300]={0};
if (GetFullPathNameA(path, 300, real, NULL)<=0) return 0;
lua_pushstring(l,real);
return 1;
}

int __declspec(dllexport) luaopen_filedir (lua_State *l) {
lua_settop(l,0);
lua_pushcclosure(l, findnext, 0);
lua_setfield(l, LUA_REGISTRYINDEX, "_____findnextfile");
lua_getglobal(l, "os");
regt(l, "glob", findfirst);
regt(l, "filesize", lfilesize);
regt(l, "filemtime", lfilemtime);
regt(l, "filectime", lfilectime);
regt(l, "fileatime", lfileatime);
regt(l, "move", lmovefile);
regt(l, "rename", lmovefile);
regt(l, "copy", lcopyfile);
regt(l, "symlink", lsymlink);
regt(l, "delete", ldeletefile);
regt(l, "mkdir", lmkdir);
regt(l, "rmdir", lrmdir);
regt(l, "chdir", lcwd);
regt(l, "appdir", lAppdir);
regt(l, "realpath", lRealpath);
regt(l, "isfile", isfile);
regt(l, "isdir", isdir);
lua_pop(l,1);
return 0;
}

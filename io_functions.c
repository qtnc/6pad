#include "consts.h"
#include "global.h"

typedef const char*(*protocolreader)(const wchar_t*, int*, BOOL*);
typedef BOOL(*protocolwriter)(const wchar_t*, const char*, int) ;
protocolreader* protocolReaders = 0;
protocolwriter* protocolWriters = 0;
int nProtocolReaders=0, nProtocolWriters=0, nMaxProtocolReaders=0, nMaxProtocolWriters=0;

wchar_t* normalizeLineEnding (wchar_t* s, int len, int le, int* optr1, int* optr2) {
int cle = 0;
int i; for (i=0; i<len; i++) {
if (s[i]=='\n') { cle = 1; break; }
else if (s[i]=='\r') { 
cle = (s[i+1]=='\n'? 0 : 2); break; }
}
if (optr1) *optr1 = cle;
int pos=0, cap = len * 1.075 +10;
wchar_t* buf = malloc(sizeof(wchar_t) * cap);
for (i=0; i < len; i++) {
if (pos>cap-4) {
cap = cap * 3/2 +1;
buf = realloc(buf, sizeof(wchar_t) * cap);
}
if (s[i]=='\n' || s[i]=='\r') {
if (s[i]=='\r' && s[i+1]=='\n') i++;
if (le==0) { buf[pos++]='\r'; buf[pos++]='\n'; }
else if (le==1) buf[pos++] = '\n';
else if (le==2) buf[pos++]='\r';
}
else buf[pos++] = s[i];
}
buf[pos]=0;
if (optr2) *optr2=pos;
free(s);
return buf;
}

void __stdcall __declspec(dllexport) registerProtocolReader (protocolreader x) {
if (!protocolReaders) protocolReaders = malloc((nMaxProtocolReaders=3) * sizeof(protocolreader)); 
if (nProtocolReaders>=nMaxProtocolReaders) protocolReaders = realloc(protocolReaders, (nMaxProtocolReaders+=3) * sizeof(protocolreader)); 
protocolReaders[nProtocolReaders++] = x;
}

void __stdcall __declspec(dllexport) registerProtocolWriter (protocolwriter x) {
if (!protocolWriters) protocolWriters = malloc((nMaxProtocolWriters=3) * sizeof(protocolwriter)); 
if (nProtocolWriters>=nMaxProtocolWriters) protocolWriters = realloc(protocolWriters, (nMaxProtocolWriters+=3) * sizeof(protocolwriter)); 
protocolWriters[nProtocolWriters++] = x;
}

const char* readFile (wchar_t* fn, int* nSize, BOOL* rdo) {
if (wcspos(fn, L"://", 0)>0) {
for (int i=0; i<nProtocolReaders; i++) {
const char* re = protocolReaders[i](fn, nSize, rdo);
if (re) return re;
}
return NULL;
}
HANDLE hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
if (hFile==INVALID_HANDLE_VALUE) return 0;
int cap = 4096, pos=0, nRead=0;
char* buf = malloc(cap);
while (ReadFile(hFile, buf+pos, cap-pos, &nRead, NULL)) {
if (nRead<=0) break;
pos += nRead;
if (pos>=cap-32) buf = realloc(buf, cap = cap*3/2+1);
}
CloseHandle(hFile);
if (nSize) *nSize = pos;
buf[pos]=0;
return buf;
}

BOOL writeFile (wchar_t* fn, const char* buf, int len) {
if (wcspos(fn, L"://", 0)>0) {
for (int i=0; i<nProtocolWriters; i++) {
if (protocolWriters[i](fn,buf,len)) return TRUE;
}
return FALSE;
}
HANDLE hFile = CreateFile(fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
if (hFile==INVALID_HANDLE_VALUE) return FALSE;
int pos=0, nWritten=0;
while (pos<len && WriteFile(hFile, buf+pos, len-pos, &nWritten, NULL)) pos+=nWritten;
CloseHandle(hFile);
return pos>=len;
}



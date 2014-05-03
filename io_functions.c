#include "consts.h"
#include "global.h"

typedef const char*(*Reader)(const wchar_t*, int*, BOOL*);
typedef BOOL(*Writer)(const wchar_t*, const char*, int) ;
typedef struct {
const char* name;
union { Reader reader; Writer writer; void* func; };
} RWEntry;
RWEntry *readers=0, *writers=0;
int nReaders=0, nMaxReaders=0, nWriters=0, nMaxWriters=0;

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

static inline void RWRegister (RWEntry** rw, int* nrw, int* nMaxRw, const char* name, void* func) {
if (*nrw>=*nMaxRw) *rw = realloc(*rw, ((*nMaxRw)+=3) * sizeof(RWEntry)); 
(*rw)[*nrw].func = func;
(*rw)[(*nrw)++].name = strdup(name);
}

static inline BOOL RWUnregister (RWEntry** rw, int* nrw, const char* name) {
for (int i=0; i<*nrw; i++) {
if (!strcmp((*rw)[i].name, name)) {
if (*nrw>0) (*rw)[i] = (*rw)[--(*nrw)];
return TRUE;
}}
return FALSE;
}

void __declspec(dllexport) RegisterReader (const char* name, Reader func) { RWRegister(&readers, &nReaders, &nMaxReaders, name, func); }
void __declspec(dllexport) RegisterWriter (const char* name, Writer func) { RWRegister(&writers, &nWriters, &nMaxWriters, name, func); }
BOOL __declspec(dllexport) UnregisterReader (const char* name) { return RWUnregister(&readers, &nReaders, name); }
BOOL __declspec(dllexport) UnregisterWriter (const char* name) { return RWUnregister(&writers, &nWriters, name); }

char* __declspec(dllexport) LoadFile (const wchar_t* fn, int* nSize, BOOL* readOnly) {
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

BOOL __declspec(dllexport) SaveFile (const wchar_t* fn, const char* buf, int len) {
HANDLE hFile = CreateFile(fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
if (hFile==INVALID_HANDLE_VALUE) return FALSE;
int pos=0, nWritten=0;
while (pos<len && WriteFile(hFile, buf+pos, len-pos, &nWritten, NULL)) pos+=nWritten;
CloseHandle(hFile);
return pos>=len;
}

const char* readFile (wchar_t* fn, int* nSize, BOOL* rdo) {
for (int i=0; i<nReaders; i++) {
const char* re = readers[i].reader(fn, nSize, rdo);
if (re) return re;
}
return NULL;
}

BOOL writeFile (wchar_t* fn, const char* buf, int len) {
for (int i=0; i<nWriters; i++) {
if (writers[i].writer(fn,buf,len)) return TRUE;
}
return FALSE;
}

static void __attribute__((constructor)) RegisterStdRW (void) {
printf("RegisterReader=%p\r\n", RegisterReader);
RegisterReader("file", LoadFile);
RegisterWriter("file", SaveFile);
}


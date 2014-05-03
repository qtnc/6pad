#include<windows.h>
#include<string.h>
#include<stdio.h>
#include<pcre.h>
#define _INSIDE_PCRE
#include "pcre-functions.h"
#include "global.h"
#include "consts.h"

#define PREGLEN 60
char* pregerrormsg = 0;
int pregerrorof = -1;
static int vector[PREGLEN];

int preg_match (pcre* regex, int options, const char* str, int offset, int length, int** result) {
if (!str || !regex) return -1;
if (length<=0) length=strlen(str);
int re = pcre_exec(regex, NULL, str, length, offset, options & PCRE_RUNTIME_MASK | PCRE_RUNTIME_FORCE, vector, PREGLEN);
if (result) {
if (re<=0) *result=0;
else *result = vector;
}
return re;
}

char* preg_replace_callback  (pcre* regex, int options, preg_callback repl, const char* str, int offset, int length, int* nRepls, void* udata) {
if (!str) return 0;
if (length<=0) length=strlen(str);
int result, nreps = 0;
int cpos=offset, pos = 0, cap = length;
char* buf = malloc(cap+1);
while (offset<length && (result = pcre_exec(regex, NULL, str, length, offset, options & PCRE_RUNTIME_MASK | PCRE_RUNTIME_FORCE, vector, PREGLEN))>=0) {
char* replacement = repl(regex, str, vector, result, udata);
if (!replacement) {  // Keep matched string intact
strapp(&buf, &pos, &cap, str+cpos, vector[1]-cpos);
cpos = offset = vector[1];
continue;
}
if (vector[0]==vector[1] && vector[0]>0 && str[vector[0]]=='\n' && str[vector[0] -1]=='\r') vector[0]=++vector[1]; // unsure "^" used alone bug patch
strapp(&buf, &pos, &cap, str+cpos, vector[0]-cpos);
strapp(&buf, &pos, &cap, replacement, strlen(replacement));
if (replacement!=udata) free(replacement);
cpos = offset = vector[1];
if (vector[0]==vector[1] && vector[0]>0 && str[vector[0]]=='\r' && str[vector[0] +1]=='\n') offset++; // unsure "$" used alone bug patch
if (vector[0]==vector[1]) offset++;
if (++nreps>=65535) break;
}
strapp(&buf, &pos, &cap, str+cpos, length-cpos); // append tail
if (nRepls) *nRepls = nreps;
buf[pos]=0;
return buf;
}

static char* defaultReplacer (pcre* regex, const char* str, const int* vector, int result, const char* repl) ;

char* preg_replace (pcre* regex, int options, const char* repl, const char* str, int offset, int length, int* nRe) {
return preg_replace_callback(regex, options, defaultReplacer, str, offset, length, nRe, repl);
}

int strpos (const char*, const char*, int);
static int evenBackslashes (const char* str, int i) {
int c = 0;
while (i>0 && str[--i]=='\\') c++;
return !(c&1);
}
static char* drepDoGroup (const char* str, const char* subj, const int* vector, int grp) {
char needle[4] = "$\0\0\0";
itoa(grp,needle+1,10);
int rpos = vector[grp*2], rlen = vector[grp*2+1]-vector[grp*2];
return strcfsub(needle, strpos, evenBackslashes, subj+rpos, rlen, str, NULL);
}

static char* defaultReplacer (pcre* regex, const char* str, const int* vector, int result, const char* irepl) {
if (!irepl) irepl = "";
const char* repl = irepl;
for(int i=0; i<result && i<=99; i++) {
const char* newrepl = drepDoGroup(repl, str, vector, i);
if (newrepl!=repl && repl!=irepl) free(repl); 
repl = newrepl;
}
#define REPL(a,b) {\
const char* newrepl = strcfsub(a, strpos, evenBackslashes, b, 1, repl, NULL); \
if (newrepl!=repl && repl!=irepl) free(repl); repl = newrepl; \
}
REPL("\\r", "\r")
REPL("\\n", "\n")
REPL("\\t", "\t")
REPL("\\e", "\e")
REPL("\\f", "\f")
REPL("\\\\", "\\")
#undef REPL
//strcfsub (const char *needle, int(*find)(const char*,const char*,int), int(*pred)(const char*,int), const char *repl, int replLen, const char *str, int* pcount)
return repl;
}

const wchar_t* doRegexReplace (const wchar_t* pattern, const wchar_t* replace, const wchar_t* string, int flags, int* nRe) {
const char* pat = strcvt(pattern, CP_UTF16, CP_UTF8, NULL);
const char* repl = strcvt(replace, CP_UTF16, CP_UTF8, NULL);
const char* str  = strcvt(string, CP_UTF16, CP_UTF8, NULL);
int options = PCRE_DEFAULT_OPTIONS;
if (flags&1) options |= PCRE_CASELESS;
if (flags&2) options |= PCRE_UTF8 | PCRE_NO_UTF8_CHECK;
pcre* regex = preg_compile(pat, options & PCRE_COMPILETIME_MASK);
if (!regex) {
free(str); free(pat); free(repl);
return 0; // Pattern syntax error
}
const char* re = preg_replace(regex, options, repl, str, 0, 0, nRe);
const wchar_t* result = strcvt(re, CP_UTF8, CP_UTF16, NULL);
free(re); free(str); free(pat); free(repl); pcre_free(regex);
return result;
}

int doRegexSearch (const wchar_t* string, const wchar_t* pattern, int offset, int flags, int* ptr1, int* ptr2) {
int encoding = (flags&2? CP_UTF8 : 0);
const char* pat = strcvt(pattern, CP_UTF16, encoding, NULL);
const unsigned char* str  = strcvt(string, CP_UTF16, encoding, NULL);
int options = PCRE_DEFAULT_OPTIONS;
if (flags&1) options |= PCRE_CASELESS;
if (flags&2) options |= PCRE_UTF8 | PCRE_NO_UTF8_CHECK;
pcre* regex = preg_compile(pat, options & PCRE_COMPILETIME_MASK);
if (!regex) {
free(str); free(pat);
return -2; // Pattern syntax error
}
int* match = 0;
int re = preg_match(regex, options, str, offset, 0, &match);
if (re>=0) {
re = match[0];
int re0 = match[0], re1 = match[1];
if (encoding==CP_UTF8) {
int c0=0, c1=0;
for (int i=re1 -1; i>=0; i--) {
if (str[i]>=0x80 && str[i]<0xC0) continue;
if (i<re0) c0++;
c1++;
}
re0=c0; re1=c1;
}
if (ptr1) *ptr1 = re0;
if (ptr2) *ptr2 = re1;
}
free(str); free(pat); pcre_free(regex);
return re;
}


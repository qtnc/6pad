#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<wchar.h>
#include "consts.h"
#include "global.h"

#define INDENTATION_DETECTION_MAX_LOOKUP 16384

wchar_t* unicodeSwitchEndianess (wchar_t* s, int l) {
union { struct { unsigned char a, b; }; short s; } u;
int i; for(i=0; i<l; i++) {
u.s = s[i];
unsigned char z = u.a;
u.a = u.b;
u.b = z;
s[i] = u.s;
}
return s;
}

static const wchar_t* mb2wc (const char* s, int csz, unsigned int cp, int* sz) {
if (!s) return NULL;
if (csz<=0) csz = strlen(s);
int nSize = MultiByteToWideChar(cp, 0, s, csz, NULL, 0);
if (nSize<0) return NULL;
wchar_t* wbuf = malloc((4+nSize)*sizeof(wchar_t));
nSize = MultiByteToWideChar(cp, 0, s, csz, wbuf, nSize);
if (nSize<0) { free(wbuf); return NULL; }
wbuf[nSize]=0;
if (sz) *sz = nSize;
return wbuf;
}

static const char* wc2mb (const wchar_t* ws, int wssz, unsigned int cp, int* sz) {
if (!ws) return NULL;
if (wssz<=0) wssz = wcslen(ws);
int nSize = WideCharToMultiByte(cp, 0, ws, wssz, NULL, 0, NULL, NULL);
if (nSize<0) return NULL;
char* buf = NULL;
buf = malloc(nSize+4);
nSize = WideCharToMultiByte(cp, 0, ws, wssz, buf, nSize, NULL, NULL);
if (nSize<0) { free(buf); return NULL; }
if (sz) *sz=nSize;
buf[nSize]=0;
return buf;
}

const void* __declspec(dllexport) strncvt (const void* str, int strLen, unsigned int cpFrom, unsigned int cpTo, int* len) {
if (!str) return NULL;
if (strLen<0) strLen = (cpFrom>=1200&&cpFrom<=1203? wcslen(str) : strlen(str));
if (cpFrom==cpTo) {
if (len) *len = strLen;
return str;
}
const void* from = NULL, *result = NULL;
BOOL alloced = FALSE;
switch(cpFrom){
case CP_UTF16_LE: 
from=str;
if (len) *len = strLen;
break;
case CP_UTF16_LE_BOM: 
from = ((const wchar_t*)str) +1; 
strLen--;
if (len) *len = strLen;
break;
case CP_UTF16_BE_BOM: 
from = ((const wchar_t*)str) +1;
strLen--;
case CP_UTF16_BE :
if (!from) from = str;
from = wcsdup(from);
unicodeSwitchEndianess(from, 1+wcslen(from));
if (len) *len = strLen;
alloced=TRUE;
break;
case CP_UTF8_BOM :
from = ((const char*)str) +3;
cpFrom = CP_UTF8;
strLen -= 3;
default :
if (!from) from=str;
from = mb2wc(from, strLen, cpFrom, &strLen);
if (len) *len = strLen;
alloced = TRUE;
break;
}
if (!from) return NULL;
switch(cpTo){
case CP_UTF16_LE: 
result = from;
((wchar_t*)result)[strLen]=0; 
alloced = FALSE;
break;
case CP_UTF16_LE_BOM :
if (alloced) {
result = from;
memmove( ((wchar_t*)result)+1, ((wchar_t*)result), sizeof(wchar_t)*(1+strLen));
}
else {
result = malloc(sizeof(wchar_t) * (4+strLen));
memcpy( ((wchar_t*)result)+1, from, sizeof(wchar_t)*(1+strLen));
}
(*((wchar_t*)result)) = 0xFEFF;
((wchar_t*)result)[++strLen] = 0;
if (len) *len = strLen;
alloced = FALSE;
break;
case CP_UTF16_BE :
result = (alloced? from : wcsdup(from));
unicodeSwitchEndianess(result, 3+wcslen(result));
alloced = FALSE;
((wchar_t*)result)[strLen] = 0;
if (len) *len = strLen;
break;
case CP_UTF16_BE_BOM :
if (alloced) {
result = from;
memmove( ((wchar_t*)result)+1, ((wchar_t*)result), sizeof(wchar_t)*(1+strLen));
}
else {
result = malloc(sizeof(wchar_t) * (4+strLen));
memcpy(((wchar_t*)result)+1, from, sizeof(wchar_t)*(1+strLen));
}
(*(((wchar_t*)result))) = 0xFEFF;
((wchar_t*)result)[++strLen] = 0;
unicodeSwitchEndianess(result, 3+strLen);
if (len) *len = strLen;
alloced = FALSE;
break;
case CP_UTF8_BOM :
result = wc2mb(from, strLen, CP_UTF8, &strLen);
memmove( ((const char*)result)+3, ((const char*)result), 1+strLen);
memcpy(result, "\xEF\xBB\xBF", 3);
if (len) (*len) = strLen+3;
break;
default :
result = wc2mb(from, strLen, cpTo, &strLen);
if (len) *len = strLen;
break;
}
if (alloced) free(from);
return result;
}

const void* __declspec(dllexport) strcvt (const void* str, unsigned int cpFrom, unsigned int cpTo, int* len) {
return strncvt(str, -1, cpFrom, cpTo, len);
}

static inline BOOL testUtf8rule (unsigned char** x, int n) {
int i = 0;
while (i<n && (*++(*x)&0xC0)==0x80) i++;
return i==n;
}

int guessEncoding (const unsigned char* ch) {
if (ch[0]==0xEF && ch[1]==0xBB && ch[2]==0xBF) return CP_UTF8_BOM;
if (ch[0]==255 && ch[1]==254) return CP_UTF16_LE_BOM;
if (ch[0]==254 && ch[1]==255) return CP_UTF16_BE_BOM;
if (ch[1]==0 && ch[3]==0 && ch[5]==0) return CP_UTF16_LE;
if (ch[0]==0 && ch[2]==0 && ch[4]==0) return CP_UTF16_BE;
//int q; for (q=0; q<4; q++) printf("ch[%d]=%d (0x%02X)\r\n", q, ch[q], ch[q]);
BOOL encutf = FALSE;
for (unsigned char* x = ch; *x; ++x) {
if (*x<0x80) continue;
if (*x>=0x81 && *x<=0x90) return 850; 
else if (*x==164) return 28605;
else if ((*x>=0x80 && *x<0xC0) || *x>=248) return CP_ACP;
else if (*x>=0xF0 && !testUtf8rule(&x, 3)) return CP_ACP;
else if (*x>=0xE0 && !testUtf8rule(&x, 2)) return CP_ACP;
else if (*x>=0xC0 && !testUtf8rule(&x, 1)) return CP_ACP;
encutf = TRUE;
}
return encutf? CP_UTF8 : CP_ACP;
}

int guessIndentationMode (const wchar_t* s, int l, int def) {
for (int i=0; i<l && i<INDENTATION_DETECTION_MAX_LOOKUP; i++) {
if (s[i]=='\n') {
if (s[i+1]=='\t') return 0;
else if (s[i+1]==' ') {
int j = i+1;
while (j<l && s[j]==' ') j++;
return j-i-1;
}}}
return def;
}

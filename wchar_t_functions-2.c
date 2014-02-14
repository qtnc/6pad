#include<string.h>

#define MAX(a,b) (((a)>(b))?(a):(b))

int wcspos (const wchar_t *str, const wchar_t *needle, int i) {
int j=0;
while (str[i]) {
        if (str[i]==needle[j]) {
if (needle[++j]==0) return i-j+1;
        }
        else j=0;
        i++;
        }
return -1;
}

int wcsipos (const wchar_t *str, const wchar_t *needle, int i) {
int j=0;
while (str[i]) {
        if (tolower(str[i])==tolower(needle[j])) {
if (needle[++j]==0) return i-j+1;
        }
        else j=0;
        i++;
        }
return -1;
}

void wcsapp (wchar_t** buf, int* pos, int* cap, const wchar_t* str, int len) {
if (len==0 || !str) return;
if (len<0) len = wcslen(str);
if (*pos + len + 1 >= *cap) {
*cap = MAX(*cap *3/2+2, *pos+len+1);
*buf = realloc(*buf, sizeof(wchar_t)*(*cap));
}
memcpy(*buf+*pos, str, len*sizeof(wchar_t));
*pos+=len;
}

wchar_t* wcsfsub (const wchar_t *needle, int(*find)(const wchar_t*,const wchar_t*,int), const wchar_t *repl, const wchar_t *str, int* pcount) {
    int needlen = wcslen(needle), replLen = wcslen(repl);
    int lastIndex = 0, index = 0, cap=0, pos=0, count=0;
wchar_t* newstr = NULL;
while ((index = find(str, needle, index )) != -1) {
wcsapp(&newstr, &pos, &cap, str+lastIndex, index-lastIndex);
wcsapp(&newstr, &pos, &cap, repl, replLen);
index += needlen;
lastIndex = index;
count++;
            }
if (count>0) {
wcsapp(&newstr, &pos, &cap, str+lastIndex, -1);
newstr[pos]=0;
}
if (pcount) *pcount = count;
return newstr? newstr : str;
}

wchar_t* wcssub (const wchar_t *needle, const wchar_t *repl, const wchar_t *str, int* count) {
return wcsfsub(needle, wcspos, repl, str, count);
}

wchar_t* wcsisub (const wchar_t *needle, const wchar_t *repl, const wchar_t *str, int* count) {
return wcsfsub(needle, wcsipos, repl, str, count);
}


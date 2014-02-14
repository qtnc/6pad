#include<string.h>

#define MAX(a,b) (((a)>(b))?(a):(b))

int strpos (const char *str, const char *needle, int i) {
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

void strapp (char** buf, int* pos, int* cap, const char* str, int len) {
if (len==0 || !str) return;
if (len<0) len = strlen(str);
if (*pos + len + 1 >= *cap) {
*cap = MAX(*cap *3/2+2, *pos+len+1);
*buf = realloc(*buf, *cap);
}
memcpy(*buf+*pos, str, len);
*pos+=len;
}

char* strcfsub (const char *needle, int(*find)(const char*,const char*,int), int(*pred)(const char*,int), const char *repl, int replLen, const char *str, int* pcount) {
    int needlen = strlen(needle), lastIndex = 0, index = 0, cap=0, pos=0, count=0;
char* newstr = NULL;
if (replLen<0) replLen = strlen(repl);
while ((index = find(str, needle, index )) != -1) {
if (!pred || pred(str,index)) {
strapp(&newstr, &pos, &cap, str+lastIndex, index-lastIndex);
strapp(&newstr, &pos, &cap, repl, replLen);
lastIndex = index+needlen;
count++;
}
index += needlen;
            }
if (count>0) {
strapp(&newstr, &pos, &cap, str+lastIndex, -1);
newstr[pos]=0;
}
if (pcount) *pcount = count;
return newstr? newstr : str;
}


char* trim (char* s) {
if (!s) return s;
int l = strlen(s), b=0, e=l -1;
if (l<=0) return s;
while (b<l && s[b]<=32) b++;
while (e>0 && s[e]<=32) e--;
s[e+1]=0;
if (b>0) s[b-1]=0;
return s+b;
}


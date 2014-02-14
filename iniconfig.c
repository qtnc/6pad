#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include "consts.h"
#include "global.h"
#include "list.h"

#define streq(a,b) (0==stricmp(a,b))

extern struct { wchar_t path[300]; time_t time; } recentFiles[7] ;
extern int curFlags, defaultEncoding, defaultLineEnding, defaultTabSpaces, fontSize;
extern BOOL searchFlags, searchRegex, searchUpward;
extern wchar_t fontFace[32];
extern list* extensionList;

static inline void parseflag (int* i, int flag, char* data) {
if (streq(data,"true") || strtoul(data, NULL, 10)!=0) *i|=flag;
else *i&=~flag;
}

void readConfig (const char* fn) {
FILE* fd = fopen(fn, "r");
if (!fd) return;
char *ch=0, line[512] = {0};
while (fgets(line, 511, fd)) {
if (line[0]=='\n' || line[0]=='\r' || line[0]==' ' || line[0]==';' || line[0]=='#') continue;
if (!(ch = strchr(line, '='))) continue;
*ch=0; ch++;
int chl = strlen(ch);
if (chl>2 && ch[chl-2]=='\r') ch[chl-2]=0;
else if (chl>1 && ch[chl-1]=='\n') ch[chl-1]=0;
if (0) {}
#define cival(n) else if (streq(line,#n)) n = strtoul(ch, NULL, 10);
#define cflag(n,f) else if (streq(line,#n)) parseflag(&curFlags, f, ch);
cival(defaultEncoding)
cival(defaultLineEnding)
cival(defaultTabSpaces)
cival(fontSize)
cflag(autoLineWrap,1)
cflag(autoReload,2)
cflag(smartHome,4)
cflag(followIndentation,8)
cflag(showStatusBar,16)
cflag(singleInstance,32)
else if (streq(line, "fontFace")) wsprintf(fontFace, L"%hs", ch);
else if (streq(line,"extension")) l_add(extensionList, strdup(ch), -1);
else if (streq(line, "searchFlags")) searchFlags = strtoul(ch,NULL,10);
else if (streq(line, "searchRegex")) searchRegex = strtoul(ch,NULL,10)!=0;
else if (streq(line, "searchUp")) searchUpward = strtoul(ch,NULL,10)!=0;
#undef cflags
#undef cival
else if (strnicmp(line, "file", 4)==0) {
int nFile = *(ch -2) -'1';
if (nFile<0 || nFile>=7) continue;
char* ch2 = strchr(ch, '@');
if (!ch2) continue;
*ch2=0; ch2++;
recentFiles[nFile].time = strtoul(ch2, NULL, 10);
int wcl;  const wchar_t* wc = strcvt(ch, CP_ACP, CP_UTF16, &wcl);
memcpy(recentFiles[nFile].path, wc, (wcl+1)*sizeof(wchar_t) );
free(wc);
//printf("file%d=%ls@%d\r\n", nFile+1, recentFiles[nFile].path, recentFiles[nFile].time);
}}
if (defaultTabSpaces>8) defaultTabSpaces=8; else if (defaultTabSpaces<0) defaultTabSpaces=0;
fclose(fd);
}

void saveConfig (const char* fn) {
FILE* fd = fopen(fn, "wb");
if (!fd) return;
fprintf(fd, "defaultEncoding=%d\r\n", defaultEncoding);
fprintf(fd, "defaultLineEnding=%d\r\n", defaultLineEnding);
fprintf(fd, "defaultTabSpaces=%d\r\n", defaultTabSpaces);
fprintf(fd, "fontFace=%ls\r\nfontSize=%d\r\n", fontFace, fontSize);
fprintf(fd, "autoLineWrap=%d\r\n", curFlags&1?1:0);
fprintf(fd, "autoReload=%d\r\n", curFlags&2?1:0);
fprintf(fd, "smartHome=%d\r\n", curFlags&4?1:0);
fprintf(fd, "followIndentation=%d\r\n", curFlags&8?1:0);
fprintf(fd, "showStatusBar=%d\r\n", curFlags&16?1:0);
fprintf(fd, "singleInstance=%d\r\n", curFlags&32?1:0);
fprintf(fd, "searchFlags=%d\r\nsearchRegex=%d\r\nsearchUp=%d\r\n", searchFlags, searchRegex, searchUpward);
int i=0; for (int i=0; i<7; i++) {
if (!*recentFiles[i].path) continue;
fprintf(fd, "file%d=%ls@%d\r\n", i+1, recentFiles[i].path, recentFiles[i].time);
}
int n = extensionList?l_len(extensionList):0;
for (i=0; i<n; i++) fprintf(fd, "extension=%s\r\n", l_item(extensionList,i));
fclose(fd);
}


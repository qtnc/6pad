#include "consts.h"
#include "list.h"
#include "pcre-functions.h"
#include "global.h"
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<wchar.h>
#include<time.h>

LRESULT CALLBACK winproc (HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK editproc (HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK tabproc (HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK lblproc (HWND,UINT,WPARAM,LPARAM);
void initializeLua () ;
void setEncoding (int enc) ;
void setLineEnding (int le) ;
void setTabSpaces (int ts) ;
void fillIndentString (wchar_t*, int, int) ;
void openDialog (int flags) ;
void saveDialog () ;
void open (const wchar_t* fn, int flags, int filterIndex) ;
void save (pagecontext* p) ;
void save4 (const wchar_t* fn, int filterIndex) ;
pagecontext* addPage () ;
void restorePage (pagecontext*) ;
void savePage (pagecontext*) ;
void addEmptyTab () ;
BOOL deleteCurrentTab () ;
void updateStatusBar () ;
void updateWindowTitle (void) ;
void updatePageName (pagecontext* p, const wchar_t* fn) ;
HWND createEditControl () ;
void handleEnterKey () ;
void handleHomeKey();
void handleTabKey () ;
void handleNextModlessWindow (int o) ;
void handleNextTab (int o) ;
void checkAutoreload();
void insertTabs (int) ;
void findNext () ;
void findPrev () ;
const wchar_t* doRegexReplace (const wchar_t*, const wchar_t*, const wchar_t*, int, int* ) ;
void switchCursor () ;
void doUndo () ;
void doRedo () ;
void addUndoState () ;
void editAboutToChange (int delta) ;
void updateRecentFilesMenu (const wchar_t* fn) ;
int addCustomCommand (void* p) ;
void removeCustomCommand (void* p) ;
void** getCustomCommand (int n) ;
void handleCustomCommand (void* p) ;
void setClipboardText (const wchar_t* text, int len) ;
wchar_t* getClipboardText (int* len) ;
void addAccelerator (int flags, int key, int cmd) ;
BOOL removeAccelerator (int cmd) ;
BOOL findAccelerator (int cmd, int* flags, int* key) ;
HWND showOutputWindow (const wchar_t* prompt, const wchar_t* text, const wchar_t* title) ;
void* showListDialog (const wchar_t* prompt, const wchar_t* itemlist, const wchar_t* title, int flags) ;
wchar_t* showFileDialog (const wchar_t* file, const wchar_t* title, const wchar_t* filters, int* nFilter, int flags) ;
int wcspos (const wchar_t* str, const wchar_t* needle, int pos) ;
int wcsipos (const wchar_t* str, const wchar_t* needle, int pos) ;

HINSTANCE hinst;
HWND win, edit, status, tabctl, console;
HFONT font; wchar_t fontFace[32]= L"Courier new"; int fontSize = 12;
HMENU hFormatMenu = 0;
HACCEL hAccel;
WNDPROC oldeditproc, oldlblproc, oldtabproc;
HWND activeModlessDlgs[16] = {0};
pagecontext *curPage=0, **pages=0;
int nPages = 0, nMaxPages=0, nActiveModlessDlgs = 0;
BOOL searchRegex=FALSE, searchUpward = FALSE; DWORD searchFlags=0;
wchar_t *curSearch=0, *curReplace=0;
void** customCommands = 0;
int nCustomCommands = 16;
const int encs[] = { CP_ACP, CP_UTF7, CP_UTF8, CP_UTF8_BOM, CP_UTF16, CP_UTF16_LE_BOM, CP_UTF16_BE, CP_UTF16_BE_BOM, 28605, 437, 850,  500, 10000 };
const int nEncs = 14;
BOOL lastWasCommand = FALSE;
list* searchList=NULL, *replaceList=NULL, *extensionList = NULL;
int curKeyFlags=0, curFlags=30, curRow=-1, curCol=-1, untitledIndex = 1;
int defaultEncoding = CP_ACP, defaultLineEnding = 0, defaultTabSpaces=0;
char confFn[512]={0};
wchar_t CLASSNAME[32]={0}, EXENAME[32]={0};
wchar_t* globalFilenameFilter = NULL; int globalFilenameFilterLength=0;
struct { wchar_t path[300]; time_t time; } recentFiles[7] = {{0,0}, {0, 0}, {0, 0}, {0,0}, {0,0}, {0,0}, {0,0} }	;

void sigsegv (int x) {
/*double log440 = (log(440)/log(2)) + 0.25;
int zz[] = { 12, -50, -50, -50, 4, -50, -50, -50, 5, -50, -50, -50, 6, -50, -50, -50, 7, -50, 9, 11, -50, 12, -50, -50, -50, -50, -50, -50,-100 };
int*z = zz;
while (*z>-100) {
if (*z>-50) Beep(round(pow(2, log440+(*z/12.0))), 120);
else Sleep(120);
z++;
}*/
int len = GetWindowTextLength(edit);
wchar_t* buf = malloc(sizeof(wchar_t) * (len+8));
GetWindowText(edit, buf, len+1);
buf[len]=0;
wchar_t wfbuf[512]={0};
wsprintf(wfbuf, L"%ls-auto-save.tmp", EXENAME);
writeFile(wfbuf, buf, len*sizeof(wchar_t));
free(buf);
wsprintf(wfbuf, MSG_SAVEONCRASH, EXENAME);
MessageBox(win?win:NULL, wfbuf, MSG_ERROR, MB_ICONERROR | MB_OK);
exit(1);
}

int WINAPI WinMain (HINSTANCE hThisInstance,                      HINSTANCE hPrevInstance,                      LPSTR lpszArgument,                      int nWindowStile) {
hinst = hThisInstance;
long long time = GetTickCount();

signal(SIGSEGV, sigsegv);
signal(SIGFPE, sigsegv);
signal(SIGILL, sigsegv);
SetErrorMode(SEM_FAILCRITICALERRORS);

customCommands = malloc(sizeof(void*) * nCustomCommands);
memset(customCommands, 0, nCustomCommands * sizeof(void*));
pages = malloc(nMaxPages * sizeof(pagecontext*));
memset(pages, 0, nMaxPages*sizeof(pagecontext*));

GetModuleFileNameA(NULL, confFn, 512);
char *confFnB = strrchr(confFn, '\\');
char* confFnX = strrchr(confFn, '.');
*confFnX=0;
wsprintf(CLASSNAME, L"QC6pad14-%hs", confFnB+1);
wsprintf(EXENAME, L"%hs", confFnB+1);
*confFnX='.';

    WNDCLASSEX wincl;
wincl.hInstance = hThisInstance;
wincl.lpszClassName = CLASSNAME;
wincl.lpfnWndProc = winproc;
wincl.style = CS_DBLCLKS;
wincl.cbSize = sizeof (WNDCLASSEX);
wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
wincl.lpszMenuName = L"menu";
wincl.cbClsExtra = 0;
wincl.cbWndExtra = 0;
wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

if (!RegisterClassEx(&wincl)) {
MessageBox(NULL, L"Couldn't register window class", L"Fatal error", MB_OK|MB_ICONERROR);
return 1;
}
INITCOMMONCONTROLSEX ccex = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES |  ICC_HOTKEY_CLASS | ICC_PROGRESS_CLASS | ICC_UPDOWN_CLASS | ICC_TAB_CLASSES  };
if (!InitCommonControlsEx(&ccex)) return 1;
win = CreateWindowEx(
WS_EX_CONTROLPARENT,
CLASSNAME, L"", 
WS_VISIBLE | WS_OVERLAPPEDWINDOW,
CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
HWND_DESKTOP, NULL, hinst, NULL);
RECT r; GetClientRect(win, &r);
tabctl = CreateWindowEx(
0, WC_TABCONTROL, L"Tab bar window text", 
WS_TABSTOP | WS_VISIBLE | WS_CHILD | TCS_SINGLELINE | TCS_FOCUSNEVER,
5, 5, r.right -10, r.bottom -49, 
win, IDT_TABBAR, hinst, NULL);
status = CreateWindowEx(
0, L"STATIC", L"Li 1, col 1", 
WS_VISIBLE | WS_CHILD | WS_BORDER | SS_NOPREFIX | SS_LEFT, 
5, r.bottom -32, r.right -10, 27, 
win, IDT_STATUSBAR, hinst, NULL);
oldlblproc = SetWindowLong(status, GWL_WNDPROC, lblproc);
oldtabproc = SetWindowLong(tabctl, GWL_WNDPROC, tabproc);

sprintf(confFnX, ".ini");
readConfig(confFn);
font = loadFont(win, fontFace, fontSize);
addEmptyTab();
edit = createEditControl();

hAccel = LoadAccelerators(hinst, L"accel");
hFormatMenu = GetSubMenu(GetMenu(win), 2);
int i; for (i=0; i < 5; i++) CheckMenuItem(hFormatMenu, i+3, MF_BYPOSITION | ((curFlags&(1<<i))? MF_CHECKED : MF_UNCHECKED));
SetFocus(edit);
setEncoding(defaultEncoding);
setLineEnding(defaultLineEnding);
setTabSpaces(defaultTabSpaces);
updateRecentFilesMenu(NULL);

int argc = 0;
wchar_t** argv = CommandLineToArgvW(GetCommandLineW(),&argc);
initializeLua(argc, argv);

//for (int i=0; i<argc; i++) printf("argv[%d]=%ls\r\n", i, argv[i]);
if (argc>=2 && argv[1] && argv[1][0]!='-' && argv[1][0]!='/') {
open(argv[1], OF_QUIET | OF_REUSEOPENEDTABS | OF_EXITONDOUBLEOPEN, 0);
updatePageName(curPage, argv[1]);
updateWindowTitle();
}
// Use other command line arguments
LocalFree(argv);

ev_onTabNew(curPage); 

time = GetTickCount() -time;
printf("Init time = %d ms\r\n", time);

ShowWindow(win, nWindowStile);
if (!(curFlags&16)) ShowWindow(status, SW_HIDE);

MSG msg;
while (GetMessage(&msg,NULL,0,0)) {
if (msg.message==WM_KEYDOWN) {
switch (LOWORD(msg.wParam)) {
case VK_LSHIFT: case VK_RSHIFT: case VK_SHIFT : curKeyFlags|=1; break;
case VK_RCONTROL: case VK_LCONTROL: case VK_CONTROL : curKeyFlags|=2; break;
case VK_LMENU: case VK_RMENU: case VK_MENU : curKeyFlags|=4; break;
case VK_F6 : if (!(curKeyFlags&6)) {
handleNextModlessWindow(curKeyFlags&1? -1 : 1);
continue;
}break;
}
if (msg.hwnd==edit) {
switch (msg.wParam) {
case VK_RETURN : 
if (curKeyFlags || !(curFlags&8)) break;
PostMessage(win, WM_USER, IDT_EDIT, VK_RETURN);
continue;
case VK_HOME :
if (!(curFlags&4) || curKeyFlags) break;
PostMessage(win, WM_USER, IDT_EDIT, VK_HOME);
continue;
case VK_TAB :
if (curKeyFlags&2) handleNextTab(curKeyFlags&1? -1 : 1); 
else if (curKeyFlags&6) break;
else PostMessage(win, WM_USER, IDT_EDIT, VK_TAB);
continue;
}}}
else if (msg.message==WM_KEYUP) {
switch (msg.wParam) {
case VK_RSHIFT: case VK_LSHIFT: case VK_SHIFT : curKeyFlags&=~1; break;
case VK_RCONTROL: case VK_LCONTROL: case VK_CONTROL : curKeyFlags&=~2; break;
case VK_RMENU : case VK_LMENU: case VK_MENU : curKeyFlags&=~6; break;
}}
lastWasCommand = FALSE;
TranslateAccelerator(win, hAccel, &msg);
if (lastWasCommand) continue;
if (msg.hwnd!=edit && msg.hwnd!=win) {
int i; for(i=0; i<nActiveModlessDlgs &&!IsDialogMessage(activeModlessDlgs[i],&msg); i++) ;
if (i!=nActiveModlessDlgs) continue;
}
TranslateMessage(&msg);	
DispatchMessage(&msg);
}

saveConfig(confFn);
if (font) DeleteObject(font);
return msg.wParam;
}

pagecontext* addPage () {
if (nPages>=nMaxPages) {
int k = nMaxPages *3/2+1;
pages = realloc(pages, k*sizeof(pagecontext*));
memset(pages+nMaxPages, 0, (k-nMaxPages)*sizeof(pagecontext*));
nMaxPages = k; 
}
pagecontext* p = malloc(sizeof(pagecontext));
memset(p, 0, sizeof(pagecontext));
p->curEncoding = defaultEncoding;
p->curLineEnding = defaultLineEnding;
p->tabSpaces = defaultTabSpaces;
fillIndentString(p->tabmlr, 10, defaultTabSpaces);
pages[nPages++] = p;
return p;
}

void restorePage (pagecontext* p) {
setEncoding(p->curEncoding);
setLineEnding(p->curLineEnding);
setTabSpaces(p->tabSpaces);
updatePageName(p, p->curFile);
updateWindowTitle();
SetWindowText(edit, p->curText);
SendMessage(edit, EM_SETSEL, p->curSelStart, p->curSelEnd);
SendMessage(edit, EM_SETREADONLY, p->editReadOnly, 0);
SendMessage(edit, EM_SETMODIFY, p->modified, 0);
updateStatusBar();
if (p->curText) free(p->curText);
p->curText=0;
}

void savePage (pagecontext* p) {
p->modified = SendMessage(edit, EM_GETMODIFY, 0, 0);
p->editReadOnly = (ES_READONLY&GetWindowLong(edit, GWL_STYLE));
SendMessage(edit, EM_GETSEL, &p->curSelStart, &p->curSelEnd);
int len = GetWindowTextLength(edit);
p->curText = malloc((len+1)*sizeof(wchar_t));
GetWindowText(edit, p->curText, len+1);
p->curText[len]=0;
}

void deletePage (pagecontext* p, int i) {
if (!p) return;
if (p->curText) free(p->curText);
if (p->curFile) free(p->curFile);
if (p->curName) free(p->curName);
for (int i=0; i<MAXUNDO; i++) if (p->undoBuffer[i].str) free(p->undoBuffer[i].str);
free(p);
if (i>=0 && i<nPages) {
memmove(pages+i, pages+i+1, (nPages-i -1)*sizeof(pagecontext*));
pages[--nPages]=0;
}
}

BOOL showTab (const wchar_t* name) {
for (int i=0; i<nPages; i++) {
if (
(pages[i]->curFile && 0==stricmp(pages[i]->curFile,name))
|| (pages[i]->curName && 0==stricmp(pages[i]->curName,name))
) {
SendMessage(tabctl, TCM_SETCURFOCUS, i, 0);
return TRUE;
}}
return FALSE;
}

void addEmptyTab () {
if (curPage) {
ev_onBeforeTabChange(curPage);
savePage(curPage);
}
TCITEM it;
it.mask = TCIF_TEXT;
it.pszText = MSG_UNTITLED;
int pos = SendMessage(tabctl, TCM_GETITEMCOUNT, 0, 0);
SendMessage(tabctl, TCM_INSERTITEM, pos, &it);
int oldpos = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
SendMessage(tabctl, TCM_HIGHLIGHTITEM, oldpos, FALSE);
SendMessage(tabctl, TCM_SETCURSEL, pos, 0);
SendMessage(tabctl, TCM_HIGHLIGHTITEM, pos, TRUE);
restorePage(curPage = addPage());
EnableWindow(edit, TRUE);
ev_onAfterTabChange(curPage);
}

BOOL deleteCurrentTab () {
if (!curPage) return;
if (!ev_onBeforeTabClose(curPage)) return;
if (SendMessage(edit, EM_GETMODIFY, 0, 0)) {
wchar_t str9[512]={0};
wsprintf(str9, MSG_SAVECHANGES, curPage->curName);
int re = MessageBox(win, str9, MSG_QUESTION, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
if (re==IDYES && curPage->curFile) save(curPage);
else if (re == IDYES && !curPage->curFile) saveDialog();
else if (re==IDCANCEL) return FALSE;
}
int pos = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
tablrelease(curPage);
deletePage(curPage, pos);
SendMessage(tabctl, TCM_DELETEITEM, pos, 0);
ev_onAfterTabClose(NULL);
if (pos>=nPages) pos = nPages -1;
if (pos>=0 && pos<nPages) {
curPage = pages[pos];
restorePage(curPage);
SendMessage(tabctl, TCM_SETCURSEL, pos, 0);
SendMessage(tabctl, TCM_HIGHLIGHTITEM, pos, TRUE);
}
else {
curPage=0;
EnableWindow(edit, FALSE);
}
return TRUE;
}

void handleNextTab (int n) {
int pos = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
pos = (pos + nPages + n)%nPages;
SendMessage(tabctl, TCM_SETCURFOCUS, pos, 0);
}

void restoreUndoState (undostate* u) {
if (!u->str) return;
SetWindowText(edit, u->str);
SendMessage(edit, EM_SETSEL, u->sStart, u->sEnd);
curPage->secondCursorPos = u->secondCursor;
curPage->secondCursor = u->secondCursor >= 0;
SendMessage(edit, EM_SETMODIFY, TRUE, 0);
updateStatusBar();
}

void saveUndoState (undostate* u) {
int len = GetWindowTextLength(edit), sStart, sEnd;
wchar_t* wstr = malloc(sizeof(wchar_t) * (len+1));
GetWindowText(edit, wstr, len+1);
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
wstr[len]=0;
u->str = wstr;
u->sStart = sStart;
u->sEnd = sEnd;
u->secondCursor = curPage->secondCursor? curPage->secondCursorPos : -1;
}

void clearUndoBuffer (int k) {
int i; for (i=k; i<MAXUNDO; i++) {
if (curPage->undoBuffer[i].str) { free(curPage->undoBuffer[i].str); curPage->undoBuffer[i].str=0; }
}
curPage->undoPos = curPage->undoLength = k;
}

void addUndoState () {
if (curPage->undoPos>=MAXUNDO) {
free(curPage->undoBuffer[0].str);
memmove(curPage->undoBuffer, curPage->undoBuffer+1, (MAXUNDO -1)*sizeof(undostate));
curPage->undoBuffer[--curPage->undoPos].str=0;
}
saveUndoState(curPage->undoBuffer+curPage->undoPos++);
if (curPage->undoPos>=curPage->undoLength) curPage->undoLength=curPage->undoPos;
curPage->shouldAddUndo = FALSE;
}

void doUndo () {
if (curPage->undoPos<=0) { MessageBeep(MB_OK); return; }
if (curPage->undoPos==curPage->undoLength) { addUndoState(); curPage->undoPos--; }
restoreUndoState(curPage->undoBuffer+(--(curPage->undoPos)));
curPage->shouldAddUndo = TRUE;
}

void doRedo () {
if (curPage->undoPos>=curPage->undoLength -1) { MessageBeep(MB_OK); return; }
restoreUndoState(curPage->undoBuffer+(++(curPage->undoPos)));
curPage->shouldAddUndo = TRUE;
}

void switchCursor () {
int e; SendMessage(edit, EM_GETSEL, 0, &e);
if (!curPage->secondCursor) curPage->secondCursorPos=e;
SendMessage(edit, EM_SETSEL, curPage->secondCursorPos, curPage->secondCursorPos);
curPage->secondCursorPos=e;
curPage->secondCursor=TRUE;
updateStatusBar();
}

void updateRecentFilesMenu (const wchar_t* fn) {
if (fn) {
int i, re=-1; time_t min = 2147483647L;
for (i=0; i<7; i++) {
if (recentFiles[i].path[0] && 0==wcscmp(recentFiles[i].path, fn)) { re = i; break; }
else if (!recentFiles[i].path[0]) { re=i; break; }
else if (recentFiles[i].time <min) { re=i; min=recentFiles[i].time; }
}
if (re>=0 && re<7) {
memcpy(recentFiles[re].path, fn, sizeof(wchar_t)*wcslen(fn)+1);
recentFiles[re].time = time(NULL);
}}

static HMENU rfm = 0;
if (!rfm) rfm = GetSubMenu(GetSubMenu(GetMenu(win), 0), 9);
wchar_t mlbl[304];
int i; for (i=0; i<7; i++) {
wsprintf(mlbl, L"&%d. %ls", i+1, recentFiles[i].path);
ModifyMenu(rfm, i, MF_BYPOSITION | MF_STRING, IDM_OPEN_RECENT_FILE+i, mlbl);
EnableMenuItem(rfm, i, MF_BYPOSITION | (recentFiles[i].path[0]? MF_ENABLED : MF_DISABLED));
}}

void doReplaceSel (int rStart, int rEnd, int sStart, int sEnd, const wchar_t* curSearch, const wchar_t* curReplace, DWORD searchFlags, BOOL searchRegex) {
addUndoState();
if (rStart<0 || rEnd<0) SendMessage(edit, EM_GETSEL, &rStart, &rEnd);
int len = GetWindowTextLength(edit);
if (len<=0) return;
wchar_t* wstr = malloc(sizeof(wchar_t) * (len+1));
GetWindowText(edit, wstr, len+1);
wstr[len]=0;
wstr[rEnd]=0;
const wchar_t* nstr = NULL;
int count = 0;
if (searchRegex) nstr = doRegexReplace(curSearch, curReplace, wstr+rStart, searchFlags, &count);
else {
if (searchFlags&1) nstr = wcssub(curSearch, curReplace, wstr+rStart, &count);
else nstr = wcsisub(curSearch, curReplace, wstr+rStart, &count);
}
if (nstr) {
SendMessage(edit, EM_REPLACESEL, TRUE, nstr);
if (sEnd-sStart<=0) SendMessage(edit, EM_SETSEL, sStart, sEnd);
else SendMessage(edit, EM_SETSEL, rStart, rStart+wcslen(nstr));
if (nstr&&nstr!=wstr) free(nstr);
}
free(wstr);
curPage->shouldAddUndo=TRUE;
wchar_t msg[128]={0};
wsprintf(msg, MSG_REPL_REPLSUCCESS, count);
SetWindowText(status,msg);
MessageBeep(MB_ICONASTERISK);
}

void setClipboardText (const wchar_t* text, int len) {
if (len<=0) len = wcslen(text);
HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(1+len));
if (!hMem) return;
wchar_t* mem = GlobalLock(hMem);
memcpy(mem, text, sizeof(wchar_t) * len);
mem[len]=0;
GlobalUnlock(hMem);
if (!OpenClipboard(win)) return;
EmptyClipboard();
SetClipboardData(CF_UNICODETEXT, hMem);
CloseClipboard();
}

wchar_t* getClipboardText (int* pLen) {
if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(win)) return NULL;
HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
const wchar_t* hMemData = GlobalLock(hMem);
int len = wcslen(hMemData);
wchar_t* wstr = malloc(sizeof(wchar_t) * (len+1));
memcpy(wstr, hMemData, sizeof(wchar_t) * len);
wstr[len]=0;
GlobalUnlock(hMem);
CloseClipboard();
if (pLen) *pLen = len;
return wstr;
}

HWND createEditControl () {
RECT r; GetClientRect(win, &r);
r.left = 5; r.top = 5; r.right -= 10; r.bottom -= 49;
SendMessage(tabctl, TCM_ADJUSTRECT, FALSE, &r);
HWND hEdit  = CreateWindow(L"EDIT", L"",
WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL | (curFlags&1? 0:ES_AUTOHSCROLL),
r.left+3, r.top+3, r.right - r.left -6, r.bottom - r.top -6,
tabctl, IDT_EDIT, hinst, NULL);
oldeditproc = SetWindowLong(hEdit, GWL_WNDPROC, editproc);
SendMessage(hEdit, EM_SETLIMITTEXT, 1073741823, 0);
SendMessage(hEdit, WM_SETFONT, font, TRUE);
int x = curPage->tabSpaces==0? 16 : ABS(curPage->tabSpaces)*4;
SendMessage(hEdit, EM_SETTABSTOPS, 1, &x);
return hEdit;
}

void recreateEditControl () {
int sStart, sEnd, len; 
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
len = GetWindowTextLength(edit);
wchar_t* text = malloc(sizeof(wchar_t) * (len+1));
GetWindowText(edit, text, len+1);
text[len]=0;
ShowWindow(edit, SW_HIDE);
DestroyWindow(edit);
edit = createEditControl();
SetWindowText(edit, text);
free(text);
SendMessage(edit, EM_SETSEL, sStart, sEnd);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
updateStatusBar();
SetFocus(edit);
}

void findNext () {
if (!curSearch) { SendMessage(win, WM_COMMAND, IDM_FIND, 0); return; }
int sStart, sEnd;
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
if (sStart!=sEnd) sStart++;
HLOCAL hEdit = SendMessage(edit, EM_GETHANDLE, 0, 0);
const wchar_t* str = LocalLock(hEdit);
int pos = -1, endpos = -1, re = -1;
if (searchRegex) {
re = doRegexSearch(str, curSearch, sStart, searchFlags, &pos, &endpos);
if (re<0) pos=endpos=-1;
}
else {
if (searchFlags&1) pos = wcspos(str, curSearch, sStart);
else pos = wcsipos(str, curSearch, sStart);
endpos = pos + wcslen(curSearch);
}
LocalUnlock(hEdit);
if (pos>=0) {
SendMessage(edit, EM_SETSEL, pos, endpos);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
}
else MessageBeep(MB_ICONASTERISK);
updateStatusBar();
}

void findPrev () {
if (!curSearch) { SendMessage(win, WM_COMMAND, IDM_FIND, 0); return; }
int sStart, sEnd;
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
HLOCAL hEdit = SendMessage(edit, EM_GETHANDLE, 0, 0);
const wchar_t* str = LocalLock(hEdit);
int pos = -1, endpos = -1, pos3=-1, endpos3=-1, n=0, wl=wcslen(curSearch);
int wcspos3 (const wchar_t* a, const wchar_t* b, int c, int* d, int* e) { int f = wcspos(a,b,c); if (f>=0) { if (d) *d=f; if (e) *e=f+wl; } return f; }
int wcsipos3 (const wchar_t* a, const wchar_t* b, int c, int* d, int* e) { int f = wcsipos(a,b,c); if (f>=0) { if (d) *d=f; if (e) *e=f+wl; } return f; }
int rgxpos3 (const wchar_t* a, const wchar_t* b, int c, int* d, int* e) { return doRegexSearch(a,b,c, searchFlags, d, e); }
int(*func)(const wchar_t*, const wchar_t*, int, int*, int*) =
(searchRegex? rgxpos3 : (
(searchFlags? wcspos3 : wcsipos3)
));
while ((n=func(str, curSearch, n, &pos3, &endpos3))>=0 && n<sStart) { n++; pos=pos3; endpos=endpos3; }
LocalUnlock(hEdit);
if (pos>=0 && pos<sStart) {
SendMessage(edit, EM_SETSEL, pos, endpos);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
}
else MessageBeep(MB_ICONASTERISK);
updateStatusBar();
}

void checkAutoreload () {
int pos = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
for (int i=0; i<nPages; i++) {
pagecontext* p = pages[i];
if (!p->curFile) continue;
unsigned long lastModified = getFileTime(p->curFile,LAST_MODIFIED_TIME);
if (lastModified>p->lastSaveTime) {
if ((i!=pos&&p->modified) || (i==pos&&SendMessage(edit, EM_GETMODIFY, 0, 0))) {
curFlags^=2;
wchar_t str9[512]={0};
wsprintf(str9, MSG_CROSSMODIFICATIONS, p->curName);
int re = MessageBox(win, str9, MSG_QUESTION, MB_YESNO | MB_ICONEXCLAMATION);
curFlags^=2;
if (re!=IDYES) continue;
}
SendMessage(tabctl, TCM_SETCURFOCUS, i, 0);
open(curPage->curFile, OF_KEEPSELECTION | OF_FORCEENCODING, curPage->curFilenameFilterIndex);
}}
SendMessage(tabctl, TCM_SETCURFOCUS, pos, 0);
}

void handleNextModlessWindow (int k) {
if (nActiveModlessDlgs<=0) return;
HWND h = GetForegroundWindow();
int n = nActiveModlessDlgs;
if (h!=win) {
int i; for (i=0; i < nActiveModlessDlgs; i++) {
if (activeModlessDlgs[i]==h) {
n=i; break;
}}}

n+=k;
if (n<0) n+=nActiveModlessDlgs;
n%=(nActiveModlessDlgs+1);
if (n==nActiveModlessDlgs) SetActiveWindow(win);
else SetActiveWindow(activeModlessDlgs[n]);
}

void handleHomeKey () {
int curline = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
int lineindex = SendMessage(edit, EM_LINEINDEX, curline, 0);
int linelength = SendMessage(edit, EM_LINELENGTH, lineindex, 0);
wchar_t data[linelength+1];
data[0]=linelength;
linelength = SendMessage(edit, EM_GETLINE, curline, data);
data[linelength]=0;
//int i; for(i=0; i<linelength && (data[i]==' ' || data[i]=='\t'); i++) ;
//if (i<linelength) lineindex+=i;
int i=-1; while (++i<linelength && (data[i]==' ' || data[i]=='\t')) ;
SendMessage(edit, EM_SETSEL, lineindex+i, lineindex+i);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
curRow = curCol = -1;
}


void handleEnterKey () {
int curchar = 0;
SendMessage(edit, EM_GETSEL, 0, &curchar);
int curline = SendMessage(edit, EM_LINEFROMCHAR, curchar, 0);
int linelength = SendMessage(edit, EM_LINELENGTH, curchar, 0);
int lineindex = SendMessage(edit, EM_LINEINDEX, curline, 0);
wchar_t data[linelength+1];
data[0]=linelength;
linelength = SendMessage(edit, EM_GETLINE, curline, data);
curPage->shouldAddUndo = TRUE;
SendMessage(edit, WM_CHAR, '\n', 0);
intptr_t  i; for (i=0; i < linelength  && (data[i]==' ' || data[i]=='\t'); i++) SendMessage(edit, WM_CHAR, data[i], 0);
i = ev_onEnter(data, linelength, curline, curchar, lineindex);
if (i>=-100 && i<=100) insertTabs(i);
else if (i==101) {
for (i=0; i < linelength  && (data[i]==' ' || data[i]=='\t'); i++) SendMessage(edit, WM_CHAR, VK_BACK, 0);
SendMessage(edit, WM_CHAR, VK_BACK, 0);
}
else {
SendMessage(edit, EM_REPLACESEL, FALSE, i);
free(i);
}
curRow = curCol = -1;
}

void insertTabs (int n) {
if (n>0) for (; n>0; n--) {
if (curPage->tabSpaces<=0) SendMessage(edit, WM_CHAR, VK_TAB, 0);
else for (int i=0; i<curPage->tabSpaces; i++) SendMessage(edit, WM_CHAR, VK_SPACE, 0);
} 
else for (; n<0; n++) {
if (curPage->tabSpaces<=0) SendMessage(edit, WM_CHAR, VK_BACK, 0);
else for (int i=0; i<curPage->tabSpaces; i++) SendMessage(edit, WM_CHAR, VK_BACK, 0);
}}

void handleTabKey () {
int sStart, sEnd;
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
intptr_t re=1, re2=0;
ev_onTab(sStart, sEnd, &re, &re2);
if (!re) return;
else if (re!=1) {
SendMessage(edit, EM_REPLACESEL, TRUE, re);
if (re2) SendMessage(edit, EM_SETSEL, sStart, sStart+wcslen(re));
free(re);
return;
}
if (sEnd-sStart<=0)  insertTabs(curKeyFlags&1? -1 : 1);
else {
//##
if (curKeyFlags&1) doReplaceSel(sStart, sEnd, sStart, sEnd, curPage->tabmlr, L"", FALSE, TRUE);
else  doReplaceSel(sStart, sEnd, sStart, sEnd, L"^(?<=\\n|\\A)", curPage->tabmlr+1, FALSE, TRUE);
}
}


void open (const wchar_t* fn, int flags, int filterIndex) {
if (flags&OF_NEWINSTANCE) {
wchar_t cmd[512];
GetModuleFileName(NULL, cmd, 512);
ShellExecute(win, L"open", cmd, fn, NULL, SW_SHOW);
return;
}
if (flags&OF_REUSEOPENEDTABS) {
COPYDATASTRUCT cp;
cp.dwData = 76;
cp.cbData = sizeof(wchar_t) * (wcslen(fn)+1);
cp.lpData = fn;
HWND hWin = NULL;
while (hWin=FindWindowEx(NULL, hWin, CLASSNAME, NULL)) {
if (SendMessage(hWin, WM_COPYDATA, win, &cp)) {
SetForegroundWindow(hWin);
if (flags&OF_EXITONDOUBLEOPEN) exit(0);
return;
}}}
if (flags&OF_NEWTAB) addEmptyTab();
if (!curPage) return;

int encoding = curPage->curEncoding;
BOOL rdo = FALSE;
encoding = (flags&OF_FORCEENCODING? curPage->curEncoding : CP_ACP);
int len = 0;
const char* buf = readFile(fn, &len, &rdo);
if (!buf) {
if (!(flags&OF_QUIET)) MessageBox(win, MSG_READ_ERROR, MSG_ERROR, MB_OK | MB_ICONERROR);
return;
}
if (!(flags&OF_FORCEENCODING)) encoding = guessEncoding(buf);
if (encoding>=1200&&encoding<=1203) len/=2;
wchar_t* data = strncvt(buf, len, encoding, CP_UTF16, &len);
if (abs( ((char*)data) - ((char*)buf) )>4) free(buf);
if (!data) {
if (!(flags&OF_QUIET)) MessageBox(win, MSG_READ_ERROR, MSG_ERROR, MB_OK | MB_ICONERROR);
return;
}
data = normalizeLineEnding(data, len, 0, &curPage->curLineEnding, &len);
data[len]=0;
curPage->tabSpaces = guessIndentationMode(data, len, defaultTabSpaces);
curPage->curEncoding = encoding;
curPage->curText = data;
curPage->curFilenameFilterIndex = filterIndex;
updatePageName(curPage, fn);
if (flags&OF_NEWTAB) ev_onTabNew(curPage);
ev_onBeforeOpen(curPage);

int sStart, sEnd;
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
SetWindowText(edit, curPage->curText);
SendMessage(edit, EM_SETMODIFY, FALSE, 0);
SendMessage(edit, EM_SETSEL, flags&OF_KEEPSELECTION?sStart:0, flags&OF_KEEPSELECTION?sEnd:0);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
setEncoding(curPage->curEncoding);
setLineEnding(curPage->curLineEnding);
setTabSpaces(curPage->tabSpaces);
if ((curPage->curFile==0&&fn==0) || (curPage->curFile!=0 && fn!=0 && wcscmp(fn,curPage->curFile)==0)) { 
clearUndoBuffer(0); 
curPage->shouldAddUndo = TRUE;
curPage->secondCursor=FALSE; 
}
updateWindowTitle();
updateRecentFilesMenu(fn);
updateStatusBar();
curPage->lastSaveTime = time(NULL);
curPage->readOnly = rdo || (flags&OF_READONLY);
if (curPage->curText) free(curPage->curText);
curPage->curText=NULL;
ev_onAfterOpen(curPage);
}

void save4 (const wchar_t* fn, int filterIndex) {
curPage->curFilenameFilterIndex = filterIndex;
updatePageName(curPage, fn);
save(curPage);
}

void save (pagecontext* p) {
if (!p || !p->curFile) return;
if (!ev_onBeforeSave(p)) return;

wchar_t* buf = p->curText;
int len = 0;

if (!buf) {
len = GetWindowTextLength(edit);
buf = malloc(sizeof(wchar_t) * (len+8));
GetWindowText(edit, buf, len+1);
buf[len]=0;
}
buf = normalizeLineEnding(buf, len, p->curLineEnding, NULL, &len);
char* cbuf = strcvt(buf, CP_UTF16, p->curEncoding, &len);
if (p->curEncoding>=1200&&p->curEncoding<=1203) len*=2;
if (!cbuf) {
MessageBox(win, MSG_WRITE_ERROR, MSG_ERROR, MB_ICONERROR | MB_OK);
return;
}
writeFile(p->curFile, cbuf, len);
if ( ((char*)cbuf) != ((char*)buf) ) free(cbuf);
p->lastSaveTime = time(NULL);
p->readOnly = FALSE;
updateRecentFilesMenu(p->curFile);
if (p==curPage) {
updateWindowTitle();
SendMessage(edit, EM_SETMODIFY, FALSE, 0);
}
if (wcscmp(p->curFile,confFn)==0) {
readConfig(confFn);
recreateEditControl();
int i; for (i=0; i < 5; i++) CheckMenuItem(hFormatMenu, IDM_RELOAD +i, ((curFlags&(1<<i))? MF_CHECKED : MF_UNCHECKED));
updateRecentFilesMenu(NULL);
}
ev_onAfterSave(p);
}

void updatePageName (pagecontext* curPage, const wchar_t* fn) {
if (!curPage) return;
if (fn && curPage->curFile!=fn) {
if (curPage->curFile) free(curPage->curFile);
curPage->curFile = wcsdup(fn);
const wchar_t *x1 =wcsrchr(curPage->curFile, L'\\'), *x2 =wcsrchr(curPage->curFile, L'/'), *x=0;
if (x1 && x2 && *x1&&*x2) x = x1>x2?x1:x2;
else if (x1 && *x1) x = x1;
else if (x2 && *x2) x = x2;
if (!x || !*x) x = curPage->curFile;
else if (x!=curPage->curFile) x++;
if (curPage->curName) free(curPage->curName);
curPage->curName = wcsdup(x);
}
else if (!fn) {
if (curPage->id<=0) curPage->id = untitledIndex;
curPage->curFile = NULL;
if (curPage->curName) free(curPage->curName);
curPage->curName = malloc(16*sizeof(wchar_t));
wsprintf(curPage->curName, MSG_UNTITLED, curPage->id);
}}

int indexOfPage (pagecontext* p) {
for (int i=0; i<nPages; i++) if (pages[i]==p) return i;
return -1;
}

void updateTabName (int pos) {
if (pos<0 || pos>=nPages) return;
TCITEM it;
it.mask = TCIF_TEXT;
it.pszText = pages[pos]->curName;
SendMessage(tabctl, TCM_SETITEM, pos, &it);
}

void updateWindowTitle (void) {
wchar_t wbuf[512];
wsprintf(wbuf, MSG_WINDOWTITLE, curPage->curName, EXENAME);
SetWindowText(win, wbuf);
TCITEM it;
it.mask = TCIF_TEXT;
it.pszText = curPage->curName;
int pos = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
SendMessage(tabctl, TCM_SETITEM, pos, &it);
}

void openDialog (int flags) {
wchar_t path[512];
OPENFILENAME ofn;
ZeroMemory(&path, 512*sizeof(wchar_t));
ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = win;
ofn.lpstrFile = path;
ofn.nMaxFile = 511;
if (curPage->curFilenameFilter) ofn.lpstrFilter = curPage->curFilenameFilter;
else if (globalFilenameFilter) ofn.lpstrFilter = globalFilenameFilter;
else ofn.lpstrFilter = MSG_FILENAMEFILTER;
ofn.nFilterIndex = curPage->curFilenameFilterIndex;
ofn.lpstrTitle = MSG_OPENTITLE;
ofn.Flags =                        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
if (GetOpenFileName(&ofn)==TRUE) {
if (ofn.Flags&OFN_READONLY) flags|=OF_READONLY;
open(ofn.lpstrFile, flags, ofn.nFilterIndex);
}
SetFocus(edit);
}

void saveDialog () {
wchar_t path[512];
OPENFILENAME ofn;
ZeroMemory(&path, 512*sizeof(wchar_t));
if (curPage->curFile) memcpy(path, curPage->curFile, (1+wcslen(curPage->curFile))*sizeof(wchar_t));
ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = win;
ofn.lpstrFile = path;
ofn.nMaxFile = 511;
if (curPage->curFilenameFilter) ofn.lpstrFilter = curPage->curFilenameFilter;
else if (globalFilenameFilter) ofn.lpstrFilter = globalFilenameFilter;
else ofn.lpstrFilter = MSG_FILENAMEFILTER;
ofn.nFilterIndex = curPage->curFilenameFilterIndex;
ofn.lpstrTitle = MSG_SAVETITLE;
ofn.Flags =                        OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_NOREADONLYRETURN;
if (GetSaveFileName(&ofn)==TRUE) save4(ofn.lpstrFile, ofn.nFilterIndex);
SetFocus(edit);
}

wchar_t* showFileDialog (const wchar_t* file, const wchar_t* title, const wchar_t* filters, int* nFilter, int flags) {
int pathlen = ((flags&3)==3? 16384 : 512);
wchar_t path[pathlen]; // = malloc(sizeof(wchar_t) * pathlen);
OPENFILENAME ofn;
ZeroMemory(&path, pathlen*sizeof(wchar_t));
if (file) memcpy(path, file, sizeof(wchar_t) * (wcslen(file)+1));
ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = win;
ofn.lpstrFile = path;
ofn.nMaxFile = pathlen -1;
ofn.lpstrFilter = filters;
ofn.nFilterIndex = filters? *nFilter : 0;
ofn.lpstrTitle = title;
if (flags&1) ofn.Flags =                        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | ((flags&3)==3? OFN_ALLOWMULTISELECT : 0);
else ofn.Flags =                        OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_NOREADONLYRETURN;
if (
(flags&1 && GetOpenFileName(&ofn))
|| (!(flags&1) && GetSaveFileName(&ofn)) 
) {
*nFilter = ofn.nFilterIndex;
return path;
} else {
free(path);
return 0;
}}

void setEncoding (int enc) {
curPage->curEncoding = enc;
static HMENU h = 0;
if (!h) h=GetSubMenu(GetSubMenu(GetMenu(win), 2), 0);
int i=0; for(i=0; i<nEncs && enc!=encs[i]; i++) ;
CheckMenuRadioItem(h, 0, nEncs, i, MF_BYPOSITION);
}

void setLineEnding (int le) {
curPage->curLineEnding = le;
static HMENU h = 0;
if (!h) h=GetSubMenu(GetSubMenu(GetMenu(win), 2), 1);
CheckMenuRadioItem(h, 0, 2, le, MF_BYPOSITION);
}

void fillIndentString (wchar_t* tabmlr, int tabmlrlen, int tabSpaces) {
tabmlr[0]='^';
if (tabSpaces>0) {
for (int i=0; i<tabSpaces; i++) tabmlr[i+1]=32;
tabmlr[tabSpaces+1]=0;
}
else {
tabmlr[1]='\t';
tabmlr[2]=0;
}}

void setTabSpaces (int n) {
if (n>8) n=8; else if (n<0) n=0;
curPage->tabSpaces = n;
int x = curPage->tabSpaces==0? 16 : ABS(curPage->tabSpaces)*4;
SendMessage(edit, EM_SETTABSTOPS, 1, &x);
fillIndentString(curPage->tabmlr, 10, curPage->tabSpaces);
static HMENU h = 0;
if (!h) h=GetSubMenu(GetSubMenu(GetMenu(win), 2), 2);
if (curPage->tabSpaces>0) {
wchar_t buf[32];
wsprintf(buf, MSG_INDENT_SPACES, curPage->tabSpaces);
ModifyMenu(h, IDM_INDENT_SPACES, MF_STRING, IDM_INDENT_SPACES, buf);
}
else ModifyMenu(h, IDM_INDENT_SPACES, MF_STRING, IDM_INDENT_SPACES, MSG_INDENT_SPACES_DEF);
CheckMenuRadioItem(h, 0, 1, curPage->tabSpaces>0? 1:0, MF_BYPOSITION);
}

void addAccelerator (int flags, int key, int cmd) {
int n = CopyAcceleratorTable(hAccel, NULL, 0);
ACCEL* accels = malloc(sizeof(ACCEL) * (n+1));
CopyAcceleratorTable(hAccel, accels, n);
accels[n].fVirt = flags;
accels[n].key = key;
accels[n].cmd = cmd;
HACCEL hNew = CreateAcceleratorTable(accels, n+1);
if (hNew) {
DestroyAcceleratorTable(hAccel);
hAccel = hNew;
}
free(accels);
}

BOOL removeAccelerator (int cmd) {
int n = CopyAcceleratorTable(hAccel, NULL, 0);
ACCEL* accels = malloc(sizeof(ACCEL) * (n+1));
CopyAcceleratorTable(hAccel, accels, n);
BOOL found = FALSE;
int i; for(i=0; i<n; i++) {
if (accels[i].cmd==cmd) {
accels[i] = accels[n -1];
found = TRUE;
break;
}}
if (found) {
HACCEL hNew = CreateAcceleratorTable(accels, n -1);
if (hNew) {
DestroyAcceleratorTable(hAccel);
hAccel = hNew;
}}
free(accels);
return found;
}

BOOL findAccelerator (int cmd, int* flags, int* key) {
int n = CopyAcceleratorTable(hAccel, NULL, 0);
ACCEL* accels = malloc(sizeof(ACCEL) * (n+1));
CopyAcceleratorTable(hAccel, accels, n);
BOOL found = FALSE; int i; 
for (i=0; i<n; i++) {
if (accels[i].cmd==cmd) {
found = TRUE;
if (flags) *flags = accels[i].fVirt;
if (key) *key = accels[i].key;
break;
}}
free(accels);
return found;
}


int addCustomCommand (void* p) {
int i; for (i=0; i < nCustomCommands; i++) {
if (!customCommands[i]) {
customCommands[i] = p;
return i;
}}
int k = nCustomCommands * 3/2 +1;
void** z = malloc(sizeof(void*) * k);
memset(z, 0, sizeof(void*) * k);
memcpy(z, customCommands, sizeof(void*) * nCustomCommands);
customCommands = z;
nCustomCommands = k;
return addCustomCommand(p);
}

void removeCustomCommand (void* p) {
int i; for (i=0; i < nCustomCommands; i++) {
if (p==customCommands[i]) {
customCommands[i]=0;
break;
}}}

void** getCustomCommand (int i) {
return &(customCommands[i]);
}

int findCustomCommand (void* p) {
int i; for (i=0; i < nCustomCommands; i++) {
if (customCommands[i]==p) return i;
}
return -1;
}

void updateStatusBar () {
int sStart=0, sEnd=0, curlen = GetWindowTextLength(edit);
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
curRow = SendMessage(edit, EM_LINEFROMCHAR, sEnd, 0);
curCol = sEnd - SendMessage(edit, EM_LINEINDEX, curRow, 0);
int prc = curlen<=0?0:(1+200LL*sEnd/curlen)/2LL;
wchar_t buf[72];
if (sStart==sEnd) wsprintf(buf, MSG_SB1, curRow+1, curCol+1, prc);
else {
int curRow2 = SendMessage(edit, EM_LINEFROMCHAR, sStart, 0);
int curCol2 = sStart - SendMessage(edit, EM_LINEINDEX, curRow2, 0);
wsprintf(buf, MSG_SB2, curRow2+1, curCol2+1, curRow+1, curCol+1, prc);
}
SetWindowText(status, buf);
ev_onStatusBarChange(curPage);
}

INT_PTR choiceDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
static HWND modalDlg = 0;
switch (msg) {
case WM_INITDIALOG : {
HWND hList = GetDlgItem(hwnd, 1002);
void** p = lp;
//##
SetDlgItemText(hwnd, 1001, *(p++));
const wchar_t* items = *(p++);
SetWindowText(hwnd, *(p++));
int flags = *(p++);
if (flags&1) modalDlg = hwnd;
else {
HWND hOk = GetDlgItem(hwnd, IDOK);
ShowWindow(hOk, SW_HIDE);
SetDlgItemText(hwnd, IDCANCEL, MSG_CLOSE);
}
while (*items) {
SendMessage(hList, LB_ADDSTRING, 0, items);
items += wcslen(items) +1;
}
}return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
HWND hList = GetDlgItem(hwnd, 1002);
int cursel, tlen; wchar_t* txt = 0;
cursel = SendMessage(hList, LB_GETCURSEL, 0, 0);
if (cursel!=LB_ERR) {
tlen = SendMessage(hList, LB_GETTEXTLEN, cursel, 0);
txt = malloc(sizeof(wchar_t) * (tlen+2));
txt[0] = cursel;
SendMessage(hList, LB_GETTEXT, cursel, txt+1);
txt[tlen+1]=0;
if (modalDlg==hwnd) {
modalDlg=0;
EndDialog(hwnd, txt);
return TRUE;
}
else {
BOOL re = handleListClick(GetDlgItem(hwnd,1002), txt[0], txt+1);
if (re) SetFocus(edit);
else SetFocus(hList);
free(txt);
return TRUE;
}}
}return TRUE;
case IDCANCEL : 
if (hwnd==modalDlg) {
modalDlg = 0;
EndDialog(hwnd,1); 
return TRUE;
}
else {
if (!ev_onCloseModlessWindow(GetDlgItem(hwnd,1002),0)) return FALSE;
int i; for (i=0; i<nActiveModlessDlgs; i++) {
if (activeModlessDlgs[i]==hwnd) {
activeModlessDlgs[i] = activeModlessDlgs[nActiveModlessDlgs -1];
}}
nActiveModlessDlgs--;
DestroyWindow(hwnd);
SetFocus(edit);
}
return TRUE;
}break;
case WM_CONTEXTMENU: 
ev_onContextMenuInModlessWindow(GetDlgItem(hwnd,1002),0);
break;
}
return FALSE;
}

INT_PTR outputDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_INITDIALOG : {
const wchar_t** x = lp;
SetDlgItemText(hwnd, 1001, *(x++));
SetDlgItemText(hwnd, 1002, *(x++));
SetWindowText(hwnd, *x);
HWND hEdit = GetDlgItem(hwnd,1002);
SendMessage(hEdit, EM_SETLIMITTEXT, 67108864, 0);
SendMessage(hEdit, WM_SETFONT, font, TRUE);
int xx = curPage->tabSpaces==0? 16 : ABS(curPage->tabSpaces)*4;
SendMessage(hEdit, EM_SETTABSTOPS, 1, &xx);
}return TRUE;
case WM_COMMAND :
if (LOWORD(wp)==IDCANCEL) {
if (!ev_onCloseModlessWindow(GetDlgItem(hwnd,1002),1)) return FALSE;
int i; for (i=0; i<nActiveModlessDlgs; i++) {
if (activeModlessDlgs[i]==hwnd) {
activeModlessDlgs[i] = activeModlessDlgs[nActiveModlessDlgs -1];
}}
nActiveModlessDlgs--;
DestroyWindow(hwnd);
SetFocus(edit);
}
break;
}
return FALSE;
}

HWND showOutputWindow (const wchar_t* prompt, const wchar_t* text, const wchar_t* title) {
HWND hDlg = 0;
void f () {
hDlg = CreateDialogParam(hinst, IDD_OUTPUT, win, outputDlgProc, &prompt);
ShowWindow(hDlg, SW_SHOW);
}
SendMessage(win, WM_USER, 1, f);
activeModlessDlgs[nActiveModlessDlgs++] = hDlg;
HWND hTxt = GetDlgItem(hDlg,1002);
return hTxt;
}

INT_PTR consoleDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_INITDIALOG : {
HWND hEdit = GetDlgItem(hwnd,1001);
SendMessage(hEdit, EM_SETLIMITTEXT, 67108864, 0);
SendMessage(hEdit, WM_SETFONT, font, TRUE);
int xx = curPage->tabSpaces==0? 16 : ABS(curPage->tabSpaces)*4;
SendMessage(hEdit, EM_SETTABSTOPS, 1, &xx);
}return TRUE;
case WM_COMMAND :
switch(LOWORD(wp)) {
case 1003 : {
HWND hEval = GetDlgItem(hwnd,1002);
int len = GetWindowTextLength(hEval);
const wchar_t* evalw[len+1];
GetWindowText(hEval,evalw,len+1);
evalw[len]=0;
const char* eval = strcvt(evalw, CP_UTF8, CP_UTF16, NULL);
luaEvalString(eval);
free(eval);
SendMessage(hEval, EM_SETSEL, 0, -1);
SetFocus(hEval);
}break;
case 1004 :
SetDlgItemText(hwnd, 1001, L"");
break;
case 2000 : {
HWND hEdit = GetDlgItem(hwnd,1001);
int ss, se, k=GetWindowTextLength(hEdit);
SendMessage(hEdit, EM_GETSEL, &ss, &se);
SendMessage(hEdit, EM_SETSEL, k, k);
SendMessage(hEdit, EM_REPLACESEL, TRUE, lp);
if (GetFocus()==hEdit) SendMessage(hEdit, EM_SETSEL, ss, se);
}break;
case IDCANCEL : {
console = NULL;
int i; for (i=0; i<nActiveModlessDlgs; i++) {
if (activeModlessDlgs[i]==hwnd) {
activeModlessDlgs[i] = activeModlessDlgs[nActiveModlessDlgs -1];
}}
nActiveModlessDlgs--;
DestroyWindow(hwnd);
SetFocus(edit);
}break;}
if (HIWORD(wp)==EN_SETFOCUS && LOWORD(wp)==1002) SendDlgItemMessage(hwnd,1002,EM_SETSEL,0,-1);
break;
}
return FALSE;
}

HWND showConsoleWindow () {
HWND hDlg = 0;
void f () {
hDlg = CreateDialogParam(hinst, IDD_CONSOLE, win, consoleDlgProc, NULL);
ShowWindow(hDlg, SW_SHOW);
}
SendMessage(win, WM_USER, 1, f);
activeModlessDlgs[nActiveModlessDlgs++] = hDlg;
return (console = hDlg);
}

void* showListDialog (const wchar_t* prompt, const wchar_t* itemlist, const wchar_t* title, int flags)  {
if (flags&1) { // Modal version
void* z[] = { prompt, itemlist, title, flags, };
int re = DialogBoxParam(hinst, IDD_CHOICE, win, choiceDlgProc, z);
if (re>=-1 && re<=1) return NULL;
else return re;
}
else { // Modless version
HWND h = 0;
void f () {
void* z[] = { prompt, itemlist, title, flags, };
h = CreateDialogParam(hinst, IDD_CHOICE, win, choiceDlgProc, z);
ShowWindow(h, SW_SHOW);
}
SendMessage(win, WM_USER, 1, f);
activeModlessDlgs[nActiveModlessDlgs++] = h;
return GetDlgItem(h,1002);
}}


INT_PTR inputDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_INITDIALOG : {
const wchar_t* s = lp;
BOOL ml = !!*(s++);
HWND hTxt = GetDlgItem(hwnd,1002);
if (!ml) SendMessage(hTxt, CB_RESETCONTENT, 0, 0);
SetDlgItemText(hwnd, 1001, s);
s += 1+wcslen(s);
SetDlgItemText(hwnd, 1002, s);
s += 1+wcslen(s);
SetWindowText(hwnd, s);
s += 1+wcslen(s);
if (!ml) while (*s) {
SendMessage(hTxt, CB_ADDSTRING, 0, s);
s += 1+wcslen(s);
}
}return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
HWND hTxt = GetDlgItem(hwnd, 1002);
int txtlen = GetWindowTextLength(hTxt);
wchar_t* wstr = malloc(sizeof(wchar_t) * (txtlen+1));
GetWindowText(hTxt, wstr, txtlen+1);
wstr[txtlen]=0;
	EndDialog(hwnd, wstr);
}return TRUE;
case IDCANCEL : EndDialog(hwnd, 1); return TRUE;
}break;
}
return FALSE;
}

INT_PTR goToLineDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_INITDIALOG : {
int num = SendMessage(edit, EM_GETLINECOUNT, 0, 0);
char tmp[100];
wsprintf(tmp, MSG_GOTOLINETEXT, num);
SetDlgItemText(hwnd, 1001, tmp);
num = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
SetDlgItemInt(hwnd, 1002, num+1, FALSE);
}return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case IDOK : {
wchar_t tmp[50];
GetDlgItemText(hwnd, 1002, tmp, 49);
int num = wcstol(tmp, NULL, 10);
if (tmp[0]=='+' || tmp[0]=='-') num += SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
else --num;
int max = SendMessage(edit, EM_GETLINECOUNT, 0, 0);
if (num<0 || num>=max) { MessageBox(hwnd, MSG_GOTOLINE_RANGE_ERROR, MSG_ERROR, MB_ICONEXCLAMATION  | MB_OK); return TRUE; }
SendMessage(win, WM_USER, IDM_GOTOLINE, num);
}
case IDCANCEL : EndDialog(hwnd, wp); return TRUE;
}}
return FALSE;
}

INT_PTR searchReplaceDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_INITDIALOG :
SetWindowText(hwnd, lp? MSG_REPL_DLGTITLE2 : MSG_REPL_DLGTITLE1);
SetDlgItemText(hwnd, IDOK, lp? MSG_REPL_BTNOK2 : MSG_REPL_BTNOK1);
EnableWindow(GetDlgItem(hwnd, 1002), lp);
EnableWindow(GetDlgItem(hwnd, 1005), !lp);
EnableWindow(GetDlgItem(hwnd, 1006), !lp);
EnableWindow(GetDlgItem(hwnd, 1007), searchRegex);
SetDlgItemText(hwnd, 1001, curSearch);
SetDlgItemText(hwnd, 1002, curReplace);
SendMessage(GetDlgItem(hwnd, 1003), BM_SETCHECK, (searchFlags&1)?BST_CHECKED:BST_UNCHECKED, 0);
SendMessage(GetDlgItem(hwnd, 1007), BM_SETCHECK, (searchFlags&2)?BST_CHECKED:BST_UNCHECKED, 0);
SendMessage(GetDlgItem(hwnd, 1004), BM_SETCHECK, searchRegex?BST_CHECKED:BST_UNCHECKED, 0);
SendMessage(GetDlgItem(hwnd, searchUpward?1005:1006), BM_SETCHECK, BST_CHECKED, 0);
if (searchList) {
HWND hCb = GetDlgItem(hwnd,1001);
int i,n; for (i=0, n=l_len(searchList); i<n; i++) SendMessage(hCb, CB_ADDSTRING, 0, l_item(searchList,i));
}
if (replaceList) {
HWND hCb = GetDlgItem(hwnd,1002);
int i,n; for (i=0, n=l_len(replaceList); i<n; i++) SendMessage(hCb, CB_ADDSTRING, 0, l_item(replaceList,i));
}
return TRUE;
case WM_COMMAND :
switch (LOWORD(wp)) {
case 1004 : 
EnableWindow(GetDlgItem(hwnd,1007), SendDlgItemMessage(hwnd, 1004, BM_GETCHECK, 0, 0) == BST_CHECKED);
break;
case IDOK : {
HWND hSearch = GetDlgItem(hwnd, 1001), hRepl = GetDlgItem(hwnd, 1002), hCbCase = GetDlgItem(hwnd, 1003), hCbRegex = GetDlgItem(hwnd, 1004), hRbDirUp = GetDlgItem(hwnd, 1005), hCbUtf8 = GetDlgItem(hwnd, 1007);
BOOL sr = IsWindowEnabled(hRepl);
BOOL searchCase = SendMessage(hCbCase, BM_GETCHECK, 0, 0) == BST_CHECKED;
BOOL searchUtf8 = SendMessage(hCbUtf8, BM_GETCHECK, 0, 0) == BST_CHECKED;
searchRegex = SendMessage(hCbRegex, BM_GETCHECK, 0, 0) == BST_CHECKED;
searchUpward = SendMessage(hRbDirUp, BM_GETCHECK, 0, 0) == BST_CHECKED;
searchFlags = (searchCase?1:0) | (searchUtf8?2:0);
int sLen = GetWindowTextLength(hSearch);
if (curSearch) free(curSearch);
curSearch = malloc(sizeof(wchar_t) * (sLen+1));
GetWindowText(hSearch, curSearch, sLen+1);
curSearch[sLen]=0;
if (searchRegex) {
char* z1 = strcvt(curSearch, CP_UTF16, CP_UTF8, NULL);
pcre* r1 = preg_compile(z1,0);
free(z1);
if (r1) pcre_free(r1);
else {
MessageBox(hwnd, MSG_REPL_REGEXERROR, MSG_ERROR, MB_ICONEXCLAMATION |  MB_OK);
SetFocus(hSearch);
return FALSE;
}}
int i = -1;
if (searchList) for (i=0; i<l_len(searchList); i++) if (0==wcscmp(curSearch, l_item(searchList,i))) break;
if (i<0 || i==l_len(searchList)) l_add(searchList, wcsdup(curSearch), 0);
if (sr) {
sLen = GetWindowTextLength(hRepl);
if (curReplace) free(curReplace);
curReplace = malloc(sizeof(wchar_t) * (sLen+1));
GetWindowText(hRepl, curReplace, sLen+1);
curReplace[sLen]=0;
int i = -1;
if (replaceList) for (i=0; i<l_len(replaceList); i++) if (0==wcscmp(curReplace, l_item(replaceList,i))) break;
if (i<0 || i==l_len(replaceList)) l_add(replaceList, wcsdup(curReplace), 0);
}
if (sr) SendMessage(win, WM_USER, IDM_REPLACE, 0);
else if (searchUpward) SendMessage(win, WM_COMMAND, IDM_FINDPREV, 0);
else SendMessage(win, WM_COMMAND, IDM_FINDNEXT, 0);
}
case IDCANCEL : EndDialog(hwnd, wp); return TRUE;
}}
return FALSE;
}

INT_PTR progressDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_INITDIALOG : {
SendDlgItemMessage(hwnd, 1002, PBM_SETRANGE, 0, 10000<<16);
SetWindowLong(hwnd, GWL_USERDATA, FALSE);
void** hptr = lp;
if (hptr) *hptr = hwnd;
}break;
case WM_COMMAND : switch(LOWORD(wp)) {
case IDCANCEL :
SetWindowLong(hwnd, GWL_USERDATA, TRUE);
break;
}break;
case WM_USER : switch (wp) {
case 1001 : SetDlgItemText(hwnd, 1001, lp); break;
case 1002 : SendDlgItemMessage(hwnd, 1002, PBM_SETPOS, lp, 0); break;
case 1003 : {
HWND hLbl = GetDlgItem(hwnd,1001);
int len = GetWindowTextLength(hLbl);
if (lp) {
const wchar_t* w = lp;
GetWindowText(hLbl, w, len+1);
}
return len;
}
case 1004 : return SendDlgItemMessage(hwnd, 1002, PBM_GETPOS, 0, 0);
case 1005 : return !GetWindowLong(hwnd, GWL_USERDATA);
case 1006 : EndDialog(hwnd,lp); break;
}break;
}
return FALSE;
}

void showProgressDialog (HWND* hwndptr) {
HWND oldfocus = 0;
runInEDT({ oldfocus = GetFocus(); });
DialogBoxParam(hinst, IDD_PROGRESS, win, progressDlgProc, hwndptr);
runInEDT({ SetFocus(oldfocus); });
}

int lblcnt=0;
LRESULT CALLBACK lblproc (HWND hwnd, UINT msg, WPARAM wp,LPARAM lp) {
if (msg==WM_GETTEXT) { 
if (++lblcnt>1) return FALSE;
}
return CallWindowProc(oldlblproc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK tabproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch(msg){
case WM_MBUTTONDOWN:
case WM_RBUTTONDOWN: 
{
TCHITTESTINFO t;
t.pt.x = LOWORD(lp);
t.pt.y = HIWORD(lp);
t.flags=0;
int n = SendMessage(hwnd, TCM_HITTEST, 0, &t);
if (t.flags==TCHT_NOWHERE) return FALSE;
else SendMessage(hwnd, TCM_SETCURFOCUS, n, 0);
return TRUE;
}break;
case WM_MBUTTONUP :
deleteCurrentTab();
break;
case WM_CONTEXTMENU : {
POINT p;
GetCursorPos(&p);
TrackPopupMenu(GetSubMenu(GetMenu(win), 0), 0, p.x, p.y, 0, win, NULL);
return TRUE;
}break;
default: break;
}
return CallWindowProc(oldtabproc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK editproc (HWND hwnd, UINT msg, WPARAM wp,LPARAM lp) {
switch (msg) {
case WM_KEYDOWN : {
lblcnt=0;
int k = LOWORD(wp);
if (!ev_onKeyDown(curPage, k, curKeyFlags)) return FALSE;
if (k!=VK_SHIFT && k!=VK_CONTROL && k!=VK_MENU) curRow = curCol = -1;
if (k==VK_DELETE) editAboutToChange(1);
}break;
case WM_KEYUP : {
if (curFlags&16) {
if (curRow<0 || curCol<0) {
updateStatusBar();
}}
if ((LOWORD(wp)==VK_UP || LOWORD(wp)==VK_DOWN || LOWORD(wp)==VK_LEFT || LOWORD(wp)==VK_RIGHT)) curPage->shouldAddUndo = TRUE;
if (LOWORD(wp)==VK_UP || LOWORD(wp)==VK_DOWN || LOWORD(wp)==VK_NEXT || LOWORD(wp)==VK_PRIOR || (curKeyFlags==2&&LOWORD(wp)==VK_HOME) || (curKeyFlags==2&&LOWORD(wp)==VK_END)) ev_onLineChange(curPage, LOWORD(wp));
ev_onKeyUp(curPage, LOWORD(wp), curKeyFlags);
}break;
case WM_CHAR :
if (!ev_onKeyPress(curPage, wp, curKeyFlags)) return FALSE;
editAboutToChange(0);
break;
case WM_PASTE :
case WM_CUT : 
editAboutToChange(0);
break;
case WM_CONTEXTMENU : {
if (!ev_onContextMenu(curPage)) return TRUE;
POINT p;
GetCursorPos(&p);
TrackPopupMenu(GetSubMenu(GetMenu(win), 1), 0, p.x, p.y, 0, win, NULL);
return TRUE;
}break;
case WM_MOUSEWHEEL : {
int n = GET_WHEEL_DELTA_WPARAM(wp);
int dir = n>0? SB_LINEUP : SB_LINEDOWN;
for (int zzz=0; zzz<n; zzz+=120) SendMessage(hwnd, EM_SCROLL, dir, 0);
}break;
}//end switch
return CallWindowProc(oldeditproc, hwnd, msg, wp, lp);
}

void editAboutToChange (int delta) {
if (curPage->shouldAddUndo) addUndoState();
if (curPage->secondCursor) {
SendMessage(edit, EM_GETSEL, 0, &curPage->secondCursorDelta);
curPage->secondCursorDelta += delta;
}}

LRESULT CALLBACK winproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
switch (msg) {
case WM_COMMAND :
if (HIWORD(wp)==EN_CHANGE) {
if (curPage->secondCursor && curPage->secondCursorDelta<=curPage->secondCursorPos) {
int e; SendMessage(edit, EM_GETSEL, 0, &e);
curPage->secondCursorPos += (e-curPage->secondCursorDelta);
}}
else switch (LOWORD(wp)) {
case IDM_SAVE : 
if (curPage->curFile && !curPage->readOnly) save(curPage);
else saveDialog(); 
break;
case IDM_OPEN : openDialog(OF_NEWTAB | OF_REUSEOPENEDTABS); break;
case IDM_OPEN_NEW_INSTANCE: openDialog(OF_NEWINSTANCE); break;
case IDM_OPEN_FORCE : openDialog(OF_NEWTAB | OF_REUSEOPENEDTABS | OF_FORCEENCODING); break;
case IDM_SAVE_AS : saveDialog(); break;
case IDM_FINDNEXT : findNext(); break;
case IDM_FINDPREV : findPrev(); break;
case IDM_UNDO : doUndo(); break;
case IDM_REDO : doRedo(); break;
case IDM_SWITCHCURSOR : switchCursor(); break;
case IDM_JOINCURSOR : curPage->secondCursor=FALSE; break;
case IDM_CURSORSELECT : if (curPage->secondCursor) {
int e; SendMessage(edit, EM_GETSEL, 0, &e);
SendMessage(edit, EM_SETSEL, curPage->secondCursorPos, e);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
curPage->secondCursor=FALSE;
updateStatusBar();
}break;
case IDM_RELOAD : if (curPage->curFile 
&& (!SendMessage(edit, EM_GETMODIFY, 0, 0) || IDYES==MessageBox(hwnd, MSG_RELOAD_WARNING, MSG_QUESTION, MB_YESNO | MB_ICONEXCLAMATION)) )
open(curPage->curFile,OF_KEEPSELECTION, curPage->curFilenameFilterIndex); break;
case IDM_RELOAD_FORCEENCODING :  if (curPage->curFile
&& (!SendMessage(edit, EM_GETMODIFY, 0, 0) || IDYES==MessageBox(hwnd, MSG_RELOAD_WARNING, MSG_QUESTION, MB_YESNO | MB_ICONEXCLAMATION)) )
open(curPage->curFile, OF_KEEPSELECTION | OF_FORCEENCODING, curPage->curFilenameFilterIndex); break;
case IDM_GOTOLINE : DialogBox(hinst, IDD_GOTOLINE, win, goToLineDlgProc); break;
case IDM_FIND : DialogBoxParam(hinst, IDD_SEARCHREPLACE, win, searchReplaceDlgProc, 0); break;
case IDM_REPLACE : DialogBoxParam(hinst, IDD_SEARCHREPLACE, win, searchReplaceDlgProc, 1); break;
case IDM_SELECTALL : 
SendMessage(edit, EM_SETSEL, 0, -1); 
SendMessage(edit, EM_SCROLLCARET, 0, 0);
updateStatusBar();
SetFocus(edit); 
break;
case IDM_COPY : {
int start, end;
SendMessage(edit, EM_GETSEL, &start, &end);
if (start==end) {
int ln = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
int li = SendMessage(edit, EM_LINEINDEX, -1, 0);
int ll = SendMessage(edit, EM_LINELENGTH, li, 0);
SendMessage(edit, EM_SETSEL, li, li+ll);
}
SendMessage(edit, WM_COPY, 0, 0); 
SetFocus(edit); 
}break;
case IDM_CUT : {
int start, end;
SendMessage(edit, EM_GETSEL, &start, &end);
if (start==end) {
int lc = SendMessage(edit, EM_GETLINECOUNT, 0, 0);
int ln = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
int li = SendMessage(edit, EM_LINEINDEX, -1, 0);
int ll = SendMessage(edit, EM_LINELENGTH, li, 0);
if (ln<lc -1) ll+=2;
SendMessage(edit, EM_SETSEL, li, li+ll);
}
SendMessage(edit, WM_CUT, 0, 0); 
SetFocus(edit); 
}break;
case IDM_PASTE : 
SendMessage(edit, WM_PASTE, 0, 0); 
SetFocus(edit); 
break;
case IDM_EXIT : SendMessage(hwnd, WM_CLOSE, 0, 0); break;
case IDM_CLOSE : return deleteCurrentTab(); break;
case IDM_LE_DOS : setLineEnding(0); break;
case IDM_LE_UNIX : setLineEnding(1); break;
case IDM_LE_MAC : setLineEnding(2); break;
case IDM_AUTORELOAD :
curFlags^=2;
CheckMenuItem(hFormatMenu, IDM_AUTORELOAD, MF_BYCOMMAND | (curFlags&2? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_SMARTHOME :
curFlags^=4;
CheckMenuItem(hFormatMenu, IDM_SMARTHOME, MF_BYCOMMAND | (curFlags&4? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_KEEPINDENT :
curFlags^=8;
CheckMenuItem(hFormatMenu, IDM_KEEPINDENT, MF_BYCOMMAND | (curFlags&8? MF_CHECKED : MF_UNCHECKED));
break;
case IDM_SHOWSTATUS : 
curFlags^=16;
CheckMenuItem(hFormatMenu, IDM_SHOWSTATUS, MF_BYCOMMAND | (curFlags&16? MF_CHECKED : MF_UNCHECKED));
ShowWindow(status, curFlags&16? SW_SHOW : SW_HIDE);
break;
case IDM_INDENT_TAB : setTabSpaces(0); break;
case IDM_INDENT_SPACES : setTabSpaces(defaultTabSpaces>0? defaultTabSpaces : 4); break;
case IDM_NEW :
untitledIndex++;
addEmptyTab();
clearUndoBuffer(0);
curPage->shouldAddUndo = TRUE;
curPage->secondCursor=FALSE;
SetWindowText(edit, NULL);
updatePageName(curPage, NULL);
updateWindowTitle();
updateStatusBar();
ev_onTabNew(curPage);
//SetFocus(tabctl);
//Sleep(100);
SetFocus(edit);
break;
case IDM_CONSOLE :
if (console) SetActiveWindow(console);
else showConsoleWindow();
break;
case IDM_ABOUT :
MessageBox(win, MSG_ABOUTMSG, MSG_ABOUTTITLE, MB_OK | MB_ICONINFORMATION);
break;
case IDM_OPEN_RECENT_FILE+0 :
case IDM_OPEN_RECENT_FILE+1 :
case IDM_OPEN_RECENT_FILE+2 :
case IDM_OPEN_RECENT_FILE+3 :
case IDM_OPEN_RECENT_FILE+4 :
case IDM_OPEN_RECENT_FILE+5 :
case IDM_OPEN_RECENT_FILE+6 :
open(recentFiles[LOWORD(wp)-IDM_OPEN_RECENT_FILE].path,  OF_NEWTAB, 0);
break;	
case IDM_REALHOME : {
int curline = SendMessage(edit, EM_LINEFROMCHAR, -1, 0);
int lineindex = SendMessage(edit, EM_LINEINDEX, curline, 0);
SendMessage(edit, EM_SETSEL, lineindex, lineindex);
}break;
case IDM_LINEWRAP : {
curFlags^=1;
recreateEditControl();
CheckMenuItem(hFormatMenu, IDM_LINEWRAP, MF_BYCOMMAND | (curFlags&1? MF_CHECKED : MF_UNCHECKED));
break;
}
case IDM_CRASH : {  char* p = 0; *p = 1; break;  }break;
}
if (LOWORD(wp)>=IDM_ENCODING && LOWORD(wp)<IDM_ENCODING+nEncs) {
int idx = LOWORD(wp) - IDM_ENCODING;
setEncoding(encs[idx]);
}
else if (LOWORD(wp)>=IDM_CUSTOMCOMMAND) {
int cmd = LOWORD(wp) -IDM_CUSTOMCOMMAND;
if (cmd>=0 && cmd<nCustomCommands && customCommands[cmd]) handleCustomCommand(customCommands[cmd]);
}
lastWasCommand = TRUE;
return TRUE;
case WM_USER :
switch (wp) {
case IDT_EDIT :
if (lp==VK_RETURN) handleEnterKey();
else if (lp==VK_HOME) handleHomeKey();
else if (lp==VK_TAB) handleTabKey();
break;
case IDM_GOTOLINE : {
int pos = SendMessage(edit, EM_LINEINDEX, lp, 0);
SendMessage(edit, EM_SETSEL, pos, pos);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
updateStatusBar();
}break;
case IDM_REPLACE : {
int sStart, sEnd, rStart, rEnd;
SendMessage(edit, EM_GETSEL, &sStart, &sEnd);
if (sEnd-sStart>0) {
rStart = sStart;
rEnd = sEnd;
}
else {
rStart=0;
rEnd = GetWindowTextLength(edit);
SendMessage(edit, EM_SETSEL, 0, -1);
SendMessage(edit, EM_SCROLLCARET, 0, 0);
}
doReplaceSel(rStart, rEnd, sStart, sEnd, curSearch, curReplace, searchFlags, searchRegex);
}break;
case 1 : {
void(*f)() = lp; 
f();
}break;
// Other user events
}break;
case WM_NOTIFY : switch (((LPNMHDR)lp)->code) {
case TCN_SELCHANGING :
if (!ev_onBeforeTabChange(curPage)) return TRUE; 
if (curPage) savePage(curPage); 
int i = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
SendMessage(tabctl, TCM_HIGHLIGHTITEM, i, FALSE);
break;
case TCN_SELCHANGE : {
int i = SendMessage(tabctl, TCM_GETCURSEL, 0, 0);
curPage = pages[i];
restorePage(curPage);
ev_onAfterTabChange(curPage);
SendMessage(tabctl, TCM_HIGHLIGHTITEM, i, TRUE);
}break;
// other notifications
}break;
case WM_SETFOCUS :
SetFocus(edit);
return TRUE;
case WM_ACTIVATE :
if (wp>0 && curFlags&2) checkAutoreload();
break;
case WM_CLOSE :
if (!ev_onBeforeClose()) return TRUE;
while (nPages>0 && deleteCurrentTab()) ;
if (nPages<=0) PostQuitMessage(0);
return TRUE;
case WM_COPYDATA : {
COPYDATASTRUCT* cp = lp;
switch (cp->dwData) {
case 76 : 
for (int i=0; i<nPages; i++) {
if (pages[i]->curFile && 0==wcscmp(pages[i]->curFile, cp->lpData)) {
SendMessage(tabctl, TCM_SETCURFOCUS, i, 0);
return TRUE;
}}
if (curFlags&32 && wp!=win) {
open(cp->lpData, OF_NEWTAB, 0);
SetForegroundWindow(win);
return TRUE;
}
return FALSE;
case 79 :
open(cp->lpData, OF_QUIET, 0);
return TRUE;
case 82 : {
union { BOOL b; double d; const char* s; } ptr;
int type = -1;
if (eval(cp->lpData, &type, &ptr)) {
COPYDATASTRUCT cpd;
cpd.dwData=82;
switch(type){
case 1: cpd.cbData=sizeof(BOOL); cpd.lpData=&(ptr.b); break;
case 2: cpd.cbData=sizeof(double); cpd.lpData=&(ptr.d); break;
case 3: cpd.cbData=ptr.s?strlen(ptr.s)+1:0; cpd.lpData=ptr.s; break;
}
if (wp) SendMessage(wp, WM_COPYDATA, win, &cpd);
return type;
}
return FALSE;
}break;
// other WM_COPYDATA
}}break;
case WM_SIZE : {
RECT r; GetClientRect(win, &r);
MoveWindow(tabctl, 5, 5, r.right -10, r.bottom -49, TRUE);
MoveWindow(status, 5, r.bottom -32, r.right -10, 27, TRUE);
r.left = 5; r.top = 5; r.right -= 10; r.bottom -= 49;
SendMessage(tabctl, TCM_ADJUSTRECT, FALSE, &r);
MoveWindow(edit, r.left+3, r.top+3, r.right-r.left -6, r.bottom-r.top -6, TRUE);
}return TRUE;
case WM_RUNPROC : {
void*(*f)(void*) = wp;
if (wp) return f(lp);
}break;
}
return DefWindowProc(hwnd,msg,wp,lp);
}


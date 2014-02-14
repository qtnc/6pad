#ifndef _____GLOBAL_H11
#define _____GLOBAL_H11

#define WM_RUNPROC WM_USER + 1563


#define MAXUNDO 16
#define newThread(x,v) CreateThread( NULL, 0, x, v, 0, NULL)

#define ABS(x) ((x)<0?(-x):(x))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

#define runInEDT(b) \
{ int _____edtfnc (void* unused) b \
SendMessage(win, WM_RUNPROC, _____edtfnc, 0); }

typedef void* luafunc;

typedef struct { 
const wchar_t* str; 
int sStart, sEnd, secondCursor; 
} undostate;

typedef struct {
int id, curSelStart, curSelEnd, curEncoding, curLineEnding, tabSpaces, secondCursorPos, secondCursorDelta, undoLength, undoPos;
int secondCursor, shouldAddUndo, modified, readOnly, editReadOnly, curFilenameFilterIndex, curFilenameFilterLength;
wchar_t *curFile, *curText, *curName, *curFilenameFilter;
wchar_t tabmlr[10];
unsigned int lastSaveTime;  
luafunc onBeforeSave, onAfterSave, onBeforeOpen, onAfterOpen, onBeforeTabClose, onEnter, onTab, onLineChange, onKeyDown, onKeyUp, onKeyPress, onContextMenu, onStatusBarChange;
undostate undoBuffer[MAXUNDO];
} pagecontext;

const void* __declspec(dllexport) strcvt (const void* str, unsigned int cpFrom, unsigned int cpTo, int* len) ;
const void* __declspec(dllexport) strncvt (const void* str, int strLen, unsigned int cpFrom, unsigned int cpTo, int* len) ;

#endif
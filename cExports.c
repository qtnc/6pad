#include "consts.h"

#define EL_WIN 0
#define EL_TAB 1
#define EL_EDIT 2
#define EL_STATUS 3
#define EL_MENUBAR 4

extern HWND win, edit, status, tabctl;

void addAccelerator (int flags, int key, int cmd) ;
BOOL removeAccelerator (int cmd) ;
int addCustomCommand (void* p) ;
void removeCustomCommand (void* p) ;

void __declspec(dllexport) RegisterAccelerator (int flags, int key, int cmd) {
addAccelerator(flags | FVIRTKEY, key, cmd);
}

BOOL __declspec(dllexport)  UnregisterAccelerator (int cmd) {
removeAccelerator(cmd);
}

int __declspec(dllexport) RegisterCommand ( void(*func)(void) ) {
char* p = func;
return addCustomCommand(p+1);
}

void __declspec(dllexport) UnregisterCommand ( void(*func)(void) ) {
char* p = func;
removeCustomCommand(p+1);
}

void* __declspec(dllexport) GetElement (int n) {
switch(n){
case EL_WIN: return win;
case EL_EDIT: return edit;
case EL_TAB: return tabctl;
case EL_STATUS: return status;
case EL_MENUBAR: return GetMenu(win);
default: return 0;
}}

#ifndef _CONSTS_H3
#define _CONSTS_H3
#define UNICODE
#define _WIN32_IE 0x0400
#define _WIN32_WINNT 0x501
#include<windows.h>
#include<commctrl.h>
#include<stdint.h>

#define VERSION_STRING L"1.4.2"

#define CP_UTF16 1200
#define CP_UTF16_LE 1200
#define CP_UTF16_BE 1201
#define CP_UTF16_LE_BOM 1202
#define CP_UTF16_BE_BOM 1203
#define CP_UTF8_BOM 65002

#define OF_FORCEENCODING 0x01
#define OF_KEEPSELECTION 0x02
#define OF_QUIET 0x04
#define OF_REUSEOPENEDTABS 0x08
#define OF_NEWINSTANCE 0x20
#define OF_EXITONDOUBLEOPEN 0x40
#define OF_NEWTAB 0x80
#define OF_READONLY 0x100

#define CREATED_TIME 0
#define LAST_ACCESSED_TIME 1
#define LAST_MODIFIED_TIME 2

#define IDT_EDIT 10001
#define IDT_STATUSBAR 10002
#define IDT_TABBAR 10003

#define IDD_GOTOLINE L"goline"
#define IDD_SEARCHREPLACE L"sr"
#define IDD_INPUT L"prompt"
#define IDD_INPUT2 L"mlPrompt"
#define IDD_OUTPUT L"output"
#define IDD_CHOICE L"choice"
#define IDD_PROGRESS L"progress"
#define IDD_PROPS L"props"
#define IDD_CONSOLE L"console"

#define IDM_NEW 1001
#define IDM_OPEN 1002
#define IDM_OPEN_FORCE 1003
#define IDM_OPEN_NEW_INSTANCE 1008
#define IDM_SAVE 1004
#define IDM_SAVE_AS 1005
#define IDM_CLOSE 1006
#define IDM_EXIT 1007
#define IDM_OPEN_RECENT_FILE 1090
#define IDM_COPY 1101
#define IDM_CUT 1102
#define IDM_PASTE 1103
#define IDM_SELECTALL 1104
#define IDM_GOTOLINE 1105
#define IDM_FIND 1106
#define IDM_REPLACE 1107
#define IDM_FINDNEXT 1108
#define IDM_FINDPREV 1109
#define IDM_UNDO 1110
#define IDM_REDO 1111
#define IDM_SWITCHCURSOR 1112
#define IDM_JOINCURSOR 1113
#define IDM_CURSORSELECT 1114
#define IDM_LE_DOS 1201
#define IDM_LE_UNIX 1202
#define IDM_LE_MAC 1203
#define IDM_ENCODING 1210
#define IDM_RELOAD 1250
#define IDM_RELOAD_FORCEENCODING 1251
#define IDM_AUTORELOAD 1252
#define IDM_LINEWRAP 1253
#define IDM_KEEPINDENT 1254
#define IDM_SMARTHOME 1255
#define IDM_SHOWSTATUS 1256
#define IDM_REALHOME 1297
#define IDM_INDENT_TAB 1298
#define IDM_INDENT_SPACES 1299
#define IDM_ABOUT 1400
#define IDM_CONSOLE 1401
#define IDM_CRASH 9999

#define IDM_CUSTOMCOMMAND 20000

#if defined ENGLISH
#include "english.h"
#elif defined FRENCH
#include "french.h"
#endif

#endif

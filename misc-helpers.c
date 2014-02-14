#include "consts.h"
#include <shlobj.h>

HFONT loadFont (HWND h, const wchar_t* n, int s) {
return CreateFont(MulDiv(s, GetDeviceCaps(GetDC(h), LOGPIXELSY), 72), 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, n);
}

int CALLBACK BrowseForFolderCallback(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) {
	wchar_t szPath[MAX_PATH+1];
	switch(uMsg) 	{
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	case BFFM_SELCHANGED: 
		if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szPath))  		{
			SendMessage(hwnd, BFFM_SETSTATUSTEXT,0,(LPARAM)szPath);	
		} 		break;
case BFFM_VALIDATEFAILED:
MessageBeep(0);
return 1;
	}
	return 0;
}

BOOL BrowseFolders(HWND hwnd, LPWSTR lpszFolder, LPWSTR lpszTitle) {
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	BOOL bResult = FALSE;
	bi.hwndOwner = hwnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = BIF_STATUSTEXT | BIF_USENEWUI| BIF_EDITBOX | BIF_VALIDATE | 0x200;
	bi.lpfn = BrowseForFolderCallback;
	bi.lParam = (LPARAM)lpszFolder;
	pidl = SHBrowseForFolder(&bi);
	if (pidl && SHGetPathFromIDList(pidl,lpszFolder))  					return TRUE;
else 			return FALSE;
}



unsigned long filetimeTo1970 (unsigned long long l) {
static unsigned long long rep = 0;
if (!rep) {
SYSTEMTIME st1 = { 1970, 1, 0, 1, 0, 0, 0, 0 };
FILETIME ft1;
SystemTimeToFileTime(&st1, &ft1);
rep = (((unsigned long long)ft1.dwHighDateTime)<<32) | ft1.dwLowDateTime;
}
return ((l-rep)/10000000LL);
}

unsigned long getFileTime (const wchar_t* fn, int type) {
unsigned long long l = 0;
FILETIME ft;
HANDLE h = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
if (h==INVALID_HANDLE_VALUE) return 0;
if (GetFileTime(h, type==0?&ft:0, type==1?&ft:0, type==2?&ft:0)) l = (((unsigned long long)ft.dwHighDateTime)<<32) | ft.dwLowDateTime;
CloseHandle(h);
return filetimeTo1970(l);
}



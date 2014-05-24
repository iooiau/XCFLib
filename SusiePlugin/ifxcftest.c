/******************************************************************************
*                                                                             *
*    ifxcftest.c                                 Copyright(c) 2010 itow,y.    *
*                                                                             *
******************************************************************************/

/*
  ifxcf - GIMP XCF format Susie plugin library
  Copyright(c) 2010 itow,y.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301, USA.
*/


#include <windows.h>
#include <tchar.h>
#include "ifxcf.c"


#define MAIN_WINDOW_CLASS TEXT("ifxcftest main")

static HINSTANCE hInst;
static HWND hwndMain;
static HLOCAL hBmInfo=NULL,hBmBits=NULL;
static struct PictureInfo CurPictureInfo;




static int __stdcall LoadProgress(int Num,int Denom,long Data)
{
	if (Denom>0) {
		TCHAR szText[64];

		wsprintf(szText,TEXT("Loading... %d %%"),Num*100/Denom);
		SetWindowText(hwndMain,szText);
	}
	return 0;
}


static BOOL LoadFile(HWND hwnd,LPCTSTR pszFileName,BOOL fLoadFromMemory)
{
	char szFileName[MAX_PATH];
	HANDLE hFile;
	int Result;
	struct PictureInfo Info;
	HLOCAL hInfo,hBits;
	TCHAR szText[256];

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,
						pszFileName,-1,szFileName,MAX_PATH,NULL,NULL);
#else
	lstrcpy(szFileName,pszFileName);
#endif

	hFile=CreateFile(pszFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
					 OPEN_EXISTING,0,NULL);
	if (hFile==INVALID_HANDLE_VALUE) {
		MessageBox(hwnd,TEXT("CreateFile()"),NULL,MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (fLoadFromMemory) {
		ULARGE_INTEGER FileSize;
		void *pBuffer;
		DWORD Read;

		FileSize.LowPart=GetFileSize(hFile,&FileSize.HighPart);
		if (FileSize.LowPart==INVALID_FILE_SIZE
				&& GetLastError()!=NO_ERROR) {
			CloseHandle(hFile);
			MessageBox(hwnd,TEXT("GetFileSize()"),NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		if (FileSize.QuadPart==0 || FileSize.QuadPart>100*1024*1024) {
			CloseHandle(hFile);
			MessageBox(hwnd,TEXT("Invalid file size."),NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		pBuffer=xcflMemoryAlloc(max(FileSize.LowPart,2048));
		if (pBuffer==NULL) {
			CloseHandle(hFile);
			MessageBox(hwnd,TEXT("Out of memory."),NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		if (!ReadFile(hFile,pBuffer,FileSize.LowPart,&Read,NULL)
				|| Read!=FileSize.LowPart) {
			xcflMemoryFree(pBuffer);
			CloseHandle(hFile);
			MessageBox(hwnd,TEXT("ReadFile()"),NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		if (FileSize.LowPart<2048)
			ZeroMemory((BYTE*)pBuffer+FileSize.LowPart,2048-FileSize.LowPart);
		if (IsSupported(szFileName,(DWORD)pBuffer)==0) {
			xcflMemoryFree(pBuffer);
			CloseHandle(hFile);
			MessageBox(hwnd,TEXT("IsSupported() return 0"),NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		Result=GetPictureInfo((LPSTR)pBuffer,Read,1,&Info);
		if (Result!=0) {
			xcflMemoryFree(pBuffer);
			CloseHandle(hFile);
			wsprintf(szText,TEXT("GetPictureInfo() Error code %d"),Result);
			MessageBox(hwnd,szText,NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		Result=GetPicture((LPSTR)pBuffer,Read,1,&hInfo,&hBits,
						  (FARPROC)LoadProgress,0);

		xcflMemoryFree(pBuffer);
	} else {
		if (IsSupported(szFileName,(DWORD)hFile)==0) {
			CloseHandle(hFile);
			MessageBox(hwnd,TEXT("IsSupported() return 0"),NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		Result=GetPictureInfo(szFileName,0,0,&Info);
		if (Result!=0) {
			CloseHandle(hFile);
			wsprintf(szText,TEXT("GetPictureInfo() Error code %d"),Result);
			MessageBox(hwnd,szText,NULL,MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		Result=GetPicture(szFileName,0,0,&hInfo,&hBits,
						  (FARPROC)LoadProgress,0);
	}

	CloseHandle(hFile);

	if (Result!=0) {
		wsprintf(szText,TEXT("GetPicture() Error code %d"),Result);
		MessageBox(hwnd,szText,NULL,MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (hBmInfo!=NULL)
		LocalFree(hBmInfo);
	if (hBmBits!=NULL)
		LocalFree(hBmBits);
	hBmInfo=hInfo;
	hBmBits=hBits;
	CurPictureInfo=Info;

	SetWindowText(hwnd,fLoadFromMemory?TEXT("[Memory]"):pszFileName);
	InvalidateRect(hwnd,NULL,TRUE);

	return TRUE;
}


static LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			int OldBkMode;
			COLORREF OldTextColor;

			BeginPaint(hwnd,&ps);
			OldBkMode=SetBkMode(ps.hdc,TRANSPARENT);
			if (hBmInfo!=NULL && hBmBits!=NULL) {
				BITMAPINFO *pbmi=(BITMAPINFO*)LocalLock(hBmInfo);
				void *pBits=LocalLock(hBmBits);
				RECT rc;
				int Width,Height;
				int OldStretchMode;
				TCHAR szText[256];

				GetClientRect(hwnd,&rc);
				if (pbmi->bmiHeader.biWidth<=rc.right
						&& pbmi->bmiHeader.biHeight<=rc.bottom) {
					Width=pbmi->bmiHeader.biWidth;
					Height=pbmi->bmiHeader.biHeight;
				} else {
					Width=(pbmi->bmiHeader.biWidth*rc.bottom)/
													pbmi->bmiHeader.biHeight;
					if (Width>rc.right)
						Width=rc.right;
					Height=(pbmi->bmiHeader.biHeight*rc.right)/
													pbmi->bmiHeader.biWidth;
					if (Height>rc.bottom)
						Height=rc.bottom;
				}
				OldStretchMode=SetStretchBltMode(ps.hdc,STRETCH_HALFTONE);
				StretchDIBits(ps.hdc,(rc.right-Width)/2,(rc.bottom-Height)/2,
							  Width,Height,0,0,
							  pbmi->bmiHeader.biWidth,pbmi->bmiHeader.biHeight,
							  pBits,pbmi,DIB_RGB_COLORS,SRCCOPY);
				SetStretchBltMode(ps.hdc,OldStretchMode);
				LocalUnlock(hBmInfo);
				LocalUnlock(hBmBits);

				wsprintf(szText,TEXT("%ld x %ld / %d x %d ppi / %d bps"),
						 CurPictureInfo.width,CurPictureInfo.height,
						 CurPictureInfo.x_density,CurPictureInfo.y_density,
						 CurPictureInfo.colorDepth);
				OldTextColor=SetTextColor(ps.hdc,RGB(0,224,0));
				TextOut(ps.hdc,0,0,szText,lstrlen(szText));
			} else {
				LPCTSTR pszMessage=TEXT("Please drop XCF file.");
				OldTextColor=SetTextColor(ps.hdc,GetSysColor(COLOR_WINDOWTEXT));
				TextOut(ps.hdc,0,0,pszMessage,lstrlen(pszMessage));
			}
			SetTextColor(ps.hdc,OldTextColor);
			SetBkMode(ps.hdc,OldBkMode);
			EndPaint(hwnd,&ps);
		}
		return 0;

	case WM_DROPFILES:
		{
			HDROP hDrop=(HDROP)wParam;
			TCHAR szFileName[MAX_PATH];

			DragQueryFile(hDrop,0,szFileName,MAX_PATH);
			LoadFile(hwnd,szFileName,GetKeyState(VK_SHIFT)<0);
			DragFinish(hDrop);
		}
		return 0;

	case WM_DESTROY:
		if (hBmInfo!=NULL)
			LocalFree(hBmInfo);
		if (hBmBits!=NULL)
			LocalFree(hBmBits);

		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


int WINAPI _tWinMain (HINSTANCE hInstance,HINSTANCE hPrevInstance,
											LPTSTR pszCmdLine, int nCmdShow)
{
	MSG msg;

#ifdef XCFL_MEMORY_DEBUG
	xcflMemoryDebugInit();
#endif

	hInst=hInstance;

	{
		WNDCLASS wc;

		wc.style=CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc=WndProc;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=hInst;
		wc.hIcon=NULL;
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=(HBRUSH)(COLOR_3DFACE+1);
		wc.lpszMenuName=NULL;
		wc.lpszClassName=MAIN_WINDOW_CLASS;
		if (RegisterClass(&wc)==0) {
			MessageBox(NULL,TEXT("Failed to register class."),NULL,
														MB_OK | MB_ICONSTOP);
			return 0;
		}
	}

	hwndMain=CreateWindowEx(WS_EX_ACCEPTFILES,MAIN_WINDOW_CLASS,
							TEXT("XCF Susie Plug-in Test"),
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT,CW_USEDEFAULT,640,480,
							NULL,NULL,hInst,NULL);
	if (hwndMain==NULL) {
		MessageBox(NULL,TEXT("Failed to create window."),NULL,
														MB_OK | MB_ICONSTOP);
		return 0;
	}
	ShowWindow(hwndMain,nCmdShow);

	while (GetMessage(&msg,NULL,0,0)>0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


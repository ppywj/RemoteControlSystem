#pragma once
#include"pch.h"
#include<windows.h>
bool ifHasSetAutoStartup()
{
	CString strPath = "C:\\Windows\\System32\\remoteControlServer.exe";
	HKEY hKey;
	CString savedKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD dataSize = 0;
		if (RegQueryValueEx(hKey, "remoteControlServer", NULL, NULL, NULL, &dataSize) == ERROR_SUCCESS)
		{
			TCHAR* buffer = new TCHAR[dataSize / sizeof(TCHAR)];
			if (RegQueryValueEx(hKey, "remoteControlServer", NULL, NULL, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS)
			{
				savedKey = buffer;
			}
			delete[] buffer;
			if (savedKey == strPath)
			{
				RegCloseKey(hKey);
				return true;
			}
			RegCloseKey(hKey);
			return false;
		}
		RegCloseKey(hKey);
		return false;
	}
	RegCloseKey(hKey);
	return false;
}


//设置为启动项自动启动
void startup()
{
	//获取启动项目录位置
	TCHAR pathdis[MAX_PATH];
	if (SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, SHGFP_TYPE_CURRENT, pathdis) != S_OK)
	{
		return;
	}
	CString pathDist = CString(pathdis) + "\\remoteControlServer.exe";
	//获取当前程序位置  如果没有参数命令行参数那么也可以用  CString pathsou=GetCommandLine();但是这个字符串带上了双引号
	char pathsou[MAX_PATH];
	GetModuleFileName(NULL, pathsou, MAX_PATH);
	CString pathSource(pathsou);
	if (PathFileExists(pathDist))//如果存在直接返回
	{
		return;
	}
	int ret = CopyFile(pathSource, pathDist, FALSE);//FALSE表示如果存在则覆盖
	if (ret == FALSE)
	{
		MessageBox(NULL, _T("设置开机自启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
	}
}

//选择开机自启动
void ChooseAutoInvoke()
{
	if (ifHasSetAutoStartup())
	{
		return;
	}
	TCHAR szPath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	CString strPath(szPath);

	CString  strSubKey = _T("SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
	CString strInfo = _T("该程序只允许用于合法用途!\n");
	strInfo += _T("继续运行该程序,将使得这台机器处于被监控状态!\n");
	strInfo += _T("如果你不希望这样,请按取消，退出程序\n");
	strInfo += _T("按下“是”按钮,该程序将被复制到你的系统目录下，并随系统启动而启动\n");
	strInfo += _T("按下“否”按钮，该程序只运行一次，不会在系统目录留下任何东西\n");
	int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
	if (ret == IDYES)
	{
		std::string strDist = "C:\\Windows\\System32\\remoteControlServer.exe";
		std::string strCmd = "mklink " + strDist + std::string(strPath.GetString()) + " " + strPath.GetString();
		system(strCmd.c_str());
		HKEY hKey = NULL;
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_WRITE, &hKey);
		if (ret != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			MessageBox(NULL, _T("设置开机自启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			exit(0);
		}
		ret = RegSetValueEx(hKey, _T("remoteControlServer"), 0, REG_EXPAND_SZ, (BYTE*)strDist.c_str(), strDist.length());
		if (ret != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			MessageBox(NULL, _T("设置开机自启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			exit(0);
		}
		RegCloseKey(hKey);
	}
	else if (ret == IDCANCEL)
	{
		exit(0);
	}
	return;
}


void ShowError()
{
	LPSTR lpMessageBuf = NULL;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMessageBuf, 0, NULL);
	OutputDebugString(lpMessageBuf);
	LocalFree(lpMessageBuf);
}

bool IsAdmin()
{
	HANDLE hToken = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		ShowError();
		return false;
	}
	TOKEN_ELEVATION eve;
	DWORD len = 0;
	if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE)
	{
		ShowError();
		return false;
	}
	CloseHandle(hToken);
	if (len == sizeof(eve))
	{
		return eve.TokenIsElevated;
	}
	TRACE("length of tokeninformation is:%d", len);
	return false;
}
void RunAsAdmin()
{
	HANDLE hToken = NULL;
	BOOL ret = LogonUser("Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
	if (!ret)
	{
		ShowError();
		MessageBox(NULL, _T("管理员登录错误"), _T("程序错误"), 0);
		exit(0);
	}
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	TCHAR sPath[MAX_PATH] = _T("");
	GetCurrentDirectory(MAX_PATH, sPath);
	CString exePath = CString(sPath) + "\\remoteControlServer.exe";
	ret = CreateProcessWithLogonW(L"Administrator", NULL, NULL, LOGON_WITH_PROFILE, NULL, (LPWSTR)exePath.GetString(), CREATE_UNICODE_ENVIRONMENT, NULL, NULL, (LPSTARTUPINFOW)&si, &pi);
	CloseHandle(hToken);
	if (!ret)
	{
		ShowError();
		MessageBox(NULL, "创建进程失败", "启动错误", MB_OK);
		exit(0);
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
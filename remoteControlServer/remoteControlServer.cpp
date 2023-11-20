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
			CServerSocket* pServer = CServerSocket::getServerSocketInstance();
			int ret=pServer->run(&CCommandEx::runCommand, (void*)&cmd, 9527);
			if (ret == 1||ret==2)
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
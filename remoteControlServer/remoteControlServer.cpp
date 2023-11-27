#include "pch.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "framework.h"
#include "remoteControlServer.h"
#include"ServerSocket.h"
#include"CCommandEx.h"
#include"Util.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的应用程序对象
CWinApp theApp;
using namespace std;
HANDLE hIOCP = INVALID_HANDLE_VALUE;
int main()
{

	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule == nullptr)
		return 1;
	CCommandEx cmd;
		// 初始化 MFC 并在失败时显示错误
	if (AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
	{
		if (!IsAdmin())
		{
			RunAsAdmin();
		}
		ChooseAutoInvoke();
		//startup();
		CServerSocket* pServer = CServerSocket::getServerSocketInstance();
		int ret = pServer->run(&CCommandEx::runCommand, (void*)&cmd, 9527);
		if (ret == 1 || ret == 2)
		{
			AfxMessageBox("网络初始化错误,请检查网络");
		}
	}
	else
		return 1;
	return nRetCode;
	_CrtDumpMemoryLeaks();
}
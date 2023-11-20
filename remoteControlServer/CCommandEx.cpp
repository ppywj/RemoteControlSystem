#include "pch.h"
#include "CCommandEx.h"
#include "resource.h"
#include"ServerSocket.h"
#include<direct.h>
#include<stdio.h>
#include<io.h>
#include<list>
#include<string>
#include <system_error>
#include<iostream>
#include<afxwin.h>
#include<atlimage.h>
#include<thread>

int CCommandEx::runCommand(void* thiz, int type, list<CPacket>& packetList, CPacket& inPacket)
{
	CCommandEx* cmdObj = (CCommandEx*)thiz;
	return cmdObj->executeCommand(type,packetList,inPacket);
}
int CCommandEx::executeCommand(int nCmd, list<CPacket>& packetList, CPacket& inPacket)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
	if (it == m_mapFunction.end())
	{
		return -1;
	}
	return (this->*it->second)(packetList,inPacket);
}

CCommandEx::CCommandEx()
{
	ifLock = false;
	struct {
		int nCmd;
		CMDFUNC func;
	}data[] = { 
		{1,&CCommandEx::MakeDriverInfo},
		{2,&CCommandEx::MakeDirectoryInfo},
		{3,&CCommandEx::RunFile},
		{4,&CCommandEx::DownloadFile},
		{5,&CCommandEx::MouseEventEx},
		{6,&CCommandEx::sendScreen},
		{7,&CCommandEx::lockMachine},
		{8,&CCommandEx::unLockMachine},
		{9,&CCommandEx::deleteFile},
		{1008,NULL},
		{-1,NULL}
	};
	for (int i = 0; data[i].nCmd != -1; i++)
	{
		m_mapFunction.insert(std::make_pair(data[i].nCmd, data[i].func));
	}
}

CCommandEx::~CCommandEx()
{
}

int CCommandEx::MakeDriverInfo(list<CPacket>& packetList, CPacket& inPacket)
{//从A 到 Z （AB 是软盘)
	std::string result;
	for (int i = 1; i <= 26; i++)
	{

		if (_chdrive(i) == 0)
		{
			if (result.size() > 0)
				result += ',';
			result += 'A' + i - 1;
		}
	}
	CPacket pack = CPacket(1, (BYTE*)result.c_str(), result.size());
	packetList.push_back(pack);
	return 0;
}

int CCommandEx::MakeDirectoryInfo(list<CPacket>& packetList, CPacket& inPacket)
{
	//int count = 0;
	std::list<FILEINFO>FileInfolist;//文件列表
	std::string strPath = inPacket.getStrData();
	//切换盘符
	if (_chdir(strPath.c_str()) != 0)
	{
		OutputDebugString(_T("没有权限,访问目录"));
		FILEINFO finfo;
		finfo.IsValid = FALSE;
		FileInfolist.push_back(finfo);
		CPacket pack = CPacket(2, (BYTE*)&finfo, sizeof(finfo));
		packetList.push_back(pack);
		return -2;
	}
	_finddata_t fdata;
	long long hfind = 0;
	if ((hfind = _findfirst("*", &fdata)) == -1)
	{
		OutputDebugString(_T("没有找到任何文件"));
		FILEINFO finfo;
		finfo.HasNext = false;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		packetList.push_back(pack);
		return -3;
	}
	do {
		FILEINFO finfo;
		finfo.IsDirectory = ((fdata.attrib & _A_SUBDIR) != 0);
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		TRACE(finfo.szFileName, "\r\n");///输出文件名
		CPacket pack = CPacket(2, (BYTE*)&finfo, sizeof(finfo));
		packetList.push_back(pack);
	} while (_findnext(hfind, &fdata) == 0);
	//报错访问冲突  这是因为这个函数是x86下的在x64中第一个参数是8字节的 

	//没有下一个文件了那么传递一个终止信号
	FILEINFO endInfo;
	endInfo.HasNext = FALSE;
	CPacket pack = CPacket(2, (BYTE*)&endInfo, sizeof(endInfo));
	packetList.push_back(pack);
	//MessageBox(NULL,std::to_string(count).c_str(), _T("传输文件数量"), MB_OK);
	return 0;
}

int CCommandEx::RunFile(list<CPacket>& packetList, CPacket& inPacket)
{
	std::string filePath = inPacket.getStrData();
	ShellExecuteA(NULL, NULL, filePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	CPacket pack(3, NULL, 0);
	packetList.push_back(pack);
	return 0;
}

int CCommandEx::DownloadFile(list<CPacket>& packetList, CPacket& inPacket)
{
	std::string strPath = inPacket.getStrData();
	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
	long long data = 0;
	//打开文件失败
	if (err != 0 || pFile == NULL)
	{
		CPacket packet(4, (BYTE*)&data, 8);
		packetList.push_back(packet);
		return -1;
	}
	//获取文件长度
	fseek(pFile, 0, SEEK_END);
	data = _ftelli64(pFile);
	fseek(pFile, 0, SEEK_SET);//移动到文件开头
	CPacket head(4, (BYTE*)&data, 8);
	packetList.push_back(head);
	char buffer[1024] = "";
	size_t rlen = 0;
	do {
		rlen = fread_s(buffer, 1024, 1, 1024, pFile);
		CPacket pack(4, (BYTE*)buffer, rlen);
		packetList.push_back(pack);
	} while (rlen >= 1024);
	CPacket pack(4, NULL, 0);
	packetList.push_back(pack);
	fclose(pFile);
	return 0;
}

int CCommandEx::MouseEventEx(list<CPacket>& packetList, CPacket& inPacket)
{
	MOUSEEV mouse = *(MouseEvent*)(inPacket.getStrData().c_str());

		DWORD nFlags = 0;
		switch (mouse.nButton)
		{
		case 0://左键
			nFlags = 1;
			break;
		case 1://右键
			nFlags = 2;
			break;
		case 2://中键
			nFlags = 4;
			break;
		case 4://没有按键
			nFlags = 8;
			break;
		}
		if (nFlags != 8)
			//移动鼠标位置
			SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0://单机
			nFlags |= 0x10;
			break;
		case 1://双击
			nFlags |= 0x20;
			break;
		case 2://按下
			nFlags |= 0x40;
			break;
		case 3://放开
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		switch (nFlags)
		{
		case 0x21://左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键放开
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://右键放开
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://中键放开
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://鼠标移动
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
		}
	return 0;
}

void CCommandEx::lockThread(void* arg)
{
	ifLock = true;
	lockDlg.Create(IDD_CLockDlg, NULL);
	lockDlg.ShowWindow(SW_SHOW);
	CRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rect.bottom = rect.bottom * 1.05;
	lockDlg.MoveWindow(rect);
	//窗口置顶
	lockDlg.SetWindowPos(&lockDlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	//限制鼠标功能
	ShowCursor(false);
	//限制鼠标活动范围
	rect.left = 0;
	rect.top = 0;
	rect.right = 1;
	rect.bottom = 1;
	ClipCursor(rect);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (!ifLock)
			break;
	}
	//while (ifLock)
	//{
	//	GetMessage(&msg, NULL, 0, 0);
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//	if (msg.message == WM_KEYDOWN)
	//		break;
	//}
	lockDlg.DestroyWindow();
	ShowCursor(true);
	TRACE("%08x\r\n", lockDlg.m_hWnd);
	_endthread();
}

int CCommandEx::lockMachine(list<CPacket>& packetList, CPacket& inPacket)
{
	if (lockDlg.m_hWnd == NULL || lockDlg.m_hWnd == INVALID_HANDLE_VALUE)
	{
		std::thread(lockThread);
		TRACE("加锁成功");
	}
	CPacket packet(7, NULL, 0);
	packetList.push_back(packet);
	return 0;
}

int CCommandEx::unLockMachine(list<CPacket>& packetList, CPacket& inPacket)
{
	ifLock = false;
	CPacket packet(8, NULL, 0);
	packetList.push_back(packet);
	TRACE("解锁成功");
	return 0;
}

int CCommandEx::deleteFile(list<CPacket>& packetList, CPacket& inPacket)
{
	CServerSocket* server = CServerSocket::getServerSocketInstance();
	int ret = DeleteFile(server->GetPacket().strData.c_str());
	//删除失败
	if (!ret)
	{
		//CPacket pack(9, (BYTE*)"1", 1);
		//server->Send(pack);
		return 1;
	}
	//删除成功
	//CPacket pack(9, (BYTE*)"0", 1);
	//server->Send(pack);
	return 0;
}

int CCommandEx::sendScreen(list<CPacket>& packetList, CPacket& inPacket)
{
	// 创建一个空的CImage对象，用于存储屏幕截图
	CImage screen;

	// 获取桌面窗口设备上下文（DC），它是Windows用来绘制图形的一个抽象概念
	HDC hScreen = ::GetDC(NULL);

	// 获取屏幕位深度，即每个像素所占的位数
	int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);

	// 获取屏幕分辨率，即宽度和高度
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);

	// 使用获取到的参数创建一个CImage对象，用于存储屏幕截图
	screen.Create(nWidth, nHeight, nBitPerPixel);

	// 将桌面窗口的图像复制到CImage对象中，实现屏幕截图
	BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);

	// 释放掉之前获取的桌面窗口设备上下文
	ReleaseDC(NULL, hScreen);

	// 分配一段可移动的全局内存空间，用于存放屏幕截图数据
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);

	// 如果分配内存失败，则返回-1
	if (hMem == NULL)
		return -1;

	// 创建一个新的IStream接口实例，并将其与刚刚分配的全局内存关联起来
	IStream* pStream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	if (hr != S_OK)
		return -1;

	// 将IStream位置重置到开头，准备读取数据
	LARGE_INTEGER bg = { 0 };
	pStream->Seek(bg, STREAM_SEEK_SET, NULL);

	// 将IStream中的一部分数据锁定，以供后续使用
	// 将屏幕截图保存到IStream中，格式为PNG
	screen.Save(pStream, Gdiplus::ImageFormatPNG);

	PBYTE pData = (PBYTE)GlobalLock(hMem);
	SIZE_T nSize = GlobalSize(hMem);
	CPacket pack(6, pData, nSize);
	// 将锁定的那部分数据打包成CPacket对象，
	packetList.push_back(pack);
	// 最后，解锁先前锁定的那段数据
	GlobalUnlock(hMem);
	pStream->Release();
	GlobalFree(hMem);
	screen.ReleaseDC();
	TRACE("send:发送了一张屏幕截图,length=%d,包大小:%d\r\n", pack.nLength, sizeof(pack));
	return 0;
}
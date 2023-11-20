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
{//��A �� Z ��AB ������)
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
	std::list<FILEINFO>FileInfolist;//�ļ��б�
	std::string strPath = inPacket.getStrData();
	//�л��̷�
	if (_chdir(strPath.c_str()) != 0)
	{
		OutputDebugString(_T("û��Ȩ��,����Ŀ¼"));
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
		OutputDebugString(_T("û���ҵ��κ��ļ�"));
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
		TRACE(finfo.szFileName, "\r\n");///����ļ���
		CPacket pack = CPacket(2, (BYTE*)&finfo, sizeof(finfo));
		packetList.push_back(pack);
	} while (_findnext(hfind, &fdata) == 0);
	//������ʳ�ͻ  ������Ϊ���������x86�µ���x64�е�һ��������8�ֽڵ� 

	//û����һ���ļ�����ô����һ����ֹ�ź�
	FILEINFO endInfo;
	endInfo.HasNext = FALSE;
	CPacket pack = CPacket(2, (BYTE*)&endInfo, sizeof(endInfo));
	packetList.push_back(pack);
	//MessageBox(NULL,std::to_string(count).c_str(), _T("�����ļ�����"), MB_OK);
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
	//���ļ�ʧ��
	if (err != 0 || pFile == NULL)
	{
		CPacket packet(4, (BYTE*)&data, 8);
		packetList.push_back(packet);
		return -1;
	}
	//��ȡ�ļ�����
	fseek(pFile, 0, SEEK_END);
	data = _ftelli64(pFile);
	fseek(pFile, 0, SEEK_SET);//�ƶ����ļ���ͷ
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
		case 0://���
			nFlags = 1;
			break;
		case 1://�Ҽ�
			nFlags = 2;
			break;
		case 2://�м�
			nFlags = 4;
			break;
		case 4://û�а���
			nFlags = 8;
			break;
		}
		if (nFlags != 8)
			//�ƶ����λ��
			SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0://����
			nFlags |= 0x10;
			break;
		case 1://˫��
			nFlags |= 0x20;
			break;
		case 2://����
			nFlags |= 0x40;
			break;
		case 3://�ſ�
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		switch (nFlags)
		{
		case 0x21://���˫��
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://�������
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://�������
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://����ſ�
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://�Ҽ�˫��
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://�Ҽ�����
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://�Ҽ�����
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://�Ҽ��ſ�
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://�м�˫��
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://�м�����
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://�м�����
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://�м��ſ�
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://����ƶ�
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
	//�����ö�
	lockDlg.SetWindowPos(&lockDlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	//������깦��
	ShowCursor(false);
	//���������Χ
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
		TRACE("�����ɹ�");
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
	TRACE("�����ɹ�");
	return 0;
}

int CCommandEx::deleteFile(list<CPacket>& packetList, CPacket& inPacket)
{
	CServerSocket* server = CServerSocket::getServerSocketInstance();
	int ret = DeleteFile(server->GetPacket().strData.c_str());
	//ɾ��ʧ��
	if (!ret)
	{
		//CPacket pack(9, (BYTE*)"1", 1);
		//server->Send(pack);
		return 1;
	}
	//ɾ���ɹ�
	//CPacket pack(9, (BYTE*)"0", 1);
	//server->Send(pack);
	return 0;
}

int CCommandEx::sendScreen(list<CPacket>& packetList, CPacket& inPacket)
{
	// ����һ���յ�CImage�������ڴ洢��Ļ��ͼ
	CImage screen;

	// ��ȡ���洰���豸�����ģ�DC��������Windows��������ͼ�ε�һ���������
	HDC hScreen = ::GetDC(NULL);

	// ��ȡ��Ļλ��ȣ���ÿ��������ռ��λ��
	int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);

	// ��ȡ��Ļ�ֱ��ʣ�����Ⱥ͸߶�
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);

	// ʹ�û�ȡ���Ĳ�������һ��CImage�������ڴ洢��Ļ��ͼ
	screen.Create(nWidth, nHeight, nBitPerPixel);

	// �����洰�ڵ�ͼ���Ƶ�CImage�����У�ʵ����Ļ��ͼ
	BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);

	// �ͷŵ�֮ǰ��ȡ�����洰���豸������
	ReleaseDC(NULL, hScreen);

	// ����һ�ο��ƶ���ȫ���ڴ�ռ䣬���ڴ����Ļ��ͼ����
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);

	// ��������ڴ�ʧ�ܣ��򷵻�-1
	if (hMem == NULL)
		return -1;

	// ����һ���µ�IStream�ӿ�ʵ������������ոշ����ȫ���ڴ��������
	IStream* pStream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	if (hr != S_OK)
		return -1;

	// ��IStreamλ�����õ���ͷ��׼����ȡ����
	LARGE_INTEGER bg = { 0 };
	pStream->Seek(bg, STREAM_SEEK_SET, NULL);

	// ��IStream�е�һ���������������Թ�����ʹ��
	// ����Ļ��ͼ���浽IStream�У���ʽΪPNG
	screen.Save(pStream, Gdiplus::ImageFormatPNG);

	PBYTE pData = (PBYTE)GlobalLock(hMem);
	SIZE_T nSize = GlobalSize(hMem);
	CPacket pack(6, pData, nSize);
	// ���������ǲ������ݴ����CPacket����
	packetList.push_back(pack);
	// ��󣬽�����ǰ�������Ƕ�����
	GlobalUnlock(hMem);
	pStream->Release();
	GlobalFree(hMem);
	screen.ReleaseDC();
	TRACE("send:������һ����Ļ��ͼ,length=%d,����С:%d\r\n", pack.nLength, sizeof(pack));
	return 0;
}
#include "pch.h"
#include "Controller.h"
#include"Utils.h"
#include<list>
using std::list;
CController::Helper CController::helper;
CController* CController::m_instance = nullptr;


// 1���鿴���̷���
// 2���鿴ָ��Ŀ¼�µ��ļ�
// 3�����ļ�
// 4�������ļ�
// 5��������
// 6��������Ļ����
// 7������
// 8������
int CController::sendCommandPacket(bool ifrecv, std::list<CPacket>& resultPackList, WORD cmd, BYTE* data, size_t size, bool ifMutiple)
{
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	if (!ifStartSendAndRcvThread)
	{
		mutex x;
		x.lock();
		ifStartSendAndRcvThread = true;
		pClient->recvThreadEntry(CClientSocket::getClientSocketInstance());
		x.unlock();
		Sleep(5);
	}
	if (ifrecv) {//��ҪӦ��

		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		CPacket pack(cmd, data, size, hEvent);
		pack.ifRecvMutiple = ifMutiple;
		pClient->addSendPacket(pack);
		TRACE("�����ͼƬ����\r\n");
		auto pr = pClient->m_packMap.insert({ hEvent, std::list<CPacket>() });
		WaitForSingleObject(hEvent, INFINITE);//һֱ�ȴ�������հ�
		TRACE("�ȵ���ͼƬ��Ӧ\r\n");
		auto it = pClient->m_packMap.find(hEvent);
		CloseHandle(hEvent);
		if (it != pClient->m_packMap.end())
		{
			for (auto i = it->second.begin(); i != it->second.end(); i++)
			{
				resultPackList.push_back(*i);
			}
			pClient->m_packMap.erase(it);
			return true;
		}
	}
	else//��ҪӦ��
	{
		CPacket pack(cmd, data, size, NULL);
		pClient->addSendPacket(pack);
		return true;
	}
	return false;
}



void CController::closeConnect()
{
	CClientSocket::getClientSocketInstance()->close();
}

const CPacket& CController::GetPacket()
{
	return CClientSocket::getClientSocketInstance()->GetPacket();
}

void CController::setIpAndPort(CString ip, short port)
{
	CClientSocket::getClientSocketInstance()->setIpAndPort(ip, port);
}

int CController::initSocket()
{
	return CClientSocket::getClientSocketInstance()->InitSocket();
}

void CController::loadDirectory(CString filePath, CListCtrl& file_list, CTreeCtrl& fileTree, HTREEITEM& hSelected)
{
	list<CPacket>resultPacketList;
	int nCmd = sendCommandPacket(true, resultPacketList, 2, (BYTE*)(LPCSTR)filePath.GetString(), filePath.GetLength(), true);
	int index = 0;
	while (!resultPacketList.empty())
	{
		PFILEINFO pInfo = (PFILEINFO)(resultPacketList.front().strData.c_str());
		if (pInfo->IsDirectory) {
			//�������� ��Ϊ��һ��������LPCSTR��
			//InserItemʹ�õ���Unicode����
			if (!(CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == ".."))
				fileTree.InsertItem(pInfo->szFileName, hSelected, TVI_LAST);
		}
		else {
			file_list.InsertItem(index++, pInfo->szFileName);
		}
		resultPacketList.pop_front();
	}
}

void CController::downLoadFile(CString filePath, CController* controller)
{
	setFilePath(filePath);
	_beginthread(downLoadFileThread, 0, (void*)controller);
}


void CController::downLoadFileThread(void* arg)
{
	bool isClose = false;
	CController* pController = (CController*)arg;
	FILE* pFile = nullptr;
	CString localPath;
	//�����ļ��Ի���    ����ֻ�����Ե��ļ� ������ļ���������������ʾ��
	CFileDialog fileDlg(TRUE, "*", pController->getFilePath(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, &pController->m_mainDlg);
	//ģ̬��ʽ�����ļ��Ի���
	long long loadedSize = 0;//������ɵĴ�С
	long long fileSize = 0;
	if (fileDlg.DoModal() == IDOK)
	{
		localPath = fileDlg.GetPathName();
		pFile = fopen(localPath, "wb+");
		if (pFile == NULL) {
			MessageBox(pController->m_mainDlg, "����û��Ȩ�ޱ�����ļ������ļ��޷�����!!!", "downLoad error", MB_OK);
			//pController->closeConnect();
			//�����д��Ż� Ӧ�������˷���һ������֪ͨ����˲��÷����ļ�������
			return;
		}
		TRACE("\r\n����:%s\r\n", pController->getFilePath());
		pController->m_statusDlg.info_edit.SetWindowText("�ļ�������������ִ����");
		pController->m_statusDlg.ShowWindow(SW_SHOW);
		pController->m_statusDlg.CenterWindow(&pController->m_mainDlg);
		pController->m_statusDlg.SetActiveWindow();
		list<CPacket>resultPacketList;
		int ret = pController->sendCommandPacket(true, resultPacketList, 4, (BYTE*)pController->getFilePath().GetString(), pController->getFilePath().GetLength(), true);
		CPacket sizePacket = resultPacketList.front();
		resultPacketList.pop_front();
		//��ȡ����
		fileSize = *(long long*)(sizePacket.strData.c_str());
		if (fileSize == 0)
		{
			pController->m_statusDlg.ShowWindow(SW_HIDE);
			MessageBox(pController->m_mainDlg, "�ļ�Ϊ�ջ�����Ȩ�޲����ԭ���޷���ȡ�ļ�", "downLoad error", MB_OK);
			fclose(pFile);
			return;
		}
		if (!ret)
		{
			pController->m_statusDlg.ShowWindow(SW_HIDE);
			AfxMessageBox("�ļ���������ִ��ʧ��", MB_TOPMOST);
			fclose(pFile);

			return;
		}
		if (resultPacketList.empty())
		{
			pController->m_statusDlg.ShowWindow(SW_HIDE);
			AfxMessageBox("�ļ���������ִ��ʧ��", MB_TOPMOST);
			fclose(pFile);
			return;
		}
		while (!resultPacketList.empty())
		{
			CPacket pack = resultPacketList.front();
			resultPacketList.pop_front();
			fwrite((pack.strData.c_str()), 1, (pack.strData.size()), pFile);
			loadedSize += pack.strData.size();
		}
	}
	if (pFile != NULL)
		fclose(pFile);
	if (!isClose) {
		pController->m_statusDlg.ShowWindow(SW_HIDE);
	}
	if (loadedSize < fileSize)
	{
		isClose = true;
		pController->m_statusDlg.ShowWindow(SW_HIDE);
		MessageBox(NULL, "�ļ�����δ��ɣ��������", "downLoad error", MB_OK);
	}
	else
	{
		isClose = true;
		pController->m_statusDlg.ShowWindow(SW_HIDE);
		// ��ȡ��ǰ����ڵľ��
		MessageBox(pController->m_mainDlg, pController->getFilePath() + "�������+������:" + localPath, "", MB_OK);
	}
	_endthread();
}

CController::CController() :

	m_thread(INVALID_HANDLE_VALUE),
	m_threadId(-1)
{
	struct {
		int nMsg;
		MSGFUNC func;
	}msgFuncArr[] = {
		{WM_SEND_PACK,&CController::OnSendPack},
		{WM_SEND_DATA,&CController::OnSendData},
		{WM_SHOW_STATUS,&CController::OnShowStatus},
		{WM_WATCH_SCREEN,&CController::OnWatchScreen},
		{0,NULL}
	};
	for (int i = 0; msgFuncArr[i].func != NULL; i++)
	{
		m_msgFuncMap.insert(std::make_pair(msgFuncArr[i].nMsg, msgFuncArr[i].func));
	}

}

CController::~CController()
{
	WaitForSingleObject(m_thread, 100);
}



void CController::setFilePath(const CString& path)
{
	m_filePath = path;
}

const CString CController::getFilePath()
{
	return m_filePath;
}


void CController::watchScreen(CController* controller)
{
	m_screenDlg.ShowWindow(SW_SHOW);
	_beginthread(watchThread, 0, (void*)controller);
}

void CController::watchThread(void* arg)
{
	CController* pController = (CController*)arg;
	pController->ifWatchDlgClose = false;
	Sleep(10);
	CClientSocket* pClient = NULL;
	do {
		pClient = CClientSocket::getClientSocketInstance();
	} while (pClient == NULL);
	list<CPacket>resultPacketList;
	for (;;)
	{
		//���Ӵ��ڹر���ֱ�ӽ����������
		if (pController->ifWatchDlgClose)
		{
			pController->isImgValid = false;
			pController->screenImg.Destroy();
			break;
		}
		if (!pController->isImgValid)
		{
			TRACE("������һ��ͼƬ����\r\n");
			int ret = pController->sendCommandPacket(true, resultPacketList, 6);
			TRACE("�����һ��ͼƬ����\r\n");
			if (ret)
			{
				if (!resultPacketList.empty()) {
					CPacket pack = resultPacketList.front();
					resultPacketList.pop_front();
					if (pack.sCmd == 6)
					{
						//TRACE("���յ�һ����Ļ��ͼ,��У��%08X:\r\n",pack.sSum);
						std::string strBuffer = pack.strData;
						if (Utils::GetImage(pController->screenImg, strBuffer) == 0)
							pController->isImgValid = true;
						else
							continue;
					}
				}
			}
			Sleep(20);
		}
		else
		{
			Sleep(1);
		}
	}
	_endthread();
}

//��ʼ��������
int CController::InitController()
{
	m_thread = (HANDLE)_beginthreadex(NULL, 0, &CController::threadEntry, this, 0, &m_threadId);
	m_statusDlg.Create(IDD_DIALOG_STATUS, &m_mainDlg);
	m_statusDlg.ShowWindow(SW_HIDE);
	m_screenDlg.Create(IDD_DIALOG_SCREEN, &m_mainDlg);
	m_screenDlg.ShowWindow(SW_HIDE);
	return 0;
}

//����������
int CController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_mainDlg;
	return m_mainDlg.DoModal();
	return 0;
}

//���Ͱ�
void CController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CController* pClient = CController::getInstance();
	if (pClient == NULL)
		return;
	if (!pClient->initSocket()) {
		return;
	}
	pClient->closeConnect();
}

//��������
void CController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
}

//չʾ״̬����
void CController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	m_statusDlg.DoModal();
}

//Զ�̼��
void CController::OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	m_screenDlg.DoModal();
}

unsigned __stdcall CController::threadEntry(void* arg)
{
	CController* controller = (CController*)arg;
	controller->threadFunc();
	_endthreadex(0);
	return 0;
}

void CController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MsgInfo* pMsg = (MsgInfo*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			map<int, MSGFUNC>::iterator it = m_msgFuncMap.find(pMsg->msg.message);

			if (it != m_msgFuncMap.end())
			{
				(this->*it->second)(pMsg->msg.message, pMsg->msg.wParam, pMsg->msg.lParam);
			}
			else
			{
				pMsg->result = -1;
			}
			ResetEvent(hEvent);
		}
		else
		{
			map<int, MSGFUNC>::iterator it = m_msgFuncMap.find(msg.message);

			if (it != m_msgFuncMap.end())
			{
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}

	}
}

//���ڿ��߳� �������ﲢû��ֱ�ӽ�Ҫ�ֲ�����uuid��msg������һ���߳� ������һ���̷߳��ʵ�ʱ�������Ѿ�ʧЧ��
//�������������Ա����map����һ�����ݳ־û�
LRESULT CController::SendMessageToThread(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)
		return -2;
	MsgInfo msgInfo(msg);
	PostThreadMessage(m_threadId, WM_SEND_MESSAGE, (WPARAM)&msgInfo, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, -1);//�����Ƶȴ�
	return msgInfo.result;
}

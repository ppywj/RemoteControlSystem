#include "pch.h"
#include "Controller.h"
#include"Utils.h"
CController::Helper CController::helper;
CController* CController::m_instance = nullptr;


int CController::sendCommandPacket(WORD cmd, BYTE* data, size_t size)
{
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	pClient->InitSocket();
	return pClient->Send(CPacket(cmd, data, size));
}

int CController::DealCommand()
{
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	return pClient->DealCommand();
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
	int nCmd = sendCommandPacket(2, (BYTE*)(LPCSTR)filePath.GetString(), filePath.GetLength());
	while (DealCommand() != 2)
	{
		TRACE("ѭ��һ��\r\n");
	}
	Sleep(1);
	int index = 0;
	PFILEINFO pInfo = (PFILEINFO)(GetPacket().strData.c_str());
	while (pInfo->HasNext) {
		if (pInfo->IsDirectory) {
			//�������� ��Ϊ��һ��������LPCSTR��
			//InserItemʹ�õ���Unicode����
			if (!(CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == ".."))
				fileTree.InsertItem(pInfo->szFileName, hSelected, TVI_LAST);
		}
		else {
			file_list.InsertItem(index++, pInfo->szFileName);

		}
		int cmd = DealCommand();
		if (cmd != 2)
			break;
		pInfo = (PFILEINFO)(GetPacket().strData.c_str());
		//TRACE("\nfileName:%s\r\n", pInfo->szFileName);
	}
	closeConnect();
}

void CController::downLoadFile(CString filePath,CController*controller)
{
	setFilePath(filePath);
	_beginthread(downLoadFileThread,0,(void*)controller);
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
	if (fileDlg.DoModal() == IDOK)
	{
		int ret = pController->sendCommandPacket(4, (BYTE*)pController->getFilePath().GetString(), pController->getFilePath().GetLength());
		TRACE("\r\n����:%s\r\n",pController->getFilePath());
		if (ret < 0)
		{
			AfxMessageBox("�ļ���������ִ��ʧ��", MB_TOPMOST);
			pController->closeConnect();
			return;
		}
		while (pController->DealCommand() != 4) {}
		//��ȡ����
		long long fileSize = *(long long*)(pController->GetPacket().strData.c_str());
		if (fileSize == 0)
		{
			MessageBox(pController->m_mainDlg, "�ļ�Ϊ�ջ�����Ȩ�޲����ԭ���޷���ȡ�ļ�", "downLoad error", MB_OK);
			pController->closeConnect();
			return;
		}
		localPath = fileDlg.GetPathName();
		pFile = fopen(localPath, "wb+");
		if (pFile == NULL) {
			MessageBox(pController->m_mainDlg,"����û��Ȩ�ޱ�����ļ������ļ��޷�����!!!", "downLoad error", MB_OK);
			pController->closeConnect();
			//�����д��Ż� Ӧ�������˷���һ������֪ͨ����˲��÷����ļ�������
			return;
		}
		long long loadedSize = 0;//������ɵĴ�С
		pController->m_statusDlg.info_edit.SetWindowText("�ļ�������������ִ����");
		pController->m_statusDlg.ShowWindow(SW_SHOW);
		pController->m_statusDlg.CenterWindow(&pController->m_mainDlg);
		pController->m_statusDlg.SetActiveWindow();
		while (loadedSize < fileSize)
		{
			int ret = pController->DealCommand();
			if (ret < 0)
			{
				MessageBox(pController->m_mainDlg,"�ļ����ش���","downLoad error",MB_OK);
				fclose(pFile);
				pController->closeConnect();
				return;
			}
			fwrite((pController->GetPacket().strData.c_str()), 1, (pController->GetPacket().strData.size()), pFile);
			loadedSize += pController->GetPacket().strData.size();
		}
		if (loadedSize < fileSize)
		{
			isClose = true;
			pController->m_statusDlg.ShowWindow(SW_HIDE);
			MessageBox(NULL,"�ļ�����δ��ɣ��������","downLoad error",MB_OK);
		}
		else
		{
			isClose = true;
			pController->m_statusDlg.ShowWindow(SW_HIDE);
			// ��ȡ��ǰ����ڵľ��
			MessageBox(pController->m_mainDlg,pController->getFilePath()+ "�������+������:" + localPath,"",MB_OK);
		}

	}
	if (pFile != NULL)
		fclose(pFile);
	pController->closeConnect();
	if (!isClose) {
		pController-> m_statusDlg.ShowWindow(SW_HIDE);
	}
	_endthread();
}

CController::CController():
	
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


void CController::watchScreen(CController*controller)
{
	m_screenDlg.ShowWindow(SW_SHOW);
	_beginthread(watchThread,0, (void*)controller);
}

void CController::watchThread(void* arg)
{
	CController* pController = (CController*)arg;
	pController->ifWatchDlgClose = false;
	Sleep(50);
	CClientSocket* pClient = NULL;
	do {
		pClient = CClientSocket::getClientSocketInstance();
	} while (pClient == NULL);

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
			int ret = pController->sendCommandPacket(6);
			if (ret)
			{
				int cmd = pClient->DealCommand();
				if (cmd == 6)
				{
					TRACE("���յ�һ����Ļ��ͼ\r\n");
					std::string strBuffer = pClient->GetPacket().strData;
					if (Utils::GetImage(pController->screenImg, strBuffer) == 0)
						pController->isImgValid = true;
					else
						continue;
				}

			}
			pClient->close();
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
LRESULT CController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

//��������
LRESULT CController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

//չʾ״̬����
LRESULT CController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.DoModal();
}

//Զ�̼��
LRESULT CController::OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_screenDlg.DoModal();
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
				LRESULT ret = (this->*it->second)(pMsg->msg.message, pMsg->msg.wParam, pMsg->msg.lParam);
				pMsg->result = ret;
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

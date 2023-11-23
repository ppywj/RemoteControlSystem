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
		TRACE("循环一次\r\n");
	}
	Sleep(1);
	int index = 0;
	PFILEINFO pInfo = (PFILEINFO)(GetPacket().strData.c_str());
	while (pInfo->HasNext) {
		if (pInfo->IsDirectory) {
			//中文乱码 因为第一个参数是LPCSTR的
			//InserItem使用的是Unicode编码
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
	//弹出文件对话框    隐藏只读属性的文件 和如果文件重名弹出覆盖提示框
	CFileDialog fileDlg(TRUE, "*", pController->getFilePath(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, &pController->m_mainDlg);
	//模态方式弹出文件对话框
	if (fileDlg.DoModal() == IDOK)
	{
		int ret = pController->sendCommandPacket(4, (BYTE*)pController->getFilePath().GetString(), pController->getFilePath().GetLength());
		TRACE("\r\n下载:%s\r\n",pController->getFilePath());
		if (ret < 0)
		{
			AfxMessageBox("文件下载命令执行失败", MB_TOPMOST);
			pController->closeConnect();
			return;
		}
		while (pController->DealCommand() != 4) {}
		//获取长度
		long long fileSize = *(long long*)(pController->GetPacket().strData.c_str());
		if (fileSize == 0)
		{
			MessageBox(pController->m_mainDlg, "文件为空或由于权限不足等原因无法读取文件", "downLoad error", MB_OK);
			pController->closeConnect();
			return;
		}
		localPath = fileDlg.GetPathName();
		pFile = fopen(localPath, "wb+");
		if (pFile == NULL) {
			MessageBox(pController->m_mainDlg,"本地没有权限保存该文件或者文件无法创建!!!", "downLoad error", MB_OK);
			pController->closeConnect();
			//这里有待优化 应该向服务端发送一个命令通知服务端不用发送文件数据了
			return;
		}
		long long loadedSize = 0;//下载完成的大小
		pController->m_statusDlg.info_edit.SetWindowText("文件下载命令正在执行中");
		pController->m_statusDlg.ShowWindow(SW_SHOW);
		pController->m_statusDlg.CenterWindow(&pController->m_mainDlg);
		pController->m_statusDlg.SetActiveWindow();
		while (loadedSize < fileSize)
		{
			int ret = pController->DealCommand();
			if (ret < 0)
			{
				MessageBox(pController->m_mainDlg,"文件下载错误","downLoad error",MB_OK);
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
			MessageBox(NULL,"文件下载未完成，请检查错误","downLoad error",MB_OK);
		}
		else
		{
			isClose = true;
			pController->m_statusDlg.ShowWindow(SW_HIDE);
			// 获取当前活动窗口的句柄
			MessageBox(pController->m_mainDlg,pController->getFilePath()+ "下载完成+储存在:" + localPath,"",MB_OK);
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
		//监视窗口关闭了直接结束这个函数
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
					TRACE("接收到一张屏幕截图\r\n");
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

//初始化控制器
int CController::InitController()
{
	m_thread = (HANDLE)_beginthreadex(NULL, 0, &CController::threadEntry, this, 0, &m_threadId);
	m_statusDlg.Create(IDD_DIALOG_STATUS, &m_mainDlg);
	m_statusDlg.ShowWindow(SW_HIDE);
	m_screenDlg.Create(IDD_DIALOG_SCREEN, &m_mainDlg);
	m_screenDlg.ShowWindow(SW_HIDE);
	return 0;
}

//启动控制器
int CController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_mainDlg;
	return m_mainDlg.DoModal();
	return 0;
}

//发送包
LRESULT CController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

//发送数据
LRESULT CController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

//展示状态窗口
LRESULT CController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.DoModal();
}

//远程监控
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

//由于跨线程 所以这里并没有直接将要局部变量uuid和msg传给另一个线程 导致另一个线程访问的时候数据已经失效了
//所以种类用类成员变量map做了一个数据持久化
LRESULT CController::SendMessageToThread(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)
		return -2;
	MsgInfo msgInfo(msg);
	PostThreadMessage(m_threadId, WM_SEND_MESSAGE, (WPARAM)&msgInfo, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, -1);//无限制等待
	return msgInfo.result;
}

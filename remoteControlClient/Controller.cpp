#include "pch.h"
#include "Controller.h"
#include"Utils.h"
#include<list>
using std::list;
CController::Helper CController::helper;
CController* CController::m_instance = nullptr;


// 1、查看磁盘分区
// 2、查看指定目录下的文件
// 3、打开文件
// 4、下载文件
// 5、鼠标操作
// 6、发送屏幕内容
// 7、锁机
// 8、解锁
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
	if (ifrecv) {//需要应答

		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		CPacket pack(cmd, data, size, hEvent);
		pack.ifRecvMutiple = ifMutiple;
		pClient->addSendPacket(pack);
		TRACE("添加了图片请求\r\n");
		auto pr = pClient->m_packMap.insert({ hEvent, std::list<CPacket>() });
		WaitForSingleObject(hEvent, INFINITE);//一直等待到完成收包
		TRACE("等到了图片响应\r\n");
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
	else//不要应答
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
			//中文乱码 因为第一个参数是LPCSTR的
			//InserItem使用的是Unicode编码
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
	CController* pController = (CController*)arg;
	pController->downLoadFileFunc();
	_endthread();
}

void CController::downLoadFileFunc()
{
	downLoadMutex.lock();//不能同时下载多个文件
	bool isClose = false;
	FILE* pFile = nullptr;
	CString localPath;
	//弹出文件对话框    隐藏只读属性的文件 和如果文件重名弹出覆盖提示框
	CFileDialog fileDlg(TRUE, "*", m_filePath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, &m_mainDlg);
	//模态方式弹出文件对话框
	long long loadedSize = 0;//下载完成的大小
	long long fileSize = 0;
	if (fileDlg.DoModal() == IDOK)
	{
		localPath = fileDlg.GetPathName();
		pFile = fopen(localPath, "wb+");
		if (pFile == NULL) {
			MessageBox(m_mainDlg, "本地没有权限保存该文件或者文件无法创建!!!", "downLoad error", MB_OK);
			downLoadMutex.unlock();
			return;
		}
		m_pDownLoadFile = pFile;
		CClientSocket::getClientSocketInstance()->setDownLoadFilePath(m_filePath, pFile);
		TRACE("\r\n下载:%s\r\n", m_filePath);
		m_statusDlg.info_edit.SetWindowText("文件下载命令正在执行中");
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_mainDlg);
		m_statusDlg.SetActiveWindow();
		list<CPacket>resultPacketList;
		int ret=sendCommandPacket(true, resultPacketList, 4, (BYTE*)m_filePath.GetString(), m_filePath.GetLength(), true);
		if (!ret)
		{
			m_statusDlg.ShowWindow(SW_HIDE);
			AfxMessageBox("文件下载命令执行失败", MB_TOPMOST);
			fclose(pFile);
			downLoadMutex.unlock();
			return;
		}
		if (resultPacketList.empty()||resultPacketList.front().sCmd==FILEEMPTY)
		{
			m_statusDlg.ShowWindow(SW_HIDE);
			AfxMessageBox("文件为空或者由于权限不足等原因下载失败", MB_TOPMOST);
			fclose(pFile);
			downLoadMutex.unlock();
			return;
		}
		if (resultPacketList.front().sCmd == LOADSIZELOWER)
		{
			isClose = true;
			m_statusDlg.ShowWindow(SW_HIDE);
			MessageBox(NULL, "文件下载未完成，请检查错误", "downLoad error", MB_OK);
		}
		else if(resultPacketList.front().sCmd ==LOADSUCCESS)
		{
			isClose = true;
			m_statusDlg.ShowWindow(SW_HIDE);
			// 获取当前活动窗口的句柄
			MessageBox(m_mainDlg, m_filePath + "下载完成+储存在:" + localPath, "", MB_OK);
		}
	}
	if (pFile != NULL)
		fclose(pFile);
	if (!isClose) {
		m_statusDlg.ShowWindow(SW_HIDE);
	}
	downLoadMutex.unlock();
}

CController::CController() :

	m_thread(INVALID_HANDLE_VALUE),
	m_threadId(-1)
{
	
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
		//监视窗口关闭了直接结束这个函数
		if (pController->ifWatchDlgClose)
		{
			pController->isImgValid = false;
			pController->screenImg.Destroy();
			break;
		}
		if (!pController->isImgValid)
		{
			TRACE("发送了一次图片命令\r\n");
			int ret = pController->sendCommandPacket(true, resultPacketList, 6);
			TRACE("完成了一次图片命令\r\n");
			if (ret)
			{
				if (!resultPacketList.empty()) {
					CPacket pack = resultPacketList.front();
					resultPacketList.pop_front();
					if (pack.sCmd == 6)
					{
						//TRACE("接收到一张屏幕截图,和校验%08X:\r\n",pack.sSum);
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
void CController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{

}

//发送数据
void CController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
}

//展示状态窗口
void CController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	m_statusDlg.DoModal();
}

//远程监控
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

CController* CController::getInstance()
{
	if (m_instance == nullptr)
		m_instance = new CController();
	return m_instance;
}

void CController::releaseInstance()
{
	if (m_instance != nullptr)
		delete m_instance;
	m_instance = nullptr;
}
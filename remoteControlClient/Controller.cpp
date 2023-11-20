#include "pch.h"
#include "Controller.h"

CController::Helper CController::helper;
CController* CController::m_instance = nullptr;


CController::CController():
	
	m_thread(INVALID_HANDLE_VALUE),
	m_threadId(-1),
	m_statusDlg(&m_mainDlg),
	m_screenDlg(&m_mainDlg)
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



//初始化控制器
int CController::InitController()
{
	m_thread = (HANDLE)_beginthreadex(NULL, 0, &CController::threadEntry, this, 0, &m_threadId);
	m_statusDlg.Create(IDD_DIALOG_STATUS, &m_mainDlg);
	m_screenDlg.Create(IDD_DIALOG_SCREEN, &m_mainDlg);
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

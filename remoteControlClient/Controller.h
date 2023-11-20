#pragma once
#include"pch.h"
#include"framework.h"
#include"CStatusDlg.h"
#include"CScreenDialog.h"
#include"remoteControlClientDlg.h"
#include"resource.h"
#include<thread>
#include<map>
using std::thread;
using std::map;
#define WM_SEND_PACK (WM_USER+1)//发送数据包
#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示转台
#define WM_WATCH_SCREEN (WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000) //发送线程消息
class CController
{
	typedef LRESULT(CController::* MSGFUNC)(UINT, WPARAM, LPARAM);
private:

	static CController* m_instance;
	CStatusDlg m_statusDlg;
	CScreenDialog m_screenDlg;
	CremoteControlClientDlg m_mainDlg;
	HANDLE m_thread;
	unsigned int m_threadId;
	map<int, MSGFUNC>m_msgFuncMap;
public:
	//初始化操作
	int InitController();
	//启动
	int Invoke(CWnd*& pMainWnd);
	//消息相应函数
	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	LRESULT OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	//线程如入口函数
	static unsigned __stdcall threadEntry(void* arg);
	//线程处理函数
	void threadFunc();
	//像线程发送消息
	LRESULT SendMessageToThread(MSG msg);
	//获取全局唯一对象
	static CController* getInstance()
	{
		if (m_instance == nullptr)
			m_instance = new CController();
		return m_instance;
	}
	//销毁全局对象
	static void releaseInstance() {
		if (m_instance != nullptr)
			delete m_instance;
		m_instance = nullptr;
	}
private:
	CController();
	~CController();
	CController& operator=(const CController client) {
	}
	struct MsgInfo {
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m)
		{
			if (this != &m)
			{
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
		}
	};

	class Helper {
	public:
		Helper() {
			//不能在这里getInstance()因为在mfc的 WinApp实体加载之前 helper就加载了 那么就创建了Controller实体 但是成员m_mainDialog的图标需要AfxGetApp()->loadIcon()去加载所以这个要在WinApp实体加载完成之后才能创建是Controller实例
		}
		~Helper()
		{
			releaseInstance();
		}
	};
	static Helper helper;
};


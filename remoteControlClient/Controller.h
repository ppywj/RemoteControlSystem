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
#define WM_SEND_PACK (WM_USER+1)//�������ݰ�
#define WM_SEND_DATA (WM_USER+2)//��������
#define WM_SHOW_STATUS (WM_USER+3)//չʾת̨
#define WM_WATCH_SCREEN (WM_USER+4)//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000) //�����߳���Ϣ
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
	//��ʼ������
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);
	//��Ϣ��Ӧ����
	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	LRESULT OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	//�߳�����ں���
	static unsigned __stdcall threadEntry(void* arg);
	//�̴߳�����
	void threadFunc();
	//���̷߳�����Ϣ
	LRESULT SendMessageToThread(MSG msg);
	//��ȡȫ��Ψһ����
	static CController* getInstance()
	{
		if (m_instance == nullptr)
			m_instance = new CController();
		return m_instance;
	}
	//����ȫ�ֶ���
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
			//����������getInstance()��Ϊ��mfc�� WinAppʵ�����֮ǰ helper�ͼ����� ��ô�ʹ�����Controllerʵ�� ���ǳ�Աm_mainDialog��ͼ����ҪAfxGetApp()->loadIcon()ȥ�����������Ҫ��WinAppʵ��������֮����ܴ�����Controllerʵ��
		}
		~Helper()
		{
			releaseInstance();
		}
	};
	static Helper helper;
};


#pragma once
#include"pch.h"
#include"framework.h"
#include"CStatusDlg.h"
#include"CScreenDialog.h"
#include"remoteControlClientDlg.h"
#include"resource.h"
#include"ClientSocket.h"
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
	typedef void(CController::* MSGFUNC)(UINT, WPARAM, LPARAM);
private:

	static CController* m_instance;
	bool ifStartSendAndRcvThread = false;  //�Ƿ������շ��߳�
	HANDLE m_thread;
	unsigned int m_threadId;
	map<int, MSGFUNC>m_msgFuncMap;
	CString m_filePath;
	FILE* m_pDownLoadFile=NULL;
	mutex downLoadMutex;

public:
	CStatusDlg m_statusDlg;
	CScreenDialog m_screenDlg;
	CremoteControlClientDlg m_mainDlg;
	//���Զ����Ļ
	void watchScreen(CController*controller);
  //����߳�
	static void watchThread(void* arg);
	CImage screenImg;//��ͼ�Ļ���
	bool isImgValid;//�����Ƿ���Ч
	bool ifWatchDlgClose = true;
	//��ʼ������
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);
	//��Ϣ��Ӧ����
	void OnSendPack(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnSendData(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	//�߳�����ں���
	static unsigned __stdcall threadEntry(void* arg);
	//�̴߳�����
	void threadFunc();
	//���̷߳�����Ϣ
	LRESULT SendMessageToThread(MSG msg);
	//��ȡȫ��Ψһ����
	static CController* getInstance();
	//����ȫ�ֶ���
	static void releaseInstance();
	//����
	int sendCommandPacket(bool ifrecv,std::list<CPacket>& resultPackList,WORD cmd, BYTE* data = nullptr, size_t size = 0,bool ifMutiple = false);
	//�հ�
	//int DealCommand();
	//�ر�����
	void closeConnect();
	//�õ���
	const CPacket& GetPacket();
	//����ip��port
	void setIpAndPort(CString ip, short port=9527);
	//��ʼ��socket
	int initSocket();
	//����Ŀ¼
	void loadDirectory(CString filePath, CListCtrl& file_list, CTreeCtrl&fileTree, HTREEITEM&hSelected);
	//�����ļ�
	void downLoadFile(CString filePath,CController*controller);


private:
	void downLoadFileFunc();
	CController();
	~CController();
	void setFilePath(const CString& path);
	const CString getFilePath();
	//�����ļ��̺߳���
	static  void downLoadFileThread(void* arg);
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


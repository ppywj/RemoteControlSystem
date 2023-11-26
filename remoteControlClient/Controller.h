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
#define WM_SEND_PACK (WM_USER+1)//发送数据包
#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示转台
#define WM_WATCH_SCREEN (WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000) //发送线程消息
class CController
{
	typedef void(CController::* MSGFUNC)(UINT, WPARAM, LPARAM);
private:

	static CController* m_instance;
	bool ifStartSendAndRcvThread = false;  //是否开启了收发线程
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
	//监控远程屏幕
	void watchScreen(CController*controller);
  //监控线程
	static void watchThread(void* arg);
	CImage screenImg;//截图的缓存
	bool isImgValid;//缓存是否有效
	bool ifWatchDlgClose = true;
	//初始化操作
	int InitController();
	//启动
	int Invoke(CWnd*& pMainWnd);
	//消息相应函数
	void OnSendPack(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnSendData(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	//线程如入口函数
	static unsigned __stdcall threadEntry(void* arg);
	//线程处理函数
	void threadFunc();
	//像线程发送消息
	LRESULT SendMessageToThread(MSG msg);
	//获取全局唯一对象
	static CController* getInstance();
	//销毁全局对象
	static void releaseInstance();
	//发包
	int sendCommandPacket(bool ifrecv,std::list<CPacket>& resultPackList,WORD cmd, BYTE* data = nullptr, size_t size = 0,bool ifMutiple = false);
	//收包
	//int DealCommand();
	//关闭连接
	void closeConnect();
	//拿到包
	const CPacket& GetPacket();
	//设置ip，port
	void setIpAndPort(CString ip, short port=9527);
	//初始化socket
	int initSocket();
	//加载目录
	void loadDirectory(CString filePath, CListCtrl& file_list, CTreeCtrl&fileTree, HTREEITEM&hSelected);
	//下载文件
	void downLoadFile(CString filePath,CController*controller);


private:
	void downLoadFileFunc();
	CController();
	~CController();
	void setFilePath(const CString& path);
	const CString getFilePath();
	//下载文件线程函数
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
			//不能在这里getInstance()因为在mfc的 WinApp实体加载之前 helper就加载了 那么就创建了Controller实体 但是成员m_mainDialog的图标需要AfxGetApp()->loadIcon()去加载所以这个要在WinApp实体加载完成之后才能创建是Controller实例
		}
		~Helper()
		{
			releaseInstance();
		}
	};
	static Helper helper;
};


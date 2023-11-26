#pragma once
#include"pch.h"
#include"framework.h"
#include<vector>
#include<string>
#include<list>
#include<map>
#include"CPacket.h"
#include<mutex>
using std::mutex;
using std::list;
using std::map;
using std::string;
#define BUF_SIZE 4*1024*1024
#define WM_SEND_PACK (WM_USER+1)//发送数据包
#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示转台
#define WM_WATCH_SCREEN (WM_USER+4)//远程监控



std::string getErrorInfo(int wsaErrorCode);

class CClientSocket
{
public:
	static CClientSocket* getClientSocketInstance()
	{
		if (m_clientSocket == NULL) {
			m_clientSocket = new CClientSocket;
		}
		return m_clientSocket;
	}
	bool InitSocket();

	CPacket& GetPacket()
	{
		return packet;
	}
	void close();



	bool Send(const char* pData, size_t nSize)
	{
		if (client_socket == -1 || client_socket == INVALID_SOCKET)
		{
			return false;
		}
		return send(client_socket, pData, nSize, 0) > 0;
	}

	bool Send(CPacket& packet)
	{
		if (client_socket == -1 || client_socket == INVALID_SOCKET)
		{
			return false;
		}
		int len = send(client_socket, packet.data(), packet.nLength + 2 + sizeof(DWORD), 0);
		return len > 0;
	}
	//获取文件路径
	bool getFilePath(std::string& pathStr)
	{
		if (packet.sCmd >= 2 || packet.sCmd <= 4)
		{
			pathStr = packet.strData;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool GetMouseEvent(MOUSEEV& mouse) {
		if (packet.sCmd == 5)
		{
			memcpy(&mouse, packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	bool ifWathClose()
	{
		return ifWathDlgClose;
	}
	void updateIfWathClose(bool status)
	{
		ifWathDlgClose = status;
	}
	void setIpAndPort(CString IP, short PORT = 9527) {
		m_ip = IP;
		m_port = PORT;
	}
	static void recvThreadEntry(void* arg);
	static void recvThread(void* arg);
	list<CPacket>m_sendPacketList;//发送包队列
	std::map<HANDLE, std::list<CPacket>>m_packMap;
	SOCKET client_socket;
	void addSendPacket(CPacket pack);
	CPacket getSendPacket();
	int getSendListSize();
	void setDownLoadFilePath(CString filePath, FILE* pFile);
private:
	CString m_filePath;
	FILE* m_pDownLoadFile;
	CClientSocket();
	CClientSocket(const CClientSocket& client);
	CClientSocket& operator=(const CClientSocket client);
	~CClientSocket();
	BOOL InitSocketEnv();
	//消息相应函数
	void OnSendPack(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnSendData(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	void OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM  lParam);
	/*******************************************************************/
	mutex sendListMutex;
	void recvFunc();
	bool ifWathDlgClose = true;//监视窗口是否关闭了
	std::vector<char>m_buffer;
	CPacket packet;
	static CClientSocket* m_clientSocket;
	CString m_ip;
	short m_port;


	static void releaseServerSocket();
	class Deletor
	{
	public:
		Deletor()
		{
			getClientSocketInstance();
		}
		~Deletor()
		{
			releaseServerSocket();
		}
	};
	static Deletor deletor;
};


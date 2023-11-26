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
	bool InitSocket()
	{
		if (client_socket == INVALID_SOCKET)
			closesocket(client_socket);
		client_socket = socket(PF_INET, SOCK_STREAM, 0);
		if (client_socket == -1)
			return false;
		sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(m_ip.GetString());
		server_addr.sin_port = 9527;
		int res = connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr));
		if (res == -1)
		{
			client_socket = INVALID_SOCKET;
			return false;
		}
		return true;
	}
	CPacket& GetPacket()
	{
		return packet;
	}
	void close()
	{
		if (client_socket != INVALID_SOCKET)
			closesocket(client_socket);
		client_socket =INVALID_SOCKET;
	}

	

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
		int len=send(client_socket, packet.data(), packet.nLength + 2 + sizeof(DWORD), 0);
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
private:
	mutex sendListMutex;
	void recvFunc();
	bool ifWathDlgClose = true;//监视窗口是否关闭了
	std::vector<char>m_buffer;
	CPacket packet;
	static CClientSocket* m_clientSocket;
	CString m_ip;
	short m_port;
	CClientSocket()
	{
		client_socket = INVALID_SOCKET;
		if (InitSocketEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(BUF_SIZE);
		memset(m_buffer.data(), 0, BUF_SIZE);
	}
	CClientSocket(const CClientSocket& client)
	{
		client_socket = client.client_socket;
		packet = client.packet;
		m_buffer = client.m_buffer;
		m_clientSocket = client.m_clientSocket;
		ifWathDlgClose = client.ifWathDlgClose;
	}
	CClientSocket& operator=(const CClientSocket client) {
		client_socket = client.client_socket;
		packet = client.packet;
		m_buffer = client.m_buffer;
		m_clientSocket = client.m_clientSocket;
		ifWathDlgClose = client.ifWathDlgClose;
	}
	~CClientSocket()
	{
		closesocket(client_socket);
		WSACleanup();
	}
	BOOL InitSocketEnv()
	{
		//初始化网络库
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;
		}
		return TRUE;
	}
	static void releaseServerSocket()
	{
		if (m_clientSocket != NULL) {
			delete m_clientSocket;
			m_clientSocket = NULL;
		}
	}
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


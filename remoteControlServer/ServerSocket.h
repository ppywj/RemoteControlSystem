#pragma once
#include"pch.h"
#include"framework.h"
#include"CPacket.h"
#include<list>
#include<thread>
using std::list;
#define BUF_SIZE 4096
class CServerSocket
{
public:
	typedef int (*SOCKET_CALLBACK)(void* arg, int cmd, list<CPacket>& packetList, CPacket& inPacket);
	int run(SOCKET_CALLBACK call_back, void* arg, short port = 9527) {
		m_arg = arg;
		m_callback = call_back;
		if (!InitSocketEnv())
			return 1;
		if (!InitSocket(port))
			return 2;
		for (;;)
		{
			if (!AcceptClient())
				continue;
			int ret = DealCommand();
			if (ret == 0)
			{
				m_callback(m_arg, packet.sCmd, m_packetList, packet);
				while (m_packetList.size()>0)
				{
					Send(m_packetList.front());
					m_packetList.pop_front();
					Sleep(1);
				}
			}

		}
		return 0;
	}
	static CServerSocket* getServerSocketInstance()
	{
		if (m_serverSocket == NULL) {
			m_serverSocket = new CServerSocket;
		}
		return m_serverSocket;
	}
	bool InitSocket(short port)
	{
		server_socket = socket(PF_INET, SOCK_STREAM, 0);
		if (server_socket == -1)
			return false;
		sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = port;
		//绑定
		if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
			return false;
		//监听
		if (listen(server_socket, 1) == -1)
			return false;
		return true;
	}
	bool AcceptClient() {
		sockaddr_in client_addr;
		int clientAddr_sz = sizeof(client_addr);
		client_socket = accept(server_socket, (sockaddr*)&client_addr, &clientAddr_sz);
		if (client_socket == -1)
			return false;
		//TRACE(_T("一个客户端连接到服务器"));
		return true;
	}
	CPacket& GetPacket()
	{
		return packet;
	}
	int DealCommand() {
		char buffer[BUF_SIZE];
		memset(buffer, 0, BUF_SIZE);
		size_t index = 0;
		while (true)
		{
			//读取到缓冲去的空位置
			size_t len = recv(client_socket, buffer + index, BUF_SIZE - index, 0);
			if (len <= 0)
				return -1;
			index += len;
			len = index;
			packet = CPacket((BYTE*)buffer, len);
			if (len >= 0)
			{
				index -= len;
				//把后面未读的数据移动到缓冲区最前面
				memmove(buffer, buffer + len, BUF_SIZE - len);
				return 0;
			}
			else
			{
				return -1;
			}
		}
	}

	bool Send(const char* pData, size_t nSize)
	{
		if (client_socket == -1)
			return false;
		return send(client_socket, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& packet)
	{
		if (client_socket == -1)
			return false;
		return send(client_socket, packet.data(), packet.nLength + sizeof(DWORD) + 2, 0);
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
	void closeClient()
	{
		closesocket(client_socket);
		client_socket = INVALID_SOCKET;
	}
private:
	CPacket packet;
	SOCKET client_socket;
	SOCKET server_socket;
	static CServerSocket* m_serverSocket;
	SOCKET_CALLBACK m_callback;
	std::list<CPacket> m_packetList;
	void* m_arg;
	CServerSocket()
	{
		server_socket = INVALID_SOCKET;
		client_socket = INVALID_SOCKET;
		if (InitSocketEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
	}
	CServerSocket(const CServerSocket& server)
	{
		client_socket = server.client_socket;
		server_socket = server.server_socket;
	}
	CServerSocket& operator=(const CServerSocket server) {}
	~CServerSocket()
	{
		closesocket(server_socket);
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
		if (m_serverSocket != NULL) {
			delete m_serverSocket;
			m_serverSocket = NULL;
		}
	}
	class Deletor
	{
	public:
		Deletor()
		{
			getServerSocketInstance();
		}
		~Deletor()
		{
			releaseServerSocket();
		}
	};
	static Deletor deletor;
};



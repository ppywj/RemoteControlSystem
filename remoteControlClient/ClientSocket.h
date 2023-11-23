#pragma once
#include<vector>
#include"pch.h"
#include"framework.h"
#include<string>
#define BUF_SIZE 800*1024


typedef struct file_info {
	file_info() {
		IsValid = 1;
		IsDirectory = 0;
		memset(szFileName, 0, sizeof(szFileName));
	}
	file_info(BOOL IsValid, BOOL IsDirectory, BOOL HasNext)
	{
		this->IsDirectory = IsDirectory;
		this->IsValid = IsValid;
		this->HasNext = HasNext;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsValid;//是否是有效的 0否 1是	
	BOOL IsDirectory;//是否为目录0 否, 1 是
	char szFileName[256];
	BOOL HasNext = TRUE;//是否还有下一个文件

}FILEINFO, * PFILEINFO;

class CPacket {
public:

	CPacket(WORD mCmd, const BYTE* Data, size_t nSize)
	{
		sHead = 0xFEFF;
		sCmd = mCmd;
		nLength = nSize + 4;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), Data, nSize);
		}
		else
		{
			strData.clear();
		}
		sSum = 0;
		for (int i = 0; i < nSize; i++)
		{
			sSum += (BYTE)strData[i] & 0xFF;
		}
	}
	CPacket() :sHead(0xFEFF), nLength(0), sCmd(0), sSum(0) {}
	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			this->sHead = pack.sHead;
			this->sCmd = pack.sCmd;
			this->nLength = pack.nLength;
			this->strData = pack.strData;
			this->sSum = pack.sSum;
		}
		return *this;
	}
	CPacket(const BYTE* pData, size_t& nSize) :sHead(0), nLength(0), sCmd(0), sSum(0)
	{
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		//没有找到包头  或者包不全
		if (i + sizeof(DWORD) + 2 + 2 > nSize)//4 2 2分别表示长度，控制命令，和校验
		{
			nSize = 0;
			return;
		}
		//读取控制命令到和校验的长度
		nLength = *(DWORD*)(pData + i);
		i += sizeof(DWORD);
		//包不全
		if (nLength + i > nSize)
		{
			nSize = 0;
			return;
		}
		//读取控制命令
		sCmd = *(WORD*)(pData + i);
		i += 2;
		//读取数据
		strData.resize(nLength - 4);
		memcpy((void*)strData.c_str(), pData + i, nLength - 4);
		i += (nLength - 4);
		//读取和校验
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			//对每一个字节的低八位累加
			sum += BYTE(strData[j]) & 0xFF;
		}
		//校验成功
		if (sum == sSum)
		{
			nSize = i;//head nLength
			return;
		}
		//校验失败
		nSize = 0;
	}
	CPacket(const CPacket& packet)
	{
		sHead = packet.sHead;
		nLength = packet.nLength;
		sCmd = packet.nLength;
		strData = packet.strData;
		sSum = packet.sSum;
	}
	~CPacket()
	{

	}
	int size()//返回数据包的大小
	{
		return nLength + 2 + sizeof(DWORD);
	}
	const char* data()
	{
		completeData.resize(nLength + 2 + sizeof(DWORD));
		BYTE* pData = (BYTE*)completeData.c_str();
		*(WORD*)pData = sHead;
		pData += 2;
		*(DWORD*)pData = nLength;
		pData += sizeof(DWORD);
		*(WORD*)pData = sCmd;
		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;
		return completeData.c_str();
	}
	//private:
	WORD sHead;//包头
	DWORD nLength;//长度（从控制命令开始到和校验结束）
	WORD sCmd;//控制命令
	std::string strData;//数据
	WORD sSum;//和校验
	std::string completeData;

};

//鼠标事件结构体
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击 移动 双击 
	WORD nButton;//左键 右键 中键
	POINT ptXY;//点击的坐标
}MOUSEEV, * PMOUSEEV;

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
		if (client_socket != -1)
			closesocket(client_socket);
		client_socket = -1;
	}

	int DealCommand(bool ifrecv = true) {
		static size_t index = 0;
		if (client_socket == -1 || client_socket == INVALID_SOCKET)
		{
			return -1;
		}
		char* buffer = m_buffer.data();


		while (true)
		{
			//读取到缓冲去的空位置
			size_t len = 0;
			if (ifrecv) {
				len = recv(client_socket, buffer + index, BUF_SIZE - index, 0);
				if ((len <= 0) && index == 0)
					return -1;
			}
			index += len;
			len = index;
			TRACE("%d\r\n", index);
			//解析来自服务端的目录数据
			packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				TRACE("index=%d,len=%d\r\n", index, len);
				index -= len;
				//把后面未读的数据移动到缓冲区最前面
				memmove(buffer, buffer + len, BUF_SIZE - len);
				return packet.sCmd;
			}
			else
			{
				return -1;
			}
		}
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
		return send(client_socket, packet.data(), packet.nLength + 2 + sizeof(DWORD), 0);
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
	void setIpAndPort(CString IP, short PORT=9527) {
		m_ip = IP;
		m_port = PORT;
	}
private:
	bool ifWathDlgClose = true;//监视窗口是否关闭了
	std::vector<char>m_buffer;
	CPacket packet;
	SOCKET client_socket;
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


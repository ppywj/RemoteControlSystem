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
	BOOL IsValid;//�Ƿ�����Ч�� 0�� 1��	
	BOOL IsDirectory;//�Ƿ�ΪĿ¼0 ��, 1 ��
	char szFileName[256];
	BOOL HasNext = TRUE;//�Ƿ�����һ���ļ�

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
		//û���ҵ���ͷ  ���߰���ȫ
		if (i + sizeof(DWORD) + 2 + 2 > nSize)//4 2 2�ֱ��ʾ���ȣ����������У��
		{
			nSize = 0;
			return;
		}
		//��ȡ���������У��ĳ���
		nLength = *(DWORD*)(pData + i);
		i += sizeof(DWORD);
		//����ȫ
		if (nLength + i > nSize)
		{
			nSize = 0;
			return;
		}
		//��ȡ��������
		sCmd = *(WORD*)(pData + i);
		i += 2;
		//��ȡ����
		strData.resize(nLength - 4);
		memcpy((void*)strData.c_str(), pData + i, nLength - 4);
		i += (nLength - 4);
		//��ȡ��У��
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			//��ÿһ���ֽڵĵͰ�λ�ۼ�
			sum += BYTE(strData[j]) & 0xFF;
		}
		//У��ɹ�
		if (sum == sSum)
		{
			nSize = i;//head nLength
			return;
		}
		//У��ʧ��
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
	int size()//�������ݰ��Ĵ�С
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
	WORD sHead;//��ͷ
	DWORD nLength;//���ȣ��ӿ������ʼ����У�������
	WORD sCmd;//��������
	std::string strData;//����
	WORD sSum;//��У��
	std::string completeData;

};

//����¼��ṹ��
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//��� �ƶ� ˫�� 
	WORD nButton;//��� �Ҽ� �м�
	POINT ptXY;//���������
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
			//��ȡ������ȥ�Ŀ�λ��
			size_t len = 0;
			if (ifrecv) {
				len = recv(client_socket, buffer + index, BUF_SIZE - index, 0);
				if ((len <= 0) && index == 0)
					return -1;
			}
			index += len;
			len = index;
			TRACE("%d\r\n", index);
			//�������Է���˵�Ŀ¼����
			packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				TRACE("index=%d,len=%d\r\n", index, len);
				index -= len;
				//�Ѻ���δ���������ƶ�����������ǰ��
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
	//��ȡ�ļ�·��
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
	bool ifWathDlgClose = true;//���Ӵ����Ƿ�ر���
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
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ����������������ã�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
		//��ʼ�������
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


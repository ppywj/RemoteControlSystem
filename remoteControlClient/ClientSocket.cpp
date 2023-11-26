#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_clientSocket = NULL;
CClientSocket::Deletor CClientSocket::deletor;

std::string getErrorInfo(int wsaErrorCode)
{
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

bool CClientSocket::InitSocket()
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
void CClientSocket::close()
{
	if (client_socket != INVALID_SOCKET)
		closesocket(client_socket);
	client_socket = INVALID_SOCKET;
}

void CClientSocket::recvThreadEntry(void* arg)
{
	_beginthread(CClientSocket::recvThread, 0, arg);
}

void CClientSocket::recvThread(void* arg)
{
	CClientSocket* pClient = (CClientSocket*)arg;
	pClient->recvFunc();
	_endthread();
}

void CClientSocket::addSendPacket(CPacket pack)
{
	sendListMutex.lock();
	m_sendPacketList.push_back(pack);
	sendListMutex.unlock();
}

CPacket CClientSocket::getSendPacket()
{
	sendListMutex.lock();
	CPacket pack;
	pack.sHead = 0;
	if (m_sendPacketList.size() > 0)
	{
		pack = m_sendPacketList.front();
		m_sendPacketList.pop_front();
	}
	sendListMutex.unlock();
	return pack;
}

int CClientSocket::getSendListSize()
{
	sendListMutex.lock();
	int size = m_sendPacketList.size();
	sendListMutex.unlock();
	return size;
}

void CClientSocket::setDownLoadFilePath(CString filePath, FILE* pFile)
{
	m_filePath = filePath;
	m_pDownLoadFile = pFile;
}


//这种机制有一个bug 当下载的文件非常大的时候list容器会内存爆炸
void CClientSocket::recvFunc()
{
	string strBuffer;
	strBuffer.resize(BUF_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	for (;;)
	{
		CPacket head;
		//不是下载文件命令

		if (getSendListSize() > 0)
		{
			head = getSendPacket();
		}
		else
			continue;
		if (head.sHead == 0)
			continue;
		if (!InitSocket())
		{
			TRACE("初始化失败\r\n");
			if (head.hEvent != NULL)
				SetEvent(head.hEvent);
			continue;

		}
		bool ifSendSuccess = Send(head);
		if (head.hEvent == NULL)
		{
			close();
			continue;
		}
		if (!ifSendSuccess)
		{
			TRACE("发送失败\r\n");
			close();
			SetEvent(head.hEvent);
			continue;
		}
		if (head.sCmd !=4)
		{
			if (!head.ifRecvMutiple) //只接受一个包
			{
				int length = recv(client_socket, pBuffer + index, BUF_SIZE - index, 0);
				//TRACE("客户端接受长度为:%d\r\n", length);
				if (length > 0 || index > 0)
				{
					index += length;
					size_t size = (size_t)index;
					CPacket pack((BYTE*)pBuffer, size);
					if (size > 0)
					{
						memmove(pBuffer, pBuffer + size, BUF_SIZE - size);
						index -= size;
						pack.hEvent = head.hEvent;
						auto pr = m_packMap.find(head.hEvent);
						pr->second.push_back(pack);
					}

				}
			}
			else//接收到多个包
			{
				auto pr = m_packMap.find(head.hEvent);
				for (;;) {
					int length = recv(client_socket, pBuffer + index, BUF_SIZE - index, 0);
					//TRACE("客户端接受长度为:%d\r\n", length);
					if (length > 0 || index > 0)
					{
						index += length;
						for (;;) {
							size_t size = (size_t)index;
							CPacket pack((BYTE*)pBuffer, size);
							if (size > 0)
							{
								memmove(pBuffer, pBuffer + size, BUF_SIZE - size);
								index -= size;
								pr->second.push_back(pack);
							}
							else
							{
								break;
							}
						}
					}
					else {
						break;
					}
				}
			}
		}
		else //是下载文件命令
		{
			auto pr = m_packMap.find(head.hEvent);
			long long fileSize = 0;
			bool ifFirstRcv = true;
			bool ifLoop = true;
			long long loadedSize = 0;
			while(ifLoop) {
				int length = recv(client_socket, pBuffer + index, BUF_SIZE - index, 0);
				//TRACE("客户端接受长度为:%d\r\n", length);
				if (length > 0 || index > 0)
				{
					index += length;
					for (;;) {
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);
						if (size > 0)
						{
							if (ifFirstRcv)
							{
								fileSize = *(long long*)(pack.strData.c_str());
								ifFirstRcv = false;
								if (fileSize == 0)
								{
									pr->second.push_back(CPacket(FILEEMPTY, NULL, 0, NULL));
									ifLoop = false;
									break;
								}
							}
							else {
								fwrite((pack.strData.c_str()), 1, (pack.strData.size()), m_pDownLoadFile);
								loadedSize += pack.strData.size();
							}
							memmove(pBuffer, pBuffer + size, BUF_SIZE - size);
							index -= size;
						}
						else
						{
							break;
						}
					}
				}
				else {
					break;
				}
			}
			if (loadedSize == fileSize)
			{
				pr->second.push_back(CPacket(LOADSUCCESS, NULL, 0, NULL));
			}
			else
			{
				pr->second.push_back(CPacket(FILEEMPTY, NULL, 0, NULL));
			}
			m_pDownLoadFile = NULL;
		}
		close();
		SetEvent(head.hEvent);
	}
	close();
}

CClientSocket::CClientSocket()
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

CClientSocket::CClientSocket(const CClientSocket& client)
{
	client_socket = client.client_socket;
	packet = client.packet;
	m_buffer = client.m_buffer;
	m_clientSocket = client.m_clientSocket;
	ifWathDlgClose = client.ifWathDlgClose;
}

CClientSocket& CClientSocket::operator=(const CClientSocket client)
{
	client_socket = client.client_socket;
	packet = client.packet;
	m_buffer = client.m_buffer;
	m_clientSocket = client.m_clientSocket;
	ifWathDlgClose = client.ifWathDlgClose;
	return *this;
}

CClientSocket::~CClientSocket()
{
	closesocket(client_socket);
	WSACleanup();
}

BOOL CClientSocket::InitSocketEnv()
{
	//初始化网络库
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
	{
		return FALSE;
	}
	return TRUE;
}



void CClientSocket::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
}

void CClientSocket::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
}

void CClientSocket::OnWatchScreen(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
}

void CClientSocket::releaseServerSocket()
{
	if (m_clientSocket != NULL) {
		delete m_clientSocket;
		m_clientSocket = NULL;
	}
}
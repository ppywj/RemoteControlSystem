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

void CClientSocket::recvFunc()
{
	string strBuffer;
	strBuffer.resize(BUF_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	for (;;)
	{
		CPacket head;
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
		int size = Send(head);
		if (head.hEvent == NULL)
		{
			close();
			continue;
		}
		if (size <= 0)
		{
			TRACE("发送失败\r\n");
			close();
			SetEvent(head.hEvent);
			continue;
		}
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
		close();
		SetEvent(head.hEvent);
	}
	close();
}

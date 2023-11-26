#pragma once
#include"pch.h"
#include"framework.h"
#include<string>


enum {
	FILEEMPTY=1000,
	LOADSIZELOWER=1001,
	LOADSUCCESS=1002
};

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

class CPacket {
public:

	CPacket(WORD mCmd, const BYTE* Data, size_t nSize,HANDLE hEvent)
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
		this->hEvent = hEvent;
	}
	CPacket() :sHead(0xFEFF), nLength(0), sCmd(0), sSum(0),hEvent(INVALID_HANDLE_VALUE),strData(""),completeData("") {}
	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			this->sHead = pack.sHead;
			this->sCmd = pack.sCmd;
			this->nLength = pack.nLength;
			this->strData = pack.strData;
			this->sSum = pack.sSum;
			this->hEvent = pack.hEvent;
			this->completeData = pack.completeData;
			this->ifRecvMutiple = pack.ifRecvMutiple;
		}
		return *this;
	}
	CPacket(const BYTE* pData, size_t& nSize) :sHead(0), nLength(0), sCmd(0), sSum(0),hEvent(INVALID_HANDLE_VALUE)
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
		sCmd = packet.sCmd;
		strData = packet.strData;
		sSum = packet.sSum;
		hEvent = packet.hEvent;
		completeData = packet.completeData;
		ifRecvMutiple = packet.ifRecvMutiple;
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
	DWORD nLength=0;//���ȣ��ӿ������ʼ����У�������
	WORD sCmd;//��������
	std::string strData;//����
	WORD sSum;//��У��
	std::string completeData;
	HANDLE hEvent;
	bool ifRecvMutiple = false;//�Ƿ���ܶ����
};


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
	BOOL IsValid;//是否是有效的 0否 1是	
	BOOL IsDirectory;//是否为目录0 否, 1 是
	char szFileName[256];
	BOOL HasNext = TRUE;//是否还有下一个文件

}FILEINFO, * PFILEINFO;


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
	DWORD nLength=0;//长度（从控制命令开始到和校验结束）
	WORD sCmd;//控制命令
	std::string strData;//数据
	WORD sSum;//和校验
	std::string completeData;
	HANDLE hEvent;
	bool ifRecvMutiple = false;//是否接受多个包
};


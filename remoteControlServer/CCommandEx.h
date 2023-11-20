#pragma once
#include"CLockDlg.h"
#include<map>
#include<list>
#include"CPacket.h"

using std::list;
class CCommandEx
{
	typedef int(CCommandEx::* CMDFUNC)(list<CPacket>& packetList, CPacket& inPacket);//成员函数指针
private:
	CLockDlg  lockDlg;
	bool ifLock;//是否锁机的信号
	std::map<int, CMDFUNC>m_mapFunction;
public:
	static int runCommand(void* thiz, int type,list<CPacket>&packetList,CPacket&inPacket);
	int executeCommand(int nCmd, list<CPacket>& packetList, CPacket& inPacket);
	CCommandEx();
	~CCommandEx();
protected:
	//创建磁盘分区
	int MakeDriverInfo(list<CPacket>& packetList, CPacket& inPacket);

	//获取目录的详细信息
	int MakeDirectoryInfo(list<CPacket>& packetList, CPacket& inPacket);

	//打开文件
	int RunFile(list<CPacket>& packetList, CPacket& inPacket);

	//下载文件
	int DownloadFile(list<CPacket>& packetList, CPacket& inPacket);


	//获取鼠标事件
	int MouseEventEx(list<CPacket>& packetList, CPacket& inPacket);

	//锁机线程函数
	void  lockThread(void* arg);

	//锁机
	int lockMachine(list<CPacket>& packetList, CPacket& inPacket);

	//解锁
	int unLockMachine(list<CPacket>& packetList, CPacket& inPacket);

	//删除文件
	int deleteFile(list<CPacket>& packetList, CPacket& inPacket);

	//发送屏幕图像
	int sendScreen(list<CPacket>& packetList, CPacket& inPacket);

};


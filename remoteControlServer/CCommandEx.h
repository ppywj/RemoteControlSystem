#pragma once
#include"CLockDlg.h"
#include<map>
#include<list>
#include"CPacket.h"

using std::list;
class CCommandEx
{
	typedef int(CCommandEx::* CMDFUNC)(list<CPacket>& packetList, CPacket& inPacket);//��Ա����ָ��
private:
	CLockDlg  lockDlg;
	bool ifLock;//�Ƿ��������ź�
	std::map<int, CMDFUNC>m_mapFunction;
public:
	static int runCommand(void* thiz, int type,list<CPacket>&packetList,CPacket&inPacket);
	int executeCommand(int nCmd, list<CPacket>& packetList, CPacket& inPacket);
	CCommandEx();
	~CCommandEx();
protected:
	//�������̷���
	int MakeDriverInfo(list<CPacket>& packetList, CPacket& inPacket);

	//��ȡĿ¼����ϸ��Ϣ
	int MakeDirectoryInfo(list<CPacket>& packetList, CPacket& inPacket);

	//���ļ�
	int RunFile(list<CPacket>& packetList, CPacket& inPacket);

	//�����ļ�
	int DownloadFile(list<CPacket>& packetList, CPacket& inPacket);


	//��ȡ����¼�
	int MouseEventEx(list<CPacket>& packetList, CPacket& inPacket);

	//�����̺߳���
	void  lockThread(void* arg);

	//����
	int lockMachine(list<CPacket>& packetList, CPacket& inPacket);

	//����
	int unLockMachine(list<CPacket>& packetList, CPacket& inPacket);

	//ɾ���ļ�
	int deleteFile(list<CPacket>& packetList, CPacket& inPacket);

	//������Ļͼ��
	int sendScreen(list<CPacket>& packetList, CPacket& inPacket);

};


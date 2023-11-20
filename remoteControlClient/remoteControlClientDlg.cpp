
// remoteControlClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include"resource.h"
#include "remoteControlClient.h"
#include "remoteControlClientDlg.h"
#include "afxdialogex.h"
#include "Utils.h"
#include<filesystem>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CScreenDialog.h"

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:

};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()


// CremoteControlClientDlg 对话框



CremoteControlClientDlg::CremoteControlClientDlg(CWnd* pParent )//=nullptr)
	: CDialogEx(IDD_REMOTECONTROLCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CremoteControlClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_DIR, fileTree);
	DDX_Control(pDX, IDC_EDIT_ip, ipEdit);
	DDX_Control(pDX, IDC_LIST1, file_list);
}

BEGIN_MESSAGE_MAP(CremoteControlClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CremoteControlClientDlg::OnBnClickedButtonConnect)
	ON_EN_CHANGE(IDC_EDIT_ip, &CremoteControlClientDlg::OnEnChangeEditip)
	ON_BN_CLICKED(IDC_BUTTON_FILEINFO, &CremoteControlClientDlg::OnBnClickedButtonFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CremoteControlClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CremoteControlClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_32771, &CremoteControlClientDlg::openFile)
	ON_COMMAND(ID_32772, &CremoteControlClientDlg::downLoadFile)
	ON_COMMAND(ID_32773, &CremoteControlClientDlg::deleteFile)
	ON_BN_CLICKED(IDC_BUTTON_WATCH, &CremoteControlClientDlg::OnBnClickedButtonWatch)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CremoteControlClientDlg 消息处理程序

BOOL CremoteControlClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_statusDlg.Create(IDD_DIALOG_STATUS, this);
	m_statusDlg.ShowWindow(SW_HIDE);
	isImgValid = false;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CremoteControlClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CremoteControlClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CremoteControlClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CremoteControlClientDlg::updateImgScreenStatus()
{
	isImgValid = false;
}

void CremoteControlClientDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


//点击连接按钮的响应函数
void CremoteControlClientDlg::OnBnClickedButtonConnect()
{
	//首先获取输入的ip地址
	CString strIP;
	GetDlgItemText(IDC_EDIT_ip, strIP);
	if (strIP.IsEmpty())
	{
		AfxMessageBox("ip不能为空");
		return;
	}
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	int ret = pClient->InitSocket(strIP.GetString());
	if (!ret)
	{
		AfxMessageBox("连接失败");
	}
	else
		AfxMessageBox("连接成功");
	//发送测试
	pClient->SCTest();
}


void CremoteControlClientDlg::OnEnChangeEditip()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

//查看文件信息按钮 点击响应函数
//获取被控制端的磁盘分区信息
void CremoteControlClientDlg::OnBnClickedButtonFileinfo()
{
	if (sendCommandPacket(1) != 0)
		return;
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	int ret = pClient->DealCommand();
	pClient->close();
	if (ret != -1)
	{
		std::string dirStr = pClient->GetPacket().strData;
		std::vector<std::string>dirs;
		Utils::split(dirs, dirStr, ',');
		for (std::string dir : dirs)
		{
			fileTree.InsertItem((dir + ":").c_str(), TVI_ROOT, TVI_LAST);
		}
	}
	pClient->close();
}
// 1、查看磁盘分区
// 2、查看指定目录下的文件
// 3、打开文件
// 4、下载文件
// 5、鼠标操作
// 6、发送屏幕内容
// 7、锁机
// 8、解锁
//向被控制端发包
int CremoteControlClientDlg::sendCommandPacket(WORD nCmd, BYTE* pData, size_t length)
{
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	CString strIP;
	GetDlgItemText(IDC_EDIT_ip, strIP);
	if (pClient->InitSocket(strIP.GetString()) == false)
		return -1;
	if (pClient->Send(CPacket(nCmd, pData, length)) == false)
		return -1;
	return 0;
}




//双击文件树中的项
void CremoteControlClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	//清空文件列表
	file_list.DeleteAllItems();
	//CPoint ptMouse;
	//GetCursorPos(&ptMouse);
	//fileTree.ScreenToClient(&ptMouse);
	//HTREEITEM hTreeSelected1 = fileTree.HitTest(ptMouse, 0);
	HTREEITEM hTreeSelected = fileTree.GetSelectedItem();
	//这两个是一样的

	if (hTreeSelected == NULL)
	{
		return;
	}
	//如果是文件 不是目录那么直接return掉

	//删除之前插入的相当于做一个更新
	DeleteTreeChildrenItem(hTreeSelected);
	CString strPath = getPath(hTreeSelected);
	int nCmd = sendCommandPacket(2, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	CClientSocket* client = CClientSocket::getClientSocketInstance();
	while (client->DealCommand() != 2)
	{
	}

	int index = 0;
	PFILEINFO pInfo = (PFILEINFO)(client->GetPacket().strData.c_str());
	while (pInfo->HasNext) {
		if (pInfo->IsDirectory) {
			//中文乱码 因为第一个参数是LPCSTR的
			//InserItem使用的是Unicode编码
			if (!(CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == ".."))
				fileTree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
		}
		else {
			file_list.InsertItem(index++, pInfo->szFileName);

		}
		int cmd = client->DealCommand();
		if (cmd < 0)
			break;
		pInfo = (PFILEINFO)(client->GetPacket().strData.c_str());
		//TRACE("fileName:%s\r\n", pInfo->szFileName);
	}
	client->close();
}

//获取完整路径
CString CremoteControlClientDlg::getPath(HTREEITEM hTree)
{
	if (hTree == NULL)
		return "";
	CString strRet, strTmp;
	do {
		strTmp = fileTree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = fileTree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}

//删除指定项的子项
void CremoteControlClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = fileTree.GetChildItem(hTree);
		if (hSub != NULL)
			fileTree.DeleteItem(hSub);
	} while (hSub != NULL);
}

void CremoteControlClientDlg::loadCurrentFile()
{
	HTREEITEM hSelected = fileTree.GetSelectedItem();
	CString dirPath = getPath(hSelected);
	file_list.DeleteAllItems();
	HTREEITEM hTreeSelected = fileTree.GetSelectedItem();
	//这两个是一样的

	if (hTreeSelected == NULL)
	{
		return;
	}
	//如果是文件 不是目录那么直接return掉

	//删除之前插入的相当于做一个更新
	DeleteTreeChildrenItem(hTreeSelected);
	CString strPath = getPath(hTreeSelected);
	int nCmd = sendCommandPacket(2, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	CClientSocket* client = CClientSocket::getClientSocketInstance();
	while (client->DealCommand() != 2)
	{
	}

	PFILEINFO pInfo = (PFILEINFO)(client->GetPacket().strData.c_str());
	while (pInfo->HasNext) {
		if (pInfo->IsDirectory)
		{
			if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..")
			{
				//.表示当前目录  .. 表示上一级目录 如果将这两个添加进来就会套娃
				int cmd = client->DealCommand();
				if (cmd < 0)
				{
					break;
				}
				pInfo = (PFILEINFO)(client->GetPacket().strData.c_str());
				continue;
			}
		}
		if (pInfo->IsDirectory) {
			fileTree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
		}
		else {
			file_list.InsertItem(0, pInfo->szFileName);
		}

		int cmd = client->DealCommand();
		if (cmd < 0)
			break;
		pInfo = (PFILEINFO)(client->GetPacket().strData.c_str());
	}
	client->close();

}

//文件传输的提示线程
void CremoteControlClientDlg::threadEntryForDownFile(void* arg)
{
	CremoteControlClientDlg* thiz = (CremoteControlClientDlg*)arg;
	thiz->threadDownFile();
	_endthread();
}

void CremoteControlClientDlg::threadEntryForWatchData(void* arg)
{
	CremoteControlClientDlg* thiz = (CremoteControlClientDlg*)arg;
	thiz->threadWatchData();
	//清空残留的屏幕数据
	AfxMessageBox("线程退出");
	_endthread();
}

void CremoteControlClientDlg::threadWatchData()
{
	Sleep(50);
	CClientSocket* pClient = NULL;
	do {
		pClient = CClientSocket::getClientSocketInstance();
	} while (pClient == NULL);

	for (;;)
	{
		//监视窗口关闭了直接结束这个函数
		if (pClient->ifWathClose())
		{
			updateImgScreenStatus();
			break;
		}
		if (!isImgValid) 
		{
			int ret = sendCommandPacket(6);
			if (ret == 0)
			{
				int cmd = pClient->DealCommand();
				if (cmd == 6)
				{
					TRACE("接收到一张屏幕截图\r\n");

					//TODO:存入screenImg
					IStream* pStream = NULL;
					BYTE* pData = (BYTE*)pClient->GetPacket().strData.c_str();
					HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
					if (hMem == NULL)
					{
						TRACE("内存不足\r\n");
						Sleep(1);
						continue;
					}
					HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
					if (hRet == S_OK) {
						ULONG length = 0;
						pStream->Write(pData, (pClient->GetPacket().strData.size()), &length);
						screenImg.Load(pStream);
						isImgValid = true;
					}
					else {
						continue;
					}
				}

			}
			pClient->close();
			Sleep(20);

		}
		else
		{
			Sleep(1);
		}

	}

}

//文件传输的线程函数
void CremoteControlClientDlg::threadDownFile()
{
	bool isClose = false;
	//首先获取文件路径
	int nListSelected = file_list.GetSelectionMark();
	CString FileName = file_list.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = fileTree.GetSelectedItem();
	CString dirPath = getPath(hSelected);
	CString filePath = dirPath + FileName;
	FILE* pFile = nullptr;
	CString localPath;
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	//弹出文件对话框    隐藏只读属性的文件 和如果文件重名弹出覆盖提示框
	CFileDialog fileDlg(TRUE, "*", FileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	//模态方式弹出文件对话框
	if (fileDlg.DoModal() == IDOK)
	{
		int ret = sendCommandPacket(4, (BYTE*)filePath.GetString(), filePath.GetLength());
		TRACE("\r\n下载:%s\r\n", filePath);
		if (ret < 0)
		{
			AfxMessageBox("文件下载命令执行失败", MB_TOPMOST);
			pClient->close();
			return;
		}
		while (pClient->DealCommand() != 4) {}
		//获取长度
		long long fileSize = *(long long*)(pClient->GetPacket().strData.c_str());
		if (fileSize == 0)
		{
			AfxMessageBox("文件为空或由于权限不足等原因无法读取文件", MB_TOPMOST);
			pClient->close();
			return;
		}
		localPath = fileDlg.GetPathName();
		pFile = fopen(localPath, "wb+");
		if (pFile == NULL) {
			AfxMessageBox("本地没有权限保存该文件或者文件无法创建!!!", MB_TOPMOST);
			pClient->close();
			//这里有待优化 应该向服务端发送一个命令通知服务端不用发送文件数据了
			return;
		}
		long long loadedSize = 0;//下载完成的大小
		BeginWaitCursor();
		m_statusDlg.info_edit.SetWindowText("文件下载命令正在执行中");
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(this);
		m_statusDlg.SetActiveWindow();
		while (loadedSize < fileSize)
		{
			int ret = pClient->DealCommand();
			if (ret < 0)
			{
				MessageBox("文件下载错误", MB_OK);
				fclose(pFile);
				pClient->close();
				return;
			}
			fwrite((pClient->GetPacket().strData.c_str()), 1, (pClient->GetPacket().strData.size()), pFile);
			loadedSize += pClient->GetPacket().strData.size();
		}
		if (loadedSize < fileSize)
		{
			isClose = true;
			EndWaitCursor();
			m_statusDlg.ShowWindow(SW_HIDE);
			MessageBox("文件下载未完成，请检查错误", MB_OK);
		}
		else
		{
			isClose = true;
			EndWaitCursor();
			m_statusDlg.ShowWindow(SW_HIDE);
			// 获取当前活动窗口的句柄
			MessageBox(FileName + "下载完成+储存在:" + localPath, MB_OK);
		}

	}
	if (pFile != NULL)
		fclose(pFile);
	pClient->close();
	if (!isClose) {
		EndWaitCursor();
		m_statusDlg.ShowWindow(SW_HIDE);
	}
}



//右键单机文件
void CremoteControlClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	file_list.ScreenToClient(&ptList);
	int selected = file_list.HitTest(ptList, 0);
	if (selected < 0)
	{
		return;
	}
	CMenu menu;
	menu.LoadMenuA(IDR_MENU1);
	CMenu* pPopup = menu.GetSubMenu(0);
	if (pPopup != NULL)
	{
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}
	*pResult = 0;
}


//打开文件
void CremoteControlClientDlg::openFile()
{
	//首先获取文件路径
	int nListSelected = file_list.GetSelectionMark();
	CString FileName = file_list.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = fileTree.GetSelectedItem();
	CString dirPath = getPath(hSelected);
	CString filePath = dirPath + FileName;
	//发送打开文件命令
	sendCommandPacket(3, (BYTE*)filePath.GetString(), filePath.GetLength());
	CClientSocket::getClientSocketInstance()->close();
}



//下载文件
void CremoteControlClientDlg::downLoadFile()
{
	_beginthread(CremoteControlClientDlg::threadEntryForDownFile, 0, this);
	Sleep(50);//保证线程正常启动之不改动界面

}





//删除文件
void CremoteControlClientDlg::deleteFile()
{
	//首先获取文件路径
	int nListSelected = file_list.GetSelectionMark();
	CString FileName = file_list.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = fileTree.GetSelectedItem();
	CString dirPath = getPath(hSelected);
	CString filePath = dirPath + FileName;
	//发送删除文件命令
	sendCommandPacket(9, (BYTE*)filePath.GetString(), filePath.GetLength());
	CClientSocket* pClient = CClientSocket::getClientSocketInstance();
	loadCurrentFile();
}


//监控按钮的点击响应函数
void CremoteControlClientDlg::OnBnClickedButtonWatch()
{
	//弹出监控对话框
	CScreenDialog dlg(this);
	CClientSocket::getClientSocketInstance()->updateIfWathClose(false);
	//开启接受解析截屏数据的线程
	_beginthread(threadEntryForWatchData, 0, this);
	dlg.DoModal();
}



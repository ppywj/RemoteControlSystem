
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
#include"Controller.h"
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



CremoteControlClientDlg::CremoteControlClientDlg(CWnd* pParent)//=nullptr)
	: CDialogEx(IDD_REMOTECONTROLCLIENT_DIALOG, pParent)
	, ip(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CremoteControlClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_DIR, fileTree);
	DDX_Control(pDX, IDC_EDIT_ip, ipEdit);
	DDX_Control(pDX, IDC_LIST1, file_list);
	DDX_Text(pDX, IDC_EDIT_ip, ip);
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

	CController::getInstance()->InitController();
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
	UpdateData();
	if (ip.IsEmpty())
	{
		AfxMessageBox("ip不能为空");
		return;
	}
	CController* pController = CController::getInstance();
	pController->setIpAndPort(ip);
	int ret = pController->initSocket();
	if (!ret)
	{
		AfxMessageBox("连接失败");
	}
	else
		AfxMessageBox("连接成功");

}


void CremoteControlClientDlg::OnEnChangeEditip()
{
	UpdateData();
	CController* pController = CController::getInstance();
	pController->setIpAndPort(ip);
}

//查看文件信息按钮 点击响应函数
//获取被控制端的磁盘分区信息
void CremoteControlClientDlg::OnBnClickedButtonFileinfo()
{
	CController* pController = CController::getInstance();
	std::list<CPacket>resultPacketList;
	if (!pController->sendCommandPacket(true,resultPacketList,1))
		return;
	if (resultPacketList.size()>0)
	{
		fileTree.DeleteAllItems();
		file_list.DeleteAllItems();
		std::string dirStr = resultPacketList.front().strData;
		std::vector<std::string>dirs;
		Utils::split(dirs, dirStr, ',');
		for (std::string dir : dirs)
		{
			fileTree.InsertItem((dir + ":").c_str(), TVI_ROOT, TVI_LAST);
		}
	}
}



//双击文件树中的项
void CremoteControlClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	CController* pController = CController::getInstance();
	//清空文件列表
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
	pController->loadDirectory(getPath(hTreeSelected), file_list, fileTree, hTreeSelected);
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


//
//void CremoteControlClientDlg::threadEntryForWatchData(void* arg)
//{
//	CremoteControlClientDlg* thiz = (CremoteControlClientDlg*)arg;
//	thiz->threadWatchData();
//	//清空残留的屏幕数据
//	AfxMessageBox("线程退出");
//	_endthread();
//}
//
//void CremoteControlClientDlg::threadWatchData()
//{
//	CController* pController = CController::getInstance();
//	Sleep(50);
//	CClientSocket* pClient = NULL;
//	do {
//		pClient = CClientSocket::getClientSocketInstance();
//	} while (pClient == NULL);
//
//	for (;;)
//	{
//		//监视窗口关闭了直接结束这个函数
//		if (pClient->ifWathClose())
//		{
//			updateImgScreenStatus();
//			break;
//		}
//		if (!isImgValid)
//		{
//			std::list<CPacket>resultPacketList;
//
//			int ret = pController->sendCommandPacket(resultPacketList, 6);
//			if (ret)
//			{
//					TRACE("接收到一张屏幕截图\r\n");
//					std::string strBuffer = pClient->GetPacket().strData;
//					if (Utils::GetImage(screenImg, strBuffer) == 0)
//						isImgValid = true;
//					else
//						continue;
//
//			}
//			Sleep(20);
//		}
//		else
//		{
//			Sleep(1);
//		}
//	}
//}



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
	CController* pController = CController::getInstance();
	//首先获取文件路径
	int nListSelected = file_list.GetSelectionMark();
	CString FileName = file_list.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = fileTree.GetSelectedItem();
	CString dirPath = getPath(hSelected);
	CString filePath = dirPath + FileName;
	//发送打开文件命令
	std::list<CPacket>resultPacketList;
	pController->sendCommandPacket(false,resultPacketList,3, (BYTE*)filePath.GetString(), filePath.GetLength());
}



//下载文件
void CremoteControlClientDlg::downLoadFile()
{
	CController* pController = CController::getInstance();
	bool isClose = false;
	//首先获取文件路径
	int nListSelected = file_list.GetSelectionMark();
	CString FileName = file_list.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = fileTree.GetSelectedItem();
	CString dirPath = getPath(hSelected);
	CString filePath = dirPath + FileName;
	pController->downLoadFile(filePath,pController);
	Sleep(50);//保证线程正常启动之不改动界面

}





//删除文件
void CremoteControlClientDlg::deleteFile()
{
	CController* pController = CController::getInstance();
	//首先获取文件路径
	int nListSelected = file_list.GetSelectionMark();
	CString FileName = file_list.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = fileTree.GetSelectedItem();
	CString dirPath = getPath(hSelected);
	CString filePath = dirPath + FileName;
	//发送删除文件命令
	std::list<CPacket>resultList;
	pController->sendCommandPacket(false,resultList,9, (BYTE*)filePath.GetString(), filePath.GetLength());
	//pController->closeConnect();
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
	pController->loadDirectory(dirPath, file_list, fileTree, hTreeSelected);
}


//监控按钮的点击响应函数
void CremoteControlClientDlg::OnBnClickedButtonWatch()
{
	CController* pController = CController::getInstance();
	pController->watchScreen(pController);
}




// CScreenDialog.cpp: 实现文件
//

#include "pch.h"
#include "remoteControlClient.h"
#include "CScreenDialog.h"
#include "afxdialogex.h"
#include"remoteControlClientDlg.h"

// CScreenDialog 对话框

IMPLEMENT_DYNAMIC(CScreenDialog, CDialogEx)

CScreenDialog::CScreenDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SCREEN, pParent)
{

}

CScreenDialog::~CScreenDialog()
{
	CClientSocket::getClientSocketInstance()->updateIfWathClose(true);
}

void CScreenDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCREEN, screenEdit);
}




BEGIN_MESSAGE_MAP(CScreenDialog, CDialogEx)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_SCREEN, &CScreenDialog::OnStnClickedScreen)
END_MESSAGE_MAP()


// CScreenDialog 消息处理程序


CPoint CScreenDialog::UserPointToScreenPoint( CPoint& point)
{
	//屏幕坐标转为客户区坐标
	//ScreenToClient(&point);
	//获取客户区的宽和高
	CRect rect;
	screenEdit.GetWindowRect(rect);
	float width = rect.Width();
	float height = rect.Height();
	int rwidth = 1920;
	int rheight = 1080;
	int x = point.x * rwidth/ width;
	int y = point.y * rheight / height;
	return CPoint(x,y);
}

BOOL CScreenDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 50, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CScreenDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CremoteControlClientDlg* pParent = (CremoteControlClientDlg*)GetParent();
		if (pParent->isScreenValid())
		{
			CRect rect;
			screenEdit.GetWindowRect(rect);
			//拿出屏幕数据并显示出来
			//pParent->getScreenImg().BitBlt(screenEdit.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			pParent->getScreenImg().StretchBlt(screenEdit.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			pParent->InvalidateRect(NULL);//通知重绘
			pParent->getScreenImg().Destroy();
			pParent->updateImgScreenStatus();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

//左键按下
void CScreenDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remotePoint = UserPointToScreenPoint(point);
	MOUSEEV event;
	event.ptXY = remotePoint;
	event.nButton = 0;//左键
	event.nAction = 2;//按下
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	//这里可能有一个bug就是套接字可能是无效的
	CClientSocket::getClientSocketInstance()->Send(pack);
	CDialogEx::OnLButtonDown(nFlags, point);
}


//左键弹起
void CScreenDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remotePoint = UserPointToScreenPoint(point);
	MOUSEEV event;
	event.ptXY = remotePoint;
	event.nButton = 0;//左键
	event.nAction = 3;//双击
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	//这里可能有一个bug就是套接字可能是无效的
	CClientSocket::getClientSocketInstance()->Send(pack);
	CDialogEx::OnLButtonUp(nFlags, point);
}

//右键按下
void CScreenDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remotePoint = UserPointToScreenPoint(point);
	MOUSEEV event;
	event.ptXY = remotePoint;
	event.nButton = 0;//左键
	event.nAction = 2;//按下
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	//这里可能有一个bug就是套接字可能是无效的
	CClientSocket::getClientSocketInstance()->Send(pack);
	CDialogEx::OnRButtonDown(nFlags, point);
}

//右键弹起
void CScreenDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remotePoint = UserPointToScreenPoint(point);
	MOUSEEV event;
	event.ptXY = remotePoint;
	event.nButton = 0;//左键
	event.nAction = 3;//弹起
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	//这里可能有一个bug就是套接字可能是无效的
	CClientSocket::getClientSocketInstance()->Send(pack);
	CDialogEx::OnRButtonUp(nFlags, point);
}

//双击左键
void CScreenDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remotePoint = UserPointToScreenPoint(point);
	MOUSEEV event;
	event.ptXY = remotePoint;
	event.nButton = 0;//左键
	event.nAction = 1;//双击
	//这里可能有一个bug就是套接字可能是无效的
	CremoteControlClientDlg* pParent=(CremoteControlClientDlg*)GetParent();
	pParent->sendCommandPacket(5, (BYTE*)&event, sizeof(event));
	CDialogEx::OnLButtonDblClk(nFlags, point);
}

//双击右键
void CScreenDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remotePoint = UserPointToScreenPoint(point);
	MOUSEEV event;
	event.ptXY = remotePoint;
	event.nButton = 2;//右键
	event.nAction = 1;//双击
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	//这里可能有一个bug就是套接字可能是无效的
	CClientSocket::getClientSocketInstance()->Send(pack);
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


//鼠标移动
void CScreenDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//CPoint remotePoint = UserPointToScreenPoint(point);
	//MOUSEEV event;
	//event.ptXY = remotePoint;
	//event.nButton = 8;
	//event.nAction = 0;
	//CPacket pack(5, (BYTE*)&event, sizeof(event));
	////这里可能有一个bug就是套接字可能是无效的
	//CClientSocket::getClientSocketInstance()->Send(pack);
	//CDialogEx::OnMouseMove(nFlags, point);
}

//单击事件处理
void CScreenDialog::OnStnClickedScreen()
{
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);
	CPoint remotePoint = UserPointToScreenPoint(point);
	MOUSEEV event;
	event.ptXY = remotePoint;
	event.nButton = 0;//左键
	event.nAction = 0;//单击
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	//这里可能有一个bug就是套接字可能是无效的
	CClientSocket::getClientSocketInstance()->Send(pack);
}

// CStatusDlg.cpp: 实现文件
//

#include "pch.h"
#include "remoteControlClient.h"
#include "CStatusDlg.h"
#include "afxdialogex.h"


// CStatusDlg 对话框

IMPLEMENT_DYNAMIC(CStatusDlg, CDialogEx)

CStatusDlg::CStatusDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_STATUS, pParent)
{

}

CStatusDlg::~CStatusDlg()
{
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_STATUS, info_edit);
}


BEGIN_MESSAGE_MAP(CStatusDlg, CDialogEx)
END_MESSAGE_MAP()


// CStatusDlg 消息处理程序

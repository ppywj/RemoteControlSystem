// CLockDlg.cpp: 实现文件
//

#include "pch.h"
#include "remoteControlServer.h"
#include "CLockDlg.h"
#include "afxdialogex.h"


// CLockDlg 对话框

IMPLEMENT_DYNAMIC(CLockDlg, CDialog)

CLockDlg::CLockDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CLockDlg, pParent)
{

}

CLockDlg::~CLockDlg()
{
	m_hWnd = NULL;
}

void CLockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLockDlg, CDialog)
END_MESSAGE_MAP()


// CLockDlg 消息处理程序

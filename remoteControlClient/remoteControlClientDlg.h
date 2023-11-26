
// remoteControlClientDlg.h: 头文件
//

#pragma once
#include"ClientSocket.h"
#include"CStatusDlg.h"
// CremoteControlClientDlg 对话框
class CremoteControlClientDlg : public CDialogEx
{
	// 构造
public:
	CremoteControlClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECONTROLCLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CStatusDlg m_statusDlg;
private:
	CImage screenImg;//截图的缓存
	bool isImgValid;//缓存是否有效
public:
	bool isScreenValid() const
	{
		return isImgValid;
	}
	CImage& getScreenImg()
	{
		return screenImg;
	}
	void updateImgScreenStatus();
public:
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnEnChangeEditip();
	afx_msg void OnBnClickedButtonFileinfo();
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	CString getPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);//删除指定项的子项
	//static void threadEntryForWatchData(void* arg);
	//void threadWatchData();
	CListCtrl file_list;
	// 目录树
	CTreeCtrl fileTree;
	// ip编辑框
	CEdit ipEdit;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void openFile();
	afx_msg void downLoadFile();
	afx_msg void deleteFile();
	afx_msg void OnBnClickedButtonWatch();
	CString ip;
};

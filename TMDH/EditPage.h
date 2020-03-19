#pragma once
#include "afxwin.h"


// CEditPageDlg 对话框

class CEditPageDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CEditPageDlg)

public:
	CEditPageDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CEditPageDlg();
	//
	void ShowText(CBuffer* pBuf,BOOL bUtf8);
// 对话框数据
	enum { IDD = IDD_EDIT_PAGE_DLG };
	CEdit m_editCtrl;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

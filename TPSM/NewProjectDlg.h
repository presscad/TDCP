#pragma once


// CNewProjectDlg 对话框

class CNewProjectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewProjectDlg)

public:
	CNewProjectDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNewProjectDlg();

// 对话框数据
	enum { IDD = IDD_NEW_PROJECT_DLG };
	CString m_sPrjName;
	CString m_sPrjPath;
	CString m_sExcelPath;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnFilePath();
	afx_msg void OnBnClickedBtnExcelPath();
public:
	afx_msg void OnEnChangeEProjectName();
};

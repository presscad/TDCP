#pragma once


// CNewProjectDlg �Ի���

class CNewProjectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewProjectDlg)

public:
	CNewProjectDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CNewProjectDlg();

// �Ի�������
	enum { IDD = IDD_NEW_PROJECT_DLG };
	CString m_sPrjName;
	CString m_sPrjPath;
	CString m_sExcelPath;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnFilePath();
	afx_msg void OnBnClickedBtnExcelPath();
public:
	afx_msg void OnEnChangeEProjectName();
};

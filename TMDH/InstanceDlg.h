#pragma once


// CInstanceDlg 对话框

class CInstanceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInstanceDlg)
public:
	BYTE m_ciModelFlag;	//0.LDS|1.TMD
	void* m_pModel;
	CHashStrList<int> m_hashSubLeg;
public:
	CInstanceDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInstanceDlg();

// 对话框数据
	enum { IDD = IDD_INSTANCE_DLG };
	CString m_sHeightName;
	CString m_sLegASerial;
	CString m_sLegBSerial;
	CString m_sLegCSerial;
	CString m_sLegDSerial;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelchangeCmbHugao();
	afx_msg void OnCbnSelchangeCmbLegA();
	afx_msg void OnCbnSelchangeCmbLegB();
	afx_msg void OnCbnSelchangeCmbLegC();
	afx_msg void OnCbnSelchangeCmbLegD();
};

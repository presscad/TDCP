#pragma once


// CInstanceDlg �Ի���

class CInstanceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInstanceDlg)
public:
	BYTE m_ciModelFlag;	//0.LDS|1.TMD
	void* m_pModel;
	CHashStrList<int> m_hashSubLeg;
public:
	CInstanceDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CInstanceDlg();

// �Ի�������
	enum { IDD = IDD_INSTANCE_DLG };
	CString m_sHeightName;
	CString m_sLegASerial;
	CString m_sLegBSerial;
	CString m_sLegCSerial;
	CString m_sLegDSerial;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelchangeCmbHugao();
	afx_msg void OnCbnSelchangeCmbLegA();
	afx_msg void OnCbnSelchangeCmbLegB();
	afx_msg void OnCbnSelchangeCmbLegC();
	afx_msg void OnCbnSelchangeCmbLegD();
};

#pragma once


// CHuGaoPlanDlg �Ի���

class CHuGaoPlanDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHuGaoPlanDlg)

public:
	CHuGaoPlanDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CHuGaoPlanDlg();

// �Ի�������
	enum { IDD = IDD_HUGAO_PLAN_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	int m_nMinHeight;
	int m_nMaxHeight;
	int m_nHeightGroup;
	int m_nMaxLegCoun;
};

#pragma once


// CHuGaoPlanDlg 对话框

class CHuGaoPlanDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHuGaoPlanDlg)

public:
	CHuGaoPlanDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHuGaoPlanDlg();

// 对话框数据
	enum { IDD = IDD_HUGAO_PLAN_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	int m_nMinHeight;
	int m_nMaxHeight;
	int m_nHeightGroup;
	int m_nMaxLegCoun;
};

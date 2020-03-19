#pragma once
#include "RememberLocationDlg.h"

// CTowerTemplateDlg 对话框
class CTowerTemplateDlg : public CRememberLocationDlg
{
	DECLARE_DYNAMIC(CTowerTemplateDlg)
public:
	int m_dwIconWidth,m_dwIconHeight;
	CImageList imageList;
	CXhChar50 m_sTowerName;
	CXhChar100 m_sTemplFile;
	BYTE m_ciCurTowerType;
	ATOM_LIST<CImageRecord> m_xImageRecList;
public:
	CTowerTemplateDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTowerTemplateDlg();
	//
	void InitImageList();
	void RefreshImageLictCtr();
// 对话框数据
	enum { IDD = IDD_TOWER_TEMPLATE_DLG };
	int m_nType;
	CListCtrl m_xIconListCtrl;
	CString m_sFilePath;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnCbnSelchangeCmbType();
	afx_msg void OnBnClickedBtnFilePath();
	afx_msg void OnItemchangedListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
};

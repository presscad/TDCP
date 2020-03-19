#pragma once
#include "RememberLocationDlg.h"

// CTowerTemplateDlg �Ի���
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
	CTowerTemplateDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CTowerTemplateDlg();
	//
	void InitImageList();
	void RefreshImageLictCtr();
// �Ի�������
	enum { IDD = IDD_TOWER_TEMPLATE_DLG };
	int m_nType;
	CListCtrl m_xIconListCtrl;
	CString m_sFilePath;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnCbnSelchangeCmbType();
	afx_msg void OnBnClickedBtnFilePath();
	afx_msg void OnItemchangedListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
};

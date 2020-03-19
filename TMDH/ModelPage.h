#pragma once


// CModelPage 对话框
#include "XhTreeCtrl.h"
#include "TreeItemInfo.h"

class CModelPageDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CModelPageDlg)
	RECT m_rcClient;
	CImageList m_images;
	ATOM_LIST<TREEITEM_INFO> m_xItemInfoList;
	HTREEITEM m_hRootItem;
	CString m_sFileName;
public:
	CModelPageDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModelPageDlg();
	//
	CTreeCtrl *GetTreeCtrl(){return &m_treeCtrl;}
	TREEITEM_INFO *GetSelItemInfo(HTREEITEM* pItem=NULL);
	void ContextMenu(CWnd *pWnd, CPoint point);
	void RelayoutDlg();
	void RefreshTreeCtrl();
	void AppendTidFile(const char* sFilePath);
// 对话框数据
	enum { IDD = IDD_MODEL_PAGE_DLG };
	CXhTreeCtrl m_treeCtrl;
	int m_iModelLevel;
	int m_iLengthUnit;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTvnSelchangedTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnKeydownTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeCmbModelLevel();
	afx_msg void OnCbnSelchangeCmbLengthUnit();
	afx_msg void OnAddItem();
	afx_msg void OnDelItem();
	afx_msg void OnUpdateItem();
	afx_msg void OnExport3dsFile();
	afx_msg void OnExportXmlFile();
	afx_msg void OnExportModFile();
};
